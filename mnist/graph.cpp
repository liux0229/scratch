#include "graph.h"

#include <functional>
#include <queue>
#include <unordered_map>

using namespace std;

namespace {

OperatorList topologicalSort(IOperator output, IOperator input) {
  struct Entry {
    size_t indegree = 0;
    OperatorList outgoing;
  };
  unordered_map<Operator*, Entry> m;
  function<void(IOperator)> iterator;
  iterator = [&m, &iterator](IOperator op) {
    auto ret = m.emplace(op.get(), Entry{op->getInputs().size()});
    if (!ret.second) {
      return;
    }
    for (auto in : op->getInputs()) {
      iterator(in);
      m[in.get()].outgoing.push_back(op);
    }
  };
  iterator(output);

  OperatorList ret;
  queue<IOperator> q;
  q.push(input);
  while (!q.empty()) {
    auto op = q.front();
    q.pop();
    // cout << ": " << op->name() << endl;
    ret.push_back(op);
    for (auto edge : m[op.get()].outgoing) {
      // cout << " --> " << edge->name() << " " << m[edge.get()].indegree <<
      // endl;
      if (--m[edge.get()].indegree == 0) {
        q.push(edge);
      }
    }
  }

  return ret;
}

class ForwardPassModel : public Model {
 public:
  ForwardPassModel(IInputOperator input, IOperator output)
      : input_(input), output_(output) {
    // Sort the operators topologically
    forwardOrder_ = topologicalSort(output, input);
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

} // namespace

IModel GraphBuilder::buildMLP(Dim inputDim, int nclass, Dims hiddenLayerDims)
    const {
  auto input = make_shared<InputOperator>(inputDim);
  IOperator op = input;

  for (auto dim : hiddenLayerDims) {
    op = make_shared<FCLayerOperator>(dim, op);
    op = make_shared<ReluOperator>(op);
  }
  op = make_shared<FCLayerOperator>(nclass, op);
  op = make_shared<SoftmaxOperator>(op);

  return make_shared<ForwardPassModel>(input, op);
}
