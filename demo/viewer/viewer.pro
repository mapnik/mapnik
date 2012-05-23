######################################################################
# Mapnik viewer - Copyright (C) 2007 Artem Pavlenko
######################################################################
QMAKE_CXX = /opt/llvm/bin/clang++
TEMPLATE = app

INCLUDEPATH += /opt/mapnik/include/
INCLUDEPATH += /opt/boost_1_49_0/include/
INCLUDEPATH += /opt/mapnik_deps/include/
INCLUDEPATH += /opt/mapnik_deps/include/freetype2

### Cairo backend
INCLUDEPATH += /opt/mapnik_deps/include/cairo
INCLUDEPATH += /opt/mapnik_deps/include/cairomm-1.0
INCLUDEPATH += /opt/mapnik_deps/include/sigc++-2.0
INCLUDEPATH += /opt/mapnik_deps/lib/sigc++-2.0/include
###

INCLUDEPATH += /Users/artem/Projects/mapnik/deps/agg/include
INCLUDEPATH += .

QMAKE_CXXFLAGS +='-DDARWIN -Wno-missing-field-initializers -fno-inline -ansi'
QMAKE_CXXFLAGS +=' -DHAVE_CAIRO'
unix:LIBS =  -L/opt/mapnik/lib -L/opt/mapnik_deps/lib -lmapnik -lfreetype -L/opt/mapnik/lib 
unix:LIBS += -lboost_system -licuuc -lboost_filesystem -lboost_regex -L/opt/boost_1_49_0/lib
unix:LIBS += -lcairo -lcairomm-1.0

# Input

CONFIG += qt debug_and_release
FORMS += forms/about.ui \
         forms/info.ui \
         forms/layer_info.ui

HEADERS += mainwindow.hpp \
           mapwidget.hpp \
           layerwidget.hpp \
           layerlistmodel.hpp \
           layerdelegate.hpp \
           styles_model.hpp 

HEADERS += about_dialog.hpp \
           info_dialog.hpp \
           layer_info_dialog.hpp

SOURCES += main.cpp \
           mainwindow.cpp \ 
           mapwidget.cpp \
           layerwidget.cpp \
           layerlistmodel.cpp \ 
           layerdelegate.cpp \
           styles_model.cpp

SOURCES += about_dialog.cpp \
           info_dialog.cpp \
           layer_info_dialog.cpp

RESOURCES += mapnik_viewer.qrc
