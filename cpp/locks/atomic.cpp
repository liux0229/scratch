#include <iostream>
#include <chrono>
#include <type_traits>
#include <string>
#include <atomic>
#include <memory>

using namespace std;
using namespace std::chrono;

int main()
{
   atomic<int> a;
   cout << boolalpha;
   cout << a.is_lock_free() << endl;
   cout << atomic_is_lock_free(&a) << endl;
   
   int64_t* x = new int64_t;
   auto y = new ((char *)x + 1) atomic<int>();
   cout << y->is_lock_free() << endl;
   cout << atomic_is_lock_free(y) << endl;
   
   *y = 1000;
   cout << *y << endl;
   
   cout << hex << x << endl;
   cout << hex << (void *)((char *)x + 1) << endl;
   
   cout << "end" << endl;
   
   // std::align not available under g++ 4.8
   void* z = (char *)x + 1;
   size_t space = 8;
   void* ret = align(4, 4, z, space);
   
   cout << hex << ret  << " " << z << " " << space << endl;
   
   return 0;
}
