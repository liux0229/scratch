#include <iostream>
#include <typeinfo>
#include <type_traits>

using namespace std;

class A {
 public:
  A() = default;
  A(const A& a) {
    cout << "A::A(const A&)" << endl;
  }
  A(A&& a) {
    cout << "A::A(A&&)" << endl;
  }
};

const A& f() {
  static const A a;
  return a;
}

A& g() {
    static A a;
    return a;
}

int main()
{
   A b = std::move(f());
   cout << is_same<decltype(std::move(f())), const A&&>::value << endl;
   cout << is_same<decltype(std::move(f())), A&&>::value << endl;
   
   A c = std::move(g());
   cout << is_same<decltype(std::move(g())), const A&&>::value << endl;
   cout << is_same<decltype(std::move(g())), A&&>::value << endl;
   return 0;
}
