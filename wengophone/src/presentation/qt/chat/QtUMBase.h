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
* Base class for the different chat widgets
* @author Chris Jimenez C 
* @date 2009.05.11
*/



#ifndef OWQTUMBASE_H
#define OWQTUMBASE_H

#include <QtGui/QWidget>	//VOXOX - JRT - 2009.09.09
#include <util/Trackable.h>

#include "QtEnumUMMode.h"


class QtUMBase : public QObject, public Trackable {
	Q_OBJECT
public:


	QtUMBase(QWidget * parent,QString key,QtEnumUMMode::Mode mode);

	~QtUMBase();

	QtEnumUMMode::Mode getMode(){ return _mode; }

	void setMode(QtEnumUMMode::Mode mode){ _mode = mode; }

	void setContactId(QString contactId) {_contactId = contactId;}
	
	QString getContactId() const {return _contactId;}

	QString getKey()const {return _key;}
	
	void setKey(QString key) {_key = key;}

	void setSenderName(QString senderName) {_senderName = senderName;}//VOXOX CHANGE by Rolando - 2009.06.10 

	QString getSenderName() const {return _senderName;}//VOXOX CHANGE by Rolando - 2009.06.10 

	virtual void setContactConnected(bool connected) = 0;

	//VOXOX - CJC - 2009.08.19 Add from here

	virtual QWidget * getWidget() = 0;

	virtual void setVisible(bool visible) = 0;

	virtual bool isGroupChat() = 0; //VOXOX - CJC - 2009.09.24 Is not good that this is here, but its a lot quicker and for convenient
	
protected:

	/** True if the Contact in this Widget is connected. */
	bool _isContactConnected;

	//ContactId
	QString _contactId;

	QString _id;

	QString _key;

	QString _senderName;


private Q_SLOTS:

	


private:

	QtEnumUMMode::Mode _mode;


};

#endif	//OWQtUMBase_H
