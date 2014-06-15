#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <string>

using namespace std;

template<bool C, typename T, typename F>
using Conditional = typename conditional<C, T, F>::type;

template<typename T>
using MakeUnsigned = typename make_unsigned<T>::type;

// This does not work as expected.
template<template<typename...> class F, typename... Args>
using Delay = F<Args...>;

void f(int) {
  cout << "f(int)" << endl;
}

void f(double) {
  cout << "f(double)" << endl;
}

template<typename T>
using Select = Conditional<is_integral<T>::value, Delay<MakeUnsigned, T>, int>;

int main()
{
   f(Conditional<true, int, double>{}); 
   f(Conditional<false, int, double>{}); 
   
   Conditional<is_integral<int>::value, MakeUnsigned<int>, int> x;
   // Conditional<is_integral<string>::value, MakeUnsigned<string>, int> y;
   Conditional<is_integral<int>::value, Delay<MakeUnsigned, int>, int> z;
   // Conditional<is_integral<string>::value, Delay<MakeUnsigned, string>, int> t;
   
   Select<short> t;
   // Select<string> a;
   
   cout << boolalpha << (typeid(x) == typeid(unsigned int)) << endl;
   cout << boolalpha << (typeid(z) == typeid(unsigned int)) << endl;
   cout << boolalpha << (typeid(t) == typeid(unsigned short)) << endl;
   
   return 0;
}
