#include <folly/Format.h>
#include <atomic>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "graph.h"
#include "trainer.h"

using namespace std;

class ForwardPassModel : public Model {
 public:
  ForwardPassModel(IInputOperator input, IOperator output)
      : input_(input), output_(output) {
    // Sort the operators topologically
    forwardOrder_ = GraphBuilder::topologicalSort(input, output);
    // for (auto op : forwardOrder_) {
    //   cout << op->name() << endl;
    // }
  }

  vector<Prediction> predict(const ExampleList& examples) const override {
    vector<Prediction> ret(examples.size());

    auto T = TaskRunner::get().nThreads();
    auto batches = ExampleRange{examples}.split(T);
    T = batches.size();

    vector<TaskRunner::Task> tasks;
    for (int i = 0; i < T; ++i) {
      tasks.push_back([this, i, &batches, &ret]() {
        input_->load(batches[i]);
        for (auto op : forwardOrder_) {
          op->compute();
        }
        auto& out = output_->get();
        // cout << "out dim: " << out << endl;

        Matrix m{out};

        auto begin = i * batches[0].size();
        for (int e = 0; e < m.rows(); ++e) {
          auto& pred = ret[begin + e];
          for (int k = 0; k < m.cols(); k++) {
            // I want a row view
            pred.prob[k] = m(e, k);
          }
        }
      });
    }

    TaskRunner::get().run(tasks);

    return ret;
  }

  void read(istream& in) {
    for (auto op : forwardOrder_) {
      op->read(in);
    }
  }

  void write(ostream& out) const {
    for (auto op : forwardOrder_) {
      op->write(out);
    }
  }

 private:
  IInputOperator input_;
  IOperator output_;
  OperatorList forwardOrder_;
};

struct Loss {
  Float total() const {
    return trainingLoss + regularizerLoss;
  }
  Float trainingLoss;
  Float regularizerLoss;
};

ostream& operator<<(ostream& out, Loss loss) {
  out << folly::format(
      "training:{} regularizer:{} total:{}",
      loss.trainingLoss,
      loss.regularizerLoss,
      loss.total());
  return out;
}

// Maintain an OutputFile abstraction that allows appending while allowing
// another program to read from it.
class OutputFile {
 public:
  using OstreamPtr = unique_ptr<ostream, function<void(ostream*)>>;

  OutputFile(string path, int cacheOstreamFor)
      : path_(path), cacheOstreamFor_(cacheOstreamFor) {
    runCommand("rm -f {}", path_);
  }

  ostream& openForAppend() {
    if (cachedOstreamAccessed_ >= cacheOstreamFor_) {
      cachedOstream_ = nullptr;
      cachedOstreamAccessed_ = 0;
    }

    if (!cachedOstream_) {
      runCommand("touch {}", path_);
      runCommand("cp {} {}", path_, tmpPath());

      cachedOstream_ = OstreamPtr(
          new ofstream(tmpPath(), ios_base::out | std::ios_base::app),
          [this](ostream* out) {
            delete out;
            runCommand("mv {} {}", tmpPath(), path_);
          });
    }

    ++cachedOstreamAccessed_;
    return *cachedOstream_;
  }

 private:
  template <typename... Args>
  static void runCommand(string fmt, Args&&... args) {
    auto ret =
        system(folly::format(fmt, std::forward<Args>(args)...).str().c_str());
    SCHECK(ret == 0);
  }

  string tmpPath() const {
    return path_ + ".tmp";
  }

  const string path_;
  const int cacheOstreamFor_;
  OstreamPtr cachedOstream_;
  int cachedOstreamAccessed_ = 0;
};

namespace {
class JsonArrayWriter {
 public:
  JsonArrayWriter(string name, ostream& out) : out_(out) {
    out_ << '"' << name << '"' << ": [";
  }
  template <typename T>
  void write(const T& x) {
    out_ << sep << x;
    sep = ", ";
  }
  ~JsonArrayWriter() {
    out_ << "]";
  }

 private:
  const char* sep = "";
  ostream& out_;
};
} // namespace

