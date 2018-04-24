#pragma once

#include "common.h"

struct ModelArchitecture {
  Dims hiddenLayerDims;
};

struct LearingRateStrategy {
  enum class Strategy {
    CONST,
  } strategy;
  double alpha;
};

struct TrainingConfig {
  enum class Algorithm {
    CONST,
    MLP,
  } algorithm;
  ModelArchitecture modelArch;
  LearingRateStrategy learningRateStrategy;
  int iterations;
  int batchSize;
};

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
