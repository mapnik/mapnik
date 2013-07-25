######################################################################
# Mapnik viewer - Copyright (C) 2007 Artem Pavlenko
######################################################################
TEMPLATE = app
QT += core gui widgets
QT += opengl

QMAKE_CXX = $$system(mapnik-config --cxx)
QMAKE_LINK = $$system(mapnik-config --cxx)
QMAKE_CXXFLAGS += $$system(mapnik-config --cxxflags --defines)
QMAKE_CXXFLAGS += $$system(mapnik-config --includes --dep-includes)
QMAKE_CXXFLAGS += "-I/Users/artem/Projects/skia/trunk/include/gpu"
QMAKE_CXXFLAGS += "-I/Users/artem/Projects/skia/trunk/include/pdf"
QMAKE_CXXFLAGS += "-I/Users/artem/Projects/skia/trunk/src/gpu"

QMAKE_LFLAGS += $$system(mapnik-config --libs)
QMAKE_LFLAGS += $$system(mapnik-config --ldflags --dep-libs)
QMAKE_LFLAGS += -lboost_timer
QMAKE_LFLAGS += -lskia_pdf
QMAKE_LFLAGS += -lskia_skgpu
QMAKE_LFLAGS += -lzlib
QMAKE_LFLAGS += "-framework Cocoa"

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
