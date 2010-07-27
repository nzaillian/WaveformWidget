#include "WaveformWidget.h"

#define DEFAULT_PADDING 0.3
#define LINE_WIDTH 1
#define POINT_SIZE 5
#define DEFAULT_COLOR Qt::blue
#define INDIVIDUAL_SAMPLE_DRAW_TOGGLE_POINT 9.0
#define MACRO_MODE_TOGGLE_CONSTANT 100.0

/*!
\file WaveformWidget.cpp
\brief WaveformWidget implementation file.
*/

/*!
	\mainpage WaveformWidget
       
	\brief This library provides a Qt widget for visualizing audio waveforms.<br><br>

	It depends on Erik de Castro Lopo's <a href='http://www.mega-nerd.com/libsndfile/'>libsndfile</a> and
 	the <a href='http://qt.nokia.com/'>Qt framework</a>.  It has been tested on Linux (kernel version 2.6.31-21) under Gnome with 
	version 4.6 of the Qt framework, but could well function in other environments.
	<br><br>
	A source archive can be found <a href='http://www.columbia.edu/~naz2106/WaveformWidget.tar.gz'>here</a>.
	<br><br>For build instructions, see the README.txt file contained in the top level directory of the source archive.  
*/

/*!
\brief Constructs an instance of WaveformWidget.
@param filePath Valid path to a WAV file.
*/
WaveformWidget::WaveformWidget(string filePath)
{
    this->srcAudioFile = new AudioUtil();
    this->audioFilePath = filePath;
    this->currentFileHandlingMode = FULL_CACHE;
    this->resetFile(this->audioFilePath);
    this->scaleFactor = -1.0;
    this->lastSize = this->size();
    this->padding = DEFAULT_PADDING;
    this->waveformColor = DEFAULT_COLOR;
}

/*The AudioUtil instance "srcAudioFile" is our only dynamically allocated object*/
WaveformWidget::~WaveformWidget()
{
    delete this->srcAudioFile;
}

/*!
\brief Reset the audio file to be visualized by this instance of WaveformWidget.

An important consideration when invoking this function is the file-handling mode that you
have set for the current instance of WaveformWidget.  If the instance of WaveformWidget is in FULL_CACHE mode,
this function will take considerably longer to execute (posssibly as long as a few seconds for an audio file of several
minutes' duration) as the entirety of the audio file to be visualized
by the widget must be loaded into memory.
@param fileName Valid path to a WAV file
*/
void WaveformWidget::resetFile(string fileName)
{
    this->audioFilePath = fileName;
    this->srcAudioFile->setFile(audioFilePath);

    switch(this->currentFileHandlingMode)
    {
        case FULL_CACHE:
            this->srcAudioFile->setFileHandlingMode(AudioUtil::FULL_CACHE);

        case DISK_MODE:
            this->srcAudioFile->setFileHandlingMode(AudioUtil::DISK_MODE);
    }

    this->peakVector.clear();
    this->dataVector.clear();
    this->currentDrawingMode = NO_MODE;
    this->establishDrawingMode();
    this->repaint();
 }

/*!
  \brief Mutator for the file-handling mode of a given instance of WaveformWidget.

An instance of WaveformWidget relies on an AudioUtil object to do much of the analysis of the audio file
that it visualizes.  This AudioUtil object can function in one of two modes: DISK_MODE or FULL_CACHE.
For a comprehensive outline of the benefits and drawbacks of each mode, see the documentation for
AudioUtil::setFileHandlingMode(FileHandlingMode mode).

@param mode The desired file-handling mode.  Valid options: WaveformWidget::FULL_CACHE, WaveformWidget::DISK_MODE
*/
void WaveformWidget::setFileHandlingMode(FileHandlingMode mode)
{
    this->currentFileHandlingMode = mode;

    switch (this->currentFileHandlingMode)
    {
        case FULL_CACHE:
            this->srcAudioFile->setFileHandlingMode(AudioUtil::FULL_CACHE);

        case DISK_MODE:
            this->srcAudioFile->setFileHandlingMode(AudioUtil::DISK_MODE);
    }
}

