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
  // feenableexcept(FE_INVALID | FE_OVERFLOW);
}

int main() {
  setup();
  ifstream configFile("training.config");
  auto trainingConfig = TrainingConfig::read(configFile);
  auto& data = trainingConfig.trainingData;

  ExampleReader trainReader{data.trainInput, data.trainLabel};
  auto trainSample = trainReader.readAll();

  ExampleReader testReader{data.testInput, data.testLabel};
  auto testSample = testReader.readAll();

  Evaluator evaluator;
  auto model = Trainer::train(
      trainSample, trainingConfig, [&evaluator, &testSample](IModel model) {
        return evaluator.evaluate(model, testSample);
      });

  double error = evaluator.evaluate(model, testSample);
  // double error = evaluator.evaluate(model, trainSample);

  cout << format("Error rate is {}%", 100 * error) << endl;
}
