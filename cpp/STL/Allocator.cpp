#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <typeinfo>

using namespace std;

template<typename T>
struct B {
  // looks like this is not inherited by design.
  using pointer = T;
};

template<typename T>
struct C : B<T> {
  // it's probably because pointer is a dependent typename under B<T>.
  // thus, we don't inherit it since we don't know what it is (type or non-type)
  typename B<T>::pointer f() { }
};

C<int*> c;

template<typename T>
struct Allocator : allocator<T> {
  using Base = allocator<T>;
  // TODO: why do we need this?
  using typename Base::pointer;
  
  template<typename U>
  struct rebind {
    using other = Allocator<U>;
  };

  // Allocator(int x) { }
  
  pointer allocate(size_t n, allocator<void>::const_pointer hint = 0) {
    cout << "allocate: " << n << " " << typeid(T).name() << endl;
    return Base::allocate(n, hint);
  }
  void deallocate(pointer p, size_t n) {
    cout << "deallocate: " << n << " " << typeid(T).name() << endl;
    Base::deallocate(p, n);
  }
};

int main()
{
#if 0
  vector<int, Allocator<int>> v;

  for (int i = 0; i < 10; ++i) {
    v.push_back(i);
  }
#endif

  set<double, less<double>, Allocator<double>> s;
  for (int i = 0; i < 10; ++i) {
    s.insert(i);
  }

  map<int, int, less<int>, Allocator<int>> m;
  for (int i = 0; i < 10; ++i) {
    m.emplace(i, i);
  }

  cout << "###" << endl;

#if 0
  for (auto& e : v) {
    cout << e << endl;
  }
#endif

  return 0;
}
