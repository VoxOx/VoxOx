/*
 * idle.cpp - detect desktop idle time
 * Copyright (C) 2003  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "idle.h"

#include <qcursor.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <QtGui/QApplication>

static IdlePlatform *platform = 0;
static int platform_ref = 0;

class Idle::Private
{
public:
	Private() {}

	QPoint lastMousePos;
	QDateTime idleSince;

	bool active;
	int idleTime;
	QDateTime startTime;
	QTimer checkTimer;
};

Idle::Idle(QObject *parent)
: QObject(parent)
{
	d = new Private;
	d->active = false;
	d->idleTime = 0;

	// try to use platform idle
	if(!platform) {
		IdlePlatform *p = new IdlePlatform;
		if(p->init())
			platform = p;
		else
			delete p;
	}
	if(platform)
		++platform_ref;

	connect(&d->checkTimer, SIGNAL(timeout()), SLOT(doCheck()));
}

Idle::~Idle()
{
	if(platform) {
		--platform_ref;
		if(platform_ref == 0) {
			delete platform;
			platform = 0;
		}
	}
	delete d;
}

bool Idle::isActive() const
{
	return d->active;
}

bool Idle::usingPlatform() const
{
	return (platform ? true: false);
}

void Idle::start()
{
	d->startTime = QDateTime::currentDateTime();

	//if(!platform) {
		// generic idle
		d->lastMousePos = QCursor::pos();
		d->idleSince = QDateTime::currentDateTime();
	//}

	// poll every second (use a lower value if you need more accuracy)
	d->checkTimer.start(1000);
}

void Idle::stop()
{
	d->checkTimer.stop();
}

void Idle::doCheck()
{
	int i;

	if(usePlatform()) // VOXOX -ASV- 08-07-2009 
		i = platform->secondsIdle();
	else {
		QPoint curMousePos = QCursor::pos();
		QDateTime curDateTime = QDateTime::currentDateTime();
		if(d->lastMousePos != curMousePos) {
			d->lastMousePos = curMousePos;
			d->idleSince = curDateTime;
		}
		i = d->idleSince.secsTo(curDateTime);
	}

	// set 'beginIdle' to the beginning of the idle time (by backtracking 'i' seconds from now)
	QDateTime beginIdle = QDateTime::currentDateTime().addSecs(-i);

	// set 't' to hold the number of seconds between 'beginIdle' and 'startTime'
	int t = beginIdle.secsTo(d->startTime);

	// beginIdle later than (or equal to) startTime?
	if(t <= 0) {
		// scoot ourselves up to the new idle start
		d->startTime = beginIdle;
	}
	// beginIdle earlier than startTime?
	else if(t > 0) {
		// do nothing
	}

	// how long have we been idle?
	int idleTime = d->startTime.secsTo(QDateTime::currentDateTime());

	secondsIdle(idleTime);
}

// VOXOX -ASV- 08-07-2009: This code is here because 'platform' on Mac only identifies activity if the 
// application has focus. This is why the 'if' statement differs from the Windows platform (where it 
// identifies activity even if the application is not the front most). In case the application doesn't 
// have focus on Mac we use Qt to track the mouse movement since it tracks the mouse even if the app is
// not the front most. The only problem is that if the application is not the active application we are
// not able to track keyboard input since we can only do so with 'platform'.
bool Idle::usePlatform(){
	bool result = false;

#if (defined(WIN32) || defined(_WIN32))
	if(platform)
		result = true;
#else
	if(QApplication::activeWindow())
		result = true;
#endif
	return result;
}