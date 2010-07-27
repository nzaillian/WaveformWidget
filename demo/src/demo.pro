# -------------------------------------------------
# Project created by QtCreator 2010-05-17T19:19:24
# -------------------------------------------------
TARGET = WaveformViewerDemo

TEMPLATE = app

INCLUDEPATH += /usr/include

DEPENDPATH += /usr/lib

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += mainwindow.h 

LIBS += -L/usr/lib \
    -lsndfile \
    -lwaveformwidget 

