/*
 * muscleDriverCANInterface.cpp
 *
 *  Created on: 18 Dec 2013
 *      Author: alex
 */

#include "muscleDriverCANInterface.h"
#include <stdio.h>
#include <iostream>
#include <ios>
#include <iomanip>

#include <QSettings>
#include <QDir>
#include <QTextStream>

//pointer to current instantiation of CAN interface
MuscleDriverCANInterface * currentCanInterface;


/*
 * This is the main timer driven event loop where
 * control functionality should be implemented.
 */


void MuscleDriverCANInterface::cyclicProcessor()
{
	static int firstTime=1;
	m_cycleCount++; //cycle counter

	/*
	 * add stuff to initialise at start-up below
	 */

	//only called once!
	if (firstTime)
	{
		accutime=0;
		elapsedTime.start();
		firstTime=0;

	}

	//just a few timing values in case we would like to record its
	//accutime=0;

	nanoSec = elapsedTime.nsecsElapsed();
	elapsedTime.restart();

	//accumulate the delta-times to micro-second precision running time
	accutime+=(unsigned long) (nanoSec+500)/1000;
	//cout<<"ns: "<<accutime<<endl;

	//make a copy of the CAN data, this is safe and contains mutex
	bufferCANData();

	//print if new data has arrived (for testing of correct bus functionality mainly)
	//this method also shows how to access the data for those who would like to code...
	//comment out if not necessary
//Fix offset of the joint to move the .
	jointData[0].s.jointPosition = (jointData[0].s.jointPosition + 2048 ) % 4096;
	//printRxData();


    /*
     * BEGIN OF CYCLIC CONTROL LOOP
     */


	/*
	 * Any processing code here
	 */

	//some hints: the float variable m_reference can be set via the user interface (signal/slot mechanism )
	//use them to change reference position or something along the lines

	//for each motor/joint (at the moment index i only 0 or 1 due to CAN bandwith)
	//the following signals are available (ints, if not indicated otherwise):

	//motorTransmitData[i].s.encoderPosition
	//motorTransmitData[i].s.omega  //float
	//motorTransmitAuxData[i].s.displacement
	//motorTransmitAuxData[i].s.current;
    //jointData[i].s.jointPosition

	//similarly each motor can be driven  via sending a duty cycle in range [-4000,4000],
	//corresponding to negative direction, 100% duty cycle and positive direction 100% duty cycle
	//the positive direction leads to a winding up of the tendon (if mounted correctly)

	//motorCommand[i].s.dutyCycle //float



	//example: send a duty cycle of reference to motor 0
	//motorCommand[0].s.dutyCycle=m_reference; //m_reference can be set via user interface during run-time
	//example: send motor 1 duty cycle to -20%
	//motorCommand[1].s.dutyCycle=-800.0;





	//simple antagonist joint position control,
	//this code should/could be put in a separate method:

	// a few variable for simple control example



    //float error;
    //float kp=3;
    //float baseDrive = 0; //base drive can be added to create pre-tension in drives
    //float pControl;
    //float centredJointPosition;
    //float reference;
    //float leftDriveValue;
    //float rightDriveValue;
    //float driveLimit=800.0;

	//here, we use the joint[0]  and set the centre position to zero
	//centredJointPosition=jointData[0].s.jointPosition - jointMidPoint[0];

	//error= (  m_reference - centredJointPosition );

    //pControl=kp *  error;

	//we mount motor driver  motor board [1] on the side of decreasing (left) joint position,
	// board [0] on the increasing side.

	//consequently we drive
  	//insert limits here if desired
    //rightDriveValue =  + pControl + baseDrive;
    //leftDriveValue  =  - pControl + baseDrive;


	//limit drive signals if desired here
	//reminder, DC between -4000 and +4000
/*
	if (rightDriveValue> driveLimit )
		rightDriveValue=driveLimit;

	//we only allow tendon pulling, so negative drives are not possible
	if (rightDriveValue < 0.0 )
		rightDriveValue = 0.0 ;

	if (leftDriveValue > driveLimit)
		leftDriveValue = driveLimit;

	//we only allow tendon pulling, so negative drives are not possible
	if (leftDriveValue < 0.0)
		leftDriveValue = 0.0;
*/

	//supply data to CAN interface

	motorCommand[0].s.dutyCycle= m_reference; //*(jointData[0].s.jointPosition - 1400 );


//	motorCommand[1].s.dutyCycle= leftDriveValue;





	/*
	 * END OF CYCLIC CONTROL LOOP
	 */






	//provide data on CAN bus
	sendMotorCommands();

	//log data if desired
	logAllData(accutime);

	//flush
	//std::cout.flush();



}









































