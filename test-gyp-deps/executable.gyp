{
    "targets": [
        {
           'target_name': 'play',
            'type': 'executable',
            'sources': [
                'src/*.cpp'
            ],
            'dependencies': [
                './dependencies/lib1/lib1.gyp:lib1',
                './dependencies/lib2/lib2.gyp:lib2'
            ],
            #'libraries': []
        }
    ]
}