#include "dataprovider.h"
#include "settingsdialog.h"
#include "mainwindow.h"
#include "qcustomplot.h"

#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

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

    this->mainWindow = NULL;
    this->spikePlot = NULL;

    this->watcher = NULL;

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(udpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

DataProvider::~DataProvider()
{
    delete host;
    delete udpSocket;
    delete watcher;
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
    if(udpSocket->bind(*host, port))
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
    // parse latest report:
    //  1) time_stamp
    //  2) placement_by_vertex.rpt
    //  3) placement_by_core.rpt


    // 1) time_stamp
    reportID.clear();
    std::ifstream reportFile1("/home/sjentzsch/HBP/SpiNNaker/spinnaker_package_jun14/reports/latest/time_stamp");
    if(reportFile1.is_open())
    {
        std::getline(reportFile1, reportID);
        reportFile1.close();
    }
    else
        return false;


    // 2) placement_by_vertex.rpt
    vecVertices.clear();
    std::ifstream reportFile3("/home/sjentzsch/HBP/SpiNNaker/spinnaker_package_jun14/reports/latest/placement_by_vertex.rpt");
    std::string strReport3;
    if(reportFile3.is_open())
    {
        std::stringstream buffer;
        buffer << reportFile3.rdbuf();
        strReport3 = buffer.str();
        reportFile3.close();
    }
    else
        return false;

    // split by vertices
    QStringList strVertices = QString::fromStdString(strReport3).split(QString("**** Vertex:"), QString::SkipEmptyParts);
    strVertices.removeFirst();
    uint currGraphOffset = 0;
    for(QStringList::Iterator strVertex = strVertices.begin(); strVertex != strVertices.end(); ++strVertex)
    {
        // read in vertex infos
        QRegExp regVertexInfo("^\\s*'(.*)'\\s*Model:\\s(.*\\S)\\s*Pop\\ssz:\\s(\\d+)\\s*Sub-vertices:\\s*(?:Slice\\s(\\d+):(\\d+)\\s\\((\\d+)\\satoms\\)\\son\\score\\s\\((\\d+),\\s(\\d+),\\s(\\d+)\\)\\s*)+$");
        if(regVertexInfo.indexIn(*strVertex) < 0)
            return false;

        vecVertices.push_back(VertexInfo(vecVertices.size(), currGraphOffset, regVertexInfo.cap(1).toStdString(), regVertexInfo.cap(2).toStdString(), regVertexInfo.cap(3).toUInt()));

        currGraphOffset += vecVertices.back().popSize;

        qDebug() << "Added Vertex" << regVertexInfo.cap(1) << "of model" << regVertexInfo.cap(2) << "with population size" << regVertexInfo.cap(3);
    }
    qDebug() << "=> we got" << QString::number(vecVertices.size()) << "vertices.";


    // 3) placement_by_core.rpt
    mapPopByCoord.clear();
    std::ifstream reportFile("/home/sjentzsch/HBP/SpiNNaker/spinnaker_package_jun14/reports/latest/placement_by_core.rpt");
    std::string strReport;
    if(reportFile.is_open())
    {
        std::stringstream buffer;
        buffer << reportFile.rdbuf();
        strReport = buffer.str();
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

            VertexInfo* vertex = NULL;
            for(size_t i=0; i<vecVertices.size(); i++)
            {
                if(vecVertices.at(i).name == sviName && vecVertices.at(i).model == sviModel && vecVertices.at(i).popSize == sviPopSize)
                    vertex = &vecVertices.at(i);
            }
            if(vertex == NULL)
                return false;

            mapPopByCoord[std::make_tuple(chip_x, chip_y, core_id)] = SubvertexInfo(vertex, sviSliceStart, sviSliceEnd, sviSliceLength);
        }
    }

    // prepare the graphs
    this->spikePlot->clearGraphs();
    for(size_t i=0; i<vecVertices.size(); i++)
    {
        Qt::GlobalColor color = Qt::black;
        if(i == 1) color = Qt::red;
        else if(i == 2) color = Qt::blue;
        else if(i == 3) color = Qt::darkGreen;

        for(size_t u=0; u<vecVertices.at(i).popSize; u++)
        {
            this->spikePlot->addGraph();
            this->spikePlot->graph(this->spikePlot->graphCount()-1)->setPen(QPen(color));
            this->spikePlot->graph(this->spikePlot->graphCount()-1)->setLineStyle(QCPGraph::lsNone);
            this->spikePlot->graph(this->spikePlot->graphCount()-1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
        }
    }
    QVector<double> tickVector;
    QVector<QString> tickVectorLabels;
    for(int i=0; i<this->spikePlot->graphCount(); i++)
        tickVector.push_back(i);
    for(size_t v=0; v<vecVertices.size(); v++)
    {
        tickVectorLabels.push_back(QString::fromStdString(vecVertices.at(v).name)+" "+QString::number(1));
        for(uint p=1; p<vecVertices.at(v).popSize; p++)
            tickVectorLabels.push_back(QString::number(p+1));
    }
    this->spikePlot->yAxis->setTickVector(tickVector);
    this->spikePlot->yAxis->setTickVectorLabels(tickVectorLabels);
    this->spikePlot->yAxis->setRange(0, this->spikePlot->graphCount());
    this->spikePlot->replot();

    return true;
}

void DataProvider::setupReportWatcher()
{
    watcher = new QFileSystemWatcher(this);
    // use a file as watch-path which is definitely be written after the files we are interested in
    watcher->addPath("/home/sjentzsch/HBP/SpiNNaker/spinnaker_package_jun14/reports/latest/chip_sdram_usage_by_core.rpt");
    connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(reportChanged(const QString&)));
    connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(reportChanged(const QString&)));

    if(!this->parseLatestReport())
        qDebug() << "Error: parse latest report was *not* successfully completed.";
}

