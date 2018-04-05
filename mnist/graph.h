#pragma once

#include "operators.h"

class GraphBuilder {
 public:
  IModel buildMLP(int nclass, Dims hiddenLayerDims) const;
};
