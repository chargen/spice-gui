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
    this->showMaximized();

    ui->serialTabWidget->setMainWindow(this);
    ui->plotTabWidget->setMainWindow(this);
    DataProvider::getInstance()->setMainWindow(this);

    DataProvider::getInstance()->connectListener(); // TODO: move somewhere else !!
    DataProvider::getInstance()->parseLatestReport(); // TODO: move somewhere else !!

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionPreferences->setEnabled(true);

    connect(ui->actionConnect, SIGNAL(triggered()), ui->serialTabWidget, SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), ui->serialTabWidget, SLOT(closeSerialPort()));
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