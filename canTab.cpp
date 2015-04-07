#include "canTab.h"
#include "ui_cantab.h"
#include "mainwindow.h"
#include "dataprovider.h"

#include <chrono>
#include <thread>

CanTab::CanTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CanTab)
{
    ui->setupUi(this);

    connect(ui->startCANButton, SIGNAL(clicked()), this, SLOT(startCAN()));
    connect(ui->stopCANButton, SIGNAL(clicked()), this, SLOT(stopCAN()));
    connect(ui->setMotorButton, SIGNAL(clicked()), this, SLOT(setMotor()));
    connect(ui->reset1Button, SIGNAL(clicked()), this, SLOT(resetMotor1()));
    connect(ui->reset2Button, SIGNAL(clicked()), this, SLOT(resetMotor2()));
    connect(ui->stopMotorButton, SIGNAL(clicked()), this, SLOT(stopMotors()));

    /*QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = x[i]*x[i]; // let's plot a quadratic function
    }
    // create graph and assign data to it:
    ui->canPlot->addGraph();
    ui->canPlot->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->canPlot->xAxis->setLabel("x");
    ui->canPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->canPlot->xAxis->setRange(-1, 1);
    ui->canPlot->yAxis->setRange(0, 1);
    ui->canPlot->replot();*/

    // include this section to fully disable antialiasing for higher performance:
/*
    ui->canPlot->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    ui->canPlot->xAxis->setTickLabelFont(font);
    ui->canPlot->yAxis->setTickLabelFont(font);
    ui->canPlot->legend->setFont(font);
*/

    /*ui->canPlot->addGraph(); // blue dot
    ui->canPlot->graph(2)->setPen(QPen(Qt::blue));
    ui->canPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->canPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    ui->canPlot->addGraph(); // red dot
    ui->canPlot->graph(3)->setPen(QPen(Qt::red));
    ui->canPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    ui->canPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);*/

    ui->canPlot->legend->setVisible(true);
    //ui->canPlot->legend->setFont(QFont("Helvetica", 9));
    //ui->canPlot->legend->setRowSpacing(-3);

    ui->canPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->canPlot->xAxis->setDateTimeFormat("mm:ss");
    ui->canPlot->xAxis->setAutoTickStep(false);
    ui->canPlot->xAxis->setTickStep(1);
    ui->canPlot->axisRect()->setupFullAxesBox();

    /*ui->canPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
    ui->canPlot->yAxis->setAutoTicks(false);
    ui->canPlot->yAxis->setAutoTickLabels(false);
    ui->canPlot->yAxis->setTickVector(QVector<double>() << 5 << 55);
    ui->canPlot->yAxis->setTickVectorLabels(QVector<QString>() << "sdg so\nhigh" << "Very\nhigh");*/
    ui->canPlot->yAxis->setRange(-850, 850);    // fixed range of the joint data
    ui->canPlot->yAxis->setLabel("Joint Position");

    ui->canPlot->yAxis2->setVisible(true);
    //ui->canPlot->yAxis2->setRange(-10, 500);  // fixed range of the spring displacement data
    ui->canPlot->yAxis2->setRange(-10, 1000);
    ui->canPlot->yAxis2->setAutoTickCount(10);
    ui->canPlot->yAxis2->setAutoTickLabels(true);
    ui->canPlot->yAxis2->setAutoTicks(true);
    ui->canPlot->yAxis2->setAutoTickStep(true);

    ui->canPlot->yAxis2->setTicks(true);
    ui->canPlot->yAxis2->setTickLabels(true);
    //ui->canPlot->yAxis2->setSubTickLength(1, 1);
    ui->canPlot->yAxis2->setLabel("Current / Spring / PWM");

    //ui->canPlot->yAxis2->setAutoTicks(true);
    //ui->canPlot->yAxis2->setAutoTickLabels(true);
    //ui->canPlot->yAxis2->set

    //ui->canPlot->yAxis->setAutoTickStep(false);
    //ui->canPlot->yAxis->setAutoSubTicks(false);
    //ui->canPlot->yAxis->setTickStep(1.0);
    //ui->canPlot->yAxis->setSubTickCount(0);
    //ui->canPlot->yAxis2->setAutoTickStep(false);
    //ui->canPlot->yAxis2->setAutoSubTicks(false);
    //ui->canPlot->yAxis2->setTickStep(1.0);
    //ui->canPlot->yAxis2->setSubTickCount(0);

    ui->canPlot->addGraph(ui->canPlot->xAxis, ui->canPlot->yAxis2);
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setPen(QPen(QBrush(QColor(200, 100, 10)), 3));//setPen(QPen(Qt::blue));
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setName("Current (R)");

    ui->canPlot->addGraph(ui->canPlot->xAxis, ui->canPlot->yAxis2);
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setPen(QPen(QBrush(QColor(180, 0, 0)), 3, Qt::DashLine));//setPen(QPen(Qt::blue));
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setName("Spring (R)");

    ui->canPlot->addGraph(ui->canPlot->xAxis, ui->canPlot->yAxis2);
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setPen(QPen(QBrush(QColor(250, 150, 50)), 3));//setPen(QPen(Qt::blue));
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setName("Current (L)");

    ui->canPlot->addGraph(ui->canPlot->xAxis, ui->canPlot->yAxis2);
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setPen(QPen(QBrush(QColor(250, 60, 60)), 3, Qt::DashLine));//setPen(QPen(Qt::blue));
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setName("Spring (L)");

    ui->canPlot->addGraph(ui->canPlot->xAxis, ui->canPlot->yAxis2);
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setPen(QPen(QBrush(QColor(10, 100, 10)), 3));//setPen(QPen(Qt::blue));
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setName("PWM (R)");

    ui->canPlot->addGraph(ui->canPlot->xAxis, ui->canPlot->yAxis2);
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setPen(QPen(QBrush(QColor(30, 150, 30)), 3));//setPen(QPen(Qt::blue));
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setName("PWM (L)");

    ui->canPlot->addGraph(ui->canPlot->xAxis, ui->canPlot->yAxis);
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setPen(QPen(QBrush(Qt::blue), 5));//setPen(QPen(Qt::blue));
    ui->canPlot->graph(ui->canPlot->graphCount()-1)->setName("Joint Position");
    //ui->canPlot->graph(ui->canPlot->graphCount()-1)->setLineStyle(QCPGraph::lsNone);
    //ui->canPlot->graph(ui->canPlot->graphCount()-1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDiamond, 5));

    ui->canPlot->replot(); // needed?

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->canPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->canPlot->xAxis2, SLOT(setRange(QCPRange)));
    //connect(ui->canPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->canPlot->yAxis2, SLOT(setRange(QCPRange)));

    DataProvider::getInstance()->setCanPlot(ui->canPlot);

    // set these values as you wish!
    this->showPastTime = 2.0;
    this->windowWidth = 10.5;
    this->rightBlankTime = 0.5;

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    this->updateFrequency = this->windowWidth - this->showPastTime - this->rightBlankTime;
    this->timeSpiNNakerStartBefore = DataProvider::getInstance()->getTimeSpiNNakerStartInMs();
    this->plotStartTime = std::floor((QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - this->timeSpiNNakerStartBefore/1000.0) - this->updateFrequency);
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(50); // Interval 0 means to refresh as fast as possible

    /*
    QVector<QCPScatterStyle::ScatterShape> shapes;
    shapes << QCPScatterStyle::ssCross;
    shapes << QCPScatterStyle::ssPlus;
    shapes << QCPScatterStyle::ssCircle;
    shapes << QCPScatterStyle::ssDisc;
    shapes << QCPScatterStyle::ssSquare;
    shapes << QCPScatterStyle::ssDiamond;
    shapes << QCPScatterStyle::ssStar;
    shapes << QCPScatterStyle::ssTriangle;
    shapes << QCPScatterStyle::ssTriangleInverted;
    shapes << QCPScatterStyle::ssCrossSquare;
    shapes << QCPScatterStyle::ssPlusSquare;
    shapes << QCPScatterStyle::ssCrossCircle;
    shapes << QCPScatterStyle::ssPlusCircle;
    shapes << QCPScatterStyle::ssPeace;
    shapes << QCPScatterStyle::ssCustom;
    // set scatter style:
    if(shapes.at(i) != QCPScatterStyle::ssCustom)
    {
        customPlot->graph()->setScatterStyle(QCPScatterStyle(shapes.at(i), 10));
    }
    else
    {
        QPainterPath customScatterPath;
        for(int i=0; i<3; ++i)
            customScatterPath.cubicTo(qCos(2*M_PI*i/3.0)*9, qSin(2*M_PI*i/3.0)*9, qCos(2*M_PI*(i+0.9)/3.0)*9, qSin(2*M_PI*(i+0.9)/3.0)*9, 0, 0);
        customPlot->graph()->setScatterStyle(QCPScatterStyle(customScatterPath, QPen(), QColor(40, 70, 255, 50), 10));
    }*/
}

