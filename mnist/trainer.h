#pragma once

#include "common.h"

enum class Algorithm {
  CONST,
  MLP,
};

// Only used for debugging purposes, should never be used to influence trainer's
// behavior
using TestEvaluator = std::function<double(IModel)>;

class Trainer {
 public:
  static IModel train(
      ExampleList examples,
      Algorithm algorithm,
      TestEvaluator evaluator = nullptr);
};
