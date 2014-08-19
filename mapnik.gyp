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
      "conditions": [
        ["OS=='win'", {
           'libraries':[
              'libboost_filesystem-vc120-mt-1_55.lib',
              'libboost_regex-vc120-mt-1_55.lib',
              'libboost_thread-vc120-mt-1_55.lib',
              'libboost_system-vc120-mt-1_55.lib',
              'libpng.lib',
              'proj.lib',
              'libtiff.lib',
              'libwebp.lib',
              'libxml2.lib',
              'libjpeg.lib',
              'icuuc.lib',
              'freetype.lib',
              'zlib.lib'
          ]
        },{
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
            ]
          }
        ]
      ],
      'include_dirs':[
          './include', # mapnik
          './deps/', # mapnik/sparsehash
          './deps/agg/include/', # agg
          './deps/clipper/include/', # clipper
          'C:/dev2/mapnik-dependencies/boost-55-vc120/include/boost-1_55/', # boost main
          '.', #boost within mapnik
          'C:/dev2/mapnik-dependencies/icu/include/', #icu
          'C:/dev2/mapnik-dependencies/freetype/include/', #freetype
          'C:/dev2/mapnik-dependencies/jpeg/', #jpg
          'C:/dev2/mapnik-dependencies/zlib/', #zlib
          'C:/dev2/mapnik-dependencies/postgresql/src/include/port/win32/', #dlfcn.h
          'C:/dev2/mapnik-dependencies/libpng/', #libpng png.h
          'C:/dev2/mapnik-dependencies/tiff/libtiff/', #libtiff tiffio.h
          'C:/dev2/mapnik-dependencies/webp/include/', #webp types.h
          'C:/dev2/mapnik-dependencies/libxml2/include/', #libxml xmlreader.h
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          './include', # mapnik
          './deps/', # mapnik/sparsehash
          './deps/agg/include/', # agg
          './deps/clipper/include/', # clipper
          'C:/dev2/mapnik-dependencies/boost-55-vc120/include/boost-1_55/', # boost main
          '.', #boost within mapnik
          'C:/dev2/mapnik-dependencies/icu/include/', #icu
          'C:/dev2/mapnik-dependencies/freetype/include/', #freetype
          'C:/dev2/mapnik-dependencies/jpeg/', #jpg
          'C:/dev2/mapnik-dependencies/zlib', #zlib
          'C:/dev2/mapnik-dependencies/postgresql/src/include/port/win32/', #dlfcn.h
          'C:/dev2/mapnik-dependencies/libpng/', #libpng png.h
          'C:/dev2/mapnik-dependencies/tiff/libtiff/', #libtiff tiffio.h
          'C:/dev2/mapnik-dependencies/webp/include/', #webp types.h
          'C:/dev2/mapnik-dependencies/libxml2/include/', #libxml xmlreader.h
        ],
        'libraries': ['-stdlib=libstdc++']
      },
      'cflags': [
      ]
    },
    {
        "target_name": "shape",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/shape/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "csv",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/csv/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ]
    },
    {
        "target_name": "ogr",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/ogr/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ],
        'conditions': [
          ['OS=="win"', {
            'libraries': [ 'C:/dev2/mapnik-dependencies/gdal/gdal/gdal111.dll', 'C:/Expat2.1.0/Bin/libexpat.dll']
          } , {
            'libraries': [ '<!@(gdal-config --libs)', '<!@(gdal-config --dep-libs)']
          }]
        ]
    },
    {
        "target_name": "gdal",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/gdal/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ],
        'conditions': [
          ['OS=="win"', {
            'libraries': [ 'C:/dev2/mapnik-dependencies/gdal/gdal/gdal111.dll', 'C:/Expat2.1.0/Bin/libexpat.dll']
          } , {
            'libraries': [ '<!@(gdal-config --libs)', '<!@(gdal-config --dep-libs)']
          }]
        ]
    },
    {
        "target_name": "postgis",
        "type": "loadable_module",
        "product_extension": "input",
        "sources": [ '<!@(find plugins/input/postgis/ -name "*.cpp")' ],
        "dependencies": [ "mapnik" ],
        'conditions': [
          ['OS=="win"', {
            'libraries': [ 'C:/dev2/mapnik-dependencies/postgresql/src/interfaces/libpq/Release/libpq.dll']
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
            'libraries': [ 'C:/dev2/mapnik-dependencies/sqlite/sqlite3.dll']
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
        "dependencies": [ "mapnik" ]
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
        "dependencies": [ "mapnik" ]
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
  ]
}