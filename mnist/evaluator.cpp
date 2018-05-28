#include "evaluator.h"

#include <folly/Optional.h>

using namespace std;

std::ostream& operator<<(std::ostream& out, const EvaluationResult& r) {
  out << folly::format("Error rate is {}% ", r.errorRate * 100.0)
      << r.errorRates * 100.0;
  return out;
}

EvaluationResult Evaluator::evaluate(IModel model, ExampleList tests) const {
  unique_ptr<ostream> writeDetails;
  if (writeEvaluationDetailsTo_ != "") {
    writeDetails = make_unique<ofstream>(writeEvaluationDetailsTo_);
  }

  int error = 0;
  vector<double> errors(N_CLASS), total(N_CLASS);
  int index = 0;

  for (const auto& e : tests) {
    auto prediction = model->predict(e);
    ++total[e.label];

    bool isError = prediction.getClass() != e.label;

    if (isError) {
      ++error;
      ++errors[e.label];
    }
    if (writeDetails && (isError || writeAll_)) {
      auto& out = *writeDetails;
      out << index << " " << prediction.prob << " " << prediction.getClass()
          << " " << e.label << endl;
    }

    ++index;
  }

  return EvaluationResult{1.0 * error / tests.size(), errors / total};
}
