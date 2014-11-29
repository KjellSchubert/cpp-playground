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
#include <map>
#include <list>
#include <forward_list>
#include <unordered_map>
#include <algorithm>
#include <numeric> // std::accumulate
#include <tuple>
#include <thread>

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
  void function(int) {} // this will be selected for int only

  template <class T>
  void function(T) = delete; // C++11 

  void play() {
    function(123); // this is supposed to compile
    //function('x'); // this is not, clang says "error: call to deleted function 'function'"
  }
}

namespace {
  class CopyCounted {
    public:
      // impl these only to count number of copies
      CopyCounted() {}
      CopyCounted(const CopyCounted& a) {
        ++copy_count;
      }
      CopyCounted& operator=(const CopyCounted& rhs) {
        ++copy_count;
        return *this;
      }

      // make it movable (compile will not auto-generate move ctor and move
      // assignemnt since we have user-defined copy ops)
      CopyCounted(CopyCounted&& rhs) = default;
      CopyCounted& operator=(CopyCounted&& rhs) = default;

      static int copy_count;
  };
  int CopyCounted::copy_count = 0;

  // another variant of CopyCounted, used in tests below
  enum class ExecutedFunc {None,Ctor,CopyCtor,CopyOp,MoveCtor,MoveOp};
  ExecutedFunc executedFunc = ExecutedFunc::None; // ugly global var for tests
  int executedFuncCount = 0;
  void accuExecutedFunc(ExecutedFunc func) {
    executedFunc = func; // so that's really just 'lastExecutedFunc'
    ++executedFuncCount;
  }
  void assertExecutedFunc(ExecutedFunc expectedCall, int expectedCallCount=1) {
    assert(expectedCall != ExecutedFunc::None);
    assert(executedFunc == expectedCall);
    assert(executedFuncCount == expectedCallCount);

    executedFunc = ExecutedFunc::None; // implicit surprise reset
    executedFuncCount = 0;
  };
  void assertExecutedFuncNone() {
    assert(executedFunc == ExecutedFunc::None);
    assert(executedFuncCount == 0);
  }

}

