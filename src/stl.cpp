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

/* TODO
template <class ElemType>
class PyFilterResult {
  public:
    PyFilterResultIterator<T> begin() {
    }

    PyFilterResultIterator<T> end() {
    }
};

template<class Container, typename ElemType>
PyFilterResult<ElemType> py_filter(const Container& container, function<bool, ElemType> predicate) {
  return PyFilterResult(container, predicate);
}
*/

// This func explores Python or lodash.js-style filter, map, reduce, so what I want is:
// int result = elems.filter([](int elem) {
//   return elem % 2 == 0;
// })
// .map([](int elem) {
//   return elem * 10
// })
// .reduce_aka_accumulate((int accu, int elem) => { // note Python3 demoted reduce(), lodash.js still has it
//   return accu + elem;
// }, 0); // 0 being initial accu
// 
// In other words I want to concat filters & transforms like in http://www.dabeaz.com/generators-uk/GeneratorsUK.pdf
// slide 59, 
//    filenames = gen_find("access-log*",logdir)
//    logfiles = gen_open(filenames)
//    loglines = gen_cat(logfiles)
//    patlines = gen_grep(pat,loglines)
//    bytecolumn = (line.rsplit(None,1)[1] for line in patlines)
//    bytes = (int(x) for x in bytecolumn if x != '-')
// I also want lazy eval to deal with infinite input, so I don't want to construct 
// explicit containers for each processing stage.
// See http://en.cppreference.com/w/cpp/concept/OutputIterator
//
// Shouldnt something like that be part of Boost? Cannot find it atm.
// Discussed here: http://programmers.stackexchange.com/questions/170464/c11-support-for-higher-order-list-functions
// and here http://www.meetingcpp.com/tl_files/mcpp/slides/12/FunctionalProgrammingInC++11.pdf
//
// P.S. more here:
// * http://paoloseverini.wordpress.com/2014/06/09/generator-functions-in-c/
// * http://www.boost.org/doc/libs/1_57_0/libs/coroutine/doc/html/index.html
void functional_filter_map_reduce_playground() {

  /* TODO
  auto seq0 = PythonStyleRange(0, 100);
  auto seq1 = py_filter(seq0, [](elem) { return elem % 3 == 0; });
  auto seq2 = py_map(seq1, [](elem) { return elem * 10; });
  auto seq3 = py_filter(seq2, [](elem) { return elem + 1; });
  auto seq4 = py_map(seq3, [](elem) { return string(elem); });
  auto seq5 = py_splice(seq4, 0, 5);
  copy_n(seq5.begin(), 10, ostream_iterator<string>(cout, "\n"));
  */
}



struct Foo {
    void print_sum(int n1, int n2)
    {
        std::cout << n1+n2 << '\n';
    }

    // add overload to make std::bind more difficult:
    //void print_sum(const Foo& f) {
    //}

    int data;
};

void testBind() {
  using namespace std::placeholders;  // for _1, _2, _3...
  Foo foo;
  auto f3 = std::bind(&Foo::print_sum, &foo, 95, _1);
  f3(5);
}

