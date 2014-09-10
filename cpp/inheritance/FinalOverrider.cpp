#include <iostream>

using namespace std;

struct O  {
  virtual void f() = 0;
};

struct A : virtual O {
  // void f() override = 0;
  // void f() override { }
};


struct AA : A {
  
};

struct B : virtual O {

};

struct BB : B {
  void f() override {
      cout << "BB::f()" << endl;
  };
};

struct C : AA, BB {
    
};

int main()
{
   C c;
   O* o = &c;
   o->f();

   return 0;
}