//CAN Receive Callback, invoked when CAN data on bus.

void CANInstantiation::rxCallback(canNotifyData * rxNotifyData)
{


    //call RX method
	if (rxNotifyData->eventType==canEVENT_RX)
	{
		if (currentCanInterface!=NULL)
			currentCanInterface->handleRxData(rxNotifyData);

	}
}


void MuscleDriverCANInterface::handleRxData(canNotifyData * rxNotifyData)
{
    //int handle;
	    long id;
	    char data[8];
	    unsigned int dlc, flags;
	    unsigned long timestamp;
        //STATUS_CODE stat;


	 while (canERR_NOMSG != canRead(busHandle0, &id, data, &dlc, &flags, &timestamp))
	 {

		 QMap<int, int>::const_iterator it=CANIDmap.find(id); //find the data index from CAN ID
		 if (it!=CANIDmap.end())
		 {
			 mutex.lock();
			 newDataArrived=1; //set the new Data Flag
			 //we found the id, copy data into storage array
			 //thereby creating a local copy of the latest CAN-bus data;
			 //cout<<"id: 0x"<<hex <<id<<" "<<"datafield index: "<< it.value() ;
			 for (int i=0;i<8;i++)
			 {
				 rxDataFields[it.value()][i]=data[i];
				 //std::cout<<"  0x"<<std::hex<< (unsigned int) data[i]<<"  ";


			 }
			 //cout<<endl;
			 mutex.unlock();
		 }
	 }
}




MuscleDriverCANInterface::MuscleDriverCANInterface(int cycleTimeInMilliSeconds)
{
		m_cycleCount=0;
		accutime=0;
		motorDriveOn=0;
		loggingEnabled=false;
		m_reference=0.0 ; //default reference signal is 0

		//read init file
		readInit();

		std::cout<<"Creating CAN connection:"<<endl;
		initCAN();

		std::cout<<"Creating timer event loop:"<<endl;
		//start the timer thread

		/*
		 *


			m_thread = new QThread(this);
			QTimer* timer = new QTimer(0); // _not_ this!
			timer->setInterval(1);
			timer->moveToThread(m_thread);
			// Use a direct connection to make sure that doIt() is called from m_thread.
			connect(timer, SIGNAL(timeout()), SLOT(doIt()), Qt::DirectConnection);
			// Make sure the timer gets started from m_thread.
			QObject::connect(m_thread, SIGNAL(started()), timer, SLOT(start()));
			m_thread->start();


		 */

		 timerThread =new QThread(this);
		 //userInterfaceThread = new QThread(this);

		 cout<<"timer: "<< timerThread->currentThread()<<endl;



		//create timer with 1ms interval
		QTimer* timer = new QTimer(0);
		timer->setInterval(cycleTimeInMilliSeconds);
		//timer->start();
		//create user interface timer
		//QTimer *userInterfaceTimer = new QTimer(0);
		//userInterfaceTimer->setSingleShot(true);
		//userInterfaceTimer->setInterval(200); //every 200ms
		//userInterfaceTimer->start();

		std::cout<<"Timers Created"<<endl;

		//let timer run in this thread
		timer->moveToThread(timerThread);
		connect(timer, SIGNAL(timeout()), this, SLOT(cyclicProcessor()));
	    connect(timerThread, SIGNAL(started()), timer, SLOT(start()));





	    //userInterfaceTimer->moveToThread(userInterfaceThread);
	    //connect(userInterfaceTimer,SIGNAL(timeout()),this, SLOT(userInterface()))	;
	    //connect(userInterfaceThread, SIGNAL(started()), userInterfaceTimer, SLOT(start()));

		std::cout<<"Started Timer Event loop:"<<endl;




	    //userInterfaceThread->start();
	    timerThread->start();

	  //  timerThread->exec();
	   // userInterfaceThread->exec();

	    cout<<"timer thread:"<<timerThread->currentThreadId()<<endl;
	    //cout<<"userInterface thread:"<<userInterfaceThread->currentThreadId()<<endl;

	    //qDebug() << QThread::currentThreadId();
		//exec thread
		std::cout<<"Set-up time verification system:"<<endl;

		std::cout<<endl<<endl<<"--------------------------------"<<endl;
		cout<<"Thread ID init Thread: "<<QThread::currentThreadId()<<endl;




}

