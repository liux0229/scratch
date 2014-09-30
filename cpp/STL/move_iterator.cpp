#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

struct S {
    S() {
        cout << "S::S()" << endl;
    }
    S(const S&) {
        cout << "S::S(const S&)" << endl;
    }
    S(S&&) {
        cout << "S::S(S&&)" << endl;
    }
};

int main()
{
   vector<S> v(5);
   vector<S> v2;
   v2.reserve(v.size());
   copy(make_move_iterator(v.begin()), make_move_iterator(v.end()), back_inserter(v2));
   
   return 0;
}
