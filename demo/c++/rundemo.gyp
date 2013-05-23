{
  'includes': [ 'common.gypi' ],
  'targets': [
    {
      'target_name': 'rundemo',
      'type': 'executable',
      'sources': [
        'rundemo.cpp',
      ],
      'conditions': [
        [ 'OS=="mac"', {
          'libraries': [
            '-lmapnik',
            '-undefined dynamic_lookup'
          ],
          'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS':[
               '<!@(mapnik-config --cflags)'
            ],
            'GCC_ENABLE_CPP_RTTI': 'YES',
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }],
        [ 'OS=="win"', {
            'defines': [
               '<!@(mapnik-config --defines)',
            ],
            'libraries': [
                '<!@(mapnik-config --libs)',
                 '<!@(mapnik-config --dep-libs)'
            ],
            'include_dirs': [
               '<!@(mapnik-config --includes)',
               '<!@(mapnik-config --dep-includes)',
            ],
            'msvs_settings': {
               'AdditionalLibraryDirectories': [
                  '<!@(mapnik-config --ldflags)'
               ],
            }
        }]
     ]
    }
  ],
}