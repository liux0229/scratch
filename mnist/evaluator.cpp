#include "evaluator.h"

double Evaluator::evaluate(IModel model, ExampleList tests) const {
  int error = 0;
  for (const auto& e : tests) {
    error += model->predict(e).getClass() != e.label;
  }

  return 1.0 * error / tests.size();
}
