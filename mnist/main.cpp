#include <fenv.h>
#include <folly/Format.h>
// #include <fstream>
#include <iostream>

#include "common.h"
#include "evaluator.h"
#include "trainer.h"

using namespace std;
using namespace folly;

void setup() {
  feenableexcept(FE_INVALID | FE_OVERFLOW);
}

int main() {
  setup();
  ifstream configFile("training.config");
  SCHECK(configFile.good());

  auto trainingConfig = TrainingConfig::read(configFile);
  auto& data = trainingConfig.trainingData;

  ExampleReader trainReader{data.trainInput, data.trainLabel};
  auto trainSample = trainReader.readAll();

  ExampleReader testReader{data.testInput, data.testLabel};
  auto testSample = testReader.readAll();

  Evaluator evaluator(
      trainingConfig.evaluationConfig.writeEvaluationDetailsTo,
      trainingConfig.evaluationConfig.writeAll);
  auto model = Trainer::train(
      trainSample, trainingConfig, [&evaluator, &testSample](IModel model) {
        return evaluator.evaluate(model, testSample).errorRate;
      });

  cout << evaluator.evaluate(model, testSample) << endl;
}
