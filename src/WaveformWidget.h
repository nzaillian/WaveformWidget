#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include "AudioUtil.h"
#include "MathUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include <sndfile.h>

#include <QSize>
#include <QColor>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QPoint>
#include <QResizeEvent>

/*!
    \file WaveformWidget.h
    \brief WaveformWidget header file.
*/

using namespace std;

/*!
\brief A Qt widget to display the waveform of an audio file.


*/
class WaveformWidget : public QWidget
{
public:
    WaveformWidget(string filePath);
    ~WaveformWidget();
    void resetFile(string fileName);
    enum FileHandlingMode {FULL_CACHE, DISK_MODE};
    void setColor(QColor color);
    void setFileHandlingMode(FileHandlingMode mode);
    FileHandlingMode getFileHandlingMode();

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent( QPaintEvent * event );

private:
    AudioUtil *srcAudioFile;
    enum DrawingMode {OVERVIEW, MACRO, NO_MODE};
    DrawingMode currentDrawingMode;
    FileHandlingMode currentFileHandlingMode;
    vector<double> peakVector;
    vector<double> dataVector;
    string audioFilePath;
    double max_peak;
    double padding;
    QSize lastSize;
    QColor waveformColor;

    double scaleFactor;
    void recalculatePeaks();
    void establishDrawingMode();
    void macroDraw(QPaintEvent* event);
    void overviewDraw(QPaintEvent* event);
};

#endif // WAVEFORMWIDGET_H