int MuscleDriverCANInterface::initCAN()
{

		 string myString;
         //CAN_MESSAGE myCanMessage, canSentMessage;
		 STATUS_CODE myCanStatus;
		 busHandle0=myCan.openCanChannel(0);

		 newDataArrived=0; //reset the new data flag


		 if (busHandle0>-1)
		 {
			 cout<<"CAN bus available:"<<endl;
			 currentCanInterface=this;

			 //configure bus
			  myCanStatus=myCan.configureCanChannel(busHandle0,CAN_BAUDRATE__1M,0); //1Mbit/s not silent
			  myCan.getErrorText(myCanStatus,myString);
			  myCan.connectCanChannel(busHandle0);
			  cout<<"CAN configured, 1Mbit/s, status is: "<<myString<<endl;
			 // cout<<"Reading CAN, waiting for START_MESSAGE:"<<endl;

			  //create Rx Callback connection

			  myCanStatus=myCan.setUprxCallback(busHandle0,CANInstantiation::rxCallback );
			  myCan.getErrorText(myCanStatus,myString);
			  cout<<"Rx call-back set-up: "<<myString<<endl;

		 }
		 else
		 {
			 cout<<"!!!Kvaser CAN bus adapter not availabl! Error!!!"<<endl;
			 return 0;
		 }
	return 1;
}


int MuscleDriverCANInterface::sendMotorCommands()
{
	 CAN_MESSAGE canSentMessage;
	  for (int i=0;i<MAX_DRIVERS_AND_JOINTS;i++)
	  {

		  //assembel CAN message
		  canSentMessage.dlc=8;
		  canSentMessage.messageID=motorRXID[i];
		  canSentMessage.extended=0;

		  //copy data
		  if (motorDriveOn==1)
		  {
			  for (int j=0;j<8;j++)
			  {
			  canSentMessage.data8[j]=motorCommand[i].data[j];
			  }
		  }
		  else
		  {
			  for (int j=0;j<8;j++)
			  {
			  			  canSentMessage.data8[j]=0;//stop motors
			  }
		  }

		  			  	//	provideConfigMessageData( (short unsigned *)(canSentMessage.data16),CANMessageCount);
		  			  	//	myCan.printCanMessage(canSentMessage);
		  myCan.sendCanMessage(busHandle0,canSentMessage);
      }

      return 0;
}

int MuscleDriverCANInterface::readInit()
{


	  for (int i=0;i<MAX_DRIVERS_AND_JOINTS;i++)
	  {
		  motorTXID[i]=-1;
		  motorRXID[i]=-1;
		  jointRXID[i]=-1;

		  motorCommand[i].s.dutyCycle=0.0; //reset motor commands
		  motorCommand[i].s.free=0.0;
		  count=0; //loop counter
	  }


	  for (int i=0;i< 3* MAX_DRIVERS_AND_JOINTS;i++)
	  {
		  rxIDFields[i]=-1;
		  for (int j=0;j<8;j++)
		  {
			  rxDataFields[i][j]=0; //reset data fields to 0
		  }

	  }

	  //realistically, we only drive 2 motors and 1 joint in a 1ms loop on CAN
	  //so, we only implement those now

	 QSettings settings( "settings.ini", QSettings::IniFormat );

	 motorTXID[0] = settings.value("nodes/MotorTxID0", -1).toUInt();
	 motorTXID[1] = settings.value("nodes/MotorTxID1", -1).toUInt();

	 motorRXID[0] = settings.value("nodes/MotorRxID0", -1).toUInt();
	 motorRXID[1] = settings.value("nodes/MotorRxID1", -1).toUInt();

	 jointRXID[0] = settings.value("nodes/JointRxID0", -1).toUInt();
	 jointRXID[1] = settings.value("nodes/JointRxID1", -1).toUInt();

	 jointMidPoint[0] =settings.value("nodes/Joint0MidPoint", 2048).toInt();
	 jointMidPoint[1] =settings.value("nodes/Joint1MidPoint", 2048).toInt();

	 logFileName =  settings.value("datalog/LogFile","default.txt").toString();

	 cout<<"Log File Name: "<<logFileName.toStdString()<<endl;

	 //the RX data is ordered in the following way

	 //motor 0:  tx data message 1
	 //motor 0:  tx data message 2
	 //joint 0:  tx data message

	// motor 1:  tx  data message 1
	 //motor 2:  tx  data message 2
	 //joint 1:  tx  data message
	// ...


	  for (int i=0;i<MAX_DRIVERS_AND_JOINTS;i++)
	  {

		  rxIDFields[(i*3) + 0]=motorTXID[i];
		  rxIDFields[(i*3) + 1]=motorTXID[i]+1;
		  rxIDFields[(i*3) + 2]=jointRXID[i];

		  if (motorTXID[i]>0)
		   	  std::cout<<"motorTXID["<<i<<"] = "<<motorTXID[i]<<endl;
		  if (jointRXID[i]>0)
		  	  std::cout<<"jointRXID["<<i<<"] = "<<jointRXID[i]<<endl;
		  if (motorRXID[i]>0)
		  	  std::cout<<"motorRXID["<<i<<"] = "<<motorRXID[i]<<endl;

		  	  std::cout<<"jointMidPoint["<<i<<"] = "<<jointMidPoint[i]<<endl;

	  }

	  for (int i=0;i<3 * MAX_DRIVERS_AND_JOINTS;i++)
	  {
		  cout<<" rxIDFields["<<i<<"]: "<<rxIDFields[i]<<endl;
		  CANIDmap[rxIDFields[i]]=i;
	  }


	  //prepare log file
		 QDir configDirectory("");
		 QString filePath = configDirectory.absolutePath();

		 filePath.append ("/logs/");
		 filePath.append(logFileName);
		 cout<<"log file path: "<<filePath.toStdString()<<endl;
		 outputFile.setFileName(filePath);

		 if (outputFile.open(QIODevice::WriteOnly))
		 {


			 cout<<"log file open!"<<endl;
			 //outputFile.close();


		 }

        return 0;
}

