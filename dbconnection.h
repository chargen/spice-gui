#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <iostream>

#include <QObject>
#include <QtSql>

class DBConnection : public QObject
{
    Q_OBJECT

public:
    DBConnection(QObject *parent = 0);
    ~DBConnection();
};

#endif // DBCONNECTION_H
