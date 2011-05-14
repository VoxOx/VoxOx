/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>

#include <QtCore/QTimer>

// Uncomment the next line to use QThread
//#define USE_QT

#ifdef USE_QT
#include <QtCore/QThread>
typedef QThread ThreadClass;
#else
#include <thread/Thread.h>
typedef Thread ThreadClass;
#endif

#include <QtGui/QApplication>

#include <qtutil/QObjectThreadSafe.h>

class MyObject : public QObjectThreadSafe {
public:
	MyObject() : QObjectThreadSafe(0) {
		printf("MyObject ctor\n");
		typedef PostEvent0<void ()> MyPostEvent;
		MyPostEvent * event = new MyPostEvent(boost::bind(&MyObject::initThreadSafe, this));
		postEvent(event);
	}

private:
	void initThreadSafe() {
		printf("initThreadSafe\n");
	}
};


class MyThread : public ThreadClass {
protected:
	void run() {
		printf("MyThread started\n");
		MyObject object;
#ifdef USE_QT
		// In Qt 4.2, the thread must run an event loop to process events, so
		// these two lines force the thread to run an event loop for 2 seconds
		QTimer::singleShot(2000, this, SLOT(quit()) );
		exec();
#else
		ThreadClass::sleep(2);
#endif
		printf("MyThread waiting\n");
		printf("MyThread ended\n");
	}
};


int main(int argc, char ** argv) {
	QApplication app(argc, argv);
	printf("If the test works, you should see a 'initThreadSafe' line in the output\n");

	MyThread thread;
	thread.start();
	QTimer::singleShot(3000, &app, SLOT(quit()) );
	app.exec();
}
