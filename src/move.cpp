// move constructor playground.
// See also:
// http://en.cppreference.com/w/cpp/language/move_constructor
// http://blogs.msdn.com/b/xiangfan/archive/2012/02/03/c-98-gt-c-11-pass-by-value-or-pass-by-reference.aspx
// http://stackoverflow.com/questions/24543330/when-is-const-reference-better-than-pass-by-value-in-c11
//
// http://google-styleguide.googlecode.com/svn/trunk/cppguide.html suggests:
//   "Use rvalue references only to define move constructors and move assignment
//    operators. Do not use std::forward."

#include <iostream>
#include <vector>
#include <iterator> // std::back_inserter
#include <functional> // std::plus
#include <algorithm> // std::transform
#include <cassert>
#include <string>

using namespace std;

namespace {

// example class for testing move c'tors, not doing anything useful
class A {
  public:

    explicit A(int elemN) : wasSalvagedByMoveCtor(false) { 
      cout << "A ctor size=" << elemN << "\n"; 
      for (int i=0; i < elemN; ++i)
        data.push_back(i);
    }

    ~A() { 
      cout << "A dtor size=" << data.size() << " wasSalvagedByMoveCtor=" << wasSalvagedByMoveCtor << "\n"; 
    }

    A(const A& rhs) : wasSalvagedByMoveCtor(false) {
      ++copyCount;
      cout << "A copy ctor size=" << rhs.data.size() << "\n"; 
      assert(!wasSalvagedByMoveCtor && !rhs.wasSalvagedByMoveCtor);
      data = rhs.data; // expensive, is not a move
    }

    int f() const {
      cout << "A.f() size=" << data.size() << "\n";
      return data.size();
    }

    // same thing as 'A& operator= (const A& rhs) = default', except it also logs
    A& operator= (const A& rhs) {
      ++copyCount;
      cout << "A copy op size=" << rhs.data.size() << "\n"; 
      data = rhs.data;
      return *this;
    }

    static A create_retByValue() {
      return A(3);
    }

#define CONFIG_ENABLE_MOVE_CTOR // comment out this line to test with or w/o move c'tor
#if defined(CONFIG_ENABLE_MOVE_CTOR)
    A(A&& rhs) : wasSalvagedByMoveCtor(false) {
      ++moveCount;
      cout << "A move ctor\n"; 
      assert(!wasSalvagedByMoveCtor && !rhs.wasSalvagedByMoveCtor);
      swap(data, rhs.data);
      rhs.wasSalvagedByMoveCtor = true;
    }
#endif

    // Vector add.
    // The longer vector is cut back to the size of the shorter one.
    static A add_argsByValue_retByValue(A a, A b) {
      assert(!a.wasSalvagedByMoveCtor && !b.wasSalvagedByMoveCtor);
      A result(0);
      auto minSize = min(a.data.size(), b.data.size());
      result.data.reserve(minSize);
      std::transform(a.data.begin(), a.data.end(), b.data.begin(), 
                   std::back_inserter(result.data), std::plus<int>());
      return result;
    }

    // same impl, except params passed by ref
    static A add_argsByRef_retByValue(const A& a, const A& b) {
      assert(!a.wasSalvagedByMoveCtor && !b.wasSalvagedByMoveCtor);
      A result(0);
      auto minSize = min(a.data.size(), b.data.size());
      result.data.reserve(minSize);
      std::transform(a.data.begin(), a.data.end(), b.data.begin(), 
                   std::back_inserter(result.data), std::plus<int>());
      return result;
    }

    static void logAndResetCopyCount() {
      cout << "summary: copyCount=" << copyCount << " moveCount=" << moveCount << endl;
      copyCount = 0;
      moveCount = 0;
    }

  private:
    vector<int> data;
    bool wasSalvagedByMoveCtor;
    
    static int copyCount;
    static int moveCount;
};

int A::moveCount = 0;
int A::copyCount = 0;

}

int func_argByValue(const A a) {
  return a.f();
}

int func_argByRef(const A& a) {
  return a.f();
}

int func_argByRvalueRef(const A&& a) {
  return a.f();
}

/* this makes compilation ambiguous
int fOverloaded(A a) {
  cout << "fOverloaded A\n";
  return a.f();
}
*/

int fOverloaded(const A& a) {
  cout << "fOverloaded A&\n";
  return a.f();
}

int fOverloaded(const A&& a) {
  cout << "fOverloaded A&&\n";
  return a.f();
}

void play_with_move() {

  cout << "\nexample A ctor\n";
  {
    A a(10);
  }
  A::logAndResetCopyCount();

  cout << "\nexample create_byValue\n";
  {
    A a(A::create_retByValue());
    a.f();
  }
  A::logAndResetCopyCount();

  // Here the args that are being passed are copy constructed, making
  // this an expensive & slow operation.
  cout << "\nexample add_argsByValue_retByValue\n";
  {
    A a(10);
    A b(20);
    auto sum = A::add_argsByValue_retByValue(a, b);
  }
  A::logAndResetCopyCount();

  // note that here despite passing args by value VS2012 generates no copy c'tor calls,
  // so number of mem operations is minimal
  cout << "\nexample add_argsByValue_retByValue with inline constructed args\n";
  {
    auto sum = A::add_argsByValue_retByValue(A(10), A(20));
  }
  A::logAndResetCopyCount();

  // Passing args by const ref is still superior in C++11:
  cout << "\nexample add_argsByRef_retByValue\n";
  {
    A a(10);
    A b(20);
    auto sum = A::add_argsByRef_retByValue(a, b);
  }
  A::logAndResetCopyCount();

  cout << "\nexample add_argsByRef_retByValue with inline constructed args\n";
  {
    auto sum = A::add_argsByRef_retByValue(A(10), A(20));
  }
  A::logAndResetCopyCount();



  cout << "\nfunc_argByValue\n";
  {
    func_argByValue(A(6));
  }
  A::logAndResetCopyCount();

  cout << "\nfunc_argByRef\n";
  {
    func_argByRef(A(6));
  }
  A::logAndResetCopyCount();

  cout << "\nfunc_argByRvalueRef\n";
  {
    func_argByRvalueRef(A(6));
  }
  A::logAndResetCopyCount();



  cout << "\nfunc_argByValue, lvalue arg\n";
  {
    A a(7);
    func_argByValue(a);
  }
  A::logAndResetCopyCount();

  cout << "\nfunc_argByRef, lvalue arg\n";
  {
    A a(7);
    func_argByRef(a);
  }
  A::logAndResetCopyCount();

  /* doesn't compile, since a is not an rvalue
  cout << "\nfunc_argByRvalueRef, lvalue arg\n";
  {
    A a(7);
    func_argByRvalueRef(a);
  }
  A::logAndResetCopyCount();
  */

 cout << "\nfOverloaded(A(8))\n";
 {
    fOverloaded(A(8));
  }
 cout << "\nfOverloaded(a)\n";
  {
    A a(9);
    fOverloaded(a);
  }

  A::logAndResetCopyCount();

  //xxx TODO test vector<A>
}
