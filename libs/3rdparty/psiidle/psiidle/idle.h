/*
 * idle.h - detect desktop idle time
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

#ifndef IDLE_H
#define IDLE_H

#include <QtCore/QObject>

class IdlePlatform;

#if (defined(WIN32) || defined(_WIN32))
	#if defined(BUILDING_PSIIDLE)
		#define PSIIDLE_EXTERN __declspec(dllexport)
	#else
		#define PSIIDLE_EXTERN __declspec(dllimport)
	#endif
#else
	#define PSIIDLE_EXTERN
#endif

class PSIIDLE_EXTERN Idle : public QObject
{
	Q_OBJECT
public:
	Idle(QObject *parent=0);
	~Idle();

	bool isActive() const;
	bool usingPlatform() const;
	void start();
	void stop();

Q_SIGNALS:
	void secondsIdle(int);

private Q_SLOTS:
	void doCheck();

private:
	
	bool usePlatform(); // VOXOX -ASV- 08-07-2009
	
	class Private;
	Private *d;
};

class IdlePlatform
{
public:
	IdlePlatform();
	~IdlePlatform();

	bool init();
	int secondsIdle();

private:
	class Private;
	Private *d;
};

#endif
