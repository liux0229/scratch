import std.array;
import std.stdio;

bool binarySearch(T)(T[] input, T value) {
  while (!input.empty) {
    writeln(input);
    auto m = input.length / 2;
    if (value > input[m]) {
      input = input[0 .. m];
    } else if (value > input[m]) {
      input = input[m + 1 .. $];
    } else {
      return true;
    }
  }
  return false;
}

unittest {
  assert(binarySearch([ 1, 3, 6, 7, 9, 15 ], 6));
  assert(!binarySearch([ 1, 3, 6, 7, 9, 15 ], 8));
  assert(!binarySearch!(int)([ 1, 3, 6, 7, 9, 15 ], 8));
}

void main() {
}
