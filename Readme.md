C++ playground
===

These are personal notes & experiments & cheat sheets on a few C++11 & C++14 changes.
Critical books: all the Scott Meyers ones (TODO: the STL one and the C++11 one)

Move c'tors
---

Pass by value vs pass by const ref vs pass by rvalue ref? This was pretty straightforward in C++98. 
Note that all decent compilers alrdy had RVO before C++11, still move c'tors give an important performance
boost in cases where pre-11 RVO could not be applied.
Rules of thumb for C++11:

* write a move c'tor if you want to be able to return objects by value (and the copy c'tor is 
  sufficiently expensive, e.g. the obj has containers as members)
* pass (non-flyweight) params still by const ref by default. Unless your impl makes a copy of the obj anyway,
  in which case pass by value (and use std::move?) TODO. See also
  [here](http://blogs.msdn.com/b/xiangfan/archive/2012/02/03/c-98-gt-c-11-pass-by-value-or-pass-by-reference.aspx).
* why would we ever bother pass rvalue refs outside move c'tors?
* TODO: play with std::move. Looks like its rarely needed outside of low-level libs like STL?

Example code in Move.cpp.

Lambda
---

Great, finally! Great to have with for_each and such, as well as for futures.

Concurrency
---

Rule of thumb: prefer Promise/Future abstractions (boost::future::then, which std::future sadly lacks) over the 
high-maintenance std::thread+mutx abstractions. 

Good links:
* http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism

STL
---

TODO: read Scott Meyers book on STL. 

* the C++ iterator representation via begin/end seems more error prone than the Python or JS ones via single iterator 
object that throws a StopItertion when reaching EOS, since you can accidentally use .
* is there finally some sort of 'for $x in $container' or foreach in C++11 now? Yes, according to [here](http://en.cppreference.com/w/cpp/language/range-for). Finally!
* new in C++11 std::initializer_list
* C++ equivalent of Python for x in range(begin,end)? Not in core afaik, but can add this efficiently, see 
  PythonStyleRange in ./STL.cpp.
* great STL container complexity summaries are [here](http://john-ahlgren.blogspot.com/2013/10/stl-container-performance.html) and 
  [here](http://stackoverflow.com/questions/181693/what-are-the-complexity-guarantees-of-the-standard-containers)
* functional Python-style map: STL has http://www.cplusplus.com/reference/algorithm/transform/, but sadly the
  output iterator creates an explicit sequence (or can this be circumvented?)
* C++ equivalents of LINQ or Python or lodash.js filter : I dont see a filter at http://www.cplusplus.com/reference/algorithm/,
  which is sad. Closest I find atm is http://stackoverflow.com/questions/2797142/higher-order-function-filter-in-c,
  which is inefficient compared to a functional filter in Python or JS because it returns a copy of a container with
  filtered elems, but instead I wanted filter to return a (Python-style) iterator that represents the subsequence. 
  Doesn't seem possible with STL :(
  TODO: try to impl efficient functional filter/map/reduce on top of STL to allow for code like:
  
      int result = elems.filter([](int elem) {
        return elem % 2 == 0;
      })
      .map([](int elem) {
        return elem * 10
      })
      // note Python3 demoted reduce(), lodash.js still has it
      .reduce_aka_accumulate((int accu, int elem) => { 
        return accu + elem;
      }, 0); // 0 being initial accu
  
* imo could benefit from generators, and the concept of a Python-style iterator that just has a next() and/or 
  the equivalent of StopIteration (not necessarily impl'd via pretty expensive exception handling)

STL & functional-style programming
---

Outlined here: http://www.meetingcpp.com/tl_files/mcpp/slides/12/FunctionalProgrammingInC++11.pdf. See also
STL.cpp's functional_filter_map_reduce_playground(). Out of the box STL does not allow me to do functional-style
or Python-style list/generator-programming, composing filter() and transform() and reduce/accumulate/fold() calls 
at will. If it did then this likely would make C++ STL code more readable & optimizable. But I think this could
be impl'ed as a thin layer on top of STL. TODO?

Still missing features:
---

* std::future::then anyone? Got boost::future::then only.
* it's time for generator yield in C++, sadly not in C++14 yet :( Once we have this we can use it with std::future::then
  and have async/await in C++ :)
  The closest we have to $generator.send(val) are output iterators?
* is there finally some sort of nodejs npm or python pypi / pip install for C++14? Dont think so. It's about time! It 
seems like cross-platform fetching C++ libs via pip install (matplotlib, numpy) or node is npm install (karma) 
simpler than (cross-platform!) building of C++ libs via C++ tools.