void play_with_member_pointers();

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
        vector<int> xs = {8, 9}; // the '=' is optional
    };
    Bar bar;
  }

  // uniform initialization via {}, Scott Meyers prefers to call it brace initialization.
  // This makes C++'s confusing plethora of ways to initialize values more uniform,
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

    // Also see http://vimeo.com/97318797 34:00 for narrowing conversions:
    A a10(2.5f); // implicit narrowing, kinda ugly
    A a11{static_cast<int>(2.5f)}; // won't compile without cast, good

    // subtle cases where brace initialization & C++98 ctor call syntax choose/prefer
    // different ctors (from http://vimeo.com/97318797 37:00):
    auto v1 = vector<int>(100, 5); // parens, chooses vector(int size, T val) ctor
    auto v2 = vector<int>{100, 5}; // braces, chooses vector(initializer_list) ctor
    assert(v1.size() == 100);
    assert(v2.size() == 2);

    // here's an example where an initializer_list is implicitly converted to
    // the expected func argument (which has a ctor taking initializer_list)
    auto str_sum = [](const vector<string>& elems) { // or std::function<string(const vector<string>&)>
      return std::accumulate(elems.begin(), elems.end(), string(), std::plus<string>());
    };
    string result = str_sum({"hello", " ", "world"});
    assert(result == "hello world");
  }

  // initializer_list:
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

  // auto
  // http://vimeo.com/97318797 @ 3:45 Scott Meyers suggests to prefer auto
  // for readability's sake, avoid unnecessary redundancy. Same reasoning 
  // as C# var. Good example for auto countainer.size() at 7:30.
  // At 9:00 good example for a hidden & surprising copy you get w/o auto.
  // At 15:00 good example for closure inlining guaranteed(?) by auto.
  {
    std::map<string, CopyCounted> m { {"hi", {}}, {"sup", {}} };
    CopyCounted::copy_count = 0;
    for (const std::pair<string, CopyCounted>& pair : m) {
      // do something
    }
    assert(CopyCounted::copy_count==2);

    // now same without unnecessary copies & explicit typing
    // Note the key type is const now.
    CopyCounted::copy_count = 0;
    for (const std::pair<const string, CopyCounted>& pair : m) {
      // do something
    }
    assert(CopyCounted::copy_count==0);

    // now the same with auto, much simpler:
    CopyCounted::copy_count = 0;
    for (const auto& pair : m) {
      // do something
    }
    assert(CopyCounted::copy_count==0);

    // this is http://vimeo.com/97318797 19:30: mix of auto & brace initializer
    // is a bit unintuitive:
    // So careful with mixing auto type deduction & {} uniform initialization
    auto surprise {5};
    static_assert(!std::is_same<decltype(surprise), int>::value, "");
    static_assert(std::is_same<decltype(surprise), initializer_list<int>>::value, "");

    // http://vimeo.com/97318797 22:00: proxy type problems, e.g. with the
    // (deprecated?) vector<bool>
    std::vector<bool> boolvec {true, false};
    bool val = boolvec[1];
    auto val_sort_of = boolvec[1];
    assert(val == false);
    assert(val_sort_of == false);
    static_assert(!std::is_same<decltype(val_sort_of), bool>::value, "");
    // at 27:00 in the video he has an example where auto results in
    // holding on to a dangling ptr (with temporary vector<bool> in play).
    // A minor concern for auto imo.

    // See for slicing concerns: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n1984.pdf
    // Slicing is a valid concern imo only if you're new to 'auto' and overlook
    // 'auto&'. o yes, auto will slice exactly as easily as an explicit type 
    // identified would have.
    class A { public: virtual ~A() {} };
    class B : public A {};
    A* bPtr = new B();
    {
      // slicing is not very surprising here, same as if you'd written 'A bSliced ='
      auto bSliced = *bPtr;
      static_assert(std::is_same<decltype(bSliced), A>::value, "");
      assert(dynamic_cast<B*>(&bSliced) == nullptr);
    }
    {
      auto& bUnsliced = *bPtr;
      static_assert(std::is_same<decltype(bUnsliced), A&>::value, "");
      assert(dynamic_cast<B*>(&bUnsliced) != nullptr);
    }

    // some notes on auto&:
    auto int_func = []() -> int { return 5; };
    // these don't compile, good:
    //   int& myInt1 = int_func();
    //   auto& myInt2 = int_func();    
    auto myInt3 = int_func();
    assert(myInt3 == 5);

    // note that there's even auto*, which only works if decltype(expr) is a 
    // pointer type. I hardly ever saw that used though:
    auto ptr_func = [&myInt3]() { return &myInt3; };
    auto* myPtr1 = ptr_func();
    auto  myPtr2 = ptr_func();
    static_assert(std::is_same<decltype(myPtr1), decltype(myPtr2)>::value, "");
    // won't compile, good:
    //   auto* myInt4 = &int_func(); // cannot take the address of an rvalue of type 'int'

    // just for completeness' sake see multi-declarator auto in
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2004/n1737.pdf
    // This is interesting mostly for for-loops, which in fact is now
    // less interesting due to the new for(elem:container)-loops.
    vector<int> foo {1,2,4};
    decltype(foo)::value_type sum = 0;
    for (auto iter = foo.begin(), end = foo.end(); iter != end; ++iter)
      sum += *iter;
    assert(sum == 7);
  }

  // lambda aka closure:
  // These make functors (class with operator()) a lot less prominent in C++,
  // awesome! Since functors are kinda verbose. Besides plenty of other languages
  // have lambdas, lack of these in C++ was a turn-off.
  //
  // Initially I found the syntax for lambdas off-putting & ugly, but I luckily 
  // quickly got used to it. No matter how funky the syntax: lambda is a vastly
  // important language feature to curb code verbosity, no matter the language:
  // it's critical for:
  //   * Javascript: var fun = function(arg) { return arg+1; };
  //   * Python: fun = lambda x: x+1
  //       (note Python only allows expressions in lambdas, no statements, and
  //       note that generator&list expressions in Python took the edge off the
  //       lack of lambda for a long time)
  //   * Java: who cares
  //   * C#: var fun = (int arg) => { return arg+1; }
  //   * C++: auto fun = [](int arg) { return arg+1; };
  // So the C++ syntax is actually very similar to JS, with '[]' instead of
  // 'function'. This syntax is fine with me. Where it gets ugly imo is binding
  // instructions within the [], which need to accomodate C++'s value vs ref
  // semantics.
  {
    // First a motivational example:
    // Imagine you wanted to square each element of a vector and add a variable x.
    // First via pre-C++11 functor:
    vector<int> values {2,5,6};

    struct SquareFunctor {
      SquareFunctor(int x) : x_(x) {}
      int operator()(int value) {
        return value * value + x_;
      }
      private: int x_;
    };
    const int x=10;
    vector<int> result;
    std::transform(values.begin(), values.end(), std::back_inserter(result), 
                   SquareFunctor(x));
    assert(result.size() == 3 && result.back() == 6*6 + 10);

    // now the same with lambdas in C++11:
    vector<int> result2;
    std::transform(values.cbegin(), values.cend(), std::back_inserter(result2), 
      [x](int value) {
        return value * value + x;
      });
    assert(result == result2);

    // So lambda got rid of the functor boilerplate, making the C++ code look
    // a lot more like the corresponding Javascript:
    //   var result = _.map(values, function(value) { // _ = lodash.js
    //     return value*value + 10;
    //   });
  }
  {
    // simplest example
    auto f1 = [](int arg) { return arg+1; };
    assert(f1(7) == 8);

    // emulate x++ aka post-increment:
    auto postIncrement = [](int &arg) {
      int retval = arg;
      ++arg;
      return retval;
    };
    int val = 7;
    assert(postIncrement(val) == 7);
    assert(val == 8);

    // example with explicit retval spec. This is rarely necessary, e.g. it is
    // mostly when the automatically deduced retval is T but you want T&.
    // Example: emulate pre-increment aka ++x
    // Note the explicit retval type specifier as int& (which would be deduced
    // as plain int otherwise).
    auto preIncrement = [](int &arg) -> int& {
      ++arg;
      return arg;
    };
    val = 7;
    int& valRef = preIncrement(val);
    assert(valRef == 8);
    assert(val == 8);
  }
  {
    int stuff {7}; // uniform init looks pretty funky here (where's the default from)
    assert(stuff == 7);
    int otherStuff {9};

    // now some examples with closure capture specs:
    // So by default a C++ closure captures no variable in function scope. This
    // here yields with clang:
    // error: variable 'stuff' cannot be implicitly captured in a lambda with no capture-default
    //auto f1 = [](int val) { return val + stuff; };
    //
    // Note that in pretty much all other langs that have lambda the closure 
    // captures the local context automatically & implicitly (Python, JS, C#), so
    // the concept of explicit capturing of local vars seems a bit alien to me.
    // But this was needed because of C++'s value vs ref semantics, which none of 
    // these other languages have to worry about (not explicitly). Actually C# has
    // ref T func params, but wasnt that only applicable for builtins? Thus simpler.
    auto f2 = [stuff, otherStuff](int arg) { // captures these 2 local vars explicitly
      return arg + stuff;
      // otherStuff was tagged as captured, but is really unsed, no problem
    };
    assert(f2(3) == 10);

    // You can also capture individual vars by reference, and can mix & match 
    // capture by ref and by value.
    // One thing I keep confusing: does the '&' belong before or after the 
    // variable name... It's before. Making it look like we're capturing the address of
    // the variable.
    auto f3 = [&stuff, &otherStuff](int arg) {
      int retval = arg + stuff;
      stuff = 77; // only has impact becuase of capture by ref
      return retval;
    };
    assert(f3(3) == 10);
    assert(stuff = 77);

    // WARNING: capturing local vars by reference is a recipe for disaster if
    // the lifetime of the closure extends the lifetime of the block the closure
    // is defined in! This is as dangerous as a returning local variable by
    // reference in regular C++ functions!
    {
      struct A {
        ~A() {
          cout << "A::~A\n";
          wasDestructed = true; 
        }
        bool wasDestructed = false;
      };
      std::function<float(int, int)> func;
      {
        A a;
        func = [&a](int x, int y) -> int {
          if (a.wasDestructed) { // undefined behavior here!
            cout << "WARNING: closure operated on dangling ptr!\n";
            return -1; // just for testing, ridiculous code of course
          }
          return x + y;
        };
      }
      // at this point we had A destructed alrdy.
      assert(func(3,4) == -1); // operated on a destructed obj, behavior is undefined 
                            // of course.
    }

    // Instead of capturing individual vars you can capture everything, either
    // via [=] or [&], which capture all local vars by value or ref respectively.
    // WARNING: this syntax is not recommended, e.g. the Google style code advises
    // to capture individual vars explicitly. I personally don't mind [=] to much
    // as long as doesnt capture pointers but only values that will be copy-constructed
    // (deep cloned!) by the closure. 
    stuff = 7;
    auto f5 = [=](int val) { return val + stuff; };
    assert(f5(3) == 10);

    // Another important aspect of capture is when does the act of capture actually
    // happen. It happens when the closure is declared, means if you captured by
    // value and change the variable later on then the closer will keep seeing the
    // old captured value:
    stuff = 7;
    auto f6 = [stuff](int x) { return stuff + x; };
    stuff = 100;
    assert(f6(0) == 7); // not 100!
  }
  {
    // C++14 changes/enhancements to lambda, Generalized Lambda-capture.
    // See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3648.html
    // The only use case of these change imo should be to allow for capture
    // expressions like [my_unique_ptr = std::move(my_unique_ptr)].
    {
      // example: hand off a unique_ptr to a worker thread:
      struct A { int x = 7; };
      unique_ptr<A> ptrToA {new A()};

      // this here won't work because unique_ptr cannot be copied:
      //auto f1 = [ptrToA]() { assert(ptrToA->x == 7); };
      
      // this here will compile but will crash due to a dangling ref in the 
      // worker thread (well, the following join() prevents the dangling
      // ref, but in general you wouldn't want to join() in this scenario
      // since it defeats the purpose of the thread).
      // WARNING: remember capture by ref is a red flag when it comes to
      // threading! (be it actual threads or pushing a closure to an
      // executor / event-loop object).
      //   auto f1 = [&ptrToA]() { assert(ptrToA->x == 7); };
      //   thread worker(f1);
      /* TODO: this doesnt compile with clang 3.5 using c++ stdlib of gcc 4.9.2)
      auto f1 = [movedPtrToA = std::move(ptrToA)]() { assert(movedPtrToA->x == 7); };
      thread worker(f1);
      worker.join();
      */

      // whats a workaround for lack of std::move in lambda captures in C++11?
      // E.g. pass shared_ptr or boost::intrusive_ptr to the thread's closure.

      // I cannot think of other use cases for generalized lambda capture 
      // expressions, but refrain from using them unnecessarily, e.g. this
      // here is ugly imo:
      int x = 3;
      auto f2 = [x = x+7]() { return x+100; };
      assert(f2() == 110);
      assert(x == 3);
    }
  }
  {
    // the capture of 'this' is somewhat special but intuitive imo, see
    // http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2009/n2927.pdf 8:
    struct A {
      void f() {
        auto f1 = []() {
          //x = 5; // error: 'this' cannot be implicitly captured in this context
          //this->x = 6; // same error
        };

        auto f2 = [=]() {
          x = 5;  // conflicts with n2927 ?
          this->x = 6;
        };
        f2();
        assert(x = 6);

        auto f3 = [&]() {
          x = 7;
          this->x = 8;
        };
        f3();
        assert(x == 8);

        auto f4 = [this]() { // explicit capture of this by value
          x = 9;
        };
        f4();
        assert(x == 9);
      }
      int x = 0;
    };
    A a;
    a.f();
  }

  // rvalue refs, move semantics
  // We alrdy had copy-elision & RVO before C++11 in certain cases for many 
  // compilers, still there was plenty of room for improvement with respect
  // to getting rid of unwanted copy ctor calls.
  // http://google-styleguide.googlecode.com/svn/trunk/cppguide.html suggests
  // to limit rvalue refs to move ctors & move assignments (which supplement
  // C++98 copy ctor & copy op).
  // Good intro http://www.aristeia.com/TalkNotes/ACCU2011_MoveSemantics.pdf
  // Details http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2010/n3053.html
  {
    struct C {
      C() { accuExecutedFunc(ExecutedFunc::Ctor); }

      // copy
      C(const C& rhs) { accuExecutedFunc(ExecutedFunc::CopyCtor); }
      C& operator=(const C& rhs) { 
        accuExecutedFunc(ExecutedFunc::CopyOp); 
        return *this; // I was surprised this is a warning only when I forget 
               // the return, clang said: 'control reaches end of non-void function'
               // May wanna make this an err
      }

      // move ctor and move assignment.
      // These would have been added by the compiler implicitly if I hadnt
      // added user-defined copy ops above, for detailed rules see
      // http://en.cppreference.com/w/cpp/language/move_operator
      // So these have exactly the same signature as copy ctor and copy op,
      // except they take (T&& rhs) instead of (const T&) args.
      C(C&& rhs) = default;
      C& operator=(C&& rhs) = default;

      vector<int> data = {7,8};
    };
    C c1;
    assertExecutedFunc(ExecutedFunc::Ctor);

    // here the regular copy ctor is called:
    C c2(c1);
    assertExecutedFunc(ExecutedFunc::CopyCtor);

    // still regular copy ctor via uniform init:
    C c3{c1}; // 100% the same as 'C c3 = {}' btw
    assertExecutedFunc(ExecutedFunc::CopyCtor);

    C c4 = c1; // 100% the same as C c4(c1), even though it sort of looks like assignment
    assertExecutedFunc(ExecutedFunc::CopyCtor);

    // lets write a func that has a move ctor kick in, making
    // sure that pre-C++11 copy elision couldnt have kicked in
    // (which kicked in for 'return MyReturnType(ctorArgs)' mostly)
    auto combineCs = [](const C& c1, const C& c2) -> C {
      C result; // naming it should prevent pre-C++11 RVO
      // concatenating vector<T>s is not as simple as one would hope btw:
      result.data = c1.data;
      result.data.insert(result.data.end(), c2.data.begin(), c2.data.end());
      return result;
    };
    C c5;
    assertExecutedFunc(ExecutedFunc::Ctor);
    c5 = combineCs(c1, c2);
    assertExecutedFunc(ExecutedFunc::Ctor); // this is the ctor in combineCs()
    // so note hat no copy op was called, only the compiler-generated
    // move ctor was executed, which in turn moved the vector content
    // efficiently via some swap().
    // To test what happens without move ctors comment out the '= default'
    // lines above.
    assert(c5.data.size() == 4 && c5.data[0] == 7);

    // how to verify the existence of move ctors at compile time: 
    static_assert(std::is_move_constructible<C>::value, "");
    static_assert(is_nothrow_move_constructible<C>::value, "");
    //static_assert(is_trivially_move_constructible<C>::value, ""); // assert fails
   
    // Why is there such a specific is_nothrow_move_constructible?
    // Because failed moves have implications on the 3 David Abrahams 
    // exception-safety guarantees (no-throw, basic, strong). See also:
    // http://www.cplusplus.com/reference/type_traits/is_nothrow_move_constructible/
    // http://stackoverflow.com/questions/14601469/what-if-an-object-passed-into-stdswap-throws-an-exception-during-swapping/14601722#14601722
  }
  {
    // another example but this time with move ctor implicitly defined by 
    // the compiler. To get this ctor we have to refrain from adding
    // user-defined ctor & copy op among other things. Example:
    struct D {
      explicit D(int x) { }  // user-defined ctor won't prevent move
      vector<int> data = {7,8};
      CopyCounted copyCounted; // to measure number of copy op/ctor calls
    };
    static_assert(std::is_move_constructible<D>::value, "");
    //static_assert(is_nothrow_move_constructible<D>::value, ""); // fails when adding CopyCounted
    //static_assert(is_trivially_move_constructible<D>::value, ""); // fails due to vector alrdy
   
    // again a move ctor scenario where RVO cannot kick in:
    auto createD = []() {
      D myD(7);
      myD.data.push_back(9);
      return myD;
    };
    CopyCounted::copy_count = 0;
    D d1 = createD();
    assert(d1.data.back() == 9);
    assert(CopyCounted::copy_count == 0); // because it was move-constructed
       // via default compiler-generated move ctor

    // now verify a regular copy still calls copy-ctor
    D d2(7);
    D d3(8);
    d3 = d2;
    assert(CopyCounted::copy_count == 1);
    CopyCounted::copy_count = 0;

    // now do the same with std::move, which will (potentially) salvage its
    // args and leave it in a sad (destructible & assignable) but otherwise
    // undefined state.
    // Note that all std::move does is turn its arg into an xvalue, which in
    // turn can bind to rvalue refs (for the gory details see
    // http://en.cppreference.com/w/cpp/language/value_category).
    assert(d2.data.size() == 2);
    d3 = std::move(d2);
    assert(d2.data.size() == 0); // technically undefined behavior, s2 was salvaged
    assert(CopyCounted::copy_count == 0);
  }
  // I find the rules for when the compiler generates implicit move/copy
  // assigment ops and ctors somewhat confusing, especially because the 
  // C++14 standard draft seems to differ from clang 3.5 and gcc 4.9 behavior, 
  // unless I misinterpret the standard wording or have an outdated draft.
  // It's probably best to only worry about 3 cases:
  //   a) no user-defined ctors: copy/move assignment and copy/move 
  //      ctors will be generated by compiler if possible (means if base
  //      classes and members have the necessary ops & ctors defined)
  //   b) both copy assignment & ctor are user-defined: move assignment
  //      and ctor will NOT be generated by compiler.
  //      This is a likely legacy C++98 case.
  //   c) all 4 move assignment ops & ctors are user-defined.
  // It's best to have our C++ style guide avoid other cases.
  {
    // case a:
    // By default the compiler will generate copy & move ctor for a class for
    // which all members & base clases are copyable/movable respectively.
    struct S {
      int a;  // is copyable & movable
    };
    static_assert( std::is_copy_constructible<S>::value, "");
    static_assert( std::is_move_constructible<S>::value, "");
    static_assert( std::is_copy_assignable<S>::value, "");
    static_assert( std::is_move_assignable<S>::value, "");
  }
  {
    // case b) with legacy class that has a user-defined copy assingment & ctor
    // This will get a compiler-generated move assignment & ctors if its members
    // allow for these.
    struct S {
      int a;
      S(const S&) {}
      S& operator=(const S&) { return *this; }
    };
    static_assert( std::is_copy_constructible<S>::value, "");
    static_assert( std::is_move_constructible<S>::value, "");
    static_assert( std::is_copy_assignable<S>::value, "");
    static_assert( std::is_move_assignable<S>::value, "");
  }
  // here some cases outside a/b/c:
  {
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2010/n3053.html says
    // a user-defined copy ctor will prevent the compiler from generating
    // a move ctor.
    // This example is taken directly from n3053: yet somehow it doesnt behave
    // as advertised with clang 3.5 and gcc 4.9. Was n3053 amended? Where?
    // Latest C++14 draft is http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3797.pdf
    // and contains the same wording as n3053.
    struct S {
      int a;

      // this line here does not prevent generation of a move assignment op,
      // opposing what n3053 says:
      S& operator=(const S&) = default;
      // Neither does this:
      //   S& operator=( const S& ) { return *this; }
      // So maybe the rules for generating a move ctor is simply that all
      // members & base classes are movable? 
    };
    static_assert( std::is_copy_constructible<S>::value, "");
    static_assert( std::is_move_constructible<S>::value, "");
    static_assert( std::is_copy_assignable<S>::value, "");
    static_assert( std::is_move_assignable<S>::value, ""); // TODO: unexpected!
  }

  {
    // if you define a move ctor then the copy ctor and copy assignment op will 
    // not be generated by the compiler:
    struct S {
      int a;
      S& operator=( S&& ) { return *this; } // move assignment
    };
    static_assert(!std::is_copy_constructible<S>::value, "");
    static_assert(!std::is_move_constructible<S>::value, ""); // move assign op prevents gen move ctor
    static_assert(!std::is_copy_assignable<S>::value, "");
    static_assert( std::is_move_assignable<S>::value, "");
    S s1;
    //S s2 = s1;  // error: call to implicitly-deleted copy constructor of 'S'
  }
  {
    // if you define a move ctor then the copy ctor and copy assignment op will 
    // not be generated by the compiler, but you can enabled these with '= default'
    struct S {
      int a;
      S& operator=( S&& ) { return *this; } // move assignment
      S& operator=( const S&) = default;
    };
    static_assert(!std::is_copy_constructible<S>::value, "");
    static_assert(!std::is_move_constructible<S>::value, ""); // move assign op prevents gen move ctor
    static_assert( std::is_copy_assignable<S>::value, "");
    static_assert( std::is_move_assignable<S>::value, "");
    S s1;
    //S s2 = s1;  // error: call to implicitly-deleted copy constructor of 'S'
  }
  {
    // another example: what happens if some members are move constructable, but
    // others are not: is the compiler still gonna be able to generate a move ctor,
    // calling a mixture of move & copy assignment operators?
    struct A_movable {
      int x = {};
    };
    static_assert( std::is_copy_constructible<A_movable>::value, "");
    static_assert( std::is_move_constructible<A_movable>::value, "");
    static_assert( std::is_copy_assignable<A_movable>::value, "");
    static_assert( std::is_move_assignable<A_movable>::value, "");

    struct A_not_movable {
      A_not_movable() {}

      // This is one way to prevent the compiler generated move ctor:
      A_not_movable(A_not_movable&&) = delete;
      A_not_movable& operator==(A_not_movable&&) = delete;

      // Since you have a user-defined move ctor the class gets no
      // compiler-generator copy assignment & ctor, but you can enable
      // these explicitly. Or can't you?
      // Initially I tried to have a class here that's not movable, but
      // is copyable, and compiler behavior didn't make sense. On the other hand
      // myabe my contrived use case was invalid: since move is an optimization
      // of copy maybe it makes no sense to have a class thats copyable
      // but not movable: after all a move can be impl'd as copy+delete.
      A_not_movable(const A_not_movable&) = delete;
      A_not_movable& operator=(const A_not_movable&) = delete;

      int y = {};
    };
    static_assert(!std::is_copy_constructible<A_not_movable>::value, "");
    static_assert(!std::is_move_constructible<A_not_movable>::value, "");
    static_assert(!std::is_copy_assignable<A_not_movable>::value, "");
    static_assert(!std::is_move_assignable<A_not_movable>::value, "");
    /*
    auto f = []() { 
      A_not_movable a;
      a.y = 7;
      return a; // error: call to deleted (copy) constructor of 'A_not_movable'
    };
    //A_not_movable a1 = f(); // error: call to deleted [move] constructor of 'A_not_movable'
    */
    A_not_movable a2, a3;
    //A_not_movable a4(a2); // error: call to deleted constructor of 'A_not_movable'
    //a3 = a2; //  error: overload resolution selected deleted operator '='

    struct B {
      A_movable a1;
      A_not_movable a2;
    };
    static_assert(!std::is_copy_constructible<B>::value, "");
    static_assert(!std::is_move_constructible<B>::value, ""); // move assign op prevents gen move ctor
    static_assert(!std::is_copy_assignable<B>::value, "");
    static_assert(!std::is_move_assignable<B>::value, "");
    // So this shows that the compiler won't create a move c'tor unless
    // every single member has a move ctor.
    // WARNING: this conflicts with my expectation: I thought the compiler
    // would generate a move ctor even if some members were not movable,
    // generating code to copy the non-movable members.
    // TODO!

    // using std::move on non-movable types is still possible, it still will
    // create an xvalue ref, but this won't be able to call a move ctor then.
    B b1;
    //B b2(std::move(b1));
    // Clang errors here with:
    //   copy constructor is implicitly deleted because 'A_not_movable' has a user-declared move
    //   constructor:
    //     A_not_movable(A_not_movable&& rhs) = delete;
    // So deleting a compiler generated move ctor will also delete the
    // compiler generated copy ctor? I wonder why...
  }
  {
    // Using rvalue refs outside of move ctors and move assignments.
    // http://google-styleguide.googlecode.com/svn/trunk/cppguide.html advises
    // against doing that, since && is relatively new & many devs are not
    // yet comfortable using it.
    // Anyway, testing this here:
    struct A {
      static int f1(int& x) {
        ++x;
        return 1;
      }
      static int f1(int&& x) {
        return 2;
      }
    };

    // choose lvalue (int&) overload of f1()
    int val = 2;
    assert(A::f1(val) == 1); // calls int& overload
    assert(val == 3);

    // choose rvalue (int&&) overload of f1() because
    // the retval of f2() is not an lvalue.
    auto f2 = []() {
      return 7; 
    };
    assert(A::f1(f2()) == 2);

    // choose lvalue (int&) overload of f1() because 
    // thr retval of f3() is an lvalue:
    // Note that surprisingly here I had to add '-> int&' since
    // otherwise the compiler would deduce a retval of 'int'.
    auto f3 = [](int& intRef) -> int& {
      intRef += 10;
      return intRef;
    };
    val = 2;
    assert(A::f1(f3(val)) == 1);
    cout << val;
    assert(val == 13); // original 2, plus 10 from f3, plus 1 from A::f1(int&)
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

  // result_of, which is kinda useless now, ignore.
  // It's borderline useless now, only kept since it was in boost & tr1 pre decltype,
  // see http://stackoverflow.com/questions/2689709/difference-between-stdresult-of-and-decltype
  {
    // this won't compile because the type results to void:
    //    std::result_of<decltype(&Foo::voidfunc)(Foo,int)>::type x;
    // Doesn't compile:
    //    std::result_of<&Foo::foomember>::type x;
    //    See http://en.cppreference.com/w/cpp/types/result_of for how to fix (decltype)
    // But result_of works for functors (which are less relevant now in C++11):
    struct Functor {
      float operator()(const string&) { return 0; }
    };
    std::result_of<Functor(const string& string)>::type x;
    static_assert(std::is_same<decltype(x), float>::value, "");
    static_assert(std::is_same<
      decltype(x), 
      decltype(std::declval<Foo>().foomember(""))>::value,
      "");

    // here the same a bit shorter using decltype:
    decltype(Foo().foomember("")) x2;
    static_assert(std::is_same<decltype(x2), float>::value, "");
  }

  // nullptr & nullptr_t
  // See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2431.pdf
  // Side note: reading n2431 I saw the funkiest C++ concept: pointer to member.
  // It generalizes member func ptr though, makes sense. So uses same .*
  // and ->* syntax as for mem func ptr. 
  // See also http://stackoverflow.com/questions/670734/c-pointer-to-class-data-member
  // The intrusive list example on their is interesting. But look at this
  // synatx in this example:
  //    func(E *E::*next_ptr)
  // This is horrible syntax at first glance. Better might be:
  //    void func(mem_ptr<E, E*> next_ptr) {
  //      E e;
  //      E* next = next_ptr.get(e);
  //    }
  //    
  // This wold be similar to mem_fn, but there is no std::mem_ptr afaik.
  {
    nullptr_t mynull = nullptr;
    void* p = nullptr;
    int* p2 = nullptr; // I was surprised this compiled. P.S. shouldnt have been.
    class X {};
    class Y {};
    X x {};
    Y y {};
    //bool alwaysFalse = &x == &y; // not supposed to compile
    bool alwaysFalse2 = &x == nullptr;
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
  // P.S.: this is also the base for the new std::function, std::mem_fn.
  // P.P.S: probably also the base for the new C++11 emplace_front() funcs 
  // I'm unsure about the printf use case, doesn't this cause excessive
  // code bloat?
  // See variadic_template_func.cpp

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
//#if !__has_feature(cxx_alias_templates)
//#error "error: !__has_feature(cxx_alias_templates)"
//#endif
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

  // std::function vs std::mem_fn (and deprecated std::mem_fun) vs .* and ->*
  // Summary of this long section:
  //   * use s.th. like std::function<void(int,char)> to store ptrs to 
  //     non-member functions.
  //   * use lambda instead of std::bind (lambdas can be stored via std::function)
  //   * in the rare case you need to store a ptr to a member function A::fun
  //     assign to std::function<...(A&,...)>, or more easily just use
  //     'auto f = mem_fn(&A::fun)'. This way you never have to use the odd 
  //     looking .* and ->* operators to call mem funcs. 
  //   * dont ever use std::bind, std::mem_fun (use std::mem_fn instead)
  //   * dont use ugly typedef syntax to decl func ptr types. Instead use C++11
  //     'using FuncPtr_t = void (*)(int,char)', or std::function<void(int,char)>
  {
    // First here's the old C++98 func ptr syntax:
    // Store a ptr to Foo::foomember const string& -> float
    Foo foo1;
    Foo* foo1ptr { &foo1 };
    typedef float (Foo::*funcptr1_t)(const string&); // this is old C++98 syntax
    funcptr1_t memfuncptr1 = &Foo::foomember;
    // See also http://www.newty.de/fpt/fpt.html for mem func ptr syntax,
    // which is ugly syntax imo.
    float x1 = (foo1.*memfuncptr1)("hi");
    float x2 = (foo1ptr->*memfuncptr1)("hi");

    // now try the same thing with std::function, which is much
    // more readable than this cryptic .* and ->* operator:
    // Note initially I forgot & after Foo, which sort of worked, but poorly.
    std::function<float(Foo&,const string&)> funcptr2 = &Foo::foomember;
    float x3 = funcptr2(foo1, "hi");

    // why is there std::mem_fn, isn't this redundant since std::function 
    // alrdy can store pointers to members? Nope: std::mem_fn() is the utility
    // function that will turn a member func ptr &A::fun into something
    // that can be treated like (and assigned to) std::function(TR(&A,T1,T2...))
    // Also note that C++98's std::mem_fun is deprecated! See 
    // http://stackoverflow.com/questions/11680807/stdmem-fun-vs-stdmem-fn
    auto funcptr3 = std::mem_fn(&Foo::foomember);
    float x4 = funcptr3(foo1, "hi");

    // or just using auto to hide the mem func type altogether, but then
    // you'll have to use .* or ->* operators:
    auto funcptr4 = &Foo::foomember;
    // doesn't compile: float x5 = funcptr4(foo1, "hi");
    float x5 = (foo1.*funcptr4)("hi");

    // this doesn't compile btw: 
    //   float(Foo,const string&) funcptr5;
   
    // and compare with 'using' syntax to typedef a func ptr type:
    using funcptr6_t = float (Foo::*)(const string&);
    funcptr6_t funcptr6 = &Foo::foomember;
    float x6 = (foo1.*funcptr6)("hi");

    // This doesn't compile:
    //   using funcptr5_t = float (*)(Foo,const string&);
    //   funcptr5_t funcptr5 = &Foo::foomember;
    // Clang says it cannot init the free func with a member func.
    // Luckily std::function & std::mem_fn can unify the syntax of these.
  
    // now functor assigned to function:
    // Functors are less important now that we have lambda, but we have
    // legacy functors:
    class Functor {
      public:
      // note I initially had Foo& f, but this wouldn't compile
      // P.S.: it didn't compile because I had messed up the signature
      // of funcptr2, now it does.
      float operator()(Foo& f, const string& s) { return 7; }
    };
    Functor functor {};
    float x8 = functor(foo1, "hi");
    funcptr2 = functor;
    
    // And test some conversions between these equivalent func types:
    funcptr2 = memfuncptr1; // std::function assigned from mem fun ptr
    // reverse doesnt compile:
    //   memfuncptr1 = funcptr2; // mem fun ptr from std::function
    funcptr2 = funcptr3; // function<> from mem_fn<>
    // reverse doesn't compile:
    //   funcptr3 = funcptr2;
    funcptr2 = funcptr6; // function from mem ptr in 'using' type alias
    funcptr2 = [](Foo& f, const string& s) -> float { return 0; }; // function<> from lambda
  }
  {
    // Now some func ptrs for global funcs, not mem ptrs:
    
    // old school C++98 typedef, kinda ugly imo:
    typedef void (*funcptr1_t)(int);
    funcptr1_t funcptr1 = &global_voidfunc; // not in JS you don't need a '&'
    // The '&' is kinda redundant in C++, whats the type of the expression 
    // without the '&' anyway? It's the same type I think:
    static_assert(!is_same<decltype(&global_voidfunc), decltype(global_voidfunc)>::value, "");
    // so no, it's not.
    funcptr1 = global_voidfunc; // so even though its not the types are convertible, TODO
    funcptr1(7);  // I could have sworn this syntax was not allowing in C++98...
    (*funcptr1)(7); // this syntax is kinda ugly imo
    // Anyway: as a rule of thumb NEVER use this ugly C++98 typedef syntax
    // to decl func ptrs.
    
    // with C++11 'using' instead of typedef:
    // The static_assert proves the types of both typedef & using are the same.
    using funcptr2_t = void (*)(int);
    funcptr2_t funcptr2 = &global_voidfunc;
    funcptr2(7);
    (*funcptr2)(7);  // this syntax we should consider deprecated?
    static_assert(std::is_same<funcptr1_t, funcptr2_t>::value, "");

    // with C++11 auto, not even bothering to typedef or using:
    auto funcptr3 = &global_voidfunc;
    funcptr3(7);
    static_assert(std::is_same<funcptr2_t, decltype(funcptr3)>::value, "");

    // with std::function:
    std::function<void(int)> funcptr4 = &global_voidfunc;
    funcptr4(7);
    //static_assert(std::is_same<funcptr2_t, decltype(funcptr4)>::value, "");
    // So not the same type, but compatible in many ways:
    funcptr4 = funcptr1;
    funcptr4 = funcptr2;
    funcptr4 = funcptr3;

    // P.S. just for completeness here's another syntax from
    // http://stackoverflow.com/questions/16498969/how-do-i-typedef-a-function-pointer-with-the-c11-using-syntax
    // I dont like this syntax too much at 1st glance, so lets forget it exists.
    using funcptr9_t = std::add_pointer<void(int)>::type;
    funcptr9_t funcptr7 = global_voidfunc;
    funcptr7(7);
    std::function<void(int)> funcptr8 = funcptr7;
  }

  // enum class: strongly typed enums, with fixed scoping
  // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2347.pdf which
  // also has a (potentiall expensive) workaround for C++98/03 (class Color 
  // with private enum).
  {
    enum OldColor { red, blue };
    enum OldWhatever { /* conflict: Red,*/ yellow, orange };
    bool crap = red >= yellow; // thats a warning only in Clang (implicit conversion)
    global_voidfunc(red); // also crap (again warning in Clang)

    // now newstyle:
    enum class Color { Red, Blue };
    enum class Whatever { Red, Yellow };
    //bool compileError = Color::Red >= Whatever::Red;
    //global_voidfunc(Color::Red); // also error now, yay!
  }

  // constexpr http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2235.pdf
  // Initially constexpr funcs were limited to a subset of the full language, 
  // e.g. ++ and while loops were not allowed.
  // Surprising: even constexpr ctors are allowed, wow (see n2235 'complex').
  //
  // Then C++14 generalized these further in http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3652.html
  // which for example allows loops and ++.
  //
  // Use cases:
  {
    struct A {
      static constexpr int mySquare(int n) {
        return n * n;
      }
    };
    assert(A::mySquare(3) == 9);
    // and you can even use them in static_asserts since the compiler is supposed
    // to eval the constexprs at compile time:
    static_assert(A::mySquare(3) == 9, "");

    // now more complex constexpr, this requires C++14
    struct C14 {
      static constexpr int countBits(int n) {
        // hardcore alt fyi:
        // http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
        int count = 0;
        for (unsigned int i=0; i<32; ++i) {
          if ((n & (unsigned(1) << i)) != 0)
            ++count;
        }
        return count;
      }
    };
    static_assert(C14::countBits(5) == 2, "");

    // here static_assert refuses to work understandably, clang says:
    // "read of non-const variable 'c' is not allowed in a constant expression"
    int c = 5;
    // compile time error: static_assert(C17::countBits(c) == 2, "");
    assert(C14::countBits(c) == 2);

    // Now lets see if we can hang the compiler with and endless looped constexpr:
    struct C14_hang {
      static constexpr int hangMe(int x) {
        int whatever = 7;
        while (x != 0) {
          ++whatever;
        }
        return whatever; // never returns for x != 0
      }
    };
    static_assert(C14_hang::hangMe(0) == 7, "");
    //static_assert(C14_hang::hangMe(1) == 7, ""); // should hang, but clang prints:
    // 'constexpr evaluation hit maximum step limit; possible infinite loop?'
    // That's pretty kick-ass!
  }

  // delegating ctors A() : A(expr1, ...) {}
  // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n1986.pdf
  // So no need for crappy commonConstructorCode() anymore, nice.
  //
  // Compare to C# syntax:
  //    A() : this(expr1, ...) {}
  // http://stackoverflow.com/questions/4009013/call-one-constructor-from-another
  {
    struct A {
      A() {}
      explicit A(int x) : x_(x) {}
      explicit A(string x) : A(std::stoi(x)) {}
      int x_ = {};
    };
    assert(A("123").x_ == 123);

    // Also see constructor function try blocks in n1986, I didnt know thats
    // a feature. Kinda weird with it implicit retrhow, I dont plan to use that.
    

    // To pull in all ctors from a base class do using base::base, which makes
    // this the umpteenth use of the 'using' keyword. This is not a frequently
    // needed feature, still nice to have:
    struct B1 : A {
    };
    //B1 b1("123"); // error: no matching constructor for initialization of 'B1'
    
    struct B2 : public A {
      using A::A; // pull in all ctors
    };
    B2 b2("123");
    assert(b2.x_ == 123);

    // this might even work with private inheritance, but I won't test that 
    // because private inheritance is undesirable.
  }

  // override/final/delete/default/using
  // While we're at pulling ctors from base classes: override & final & delete 
  // keywords for inherited funcs.
  //   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2346.htm
  // I like the idea of this change (except maybe that the concept of a deleted 
  // inherited func is questionable), but I don't like the execution: it
  // seems inconsistent to me that syntax require an '=' before a 'delete'd
  // func but you must not have the '=' before override and final.
  //
  // Note that tagging a dtor as final effectively makes the class final.
  {
    struct A {
      virtual ~A() {}
      virtual void vf1() {}
      virtual void vf2() =0;
      void f1() {}
      void f2() {}
      void f2(float) {}
    };
    struct B : public A {
      virtual void vf1() final {}
      virtual void vf2() override {}
    };
    struct C : public B {
      virtual ~C() final {}
      //virtual void vf1() {} // error: declaration of 'vf1' overrides a 'final' function
    };
    //struct D : public C { }; // impossible, C::~C is final 
    //A a1; // error: variable type 'A' is an abstract class. Clang notes are awesome!
    C c1;

    class C1 : public B {
      public:
        void f2(int) {}  // shadows A::f2(void)
    };
    C1 c1_1;
    //c1_1.f2(); // shadowed, good Clang note again
    
    class C2 : public B {
      public:
        void f2(int) {}

        // unshadow:
        using B::f2;  // this unshadows, even though B::f2 technically comes from A
        //using A::f2; // this un-shadows A::f2(void), also works
        // TODO: is there a way to only import one of the
        // overloads from a baseclass?
        // E.g. via:
        //    using B::f2(float); // won't compile
        //    using f2(float); // nope
        //    using void f2(float); // nope
        //    void f2(float) = using; // like 'delete', but won't work
        //    void f2(float) override {} // is supposed to fail (base func not virtual)
        // No big deal, excessively overloading is not desirable anyway,
        // and hardly ever you'd want to import a subset of inherited overloads,
        // just like it's weird to delete inherited funcs to beging with.
        // Actually a mixture of using & delete might do the trick:
        void f2(float) = delete;
     
        // semi-delete/hide inherited funcs
        // this is crappy from an OOP standpoint of course
        void f1() = delete; 
    };
    C2 c2_1;
    c2_1.f2();
    //c2_1.f2(0.5f); // this was affected by shading, using and delete. The poor thing.
    //c2_1.f1(); // error: attempt to use a deleted function

    struct X { // no virtual dtor, so not suited for inheriting from
      private: vector<int> foo {3,4};
    };
    struct Y : public X {
      private: vector<int> bla {1,2}; // leak
    }; // surprised Clang doesnt warn here (non-virt dtor in X)
    unique_ptr<X> xPtrToY(new Y()); // TODO: verify leak with Clang Memory Sanitizer (need x64)

    // Imo the most important use case of delete is disabling undesired conversions
    // and compiler-generated default ctors: no longer have to add private: decls for 
    // these to hide them, but can explicitly disable via '= delete'.
    // (see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2346.htm#delete).
    {
      struct WithConversion {
        explicit WithConversion(int x) {}
      };
      WithConversion w1(7); // OK
      WithConversion w2(7.5); // implicit conversion/narrowing, clang warns at least, no error
      WithConversion w3('x'); // potential bug with implicit conversion

      struct WithoutConversion {
        explicit WithoutConversion(int x) {}
        explicit WithoutConversion(char c) = delete; // C++11
        explicit WithoutConversion(float f) = delete; // C++11
        //WithoutConversion(WithoutConversion&& rhs) = delete; // autogen
      };
      WithoutConversion o1(7); // OK
      //WithoutConversion o2('x'); //error: call to deleted constructor of 'WithoutConversion', good!
      //WithoutConversion o3(7.5f); // error, good
      //WithoutConversion o4(7.5); //error: call to constructor of 'WithoutConversion' is ambiguous.
                                 // Clang's note are EPIC in this case again! Confusions seem
                                 // between auto-gen move & copy c'tors
      //WithoutConversion o5(static_cast<double>(7.5));
      WithoutConversion o6(wchar_t('x')); // sadly this compiles still, we got lots of implicit convs left
    }

    {
      struct NotCopyable {
        // since adding a deleted copy ctor counts as having added a ctor (which 
        // is weird imo) the compiler won't add a default no-arg ctor either.
        NotCopyable() = default; // or '{}'

        // this here is more concise than the old C++98 syntax:
        //    private: NotCopyable(const NotCopyable& rhs);
        NotCopyable(const NotCopyable& rhs) = delete;

        // deleting the copy ctor wont implicitly delete the copy aka assignment
        // operator, so we need to explicitly delete this also:
        NotCopyable& operator=(const NotCopyable& rhs) = delete;
      };
      NotCopyable n1, n2;
      //NotCopyable n3(n1); // errors, because default ctor was deleted
      //n2 = n1; // clang says "error: overload resolution selected deleted operator '='"
    }
  }
  
  // alignas/std::alignment (and redundant(?) alignof)
  // I wonder why we have both alignof and std::alignment_of, one of these is 
  // redundant (alignof(expr) == std::alignment_of<decltype(expr)>::value).
  // See http://stackoverflow.com/questions/7053190/what-are-the-alignas-and-alignof-keywords-used-for
  // So this is interesting only for low lvl SSE and such, so will rarely be needed.
  {
    int x = 0;
    //static_assert(alignof(x) == 4, ""); // GNU extension? But I see it in n2341
    static_assert(std::alignment_of<decltype(x)>::value == 4, "");
    static_assert(std::alignment_of<int>::value == 4, "");
    alignas(16) char charsStoringInt[4] = {1, 0, 0, 0};
    assert(((&charsStoringInt[0] - static_cast<char*>(nullptr)) & 15) == 0);

  }

  // attribute support, similar to C# [Test] sorts of attribs.
  // inspired by:
  //   * GCC __attribute__((aligned(16))) class Z {int i;}
  //   * MSVC __declspec(dllimport)
  // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2761.pdf syntax:
  //
  // [[attr1]] class C [[ attr2 ]] { } [[ attr3 ]] c [[ attr4 ]], d [[ attr5 ]];
  // attr1 applies to declarator-ids c, d 
  // attr2 applies to the definition of class C 
  // attr3 applies to type C 
  // attr4 applies to declarator-id c 
  // attr5 applies to declarator-id d 
  //
  // They list as a use case OpenMP. Others:
  //   * void fatal [[noreturn]] (void)
  //   * [[final]] on classes and virtual funcs to prevent inheritance/override
  //   * http://clang.llvm.org/docs/AttributeReference.html : 
  //
  // I'm not a big fan of this feature atm. Ignore.
  {
    //class A [[final]] { public: virtual ~A() {} };
    // clang says "warning: unknown attribute 'final' ignored [-Wunknown-attributes]"
  }

  // std::ref - what exactly is this for? So this creates a reference-like object?
  // http://stackoverflow.com/questions/15530460/what-would-be-a-hello-world-example-for-stdref
  // http://stackoverflow.com/questions/9582312/how-to-use-stdref
  // I have not seen a single use case for std::ref that wasn't related to 
  // std::bind or some other functor. Since std::bind should be replaced by
  // lambdas in C++11 that makes std::ref kinda useless in my book.
  // Anyone has a usecase beyond bind()?
  {
    // no need for std::ref:
    int x = 5;
    std::thread worker {[&x]() { ++x; }}; // dangerous bind, unless you join below!
    worker.join();
    assert(x == 6);
  }

  // C++14 variable templates (not to be confused with variadic templates)
  // See usecase PI<float>, PI<double>.
  {
    // TODO
  }

  play_with_member_pointers();
}

// exercise: hide the ->* member ptr syntax for non-function members, similar
// to how std::mem_fn hides that. 
// See http://stackoverflow.com/questions/670734/c-pointer-to-class-data-member

// just some syntactic sugar around C++ member pointer syntax for
// exercise's sake:
template <class T, typename V>
class mem_ptr {
  public:
    typedef V T::*raw_mem_ptr_t; // this is the imo ugly raw ptr syntax we want to hide

    // ctor from &T::someMember
    mem_ptr(raw_mem_ptr_t ptr) : raw_mem_ptr(ptr) {}
    V get(const T& t) const { return t.*raw_mem_ptr; }
    void set(T& t, const V& v) { t.*raw_mem_ptr = v; }
  private:
    raw_mem_ptr_t raw_mem_ptr;
};

// intrusive ptr list (unlike the preferably std::forward_list, which is
// non-intrusive)
template <class T>
class List {
  public:
    List(mem_ptr<T, T*> next_ptr) : next_ptr_(next_ptr) {}
    void prepend(T& e) {
      next_ptr_.set(e, head_);
      head_ = &e;
    }
    int size() const {
      auto current = head_;
      int count = 0;
      while (current != nullptr) {
        ++count;
        current = next_ptr_.get(*current);
      }
      return count;
    }
  private:
    mem_ptr<T, T*> next_ptr_;
    T* head_ = {nullptr};
};

void play_with_member_pointers() {

  // My list elem with intrusive ptr (see SO). I assume list<T> is (or std::forward
  // list is equally efficient, so this example is kinda impractical)
  struct Apple {
    Apple() {}
    explicit Apple(int d) : data(d) {}
    int data = {};
    Apple* next = nullptr; // intrusive ptr
  };

  Apple apple1;
  mem_ptr<Apple, int> ptrToAppleIntMember = { &Apple::data };
  mem_ptr<Apple, Apple*> ptrToApplePtrMember = { &Apple::next };

  List<Apple> apples(&Apple::next);
  apples.prepend(apple1);
  Apple apple2;
  apples.prepend(apple2);
  assert(apples.size() == 2);

  // now for comparison use std::list and C++11 std::forward_list:
  std::list<Apple> apples2 { apple1, apple2 };
  apples2.push_front(apple1); // a copy is pushed (unlike for my List above)
  // test the new C++11 emplace while we're at it:
  apples2.emplace_front(5); // calls Apple(int) ctor
  assert(apples2.front().data == 5);
  assert(std::next(apples2.begin())->data == 0); // special case of std::advance()
  assert(apples2.size() == 4);

  std::forward_list<Apple> apples3 { apple1, apple1, apple2, apple1 };
  // there's no size() on forward_list, probably because it's O(N) was
  // gonna be deceiving. But a count() would have been nice...
  // Anyway, std::distance is handy:
  assert(std::distance(apples3.begin(), apples3.end()) == 4);

  cout << "we have 0 oranges!\n";

}

