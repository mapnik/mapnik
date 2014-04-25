{
  'includes': [
    './common.gypi',
    './config.gypi',
  ],
  'targets': [
    {
      'target_name': 'mapnik',
      'product_name': 'mapnik',
      'type': 'shared_library',
      'sources': [
        '<!@(find deps/agg/src/ -name "*.cpp")',
        '<!@(find deps/clipper/src/ -name "*.cpp")',
        '<!@(find src -name "*.cpp")',
        '<!@(find include -name "*.hpp")'
      ],
      'xcode_settings': {
        'SDKROOT': 'macosx',
        'SUPPORTED_PLATFORMS':['macosx'],
        'PUBLIC_HEADERS_FOLDER_PATH': 'include',
        'OTHER_CPLUSPLUSFLAGS':[
          '-ftemplate-depth-300',
          '-fvisibility-inlines-hidden'
        ]
      },
      'libraries':[
        '-lboost_filesystem',
        '-lboost_regex',
        '-lboost_thread',
        '-lcairo',
        '-lpixman-1',
        '-lexpat',
        '-lfontconfig',
        '-lpng',
        '-lproj',
        '-ltiff',
        '-lwebp',
        '-lxml2',
        '-licui18n',
        '-lboost_system',
        '-ljpeg',
        '-licuuc',
        '-lfreetype',
        '-licudata',
        '-lz',
        '-Wl,-search_paths_first',
        '-stdlib=libstdc++'
      ],
      'include_dirs':[
          './include', # mapnik
          './deps/', # mapnik/sparsehash
          './deps/agg/include/', # agg
          './deps/clipper/include/', # clipper
          './' # boost
      ],
      'cflags': [
      ]
    }
  ]
}