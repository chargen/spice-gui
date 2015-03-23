#include "controlTab.h"
#include "ui_controltab.h"
#include "mainwindow.h"
#include "dataprovider.h"
#include "canDataProvider.h"

#include <chrono>
#include <thread>

ControlTab::ControlTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlTab)
{
    ui->setupUi(this);

    modeAutoTraj = false;
    this->ui->valueEdit->setText(QString::number(this->ui->valueSlider->value()));
    this->ui->valueEdit->setDisabled(true);
    connect(ui->toggleButton, SIGNAL(clicked()), this, SLOT(toggleMode()));
    connect(ui->valueSlider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));

    // include this section to fully disable antialiasing for higher performance:
/*
    ui->controlPlot->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    ui->controlPlot->xAxis->setTickLabelFont(font);
    ui->controlPlot->yAxis->setTickLabelFont(font);
    ui->controlPlot->legend->setFont(font);
*/

    /*ui->controlPlot->addGraph(); // blue dot
    ui->controlPlot->graph(2)->setPen(QPen(Qt::blue));
    ui->controlPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->controlPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    ui->controlPlot->addGraph(); // red dot
    ui->controlPlot->graph(3)->setPen(QPen(Qt::red));
    ui->controlPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    ui->controlPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);*/

    ui->controlPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->controlPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->controlPlot->xAxis->setAutoTickStep(false);
    ui->controlPlot->xAxis->setTickStep(1);
    ui->controlPlot->axisRect()->setupFullAxesBox();

    /*ui->controlPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
    ui->controlPlot->yAxis->setAutoTicks(false);
    ui->controlPlot->yAxis->setAutoTickLabels(false);
    ui->controlPlot->yAxis->setTickVector(QVector<double>() << 5 << 55);
    ui->controlPlot->yAxis->setTickVectorLabels(QVector<QString>() << "sdg so\nhigh" << "Very\nhigh");*/
    ui->controlPlot->yAxis->setRange(0, 500);    // fixed range for error value
    ui->controlPlot->yAxis->setLabel("Error Value");

    ui->controlPlot->yAxis2->setVisible(true);
    //ui->controlPlot->yAxis2->setRange(-10, 500);  // fixed range of the spring displacement data
    ui->controlPlot->yAxis2->setRange(-800, 800);
    ui->controlPlot->yAxis2->setAutoTickCount(10);
    ui->controlPlot->yAxis2->setAutoTickLabels(true);
    ui->controlPlot->yAxis2->setAutoTicks(true);
    ui->controlPlot->yAxis2->setAutoTickStep(true);

    ui->controlPlot->yAxis2->setTicks(true);
    ui->controlPlot->yAxis2->setTickLabels(true);
    //ui->controlPlot->yAxis2->setSubTickLength(1, 1);
    ui->controlPlot->yAxis2->setLabel("Angle");

    //ui->controlPlot->yAxis2->setAutoTicks(true);
    //ui->controlPlot->yAxis2->setAutoTickLabels(true);
    //ui->controlPlot->yAxis2->set

    //ui->controlPlot->yAxis->setAutoTickStep(false);
    //ui->controlPlot->yAxis->setAutoSubTicks(false);
    //ui->controlPlot->yAxis->setTickStep(1.0);
    //ui->controlPlot->yAxis->setSubTickCount(0);
    //ui->controlPlot->yAxis2->setAutoTickStep(false);
    //ui->controlPlot->yAxis2->setAutoSubTicks(false);
    //ui->controlPlot->yAxis2->setTickStep(1.0);
    //ui->controlPlot->yAxis2->setSubTickCount(0);

    ui->controlPlot->addGraph(ui->controlPlot->xAxis, ui->controlPlot->yAxis);
    ui->controlPlot->graph(ui->controlPlot->graphCount()-1)->setPen(QPen(QBrush(Qt::red), 5));//setPen(QPen(Qt::blue));

    ui->controlPlot->addGraph(ui->controlPlot->xAxis, ui->controlPlot->yAxis);
    ui->controlPlot->graph(ui->controlPlot->graphCount()-1)->setPen(QPen(QBrush(QColor(200, 50, 50)), 5));//setPen(QPen(Qt::blue));

    //ui->controlPlot->addGraph(ui->controlPlot->xAxis, ui->controlPlot->yAxis2);
    //ui->controlPlot->graph(ui->controlPlot->graphCount()-1)->setPen(QPen(QBrush(QColor(250, 50, 50)), 5));//setPen(QPen(Qt::blue));

    ui->controlPlot->addGraph(ui->controlPlot->xAxis, ui->controlPlot->yAxis2);
    ui->controlPlot->graph(ui->controlPlot->graphCount()-1)->setPen(QPen(QBrush(QColor(250, 150, 50)), 5));
    ui->controlPlot->addGraph(ui->controlPlot->xAxis, ui->controlPlot->yAxis2);
    ui->controlPlot->graph(ui->controlPlot->graphCount()-1)->setPen(QPen(QBrush(Qt::blue), 5));
    //ui->controlPlot->graph(ui->controlPlot->graphCount()-1)->setLineStyle(QCPGraph::lsNone);
    //ui->controlPlot->graph(ui->controlPlot->graphCount()-1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    ui->controlPlot->replot(); // needed?

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->controlPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->controlPlot->xAxis2, SLOT(setRange(QCPRange)));
    //connect(ui->controlPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->controlPlot->yAxis2, SLOT(setRange(QCPRange)));

    //DataProvider::getInstance()->setcontrolPlot(ui->controlPlot);

    // set these values as you wish!
    this->showPastTime = 2.0;
    this->windowWidth = 10.5;
    this->rightBlankTime = 0.5;

    // interval to process data, calculate errors and send data via serial connection to SpiNNaker
    this->processDataInterval = 50;

    this->prev_error = 0;
    this->integral = 0;
    // set these gains appropriately!
    this->Kp = 1.0;
    this->Ki = 0.0;
    this->Kd = 1.0;

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    this->updateFrequency = this->windowWidth - this->showPastTime - this->rightBlankTime;
    this->plotStartTime = std::floor(QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - this->updateFrequency);
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(50); // Interval 0 means to refresh as fast as possible



    // setup the timer to send data to SpiNNaker via serial port
    QTimer *timerDataSend = new QTimer(this);
    connect(timerDataSend, SIGNAL(timeout()), this, SLOT(sendData()));
    timerDataSend->start(this->processDataInterval);



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

