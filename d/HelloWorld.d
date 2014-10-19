import std.stdio;

void main() {
  foreach (x; 10 .. 20) {
    writeln(x);
  }
  writeln("Hello, world!");
  int[] a = new int[5];
  foreach (i, ref x; a) {
    x = cast(int)(i * 10);
  }
  auto b = a.dup;
  b[3] = 1234;
  writeln("a = ", a);
  writeln("b = ", b);
}

