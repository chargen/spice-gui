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

    void insertBus(double time, uint current_l, uint spring_l, float pwm_l, uint current_r, uint spring_r, float pwm_r);
    void insertControl(double time, uint current_angle, uint target_angle, uint left_error, uint right_error);
    void insertSpike(double time, uint population, uint neuron);

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