/*!
\brief Accessor for the file-handling mode of a given instance of WaveformWidget.

@return The file-handling mode of a given instance of WaveformWidget.
*/
WaveformWidget::FileHandlingMode WaveformWidget::getFileHandlingMode()
{
    return this->currentFileHandlingMode;
}

void WaveformWidget::recalculatePeaks()
{
    /*calculate scale factor*/
    vector<double> normPeak = srcAudioFile->calculateNormalizedPeaks();
    double peak = MathUtil::getVMax(normPeak);
    this->scaleFactor = 1.0/peak;
    this->scaleFactor = scaleFactor - scaleFactor * this->padding;

    /*calculate frame-grab increments*/
    int totalFrames = srcAudioFile->getTotalFrames();
    int frameIncrement = totalFrames/this->width();


    if(this->currentDrawingMode != MACRO)
    {
        if(srcAudioFile->getNumChannels() == 2)
        {
            this->peakVector.clear();

            vector<double> regionMax;

            /*
              Populate the peakVector with peak values for each region of the source audio
              file to be represented by a single pixel of the widget.
            */

            for(int i = 0; i < totalFrames; i += frameIncrement)
            {
                regionMax = srcAudioFile->peakForRegion(i, i+frameIncrement);
                double frameAbsL = fabs(regionMax[0]);
                double frameAbsR = fabs(regionMax[1]);

                this->peakVector.push_back(frameAbsL);
                this->peakVector.push_back(frameAbsR);
            }
        }

        if(this->srcAudioFile->getNumChannels() == 1)
        {

            this->peakVector.clear();
            vector<double> regionMax;

            /*
              Populate the peakVector with peak values for each region of the source audio
              file to be represented by a single pixel of the widget.
            */

            for(int i = 0; i < totalFrames; i += frameIncrement)
            {
                regionMax = srcAudioFile->peakForRegion(i, i+frameIncrement);
                double frameAbs = fabs(regionMax[0]);

                this->peakVector.push_back(frameAbs);
            }
        }
    }
}

void WaveformWidget::paintEvent( QPaintEvent * event )
{

#ifdef DEBUG
    char m[200];
    sprintf(m, "widget width: %d\naudio file size in frames:%d\n", this->width(), this->srcAudioFile->getTotalFrames());
    qDebug()<<m;
#endif

    this->establishDrawingMode();

    if(this->currentDrawingMode == OVERVIEW)
    {
        this->overviewDraw(event);
    }
    else if(this->currentDrawingMode == MACRO)
    {
        this->macroDraw(event);
    }

#ifdef DEBUG
    if(currentMode == MACRO)
        qDebug()<<"mode : MACRO\n";
    if(currentMode==OVERVIEW)
       qDebug()<<"mode: OVERVIEW\n";
#endif
}