class SGDTrainer {
 public:
  SGDTrainer(
      IInputOperator input,
      IOperator output,
      ExampleList examples,
      TrainingConfig trainingConfig,
      TestEvaluator evaluator)
      : input_(input),
        output_(output),
        examples_(examples),
        trainingConfig_(trainingConfig),
        evaluator_(evaluator),
        learningCurveOutput_(
            learningCurveConfig().writeTo,
            learningCurveConfig().flushEvery /
                learningCurveConfig().writeOutEvery) {}
  pair<IInputOperator, IOperator> train() {
    label_ = make_shared<InputOperator>(Dims{});
    auto lossOp = make_shared<LossOperator>(output_, label_);

    // fuse softmax and loss
    lossOp_ = make_shared<SoftmaxLossOperator>(
        dynamic_pointer_cast<SoftmaxOperator>(output_), lossOp);

    forwardPass_ = GraphBuilder::topologicalSort(input_, lossOp_);
    for (auto op : forwardPass_) {
      cout << op->name() << endl;
    }

    if (trainingConfig_.modelArch.readModelFrom == "") {
      trainInternal();
    }

    return make_pair(input_, output_);
  }

 private:
  const LearningCurveConfig& learningCurveConfig() const {
    return trainingConfig_.diagnosticsConfig.learningCurveConfig;
  }

  // TODO:
  // We need to remove regularizer from the forward pass and backward pass
  // list and compute its loss, gradient and apply its gradient separately
  void trainInternal() {
    addRegularizer();
    backwardPass_ = buildBackwardPass(forwardPass_);

    const double alpha = trainingConfig_.learningRateStrategy.alpha;
    int exampleIndex = 0;

    for (int i = 0; i < trainingConfig_.iterations; ++i) {
      // cout << "i=" << i << endl;
      printTotalLoss(i);
      printEvaluationResult(i);

      auto batch = prepareBatch(exampleIndex);
      auto g = computeGradient(batch);

      if (trainingConfig_.diagnosticsConfig.verifyGradient) {
        verifyGradient(batch, g);
      }

      // At this point we have done the forward pass
      writeLearningCurve(i);

      SCHECK(forwardPass_.size() + 1 == g.size());
      for (size_t k = 0; k < forwardPass_.size(); ++k) {
        // TODO: look into this copy
        forwardPass_[k]->applyGradient(g[k] * -alpha);
      }
      regularizer_->applyGradient(g[forwardPass_.size()] * -alpha);
    }
  }

  ExampleRange prepareBatch(int& start) {
    const int K = trainingConfig_.batchSize;
    const int N = examples_.size();

    auto ret = ExampleRange(examples_, start, K);

    start = (start + K) % N;
    return ret;
  }

  void loadBatch(const ExampleRange& batch) const {
    input_->load(batch, false);
    label_->load(batch, true);
  }

  // TODO: this would not be completely correct after we perform multithreaded
  // execution
  void writeLearningCurve(int iteration) {
    if (iteration %
            trainingConfig_.diagnosticsConfig.learningCurveConfig
                .writeOutEvery !=
        0) {
      return;
    }

    auto& out = learningCurveOutput_.openForAppend();

    // Write Json format

    out << "{ ";

    out << folly::format("\"iteration\": {}, ", iteration);
    out << folly::format(
        "\"loss\": {}, ", getTotalLoss(Vector{lossOp_->get()}(0)).trainingLoss);

    {
      JsonArrayWriter writer("w.norm", out);
      for (auto op : forwardPass_) {
        if (dynamic_pointer_cast<RegularizerOperator>(op)) {
          continue;
        }
        for (auto* w : op->getParameterList()) {
          writer.write(w->l2Norm());
        }
      }
    }

    {
      out << " , ";
      JsonArrayWriter writer("w.g.norm", out);
      for (auto op : reverse(backwardPass_)) {
        for (auto& g : op->parameterGradient()) {
          writer.write(g.l2Norm());
        }
      }
    }

    {
      out << " , ";
      JsonArrayWriter writer("x.g.norm", out);
      for (auto op : reverse(backwardPass_)) {
        for (auto& g : op->inputGradient()) {
          writer.write(g.l2Norm());
        }
      }
    }

    {
      out << " , ";
      JsonArrayWriter writer("out.norm", out);
      for (auto op : forwardPass_) {
        if (dynamic_pointer_cast<RegularizerOperator>(op)) {
          continue;
        }
        // writer.write(op->name());
        // writer.write(op->get());
        writer.write(op->get().l2Norm());
      }
    }

    out << "}" << endl;
  }

