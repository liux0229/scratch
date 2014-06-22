#include <type_traits>
#include <iostream>
#include <vector>

using namespace std;

#if 0

struct A {
    void f() { }
};

struct B {
    void f(int x) { }
};

struct C : A, B {
};

#endif

struct B1 {
    void f() { }
    static void f(int) { }
    int i;
};

struct B2 {
    void f(double) { cout << "B2::f" << endl; }
};

struct I1 : B1 { };
struct I2 : B1 { };

struct D : I1, I2, B2 {
    using B1::f;
    using B2::f;
    void g() {
        // f();
        f(0);
        f(0.0);
        // int B1::* mp1 = &D::i; // this should be illegal, contrary to what the standard says
        // int D::* mp2 = &D::i;
    }
};

int main() {
    // C c;
    // c.f();
    // c.f(1);
    D d;
    d.g();
}