/*
the macroDraw drawing function takes into account every single sample in the region of
the source audio file to be drawn in the body of the widget.  It maintains an optimal position
for every sample, and rounds this to the nearest integer (because there are no pixels with
non-integer indices) for drawing.
*/
void WaveformWidget::macroDraw(QPaintEvent *event)
{
    int yMidpoint = this->height()/2;

    int minX = event->region().boundingRect().x();
    int maxX = event->region().boundingRect().x() + event->region().boundingRect().width();

    int startFrame = (int) ((double)this->srcAudioFile->getTotalFrames())*(((double)minX)/((double)this->width()));
    int endFrame = (int) ((double)this->srcAudioFile->getTotalFrames())*(((double)maxX)/((double)this->width()));

    bool drawIndividualSamples = false;

    QPainter linePainter(this);
    QPainter pointPainter(this);
    linePainter.setPen(QPen(this->waveformColor, LINE_WIDTH, Qt::SolidLine, Qt::RoundCap));
    pointPainter.setPen(QPen(this->waveformColor, 1, Qt::SolidLine, Qt::RoundCap));

    double optimalPosition = (double) minX;
    double prevOptimalPosition = optimalPosition;

    /* Double-channel macro drawing routine:*/

    if(this->srcAudioFile->getNumChannels()==2)
    {
        int chan1YMidpoint = yMidpoint - this->height()/4;
        int chan2YMidpoint = yMidpoint + this->height()/4;

        double optimalSpacing = ((double)this->width())/(((double)this->dataVector.size())/2);
        if(optimalSpacing > INDIVIDUAL_SAMPLE_DRAW_TOGGLE_POINT)
        {
            pointPainter.setPen(QPen(this->waveformColor, POINT_SIZE, Qt::SolidLine, Qt::SquareCap));
            drawIndividualSamples = true;
        }
        int startIndex = 2*startFrame;
        int endIndex;

        /*Reading the values in the dataVector four additional indices deep will
          allow us to graph a few more points just out of frame so that
          the line will surpass the right edge of the viewable area, rather
          than stops short of it.  The same thing is done in the macroDraw routine for
          single-channel audio further down as well*/
        if(endFrame < this->srcAudioFile->getTotalFrames()-4)
        {
            endIndex = 2*endFrame + 4;
        }
        else{
            endIndex = 2*endFrame;
        }
        double prevLChannelVal = this->dataVector.at(startIndex);
        double prevRChannelVal = this->dataVector.at(startIndex+1);

/*
    Meat of the drawing routine:
*/
        for(int i = startIndex + 2; i < endIndex; i+=2)
        {
            double lChannelVal = this->dataVector.at(i);
            double rChannelVal = this->dataVector.at(i+1);

            /*
                If our zoom-level is such that it would be useful to see blocks
                representing individual samples, draw such blocks:
            */
            if(drawIndividualSamples == true)
            {
                pointPainter.drawPoint(QPoint(MathUtil::round(optimalPosition), chan1YMidpoint+((this->height()/4)*lChannelVal*scaleFactor)));
                pointPainter.drawPoint(QPoint(MathUtil::round(optimalPosition), chan2YMidpoint+((this->height()/4)*rChannelVal*scaleFactor)));
            }

            /*
                Draw lines from previous samples to current samples:
            */
            linePainter.drawLine(MathUtil::round(prevOptimalPosition), chan1YMidpoint+((this->height()/4)*prevLChannelVal*scaleFactor), MathUtil::round(optimalPosition), chan1YMidpoint+((this->height()/4)*lChannelVal*scaleFactor));
            linePainter.drawLine(MathUtil::round(prevOptimalPosition), chan2YMidpoint+((this->height()/4)*prevRChannelVal*scaleFactor), MathUtil::round(optimalPosition), chan2YMidpoint+((this->height()/4)*rChannelVal*scaleFactor));


            prevLChannelVal = lChannelVal;
            prevRChannelVal = rChannelVal;

            prevOptimalPosition = optimalPosition;
            optimalPosition += optimalSpacing;
        }

#ifdef DEBUG
        qDebug()<<"width: "<<this->width()<<" \nv size: "<<this->dataVector.size()<<"\noptimal spacing "<<optimalSpacing;
        qDebug()<<"audio file size: "<<this->srcAudioFile->getTotalFrames();
#endif
    }
    /*Single-channel macro drawing routine: */
    else if(this->srcAudioFile->getNumChannels() == 1)
    {
        double optimalSpacing = ((double)this->width())/(((double)this->dataVector.size()));
        if(optimalSpacing > INDIVIDUAL_SAMPLE_DRAW_TOGGLE_POINT)
        {
            pointPainter.setPen(QPen(this->waveformColor, POINT_SIZE, Qt::SolidLine, Qt::SquareCap));
            drawIndividualSamples = true;
        }
        int startIndex = startFrame;
        int endIndex;
        if(endFrame < this->srcAudioFile->getTotalFrames()-2)
        {
            endIndex = endFrame+2;
        }
        else{
           endIndex = endFrame;
        }

        double prevAudioDataVal = this->dataVector.at(startIndex);

/*
      Meat of the drawing routine:
*/
        for(int i = startIndex; i < endIndex; i++)
        {
            double audioDataVal = this->dataVector.at(i);

            /*
                If our zoom-level is such that it would be useful to see blocks
                representing individual samples, draw such blocks:
            */
            if(drawIndividualSamples == true){
                pointPainter.drawPoint(QPoint(MathUtil::round(optimalPosition), yMidpoint+((this->height()/2)*audioDataVal*scaleFactor)));
            }
            /*
                Draw a line from the previous sample to the current sample:
            */
            linePainter.drawLine(MathUtil::round(prevOptimalPosition), yMidpoint+((this->height()/2)*prevAudioDataVal*scaleFactor), MathUtil::round(optimalPosition), yMidpoint+((this->height()/2)*audioDataVal*scaleFactor));

            prevAudioDataVal = audioDataVal;

            prevOptimalPosition = optimalPosition;
            optimalPosition += optimalSpacing;
        }
    }

}

