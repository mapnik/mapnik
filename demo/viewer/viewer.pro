######################################################################
# Mapnik viewer - Copyright (C) 2007 Artem Pavlenko
######################################################################
CC = g++
TEMPLATE = app

INCLUDEPATH += /opt/mapnik/include
INCLUDEPATH += /opt/boost/include/boost-1_34_1
INCLUDEPATH += /usr/local/include/freetype2
INCLUDEPATH += /Users/artem/projects/mapnik/agg/include
INCLUDEPATH += .

QMAKE_CXXFLAGS +=' -DDARWIN'
unix:LIBS =  -L/opt/mapnik/lib -lmapnik -lfreetype

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
