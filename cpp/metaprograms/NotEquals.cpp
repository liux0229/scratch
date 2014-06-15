#include <iostream>

using namespace std;

struct S {
    bool operator==(const S& b) const { return true; }
    #if 0
    bool operator!=(const S& b) const {
        cout << "S::operator!=" << endl;
        return false;
    }
    #endif
};

bool operator!=(const S& a, const S& b) {
    cout << "free operator!=" << endl;
    return false;
}

template<typename T>
bool operator!=(const T& a, const T& b)
{
    cout << "template operator!=" << endl;
    return !(a == b);
}

int main()
{
    S a, b;
    cout << boolalpha << (a != b) << endl;
}
