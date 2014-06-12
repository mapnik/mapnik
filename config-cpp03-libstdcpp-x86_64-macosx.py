# cpp03-libstdcpp-x86_64-macosx
#
# Forcing stdlib is the key for this platform.

CUSTOM_CXXFLAGS = '-fvisibility-inlines-hidden -DU_CHARSET_IS_UTF8=1 -stdlib=libstdc++'
CUSTOM_LDFLAGS = '-stdlib=libstdc++'
RUNTIME_LINK = 'static'
INPUT_PLUGINS = 'csv,gdal,geojson,occi,ogr,osm,postgis,python,raster,rasterlite,shape,sqlite'
WARNING_CXXFLAGS = '-Wno-deprecated-register -Wno-redeclared-class-member'
PATH = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/bin/'
BOOST_INCLUDES = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/include/boost'
BOOST_LIBS = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/lib'
BOOST_PYTHON_LIB = 'boost_python-2.7'
FREETYPE_CONFIG = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/bin/freetype-config'
ICU_INCLUDES = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/include'
ICU_LIBS = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/lib'
PNG_INCLUDES = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/include'
PNG_LIBS = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/lib'
JPEG_INCLUDES = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/include'
JPEG_LIBS = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/lib'
TIFF_INCLUDES = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/include'
TIFF_LIBS = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/lib'
PROJ_INCLUDES = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/include'
PROJ_LIBS = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/lib'
BENCHMARK = True
CAIRO_INCLUDES = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/include'
CAIRO_LIBS = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/lib'
SQLITE_INCLUDES = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/include'
SQLITE_LIBS = '../mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/lib'
FRAMEWORK_PYTHON = False
BINDINGS = 'python'
XMLPARSER = 'ptree'

