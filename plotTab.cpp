#include "plotTab.h"
#include "ui_plottab.h"
#include "mainwindow.h"

PlotTab::PlotTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlotTab)
{
    ui->setupUi(this);

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
    ui->spikePlot->addGraph(); // blue line
    ui->spikePlot->graph(0)->setPen(QPen(Qt::blue));
    ui->spikePlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    ui->spikePlot->graph(0)->setAntialiasedFill(false);
    ui->spikePlot->addGraph(); // red line
    ui->spikePlot->graph(1)->setPen(QPen(Qt::red));
    ui->spikePlot->graph(0)->setChannelFillGraph(ui->spikePlot->graph(1));

    ui->spikePlot->addGraph(); // blue dot
    ui->spikePlot->graph(2)->setPen(QPen(Qt::blue));
    ui->spikePlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->spikePlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    ui->spikePlot->addGraph(); // red dot
    ui->spikePlot->graph(3)->setPen(QPen(Qt::red));
    ui->spikePlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    ui->spikePlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

    ui->spikePlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->spikePlot->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->spikePlot->xAxis->setAutoTickStep(false);
    ui->spikePlot->xAxis->setTickStep(2);
    ui->spikePlot->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->spikePlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->spikePlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->spikePlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->spikePlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
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
  if (key-lastPointKey > 0.01) // at most add point every 10 ms
  {
    double value0 = qSin(key); //sin(key*1.6+cos(key*1.7)*2)*10 + sin(key*1.2+0.56)*20 + 26;
    double value1 = qCos(key); //sin(key*1.3+cos(key*1.2)*1.2)*7 + sin(key*0.9+0.26)*24 + 26;
    // add data to lines:
    ui->spikePlot->graph(0)->addData(key, value0);
    ui->spikePlot->graph(1)->addData(key, value1);
    // set data of dots:
    ui->spikePlot->graph(2)->clearData();
    ui->spikePlot->graph(2)->addData(key, value0);
    ui->spikePlot->graph(3)->clearData();
    ui->spikePlot->graph(3)->addData(key, value1);
    // remove data of lines that's outside visible range:
    ui->spikePlot->graph(0)->removeDataBefore(key-8);
    ui->spikePlot->graph(1)->removeDataBefore(key-8);
    // rescale value (vertical) axis to fit the current data:
    ui->spikePlot->graph(0)->rescaleValueAxis();
    ui->spikePlot->graph(1)->rescaleValueAxis(true);
    lastPointKey = key;
  }
  // make key axis range scroll with the data (at a constant range size of 8):
  ui->spikePlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
  ui->spikePlot->replot();

  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
    mainWindow->showMessageStatusBar(
          QString("%1 FPS, Total Data points: %2")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(ui->spikePlot->graph(0)->data()->count()+ui->spikePlot->graph(1)->data()->count())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }
}
