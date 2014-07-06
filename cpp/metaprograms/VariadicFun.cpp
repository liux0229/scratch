#include <iostream>

using namespace std;

template<typename... Ts> void func(Ts... args){
    const int size = sizeof...(args) + 2;
    // int res[size] = {1,args...,2};
    // since initializer lists guarantee sequencing, this can be used to
    // call a function on each element of a pack, in order:
    int dummy[sizeof...(Ts)] = { (std::cout << args, 0)... };
    cout << endl;
}

#if 0
template<class T, int N>
ostream& operator<<(ostream& out, const T (&x)[N]) {
    for (int i = 0; i < N; ++i) {
        if (i > 0) {
            out << ",";
        }
        out << x[i];
    }
    out << endl;
    return out;
}
#endif

template<int N1, int N2>
void f(const char (&x)[N1], int (&y)[N2]) {
    cout << N1 << endl;
    cout << N2 << endl;
    cout << x << endl;
    for (int i = 0; i < N2; ++i) {
        cout << y[i] << " ";
    }
    cout << endl;
}

template<typename ...Ts, int... N> void g(Ts (&...arr)[N]) {
    f(arr...);
}

int main()
{
    std::string x{"hello"};
    func(1, 2, "123", 1.5, x);
    
    int n[] { 1, 2, 3, 4, 5, 6 };
    g<const char, int>("Hello World", n);
}