/*
 * make a copy of current CAN bus data
 */
void MuscleDriverCANInterface::bufferCANData()
{
	mutex.lock();
	 for (int i=0;i< MAX_DRIVERS_AND_JOINTS;i++)
	 {
			 // cout<<" rxIDFields["<<i<<"]: "<<rxIDFields[i]<<endl;
			 // CANIDmap[rxIDFields[i]]=i;
		  //the data is stored in this array
		  //rxDataFields[2*3][8]
		   for (int j=0;j<8;j++)
		   {
			   motorTransmitData[i].data[j]=rxDataFields[i*3 + 0][j];
			   motorTransmitAuxData[i].data[j]=rxDataFields[i*3 + 1][j];
		   	   jointData[i].data[j]= rxDataFields[i*3 + 2][j];
		   }


	 }
	 mutex.unlock();
}



void MuscleDriverCANInterface::printRxData(void)
{
	if (newDataArrived==1)
		{
			//print new data
			 for (int i=0;i<MAX_DRIVERS_AND_JOINTS;i++)
			 {
				 std::cout<<dec;
				 std::cout<<"index: "<<i<<" pos: "<<motorTransmitData[i].s.encoderPosition<<",   omega "<<motorTransmitData[i].s.omega<<" displ: "<<motorTransmitAuxData[i].s.displacement <<"  current: "<<motorTransmitAuxData[i].s.current<<"  joint pos: "<<jointData[i].s.jointPosition<<endl;
			 }
			 //reset new data flag
			 newDataArrived=0;
		}

}

void MuscleDriverCANInterface::logAllData(unsigned long time)
{

	if (loggingEnabled)
	{
		 count++;
		 QTextStream fileOutput(&outputFile);

		 fileOutput<<time<<",";
		 for (int i=0;i<MAX_DRIVERS_AND_JOINTS;i++)
		 {
			 ;
			 fileOutput<<motorCommand[i].s.dutyCycle<<","<<motorTransmitData[i].s.encoderPosition<<","<<motorTransmitData[i].s.omega<<"," <<motorTransmitAuxData[i].s.displacement<<","<<motorTransmitAuxData[i].s.current<<","<<jointData[i].s.jointPosition<<",";
		 }

		 fileOutput<< count<<endl;
	}
}


void MuscleDriverCANInterface::start()
{
	cout<<"MOTOR START REQUESTED!"<<endl;
	motorDriveOn=1;
}

void MuscleDriverCANInterface::stop()
{
	cout<<"MOTOR STOP REQUESTED!"<<endl;
	motorDriveOn=0;
	m_reference=0.0;
}

void MuscleDriverCANInterface::detachCAN()
{
	cout<<"CAN detached from BUS!"<<endl;
	cout<<"Restart Application to connect again."<<endl;
	motorDriveOn=0;
	outputFile.close();

	//myCan.closeCanChannel(busHandle0);
	timerThread->exit();

}

void MuscleDriverCANInterface::getReference(float ref)
{
	cout<<"REFERENCE RECEIVED: "<< ref<<endl;
	m_reference=ref;
}

void MuscleDriverCANInterface::enableLogging (bool enabled)
{
	if (enabled)
	{
		cout<<"ENABLED LOGGING"<<endl;
	}
	else
	{
		cout<<"DISABLED LOGGING"<<endl;
	}
	loggingEnabled=enabled;
}