ControlTab::~ControlTab()
{

}

void ControlTab::setMainWindow(MainWindow* mainWindow_)
{
    this->mainWindow = mainWindow_;
}

void ControlTab::realtimeDataSlot()
{
    if(QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - this->plotStartTime > this->updateFrequency)
    {
        /*double value0 = 0;//qSin(key); //sin(key*1.6+cos(key*1.7)*2)*10 + sin(key*1.2+0.56)*20 + 26;
        double value1 = 1;//qCos(key); //sin(key*1.3+cos(key*1.2)*1.2)*7 + sin(key*0.9+0.26)*24 + 26;
        double value2 = 2;

        bool spike0 = rand()%100 < 5;
        bool spike1 = rand()%100 < 3;
        bool spike2 = rand()%100 < 1;*/

        // add data to lines:
        /*if(spike0)
            ui->controlPlot->graph(0)->addData(key, value0);
        if(spike1)
            ui->controlPlot->graph(1)->addData(key, value1);
        if(spike2)
            ui->controlPlot->graph(2)->addData(key, value2);*/
        // set data of dots:
        /*if(spike0)
        {
            ui->controlPlot->graph(2)->clearData();
            ui->controlPlot->graph(2)->addData(key, value0);
        }
        if(spike1)
        {
            ui->controlPlot->graph(3)->clearData();
            ui->controlPlot->graph(3)->addData(key, value1);
        }*/

        // remove data of lines that's outside visible range

        this->plotStartTime += this->updateFrequency;

        for(int i=0; i<ui->controlPlot->graphCount(); i++)
            ui->controlPlot->graph(i)->removeDataBefore(this->plotStartTime - this->showPastTime);

        // rescale value (vertical) axis to fit the current data
        /*if(ui->controlPlot->graphCount() >= 1)
            ui->controlPlot->graph(0)->rescaleValueAxis();
        for(int i=1; i<ui->controlPlot->graphCount(); i++)
            ui->controlPlot->graph(i)->rescaleValueAxis(true);*/

        ui->controlPlot->xAxis->setRange(this->plotStartTime - this->showPastTime, this->windowWidth, Qt::AlignLeft);
    }

    ui->controlPlot->replot();

    // calculate frames per second:
    /*static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        int dataCount = 0;
        for(int i=0; i<ui->controlPlot->graphCount(); i++)
            dataCount += ui->controlPlot->graph(0)->data()->count();

        mainWindow->showMessageStatusBar(
            QString("%1 FPS, %2 Graphs, Total Data points: %3")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(ui->controlPlot->graphCount())
            .arg(dataCount)
            , 0);
        lastFpsKey = key;
        frameCount = 0;
    }*/
}

