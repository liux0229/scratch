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
    const LearingRateStrategy& learningRateStrategy) {
  out << "learning rate = " << learningRateStrategy.alpha;
  return out;
}

ostream& operator<<(ostream& out, const TrainingConfig& trainingConfig) {
  out << trainingConfig.modelArch << " " << trainingConfig.learningRateStrategy
      << " "
      << folly::format(
             "iterations={} batch={}",
             trainingConfig.iterations,
             trainingConfig.batchSize);
  return out;
}

namespace {
void expectToken(istream& in, string expected) {
  string token;
  in >> token;
  SCHECK_MSG(
      token == expected, format("Expect {}; received {}", expected, token));
}

template <typename T>
T expect(istream& in) {
  T x;
  in >> x;
  SCHECK(!in.fail());
  return x;
}

template <typename T>
T parse(string s) {
  istringstream in(s);
  T ret;
  in >> ret;
  return ret;
}
} // namespace

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

    if (token == "modelArch") {
      config.modelArch = ModelArchitecture::read(in);
    } else if (token == "learningRateStrategy") {
      config.learningRateStrategy = LearningRateStrategy::read(in);
    } else if (token == "iterations") {
      config.iterations = expect<int>(in);
    } else if (token == "batchSize") {
      config.batchSize = expect<int>(in);
    } else {
      SCHECK_MSG(false, format("Unexpected token: {}", token));
    }

    expectToken(in, ",");
  }
  return config;
}

ModelArchitecture ModelArchitecture::read(istream& in) {
  ModelArchitecture ret;
  expectToken(in, "{");

  expectToken(in, "fcLayer");
  expectToken(in, "=");

  expectToken(in, "}");
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
    expectToken(in, ",");
  }

  return ret;
}
} // namespace

FullyConnectedLayer FullyConnectedLayer::read(istream& in) {
  FullyConnectedLayer ret;

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
