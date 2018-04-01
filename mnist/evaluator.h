#pragma once

#include "common.h"

class Evaluator {
public:
  double evaluate(IModel model, ExampleList tests) const;
};
