{
  'includes': [ 'common.gypi' ],
  'include_dirs': [
     '<!@(mapnik-config --includes)',
     '<!@(mapnik-config --dep-includes)',
  ],
  'defines': [
     '<!@(mapnik-config --defines)',
  ],
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