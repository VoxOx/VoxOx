/*
 * growlnotifier.h - A simple Qt interface to Growl
 *
 * Copyright (C) 2005  Remko Troncon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * You can also redistribute and/or modify this program under the
 * terms of the Psi License, specified in the accompanied COPYING
 * file, as published by the Psi Project; either dated January 1st,
 * 2005, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef GROWLNOTIFIER_H
#define GROWLNOTIFIER_H

#include <CoreFoundation/CoreFoundation.h>
#include <Growl-WithInstaller/Growl.h>

#include <QtGui/QPixmap>
#include <QtCore/QString>
#include <QtCore/QStringList>


/**
 * \brief A class for emitting a clicked signal to the interested party.
 */
class GrowlNotifierSignaler : public QObject
{
	Q_OBJECT

public: 
	GrowlNotifierSignaler() { };
	void emitNotificationClicked(void* context) { Q_EMIT notificationClicked(context); }
	void emitNotificationTimeout(void* context) { Q_EMIT notificationTimedOut(context); }

Q_SIGNALS:
	void notificationClicked(void*);
	void notificationTimedOut(void*);
};

/**
 * \brief A simple interface to Growl.
 */
class GrowlNotifier
{
public:
	
	static GrowlNotifier* Instance();//VOXOX CHANGE by Alexander 06-02-09: implemented Singleton to the Growl notifier
	
	void notify(const QString& name, const QString& title, 
		const QString& description, const QPixmap& icon = QPixmap(), 
		bool sticky = false, const QObject* receiver = 0, 
		const char* clicked_slot = 0, const char* timeout_slot = 0,
		void* context = 0);

protected:
	GrowlNotifier(
		const QStringList& notifications, 
		const QStringList& default_notifications,
		const QString& app_name = "");

private:
	
	static GrowlNotifier* _instance;
	
	struct Growl_Delegate delegate_;
	
	GrowlNotifierSignaler* signaler_;
};

#endif
