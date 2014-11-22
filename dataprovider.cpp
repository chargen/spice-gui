#include "dataprovider.h"
#include "settingsdialog.h"
#include "mainwindow.h"

#include <fstream>
#include <sstream>

#include <QtWidgets>
#include <QMessageBox>
#include <QtNetwork>
#include <QRegExp>
#include <QStringList>

DataProvider* DataProvider::dataProvider = NULL;

DataProvider::DataProvider(QObject *parent) :
    QObject(parent)
{
    udpSocket = new QUdpSocket(this);
    host = NULL;
    port = 0;

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(udpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

DataProvider::~DataProvider()
{
    delete host;
    delete udpSocket;
}

DataProvider* DataProvider::getInstance()
{
    if(dataProvider == NULL)
        dataProvider = new DataProvider();
    return dataProvider;
}

void DataProvider::setMainWindow(MainWindow* mainWindow_)
{
    this->mainWindow = mainWindow_;
}

void DataProvider::connectListener()
{
    if(host != NULL)
        delete host;
    host = new QHostAddress(SettingsDialog::getInstance()->settings().liveSpikeAddress);
    port = SettingsDialog::getInstance()->settings().liveSpikePort;

    mainWindow->showMessageStatusBar(QString("Binding Live-Spike-Listener to ") + host->toString() + QString(", port ") + QString::number(port));
    if(udpSocket->bind(QHostAddress("192.168.240.1"), port))
        mainWindow->showMessageStatusBar(QString("Binded Live-Spike-Listener to ") + host->toString() + QString(", port ") + QString::number(port));
    else
        mainWindow->showMessageStatusBar(QString("Binding Live-Spike-Listener failed to ") + host->toString() + QString(", port ") + QString::number(port));
}

void DataProvider::disconnectListener()
{
    // TODO
}

// TODO: obsolete, but cool. does not require Qt, nor boost.
std::vector<std::string> stringSplit(std::string s, const std::string &delim, bool removeLeadingBlock = true)
{
    std::vector<std::string> elems;

    size_t pos = 0;
    std::string token;
    bool firstRun = true;
    while((pos = s.find(delim)) != std::string::npos)
    {
        token = s.substr(0, pos);
        if(firstRun && removeLeadingBlock)
            firstRun = false;
        else
            elems.push_back(token);
        s.erase(0, pos + delim.length());
    }
    elems.push_back(s);

    return elems;
}

bool DataProvider::parseLatestReport()
{
    mapPopByCoord.clear();

    // parse latest report
    std::ifstream reportFile("/home/sjentzsch/HBP/SpiNNaker/spinnaker_package_jun14/reports/latest/placement_by_core.rpt");
    std::string strReport;
    if(reportFile.is_open())
    {
        std::stringstream buffer;
        buffer << reportFile.rdbuf();
        strReport = buffer.str();

        qDebug() << "latest report successfully opened!";
        reportFile.close();
    }
    else
        return false;

    // split by chips
    QStringList strChips = QString::fromStdString(strReport).split(QString("**** Chip:"), QString::SkipEmptyParts);
    strChips.removeFirst();
    for(QStringList::Iterator strChip = strChips.begin(); strChip != strChips.end(); ++strChip)
    {
        // read in chip x and y coordinates
        QRegExp regChipCoords("^\\s*\\((\\d+),\\s*(\\d+)\\)");
        regChipCoords.indexIn(*strChip);
        QStringList strChipCoords = regChipCoords.capturedTexts();
        if(strChipCoords.size() != 3)
            return false;
        size_t chip_x = static_cast< size_t >(strChipCoords.at(1).toUInt());
        size_t chip_y = static_cast< size_t >(strChipCoords.at(2).toUInt());

        // split by cores
        QStringList strCores = strChip->split(QString("Processor"), QString::SkipEmptyParts);
        strCores.removeFirst();
        for(QStringList::Iterator strCore = strCores.begin(); strCore != strCores.end(); ++strCore)
        {
            // read in core id and all the subvertex infos
            QRegExp regCoreId("^\\s*(\\d+):\\sVertex:\\s'(.*)',\\spop\\ssz:\\s(\\d+)\\s*Slice\\son\\sthis\\score:\\s(\\d+):(\\d+)\\s\\((\\d+)\\satoms\\)\\s*Model:\\s(.*\\S)\\s*$");
            if(regCoreId.indexIn(*strCore) < 0)
                return false;
            size_t core_id = static_cast< size_t >(regCoreId.cap(1).toUInt());

            std::string sviName = regCoreId.cap(2).toStdString();
            std::string sviModel = regCoreId.cap(7).toStdString();
            uint sviPopSize = regCoreId.cap(3).toUInt();
            uint sviSliceStart = regCoreId.cap(4).toUInt();
            uint sviSliceEnd = regCoreId.cap(5).toUInt();
            uint sviSliceLength = regCoreId.cap(6).toUInt();

            if(sviSliceEnd - sviSliceStart + 1 != sviSliceLength)
                return false;

            qDebug() << "New entry for (chip_x, chip_y, core_id) = (" << chip_x << ", " << chip_y << ", " << core_id << "): SubvertexInfo(name, model, popSize, sliceStart, sliceEnd, sliceLength) = (" << QString::fromStdString(sviName) << ", " << QString::fromStdString(sviModel) << ", " << QString::number(sviPopSize) << ", " << QString::number(sviSliceStart) << ", " << QString::number(sviSliceEnd) << ", " << QString::number(sviSliceLength) << ")";
            mapPopByCoord[std::make_tuple(chip_x, chip_y, core_id)] = SubvertexInfo(sviName, sviModel, sviPopSize, sviSliceStart, sviSliceEnd, sviSliceLength);
        }
    }

    return true;
}

void DataProvider::readData()
{
    while(udpSocket->hasPendingDatagrams())
    {
         QByteArray datagram;
         datagram.resize(udpSocket->pendingDatagramSize());

         QHostAddress senderAddress;
         quint16 senderPort;
         udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);    // TODO: might fail -> handle the case?

         qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << ": Received a package from " << senderAddress.toString() << " port " << QString::number(senderPort) << " of size " << QString::number(datagram.size()) << ": " << datagram.toHex();
    }
}

void DataProvider::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        mainWindow->showMessageStatusBar("The host was not found. Please check the host name and port settings.");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        mainWindow->showMessageStatusBar("The connection was refused by the peer. Make sure the fortune server is running, and check that the host name and port settings are correct.");
        break;
    default:
        mainWindow->showMessageStatusBar(QString("The following error occurred: %1.").arg(udpSocket->errorString()));
    }
}
