TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
        main.cpp \

INCLUDEPATH += $$PWD/../../external/common/include
INCLUDEPATH += $$PWD/../../external/libconfig/include
INCLUDEPATH += $$PWD/..


LIBS += -L$$PWD/lib64 -lpthread

LIBS += -L$$PWD/../../external/common/lib -lcommontools
LIBS += -L$$PWD/../../external/libconfig/lib -lconfig++
LIBS += -L$$PWD/../../external/ctp/lib -lthosttraderapi

