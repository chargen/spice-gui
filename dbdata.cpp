#include "dbdata.h"
#include "settingsdialog.h"

DBData::DBData(QObject *parent) :
    QObject(parent)
{
    this->spikeRecord = false;
    this->controlRecord = false;

    SettingsDialog::Settings currentSettings = SettingsDialog::getInstance()->settings();

    // create a default connection, as we do not pass the second argument
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(currentSettings.spynnakerCfgPath+"/data_latest.db");

    // TODO: check if this database even exists ... we do NOT want to create one (this would be the default SQLite behavior)

    if(db.open())
    {
        ::std::cout << "MY-DB OPENED!" << ::std::endl;

        QSqlQuery query;
        bool ret = query.exec("CREATE TABLE IF NOT EXISTS `spikes` (`time`	NUMERIC NOT NULL, `population` INTEGER NOT NULL, `neuron` INTEGER NOT NULL, PRIMARY KEY(time,population,neuron));");
        if(ret)
        {
            ::std::cout << "MY-DB CREATED (IF NOT EXISTS)!" << ::std::endl;
        }

        ret = query.exec("PRAGMA synchronous = OFF;");
        if(ret)
        {
            ::std::cout << "MY-DB STUFF1 WORKED!" << ::std::endl;
        }

        ret = query.exec("PRAGMA journal_mode = MEMORY;");
        if(ret)
        {
            ::std::cout << "MY-DB STUFF2 WORKED!" << ::std::endl;
        }

        insertQuerySpike = new QSqlQuery();

        insertQuerySpike->prepare("INSERT INTO `spikes` (time, population, neuron) "
                                    "VALUES (:time, :population, :neuron)");

        //this->insertSpike(3.56, 4, 2);
        //this->insertSpike(8.89, 0, 3);
        //this->insertSpike(13.56, 8, 27);

        /*QSqlQuery query;
        query.exec("SELECT chip_x, chip_y FROM Processor WHERE physical_id > 15");
        while(query.next())
        {
            QString name = query.value(0).toString();
            int salary = query.value(1).toInt();
            qDebug() << name << salary;
        }*/
    }
    else
    {
        ::std::cout << "MY-DB *NOT* OPENED!!" << ::std::endl;

        // call QSqlDatabase::lastError() to get error information
    }
}

DBData::~DBData()
{
    // close and remove the default database connection
    QSqlDatabase::database().close();
    QSqlDatabase::database().removeDatabase(QSqlDatabase::database().connectionName());
}

void DBData::insertSpike(double time, uint population, uint neuron)
{
    if(this->spikeRecord)
    {
        insertQuerySpike->bindValue(":time", time);
        insertQuerySpike->bindValue(":population", population);
        insertQuerySpike->bindValue(":neuron", neuron);
        insertQuerySpike->exec();
    }
}

void DBData::setSpikeRecord(bool _active)
{
    this->spikeRecord = _active;
}

void DBData::setControlRecord(bool _active)
{
    this->controlRecord = _active;
}
