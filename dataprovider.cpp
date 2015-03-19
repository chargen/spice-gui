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
    this->canPlot = NULL;

    this->timeLastParsedInMs = 0;

    this->watcher = NULL;

    serial = new QSerialPort(this);

    this->canInterface = NULL;

    this->dbData = new DBData(this);
    this->dbData->setSpikeRecord(false);
    this->dbData->setControlRecord(true);
    //this->dbConnection = new DBConnection(this);
    // test code starts!!

    // test code ends!!

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(udpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

DataProvider::~DataProvider()
{
    delete this->host;
    delete this->udpSocket;
    delete this->watcher;
    delete this->dbConnection;
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

    SettingsDialog::Settings currentSettings = SettingsDialog::getInstance()->settings();

    // 1) time_stamp
    reportID.clear();
    std::ifstream reportFile1(currentSettings.spynnakerCfgPath.toStdString()+"/reports/latest/time_stamp");
    while(!reportFile1.is_open())
    {
        qDebug() << "ParseLatestReport: open time_stamp file...";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        reportFile1.open(currentSettings.spynnakerCfgPath.toStdString()+"/reports/latest/time_stamp");
    }

    std::getline(reportFile1, reportID);
    reportFile1.close();


    // 2) placement_by_vertex.rpt
    vecVertices.clear();
    std::ifstream reportFile3(currentSettings.spynnakerCfgPath.toStdString()+"/reports/latest/placement_by_vertex.rpt");
    std::string strReport3;

    while(!reportFile3.is_open())
    {
        qDebug() << "ParseLatestReport: open placement_by_vertex file...";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        reportFile3.open(currentSettings.spynnakerCfgPath.toStdString()+"/reports/latest/placement_by_vertex.rpt");
    }

    std::stringstream buffer;
    buffer << reportFile3.rdbuf();
    strReport3 = buffer.str();
    reportFile3.close();

    // split by vertices
    QStringList strVertices = QString::fromStdString(strReport3).split(QString("**** Vertex:"), QString::SkipEmptyParts);
    strVertices.removeFirst();
    uint currGraphOffset = 1;
    uint numPopulationsToPlot = 0;
    for(QStringList::Iterator strVertex = strVertices.begin(); strVertex != strVertices.end(); ++strVertex)
    {
        // read in vertex infos
        QRegExp regVertexInfo("^\\s*'(.*)'\\s*Model:\\s(.*\\S)\\s*Pop\\ssz:\\s(\\d+)\\s*Sub-vertices:\\s*(?:Slice\\s(\\d+):(\\d+)\\s\\((\\d+)\\satoms\\)\\son\\score\\s\\((\\d+),\\s(\\d+),\\s(\\d+)\\)\\s*)+$");
        if(regVertexInfo.indexIn(*strVertex) < 0)
        {
            qDebug() << "ParseLatestReport: No regexp match for the vertex found.";
            return false;
        }

        // TODO: remove this; hard-code max-neurons-to-be-plotted per vertex here!!
        QString popName = regVertexInfo.cap(1);
        uint maxGraphsPerVertex = 0;
        // TODO: optimize me
        if(popName.endsWith("_PLOT"))
        {
            popName.chop(5);
            if(regVertexInfo.cap(3).toUInt() > 128)
                maxGraphsPerVertex = 128;
            else
                maxGraphsPerVertex = regVertexInfo.cap(3).toUInt();
        }

        //if(regVertexInfo.cap(1).toStdString() != "Monitor")
        //    maxGraphsPerVertex = 10;
        //if(regVertexInfo.cap(1).toStdString() == "mySourceI")
        //    maxGraphsPerVertex = 5;

        vecVertices.push_back(VertexInfo(vecVertices.size(), currGraphOffset, maxGraphsPerVertex, popName.toStdString(), regVertexInfo.cap(2).toStdString(), regVertexInfo.cap(3).toUInt()));

        currGraphOffset += maxGraphsPerVertex;
        if(maxGraphsPerVertex > 0)
        {
            numPopulationsToPlot++;
            currGraphOffset += 1;
        }

        qDebug() << "Added Vertex" << popName << "of model" << regVertexInfo.cap(2) << "with population size" << regVertexInfo.cap(3);
    }
    qDebug() << "=> we got" << QString::number(vecVertices.size()) << "vertices.";


    // 3) placement_by_core.rpt
    mapPopByCoord.clear();
    std::ifstream reportFile(currentSettings.spynnakerCfgPath.toStdString()+"/reports/latest/placement_by_core.rpt");
    std::string strReport;

    while(!reportFile.is_open())
    {
        qDebug() << "ParseLatestReport: open placement_by_core file...";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        reportFile.open(currentSettings.spynnakerCfgPath.toStdString()+"/reports/latest/placement_by_core.rpt");
    }

    std::stringstream buffer2;
    buffer2 << reportFile.rdbuf();
    strReport = buffer2.str();
    reportFile.close();

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
        {
            qDebug() << "ParseLatestReport: strChipCoords hat not the expected size of 3.";
            return false;
        }
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
            {
                qDebug() << "ParseLatestReport: No regexp match for the core found.";
                return false;
            }
            size_t core_id = static_cast< size_t >(regCoreId.cap(1).toUInt());

            // TODO: optimize me
            QString sviNameTemp = regCoreId.cap(2);
            if(sviNameTemp.endsWith("_PLOT"))
                sviNameTemp.chop(5);
            std::string sviName = sviNameTemp.toStdString();
            std::string sviModel = regCoreId.cap(7).toStdString();
            uint sviPopSize = regCoreId.cap(3).toUInt();
            uint sviSliceStart = regCoreId.cap(4).toUInt();
            uint sviSliceEnd = regCoreId.cap(5).toUInt();
            uint sviSliceLength = regCoreId.cap(6).toUInt();

            if(sviSliceEnd - sviSliceStart + 1 != sviSliceLength)
            {
                qDebug() << "ParseLatestReport: core pop slice end and start do not match the length.";
                return false;
            }

            qDebug() << "New entry for (chip_x, chip_y, core_id) = (" << chip_x << ", " << chip_y << ", " << core_id << "): SubvertexInfo(name, model, popSize, sliceStart, sliceEnd, sliceLength) = (" << QString::fromStdString(sviName) << ", " << QString::fromStdString(sviModel) << ", " << QString::number(sviPopSize) << ", " << QString::number(sviSliceStart) << ", " << QString::number(sviSliceEnd) << ", " << QString::number(sviSliceLength) << ")";

            VertexInfo* vertex = NULL;
            for(size_t i=0; i<vecVertices.size(); i++)
            {
                if(vecVertices.at(i).name == sviName && vecVertices.at(i).model == sviModel && vecVertices.at(i).popSize == sviPopSize)
                    vertex = &vecVertices.at(i);
            }
            if(vertex == NULL)
            {
                qDebug() << "ParseLatestReport: No matching vertex found.";
                return false;
            }

            mapPopByCoord[std::make_tuple(chip_x, chip_y, core_id)] = SubvertexInfo(vertex, sviSliceStart, sviSliceEnd, sviSliceLength);
        }
    }

    // set timeLastParsedInMs
    this->timeLastParsedInMs = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // prepare the graphs
    this->spikePlot->clearGraphs();
    for(size_t i=0; i<vecVertices.size(); i++)
    {
        QColor color = QColor(0, 0, 0, 70);
        if((i+0)%3 == 0) color = QColor(255, 0, 0, 70);
        else if((i+1)%3 == 0) color = QColor(0, 0, 255, 70);
        else if((i+2)%3 == 0) color = QColor(10, 100, 10, 70);
        // TODO: add more colors for the vertices/pops here

        size_t graphCountWithDummy = vecVertices.at(i).graphCount;
        if(graphCountWithDummy > 0)
            graphCountWithDummy++;
        for(size_t u=0; u<graphCountWithDummy; u++)
        {
            this->spikePlot->addGraph();
            this->spikePlot->graph(this->spikePlot->graphCount()-1)->setPen(QPen(color));
            this->spikePlot->graph(this->spikePlot->graphCount()-1)->setLineStyle(QCPGraph::lsNone);
            //this->spikePlot->graph(this->spikePlot->graphCount()-1)->setBrush(QBrush(color));
            this->spikePlot->graph(this->spikePlot->graphCount()-1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
        }
    }

    QVector<double> tickVector;
    QVector<QString> tickVectorLabels;
    for(int i=0; i<this->spikePlot->graphCount(); i++)
    {
        tickVector.push_back(i);
        tickVectorLabels.push_back("");
    }
    for(size_t v=0; v<vecVertices.size(); v++)
    {
        tickVectorLabels[vecVertices.at(v).graphOffset] = QString::fromStdString(vecVertices.at(v).name);

        /*if(true)
        {
            tickVectorLabels.push_back("");
            if(vecVertices.at(v).graphCount > 0)
                tickVectorLabels.push_back(QString::fromStdString(vecVertices.at(v).name));
            for(uint p=1; p<vecVertices.at(v).graphCount; p++)
                tickVectorLabels.push_back("");

        }
        else
        {
            tickVectorLabels.push_back("");
            if(vecVertices.at(v).graphCount > 0)
                tickVectorLabels.push_back(QString::fromStdString(vecVertices.at(v).name)+" "+QString::number(1));
            for(uint p=1; p<vecVertices.at(v).graphCount; p++)
                tickVectorLabels.push_back(QString::number(p+1));
        }*/
    }
    this->spikePlot->yAxis->setTickVector(tickVector);
    this->spikePlot->yAxis2->setTickVector(tickVector);
    this->spikePlot->yAxis->setTickVectorLabels(tickVectorLabels);
    this->spikePlot->yAxis->setRange(0, this->spikePlot->graphCount());

    // add lines for population separation
    for(size_t v=0; v<vecVertices.size(); v++)
    {
        std::cout << "vertex " << v << ": " << vecVertices.at(v).graphOffset << " | " << vecVertices.at(v).graphCount << ::std::endl;

        if(vecVertices.at(v).graphOffset > 1)
        {
            QCPItemStraightLine* line = new QCPItemStraightLine(this->spikePlot);
            line->setAntialiased(false);
            line->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            this->spikePlot->addItem(line);
            line->point1->setCoords(0, vecVertices.at(v).graphOffset-1);
            line->point2->setCoords(1, vecVertices.at(v).graphOffset-1);
        }
    }
    //arrow->start->setParentAnchor(textLabel->bottom);
    //line->set->setCoords(4, 1.6); // point to (4, 1.6) in x-y-plot coordinates
    //line->setHead(QCPLineEnding::esSpikeArrow);

    this->spikePlot->replot();

    return true;
}

