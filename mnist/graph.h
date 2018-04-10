#pragma once

#include "operators.h"

class GraphBuilder {
 public:
  IModel buildMLP(Dim inputDim, int nclass, Dims hiddenLayerDims) const;
};