  void printTotalLoss(int i) {
    if (i % trainingConfig_.diagnosticsConfig.lossIterations != 0) {
      return;
    }

    auto loss = runForwardPassAndComputeLoss(ExampleRange(examples_));
    cout << "i=" << i << " loss: " << getTotalLoss(loss) << endl;

    for (auto op : forwardPass_) {
      op->setDiagnostics(true);
    }
  }

  void printEvaluationResult(int i) {
    if (evaluator_ &&
        i % trainingConfig_.diagnosticsConfig.testErrorIterations == 0) {
      auto model = make_shared<ForwardPassModel>(input_, output_);
      cout << folly::format(
                  "i={} test error rate={}%", i, evaluator_(model) * 100)
           << endl;
    }
  }

  void addRegularizer() {
    if (trainingConfig_.regularizerConfig.policy == RegularizerConfig::L2) {
      regularizer_ = make_shared<L2RegularizerOperator>(
          trainingConfig_.regularizerConfig.lambda);
    }
    if (!regularizer_) {
      return;
    }

    for (auto op : forwardPass_) {
      op->attachRegularizer(*regularizer_);
    }
    // forwardPass_.push_back(regularizer_);
  }

  BackPropOperatorList buildBackwardPass(const OperatorList& forwardPass) {
    BackPropOperatorList ret;

    for (auto op : reverse(forwardPass)) {
      ret.push_back(op->getBackPropOperator());
    }
    regularizerBackOp_ = regularizer_->getBackPropOperator();
    for (auto op : forwardPass) {
      // TODO: verify this when we have graphs with more than one input
      int index = 0;
      for (auto in : op->getInputs()) {
        in->getBackPropOperator()->addParent(op->getBackPropOperator(), index);
        ++index;
      }
    }

    // for (auto op : ret) {
    //   cout << op->name() << " parents: ";
    //   for (auto p : op->parents()) {
    //     cout << folly::format("{}:{} ", p.op->name(), p.inputIndex);
    //   }
    //   cout << endl;
    // }

    return ret;
  }

  Float runForwardPassAndComputeLoss(ExampleRange batch) const {
    int T = TaskRunner::get().nThreads();
    auto batches = batch.split(T);
    // cout << "Threaded batch size: " << batches.size() << endl;
    T = batches.size();

    lossOp_->setWeight(1.0 / batch.size());
    vector<Float> losses(T, 0.0);
    vector<TaskRunner::Task> tasks;
    for (int i = 0; i < T; ++i) {
      tasks.push_back([this, i, &batches, &losses]() {
        loadBatch(batches[i]);
        losses[i] = computeLoss();
      });
    }

    TaskRunner::get().run(tasks);

    return accumulate(losses.begin(), losses.end(), 0.0);
  }

  Float computeLoss() const {
    // cout << "compute loss" << endl;
    for (auto op : forwardPass_) {
      // cout << "eval " << op->name() << endl;
      op->compute();
    }
    // cout << "forward pass" << endl;
    return Vector{lossOp_->get()}(0);
  }

  Loss getTotalLoss(Float loss) const {
    Float regularizerLoss = 0.0;
    if (regularizer_) {
      regularizer_->compute();
      regularizerLoss = Vector{regularizer_->get()}(0);
    }

    return Loss{loss, regularizerLoss};
  }

  // return gradient from each operator following the forward order
  GradientList computeGradient(ExampleRange batch) const {
    SCHECK(forwardPass_.size() == backwardPass_.size());

    lossOp_->setWeight(1.0 / batch.size());

    int T = TaskRunner::get().nThreads();
    auto batches = batch.split(T);
    T = batches.size();

    vector<TaskRunner::Task> tasks;
    tasks.reserve(T);
    vector<GradientList> gradientsPerThread(
        T, GradientList(forwardPass_.size()));

    for (int i = 0; i < T; ++i) {
      tasks.push_back([this, i, &batches, &gradientsPerThread]() {
        loadBatch(batches[i]);

        runForwardPass();

        // backward pass
        int index = backwardPass_.size() - 1;
        for (auto op : backwardPass_) {
          op->runBackProp();

          // TODO: look into this copy
          // (e.g. we can make a shallow shared copy;
          // need to write a unit test to verify).
          gradientsPerThread[i][index] = op->parameterGradient();

          // if (op->name() == "l2_regularizer_grad") {
          //   cout << "compute gradient: " << gradients[index][0].data()[0]
          //   << endl;
          // }
          --index;
        }
      });
    }

    TaskRunner::get().run(tasks);

    for (int i = 1; i < T; ++i) {
      for (int j = 0; j < gradientsPerThread[0].size(); ++j) {
        for (int k = 0; k < gradientsPerThread[0][j].size(); ++k) {
          gradientsPerThread[0][j][k] += gradientsPerThread[i][j][k];
        }
      }
    }

    auto gradientList = std::move(gradientsPerThread[0]);
    regularizerBackOp_->runBackProp();
    // cout << "Regularizer gradient size: "
    //      << regularizerBackOp_->parameterGradient().size() << endl;
    gradientList.push_back(regularizerBackOp_->parameterGradient());
    return gradientList;
  }

