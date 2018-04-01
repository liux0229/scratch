#include "graph.h"

using namespace std;

class ForwardPassModel : public Model {
 public:
  ForwardPassModel(IInputOperator input, IOperator output)
      : input_(input), output_(output) {}

  Prediction predict(const Example& e) const override {
    input_->load(ExampleList{e});
    auto& out = output_->compute();

    Vector v{out};
    // use iterator
    Prediction pred;
    for (int i = 0; i < v.n(); i++) {
      pred.prob[i] = v(i);
    }

    return pred;
  }

 private:
  IInputOperator input_;
  IOperator output_;
};

IModel GraphBuilder::buildMLP(int nclass, Dims hiddenLayerDims) const {
  auto input = make_shared<InputOperator>();
  IOperator op;

  for (auto dim : hiddenLayerDims) {
    op = make_shared<FCLayerOperator>(dim, op);
    op = make_shared<ReluOperator>(op);
  }
  op = make_shared<FCLayerOperator>(nclass, op);
  op = make_shared<SoftmaxOperator>(op);

  return make_shared<ForwardPassModel>(input, op);
}
