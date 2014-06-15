// TODO: 
// 1. const vs non-const
// 2. perfect forwarding

#include <type_traits>
#include <iostream>

using namespace std;

template<int N, typename... T>
struct selector;

template<int N, typename Head, typename... Tail>
struct selector<N, Head, Tail...> {
    using type = typename selector<N - 1, Tail...>::type;
};

template<typename Head, typename... Tail>
struct selector<0, Head, Tail...> {
    using type = Head;
};

template<int N, typename... T>
using Select = typename selector<N, T...>::type;

template<typename... T>
struct Tuple;

template<typename Head, typename... Tail>
struct Tuple<Head, Tail...> : Tuple<Tail...> {
    using Base = Tuple<Tail...>;
    Tuple(const Head& h, const Tail&... t) : Base(t...), h_(h) { }
    Base& base() { return *this; }
    void output(ostream& out) const {
        out << h_ << ", ";
        Base::output(out);
    }
    
    Head h_;
};

template<typename Head>
struct Tuple<Head> {
    Tuple(const Head& h) : h_(h) { }
    void output(ostream& out) const {
        out << h_;
    }
    
    Head h_;
};

// Note that we could simply encode Ret this way without using decltype
// However I need to think more on what the difference really means.
template<int N, typename Ret>
struct getNth {
    template<typename... T>
    static Ret& get(Tuple<T...>& x) {
        return getNth<N - 1, Ret>::get(x.base());
    }
};

template<typename Ret>
struct getNth<0, Ret> {
    template<typename... T>
    static Ret& get(Tuple<T...>& x) {
        return x.h_;
    }
};

template<int N, typename... T>
Select<N, T...>& get(Tuple<T...>& x) {
    return getNth<N, Select<N, T...>>::get(x);
}

template<typename... T>
ostream& operator<<(ostream& out, const Tuple<T...>& x) {
    out << "{";
    x.output(out);
    out << "}";
    return out;
}

template<typename... T>
Tuple<T...> MakeTuple(T&&... values) {
    return Tuple<T...>(forward<T>(values)...);
}

int main() {
    short xs[2] { 10, 20 };
    Tuple<int, char> x(1, 'c');
    auto y = MakeTuple(1, 'c', 2.0, MakeTuple("Hello, world!", xs));
    cout << x << endl;
    cout << y << endl;
    
    cout << get<0>(x) << endl;
    cout << get<1>(x) << endl;
    get<1>(x) = 'd';
    cout << get<1>(x) << endl;
    
    cout << get<3>(y) << endl;
    // TODO: member function style is more straight-forward
    cout << get<0>(get<3>(y)) << endl;
}
