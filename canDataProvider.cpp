#include "canDataProvider.h"

CanDataProvider* CanDataProvider::canDataProvider = NULL;

CanDataProvider::CanDataProvider(QObject *parent) :
    QObject(parent), mutexMotorDataSet1(), mutexMotorDataSet2(), mutexJointDataSet()
{

}

CanDataProvider::~CanDataProvider()
{

}

CanDataProvider* CanDataProvider::getInstance()
{
    if(canDataProvider == NULL)
        canDataProvider = new CanDataProvider();
    return canDataProvider;
}

std::array<motorDataSet1,MAX_DRIVERS_AND_JOINTS> CanDataProvider::getLatestMotorDataSet1()
{
    QMutexLocker locker(&mutexMotorDataSet1);
    return this->latestMotorDataSet1;
}

std::array<motorDataSet2,MAX_DRIVERS_AND_JOINTS> CanDataProvider::getLatestMotorDataSet2()
{
    QMutexLocker locker(&mutexMotorDataSet2);
    return this->latestMotorDataSet2;
}

std::array<jointDataSet,MAX_DRIVERS_AND_JOINTS> CanDataProvider::getLatestJointDataSet()
{
    QMutexLocker locker(&mutexJointDataSet);
    return this->latestJointDataSet;
}

void CanDataProvider::setMotorDataSet1(std::array<motorDataSet1,MAX_DRIVERS_AND_JOINTS>* newMotorDataSet1)
{
    QMutexLocker locker(&mutexMotorDataSet1);
    this->latestMotorDataSet1 = *newMotorDataSet1;
}

void CanDataProvider::setMotorDataSet2(std::array<motorDataSet2,MAX_DRIVERS_AND_JOINTS>* newMotorDataSet2)
{
    QMutexLocker locker(&mutexMotorDataSet2);
    this->latestMotorDataSet2 = *newMotorDataSet2;
}

void CanDataProvider::setJointDataSet(std::array<jointDataSet,MAX_DRIVERS_AND_JOINTS>* newJointDataSet)
{
    QMutexLocker locker(&mutexJointDataSet);
    this->latestJointDataSet = *newJointDataSet;
}