CanTab::~CanTab()
{

}

void CanTab::setMainWindow(MainWindow* mainWindow_)
{
    this->mainWindow = mainWindow_;
}

void CanTab::realtimeDataSlot()
{
    qint64 timeSpiNNakerStart = DataProvider::getInstance()->getTimeSpiNNakerStartInMs();

    if(timeSpiNNakerStart != this->timeSpiNNakerStartBefore)
    {
        this->timeSpiNNakerStartBefore = timeSpiNNakerStart;
        this->plotStartTime = std::floor((QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - this->timeSpiNNakerStartBefore/1000.0) - this->updateFrequency);
        for(int i=0; i<ui->canPlot->graphCount(); i++)
            ui->canPlot->graph(i)->clearData();
    }

    double time = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - timeSpiNNakerStart/1000.0;

    if(time - this->plotStartTime > this->updateFrequency)
    {
        /*double value0 = 0;//qSin(key); //sin(key*1.6+cos(key*1.7)*2)*10 + sin(key*1.2+0.56)*20 + 26;
        double value1 = 1;//qCos(key); //sin(key*1.3+cos(key*1.2)*1.2)*7 + sin(key*0.9+0.26)*24 + 26;
        double value2 = 2;

        bool spike0 = rand()%100 < 5;
        bool spike1 = rand()%100 < 3;
        bool spike2 = rand()%100 < 1;*/

        // add data to lines:
        /*if(spike0)
            ui->canPlot->graph(0)->addData(key, value0);
        if(spike1)
            ui->canPlot->graph(1)->addData(key, value1);
        if(spike2)
            ui->canPlot->graph(2)->addData(key, value2);*/
        // set data of dots:
        /*if(spike0)
        {
            ui->canPlot->graph(2)->clearData();
            ui->canPlot->graph(2)->addData(key, value0);
        }
        if(spike1)
        {
            ui->canPlot->graph(3)->clearData();
            ui->canPlot->graph(3)->addData(key, value1);
        }*/

        // remove data of lines that's outside visible range

        this->plotStartTime += this->updateFrequency;

        for(int i=0; i<ui->canPlot->graphCount(); i++)
            ui->canPlot->graph(i)->removeDataBefore(this->plotStartTime - this->showPastTime);

        // rescale value (vertical) axis to fit the current data
        /*if(ui->canPlot->graphCount() >= 1)
            ui->canPlot->graph(0)->rescaleValueAxis();
        for(int i=1; i<ui->canPlot->graphCount(); i++)
            ui->canPlot->graph(i)->rescaleValueAxis(true);*/

        ui->canPlot->xAxis->setRange(this->plotStartTime - this->showPastTime, this->windowWidth, Qt::AlignLeft);
    }

    ui->canPlot->replot();

    // calculate frames per second:
    /*static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        int dataCount = 0;
        for(int i=0; i<ui->canPlot->graphCount(); i++)
            dataCount += ui->canPlot->graph(0)->data()->count();

        mainWindow->showMessageStatusBar(
            QString("%1 FPS, %2 Graphs, Total Data points: %3")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(ui->canPlot->graphCount())
            .arg(dataCount)
            , 0);
        lastFpsKey = key;
        frameCount = 0;
    }*/
}

void CanTab::startCAN()
{
    DataProvider::getInstance()->startCan();
}

void CanTab::stopCAN()
{
    DataProvider::getInstance()->stopCan();
}

void CanTab::setMotor()
{    
    DataProvider::getInstance()->canInterface->setReference1(ui->motor1Slider->value());
    DataProvider::getInstance()->canInterface->setReference2(ui->motor2Slider->value());

    DataProvider::getInstance()->canInterface->start();
    //std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    //DataProvider::getInstance()->canInterface->stop();
}

void CanTab::stopMotors()
{
    DataProvider::getInstance()->canInterface->stop();
}

void CanTab::resetMotor1()
{
    ui->motor1Slider->setValue(0);
}

void CanTab::resetMotor2()
{
    ui->motor2Slider->setValue(0);
}

void CanTab::savePlot()
{
    QString filename = "canPlot";

    ui->canPlot->savePdf(filename+".pdf", false, 0, 0);
    //ui->canPlot->savePdf(filename+"noCosmetics.pdf", true);  // better for editing in Inkscape?

    //ui->canPlot->setAntialiasedElements(QCP::aeAll);
    ui->canPlot->savePng(filename+".png", 0, 0, 2.0, 100);
}
