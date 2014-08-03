#include <iostream>
#include <system_error>
#include <future>

using namespace std;

namespace std {

template<>
struct is_error_code_enum<errc> : true_type {
};

}

int main()
{
   error_code code(EPIPE, system_category()); 
   cout << boolalpha << (EPIPE == static_cast<int>(errc::broken_pipe)) << endl;
   cout << code.message() << endl;
   
   cout << error_code{EINTR, system_category()}.message() << endl;
   cout << std::is_error_code_enum<errc>::value << endl;
   
   // how does the implementation know how to map this to the error message?
   cout << error_code{errc::resource_unavailable_try_again}.message() << endl;
   // outputs generic
   cout << error_code{errc::resource_unavailable_try_again}.category().name() << endl;
   
   cout << error_code{future_errc::no_state}.message() << endl;
   cout << error_code{future_errc::no_state}.category().name() << endl;
   
   cout << error_code{future_errc::no_state} << endl;
   
   cout << make_error_code(errc::resource_unavailable_try_again).message() << endl;
   cout << make_error_code(future_errc::broken_promise).message() << endl;
   
   return 0;
}
