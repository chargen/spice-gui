#include "dbconnection.h"
#include "settingsdialog.h"

DBConnection::DBConnection(QObject *parent) :
    QObject(parent)
{
    SettingsDialog::Settings currentSettings = SettingsDialog::getInstance()->settings();

    // create a default connection, as we do not pass the second argument
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(currentSettings.applicationDataFilePath+"latest/visualiser_database.db");

    // TODO: check if this database even exists ... we do NOT want to create one (this would be the defauls SQLite behavior)

    if(db.open())
    {
        ::std::cout << "MY-DB OPENED!" << ::std::endl;

        QSqlQuery query;
        query.exec("SELECT chip_x, chip_y FROM Processor WHERE physical_id > 15");
        while(query.next())
        {
            QString name = query.value(0).toString();
            int salary = query.value(1).toInt();
            qDebug() << name << salary;
        }
    }
    else
    {
        ::std::cout << "MY-DB *NOT* OPENED!!" << ::std::endl;

        // call QSqlDatabase::lastError() to get error information
    }
}

DBConnection::~DBConnection()
{
    // close and remove the default database connection
    QSqlDatabase::database().close();
    QSqlDatabase::database().removeDatabase(QSqlDatabase::database().connectionName());
}
