#include <iostream>

#include <folly/Format.h>

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

 private:
  IInputOperator input_;
  IOperator output_;
  OperatorList forwardOrder_;
};

class SGDTrainer {
 public:
  SGDTrainer(IInputOperator input, IOperator output, ExampleList examples)
      : input_(input), output_(output), examples_(examples) {}
  pair<IInputOperator, IOperator> train() {
    label_ = make_shared<InputOperator>(Dims{});
    lossOp_ = make_shared<LossOperator>(output_, label_);
    forwardPass_ = GraphBuilder::topologicalSort(input_, lossOp_);
    backwardPass_ = buildBackwardPass(forwardPass_);

    int N = examples_.size(); // 60000
    const int K = 200;
    int start = 0;
    int Iter = 100000;

    // ExampleList batch;
    // for (int i = 0; i < K; ++i) {
    //   batch.push_back(examples_[i]);
    // }
    // input_->load(batch, false);
    // label_->load(batch, true);

    double alpha = 0.15;
    for (int i = 0; i < Iter; ++i) {
      // cout << "i=" << i << " loss: " << computeLoss() << endl;

      if (i % 1000 == 0) {
        input_->load(examples_, false);
        label_->load(examples_, true);
        cout << "i=" << i << " loss: " << computeLoss() << endl;
      }

      ExampleList batch;
      if (start + K > N) {
        start = 0;
      }
      for (int j = 0; j < K; ++j) {
        batch.push_back(examples_[start + j]);
      }
      start += K;

      input_->load(batch, false);
      label_->load(batch, true);

      auto g = computeGradient(batch);
      // verifyGradient(batch, g);

      for (size_t k = 0; k < forwardPass_.size(); ++k) {
        forwardPass_[k]->applyGradient(g[k] * -alpha);
      }
    }

    return make_pair(input_, output_);
  }

 private:
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

  double computeLoss() const {
    for (auto op : forwardPass_) {
      op->compute();
    }
    return Vector{lossOp_->get()}(0);
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
      gradients[index--] = op->parameterGradient();
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
          // cout << "gradient: " << g[i][j] << endl;
          // cout << "debug gradient: " << gDebug[i][j] << endl;
          // SCHECK(false);
        }
      }
    }
  }

  GradientList computeGradientDebug(const ExampleList& batch) const {
    GradientList gradients;
    gradients.reserve(forwardPass_.size());

    for (auto op : forwardPass_) {
      auto g = op->computeGradientDebug([this]() { return computeLoss(); });

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
  IInputOperator label_;
  IOperator output_;
  IOperator lossOp_;
  ExampleList examples_;
  OperatorList forwardPass_;
  BackPropOperatorList backwardPass_;
};

IModel Trainer::train(ExampleList examples, Algorithm algorithm) {
  switch (algorithm) {
    case Algorithm::CONST:
      return make_shared<ConstModel>();
    case Algorithm::MLP:
      auto ops =
          GraphBuilder::buildMLP(Tensor{examples, false}.dims()[1], 10, {30});
      ops = SGDTrainer(ops.first, ops.second, examples).train();
      return make_shared<ForwardPassModel>(ops.first, ops.second);
  }
  return nullptr;
}
