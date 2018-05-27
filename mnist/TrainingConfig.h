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

struct TrainingDataConfig {
  static TrainingDataConfig read(std::istream& in);

  std::string trainInput;
  std::string trainLabel;
  std::string testInput;
  std::string testLabel;
};

struct RegularizerConfig {
  static RegularizerConfig read(std::istream& in);

  enum { None, L2 } policy;
  double lambda;
};

struct TrainingConfig {
  static TrainingConfig read(std::istream& in);

  TrainingDataConfig trainingData;
  ModelArchitecture modelArch;
  LearningRateStrategy learningRateStrategy;
  RegularizerConfig regularizerConfig;
  int iterations;
  int batchSize;
};

std::ostream& operator<<(std::ostream& out, const ModelArchitecture& modelArch);

std::ostream& operator<<(
    std::ostream& out,
    const LearningRateStrategy& learningRateStrategy);

std::ostream& operator<<(
    std::ostream& out,
    const RegularizerConfig& regularizerConfig);

std::ostream& operator<<(
    std::ostream& out,
    const TrainingConfig& trainingConfig);
