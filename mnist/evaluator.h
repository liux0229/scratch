#pragma once

#include "common.h"

struct EvaluationResult {
  double errorRate;
  std::vector<double> errorRates;
};

std::ostream& operator<<(std::ostream& out, const EvaluationResult&);

class Evaluator {
 public:
  Evaluator(std::string writeEvaluationDetailsTo, bool writeAll)
      : writeEvaluationDetailsTo_(writeEvaluationDetailsTo),
        writeAll_(writeAll) {}
  EvaluationResult evaluate(IModel model, ExampleList tests) const;

  std::string writeEvaluationDetailsTo_;
  bool writeAll_;
};
