#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include <iostream>
#include <map>
#include <tuple>
#include <string>
#include <vector>

#include <QObject>
#include <QUdpSocket>
#include <QFileSystemWatcher>
#include <QtSerialPort/QSerialPort>

#include "can/muscleDriverCANInterface.h"
#include "dbconnection.h"
#include "dbdata.h"

class MainWindow;

class QCustomPlot;

class DataProvider : public QObject
{
    Q_OBJECT

public:
    // on each core lies at most one Subvertex

    struct VertexInfo {
        VertexInfo()
            : id(0), graphOffset(0), graphCount(0), name(""), model(""), popSize(0) {}
        VertexInfo(uint id_, uint graphOffset_, uint graphCount_, std::string name_, std::string model_, uint popSize_)
            : id(id_), graphOffset(graphOffset_), graphCount(graphCount_), name(name_), model(model_), popSize(popSize_) {}

        /* ID of the vertex within the vector of vertices */
        uint id;

        /* global offset where the graphs of this vertex begin within the graph-list */
        uint graphOffset;

        /* number of graphs for this population <-> number of neurons which we are interested in for plotting */
        uint graphCount;

        /* label or name of this vertex */
        std::string name;

        /* name of the model used when defining this population */
        std::string model;

        /* number of neurons of this population */
        uint popSize;
    };

    struct SubvertexInfo {
        SubvertexInfo()
            : vertex(NULL), sliceStart(0), sliceEnd(0), sliceLength(0) {}
        SubvertexInfo(VertexInfo* vertex_, uint sliceStart_, uint sliceEnd_, uint sliceLength_)
            : vertex(vertex_), sliceStart(sliceStart_), sliceEnd(sliceEnd_), sliceLength(sliceLength_) {}

        VertexInfo* vertex;
        uint sliceStart;
        uint sliceEnd;
        uint sliceLength;
    };

    static DataProvider* getInstance();

    void setMainWindow(MainWindow* mainWindow_);

    void connectListener();
    void disconnectListener();

    bool parseLatestReport();

    void setupReportWatcher();

    void setupAppRunningWatcher();

    void setSpikePlot(QCustomPlot *spikePlot_);
    void setCanPlot(QCustomPlot *canPlot_);
    void setControlPlot(QCustomPlot *controlPlot_);

    qint64 getTimeSpiNNakerStartInMs() {return this->timeSpiNNakerStartInMs;}

    QSerialPort* serial;

    DBData* dbData;

    void startCan();
    void stopCan();
    MuscleDriverCANInterface* canInterface;

signals:

public slots:

private slots:
    void readData();
    void reportChanged(const QString& path);
    void appRunning(const QString& path);
    void displayError(QAbstractSocket::SocketError socketError);
    void handleError(QSerialPort::SerialPortError error);
    void openSerialPort();
    void closeSerialPort();
    void saveAllPlots();
    void calcSpikeRates();

private:
    static DataProvider* dataProvider;
    explicit DataProvider(QObject *parent = 0);
    virtual ~DataProvider();

    MainWindow *mainWindow;
    QUdpSocket *udpSocket;
    QHostAddress* host;
    quint16 port;

    std::map< std::tuple< size_t, size_t, size_t >, SubvertexInfo > mapPopByCoord;
    std::string reportID;
    std::vector< VertexInfo > vecVertices;
    qint64 timeSpiNNakerStartInMs;

    QCustomPlot *spikePlot;
    QCustomPlot *canPlot;
    QCustomPlot *controlPlot;

    QFileSystemWatcher* watcher;
    QFileSystemWatcher* watcher2;

    DBConnection* dbConnection;

    QMutex mutex;
    float dutyCycle[MAX_DRIVERS_AND_JOINTS];
    float omega[MAX_DRIVERS_AND_JOINTS];
    int32_t encoderPosition[MAX_DRIVERS_AND_JOINTS];
    int16_t current[MAX_DRIVERS_AND_JOINTS];
    int16_t displacement[MAX_DRIVERS_AND_JOINTS];
    int16_t jointPosition[MAX_DRIVERS_AND_JOINTS];
};

#endif // DATAPROVIDER_H