/*
    The overview drawing function works with the peakVector, which contains the peak value
    for every region (and each channel) of the source audio file to be represented by a single
    pixel of the widget.  The function steps through this vector and draws two vertical bars
    for each such value -- one above the Y-axis midpoint for the channel, and one below.
*/
void WaveformWidget::overviewDraw(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(QPen(this->waveformColor, 1, Qt::SolidLine, Qt::RoundCap));

    int minX = event->region().boundingRect().x();
    int maxX = event->region().boundingRect().x() + event->region().boundingRect().width();

    /*grab peak values for each region to be represented by a pixel in the visible
    portion of the widget, scale them, and draw: */
    if(this->srcAudioFile->getNumChannels() == 2)
    {

            int startIndex = 2*minX;
            int endIndex = 2*maxX;

            int yMidpoint = this->height()/2;
            int counter = minX;

            for(int i = startIndex;  i < endIndex; i+=2)
            {

                int chan1YMidpoint = yMidpoint - this->height()/4;
                int chan2YMidpoint = yMidpoint + this->height()/4;


                painter.drawLine(counter, chan1YMidpoint, counter, chan1YMidpoint+((this->height()/4)*this->peakVector.at(i)*scaleFactor));
                painter.drawLine(counter, chan1YMidpoint, counter, chan1YMidpoint -((this->height()/4)*this->peakVector.at(i)*scaleFactor));

                painter.drawLine(counter, chan2YMidpoint, counter, chan2YMidpoint+((this->height()/4)*this->peakVector.at(i+1)*scaleFactor)   );
                painter.drawLine(counter, chan2YMidpoint, counter, chan2YMidpoint -((this->height()/4)*this->peakVector.at(i+1)*scaleFactor)   );

                counter++;
            }

    }

    if(srcAudioFile->getNumChannels() == 1)
    {
           int curIndex = minX;
           int yMidpoint = this->height()/2;


           for(unsigned int i = 0; i < peakVector.size(); i++)
           {
               painter.drawLine(curIndex, yMidpoint, curIndex, yMidpoint+((this->height()/4)*this->peakVector.at(i)*scaleFactor)   );
               painter.drawLine(curIndex, yMidpoint, curIndex, yMidpoint -((this->height()/4)*this->peakVector.at(i)*scaleFactor)   );

               curIndex++;
           }

    }

}

/*
This function determines which drawing mode the current instance of WaveformWidget
should be operating in based on its dimensions and the size of the audio file that it
visualizes.
*/
void WaveformWidget::establishDrawingMode()
{
    int audioFileSize = this->srcAudioFile->getTotalFrames();

    if(this->currentDrawingMode == NO_MODE)
    {
        if(this->width() < audioFileSize/MACRO_MODE_TOGGLE_CONSTANT)
        {
            this->currentDrawingMode = OVERVIEW;
            this->recalculatePeaks();
        }else
        {
            this->currentDrawingMode = MACRO;
            this->dataVector = this->srcAudioFile->getAllFrames();
        }
    }

    if(this->currentDrawingMode != MACRO && this->width() >= audioFileSize/MACRO_MODE_TOGGLE_CONSTANT)
    {
        this->currentDrawingMode = MACRO;
        this->dataVector = this->srcAudioFile->getAllFrames();
    }

    if(this->currentDrawingMode == MACRO && this->width() < audioFileSize/MACRO_MODE_TOGGLE_CONSTANT)
    {
        this->currentDrawingMode = OVERVIEW;
    }

    if(this->size()!=this->lastSize && this->currentDrawingMode != MACRO)
    {
        this->recalculatePeaks();
    }

    this->lastSize = this->size();

}


/*!
    \brief Mutator for waveform color.

    @param The desired color for the waveform visualization.
*/
void WaveformWidget::setColor(QColor color)
{
    this->waveformColor = color;
}


void WaveformWidget::resizeEvent(QResizeEvent *)
{

}

