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

    void insertBus(double time, int current_l, int spring_l, float pwm_l, int current_r, int spring_r, float pwm_r);
    void insertControl(double time, int current_angle, int target_angle, int left_error, int right_error);
    void insertSpike(double time, QString population, uint neuron);

    void setBusRecord(bool _active);
    void setControlRecord(bool _active);
    void setSpikeRecord(bool _active);

    void clearAllData();
    void setupDB();

private:
    QSqlDatabase db;

    QSqlQuery* insertQueryBus;
    QSqlQuery* insertQueryControl;
    QSqlQuery* insertQuerySpike;

    bool busRecord;
    bool controlRecord;
    bool spikeRecord;
};

#endif // DBDATA_H
