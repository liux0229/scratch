// Example program
#include <iostream>
#include <string>
#include <functional>
#include <algorithm>
#include <memory>

using namespace std;

void f(const function<const int&(const int &,const int&)>& x) {
    cout << x(1, 2) << endl;
}

/*
void f(const int& (*x)(const int&, const int&)) {
    cout << x(1, 2) << endl;
}
*/

template<typename T>
void gg(T t) {
    cout << t(1,2) << endl;
}

/*
int g(int,int) {
    return 0;
}

int g(int) {
    return 0;
}
*/

template<typename T>
T&& myMin(T&& a, T&& b) {
    return a < b ? std::forward<T>(a) : std::forward<T>(b);
}

int main()
{
    // f(&std::min<int>);
    // f(&std::max<int>);
    
    // function<int(int,int)> func = function<int(int,int)>(g);
    
    // f(&g);
    
    unique_ptr<int> pa(new int(5)), pb(new int(6));
    auto ret = myMin(move(pa), move(pb));
    cout << *ret << endl;
}
