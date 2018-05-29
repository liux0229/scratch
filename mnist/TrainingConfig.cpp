#include "TrainingConfig.h"

#include <folly/Format.h>
#include <sstream>

using namespace std;
using namespace folly;

ostream& operator<<(ostream& out, const ModelArchitecture& modelArch) {
  out << "model arch: "
      << "FC " << modelArch.fcLayer.hiddenLayerDims;
  return out;
}

ostream& operator<<(
    ostream& out,
    const LearningRateStrategy& learningRateStrategy) {
  out << "learning rate = " << learningRateStrategy.alpha;
  return out;
}

ostream& operator<<(ostream& out, const RegularizerConfig& regularizerConfig) {
  if (regularizerConfig.policy == RegularizerConfig::L2) {
    out << folly::format("L2({})", regularizerConfig.lambda);
  }
  return out;
}

ostream& operator<<(ostream& out, const TrainingConfig& trainingConfig) {
  out << trainingConfig.modelArch << " " << trainingConfig.learningRateStrategy
      << " " << trainingConfig.regularizerConfig << " "
      << folly::format(
             "iterations={} batch={}",
             trainingConfig.iterations,
             trainingConfig.batchSize);
  return out;
}

template <typename T>
using Processors = map<string, function<void(T&)>>;

template <typename T>
T parseConfig(istream& in, const Processors<T>& processors) {
  T config;
  expectToken(in, "{");

  while (true) {
    string token;
    in >> token;
    if (token == "}") {
      break;
    }

    expectToken(in, "=");

    auto it = processors.find(token);
    if (it != processors.end()) {
      it->second(config);
    } else {
      SCHECK_MSG(false, format("Unexpected token: {}", token));
    }
  }
  return config;
}

#define OP(block) [&](auto& config) { block }

TrainingConfig TrainingConfig::read(istream& in) {
  Processors<TrainingConfig> processors{
      {"trainingData", OP(config.trainingData = TrainingDataConfig::read(in);)},
      {"modelArch", OP(config.modelArch = ModelArchitecture::read(in);)},
      {"learningRateStrategy",
       OP(config.learningRateStrategy = LearningRateStrategy::read(in);)},
      {"regularizerConfig",
       OP(config.regularizerConfig = RegularizerConfig::read(in);)},
      {"diagnosticsConfig",
       OP(config.diagnosticsConfig = DiagnosticsConfig::read(in);)},
      {"evaluationConfig",
       OP(config.evaluationConfig = EvaluationConfig::read(in);)},
      {"iterations", OP(config.iterations = expect<int>(in);)},
      {"batchSize", OP(config.batchSize = expect<int>(in);)},
      {"writeModelTo", OP(config.writeModelTo = readString(in);)},
  };
  return parseConfig(in, processors);
}

TrainingDataConfig TrainingDataConfig::read(istream& in) {
  Processors<TrainingDataConfig> processors{
      {"trainInput", OP(config.trainInput = readString(in);)},
      {"trainLabel", OP(config.trainLabel = readString(in);)},
      {"testInput", OP(config.testInput = readString(in);)},
      {"testLabel", OP(config.testLabel = readString(in);)},
  };
  return parseConfig(in, processors);
}

ModelArchitecture ModelArchitecture::read(istream& in) {
  Processors<ModelArchitecture> processors{
      {"fcLayer", OP(config.fcLayer = FullyConnectedLayer::read(in);)},
      {"readModelFrom", OP(config.readModelFrom = readString(in);)},
  };
  return parseConfig(in, processors);
}

namespace {
Dims readDims(istream& in) {
  Dims ret;
  expectToken(in, "{");

  while (true) {
    string token;
    in >> token;
    if (token == "}") {
      break;
    }
    ret.push_back(parse<int>(token));
  }

  return ret;
}
} // namespace

FullyConnectedLayer FullyConnectedLayer::read(istream& in) {
  Processors<FullyConnectedLayer> processors{
      {"hiddenLayerDims", OP(config.hiddenLayerDims = readDims(in);)},
  };
  return parseConfig(in, processors);
}

LearningRateStrategy LearningRateStrategy::read(istream& in) {
  Processors<LearningRateStrategy> processors{
      {"alpha", OP(config.alpha = expect<double>(in);)},
  };
  return parseConfig(in, processors);
}

RegularizerConfig RegularizerConfig::read(std::istream& in) {
  Processors<RegularizerConfig> processors{
      {"policy", OP({
         auto policy = expect<string>(in);
         if (policy == "None") {
           config.policy = RegularizerConfig::None;
         } else if (policy == "L2") {
           config.policy = RegularizerConfig::L2;
         } else {
           SCHECK(
               false,
               folly::format("Regularizer policy {} not expected", policy));
         }
       })},
      {"lambda", OP(config.lambda = expect<double>(in);)},
  };

  return parseConfig(in, processors);
}

LearningCurveConfig LearningCurveConfig::read(std::istream& in) {
  Processors<LearningCurveConfig> processors{
      {"writeTo", OP(config.writeTo = readString(in);)},
      {"writeOutEvery", OP(config.writeOutEvery = expect<int>(in);)},
      {"flushEvery", OP(config.flushEvery = expect<int>(in);)},
  };
  return parseConfig(in, processors);
}

DiagnosticsConfig DiagnosticsConfig::read(std::istream& in) {
  Processors<DiagnosticsConfig> processors{
      {"lossIterations", OP(config.lossIterations = expect<int>(in);)},
      {"testErrorIterations",
       OP(config.testErrorIterations = expect<int>(in);)},
      {"learningCurveConfig",
       OP(config.learningCurveConfig = LearningCurveConfig::read(in);)},
  };
  return parseConfig(in, processors);
}

EvaluationConfig EvaluationConfig::read(std::istream& in) {
  Processors<EvaluationConfig> processors{
      {"writeEvaluationDetailsTo",
       OP(config.writeEvaluationDetailsTo = readString(in);)},
      {"writeAll", OP(config.writeAll = expect<int>(in);)},
  };
  return parseConfig(in, processors);
}
