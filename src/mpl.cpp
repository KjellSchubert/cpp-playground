// move constructor playground.
// See also:
// http://en.cppreference.com/w/cpp/language/move_constructor
// http://blogs.msdn.com/b/xiangfan/archive/2012/02/03/c-98-gt-c-11-pass-by-value-or-pass-by-reference.aspx
// http://stackoverflow.com/questions/24543330/when-is-const-reference-better-than-pass-by-value-in-c11

#include <iostream>
#include <fstream>
#include <vector>
#include <iterator> // std::back_inserter
#include <functional> // std::plus
#include <algorithm> // std::transform
#include <cassert> // assert
//#include <initializer_list> // std::initializer_list
#include <string>
#include <iomanip>
#include <set>
#include <map>
#include <unordered_set>
#include <list>
#include <codecvt> // not in gcc 4.9 yet, requires clang libc++

using namespace std;

// This is atm not a playground for boost::mpl, at lot of whose features
// got outdated by C++11 I suspect. It's a playground for template meta 
// programming concepts though.


// first a simplest example for template meta programming, getting the
// compiler to do some compile-time calculations:
template <int a, int b>
struct my_multiply {
  static const int value = a * b;
};

template <class T>
constexpr T multiply(T a, T b) { return a * b; };
  // or replace auto with int

void mpl_multiply() {
  static_assert(my_multiply<4, 6>::value == 24, "");

  // I was surprised this here worked...
  {
    const int a = 4;
    static_assert(my_multiply<a, 2>::value == 8, "");
  }
  
  //... because of that const_cast possibility here:
  {
    const int a = 4;
    //*const_cast<int*>(&a) = 6;
    //assert(a == 6); // surprise: since when does this fail? TODO
    static_assert(my_multiply<a, 2>::value == 8, "");
    const int product = my_multiply<a, 2>::value;
    assert(product == 8);
    // behavior sort makes sense: so the compiler sees the original value
    // for a, it in general cannot sort out the result of the post-const_cast
    // modification (which in general could as well be a an unpredictable
    //   a := get_time()).
  }

  // This kind of template meta prog is mostly obsolete with C++14 and
  // constexpr, which achieves the same goals with a more straightforward
  // syntax:
  static_assert(multiply(2, 4) == 8, "");
}


// One of the TMP pre-C++11 examples is factorial, since it's recursive:
// Note that the TMP variant is interesting in that it looks like its 
// straight from a purely functional programming language, except with
// a funky & not very intuitive syntax:
template <int n>
struct factorial {
  static const int value = n * factorial<n-1>::value;
};
template<>
struct factorial<0> {
  static const int value = 1;
};

// Can we write this via constexp? Yes, and its much more straightforward
// and shorter that way. 
constexpr int factorial2(int n) {
  return n == 0 ? 1 : n * factorial2(n - 1); 
}

void mpl_factorial() {
  static_assert(factorial2(4) == 24, "");
  static_assert(factorial<4>::value == 24, "");

  // now check how clang deals with potential endless loop bombs:
  //static_assert(factorial2(-1) == 0, ""); // constexpr evaluation exceeded maximum depth of 512 calls
  //static_assert(factorial<-1>::value == 0, ""); // error: recursive template instantiation 
                                                // exceeded maximum depth of 256
  // good!
}


// type traits in C++11 are plentiful, but sometimes you need custom one, e.g. does
// type T have a T* clone() method or not. You could have these traits in C++98 alrdy
// (typically based on SFINAE), I'm wondering if C++11 makes writing these easier 
// in any way.
// See also http://www.gotw.ca/gotw/071.htm
// See also runtime/dynamic polymorphics (via inheritance & vtables) vs
// compile-time/static polymorphism (via TMP).
// See http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern.
// See https://akrzemi1.wordpress.com/2012/03/19/meta-functions-in-c11/
// And http://blog.biicode.com/template-metaprogramming-cpp-ii/

// A first convenient concept of TMP is boost::mpl::if_<cond, T1, T2>::type, 
// see http://www.boost.org/doc/libs/1_57_0/libs/mpl/doc/refmanual/if.html
// aka C++11 std::conditional http://en.cppreference.com/w/cpp/types/conditional
// Here's an impl of if_ aka conditional if we'd bother to impl it ourselves:
template <bool cond, class T1, class T2>
struct if_ { };
template <class T1, class T2> // partial specialization
struct if_<true, T1, T2> { typedef T1 type; };
template <class T1, class T2>
struct if_<false, T1, T2> { typedef T2 type; };

