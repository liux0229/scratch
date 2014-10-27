import std.stdio;

struct S {
}

class C {
}

int isExpression() {
  writeln(is(int == struct));
  writeln(is(S == struct));
  writeln(is(S == class));
  writeln(is(C == class));
  writeln(is(int == return));

  auto f = function int(int) { return -1; };
  // writeln(is(typeid(f) == function));
  writeln(is(isExpression == function));
  
  return 0;
}

void main() {
  isExpression();
}
