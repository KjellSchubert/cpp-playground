#include <array>
#include <iostream>
#include <tuple>
#include <cassert>
#include <string>
#include <memory> // unique_ptr
#include <type_traits>
#include <typeinfo> // for typeid() results (RTTI)
#include <utility> // std::declval
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <tuple>

using namespace std;

namespace {

float foo(const string& s) {
  return s.length();
}

class Foo {
public:
  float foomember(const string& s) {
    return foo(s);
  }

  void voidfunc(int) { }
};

void global_voidfunc(int) {
}

}

void play_with_cpp11() {
  
  // Non-static data member initializers n2756
  {
    class Bar {
      public:
        Bar() {
          cout << "Bar ctor member init\n";
          assert(x == 7);
          assert(xs.size() == 2);
        }
      private:
        int x = 7;
        vector<int> xs {8, 9};
    };
    Bar bar;
  }

  // initializer lists:
  {
    vector<int> xs {8, 9};
    assert(xs.size() == 2);

    // http://stackoverflow.com/questions/3250123/what-would-a-stdmap-extended-initializer-list-look-like
    unordered_map<int, string> i2s {
      make_pair(2, "two"),
      make_pair(3, "three")
    };
    // or even shorter:
    i2s = {
      {2, "two"},
      {4, "four"}
    };
    assert(i2s[2] == "two");

    // and using initializer_list for your own types:
    class Baz {
      public:
        Baz(const std::initializer_list<int>& elems) {
          assert(elems.size() == 2);
          sum = std::accumulate(elems.begin(), elems.end(), 0, std::plus<int>());
        }
        int sum;
    };
    Baz baz {6, 7};
    Baz baz2 = {8, 9};
    assert(baz2.sum == 17);
    // also see http://en.cppreference.com/w/cpp/utility/initializer_list for
    // using initializer_list outside of c'tors
  }

  // array<T>
  {
    cout << "play_with_cpp11 array\n";
    std::array<int,3> myarray = {10, 20, 30};
    assert(myarray[1] == 20);
  }

  // type_traits
  {
    // these traits are more likely to be used in templates I guess, but whatver,
    // here some tests:
    
    // first runtime assert (which could & should be static_assert)
    assert( std::is_void<void>::value );
    static_assert( std::is_void<void>::value, "");

    //typedef std::result_of<Foo::voidfunc(Foo*, int)>::type voidfunc_retval;
    static_assert(!std::is_same<int, float>::value, "");
    static_assert(std::is_same<void, decltype(global_voidfunc(0))>::value, "");
    static_assert(std::is_same<void, decltype(Foo().voidfunc(0))>::value, "");

    // this Foo() might not compile when using in a template where T=Foo
    // doesn't have the expected c'tor signature. In this case used std::declval:
    static_assert(std::is_same<void, decltype(std::declval<Foo>().voidfunc(0))>::value, "");

    static_assert(std::is_nothrow_move_constructible<Foo>::value
                  && std::is_nothrow_move_assignable<Foo>::value,
                  "");

    static_assert(noexcept(global_voidfunc), "");
    static_assert(noexcept(&Foo::voidfunc), "");

    static_assert(std::is_pointer<int*>::value, "");
    static_assert(!std::is_pointer<int>::value, "");
    static_assert(!std::is_pointer<unique_ptr<int>>::value, ""); // it's not a raw ptr
  }

  // decl_type
  {
    // see http://en.cppreference.com/w/cpp/language/decltype
    // see also std::declval and std::result_of
    cout << "decltype\n";

    decltype(foo("hi")) i_am_float = 0.5f;
    static_assert(1+2 == 3, "whatever");

    // doesn't work:
    //   static_assert(decltype(foo("hi")) == decltype(static_cast<int>(0)));
    // but this does:
    static_assert(std::is_same<
        decltype(foo("hi")), 
        decltype(static_cast<float>(0)) // silly ong-hand for just 'float'
      >::value, "");
    
    // compare with the older RTTI:
    int myint = 123;
    cout << "typeid(std::cout << myint).name() = " 
         << typeid(std::cout << myint).name() << endl; // Clang: 'So'
    cout << "typeid(3+0.4f).name() = " 
         << typeid(3+0.4f).name() << endl; // Clang: 'f' for float
    cout << "typeid(Foo()).name() = "
         << typeid(Foo()).name() << endl; // Clang: 'FN12_GLOBAL__N_13FooEvE'
  }

  // result_of
  // I'm unsure why I'd use this when I have decltype, anyway:
  {
    // this won't compile because the type results to void:
    //    std::result_of<decltype(&Foo::voidfunc)(Foo,int)>::type x;
    std::result_of<decltype(&Foo::foomember)(Foo,string)>::type x;
    static_assert(std::is_same<decltype(x), float>::value, "");

    // here the same a bit shorter using decltype:
    decltype(Foo().foomember("")) x2;
    static_assert(std::is_same<decltype(x2), float>::value, "");
  }

  // nullptr & nullptr_t
  {
    nullptr_t mynull = nullptr;
    void* p = nullptr;
    int* p2 = nullptr; // I was surprised this compiled
    // (see http://en.cppreference.com/w/cpp/language/nullptr)
  }

  // std::tuple as a generalization of std::pair. This is great for funcs 
  // returning multiple values
  {
    auto func = [](int val) {
      // std::to_string() is also C++11 btw
      return std::make_tuple(val+1, std::to_string(val), "bob");
    };
    auto resultTuple = func(7);
    assert(std::get<0>(resultTuple) == 8);
    assert(std::get<1>(resultTuple) == "7");

    // now with std::tie things being even more Pythonesque :)
    int result1;
    string result2;
    string result3;
    std::tie(result1, result2, result3) = func(7);
    assert(result1 == 8);

    // the only syntax improvement I could imagine here would be:
    // (auto result1, auto result2, auto result3) = func7(7).
    // Is this possible? Not especially important. In any case
    // the longer the tuple the less this syntax is even readable,
    // and often a retval value class is more readable than a tuple.

    // verify pair<> is compatible with std::get for consistency's sake:
    auto myPair = make_pair(5, "fuenf");
    assert(std::get<0>(myPair) == 5);
  }

  // std::to_string
  {
    assert(std::to_string(true) == "1"); // kinda shitty?
    assert(std::to_string(123) == "123"); // finally a decent itoa

    // while we're at itoa here's atoi aka stoi:
    assert(std::stoi("123") == 123);
    assert(std::stoi("1F", 0, 16) == 31);
    try {
      std::stoi("foo");
      assert(false);
    }
    catch (const invalid_argument ex) {
      cout << "expected ex: " << ex.what() << endl;
    }
  }

  // variadic templates
  // E.g. see http://lbrandy.com/blog/2013/03/variadic_templates/
  // This is the basic for std::tuple, and at Wikipedia they list a printf
  // use case. Anyone see any other use case? The tuple one alone
  // is probably worth the hassle.
  // I'm unsure about the printf use case, doesn't this cause excessive
  // code bloat?
  {
    // TODO
  }
}
