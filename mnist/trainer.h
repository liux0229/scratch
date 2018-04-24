#pragma once

#include "common.h"

struct FullyConnectedLayer {
  Dims hiddenLayerDims;
};

struct ModelArchitecture {
  FullyConnectedLayer fcLayer;
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

std::ostream& operator<<(std::ostream& out, const ModelArchitecture& modelArch);

std::ostream& operator<<(
    std::ostream& out,
    const LearingRateStrategy& learningRateStrategy);

std::ostream& operator<<(
    std::ostream& out,
    const TrainingConfig& trainingConfig);

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
