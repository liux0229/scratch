#include <iostream>
#include <functional>
#include <string>

using namespace std;

struct S {
  // static constexpr int f(int x) {
  //    return m_[x];
  // }
  
  static constexpr int m_ []{ 2, 1, 0 };
};

int main()
{
   // cout << S::f(0) << endl;
   
   cout << S::m_[0] << endl;
   
   return 0;
}
