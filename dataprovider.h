#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include <iostream>
#include <map>
#include <tuple>
#include <string>

#include <QObject>
#include <QUdpSocket>

class MainWindow;

class DataProvider : public QObject
{
    Q_OBJECT

public:
    struct SubvertexInfo {
        SubvertexInfo()
            : name(""), model(""), popSize(0), sliceStart(0), sliceEnd(0), sliceLength(0) {}
        SubvertexInfo(std::string name_, std::string model_, uint popSize_, uint sliceStart_, uint sliceEnd_, uint sliceLength_)
            : name(name_), model(model_), popSize(popSize_), sliceStart(sliceStart_), sliceEnd(sliceEnd_), sliceLength(sliceLength_) {}

        std::string name;
        std::string model;
        uint popSize;
        uint sliceStart;
        uint sliceEnd;
        uint sliceLength;
    };

    static DataProvider* getInstance();

    void setMainWindow(MainWindow* mainWindow_);

    void connectListener();
    void disconnectListener();

    bool parseLatestReport();

signals:

public slots:

private slots:
    void readData();
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
};

#endif // DATAPROVIDER_H
