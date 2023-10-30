######################################################################
# Mapnik viewer - Copyright (C) 2007 Artem Pavlenko
######################################################################
TEMPLATE = app
QT += core gui widgets
#LIBS += $$system(mapnik-config --libs --dep-libs)
#QMAKE_CXX = $$system(mapnik-config --cxx)
#QMAKE_LINK = $$system(mapnik-config --cxx)
#QMAKE_CXXFLAGS += $$system(mapnik-config --cxxflags)
#QMAKE_CXXFLAGS += $$system(mapnik-config --includes --dep-includes --defines)
#QMAKE_LFLAGS += $$system(mapnik-config --ldflags)

#LIBS += -L/usr/local/lib -lmapnik -lboost_filesystem -lboost_regex -lcairo -lpng -lproj -lsqlite3 -ltiff -lwebp -licui18n -lboost_system -lharfbuzz -ljpeg -licuuc -lfreetype -lz
LIBS += -L/usr/local/lib -lmapnik -lboost_filesystem -lboost_regex -lcairo -lpng -lproj -lsqlite3 -ltiff -lwebp -licui18n -lboost_system -lharfbuzz -ljpeg -licuuc -lfreetype -lz
QMAKE_CXX = c++
QMAKE_LINK =  c++
QMAKE_CXXFLAGS += -std=c++14 -DU_USING_ICU_NAMESPACE=0 -stdlib=libc++ -fvisibility=hidden -fvisibility-inlines-hidden -ftemplate-depth-300 -O3
QMAKE_CXXFLAGS += -I/usr/local/include -I/usr/local/include/mapnik/agg -I/usr/local/include/mapnik/deps -I/opt/homebrew/opt/sqlite3/include -I/opt/homebrew/opt/icu4c/include -I/opt/homebrew/opt/harfbuzz/include -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/opt/jpeg/include -I/opt/homebrew/opt/proj/include -I/opt/homebrew/opt/libpng/include -I/opt/homebrew/opt/webp/include -I/opt/homebrew/opt/libtiff/include -I/opt/homebrew/opt/boost/include -I/opt/homebrew/Cellar/gdal/3.5.0/include -I/opt/homebrew/include -I/opt/homebrew/opt/cairo/include/cairo -I/opt/homebrew/opt/cairo/include/pixman-1 -DMAPNIK_MEMORY_MAPPED_FILE -DMAPNIK_HAS_DLCFN -DBIGINT -DBOOST_REGEX_HAS_ICU -DHAVE_JPEG -DMAPNIK_USE_PROJ -DMAPNIK_PROJ_VERSION=90000 -DHAVE_PNG -DHAVE_WEBP -DHAVE_TIFF -DDARWIN -DMAPNIK_THREADSAFE -DBOOST_SPIRIT_NO_PREDEFINED_TERMINALS=1 -DBOOST_PHOENIX_NO_PREDEFINED_TERMINALS=1 -DBOOST_SPIRIT_USE_PHOENIX_V3=1 -DNDEBUG -DHAVE_CAIRO -DGRID_RENDERER
QMAKE_LFLAGS += -L/opt/homebrew/opt/sqlite3/lib -L/opt/homebrew/opt/icu4c/lib -L/opt/homebrew/opt/harfbuzz/lib -L/opt/homebrew/opt/freetype -L/opt/homebrew/opt/jpeg/lib -L/opt/homebrew/opt/proj/lib -L/opt/homebrew/opt/libpng/lib -L/opt/homebrew/opt/webp/lib -L/opt/homebrew/opt/libtiff/lib -L/opt/homebrew/opt/boost/lib -L/opt/homebrew/Cellar/gdal/3.5.0/lib -L/opt/homebrew/lib -stdlib=libc++


# Input

CONFIG += qt debug_and_release
#FORMS += forms/about.ui \
#         forms/info.ui \
#         forms/layer_info.ui

HEADERS += mainwindow.hpp \
           ThreadPool.h \
           mapwidget.hpp \
           roadmerger.h \
           waitingspinnerwidget.h

SOURCES += main.cpp \
           mainwindow.cpp \
           mapwidget.cpp \
           roadmerger.cpp \
           waitingspinnerwidget.cpp

RESOURCES += mapnik_viewer.qrc \
#    mapviewer.qrc
