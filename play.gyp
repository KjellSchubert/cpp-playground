# usage (run on Ubuntu 14.04 clang 3.5, partially on VS2012) :
#  > gyp play.gyp --depth=.
#    How to get rid of the --depth?
#  > make
#  > out/Default/play
{
    'make_global_settings': [
      ['CXX','/usr/bin/clang++'],
      ['LINK','/usr/bin/clang++'],
      # doesnt work: ['OTHER_CPLUSPLUSFLAGS', '-stdlib=libc++'],
    ],

    'targets': [
    {
        'target_name': 'play',
            'type': 'executable',
            'cflags': [ 
               '-g', 
               '-O0',

               # optionally pick a sanitizer:
               # Note this requires 'uname -p' == x86_64, see 
               # http://llvm.org/releases/3.5.0/tools/clang/docs/MemorySanitizer.html
               #'-fsanitize=memory', 

               # pick either standard (#if will exclude features from compile):
               #'-std=c++11',
               #'-std=c++14', # should have worked for clang 3.5 but didnt
               '-std=c++1y',
               #'-std=c++1z',
              
               # to use clang's libc++ instead of the gcc default (for <codecvt> mostly):
               '-stdlib=libc++'
            ],
            'sources': [
                'src/cpp11.cpp',
                'src/locale.cpp',
                'src/main.cpp',
                'src/move.cpp',
                'src/stl.cpp',
                'src/mpl.cpp',
                'src/threading.cpp',
                'src/variadic_template_func.cpp',
            ],
            'libraries': [
                '-lpthread',
                '-nodefaultlibs -lc++ -lm -lc -lgcc_s -lgcc', # if using -stdlib=libc++
            ],
    },
    ],
}
