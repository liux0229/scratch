#pragma once

#include "common.h"

struct FullyConnectedLayer {
  static FullyConnectedLayer read(std::istream& in);

  Dims hiddenLayerDims;
};

struct ModelArchitecture {
  static ModelArchitecture read(std::istream& in);

  FullyConnectedLayer fcLayer;
};

struct LearningRateStrategy {
  static LearningRateStrategy read(std::istream& in);

  double alpha;
};

struct TrainingConfig {
  static TrainingConfig read(std::istream& in);

  ModelArchitecture modelArch;
  LearningRateStrategy learningRateStrategy;
  int iterations;
  int batchSize;
};

std::ostream& operator<<(std::ostream& out, const ModelArchitecture& modelArch);

std::ostream& operator<<(
    std::ostream& out,
    const LearningRateStrategy& learningRateStrategy);

std::ostream& operator<<(
    std::ostream& out,
    const TrainingConfig& trainingConfig);
