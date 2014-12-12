This toy project shows how gyp deals with library dependencies: to test run

>gyp play.gyp --depth=.
>make

which will also build the upstream module 'lib1'. See the define in lib1.gyp
which are inherited by downstream project 'executable'. So much simpler than
writing makefiles directly...

See also [gyp lang spec](https://code.google.com/p/gyp/wiki/GypLanguageSpecification)
which only had one minor surprise for me, with the actual syntax for 'defines' 
differing from the gyp doc (clarified in Sep 13, 2012 comment).