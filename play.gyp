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
            'cflags': [ '-g', '-O0', '-std=c++11' ],
            'sources': [
                'src/cpp11.cpp',
                'src/main.cpp',
                'src/move.cpp',
                'src/stl.cpp',
                'src/threading.cpp',
                'src/variadic_template_func.cpp',
            ],
            'libraries': [
                '-lpthread'
            ],
    },
    ],
}
