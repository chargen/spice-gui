#include "serialTab.h"
#include "ui_serialtab.h"
#include "mainwindow.h"
#include "console.h"
#include "settingsdialog.h"

#include <QMessageBox>
#include <QtSerialPort/QSerialPort>

SerialTab::SerialTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialTab)
{
    ui->setupUi(this);
    console = ui->plainTextEdit;
    //console->setEnabled(false);
    serial = new QSerialPort(this);

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));

    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));
}

SerialTab::~SerialTab()
{
    delete ui;
}

void SerialTab::setMainWindow(MainWindow* mainWindow_)
{
    this->mainWindow = mainWindow_;
}

void SerialTab::openSerialPort()
{
    SettingsDialog::Settings p = SettingsDialog::getInstance()->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite)) {
            //console->setEnabled(true);
            console->setLocalEchoEnabled(p.localEchoEnabled);
            mainWindow->setEnabledConnect(false);
            mainWindow->setEnabledDisconnect(true);
            mainWindow->showMessageStatusBar(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                       .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        mainWindow->showMessageStatusBar(tr("Open error"));
    }
}

void SerialTab::closeSerialPort()
{
    serial->close();
    //console->setEnabled(false);
    mainWindow->setEnabledConnect(true);
    mainWindow->setEnabledDisconnect(false);
    mainWindow->showMessageStatusBar(tr("Disconnected"));
}

void SerialTab::writeData(const QByteArray &data)
{
    serial->write(data);
}

void SerialTab::readData()
{
    QByteArray data = serial->readAll();
    console->putData(data);
}

void SerialTab::clear()
{
    console->clear();
}

void SerialTab::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}
