#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QScrollArea>
#include <QMenuBar>
#include <QFileDialog>
#include <QMenu>

#include "../../src/WaveformWidget.h"

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
    QScrollArea *scrollArea;
    QGridLayout *layout;
    QPushButton *zoomInButton;
    QPushButton *zoomOutButton;
    QPushButton *setFileButton;
    void resizeEvent(QResizeEvent * );
    WaveformWidget *waveformWidget;


public slots:
    void zoomInClicked();
    void zoomOutClicked();
    void setSourceClicked();

};

#endif // MAINWINDOW_H
