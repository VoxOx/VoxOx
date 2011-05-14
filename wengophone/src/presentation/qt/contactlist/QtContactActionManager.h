/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* Custom tooltip manager
* @author Chris Jimenez C 
* @date 2009.05.11
*/

#ifndef QTCONTACTACTIONMANAGER_H
#define QTCONTACTACTIONMANAGER_H

#include <QtCore/QString>
#include <QtCore/QObject>
#include <util/NonCopyable.h>
class QPixmap;
class CWengoPhone;
class QtContactTooltip;
class QtContact;
class QPoint;
class CUserProfile;



class QtContactActionManager:public QObject,NonCopyable  {
	Q_OBJECT
public:

	static QtContactActionManager * getInstance(CWengoPhone & cWengoPhone);

	QtContactActionManager(CWengoPhone & cWengoPhone);

	~QtContactActionManager();

	void callContact(const QString & contactId);

	void smsContact(const QString & contactId);

	void chatContact(const QString & contactId);

	void emailContact(const QString & contactId);

	void profileContact(const QString & contactId);

	void sendFileContact(const QString & contactId);

	void sendFaxContact(const QString & contactId);//VOXOX - CJC - 2009.06.23 

	void openSocialContactPage(const QString & contactId);

	QString addNumberToContact(const QString & contactId);

	QString addMobileNumberToContact(const QString & contactId);

	QString addFaxNumberToContact(const QString & contactId);//VOXOX - CJC - 2009.06.23 

	QString addEmailToContact(const QString & contactId);

public Q_SLOTS:


private:
	bool isVoxOxIMConnected();	//VOXOX - JRT - 2009.07.26 

	CUserProfile* getCUserProfile();

	CWengoPhone & _cWengoPhone;

	static QtContactActionManager * instance;	

	
};

#endif	//QtContactTooltip_H
