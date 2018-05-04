#include <fenv.h>
#include <folly/Format.h>
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

  ExampleReader trainReader{
      "/data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/data/train-images-idx3-ubyte",
      "/data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/data/train-labels-idx1-ubyte"};
  auto trainSample = trainReader.readAll();

  ExampleReader testReader{
      "/data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/data/t10k-images-idx3-ubyte",
      "/data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/data/t10k-labels-idx1-ubyte"};
  auto testSample = testReader.readAll();

  TrainingConfig trainingConfig{
      TrainingConfig::Algorithm::MLP,
      ModelArchitecture{Dims{800}},
      LearingRateStrategy{LearingRateStrategy::Strategy::CONST, 0.01},
      100000, // iterations
      64, // batch
  };

  Evaluator evaluator;
  auto model = Trainer::train(
      trainSample, trainingConfig, [&evaluator, &testSample](IModel model) {
        return evaluator.evaluate(model, testSample);
      });

  double error = evaluator.evaluate(model, testSample);
  // double error = evaluator.evaluate(model, trainSample);

  cout << format("Error rate is {}%", 100 * error) << endl;
}
