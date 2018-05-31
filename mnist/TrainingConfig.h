#pragma once

#include "common.h"

struct FullyConnectedLayer {
  static FullyConnectedLayer read(std::istream& in);

  Dims hiddenLayerDims;
};

struct ModelArchitecture {
  static ModelArchitecture read(std::istream& in);

  FullyConnectedLayer fcLayer;
  std::string readModelFrom;
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

struct LearningCurveConfig {
  static LearningCurveConfig read(std::istream& in);

  std::string writeTo;
  int writeOutEvery = 10;
  int flushEvery = 100;
};

struct DiagnosticsConfig {
  static DiagnosticsConfig read(std::istream& in);

  int lossIterations = 1000;
  int testErrorIterations = 5000;
  bool verifyGradient = false;
  LearningCurveConfig learningCurveConfig;
};

struct EvaluationConfig {
  static EvaluationConfig read(std::istream& in);

  std::string writeEvaluationDetailsTo;
  bool writeAll = false;
};

struct TrainingConfig {
  static TrainingConfig read(std::istream& in);

  TrainingDataConfig trainingData;
  ModelArchitecture modelArch;
  LearningRateStrategy learningRateStrategy;
  RegularizerConfig regularizerConfig;
  DiagnosticsConfig diagnosticsConfig;
  EvaluationConfig evaluationConfig;
  int iterations;
  int batchSize;
  std::string writeModelTo;
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
