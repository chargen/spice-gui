#include "canDataProvider.h"

CanDataProvider* CanDataProvider::canDataProvider = NULL;

CanDataProvider::CanDataProvider(QObject *parent) :
    QObject(parent), mutexStreaming(), mutexMotorDataSet1(), mutexMotorDataSet2(), mutexJointDataSet()
{
    this->streaming = false;
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

bool CanDataProvider::isStreaming()
{
    QMutexLocker locker(&mutexStreaming);
    return this->streaming;
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

unsigned long CanDataProvider::getLatestMotorDataSet1TimeMicroSec()
{
    QMutexLocker locker(&mutexMotorDataSet1);
    return this->latestMotorDataSet1TimeMicroSec;
}

unsigned long CanDataProvider::getLatestMotorDataSet2TimeMicroSec()
{
    QMutexLocker locker(&mutexMotorDataSet2);
    return this->latestMotorDataSet2TimeMicroSec;
}

unsigned long CanDataProvider::getLatestJointDataSetTimeMicroSec()
{
    QMutexLocker locker(&mutexJointDataSet);
    return this->latestJointDataSetTimeMicroSec;
}

void CanDataProvider::setStreaming(bool newStreaming)
{
    QMutexLocker locker(&mutexStreaming);
    this->streaming = newStreaming;
}

void CanDataProvider::setMotorDataSet1(std::array<motorDataSet1,MAX_DRIVERS_AND_JOINTS>* newMotorDataSet1, unsigned long newMotorDataSet1TimeMicroSec)
{
    QMutexLocker locker(&mutexMotorDataSet1);
    this->latestMotorDataSet1 = *newMotorDataSet1;
    this->latestMotorDataSet1TimeMicroSec = newMotorDataSet1TimeMicroSec;
}

void CanDataProvider::setMotorDataSet2(std::array<motorDataSet2,MAX_DRIVERS_AND_JOINTS>* newMotorDataSet2, unsigned long newMotorDataSet2TimeMicroSec)
{
    QMutexLocker locker(&mutexMotorDataSet2);
    this->latestMotorDataSet2 = *newMotorDataSet2;
    this->latestMotorDataSet2TimeMicroSec = newMotorDataSet2TimeMicroSec;
}

void CanDataProvider::setJointDataSet(std::array<jointDataSet,MAX_DRIVERS_AND_JOINTS>* newJointDataSet, unsigned long newJointDataSetTimeMicroSec)
{
    QMutexLocker locker(&mutexJointDataSet);
    this->latestJointDataSet = *newJointDataSet;
    this->latestJointDataSetTimeMicroSec = newJointDataSetTimeMicroSec;
}