void DataProvider::setupReportWatcher()
{
    // TODO: REMOVE ME!!! SHOULD NOT BE NEEDED ANYMORE SOON!!

    watcher = new QFileSystemWatcher(this);
    // use a file as watch-path which is definitely be written after the files we are interested in
    watcher->addPath(SettingsDialog::getInstance()->settings().spynnakerCfgPath+"/reports/latest/chip_sdram_usage_by_core.rpt");
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

        // TODO: uncomment or log or so ... ?!
        //qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << ": Received a package from " << senderAddress.toString() << " port " << QString::number(senderPort) << " of size " << QString::number(datagram.size()) << ": " << datagram.toHex();

        QDataStream dataStream(datagram.mid(14));
        dataStream.setByteOrder(QDataStream::LittleEndian);
        quint32 scpTime, scpNumSpikes, scpArg3;
        dataStream >> scpTime;
        dataStream >> scpNumSpikes;
        dataStream >> scpArg3;
        //qDebug() << "DATA EXTRACT: " << scpTime << " | " << scpNumSpikes << "|" << scpArg3;

        for(quint32 i=0; i<scpNumSpikes; i++)
        {
            // TODO: does this always produce the right results in the end? what if bytes are swapped in another way (e.g. twice the half)?
            quint8 dataChipX, dataChipY, dataCoreID;//, dataS;
            quint16 dataLeftover, dataNeuronID;
            dataStream >> dataLeftover;
            //dataS = (dataLeftover >> 11) & 0x10;
            dataCoreID = (dataLeftover >> 11) & 0xF;
            dataNeuronID = (dataLeftover & 0x7FF);
            dataStream >> dataChipY;
            dataStream >> dataChipX;

            SubvertexInfo* subvertex = &mapPopByCoord.at(std::make_tuple(dataChipX, dataChipY, dataCoreID));

            //qDebug() << "=> Spike: " << dataChipX << " | " << dataChipY << " | " << dataS << " | " << dataCoreID << " | " << dataNeuronID << " | " << QString::fromStdString(subvertex->vertex->model) << " | " << QString::fromStdString(subvertex->vertex->name);

            if(this->spikePlot != NULL)
            {
                // omit spikes which come from neurons we are not interested in for plotting
                if(dataNeuronID < subvertex->vertex->graphCount)
                {
                    // TODO: read in "Machine time step" from report file machine_structure.rpt and use this as scale factor

                    // should be 1000.0 for a timestep of 1.0
                    // before: 10.000
                    double key = scpTime/1000.0*0.6;//QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - this->getTimeLastParsedInMs()/1000.0;
                    uint graphID = subvertex->vertex->graphOffset + dataNeuronID;
                    this->spikePlot->graph(graphID)->addData(key, graphID);

                    // TODO: test performance !! how to avoid repetatively calls when active is false?
                    this->dbData->insertSpike(key, subvertex->vertex->graphOffset, dataNeuronID);
                }
            }
        }
    }
}

