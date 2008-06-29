######################################################################
# Mapnik viewer - Copyright (C) 2007 Artem Pavlenko
######################################################################
CC = g++
TEMPLATE = app

INCLUDEPATH += /opt/mapnik/include
INCLUDEPATH += /opt/boost-trunk/include/boost-1_35
INCLUDEPATH += /usr/X11/include
INCLUDEPATH += .

QMAKE_CXXFLAGS +=' -DDARWIN'
unix:LIBS =  -L/opt/mapnik/lib -L/opt/boost-trunk/lib -L/usr/X11/lib -lmapnik -lfreetype -licuuc -licudata -lboost_regex-mt-1_35 -lboost_filesystem-mt-1_35 -lboost_thread-mt-1_35 -lboost_system-mt-1_35 -lboost_program_options-mt-1_35

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
           styles_model.hpp \ 
           renderthread.hpp

HEADERS += about_dialog.hpp \
           info_dialog.hpp \
           layer_info_dialog.hpp

SOURCES += main.cpp \
           mainwindow.cpp \ 
           mapwidget.cpp \
           layerwidget.cpp \
           layerlistmodel.cpp \ 
           layerdelegate.cpp \
           styles_model.cpp \
           renderthread.cpp

SOURCES += about_dialog.cpp \
           info_dialog.cpp \
           layer_info_dialog.cpp

RESOURCES += mapnik_viewer.qrc
