{
  'includes': [
    './common.gypi'
  ],
  'variables': {
    'includes%':'',
    'libs%':'',
    'common_defines': [
      'U_CHARSET_IS_UTF8=1',
      'BIGINT',
      'BOOST_REGEX_HAS_ICU',
      'HAVE_JPEG',
      'MAPNIK_USE_PROJ4',
      'HAVE_PNG',
      'HAVE_TIFF',
      'HAVE_WEBP',
      'MAPNIK_THREADSAFE',
      'HAVE_CAIRO',
      'GRID_RENDERER',
      'SVG_RENDERER'
    ],
    'common_includes': [
      './include', # mapnik
      './deps/', # mapnik/sparsehash
      './deps/agg/include/', # agg
      './deps/clipper/include/', # clipper
      './', # boost shim
      '<@(includes)/',
      '<@(includes)/freetype2',
      '<@(includes)/libxml2',
      '<@(includes)/cairo'
    ],
    "conditions": [
      ["OS=='win'", {
          'common_defines': ['LIBXML_STATIC'], # static libxml: libxml2_a.lib
          'common_libraries': []
      }, {
          'common_defines': ['SHAPE_MEMORY_MAPPED_FILE'],
          'common_libraries': [
            '-Wl,-search_paths_first',
            '-stdlib=libstdc++',
            '-L<@(libs)'
          ]
      }]
    ]
  },
  'targets': [
    {
      'target_name': 'mapnik',
      'product_name': 'mapnik',
      'type': 'shared_library',
      'sources': [
        '<!@(find deps/agg/src/ -name "*.cpp")',
        '<!@(find deps/clipper/src/ -name "*.cpp")',
        '<!@(find src -name "*.cpp")'
      ],
      'xcode_settings': {
        'SDKROOT': 'macosx',
        'SUPPORTED_PLATFORMS':['macosx'],
        'PUBLIC_HEADERS_FOLDER_PATH': 'include',
        'OTHER_CPLUSPLUSFLAGS':[
          '-ftemplate-depth-300'
        ]
      },
      'msvs_settings': {
        'VCLinkerTool': {
          'AdditionalLibraryDirectories': [
              '<@(libs)/'
          ]
        }
      },
      'defines': [
        '<@(common_defines)'
      ],
      'libraries': [
        '<@(common_libraries)'
      ],
      "conditions": [
        ["OS=='win'", {
		   'defines': ['MAPNIK_EXPORTS'],
           'libraries':[
              'libboost_filesystem-vc120-mt-1_56.lib',
              'libboost_regex-vc120-mt-1_56.lib',
              'libboost_thread-vc120-mt-1_56.lib',
              'libboost_system-vc120-mt-1_56.lib',
              'libpng16.lib',
              'proj.lib',
              'libtiff.lib',
              'libwebp.lib',
              #'libxml2.lib', #dynamic
              'libxml2_a.lib', #static
              # needed if libxml2 is static
              'ws2_32.lib',
              'libjpeg.lib',
              'icuuc.lib',
              'icuin.lib',
              'freetype.lib',
              'zlib.lib',
              'cairo.lib'
          ]
        },{
            'libraries':[
              '-lboost_filesystem',
              '-lboost_regex',
              '-lboost_thread',
              '-lboost_system',
              '-lcairo',
              '-lpixman-1',
              '-lexpat',
              '-lpng',
              '-lproj',
              '-ltiff',
              '-lwebp',
              '-lxml2',
              '-licui18n',
              '-ljpeg',
              '-licuuc',
              '-lfreetype',
              '-licudata',
              '-lz'
            ]
          }
        ]
      ],
      'include_dirs':[
          '<@(common_includes)'
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<@(common_includes)'
        ],
        'defines': [
          '<@(common_defines)'
        ],
        'libraries':[
          '<@(common_libraries)'
        ],
        'msvs_settings': {
          'VCLinkerTool': {
            'AdditionalLibraryDirectories': [
                '<@(libs)/'
            ]
          }
        }
      }
    },
    {
        "target_name": "shape",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/shape/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ],
        "conditions": [
          ["OS=='win'", {
             'libraries':[
                'libboost_thread-vc120-mt-1_56.lib',
                'libboost_system-vc120-mt-1_56.lib',
                'icuuc.lib'
            ],
          },{
              'libraries':[
                '-lboost_thread'
              ]
            }
          ]
        ]
    },
    {
        "target_name": "csv",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/csv/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ],
        "conditions": [
          ["OS=='win'", {
             'libraries':[
                'libboost_thread-vc120-mt-1_56.lib',
                'libboost_system-vc120-mt-1_56.lib',
                'icuuc.lib'
            ],
          },{
              'libraries':[
                '-lboost_thread'
              ]
            }
          ]
        ]
    },
    {
        "target_name": "raster",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/raster/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ]
    },
 #   {
 #       "target_name": "gdal",
 #       "type": "loadable_module",
 #       "product_extension": "input",
 #       "sources": [ '<!@(find plugins/input/gdal/ -name "*.cpp")' ],
 #       "dependencies": [ "mapnik" ],
 #       'conditions': [
 #         ['OS=="win"', {
 #           'libraries': [
