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
* Class widget to support chat to email
* @author Chris Jimenez C 
* @date 2009.05.11
*/


#ifndef OWQTUMFAX_H
#define OWQTUMFAX_H

#include "QtUMBase.h"

#include "QtEnumUMMode.h"

#include <control/chat/CChatHandler.h>

#include <util/Trackable.h>


namespace Ui { class UMFax; }
/**
 *
 * @ingroup presentation
 * @author Mr K.
 * @author Mathieu Stute
 */
class QtUMFax : public QtUMBase {
	Q_OBJECT
public:

	QtUMFax(CChatHandler & cChatHandler,QWidget * parent,QString key,QtEnumUMMode::Mode mode = QtEnumUMMode::UniversalMessageFax);

	virtual ~QtUMFax();

	void setUsername(const QString & userName) {_userName = userName;}

	const QString & getUsername() const {return _userName;}

	void setUserKey(const QString & userKey) {_userKey = userKey;}

	const QString & getUserKey() const {return _userKey;}

	void setRecipient(const QString & recepient) {_recepient = recepient;}

	const QString & getRecepient() const {return _recepient;}

	virtual QWidget * getWidget(){return _widget;}
	virtual void setVisible(bool visible){};
	virtual bool	isGroupChat(){return false;}
	void setPhoneNumber(const QString & phoneNumber) {_phoneNumber = phoneNumber;}

	const QString & getPhoneNumber() const {return _phoneNumber;}
	void setContactConnected(bool connected);

	void init(const QString & faxNumber,const QString & userName,const QString & md5,const QString & contactId,const QString & senderName);

public Q_SLOTS:


Q_SIGNALS:


protected:
	

private Q_SLOTS:


private:


	CChatHandler & _cChatHandler;

	QString _userName;

	QString _userKey;

	QString _recepient;

	QString _phoneNumber;

	Ui::UMFax * _ui;

	QWidget * _widget;

	bool _isContactConnected;


};

#endif //OWQtUMFax_H
