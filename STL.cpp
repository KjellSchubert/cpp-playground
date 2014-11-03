// move constructor playground.
// See also:
// http://en.cppreference.com/w/cpp/language/move_constructor
// http://blogs.msdn.com/b/xiangfan/archive/2012/02/03/c-98-gt-c-11-pass-by-value-or-pass-by-reference.aspx
// http://stackoverflow.com/questions/24543330/when-is-const-reference-better-than-pass-by-value-in-c11

#include <iostream>
#include <vector>
#include <iterator> // std::back_inserter
#include <functional> // std::plus
#include <algorithm> // std::transform
#include <cassert> // assert
//#include <initializer_list> // std::initializer_list
#include <string>

using namespace std;

/* not VS yet, Clang/gcc only:
template <class T>
struct S {
    std::vector<T> v;
    S(std::initializer_list<T> l) : v(l) {
         std::cout << "constructed with a " << l.size() << "-element list\n";
    }
    void append(std::initializer_list<T> l) {
        v.insert(v.end(), l.begin(), l.end());
    }
    std::pair<const T*, std::size_t> c_arr() const {
        return {&v[0], v.size()};  // list-initialization in return statement
                                   // this is NOT a use of std::initializer_list
    }
};
*/

// could also make template class, iter over chars for example
class PythonStyleRange {
  public:
    PythonStyleRange(int begin, int end, int delta=1) : _begin(begin), _end(end), _delta(delta) {}

  private:
    int _begin;
    int _end;
    int _delta;

    class PythonStyleRangeIterator {
      public:
        PythonStyleRangeIterator(int i) : _i(i) {}
        int operator*() const { return _i; }
        void operator++() { ++_i; }
        bool operator!=(const PythonStyleRangeIterator& rhs) { return _i != rhs._i; }
      private:
        int _i;
    };

  public:

    // I thought I can return T directly, dont even need class PythonStyleRangeIterator, but need iter
    // after all for operator*()
    PythonStyleRangeIterator begin() const { return PythonStyleRangeIterator(_begin); }
    PythonStyleRangeIterator end() const { return PythonStyleRangeIterator(_end); }
};

void main() {

  // From http://en.cppreference.com/w/cpp/utility/initializer_list:
  // not not VS2012 yet: std::vector<int> elems = { 6, 9, 12, 15};
  std::vector<int> elems;
  for (int i=0; i<4; ++i) // no C++ equivalent of Python for i in range?
    elems.push_back(6 + i*3);

  // range for over Python-style range
  elems.clear();
  for (auto i : PythonStyleRange(0,4)) // is this any less efficient than the more verbose for loop? Probably not, but I didn't check assembly.
    elems.push_back(6 + i*3);

  // range for over container
  for (auto elem : elems)
    cout << "ranged for loop: " << elem << endl;

  // lodash.js / JS forEach with lambda
  for_each(elems.begin(), elems.end(), [](int elem) { 
    cout << "for_each elem "  << elem << endl; 
  });

  // Python/lodash filter via http://www.cplusplus.com/reference/algorithm/all_of/
  // Nope, all_of is combination of filter() and reduce/aggregate(), just testing a condition. Oddball.
  /*
  all_of(elems.begin(), elems.end(), [](int elem) {
    return elem % 2 = 0;
  });
  */

  // find_if aka findFirst
  {
    auto it = find_if(elems.begin(), elems.end(), [](int elem) {
      return elem % 2 != 0;
    });
    cout << "found " << (it != elems.end() ? to_string(*it) : "nothing") << endl;

    it = find_if(elems.begin(), elems.end(), [](int elem) {
      return elem == 7; // in this case a plain find() would have been cheaper, just testing here tho
    });
    cout << "found " << (it != elems.end() ? to_string(*it) : "nothing") << endl;
  }

  // sort of functional map, just in an awkward & inefficient fashion
  // what I want instead is 
  // int result = elems.filter([](int elem) {
  //   return elem % 2 == 0;
  // })
  // .map([](int elem) {
  //   return elem * 10
  // })
  // .reduce_aka_accumulate((int accu, int elem) => { // note Python3 demoted reduce(), lodash.js still has it
  //   return accu + elem;
  // }, 0); // 0 being initial accu
  {
    vector<int> mappedElems;
    std::transform(elems.begin(), elems.end(), std::back_inserter(mappedElems), [](int elem) {
      return elem * 10;
    });
    for_each(mappedElems.begin(), mappedElems.end(), [](int elem) { 
      cout << "mapped elem "  << elem << endl; 
    });
  }

  // 

  // TODO: <functional> bind
}