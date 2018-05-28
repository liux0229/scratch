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

TrainingConfig TrainingConfig::read(istream& in) {
  TrainingConfig config;
  expectToken(in, "{");
  while (true) {
    string token;
    in >> token;
    if (token == "}") {
      break;
    }

    expectToken(in, "=");

    if (token == "trainingData") {
      config.trainingData = TrainingDataConfig::read(in);
    } else if (token == "modelArch") {
      config.modelArch = ModelArchitecture::read(in);
    } else if (token == "learningRateStrategy") {
      config.learningRateStrategy = LearningRateStrategy::read(in);
    } else if (token == "regularizerConfig") {
      config.regularizerConfig = RegularizerConfig::read(in);
    } else if (token == "diagnosticsConfig") {
      config.diagnosticsConfig = DiagnosticsConfig::read(in);
    } else if (token == "evaluationConfig") {
      config.evaluationConfig = EvaluationConfig::read(in);
    } else if (token == "iterations") {
      config.iterations = expect<int>(in);
    } else if (token == "batchSize") {
      config.batchSize = expect<int>(in);
    } else if (token == "writeModelTo") {
      config.writeModelTo = readString(in);
    } else {
      SCHECK_MSG(false, format("Unexpected token: {}", token));
    }
  }
  return config;
}

TrainingDataConfig TrainingDataConfig::read(istream& in) {
  TrainingDataConfig ret;

  expectToken(in, "{");
  while (true) {
    string token;
    in >> token;
    if (token == "}") {
      break;
    }

    expectToken(in, "=");

    if (token == "trainInput") {
      ret.trainInput = readString(in);
    } else if (token == "trainLabel") {
      ret.trainLabel = readString(in);
    } else if (token == "testInput") {
      ret.testInput = readString(in);
    } else if (token == "testLabel") {
      ret.testLabel = readString(in);
    }
  }

  return ret;
}

ModelArchitecture ModelArchitecture::read(istream& in) {
  ModelArchitecture ret;
  expectToken(in, "{");

  expectToken(in, "fcLayer");
  expectToken(in, "=");

  ret.fcLayer = FullyConnectedLayer::read(in);

  while (true) {
    string token;
    in >> token;
    if (token == "}") {
      break;
    } else if (token == "readModelFrom") {
      expectToken(in, "=");
      ret.readModelFrom = readString(in);
    } else {
      SCHECK(false, "Unexpected token: " + token);
    }
  }

  return ret;
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
  FullyConnectedLayer ret;
  expectToken(in, "{");

  expectToken(in, "hiddenLayerDims");
  expectToken(in, "=");
  ret.hiddenLayerDims = readDims(in);

  expectToken(in, "}");
  return ret;
}

LearningRateStrategy LearningRateStrategy::read(istream& in) {
  LearningRateStrategy ret;
  expectToken(in, "{");

  expectToken(in, "alpha");
  expectToken(in, "=");

  ret.alpha = expect<double>(in);

  expectToken(in, "}");

  return ret;
}

RegularizerConfig RegularizerConfig::read(std::istream& in) {
  RegularizerConfig ret;
  expectToken(in, "{");

  expectToken(in, "policy");
  expectToken(in, "=");

  auto readLambda = [&ret, &in]() {
    expectToken(in, "lambda");
    expectToken(in, "=");
    ret.lambda = expect<double>(in);
  };

  auto policy = expect<string>(in);
  if (policy == "None") {
    ret.policy = RegularizerConfig::None;
  } else if (policy == "L2") {
    ret.policy = RegularizerConfig::L2;
    readLambda();
  } else {
    SCHECK(false, folly::format("Regularizer policy {} not expected", policy));
  }

  expectToken(in, "}");

  return ret;
}

DiagnosticsConfig DiagnosticsConfig::read(std::istream& in) {
  DiagnosticsConfig ret;
  expectToken(in, "{");

  while (true) {
    string token;
    in >> token;
    if (token == "}") {
      break;
    }

    expectToken(in, "=");

    if (token == "lossIterations") {
      ret.lossIterations = expect<int>(in);
    } else if (token == "testErrorIterations") {
      ret.testErrorIterations = expect<int>(in);
    } else {
      SCHECK_MSG(false, format("Unexpected token: {}", token));
    }
  }

  return ret;
}

EvaluationConfig EvaluationConfig::read(std::istream& in) {
  EvaluationConfig ret;
  expectToken(in, "{");

  while (true) {
    string token;
    in >> token;
    if (token == "}") {
      break;
    }

    expectToken(in, "=");

    if (token == "writeEvaluationDetailsTo") {
      ret.writeEvaluationDetailsTo = readString(in);
    } else if (token == "writeAll") {
      ret.writeAll = expect<int>(in);
    } else {
      SCHECK_MSG(false, format("Unexpected token: {}", token));
    }
  }

  return ret;
}
