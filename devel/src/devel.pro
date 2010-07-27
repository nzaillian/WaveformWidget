# -------------------------------------------------
# Project created by QtCreator 2010-05-17T19:19:24
# -------------------------------------------------
TARGET = WaveformViewerDevel
TEMPLATE = app
INCLUDEPATH += /usr/include
SOURCES += main.cpp \
    mainwindow.cpp \
    ../../src/WaveformWidget.cpp \
    ../../src/AudioUtil.cpp
HEADERS += mainwindow.h \
    ../../src/MathUtil.h \
    ../../src/AudioUtil.h \
    ../../src/WaveformWidget.h \
    ../../src/AudioUtil.h
LIBS += -lsndfile \
    -L/usr/lib
