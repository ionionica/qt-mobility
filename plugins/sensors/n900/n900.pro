INCLUDEPATH+=../../../src/sensors

include(version.pri)
include(n900.pri)
include(../../../common.pri)

TEMPLATE = lib
CONFIG += plugin
TARGET = $$qtLibraryTarget(sensors_n900)

QT=core
CONFIG+=mobility
MOBILITY+=sensors

STRICT=$$(STRICT)
equals(STRICT,1) {
    QMAKE_CXXFLAGS+=-Werror
    QMAKE_LFLAGS+=-Wl,-no-undefined
}

DESTDIR = $$OUTPUT_DIR/bin/examples/sensors
target.path = $$SOURCE_DIR/plugins/sensors
INSTALLS += target
