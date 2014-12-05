#include "serialTab.h"
#include "ui_serialtab.h"
#include "mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include "dataprovider.h"

#include <QMessageBox>
#include <QtSerialPort/QSerialPort>

SerialTab::SerialTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialTab)
{
    ui->setupUi(this);
    console = ui->plainTextEdit;
    //console->setEnabled(false);

    connect(DataProvider::getInstance()->serial, SIGNAL(readyRead()), this, SLOT(readData()));
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

void SerialTab::writeData(const QByteArray &data)
{
    DataProvider::getInstance()->serial->write(data);
}

void SerialTab::readData()
{
    QByteArray data = DataProvider::getInstance()->serial->readAll();
    console->putData(data);
}

void SerialTab::clear()
{
    console->clear();
}
