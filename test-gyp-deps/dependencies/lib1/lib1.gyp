{
    "targets": [
        {
           'target_name': 'lib1',
            'type': 'static_library',
            'sources': [
                'src/*.cpp'
            ],
            'include_dirs': [
                'include/'
            ],
            'dependencies': [
                '../../dependencies/lib2/lib2.gyp:lib2'
            ],
            'all_dependent_settings': {
                'defines': [
                    'SOME_LIB1_DEFINE=77',
                    'OTHER_LIB1_DEFINE'
                ],
                'include_dirs': [
                    'include/'
                ]
            }
        }
    ]
}