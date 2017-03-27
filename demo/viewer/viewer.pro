######################################################################
# Mapnik viewer - Copyright (C) 2007 Artem Pavlenko
######################################################################
TEMPLATE = app
QT += core gui widgets
QMAKE_CXX = $$system(mapnik-config --cxx)
QMAKE_LINK = $$system(mapnik-config --cxx)
QMAKE_CXXFLAGS += $$system(mapnik-config --cxxflags)
QMAKE_CXXFLAGS += $$system(mapnik-config --includes --dep-includes --defines)
QMAKE_LFLAGS += $$system(mapnik-config --libs)
QMAKE_LFLAGS += $$system(mapnik-config --ldflags --dep-libs)
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
