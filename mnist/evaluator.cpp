#include "evaluator.h"

double Evaluator::evaluate(IModel model, ExampleList tests) const {
  int correct = 0;
  for (const auto& e : tests) {
    correct += model->predict(e).getClass() == e.label;
  }

  return 1.0 * correct / tests.size();
}
