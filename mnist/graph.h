#pragma once

#include "operators.h"

class GraphBuilder {
 public:
  static OperatorList topologicalSort(IOperator output, IOperator input);
  // return <input, output>
  static std::pair<IInputOperator, IOperator>
  buildMLP(Dim inputDim, int nclass, Dims hiddenLayerDims);
};
