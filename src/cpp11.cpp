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

  void voidfunc(int val) { 
    cout << "Foo::voidfunc " << val << endl;
  }
};

void global_voidfunc(int val) {
  cout << "global_voidfunc " << val << endl;
}

// These using decls work fine at global/namespace scope, but not
// function scope (clang 3.5). Really?
//template<class T> using MyAlloc = std::allocator<T>;
//template<class T> using VecWithAlloc = vector<T, MyAlloc<T>>;

}

namespace PlayWithDelete {

  // from http://stackoverflow.com/questions/12877546/how-do-i-avoid-implicit-casting-on-non-constructing-functions-c 
  void function(int); // this will be selected for int only

  template <class T>
  void function(T) = delete; // C++11 
  // TODO
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

  // uniform initialization via {}
  // This makes C++'s confusing plethora of way to initialize values more uniform,
  // and also solves 'most vexing parse' (which was 'fixable' via confusing 
  // paren before, the initializer fix I like better)
  // Great summary here: http://mbevin.wordpress.com/2012/11/16/uniform-initialization/
  {
    class A {
    public:
      A() {}
      explicit A(int x) {}
      A(int x, string s) {}
    };

    // look at the old ways to init vars:
    int x1=5;
    int x2(5); // direct-initialization
    assert(x2 == 5);
    A a1(5);
    A a2(5, "hi");
    A a3(); // this is a function decl, Clang warns '[-Wvexing-parse]', good
    A a4; // this is an A
    static_assert(std::is_function<decltype(a3)>::value, "");
    static_assert(std::is_same<decltype(a4), A>::value, "");
    // also see 'most vexing parse' issue solved by this.

    // new uniform ways:
    int x100 {5};
    int x101 = {5}; // the fact that '=' is optional makes it less uniform... best omit =
    A a100 {5};
    //A a101 = {5}; // error: chosen constructor is explicit in copy-initialization, good
    A a102 {5, "hi"};
    A a103 = {5, "hi"};
    A a104 {};
    std::vector<int> v {1, 4};
    std::array<int,2> a {1, 4};
    // now here it gets so beautiful it makes my eyes water, this is
    // as concise as Python dict() {} init, or JS {} object/map/dict init:
    std::unordered_map<int,string> m1 { {0,"zero"}, {1,"one"}, {2,"two"} };
    std::unordered_map<int,A> m2 { {0,{5, "hi"}}, {1,a103}, {2,{}} };
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
  // What advantages does this have over C-style arrays (besides obviously 
  // resembling similar containers like std::vector more)?
  //   * polymorphic arrays: cannot as easily pass B[] to func(A[]) anymore
  //   * avoids the imo ugly C++ syntax for C arrays (with 'B bs[5]' instead 'B[5] bs')
  // See http://stackoverflow.com/questions/6111565/now-that-we-have-stdarray-what-uses-are-left-for-c-style-arrays
  // Huge array<T> would live on the stack and could cause stackoverflow?
  {
    cout << "play_with_cpp11 array\n";
    std::array<int,3> myarray = {10, 20, 30};
    assert(myarray[1] == 20);

    // polymorphic array
    class A {};
    class B : public A {};
    {
      B bs[3]; // = {...}
      auto funcTakingAs = [](A as[3]) { /* likely crash here when called with B[] */ };
      funcTakingAs(bs);
    }

    // the same does not compile with array<T>:
    {
      array<B,3> bs; // = {}
      auto funcTakingAs = [](const array<A,3>& as) {};
      auto funcTakingBs = [](const array<B,3>& bs) {};
      //funcTakingAs(bs); // doesn compile, good!
      funcTakingBs(bs);
    }

    // one (minor?) advantage of the C-style array decl syntax was that it would
    // auto-count the size, so you could 'int ints[] = {1,2,3}'. In contrast
    // std::array will auto-init with default value.
    // See also http://stackoverflow.com/questions/12750265/what-is-the-default-value-of-a-char-array-in-c/12750525#12750525
    // especially the char foo[N] = {0}; comment (which indicates C-style arrays
    // will also be init'ed when using this new intializer syntax)
    array<int, 3> ints = {1,2};
    assert(ints[2] == 0);
    //array<int, 3> ints_too_many = {1,2,3,4}; // won't compile, good!
  }

  // http://stackoverflow.com/questions/12877546/how-do-i-avoid-implicit-casting-on-non-constructing-functions-c

  // type_traits
  {
    // Note that std::is_same could be impl'd in the past easily with a handfull
    // of template<T,U> code, see .
    // It's still great that this was standardized now.
    // These traits are more likely to be used in templates I guess, but whatver,
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

  // template alias, from n1489. Or in general type alias.
  // This has two uses:
  //  a) bind some template params, e.g. the Allocator on an container (Vec<T> example)
  //     (only if the T is fixed you could use typedef instead, so not within
  //     a template decl itself)
  //  b) typedef a template alias, e.g. make MyContainer<T> := std::vector<T>
  // See http://stackoverflow.com/questions/10747810/what-is-the-difference-between-typedef-and-using-in-c11
  // and http://en.cppreference.com/w/cpp/language/type_alias
  {
    // template typedef, with all template params bound
    typedef vector<int> IntVec;
    auto elems = IntVec {3,4};
    assert(elems.size() == 2);

    // same typedef but via 'using' template alias
    using IntVec2 = vector<int>;
    auto elems2 = IntVec {3,4};
    assert(elems2.size() == 2);
    static_assert(std::is_same<IntVec, IntVec2>::value, "");
    static_assert(std::is_same<IntVec, vector<int>>::value, "");

    // now binding the allocator via 'using'
    // (from http://en.cppreference.com/w/cpp/language/type_alias)
#if !__has_feature(cxx_alias_templates)
#error "error: !__has_feature(cxx_alias_templates)"
#endif
    // Surprisingly I could not define a template alias at function
    // scope directly. Really?
    // So this doesn't work:
    //   template<class T> using MyAlloc = std::allocator<T>;
    //   template<class T> using VecWithAlloc = vector<T, MyAlloc<T>>;
    // These lines do compile outside a function scope though.
    // This here works also (in class scope):
    class Something {
      public:
        template<class T> using MyAlloc = std::allocator<T>;
        template<class T> using VecWithAlloc = vector<T, MyAlloc<T>>;
    };
    Something::VecWithAlloc<int> v {6,7};
    assert(v.size() == 2);

    // now decl a function (ptr) type via 'using'
    {
      typedef void (*MyFunc1)(int); // ugly syntax for func ptr
      using MyFunc2 = void(&)(int); // less ugly & equivalent? P.S. that a ref, not a ptr
      // odd here: you can assign either the func name or its address (&func)
      MyFunc1 func1 = global_voidfunc;
      MyFunc2 func2 = global_voidfunc;
      // not the same?! static_assert(std::is_same<MyFunc1, MyFunc2>::value, "");
    
      // both behave approx the same, but you cannot reassign a value
      // to func2. Surprising!
      func1 = &global_voidfunc;
      //  func2 = &global_voidfunc; // this won't compile, odd. P.S.: not odd

      // P.S.: they dont behave the same since MyFunc2 is a ref to a function
      // and not a ptr to one. Here's a pointer to one:
      //
      using MyFunc3 = void(*)(int); // less ugly & equivalent?
      MyFunc3 func3 = global_voidfunc;
      func3 = &global_voidfunc;
      static_assert(std::is_same<MyFunc1, MyFunc3>::value, ""); // same after all :)

      // this syntax does not work (it oddly works in the 'using'
      // expression but not in the assignement.
      //using MyFunc4 = void(int);
      //MyFunc4 func4 = global_voidfunc;

      func1(7);
      func2(7);
      func3(7);
    }

    // type alias for raw_ptr, again oddly doesn't compile at function scope
    // but only class or global scope.
    class Stuff {
      public:
      template<class T> using raw_ptr = T*; 
    };
    Stuff::raw_ptr<int> x = static_cast<int*>(nullptr);

    // So for example with pair<a,b> bind one of the template args,
    // same as allocator example. Again only works at global scope.
    class Stuff2 {
      public:
      template <typename T> using NamedValue = std::pair<string, T>;
    };

    auto elem1 = Stuff2::NamedValue<int>("twentyfive", 25);
    assert(elem1.second == 25);
  }
}
