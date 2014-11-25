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
http://google-styleguide.googlecode.com/svn/trunk/cppguide.html recommends explicit lambda captures, and
to not used default ones at all.

Auto
---
Quote http://google-styleguide.googlecode.com/svn/trunk/cppguide.html: "Programmers have to 
understand the difference between auto and const auto& or they'll get copies when they didn't mean to."
When do I get a copy?


Initializer lists
---

Not available in VS2012 yet. See http://google-styleguide.googlecode.com/svn/trunk/cppguide.html for examples,
as well as http://en.cppreference.com/w/cpp/utility/initializer_list.

Concurrency
---

Rule of thumb: prefer Promise/Future abstractions (boost::future::then, which std::future sadly lacks) over the 
high-maintenance std::thread+mutx abstractions. 

Good links:
* http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism

STL
---

* the C++ iterator representation via begin/end seems more error prone than the Python or JS ones via single iterator 
  object that throws a StopItertion when reaching EOS, since you could accidentally compare x.begin() with y.end().
  I've seen sequence<T> with (c)begin() (c)end() in some code.
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

I/O libs:
---

* 0MQ aka [ZeroMQ](http://zeromq.org/) has bindings for multiple platforms, e.g. nodejs
* boost::asio, unsure about interop with other langugages

Random links:
* [Going Native 2013](http://channel9.msdn.com/Events/GoingNative/2013)
* [Going Native 2012](http://channel9.msdn.com/Events/GoingNative/GoingNative-2012)
* [Google C++ Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.html): their EH stance
  surprised me, they seem to have a lot of non-exception-safe code still. One of the few things that I 
  disagreed with was the 'and an excessively "functional" style of programming' comment. Some example
  would have been helpful to illustrate this excessiveness.
* https://ascend4nt.wordpress.com/2012/12/28/c-11-must-have-books/
* [CERT C++ Coding Standard](https://www.securecoding.cert.org/confluence/pages/viewpage.action?pageId=637)
* http://vimeo.com/97318797 as a preview to the (as of end 2014) upcoming C++11/14 Scott Meyers book

Dependency Hell: C++ package managers
---

Python has pip/pypi, nodejs has npm, why does C++ not have an agreed upon dependency 
manager yet? Some links:

* http://www.reddit.com/r/cpp/comments/2eiulw/dependency_manager_for_ccmake_projects/
* http://meetingcpp.com/index.php/vl14/items/21.html (only a summary as of Nov 2014)
* semi-dead: https://github.com/iauns/cpm - what killed it?
* semi-dead: pulling C++ libs thru 
  [nuget: GoingNative 2013](http://channel9.msdn.com/Events/GoingNative/2013/Find-Build-Share-Use-Using-NuGet-for-C-and-Cpp-Libraries).
  I liked that he reused an existing repo (nuget's), but the choice of nuget probably killed it 
  as a potential cross-platform solution. 
* the only working dep mgr I found is [node-gyp](https://github.com/TooTallNate/node-gyp)
* we use C++ libs stored in a local Artifactory and downloaded via ivy.xml, but this
  won't scale across companies and pushing the 3rd party C++ libs into Artifactory 
  is still labor-intensive & awkward.

C++ crossplatform builds
---

* I like gyp: generates Makefiles or vcproj from a pleasant looking json-ish *.gyp file
* autoconf: it took me 30+ mins to set up a configure.ac Makefile.am for a helloword 
  project. Dislike at first sight. I don't like the archaic looking m4 either. Though
  admittedly for library consumers this is easy to use with ./configure and make. At least
  on Linux, for Windows users this is not as straightforward.
* CMake seems to be cross-platform also, did not look as a appealing to me as gyp though
  at first glance.

Still missing features:
---

* async:
    - std::future::then anyone? Got boost::future::then only.
    - executors/schedulers: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3731.pdf. 
      There are plenty of variants for this in the making, e.g. https://github.com/chriskohlhoff/executors
      from the bost::asio author (see also https://www.youtube.com/watch?v=D-lTwGJRx0o)
* it's time for generator yield in C++, sadly not in C++14 yet :( Once we have this we can use it with std::future::then
  and have async/await in C++ :) Some links on this:
    - I like the generator remarks and discussion of eager vs lazy eval oin STL transform
      at [this blog entry](http://paoloseverini.wordpress.com/2014/06/09/generator-functions-in-c/), 
      continued here: http://isocpp.org/blog/2014/06/generator-functions
    - [boost::coroutine](http://www.boost.org/doc/libs/1_57_0/libs/coroutine/doc/html/index.html) can
      impl coroutines with standard C++11?! Built on Duff's Device?
    - standardization attempts are [here](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3708.pdf), sadly
      without a yield language keyword. P.S.: the newer version of this looks much better: 
      http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2014/n4134.pdf. This now resembles
      Python asyncio more than earlier drafts had. I still wish the C++ impl could get away with 
      just yield or 'yield from', without needing the extra 'await' operator. How could we write
      the event loop then though? In duck-typed languages (Python & nodejs) writing the event loop 
      that consumes the generator that produced the futures and sends back in the resolved values
      (or exceptions) is pretty straighforward, but in type-safe C++ we would need the event loop
      to consume future<T> and send back in the T, with the T being variable for most generators
      (e.g. a generator yielding first future<shared_ptr<stream>> and then future<int> while
      finally returning a string).
      In other words the boost::coroutine would have to be pull_type=future<T> and push_type=T.
      Unless we go for a non-type-safe & ugly future<variant> solution. 