void ControlTab::sendData()
{
    //::std::cout << CanDataProvider::getInstance()->getLatestJointDataSet().at(0).s.jointPosition << ::std::endl; 

    int16_t target_angle = 0;
    int16_t current_angle = CanDataProvider::getInstance()->getLatestJointDataSet().at(0).s.jointPosition;
    double curr_can_time_angle = CanDataProvider::getInstance()->getLatestJointDataSetTimeMicroSec();

    if(this->modeAutoTraj)
    {
        target_angle = 800.0*sin(QDateTime::currentDateTime().toMSecsSinceEpoch()/10000.0);
        this->ui->valueSlider->setValue(target_angle);
    }
    else
    {
        target_angle = this->ui->valueSlider->value();
    }

    int max_error = 500;
    int16_t left_error_final = 0;
    int16_t right_error_final = 0;

    // send out error and control commands via serial interface
    if(DataProvider::getInstance()->canInterface != NULL && DataProvider::getInstance()->serial->isOpen())
    {
        // limit target angle (setpoint) to appropriate limits
        if(target_angle < -800)
            target_angle = -800;
        if(target_angle > 800)
            target_angle = 800;

        // calculate error for overall system following PID-approach
        double error = target_angle - current_angle;
        double dt = ((curr_can_time_angle-this->prev_can_time_angle)/1000000);
        this->integral += error*dt;
        if(this->Ki*this->integral > max_error)
            this->integral = max_error / this->Ki;
        else if(this->Ki*this->integral < -max_error)
            this->integral = -max_error / this->Ki;

        double derivative = (error - this->prev_error)/dt;
        int error_final = (int)(this->Kp*error + this->Ki*this->integral + this->Kd*derivative);
        this->prev_error = error;
        this->prev_can_time_angle = curr_can_time_angle;

        if(error_final < 0)
        {
            right_error_final = 0;
            left_error_final = std::min(max_error, -error_final);
        }
        else
        {
            left_error_final = 0;
            right_error_final = std::min(max_error, error_final);
        }

        /*int16_t curr_min_target_left = target_angle - current_angle;
        if(curr_min_target_left < 0)
        {
            if(curr_min_target_left < - 500)
                left_error_final = 500;
            else
                left_error_final = -curr_min_target_left; //curr_min_target_2 *1000/(500*500);
        }

        int16_t curr_min_target_right = current_angle - target_angle;
        if(curr_min_target_right < 0)
        {
            if(curr_min_target_right < - 500)
                right_error_final = 500;
            else
                right_error_final = -curr_min_target_right; //curr_min_target_2 *1000/(500*500);
        }*/

        //::std::cout << "target: " << target_angle << "\tcurrent: " << current_angle << "\terror left:" << left_error_final << "\terror right: " << right_error_final << ::std::endl;



        // Send out via serial connection: target angle (or setpoint), error for right muscle, error for left muscle

        QString valueHexString = QString::number(target_angle + 800, 16);
        QString string = "@FEFFFE30.00000";
        for(int i=0; i<3-valueHexString.length(); i++)
            string.append("0");
        string += valueHexString;
        string.append("\n");
        QByteArray data(string.toStdString().c_str());

        //qDebug() << "setCurrent2: " << string << " (" << string.length() << ")";

        DataProvider::getInstance()->serial->write(data);

        // TODO: needed? but it really hurts here, and slows down everything!!
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        valueHexString = QString::number(right_error_final, 16);
        string = "@FEFFFE31.00000";
        for(int i=0; i<3-valueHexString.length(); i++)
            string.append("0");
        string += valueHexString;
        string.append("\n");
        QByteArray data2(string.toStdString().c_str());

        //qDebug() << "errorCurrent2: " << string << " (" << string.length() << ")";

        DataProvider::getInstance()->serial->write(data2);

        // TODO: needed? but it really hurts here, and slows down everything!!
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        valueHexString = QString::number(left_error_final, 16);
        string = "@FEFFFE32.00000";
        for(int i=0; i<3-valueHexString.length(); i++)
            string.append("0");
        string += valueHexString;
        string.append("\n");
        QByteArray data3(string.toStdString().c_str());

        //qDebug() << "errorCurrent2: " << string << " (" << string.length() << ")";

        DataProvider::getInstance()->serial->write(data3);

        // TODO: needed? but it really hurts here, and slows down everything!!
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        valueHexString = QString::number(left_error_final, 16);
        string = "@FEFFFE40.00000";
        for(int i=0; i<3-valueHexString.length(); i++)
            string.append("0");
        string += valueHexString;
        string.append("\n");
        QByteArray data4(string.toStdString().c_str());

        DataProvider::getInstance()->serial->write(data4);

        // TODO: needed? but it really hurts here, and slows down everything!!
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        valueHexString = QString::number(right_error_final, 16);
        string = "@FEFFFE41.00000";
        for(int i=0; i<3-valueHexString.length(); i++)
            string.append("0");
        string += valueHexString;
        string.append("\n");
        QByteArray data5(string.toStdString().c_str());

        DataProvider::getInstance()->serial->write(data5);

        // TODO: needed? but it really hurts here, and slows down everything!!
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    this->ui->controlPlot->graph(0)->addData(QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0, left_error_final);
    this->ui->controlPlot->graph(1)->addData(QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0, right_error_final);
    this->ui->controlPlot->graph(2)->addData(QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0, target_angle);
    this->ui->controlPlot->graph(3)->addData(QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0, current_angle);
}

void ControlTab::toggleMode()
{
    modeAutoTraj = !modeAutoTraj;
    if(modeAutoTraj)
    {
        this->ui->valueSlider->setDisabled(true);
    }
    else
    {
        this->ui->valueSlider->setEnabled(true);
    }

    //std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    //DataProvider::getInstance()->canInterface->stop();
}

void ControlTab::setValue(int newValue)
{
    this->ui->valueEdit->setText(QString::number(newValue));
}
