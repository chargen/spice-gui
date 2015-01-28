/*
 * muscleDriverCANInterface.h
 *
 *  Created on: 17 Dec 2013
 *      Author: alex
 */

#ifndef MUSCLEDRIVERCANINTERFACE_H_
#define MUSCLEDRIVERCANINTERFACE_H_

#include <stdint.h>

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QMap>
#include <QFile>
#include <QElapsedTimer>

#include "KvaserCanInterface.h"
#include "../qcustomplot.h"

//for a fast 1 DOF system we only use 2 drivers with a 1ms control loop,
//otherwise the CAN bandwidth is not sufficient.

#define MAX_DRIVERS_AND_JOINTS 2

union motorControlCommand{
	char data[8];
	struct{
		float dutyCycle;
		float free;
    } s;
};

union motorDataSet1 {
	char data[8];
    struct {
	  float omega;
	  int32_t encoderPosition;
    } s;
};

union motorDataSet2 {
	char data[8];
    struct {
		int16_t current;
		int16_t displacement;
		uint16_t free1;
		uint16_t free2;
    } s;
};

union jointDataSet {
	char data[8];
    struct {
		int16_t jointPosition;
		int16_t free1;
		int16_t free2;
		int16_t free3;
    } s;
};

// overwrite CAN interface to implement Callback
class CANInstantiation : public KvaserCanInterface
{
public:
    static void rxCallback(canNotifyData * rxNotifyData);
};

// class MuscleDriverCANInterface
class MuscleDriverCANInterface : public QObject
{
    Q_OBJECT

public:
    MuscleDriverCANInterface(int cycleTimeInMilliSeconds, QCustomPlot *canPlot_ = NULL);
    ~MuscleDriverCANInterface();

    int mode() const {return m_mode;}

    void handleRxData(canNotifyData * rxNotifyData);

public slots:
    void start();
    void stop();
    void detachCAN();
    void setReference1(float ref);
    void setReference2(float ref);
    void enableLogging(bool enabled);

private slots:
    void cyclicProcessor(void);

private:
    int initCAN();
    int readInit();
    int sendMotorCommands();
    CANInstantiation myCan;
    CHANNEL_HANDLE busHandle0;
    float m_reference1;
    float m_reference2;
    int motorTXID[MAX_DRIVERS_AND_JOINTS];
    int motorRXID[MAX_DRIVERS_AND_JOINTS];
    int jointRXID[MAX_DRIVERS_AND_JOINTS];
    int jointMidPoint[MAX_DRIVERS_AND_JOINTS];

    //array with all ids
    int rxIDFields[2*MAX_DRIVERS_AND_JOINTS + MAX_DRIVERS_AND_JOINTS]; //each motor has 2 tx message and each joint has one
    QMap<int, int> CANIDmap;
    //array with the associated data,  motor drives with two messages each plus  joint
    char rxDataFields[2*MAX_DRIVERS_AND_JOINTS + MAX_DRIVERS_AND_JOINTS] [8];
    //signals:
    //    void valueChanged(int newValue);

private:
    motorDataSet1 motorTransmitData[MAX_DRIVERS_AND_JOINTS];
    motorDataSet2 motorTransmitAuxData[MAX_DRIVERS_AND_JOINTS];
    jointDataSet jointData[MAX_DRIVERS_AND_JOINTS];
    motorControlCommand motorCommand[MAX_DRIVERS_AND_JOINTS];

    int newDataArrived;
    int motorDriveOn;
    bool loggingEnabled;
    QElapsedTimer elapsedTime;
    QString logFileName;
    QFile outputFile;
    QMutex mutex;

    void bufferCANData();
    void printRxData(void);
    void transmitMotorControlCommand(void);
    void logAllData(unsigned long time);
    unsigned long accutime;

    unsigned int m_cycleCount;
    int m_mode;
    int count;
    QThread *timerThread;
    // QThread  *userInterfaceThread;
    qint64 nanoSec;

    QCustomPlot *canPlot;
};

#endif /* MUSCLEDRIVERCANINTERFACE_H_ */
