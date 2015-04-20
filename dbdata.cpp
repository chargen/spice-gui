#include "dbdata.h"
#include "settingsdialog.h"

DBData::DBData(QObject *parent) :
    QObject(parent)
{
    this->busRecord = false;
    this->controlRecord = false;
    this->spikeRecord = false;

    insertQueryBus = NULL;
    insertQueryControl = NULL;
    insertQuerySpike = NULL;

    SettingsDialog::Settings currentSettings = SettingsDialog::getInstance()->settings();

    // create a default connection, as we do not pass the second argument
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(currentSettings.spynnakerCfgPath+"/data_latest.db");

    // TODO: check if this database even exists ... we do NOT want to create one (this would be the default SQLite behavior)

    if(db.open())
    {
        ::std::cout << "Opened Database." << ::std::endl;
        this->setupDB();
    }
    else
    {
        ::std::cerr << "Error: Could not open database!" << ::std::endl;
        // call QSqlDatabase::lastError() to get error information
    }
}

DBData::~DBData()
{
    // close and remove the default database connection
    QSqlDatabase::database().close();
    QSqlDatabase::database().removeDatabase(QSqlDatabase::database().connectionName());
}

void DBData::insertBus(double time, uint current_l, uint spring_l, float pwm_l, uint current_r, uint spring_r, float pwm_r)
{
    if(this->busRecord)
    {
        insertQueryBus->bindValue(":time", time);
        insertQueryBus->bindValue(":current_l", current_l);
        insertQueryBus->bindValue(":spring_l", spring_l);
        insertQueryBus->bindValue(":pwm_l", pwm_l);
        insertQueryBus->bindValue(":current_r", current_r);
        insertQueryBus->bindValue(":spring_r", spring_r);
        insertQueryBus->bindValue(":pwm_r", pwm_r);
        insertQueryBus->exec();
    }
}

void DBData::insertControl(double time, uint current_angle, uint target_angle, uint left_error, uint right_error)
{
    if(this->controlRecord)
    {
        insertQueryControl->bindValue(":time", time);
        insertQueryControl->bindValue(":current_angle", current_angle);
        insertQueryControl->bindValue(":target_angle", target_angle);
        insertQueryControl->bindValue(":left_error", left_error);
        insertQueryControl->bindValue(":right_error", right_error);
        insertQueryControl->exec();
    }
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

void DBData::setBusRecord(bool _active)
{
    this->busRecord = _active;
}

void DBData::setControlRecord(bool _active)
{
    this->controlRecord = _active;
}

void DBData::setSpikeRecord(bool _active)
{
    this->spikeRecord = _active;
}

void DBData::setupDB()
{
    QSqlQuery query;
    bool ret = false;


    ret = query.exec("CREATE TABLE IF NOT EXISTS `bus` (`time` NUMERIC NOT NULL, `current_l` INTEGER NOT NULL, `spring_l` INTEGER NOT NULL, `pwm_l` REAL NOT NULL, `current_r` INTEGER NOT NULL, `spring_r` INTEGER NOT NULL, `pwm_r` REAL NOT NULL, PRIMARY KEY(time));");
    if(ret)
        ::std::cout << "Created table `bus` (if not already existed)." << ::std::endl;

    ret = query.exec("CREATE TABLE IF NOT EXISTS `controls` (`time` NUMERIC NOT NULL, `current_angle` INTEGER NOT NULL, `target_angle` INTEGER NOT NULL, `left_error` INTEGER NOT NULL, `right_error` INTEGER NOT NULL, PRIMARY KEY(time));");
    if(ret)
        ::std::cout << "Created table `controls` (if not already existed)." << ::std::endl;

    ret = query.exec("CREATE TABLE IF NOT EXISTS `spikes` (`time` NUMERIC NOT NULL, `population` INTEGER NOT NULL, `neuron` INTEGER NOT NULL, PRIMARY KEY(time,population,neuron));");
    if(ret)
        ::std::cout << "Created table `spikes` (if not already existed)." << ::std::endl;


    ret = query.exec("PRAGMA synchronous = OFF;");
    if(ret)
        ::std::cout << "Set: PRAGMA synchronous = OFF;" << ::std::endl;

    ret = query.exec("PRAGMA journal_mode = MEMORY;");
    if(ret)
        ::std::cout << "Set: PRAGMA journal_mode = MEMORY;" << ::std::endl;

    if(insertQueryBus != NULL)
        delete insertQueryBus;
    insertQueryBus = new QSqlQuery();
    insertQueryBus->prepare("INSERT INTO `bus` (time, current_l, spring_l, pwm_l, current_r, spring_r, pwm_r) "
                                "VALUES (:time, :current_l, :spring_l, :pwm_l, :current_r, :spring_r, :pwm_r)");

    if(insertQueryControl != NULL)
        delete insertQueryControl;
    insertQueryControl = new QSqlQuery();
    insertQueryControl->prepare("INSERT INTO `controls` (time, current_angle, target_angle, left_error, right_error) "
                                "VALUES (:time, :current_angle, :target_angle, :left_error, :right_error)");

    if(insertQuerySpike != NULL)
        delete insertQuerySpike;
    insertQuerySpike = new QSqlQuery();
    insertQuerySpike->prepare("INSERT INTO `spikes` (time, population, neuron) "
                                "VALUES (:time, :population, :neuron)");
}

void DBData::clearAllData()
{
    QSqlQuery query;
    bool ret = false;

    ret = query.exec("DROP TABLE `bus`;");
    if(ret)
        ::std::cout << "Dropped `bus`." << ::std::endl;
    else
        ::std::cerr << "FAILED to drop `bus`." << ::std::endl;

    ret = query.exec("DROP TABLE `controls`;");
    if(ret)
        ::std::cout << "Dropped `controls`." << ::std::endl;
    else
        ::std::cerr << "FAILED to drop `controls`." << ::std::endl;

    ret = query.exec("DROP TABLE `spikes`;");
    if(ret)
        ::std::cout << "Dropped `spikes`." << ::std::endl;
    else
        ::std::cerr << "FAILED to drop `spikes`." << ::std::endl;

    ret = query.exec("VACUUM;");
    if(ret)
        ::std::cout << "Success: VACUUM;" << ::std::endl;
    else
        ::std::cerr << "FAIL: VACUUM;" << ::std::endl;

    this->setupDB();
}
