#ifndef CANDATAPROVIDER_H
#define CANDATAPROVIDER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>

#include "can/muscleDriverCANInterface.h"

class CanDataProvider : public QObject
{
    Q_OBJECT

public:
    static CanDataProvider* getInstance();

    std::array<motorDataSet1,MAX_DRIVERS_AND_JOINTS> getLatestMotorDataSet1();
    std::array<motorDataSet2,MAX_DRIVERS_AND_JOINTS> getLatestMotorDataSet2();
    std::array<jointDataSet,MAX_DRIVERS_AND_JOINTS> getLatestJointDataSet();
    unsigned long getLatestMotorDataSet1TimeMicroSec();
    unsigned long getLatestMotorDataSet2TimeMicroSec();
    unsigned long getLatestJointDataSetTimeMicroSec();

    void setMotorDataSet1(std::array<motorDataSet1,MAX_DRIVERS_AND_JOINTS>* newMotorDataSet1, unsigned long newMotorDataSet1TimeMicroSec);
    void setMotorDataSet2(std::array<motorDataSet2,MAX_DRIVERS_AND_JOINTS>* newMotorDataSet2, unsigned long newMotorDataSet2TimeMicroSec);
    void setJointDataSet(std::array<jointDataSet,MAX_DRIVERS_AND_JOINTS>* newJointDataSet, unsigned long newJointDataSetTimeMicroSec);

private:
    static CanDataProvider* canDataProvider;
    explicit CanDataProvider(QObject *parent = 0);
    virtual ~CanDataProvider();

    QMutex mutexMotorDataSet1;
    std::array<motorDataSet1,MAX_DRIVERS_AND_JOINTS> latestMotorDataSet1;
    unsigned long latestMotorDataSet1TimeMicroSec;

    QMutex mutexMotorDataSet2;
    std::array<motorDataSet2,MAX_DRIVERS_AND_JOINTS> latestMotorDataSet2;
    unsigned long latestMotorDataSet2TimeMicroSec;

    QMutex mutexJointDataSet;
    std::array<jointDataSet,MAX_DRIVERS_AND_JOINTS> latestJointDataSet;
    unsigned long latestJointDataSetTimeMicroSec;
};

#endif // CANDATAPROVIDER_H
