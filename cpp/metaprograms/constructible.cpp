#include <iostream>
#include <chrono>
#include <type_traits>
#include <string>

using namespace std;
using namespace std::chrono;

struct T {
    T() = default;
    T(const T&) = default;
    T& operator=(const T&) = default;
    T(T&&) = delete;
    T& operator=(T&&) = delete;
};

class S {
    public:
    S() { }
    S(const S&) = delete;
    S& operator=(const S&) = delete;
    S(S&&) = default;
    
    private:
    string s_;
};

int main()
{
    cout << boolalpha;
    cout << is_copy_constructible<S>::value << endl;
    cout << is_copy_assignable<S>::value << endl;
    cout << is_move_constructible<S>::value << endl;
    cout << is_move_assignable<S>::value << endl;
    
    // S s1;
    // S s2(move(s1));
   return 0;
}
