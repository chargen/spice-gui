/*
 * UserInterface.h
 *
 *  Created on: 27 Feb 2014
 *      Author: alex
 */

#ifndef USERINTERFACE_H_
#define USERINTERFACE_H_
#include <QObject>

class UserInterface : public QObject
{
	Q_OBJECT
	public:
		UserInterface();

	public slots:
		void run();

	 signals:
		void startMotors (void);
		void stopMotors(void);
		void detachFromCAN(void);
		void setReference(float ref);
		void setLoggingEnable(bool logEn);



	private:
		int  m_MuscleDriverCANInterface;
};


#endif /* USERINTERFACE_H_ */
