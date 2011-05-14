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

#ifndef QTPROFILEBAR_H
#define QTPROFILEBAR_H

#include <model/webservices/info/WsInfo.h>

#include <imwrapper/EnumPresenceState.h>
#include <imwrapper/IMAccount.h>

#include <util/Trackable.h>

#include <QtGui/QFrame>
#include <QtGui/QPixmap>
#include <QtCore/QMap>

class QGridLayout;

class QtIMAccountMonitor;
class QtIMProfileWidget;
class QtEventWidget;
class QtCreditWidget;

class CHistory;
class CUserProfile;
class CWengoPhone;
class ConnectHandler;
class IPhoneLine;
class PresenceHandler;
class UserProfile;
class WengoStyleLabel;

// VOXOX CHANGE by ASV 04-20-2009: we need to declare this in order to use it in the header file
class QVBoxLayout;

/**
 * Profile bar: shows the user profile at the top of the main window.
 *
 * TODO refactor, class too big, no .ui file ect...
 *
 * @author Mathieu Stute
 * @author Tanguy Krotoff
 */
class QtProfileBar : public QFrame, public Trackable {
	Q_OBJECT
public:

	QtProfileBar(QWidget * parent);

	void init(CWengoPhone*, CUserProfile*, QtIMAccountMonitor*);

	void reset();


	//VOXOX - CJC - 2009.06.05 Get widget
	QtIMProfileWidget * getQtImProfileWidget(){return _qtImProfileWidget;}

	~QtProfileBar();

Q_SIGNALS:
	void wsInfoWengosEvent(const QString & wengos);
	void wsInfoVoiceMailEvent(int count);
	void wsInfoLandlineNumberEvent(const QString & number);

	void phoneLineCreatedEvent();
	
	

private Q_SLOTS:

	void wsInfoWengosEventSlot(const QString & wengos);
	
	void wsInfoLandlineNumberEventSlot(const QString & number);
	
	void phoneLineCreatedEventSlot();

private:

	/**
	 * Initializes widgets content.
	 */
	void init();

	void wsInfoCreatedEventHandler(UserProfile & sender, WsInfo & wsInfo);

	void wsInfoWengosEventHandler(WsInfo & sender, int id, WsInfo::WsInfoStatus status, std::string wengos);

	void wsInfoVoiceMailEventHandler(WsInfo & sender, int id, WsInfo::WsInfoStatus status, int voicemail);

	void wsInfoLandlineNumberEventHandler(WsInfo & sender, int id, WsInfo::WsInfoStatus status, std::string number);

	void phoneLineCreatedEventHandler(UserProfile & sender, IPhoneLine & phoneLine);

	QtIMProfileWidget * _qtImProfileWidget;

	QPixmap _statusPixmap;

	CUserProfile * _cUserProfile;

	CWengoPhone * _cWengoPhone;

	QtIMAccountMonitor* _qtImAccountMonitor;
		
	QVBoxLayout* _mainLayout;	//VOXOX - JRT - 2009.04.16 - Avoid memory leak.

};

#endif	//QTPROFILEBAR_H
