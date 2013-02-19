# -------------------------------------------------
# QtCreator Project For Mapnik
# -------------------------------------------------
QT =

TARGET = mapnik
TEMPLATE = lib

INCLUDEPATH = \
    ../deps/agg/include \
    ../include \
    /usr/lib/oracle/11.2.0.3/client/include

include(All.files)

OTHER_FILES += \
    ../SConstruct \
    ../config.py \
    ../CHANGELOG.md

unix {
    DEFINES += LINUX=1 MAPNIK_LOG=1
}
