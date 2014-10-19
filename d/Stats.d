import std.string;
import std.algorithm;
import std.exception;
import std.stdio;

interface Stat {
  void add(double x);
  void postProcess();
  double getResult();
}

abstract class IncrementalStat : Stat {
  protected double result_;

  void postProcess() {
  }

  double getResult() {
    return result_;
  }
}

class Min : IncrementalStat {
  this() {
    result_ = double.max;
  }

  void add(double x) {
    result_ = min(result_, x);
  }
}

class Max : IncrementalStat {
  this() {
    result_ = double.min_normal;
  }

  void add(double x) {
    result_ = max(result_, x);
  }
}

class Average : IncrementalStat {
  private int items_ = 0;

  this() {
    result_ = 0.0;
  }

  void add(double x) {
    result_ += x;
    ++items_;
  }

  override void postProcess() {
    result_ /= items_;
  }
}

void main(string[] args) {
  Stat[] stats;
  foreach (arg; args[1 .. $]) {
    auto s = cast(Stat) Object.factory("Stats." ~ arg);
    enforce(s, "Invalid statistics function: " ~ arg);
    stats ~= s;
  }
  
  for (double x; readf(" %f ", &x); ) {
    foreach (s; stats) {
      s.add(x);
    }
  }
  foreach (s; stats) {
    s.postProcess();
    writeln(s.getResult());
  }
}
