#include "Vector.h"
#include <iostream>
#include <cassert>

using namespace std;
using namespace library;

void check(bool c) {
  assert(c);
}

void check(const Vector<int>& v, int val, size_t size, size_t capacity) {
  check(v.size() == size);
  check(v.capacity() == capacity);
  for (int x : v) {
    check(x == val);
  }
}
void check(const Vector<int>& v, int val, size_t size) {
  check(v, val, size, size);
}

void TestVector() {
  {
    Vector<int> v;
    check(v, 0, 0);
  }
  {
    Vector<int> v(10);
    check(v, 0, 10);
  }
  {
    int val = 1001;
    Vector<int> v(10, val);
    check(v, val, 10);

    Vector<int> v2{v};
    check(v2, val, v.size());

    Vector<int> v3(20, 501);
    v2 = v3;
    check(v2, *v3.begin(), v3.size());

    Vector<int> v4{move(v3)};
    check(v4, 501, 20);
  }
}

int main() {
  TestVector();
  cout << "All tests passed" << endl;
}