void play_with_stl() {

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
  // See http://stackoverflow.com/questions/15927033/what-is-the-correct-way-of-using-c11s-range-based-for
  // So remember to use for ([const] auto&) when appropriate.
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
  // Could become a single func call if I'd use ostream_iterator, but this still 
  // doesn't allow you to easily chain multiple iterator/generator processing steps
  // the way you do with lodash.js or Python list/generator comprehensions.
  {
    vector<int> mappedElems;
    std::transform(elems.begin(), elems.end(), std::back_inserter(mappedElems), [](int elem) {
      return elem * 10;
    });
    for_each(mappedElems.begin(), mappedElems.end(), [](int elem) { 
      cout << "mapped elem "  << elem << endl; 
    });
  }

  // containers:
  // http://john-ahlgren.blogspot.com/2013/10/stl-container-performance.html

  // set (and map), with logN insert and find (tree-based, not hash based)
  // Requires a less-than comparison function
  // (http://lafstern.org/matt/col1.pdf)
  // BTW: I always found it odd that the STL gave the tree-based impl the shorter
  // name, and the hash-based (unordered_set/map) one the longer name. Since most
  // of the time you want the hash-based impl, which tends to be faster for the 
  // average language & usecase, e.g. see pypi:rbtree being slower than dict()
  // and Java http://stackoverflow.com/questions/1463284/hashset-vs-treeset. Is
  // the same true for C++? My guess yes, but I didnt benchmark enough use cases
  // to be 'sure'.
  // You definitely want std::set/map when you have to keep your container sorted
  // at all times (a requirement which I rarely needed).
  {
    //set<string> cont = { "a", "b" };
    set<string> cont;
    cont.insert("bla");
    auto foo1 = "foo";
    auto foo2 = "foo";
    cont.insert(foo1);
    cont.insert(foo1);
    cont.insert(foo2);
    cout << "set contains 'foo'? " << (cont.find("foo") != cont.end())  << endl;
    for (auto elem : cont)
      cout << "set elem " << elem << endl;
  }

  {
    map<string, int> sToInt;
    typedef pair<string, int> KeyValuePair;
    sToInt.insert(KeyValuePair("a", 1));
    sToInt.insert(KeyValuePair("b", 55));
    sToInt.insert(KeyValuePair("foo", 99));
    auto printContains = [=](string elem) {
      cout << "map contains '" << elem << "'? " << (sToInt.find(elem) != sToInt.end())  << endl;
    };
    printContains("foo");
    printContains("bla");
    for (auto pair : sToInt)
      cout << "map elem " << pair.first << " " << pair.second << endl;
  }

  // multiset: like set but dup keys are allowed, same complexity
  {
    multiset<string> cont;
    cont.insert("bla");
    auto foo1 = "foo";
    auto foo2 = "foo";
    auto foo3 = "foo";
    cont.insert(foo1);
    cont.insert(foo1);
    cont.insert(foo2);
    cont.find(foo1);

    auto printContains = [=](string elem) {
      cout << "multiset contains '" << elem << "'? " << (cont.find(elem) != cont.end())  << endl;
    };
    printContains(foo1);
    printContains(foo2);
    printContains(foo3);
    for (auto elem : cont)
      cout << "multiset elem " << elem << endl;
  }

  // unordered_set and unordered_map (hash tables with O(1) insert & find except when collision)
  // Requires an equals comparison & hash function I guess
  {
    unordered_set<string> cont;
    // code is 100% the same as for set:
    cont.insert("bla");
    auto foo1 = "foo";
    auto foo2 = "foo";
    cont.insert(foo1);
    cont.insert(foo1);
    cont.insert(foo2);
    cout << "unordered_set contains 'foo'? " << (cont.find("foo") != cont.end())  << endl;
    for (auto elem : cont)
      cout << "unordered_set elem " << elem << endl;

    cout << "same list using ostream_iterator:\n";
    copy(cont.begin(), cont.end(), ostream_iterator<string>(cout, "\n"));
  }

  functional_filter_map_reduce_playground();

  // TODO priority_queue with O(1) top and O(logN) pop

  // http://en.cppreference.com/w/cpp/utility/functional/bind
  // Is this useful at all still when we have lambdas now? Not on the application level I think, 
  // maybe somewhere in the bowels of STL. See also http://vimeo.com/97318797
  // 16:40 making fun of bind.
  {
    cout << endl << "std::bind\n";
    multiset<string> cont;
    auto ins = [&](string s) {
      cont.insert(s);
    };
    cont.insert("bla");
    ins("foo");
    ins("bar");
    // Why doesn't this compile? Both VS2012 & clang 3.5 err msgs seemed pretty
    // cryptic to me. Turns out mutiset<T>::insert has several overloads, and
    // you need to explicitly disambiguate between them for std::bind, see
    // http://stackoverflow.com/questions/17874489/disambiguate-overloaded-member-function-pointer-being-passed-as-template-paramet
    // So this here doesn't compile:
    //  auto ins_baz = std::bind(&multiset<string>::insert, std::ref(cont), "baz");
    // This here does:
    typedef multiset<string> Cont;
    typedef Cont::iterator (Cont::*ContInsertFunc)(const std::string&); // disgusting syntax!
    // See also http://www.oopweb.com/CPP/Documents/FunctionPointers/Volume/CCPP/FPT/em_fpt.html
    // for this terrible C++ member func ptr syntax.
    // Also http://stackoverflow.com/questions/4832275/c-typedef-member-function-signature-syntax
    // See http://stackoverflow.com/questions/16016112/stdbind-of-class-member-function:
    // '&cont' vs 'cont' vs 'std::ref(cont)'binding: one binds copy, the other binds
    // the ref.
    auto ins_baz = std::bind(
      static_cast<ContInsertFunc>(&Cont::insert), 
      std::ref(cont), // or just 'cont' or '&cont' (the former will insert into a copy!)
      "baz");
    ins_baz();
    ins_baz();
    
    // Does C++11 have any better ways to specify member func ptr?
    // See examples at http://en.cppreference.com/w/cpp/utility/functional/function
    // Something like this here should have worked but gives me errors:
    //   std::function<Cont::iterator(Cont&, const string&)> contInsertFunc = &Cont::insert; 
    // note auto won't work here because insert() is overloaded.
    // Got little incentive to get this to work since lambdas are so much more 
    // readable in this overloaded func case.
    // This here works in the case without overloads:
    std::function<void(Foo&, int, int)> Foo_print_sum = &Foo::print_sum;
    Foo foo;
    Foo_print_sum(foo, 1, 2);
            


    // C++11 lambda makes bind() borderline useless imo. But see
    // http://stackoverflow.com/questions/6868171/c0x-lambda-wrappers-vs-bind-for-passing-member-functions
    cout << "final content of cont:\n";
    copy(cont.begin(), cont.end(), ostream_iterator<string>(cout, "\n"));
    cout << "--end cont\n";
  }
  testBind();

  // ifstream/ofstream
  {
    // Whats a decent way to deal with encodings in C++?
    // See http://stackoverflow.com/questions/1274910/does-wifstream-support-different-encodings
    // So ICU does not play nicely with std:: streams?
    // Boost has UTF8 facet: http://www.boost.org/doc/libs/1_39_0/libs/serialization/doc/codecvt.html
    // The facet stuff seems a bit verbose to me, and the 'Set a New global locale' is not
    // gonna play well in multithreaded programs :(
    // In any case the boost class made it into std.
    // Copied from http://en.cppreference.com/w/cpp/locale/codecvt :
    
    // UTF-8 narrow multibyte encoding
    std::string data = u8"z\u00df\u6c34\U0001f34c";
    std::ofstream("text.txt") << data;

    std::wifstream fin("text.txt");
    fin.imbue(std::locale("en_US.UTF-8"));
    std::cout << "The UTF-8 file contains the following UCS4 aka utf32 code points: \n";
    for (wchar_t c; fin >> c; )
       std::cout << "U+" << std::hex << std::setw(4) << std::setfill('0') << c << '\n';

    // note that C++11 introduces http://en.cppreference.com/w/cpp/locale/codecvt_utf8
  }
}
