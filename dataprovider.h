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

class MainWindow;

class QCustomPlot;

class DataProvider : public QObject
{
    Q_OBJECT

public:
    // on each core lies at most one Subvertex

    struct VertexInfo {
        VertexInfo()
            : id(0), graphOffset(0), name(""), model(""), popSize(0) {}
        VertexInfo(uint id_, uint graphOffset_, std::string name_, std::string model_, uint popSize_)
            : id(id_), graphOffset(graphOffset_), name(name_), model(model_), popSize(popSize_) {}

        uint id;
        uint graphOffset;
        std::string name;
        std::string model;
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

    void setSpikePlot(QCustomPlot *spikePlot_);

signals:

public slots:

private slots:
    void readData();
    void reportChanged(const QString& path);
    void displayError(QAbstractSocket::SocketError socketError);

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

    QCustomPlot *spikePlot;

    QFileSystemWatcher* watcher;
};

#endif // DATAPROVIDER_H
