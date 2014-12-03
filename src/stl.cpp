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

void stl_play_with_sort();

void stl_play_with_custom_type_traits();

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

  // some string tests
  {
    string s = "a";
    cout << "sizeof(string)=" << sizeof(s) << '\n'; // I see 4 for gcc 4.9 / clang 3.5
    const auto delta = s.data() - reinterpret_cast<string::pointer>(&s);
    bool hasSmallStringOptimization = 0 <= delta && delta <= sizeof(s);
    cout << "hasSmallStringOptimization=" << boolalpha << hasSmallStringOptimization << endl;
  }
  {
    string s = "hello world";
    //s.append('s');
    s.append("s!");
    const int indexLastL = s.rfind("l");
    assert(indexLastL != string::npos);
    s.erase(s.begin() + indexLastL);
    assert(s == "hello words!");
    cout << "string extra capacity after manips is " 
         << dec << (s.capacity() - s.length()) << endl;
    s.shrink_to_fit(); // since C++11, no more need for swap trick
    //assert(s.capacity() == s.length()); // not really guaranteed, or is it? not critical
  }
  {
    // note string.size & string.length are 100% identical,
    // both returning len in bytes, not chars (since encoding
    // is unspecified):

    std::string s = u8"\u00c4 10 \u20AC"; // A-umlaut & euro symbol. And notice the 'u8', which is C++11
    // so here the string just knows its bytes, still not
    // its encoding. Unlike a string in most other languages, e.g.
    // python3. wstring or basic_string<uchar16_t> or more like
    // a python3 string, as in str.length() will return len in
    // chars, not bytes. Though I guess uchar16_t still could
    // represent some code points / glyphs in >1 uchar16s.
    // wchar_t should be 32 bit (is this guaranteed?).
    //   >python3
    //   s = "\u00c4 10 \u20AC"
    //   print(s) # prints umlaut, though my terminal shows ? for Euro
    //   print(len(s)) # prints 6, so len in (utf32?) chars
    //   b = s.encode('utf-8')
    //   print(len(b)) # so utf8 encoding is 9 bytes
    //   print(b) # b'\xc3\x84 10 \xe2\x82\xac'
    //   assert len(s.encode('utf16')) - 2 == 2 * len(s) # 2 bytes for utf16 FF FE marker
    assert(s.length() == s.size());
    assert(s.length() == 9); // len in utf8 bytes (string is like a vector<byte>)
    std::string s2 = "\xc3\x84 10 \xe2\x82\xac"; // happens to be utf8
    assert(s == s2);
    
    // this here cannot work:
    //   wstring ws = s; // error: no viable conversion from 'basic_string<char>' to 'basic_string<wchar_t>'
    // because the string is not aware of its encoding. And I think
    // even the wstring is not aware of its encoding, which only by
    // conventions is utf32. Or does the standard explicitly enforce
    // utf32 for library interop? Unsure.
    // 
    // This here could work:
    //   wstring ws(s, 'utf8')
    // if C++ had such a ctor. Out options are:
    //   * ICU
    //   * libiconv
    //   * C++11 #include <codecvt> - sadly not supported in GCC 4.9 yet
    //   * use clang/llvm's own libc++
    //   * boost::codecvt?
    //   * a homebrewn utf8->16 converter (20ish lines of undesirable code)
    // I tried libc++:
    //   >apt-cache search "libc\+\+" | more
    //   >apt-cache show "libc\+\+-dev"
    //   >sudo apt-get install libc++-dev
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert16;
    std::u16string s3 = convert16.from_bytes(s);
    // About string literals: see http://en.cppreference.com/w/cpp/language/string_literal
    // So the variants are:
    //   "" for char[], so a byte sequence
    //   L"" is wide string literal (pre C++11)
    //   u8"" u"" U"" are new C++11 with utf8/16/32 encodings of the escaped chars
    assert(s3.data() == u16string(u"\u00c4 10 \u20AC"));
    //cout << "utf16: " << s3 << '\n'; // error, you cannot cout u16strings?

    // now the same conversion with wstring instead u16string:
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>,wchar_t> convertW;
    std::wstring s4 = convertW.from_bytes(s);
    assert(s4.data() == wstring(L"\u00c4 10 \u20AC"));
  }

  // reverse_iterators & reverse_iterator.base()
  {
    // pretty contrived example: there should hardly ever be a need for
    // a reverse_iterator on a map:
    set<int> elems = {3, 6, 8, 10};
    assert(*elems.begin() == 3);
    assert(*elems.rbegin() == 10);
    auto reverse_iter = elems.rbegin();
    //reverse_iter += 1; // doesnt work , good
    std::advance(reverse_iter, 1); // mutating func btw, unlike std::next which rets a copy
    assert(*reverse_iter == 8);
    reverse_iter = std::next(reverse_iter);
    assert(*reverse_iter == 6);

    // WARNING: the behavior of reverse_iter.base() is surprising and
    // unintuitive imo: the reverse_iter.base() points not to the 
    // same element the reverse_iter points to! Reasoning seems to be
    // that the reverse_iterator is supposed to facilitate intuitve
    // insertion, but not deletion. See Meyers STL book item 28.
    // http://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
    auto iter = reverse_iter.base();
    assert(*iter == 8);
    elems.erase(iter, elems.end());
    assert(elems == set<int>({3,6}));
  }
  {
    // same test with vector, code is 100% identical to map code:
    vector<int> elems = {3, 6, 8, 10};
    assert(*elems.rbegin() == 10);
    auto reverse_iter = elems.rbegin();
    //reverse_iter += 1; // works with vector iter
    std::advance(reverse_iter, 1); // mutating func btw, unlike std::next which rets a copy
    assert(*reverse_iter == 8);
    reverse_iter = std::next(reverse_iter);
    assert(*reverse_iter == 6);
    auto iter = reverse_iter.base();
    assert(*iter == 8);
    elems.erase(iter, elems.end());
    assert(elems == vector<int>({3,6}));

  }

  /*
  // checking if Clang diagnoses (somewhat) common ctor problems:
  {
    struct A {
      A(string x) : s(x) {} 
      string f() { return "hi"; }
      string getS() const { return s; }
      string s = "foo";
    };
    struct B : public A {
      //B() : A(f()) {} // sadly no clang warning here :(
      B() : A(getS()) {} // no warn here either, and we actually access uninit mem here
    };
    B b;
    cout << b.getS(); // prints garbage, but the (naive) intention was to get "hi"
    assert(b.getS() == "hi");
  }
  */

  // testing istream_iterator, call the app like this:
  //   echo "hello world" | out/Default/play
  // Otherwise it'll wait for interactive input until you ctrl-d (Linux) or 
  // ctrl-z (Windows), see http://stackoverflow.com/questions/3603047/find-the-end-of-stream-for-cin-ifstream
  /*
  {
    cout << "reading from cin (ctrl-d/z to end or echo ... |):\n";
    vector<string> coll;
    copy(
      istream_iterator<string>( cin ),
      istream_iterator<string>(),
      back_inserter( coll ) );
    cout << "coll.size=" << coll.size() << endl;
    copy(coll.begin(), coll.end(), ostream_iterator<string>(cout, "\n"));
  }
  */
  /*
  {
    cout << "reading from cin (ctrl-d/z to end or echo ... |):\n";
    int lineN = 0;
    while (!cin.eof()) {
      string line;
      getline(cin, line);
      cout << "read line:'" << line << "'" << endl;
      ++lineN;
    }
    cout << "read lines: " << lineN << '\n';
  }
  */
  // same test but using iofstream instead of cin:
  // what about char encodings?
  // http://stackoverflow.com/questions/5026555/c-how-to-write-read-ofstream-in-unicode-utf8
  {
    // the default ofstream mode is not binary?!
    // a) 
    ofstream("test.txt", ofstream::out | ofstream::binary) << "hello\nworld";
    // b) use iostream.imbue:
    /*
    std::locale utf8_locale(std::locale(), new utf8cvt<false>);
    {
      ofstream out("test.txt");
      out.imbue(utf8_locale);
      out << "hello\nworld";
    }
    */
    
    auto inStream = ifstream("test.txt", ifstream::in | ifstream::binary);
    //inStream.imbue(utf8_locale);
    vector<string> lines;
    while (!inStream.eof()) {
      string line;
      getline(inStream, line);
      cout << "read line:'" << line << "'" << endl;
      lines.push_back(line);
    }

    auto expectedLines = vector<string>{"hello", "world"};
    assert(lines == expectedLines);

    // while we're at it try some alternative ways to print vector content:
    cout << "ostream_iterator:\n";
    copy(lines.begin(), lines.end(), ostream_iterator<string>(cout, "\n"));
    cout << "for_each:\n";
    std::for_each(lines.cbegin(), lines.cend(), [](const string& line) { 
      // note for_each passes a value, not an iter to the functor/lambda
      cout << line << '\n'; 
    });
    cout << "for:\n";
    for (const auto& line : lines)
      cout << line << '\n';
  }

  stl_play_with_sort();

  // bit fields: I first thought these were part of C++11, but they were
  // available in C++98 alrdy. This code here has nothing to do with STL,
  // but whatever.
  // See also the much despised std::vector<bool> and also std::bitset<>.
  {
    struct S {
      S() : a(2), b(2) {}
      // you actually cannot decl bitfields in funcs, kinda inconsistent
      char a : 3; // int works too
      char b : 4;
      // error: bitfield member cannot have an in-class initializer, so much
      // for uniform initialization...
    };
    S s;
    s.a = 9; // warning: implicit truncation from 'int' to bitfield changes value from 9 to 1
    assert(s.a == (9 & 0x7));
    static_assert(sizeof(S) == 1, ""); // not guaranteed
    cout << "bitfield s bit value=0x" << hex 
      << static_cast<int>(*reinterpret_cast<char*>(&s)) << endl;
    cout << "decimal 16=" << 16 << endl; // the statefulness of cout can be annoying...
    cout << dec;
    // see http://stackoverflow.com/questions/1513625/c-how-to-reset-the-output-stream-manipulator-flags
    // for boost::io::ios_all_saver guard.
  }
  {
    vector<bool> vec = {true, false, true};
    assert(vec.size() == 3);
    assert(vec[1] == false);
    vec[1] = true;
    assert(std::all_of(vec.begin(), vec.end(), [](bool b) { return b; }));
  }
  {
    //bitset<3> s = {true, false, true}; // no initializer_list accepted? lame
    //  discussion is at https://groups.google.com/a/isocpp.org/forum/#!topic/std-discussion/VAFqmwOtqZo
    bitset<3> s;
    assert(s.none());
    for (int i=0; i<s.size(); ++i)
      s.set(i);
    assert(s.all());
  }
}

