#include <QtGui/QApplication>
#include "mainwindow.h"
#include "../../src/AudioUtil.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Z's Waveform Viewer");
    w.show();
    return a.exec();

}