void DataProvider::reportChanged(const QString& path)
{
    // files might not be ready yet, resp. do not exist yet (see Qt bug below)
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if(!this->parseLatestReport())
        qDebug() << "Error: parse latest report was *not* successfully completed.";

    // workaround for Qt bug: http://stackoverflow.com/questions/18300376/qt-qfilesystemwatcher-singal-filechanged-gets-emited-only-once
    QFileInfo checkFile(path);
    while(!checkFile.exists())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    watcher->addPath(path);
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

        QDataStream dataStream(datagram.mid(14));
        dataStream.setByteOrder(QDataStream::LittleEndian);
        quint32 scpTime, scpNumSpikes, scpArg3;
        dataStream >> scpTime;
        dataStream >> scpNumSpikes;
        dataStream >> scpArg3;
        qDebug() << "DATA EXTRACT: " << scpTime << " | " << scpNumSpikes << "|" << scpArg3;

        for(quint32 i=0; i<scpNumSpikes; i++)
        {
            // TODO: does this always produce the right results in the end? what if bytes are swapped in another way (e.g. twice the half)?
            quint8 dataChipX, dataChipY, dataCoreID, dataS;
            quint16 dataLeftover, dataNeuronID;
            dataStream >> dataLeftover;
            dataS = (dataLeftover >> 11) & 0x10;
            dataCoreID = (dataLeftover >> 11) & 0xF;
            dataNeuronID = (dataLeftover & 0x7FF);
            dataStream >> dataChipY;
            dataStream >> dataChipX;

            SubvertexInfo* subvertex = &mapPopByCoord.at(std::make_tuple(dataChipX, dataChipY, dataCoreID));

            qDebug() << "=> Spike: " << dataChipX << " | " << dataChipY << " | " << dataS << " | " << dataCoreID << " | " << dataNeuronID << " | " << QString::fromStdString(subvertex->vertex->model) << " | " << QString::fromStdString(subvertex->vertex->name);

            if(this->spikePlot != NULL)
            {
                double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
                uint graphID = subvertex->vertex->graphOffset + dataNeuronID;
                this->spikePlot->graph(graphID)->addData(key, graphID);
            }
        }
    }
}

void DataProvider::setSpikePlot(QCustomPlot *spikePlot_)
{
    this->spikePlot = spikePlot_;
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
