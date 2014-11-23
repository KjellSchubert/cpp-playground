#include <iostream>
#include <stdexcept>

using namespace std;

// sample copied from http://en.wikipedia.org/wiki/Variadic_template
// More interesting than a printf might be a Python3-style:
// "hello {} {}".format("world", 7)

void variadic_printf(const char *s)
{
  while (*s) {
    if (*s == '%') {
      if (*(s + 1) == '%') {
	      ++s;
      }
      else {
         throw std::runtime_error("invalid format string: missing arguments");
      }
    }
    std::cout << *s++;
  }
}
 
template<typename T, typename... Args>
void variadic_printf(const char *s, T value, Args... args)
{
  while (*s) {
    if (*s == '%') {
      if (*(s + 1) == '%') {
	      ++s;
      }
      else {
        std::cout << value;
        s += 2;
        variadic_printf(s, args...); // call even when *s == 0 to detect extra arguments
        return;
      }
    }
    std::cout << *s++;
  }    
}

void variadic_template_func() {
  cout << "variadic_template_func enters\n";
  variadic_printf("hello %x %x\n", "world", 7);
}
