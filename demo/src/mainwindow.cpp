#include "mainwindow.h"
#define SCROLLBAR_W 30
#define SCROLLER_PADDING_TOP 50

MainWindow::MainWindow()
{

    waveformWidget = new WaveformWidget("../../devel/src/test.wav");

    waveformWidget->setGeometry(0,0, 8192000, this->height());

    layout  = new QGridLayout(this);

    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(waveformWidget);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(scrollArea, 0, 0, 12, 12);

    zoomInButton = new QPushButton("zoom in", this);
    zoomOutButton = new QPushButton("zoom out", this);
    setFileButton = new QPushButton("set source", this);
    layout->addWidget(zoomInButton, 12,3,1,2 );
    layout->addWidget(zoomOutButton, 12, 5, 1, 2);
    layout->addWidget(setFileButton, 12, 7, 1, 2);



   this->setGeometry(50,50,500,300);


    waveformWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    waveformWidget->setGeometry(0,0,waveformWidget->width(), scrollArea->height());


    waveformWidget->repaint();

    QObject::connect(this->zoomInButton, SIGNAL(clicked()),this, SLOT(zoomInClicked()));
    QObject::connect(this->zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOutClicked()));
    QObject::connect(this->setFileButton, SIGNAL(clicked()), this, SLOT(setSourceClicked()));


}

void MainWindow::setSourceClicked()
{
    std::string fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"~", "Audio(*.WAV *.wav)").toStdString();
    waveformWidget->resetFile(fileName);
}

MainWindow::~MainWindow()
{
    delete waveformWidget;
}

void MainWindow::resizeEvent(QResizeEvent * )
{
    waveformWidget->setGeometry(0,0,waveformWidget->width(), scrollArea->height()-SCROLLBAR_W);
}

void MainWindow::zoomInClicked()
{
   waveformWidget->setGeometry(0,0,waveformWidget->width()*2, scrollArea->height()-SCROLLBAR_W);
}

void MainWindow::zoomOutClicked()
{
    waveformWidget->setGeometry(0,0,waveformWidget->width()/2, scrollArea->height()-SCROLLBAR_W);
}
