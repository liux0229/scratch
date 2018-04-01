#include "graph.h"

using namespace std;

class ForwardPassModel : public Model {
 public:
  ForwardPassModel(IOperator input, IOperator output) {}

  Prediction predict(const Example& e) const override {
    input_->load(ExampleList{e});
    auto& out = output_->compute();

    Vector v{out};
    // use iterator
    Prediction pred;
    for (int i = 0; i < out.n(); i++) {
      pred[i] = v(i);
    }
  }

 private:
  IOperator input_;
  IOperator output_;
};

IModel GraphBuilder::buildMLP(int nclass, Dims hiddenLayerDims) const {
  auto op = make_shared<InputOperator>();
  auto input = op;

  for (auto dim : hiddenLayerDims) {
    op = make_shared<FCLayerOperator>(dim, op);
    op = make_shared<ReluOperator>(op);
  }
  op = make_shared<FCLayerOperator>(nclass, op);
  op = make_shared<SoftmaxOperator>(op);

  return make_shared<ForwardPassModel>(input_, output_);
}
