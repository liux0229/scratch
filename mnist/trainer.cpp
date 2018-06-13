#include <folly/Format.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "graph.h"
#include "trainer.h"

using namespace std;

class ConstModel : public Model {
  Prediction predict(const Example& e) const override {
    Prediction p;
    fill(p.prob.begin(), p.prob.end(), 1.0 / p.prob.size());
    return p;
  }
};

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

  Prediction predict(const Example& e) const override {
    input_->load(ExampleList{e});
    for (auto op : forwardOrder_) {
      op->compute();
    }
    auto& out = output_->get();
    // cout << "out dim: " << out << endl;

    Matrix m{out};
    // I want a row view
    Prediction pred;
    for (int i = 0; i < m.cols(); i++) {
      pred.prob[i] = m(0, i);
    }

    return pred;
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

  void trainInternal() {
    addRegularizer();
    backwardPass_ = buildBackwardPass(forwardPass_);

    const double alpha = trainingConfig_.learningRateStrategy.alpha;
    int exampleIndex = 0;

    for (int i = 0; i < trainingConfig_.iterations; ++i) {
      cout << "i=" << i << endl;
      printTotalLoss(i);
      printEvaluationResult(i);

      auto batch = prepareBatch(exampleIndex);
      input_->load(batch, false);
      label_->load(batch, true);
      auto g = computeGradient(batch);

      if (trainingConfig_.diagnosticsConfig.verifyGradient) {
        verifyGradient(batch, g);
      }

      // At this point we have done the forward pass
      writeLearningCurve(i);

      for (size_t k = 0; k < forwardPass_.size(); ++k) {
        // TODO: look into this copy
        forwardPass_[k]->applyGradient(g[k] * -alpha);
      }
    }
  }

  ExampleList prepareBatch(int& start) {
    const int K = trainingConfig_.batchSize;
    const int N = examples_.size();

    ExampleList batch;
    if (start + K > N) {
      start = 0;
    }
    for (int j = 0; j < K; ++j) {
      batch.push_back(examples_[start + j]);
    }
    start += K;
    return batch;
  }

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
        "\"loss\": {}, ", getLossAfterForwardPass().trainingLoss);

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
    if (i % trainingConfig_.diagnosticsConfig.lossIterations == 0) {
      input_->load(examples_, false);
      label_->load(examples_, true);
      cout << "i=" << i << " loss: " << computeLoss() << endl;
      for (auto op : forwardPass_) {
        op->setDiagnostics(true);
      }
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
    forwardPass_.push_back(regularizer_);
  }

  BackPropOperatorList buildBackwardPass(
      const OperatorList& forwardPass) const {
    BackPropOperatorList ret;

    for (auto op : reverse(forwardPass)) {
      ret.push_back(op->getBackPropOperator());
    }
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

  Loss computeLoss() const {
    cout << "compute loss" << endl;
    for (auto op : forwardPass_) {
      cout << "eval " << op->name() << endl;
      op->compute();
    }
    cout << "forward pass" << endl;
    return getLossAfterForwardPass();
  }

  Loss getLossAfterForwardPass() const {
    Float regularizerLoss =
        regularizer_ ? Vector{regularizer_->get()}(0) : static_cast<Float>(0.0);
    return Loss{Vector{lossOp_->get()}(0), regularizerLoss};
  }

  // return gradient from each operator following the forward order
  GradientList computeGradient(const ExampleList& batch) const {
    SCHECK(forwardPass_.size() == backwardPass_.size());

    GradientList gradients(forwardPass_.size());

    // forward pass
    for (auto op : forwardPass_) {
      op->compute();
    }

    // backward pass
    int index = backwardPass_.size() - 1;
    for (auto op : backwardPass_) {
      op->runBackProp();

      // TODO: look into this copy
      gradients[index] = op->parameterGradient();

      // if (op->name() == "l2_regularizer_grad") {
      //   cout << "compute gradient: " << gradients[index][0].data()[0] <<
      //   endl;
      // }
      --index;
    }

    return gradients;
  }

  void verifyGradient(const ExampleList& batch, const GradientList& g) const {
    const double eps = 1e-2;
    auto gDebug = computeGradientDebug(batch);
    SCHECK(g.size() == gDebug.size());

    for (int i = 0; i < static_cast<int>(g.size()); ++i) {
      SCHECK(g[i].size() == gDebug[i].size());
      for (int j = 0; j < static_cast<int>(g[i].size()); ++j) {
        if (!g[i][j].equals(gDebug[i][j], eps)) {
          cout << folly::format(
              "{} #{} gradient not equal:\n", forwardPass_[i]->name(), j);
          if (trainingConfig_.diagnosticsConfig.gradientVerifyDetails) {
            cout << "gradient: " << g[i][j] << endl;
            cout << "debug gradient: " << gDebug[i][j] << endl;
            SCHECK(false);
          }
        }
      }
    }
  }

  GradientList computeGradientDebug(const ExampleList& batch) const {
    GradientList gradients;
    gradients.reserve(forwardPass_.size());

    for (auto op : forwardPass_) {
      auto g =
          op->computeGradientDebug([this]() { return computeLoss().total(); });

      // cout << op->name() << ": " << endl;
      // for (auto& gr : g) {
      //   // cout << gr.dims() << endl;
      //   cout << gr << endl;
      // }

      gradients.push_back(std::move(g));
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
  IOperator lossOp_;
  IRegularizerOperator regularizer_;
  OperatorList forwardPass_;
  BackPropOperatorList backwardPass_;
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