void DataProvider::setSpikePlot(QCustomPlot *spikePlot_)
{
    this->spikePlot = spikePlot_;
}

void DataProvider::setCanPlot(QCustomPlot *canPlot_)
{
    this->canPlot = canPlot_;
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

void DataProvider::openSerialPort()
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
            //console->setLocalEchoEnabled(p.localEchoEnabled);
            mainWindow->setEnabledConnect(false);
            mainWindow->setEnabledDisconnect(true);
            mainWindow->showMessageStatusBar(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                       .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        //QMessageBox::critical(this, tr("Error"), serial->errorString());

        mainWindow->showMessageStatusBar(tr("Open error"));
    }
}

void DataProvider::closeSerialPort()
{
    serial->close();
    //console->setEnabled(false);
    mainWindow->setEnabledConnect(true);
    mainWindow->setEnabledDisconnect(false);
    mainWindow->showMessageStatusBar(tr("Disconnected"));
}

void DataProvider::handleError(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::ResourceError)
    {
        mainWindow->showMessageStatusBar(QString("The following error occurred: %1.").arg(serial->errorString()));
        //QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}

void DataProvider::startCan()
{
    if(canInterface == NULL)
    {
        qDebug() << "Start CAN ...";

        // start a CAN interface with a 10ms cycle time
        canInterface = new MuscleDriverCANInterface(10, this->canPlot);
        /*QThread* canThread = new QThread;
        canInterface->moveToThread(canThread);
        canThread->start();*/
    }
}

void DataProvider::stopCan()
{
    if(canInterface != NULL)
    {
        qDebug() << "Stop CAN ...";

        canInterface->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        canInterface->detachCAN();
        // TODO fixme, crashes always :)
        delete canInterface;
        canInterface = NULL;
    }
}
