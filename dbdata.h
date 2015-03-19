#ifndef DBDATA_H
#define DBDATA_H

#include <iostream>

#include <QObject>
#include <QtSql>

class DBData : public QObject
{
    Q_OBJECT

public:
    DBData(QObject *parent = 0);
    ~DBData();

    void insertSpike(double time, uint population, uint neuron);
    //void insertSpike(double time, uint population, uint neuron);

    void setSpikeRecord(bool _active);
    void setControlRecord(bool _active);

private:
    QSqlDatabase db;
    QSqlQuery* insertQuerySpike;
    bool spikeRecord;
    bool controlRecord;
};

#endif // DBDATA_H
