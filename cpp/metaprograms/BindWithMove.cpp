#include <iostream>
#include <memory>
#include <functional>

using namespace std;

int f(int x, double y, unique_ptr<int> p) {
  return x + y + *p;
}

 unique_ptr<int>&& Move(unique_ptr<int>& p) {
    return std::move(p);
 }

int main()
{
   {
     unique_ptr<int> p(new int{5});
     cout << f(1, 2.0, std::move(p));
   }
   
   {
     unique_ptr<int> p(new int{5});
     auto ff = bind(&f, 1, 2.0, bind(std::move<unique_ptr<int>&>, std::move(p)));
     ff();
   }
   
   return 0;
}
