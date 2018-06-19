#pragma once

#include "common.h"
#include "operators.h"

struct ModelLayer {
  virtual ~ModelLayer() {}
  virtual IOperator create(IOperator input) const = 0;
  virtual void output(std::ostream& out) const = 0;
};
using IModelLayer = std::shared_ptr<ModelLayer>;

struct FullyConnectedLayer : ModelLayer {
  static FullyConnectedLayer read(std::istream& in);

  IOperator create(IOperator op) const override;

  void output(std::ostream& out) const {
    out << "FC " << hiddenLayerDims;
  }

  Dims hiddenLayerDims;
};

struct CNNLayer : ModelLayer {
  static CNNLayer read(std::istream& in);

  IOperator create(IOperator op) const override;

  void output(std::ostream& out) const {
    out << "CNN " << channel << "," << width;
  }

  int channel;
  int width;
};

struct PoolLayer : ModelLayer {
  static PoolLayer read(std::istream& in);

  IOperator create(IOperator op) const override;

  void output(std::ostream& out) const {
    out << "Pool " << width << "," << stride;
  }

  int width;
  int stride;
};

struct ModelArchitecture {
  static ModelArchitecture read(std::istream& in);

  std::vector<IModelLayer> layers;
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
  bool gradientVerifyDetails = false;
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
  int threads;
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
