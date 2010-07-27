# -------------------------------------------------
# Project created by QtCreator 2010-05-17T19:19:24
# -------------------------------------------------
TARGET = build/waveformwidget

TEMPLATE = lib

CONFIG += dll

INCLUDEPATH += /usr/include

SOURCES += WaveformWidget.cpp \
    AudioUtil.cpp 

HEADERS += WaveformWidget.h \
    AudioUtil.h \
    MathUtil.h

LIBS += -lsndfile \
    -L/usr/lib