void mpl_conditional() {
  static_assert(std::is_same<if_<true , int, char>::type, int >::value, "");
  static_assert(std::is_same<if_<false, int, char>::type, char>::value, "");
  // now the same with std::conditional:
  static_assert(std::is_same<std::conditional<true , int, char>::type, int >::value, "");
  static_assert(std::is_same<std::conditional<false, int, char>::type, char>::value, "");
}


struct MyClonable {
  virtual MyClonable* Clone() =0;
  virtual ~MyClonable() {}
};

// imagine you want this template to use one of two different impls,
// depending type traits of T, e.g. select one template impl if the passed
// type has a method clone with a certain signature, otherwise pick
// a fallback type. This allows for some sort of duck-typing and compile-time
// polymorphism in C++ via TMP. Probably not good design in general, just
// testing this here for fun.
// Let's first write the type trait that answers the question if T has a 
// member T* Clone(), without using std::is_base_of<MyClonable, T>::value>.
// See http://stackoverflow.com/questions/2122319/c-type-traits-to-check-if-class-has-operator-member
// and http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
// This code was copied from the latter, using C++11 decltype for a shorter
// impl than the C++98 variant. So this type trait allows for duck-typing.
// In any case we wouldn't need the macro if we'd wanna impl this for a
// single member only:
template < class T >
class HasMember_Clone {
  using Yes = char[2];
  using  No = char[1];
  static_assert(sizeof(Yes) != sizeof(No), ""); // guaranteed by language spec

  struct Fallback { int member; };
  struct Derived : T, Fallback { };

  template < class U > 
  static No& test ( decltype(U::Clone)* );
  template < typename U >  
  static Yes& test ( U* );   
  
public:
  static constexpr bool value = sizeof(test<Derived>(nullptr)) == sizeof(Yes);
};


template <class T>
struct MyFooImplWithClone {
  int f() { 
    auto cloneMethodPtr = &T::Clone;
    return 1; 
  }
};
template <class T>
struct MyFooImplNoClone {
  int f() { return 0; }
};

template <class T>
class MyFoo {
  public:
    typedef typename std::conditional<
      // either
      //   a) std::is_base_of<MyClonable, T>::value, or
      //   b) HasMember_Clone<T>::value
      std::is_base_of<MyClonable, T>::value, // TODO fails atm: HasMember_Clone<T>::value,
      MyFooImplWithClone<T>,
      MyFooImplNoClone<T>>::type 
      type;
};

template <class T> // template type alias via 'using'
using MyFoo2 = typename MyFoo<T>::type;

void play_with_custom_type_traits() {
  struct S0 : public MyClonable {
    S0* Clone() override { return new S0(); }
  };
  struct S1 {
    S1* Clone() { return new S1(); }
  };
  struct S2 {};
  static_assert(std::is_same<MyFoo<S0>::type, MyFooImplWithClone<S0>>::value, "");
  static_assert(std::is_same<MyFoo<S2>::type, MyFooImplNoClone  <S2>>::value, "");
  assert(MyFoo2<S0>().f() == 1);
  assert(MyFoo2<S1>().f() == 0); // does not derive from MyClonable
  assert(MyFoo2<S2>().f() == 0);
}

// What is a comfortable way to do template meta programming in C++11?
// E.g. see https://akrzemi1.wordpress.com/2012/03/19/meta-functions-in-c11/
// Using the same selector idiom you can plug in other custom type_traits
#if 0
template <class T>
class HasCloneMethod {
  using yes_t = char[1]; // could also typedef instead of 'using' type alias
  using no_t = char[2];
  static_assert(sizeof(yes_t) != sizeof(no_t), ""); // C++ guarantees that

  /*
  template <typename Func, class U>
  class IsCloneMethod
  {
  };
  template <class U>
  class IsCloneMethod<U* (U::*)(), U> {
    typedef void* type; // actual type irrelevant, existence of type matters
  };
  */
  
  static yes_t& testFunc(typename T::clone);
  static no_t& testFunc(...); // catch all
 
  public:

  static bool value = sizeof(testFunc()) == sizeof(yes_t);
};
#endif

void play_with_mpl() {
  mpl_multiply();
  mpl_factorial();
  mpl_conditional();
  play_with_custom_type_traits();
}
