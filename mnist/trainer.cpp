#include <iostream>

#include "trainer.h"

using namespace std;

class ConstModel : public Model {
  Prediction predict(const Example& e) const override {
    Prediction p;
    fill(p.prob.begin(), p.prob.end(), 1.0 / p.prob.size());
    return p;
  }
};

IModel Trainer::train(ExampleList examples, Algorithm algorithm) {
  switch (algorithm) {
    case Algorithm::CONST:
      return make_shared<ConstModel>();
  }
  return nullptr;
}
