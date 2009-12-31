######################################################################
# Mapnik viewer - Copyright (C) 2007 Artem Pavlenko
######################################################################
CC = g++
TEMPLATE = app

INCLUDEPATH += /opt/mapnik/include
INCLUDEPATH += /opt/boost/include/boost-1_39
INCLUDEPATH += /usr/X11/include/
INCLUDEPATH += /usr/X11/include/freetype2
INCLUDEPATH += .

QMAKE_CXXFLAGS +=' -DDARWIN -Wno-missing-field-initializers'
unix:LIBS =  -L/opt/mapnik/lib -L/usr/X11/lib -lmapnik -lfreetype -L/usr/local/lib -licuuc
unix:LIBS += -lboost_system -lboost_filesystem -lboost_regex -L/opt/boost/lib

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