struct S {
  string firstName;
  string lastName;

  /*
  // whoa, I'm surprised you can decl a friend function within the class.
  // But this func here doesnt really need to be a friend, so I pull it out.
  friend ostream& operator <<(ostream& out, const S& s) {
    out << s.firstName << ' ' << s.lastName;
    return out;
  }
  */
};

ostream& operator <<(ostream& out, const S& s) {
  out << s.firstName << ' ' << s.lastName;
  return out;
}

// like std::less<T>, but for a member of T
template <class T, class MemberT>
struct lessForMember {
  //typedef MemberT T::*member_ptr_t;
  // or with C++11 'using' type alias:
  using member_ptr_t = MemberT T::*;
  lessForMember(member_ptr_t memberPtr_) : memberPtr(memberPtr_) {}
  bool operator ()(const T& lhs, const T& rhs) {
    cout << "cmp '" << lhs.*memberPtr << "' vs '" << rhs.*memberPtr << "'\n";
    return lhs.*memberPtr < rhs.*memberPtr;
  }
  private:
    member_ptr_t memberPtr;
};

void stl_play_with_sort() {
  {
    auto elems = vector<S>{
      {"Robert", "Heinlein"}, 
      {"Alfred", "Hitchcock"},
      {"Otto", "Lilienthal"}
    };
    auto lessFirstName = [](const S& lhs, const S& rhs) {
      return lhs.firstName < rhs.firstName;
    };
    auto lessLastName = [](const S& lhs, const S& rhs) {
      return lhs.lastName < rhs.lastName;
    };
    cout << "sorted by firstName:\n";
    std::sort(elems.begin(), elems.end(), lessFirstName);
    copy(elems.begin(), elems.end(), ostream_iterator<S>(cout, "\n"));

    // straightforward so far. Would be slightly less verbose if 
    // we had a lessMember<S, member_ptr<S>> that maps S to a
    // member and uses less on that. Overdesigned, just experimental:
    cout << "sorted by lastName:\n";
    std::sort(elems.begin(), elems.end(), lessForMember<S, string>(&S::lastName));
    copy(elems.begin(), elems.end(), ostream_iterator<S>(cout, "\n"));

    // Now let's say we want >=2 sort orders accessible at an given time.
    // Instead of cloning the vector<T> and sorting each vector of clones
    // we could sort a vector of ptrs or iterators:
    vector<const S*> elemsSortedByFirst; // shared_ptr or boost::intrusive_ptr would be safer
    elemsSortedByFirst.reserve(elems.size());
    std::transform(elems.begin(), elems.end(), back_inserter(elemsSortedByFirst),
      [](const S& elem) {
        return &elem;
    });
    vector<const S*> elemsSortedByLast = elemsSortedByFirst; // shallow clone of vector
    auto sortPtrByMember = [](string S::*memberPtr) {
      return [memberPtr](const S* lhs, const S* rhs) {
        return lhs->*memberPtr < rhs->*memberPtr;
      };
    };
    std::sort(elemsSortedByFirst.begin(), elemsSortedByFirst.end(), 
      sortPtrByMember(&S::firstName));
    std::sort(elemsSortedByFirst.begin(), elemsSortedByFirst.end(), 
      sortPtrByMember(&S::firstName));
    cout << "ptrs sorted by first:\n";
    for (const S* ptr : elemsSortedByFirst)
      cout << *ptr << "\n";
    cout << "ptrs sorted by last:\n";
    for (const S* ptr : elemsSortedByLast)
      cout << *ptr << "\n";
  }
}

