#include "graph.h"

#include <functional>
#include <queue>
#include <unordered_map>

using namespace std;

// static
OperatorList GraphBuilder::topologicalSort(IOperator input, IOperator output) {
  struct Entry {
    size_t indegree = 0;
    OperatorList outgoing;
  };

  unordered_map<Operator*, Entry> m;
  function<void(IOperator)> iterator;

  OperatorList ret;
  queue<IOperator> q;

  iterator = [&m, &iterator, &q](IOperator op) {
    auto size = op->getInputs().size();
    if (size == 0) {
      q.push(op);
      // cout << op->name() << " enqueue" << endl;
    }

    auto ret = m.emplace(op.get(), Entry{size});
    if (!ret.second) {
      return;
    }
    for (auto in : op->getInputs()) {
      iterator(in);
      m[in.get()].outgoing.push_back(op);
    }
  };
  iterator(output);

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

// static
pair<IInputOperator, IOperator> GraphBuilder::buildMLP(
    Dim inputDim,
    int nclass,
    const ModelArchitecture& arch) {
  auto input = make_shared<InputOperator>(Dims{inputDim});
  IOperator op = input;

  for (auto layer : arch.layers) {
    op = layer->create(op);
  }

  if (op->dims().size() > 1) {
    op = make_shared<AdapterOperator>(Dims{op->dims().dimSize}, op);
  }
  op = make_shared<FCLayerOperator>(nclass, op);
  op = make_shared<SoftmaxOperator>(op);

  return make_pair(input, op);
}
