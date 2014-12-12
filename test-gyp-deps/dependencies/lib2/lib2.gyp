{
    "targets": [
        {
           'target_name': 'lib2',
            'type': 'none', # it's #include-only, so not a static_library
            'all_dependent_settings': {
                'include_dirs': [
                    'include/'
                ]
            }
        }
    ]
}