#                'gdal111.dll',
#                'libexpat.dll',
#                'libboost_thread-vc120-mt-1_56.lib',
#                'libboost_system-vc120-mt-1_56.lib',
#                'icuuc.lib'
#            ]
#          } , {
#            'libraries': [ '<!@(gdal-config --libs)', '<!@(gdal-config --dep-libs)']
#          }]
#       ]
#   },
    {
        "target_name": "postgis",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/postgis/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ],
        'conditions': [
          ['OS=="win"', {
            'libraries': [
                'libpq.lib',
                'wsock32.lib',
                'advapi32.lib',
                'shfolder.lib',
                'secur32.lib',
                'icuuc.lib',
                'ws2_32.lib',
            ],
            # TODO - ideally we find a way to avoid this
            # currently needed to dodge double-linking
            'msvs_settings': {
              'VCLinkerTool': {
                'AdditionalOptions': [
                    '/NODEFAULTLIB:libcmt.lib'
                ],
              }
            }
          } , {
            'libraries': [ '<!@(pkg-config libpq --libs --static)']
          }]
        ]
    },
    {
        "target_name": "pgraster",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/pgraster/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ],
        'conditions': [
          ['OS=="win"', {
            'libraries': [
                'libpq.lib',
                'wsock32.lib',
                'advapi32.lib',
                'shfolder.lib',
                'secur32.lib',
                'icuuc.lib',
                'ws2_32.lib',
            ],
            # TODO - ideally we find a way to avoid this
            # currently needed to dodge double-linking
            'msvs_settings': {
              'VCLinkerTool': {
                'AdditionalOptions': [
                    '/NODEFAULTLIB:libcmt.lib'
                ],
              }
            }
          } , {
            'libraries': [ '<!@(pkg-config libpq --libs --static)']
          }]
        ]
    },
    {
        "target_name": "sqlite",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/sqlite/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ],
        'conditions': [
          ['OS=="win"', {
            'libraries': [
                  'sqlite3.lib',
                  'icuuc.lib',
            ]
          } , {
            'libraries': [ '<!@(pkg-config sqlite3 --libs)']
          }]
        ]
    },
    {
        "target_name": "geojson",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/geojson/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ],
        'conditions': [
          ['OS=="win"', {
            'libraries': [
                'icuuc.lib',
            ]
          }]
        ]
    },
    {
        "target_name": "agg_blend_src_over_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/agg_blend_src_over_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "clipping_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/clipping_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "conversions_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/conversions_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "exceptions_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/exceptions_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "font_registration_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/font_registration_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "fontset_runtime_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/fontset_runtime_test.cpp"],
        "dependencies": [ "mapnik" ],
        "conditions": [
          ["OS=='win'", {
             'libraries':[
                'icuuc.lib'
            ],
          }]
        ]
    },
    {
        "target_name": "geometry_converters_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/geometry_converters_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "image_io_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/image_io_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "label_algo_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/label_algo_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "map_request_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/map_request_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "params_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/params_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "wkb_formats_test",
        "type": "executable",
        "sources": [ "./tests/cpp_tests/wkb_formats_test.cpp"],
        "dependencies": [ "mapnik" ]
    },
  ],
  'conditions': [
    # won't link yet on windows
    ["OS!='win'", {
       'targets': [
       {
           "target_name": "test_rendering",
           "type": "executable",
           "sources": [ "./benchmark/test_rendering.cpp" ],
           "dependencies": [ "mapnik" ],
           "conditions": [
             ["OS=='win'", {
                'libraries':[
                   'libboost_thread-vc120-mt-1_56.lib'
               ],
               'defines': ['MAPNIK_EXPORTS']
             },{
                 'libraries':[
                   '-lboost_thread'
                 ]
               }
             ]
           ]
       }
    ]}
    ]
  ]
}