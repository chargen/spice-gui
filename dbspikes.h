#ifndef DBSPIKES_H
#define DBSPIKES_H

#include <iostream>

#include <QObject>
#include <QtSql>

class DBSpikes : public QObject
{
    Q_OBJECT

public:
    DBSpikes(QObject *parent = 0);
    ~DBSpikes();

    void insertSpike(double time, uint population, uint neuron);
    void setActive(bool _active);

private:
    QSqlDatabase db;
    QSqlQuery* insertQuery;
    bool active;
};

#endif // DBSPIKES_H
