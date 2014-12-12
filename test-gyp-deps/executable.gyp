{
    "targets": [
        {
           'target_name': 'play',
            'type': 'executable',
            'sources': [
                'src/*.cpp'
            ],
            'dependencies': [
                './dependencies/lib1/lib1.gyp:lib1'
            ],
            #'libraries': []
        }
    ]
}