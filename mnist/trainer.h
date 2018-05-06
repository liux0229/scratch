#pragma once

#include "common.h"
#include "TrainingConfig.h"

// Only used for debugging purposes, should never be used to influence trainer's
// behavior
using TestEvaluator = std::function<double(IModel)>;

class Trainer {
 public:
  static IModel train(
      ExampleList examples,
      TrainingConfig trainingConfig,
      TestEvaluator evaluator = nullptr);
};