  void runForwardPass() const {
    for (auto op : forwardPass_) {
      op->compute();
    }
  }

  void verifyGradient(const ExampleRange& batch, const GradientList& g) const {
    const double eps = 1e-2;
    auto gDebug = computeGradientDebug(batch);
    SCHECK_MSG(
        g.size() == gDebug.size(),
        folly::format("{} vs {}", g.size() == gDebug.size()).str());

    for (int i = 0; i < static_cast<int>(g.size()); ++i) {
      SCHECK(g[i].size() == gDebug[i].size());
      for (int j = 0; j < static_cast<int>(g[i].size()); ++j) {
        // cout << "gradient: " << g[i][j] << endl;
        // cout << "debug gradient: " << gDebug[i][j] << endl;

        if (!g[i][j].equals(gDebug[i][j], eps)) {
          auto name = i < forwardPass_.size() ? forwardPass_[i]->name()
                                              : regularizer_->name();
          cout << folly::format("{} #{} gradient not equal:\n", name, j);
          if (trainingConfig_.diagnosticsConfig.gradientVerifyDetails) {
            cout << "gradient: " << g[i][j] << endl;
            cout << "debug gradient: " << gDebug[i][j] << endl;
            SCHECK(false);
          }
        }
      }
    }

    if (trainingConfig_.diagnosticsConfig.gradientVerifyDetails) {
      cout << folly::format(
                  "Gradient verification passed for {} examples", batch.size())
           << endl;
    }
  }

  GradientList computeGradientDebug(ExampleRange batch) const {
    GradientList gradients;
    gradients.reserve(forwardPass_.size());

    for (auto op : forwardPass_) {
      gradients.push_back(op->computeGradientDebug([this, &batch]() {
        // TODO: This reloads the input for every forward pass
        // This is necessary to ensure thread local inputs are accessed
        // correctly

        // Note: this should not include the regularization term
        return runForwardPassAndComputeLoss(batch);
      }));
    }

    // TODO: always creates the regularizer so we can avoid these checks
    if (regularizer_) {
      gradients.push_back(regularizer_->computeGradientDebug([this]() {
        // For the regularizer, we shouldn't compute the total loss
        return getTotalLoss(0.0).total();
      }));
    }

    return gradients;
  }

  IInputOperator input_;
  IOperator output_;
  ExampleList examples_;
  TrainingConfig trainingConfig_;
  // should never be used to influence trainer's behavior
  TestEvaluator evaluator_;
  OutputFile learningCurveOutput_;

  IInputOperator label_;
  ILossOperator lossOp_;
  IRegularizerOperator regularizer_;
  OperatorList forwardPass_;
  BackPropOperatorList backwardPass_;
  IBackPropOperator regularizerBackOp_;
};

IModel Trainer::train(
    ExampleList examples,
    TrainingConfig trainingConfig,
    TestEvaluator evaluator) {
  cout << trainingConfig << endl;

  auto ops = GraphBuilder::buildMLP(
      Tensor{examples, false}.dims()[1], 10, trainingConfig.modelArch);
  ops = SGDTrainer(ops.first, ops.second, examples, trainingConfig, evaluator)
            .train();

  cout << trainingConfig << endl;

  auto model = make_shared<ForwardPassModel>(ops.first, ops.second);
  if (trainingConfig.modelArch.readModelFrom != "") {
    ifstream in(trainingConfig.modelArch.readModelFrom);
    SCHECK(!in.fail());
    model->read(in);
  }

  if (trainingConfig.writeModelTo != "") {
    ofstream out(trainingConfig.writeModelTo);
    model->write(out);
  }

  return model;
}
