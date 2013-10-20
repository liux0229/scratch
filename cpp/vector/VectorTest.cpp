#include "Vector.h"
#include <iostream>
#include <cassert>

using namespace std;
using namespace library;

#define CHECK_EQ(a, b) \
  do { \
    if ((a) != (b)) { \
      cerr << "CHECK_EQ: " << #a << "<" << (a) << "> != " \
           << #b << "<" << (b) << ">" << endl; \
      assert(false); \
    } \
  } while (false)

template<typename T>
void check(const Vector<T>& v, const T& val, size_t size, size_t capacity) {
  CHECK_EQ(size, v.size());
  CHECK_EQ(capacity, v.capacity());
  for (const T& x : v) {
    CHECK_EQ(val, x);
  }
}

template<typename T>
void check(const Vector<T>& v, const T& val, size_t size) {
  check(v, val, size, size);
}

struct Element {
  Element() { }
  Element(int xx) : x(xx) { }
  int x { 0 };
};

bool operator==(const Element& a, const Element& b) { 
  return a.x == b.x; 
}
bool operator!=(const Element& a, const Element& b) { 
  return !(a == b);
}

ostream& operator<<(ostream& out, const Element& e) {
  out << e.x;
  return out;
}

void TestVector() {
  using VE = Vector<Element>;
  {
    VE v;
    check(v, Element{0}, 0);
  }
  {
    VE v(10);
    check(v, Element{0}, 10);
  }
  {
    Element val1 {1001};
    int n1 = 10;
    VE v1(n1, val1);
    check(v1, val1, n1);

    VE v2{v1};
    check(v2, val1, v1.size());

    Element val2 {501};
    int n2 = 20;
    VE v3(n2, val2);
    v2 = v3;
    check(v2, *v3.begin(), v3.size());

    VE v4{move(v3)};
    check(v4, val2, n2);

    v4 = move(v1);
    check(v4, val1, n1);
  }
  {
    VE v(10, Element{1});
    v.resize(20, Element{2});
    CHECK_EQ(20, v.size());
    CHECK_EQ(20, v.capacity());
    for (size_t i = 0; i < 10; ++i) {
      CHECK_EQ(Element{1}, v[i]);
    }
    for (size_t i = 10; i < 20; ++i) {
      CHECK_EQ(Element{2}, v[i]);
    }
    v.resize(15);
    CHECK_EQ(15, v.size());
    CHECK_EQ(20, v.capacity());
    for (size_t i = 0; i < 10; ++i) {
      CHECK_EQ(Element{1}, v[i]);
    }
    for (size_t i = 10; i < 15; ++i) {
      CHECK_EQ(Element{2}, v[i]);
    }
  }
  {
    VE v(10, Element{1});
    v.reserve(20);
    check(v, Element{1}, 10, 20);
  }
  {
    VE v;
    for (int i = 0; i < 1000; ++i) {
      v.push_back(Element{1001});
    }
    check(v, Element{1001}, 1000, 1024);
  }
}

int main() {
  Element e;
  Element e2{move(e)};
  TestVector();
  cout << "All tests passed" << endl;
}
