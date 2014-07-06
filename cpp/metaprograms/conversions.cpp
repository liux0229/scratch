#include <iostream>
#include <limits>

using namespace std;

template<typename T>
T f() { return numeric_limits<T>::max(); }

template<typename T>
int g() {
    return numeric_limits<T>::max();
}

struct A {
    // template<typename T>
    // operator T() { return f<T>(); }
    
    template<typename T>
    operator double() { return g<T>(); }
};

int main()
{
    // cout << f<int>() << endl;
    // cout << f<double>() << endl;
    
    // cout << g<int>() << endl;
    // cout << g<double>() << endl;
    
    A a;
    // cout << a.operator int() << endl;
    // cout << static_cast<double>(a) << endl;
    
    // cout << a.operator double<int>() << endl; // <-- syntax error
}
