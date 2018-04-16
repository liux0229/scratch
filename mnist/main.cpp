#include <iostream>
#include <folly/Format.h>

#include "common.h"
#include "evaluator.h"
#include "trainer.h"

using namespace std;
using namespace folly;

int main() {
  ExampleReader trainReader{"/data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/data/train-images-idx3-ubyte",
                            "/data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/data/train-labels-idx1-ubyte"};

  auto trainSample = trainReader.readAll();
  // auto model = Trainer::train(trainReader.readAll(), Algorithm::CONST);
  auto model = Trainer::train(trainSample, Algorithm::MLP);

  ExampleReader testReader{"/data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/data/t10k-images-idx3-ubyte",
                           "/data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/data/t10k-labels-idx1-ubyte"};
  Evaluator evaluator;
  double error = evaluator.evaluate(model, testReader.readAll());
  // double error = evaluator.evaluate(model, trainSample);

  cout << format("Error rate is {}%", 100 * error) << endl;
}
