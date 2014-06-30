#include <iostream>
#include <vector>

using namespace std;

struct T {
  T() = default;
  T(T&& other) = default;
  vector<int> V;
};

struct S {
    S() { }
    S(S&&) noexcept = default;
    int a[5];
    T t;
};

int main()
{
   S s;
   cout << "Hello World!!!" << endl; 
   
   return 0;
}
