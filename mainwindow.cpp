#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "dataprovider.h"

#include <iostream>

#include <QMessageBox>
#include <QTabWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //this->showMaximized();
    this->setGeometry(QRect(100, 100, 600, 300));



    // uncomment me for unripped tabs!!
    QWidget* rippedOffTab = ui->tabWidget->currentWidget();
    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    rippedOffTab->setParent(0);
    rippedOffTab->setWindowFlags(Qt::Window);
    rippedOffTab->setGeometry(QRect(100, 500, 600, 300));
    rippedOffTab->show();

    rippedOffTab = ui->tabWidget->currentWidget();
    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    rippedOffTab->setParent(0);
    rippedOffTab->setWindowFlags(Qt::Window);
    rippedOffTab->setGeometry(QRect(800, 100, 600, 300));
    rippedOffTab->show();

    rippedOffTab = ui->tabWidget->currentWidget();
    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    rippedOffTab->setParent(0);
    rippedOffTab->setWindowFlags(Qt::Window);
    rippedOffTab->setGeometry(QRect(800, 500, 600, 300));
    rippedOffTab->show();



    ui->serialTabWidget->setMainWindow(this);
    ui->plotTabWidget->setMainWindow(this);
    DataProvider::getInstance()->setMainWindow(this);

    DataProvider::getInstance()->connectListener(); // TODO: move somewhere else !!

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionLogOn->setEnabled(true);
    ui->actionLogOff->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionPreferences->setEnabled(true);

    connect(ui->actionConnect, SIGNAL(triggered()), DataProvider::getInstance(), SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), DataProvider::getInstance(), SLOT(closeSerialPort()));
    connect(ui->actionSaveAllPlots, SIGNAL(triggered()), DataProvider::getInstance(), SLOT(saveAllPlots()));
    connect(ui->actionLogOn, SIGNAL(triggered()), this, SLOT(setLogOn()));
    connect(ui->actionLogOff, SIGNAL(triggered()), this, SLOT(setLogOff()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionClear, SIGNAL(triggered()), ui->serialTabWidget, SLOT(clear()));
    connect(ui->actionPreferences, SIGNAL(triggered()), SettingsDialog::getInstance(), SLOT(show()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    this->showMessageStatusBar("Not connected to serial I/O-board.");
}

MainWindow::~MainWindow()
{
    //TODO: delete (singleton) settings;
    delete ui;
}

void MainWindow::showMessageStatusBar(const QString &text, int timeout)
{
    ::std::cout << text.toStdString() << ::std::endl;
    this->ui->statusBar->showMessage(text, timeout);
}

void MainWindow::setEnabledConnect(bool enabled)
{
    ui->actionConnect->setEnabled(enabled);
}

void MainWindow::setEnabledDisconnect(bool enabled)
{
    ui->actionDisconnect->setEnabled(enabled);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About SpiNNingCerebellum"),
                       tr("This my friend is <b>top secret</b> !"));
}

void MainWindow::setLogOn()
{
    ::std::cout << "Turn on logging." << ::std::endl;

    DataProvider::getInstance()->dbData->setBusRecord(true);
    DataProvider::getInstance()->dbData->setControlRecord(true);
    DataProvider::getInstance()->dbData->setSpikeRecord(true);

    ui->actionLogOn->setEnabled(false);
    ui->actionLogOff->setEnabled(true);
}

void MainWindow::setLogOff()
{
    ::std::cout << "Turn off logging." << ::std::endl;

    DataProvider::getInstance()->dbData->setBusRecord(false);
    DataProvider::getInstance()->dbData->setControlRecord(false);
    DataProvider::getInstance()->dbData->setSpikeRecord(false);

    ui->actionLogOff->setEnabled(false);
    ui->actionLogOn->setEnabled(true);
}
