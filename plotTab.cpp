#include "plotTab.h"
#include "ui_plottab.h"
#include "mainwindow.h"
#include "dataprovider.h"

PlotTab::PlotTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlotTab)
{
    ui->setupUi(this);

    connect(ui->reloadReportButton, SIGNAL(clicked()), this, SLOT(reloadReport()));

    /*QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = x[i]*x[i]; // let's plot a quadratic function
    }
    // create graph and assign data to it:
    ui->spikePlot->addGraph();
    ui->spikePlot->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->spikePlot->xAxis->setLabel("x");
    ui->spikePlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->spikePlot->xAxis->setRange(-1, 1);
    ui->spikePlot->yAxis->setRange(0, 1);
    ui->spikePlot->replot();*/

    // include this section to fully disable antialiasing for higher performance:
/*
    ui->spikePlot->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    ui->spikePlot->xAxis->setTickLabelFont(font);
    ui->spikePlot->yAxis->setTickLabelFont(font);
    ui->spikePlot->legend->setFont(font);
*/

    /*ui->spikePlot->addGraph(); // blue dot
    ui->spikePlot->graph(2)->setPen(QPen(Qt::blue));
    ui->spikePlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->spikePlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    ui->spikePlot->addGraph(); // red dot
    ui->spikePlot->graph(3)->setPen(QPen(Qt::red));
    ui->spikePlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    ui->spikePlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);*/

    ui->spikePlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->spikePlot->xAxis->setDateTimeFormat("mm:ss");
    ui->spikePlot->xAxis->setAutoTickStep(false);
    ui->spikePlot->xAxis->setTickStep(1);
    ui->spikePlot->axisRect()->setupFullAxesBox();

    ui->spikePlot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
    ui->spikePlot->yAxis->setAutoTicks(false);
    ui->spikePlot->yAxis->setAutoTickLabels(false);
    ui->spikePlot->yAxis->setTickVector(QVector<double>() << 5 << 55);
    ui->spikePlot->yAxis->setTickVectorLabels(QVector<QString>() << "sdg so\nhigh" << "Very\nhigh");
    ui->spikePlot->yAxis->setRange(0, 60);

    //ui->spikePlot->yAxis->setAutoTickStep(false);
    ui->spikePlot->yAxis->setAutoSubTicks(false);
    //ui->spikePlot->yAxis->setTickStep(1.0);
    ui->spikePlot->yAxis->setSubTickCount(0);
    //ui->spikePlot->yAxis2->setAutoTickStep(false);
    ui->spikePlot->yAxis2->setAutoSubTicks(false);
    //ui->spikePlot->yAxis2->setTickStep(1.0);
    ui->spikePlot->yAxis2->setSubTickCount(0);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->spikePlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->spikePlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->spikePlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->spikePlot->yAxis2, SLOT(setRange(QCPRange)));

    DataProvider::getInstance()->setSpikePlot(ui->spikePlot);

    DataProvider::getInstance()->setupReportWatcher();

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(15); // Interval 0 means to refresh as fast as possible

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

PlotTab::~PlotTab()
{

}

void PlotTab::setMainWindow(MainWindow* mainWindow_)
{
    this->mainWindow = mainWindow_;
}

void PlotTab::realtimeDataSlot()
{
    // calculate two new data points:
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    static double lastPointKey = 0;
    if(key-lastPointKey > 0.01) // at most add point every 10 ms
    {
        /*double value0 = 0;//qSin(key); //sin(key*1.6+cos(key*1.7)*2)*10 + sin(key*1.2+0.56)*20 + 26;
        double value1 = 1;//qCos(key); //sin(key*1.3+cos(key*1.2)*1.2)*7 + sin(key*0.9+0.26)*24 + 26;
        double value2 = 2;

        bool spike0 = rand()%100 < 5;
        bool spike1 = rand()%100 < 3;
        bool spike2 = rand()%100 < 1;*/

        // add data to lines:
        /*if(spike0)
            ui->spikePlot->graph(0)->addData(key, value0);
        if(spike1)
            ui->spikePlot->graph(1)->addData(key, value1);
        if(spike2)
            ui->spikePlot->graph(2)->addData(key, value2);*/
        // set data of dots:
        /*if(spike0)
        {
            ui->spikePlot->graph(2)->clearData();
            ui->spikePlot->graph(2)->addData(key, value0);
        }
        if(spike1)
        {
            ui->spikePlot->graph(3)->clearData();
            ui->spikePlot->graph(3)->addData(key, value1);
        }*/

        // remove data of lines that's outside visible range
        for(int i=0; i<ui->spikePlot->graphCount(); i++)
            ui->spikePlot->graph(i)->removeDataBefore(key-10);

        // rescale value (vertical) axis to fit the current data
        /*if(ui->spikePlot->graphCount() >= 1)
            ui->spikePlot->graph(0)->rescaleValueAxis();
        for(int i=1; i<ui->spikePlot->graphCount(); i++)
            ui->spikePlot->graph(i)->rescaleValueAxis(true);*/

        lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 10):
    ui->spikePlot->xAxis->setRange(key+0.25, 10, Qt::AlignRight);
    ui->spikePlot->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        int dataCount = 0;
        for(int i=0; i<ui->spikePlot->graphCount(); i++)
            dataCount += ui->spikePlot->graph(0)->data()->count();

        mainWindow->showMessageStatusBar(
            QString("%1 FPS, %2 Graphs, Total Data points: %3")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(ui->spikePlot->graphCount())
            .arg(dataCount)
            , 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

void PlotTab::reloadReport()
{
    qDebug() << "Reload latest report ...";
    DataProvider::getInstance()->parseLatestReport();
}
