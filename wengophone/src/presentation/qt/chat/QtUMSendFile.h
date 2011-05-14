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

#ifndef OWQTUMSENDFILE_H
#define OWQTUMSENDFILE_H


#include "QtUMBase.h"

#include "QtEnumUMMode.h"

#include <control/chat/CChatHandler.h>

#include <util/Trackable.h>

class QUrl;	//VOXOX - JRT - 2009.09.09


namespace Ui { class UMSendFile; }
/**
 *
 * @ingroup presentation
 * @author Mr K.
 * @author Mathieu Stute
 */
class QtUMSendFile : public QtUMBase {
	Q_OBJECT
public:

	QtUMSendFile(CChatHandler & cChatHandler,QWidget * parent,QString key,QtEnumUMMode::Mode mode = QtEnumUMMode::UniversalMessageFileTransfer);

	virtual ~QtUMSendFile();

	void setUsername(const QString & userName) {_userName = userName;}

	const QString & getUsername() const {return _userName;}

	void setMd5(const QString & md5) {_md5 = md5;}

	const QString & getMd5() const {return _md5;}

	void setContactIdUploadFile(const QString & contactIdUploadFile) {_contactIdUploadFile = contactIdUploadFile;}

	const QString & getContactIdUploadFile() const {return _contactIdUploadFile;}

	virtual QWidget * getWidget(){return _widget;}
	virtual void setVisible(bool visible){};
	virtual bool	isGroupChat(){return false;}
	void setContactConnected(bool connected);

	void init(const QString & contactId,const QString & userName,const QString & md5,const QString & contactIdUploadFile,const QString & senderName) ;




public Q_SLOTS:


Q_SIGNALS:


protected:
	

private Q_SLOTS:

	
	void beforeNavigateSlot(const QUrl & link);

private:




	CChatHandler & _cChatHandler;

	QString _userName;

	QString _contactIdUploadFile;

	QString _md5;


	Ui::UMSendFile * _ui;

	QWidget * _widget;

	bool _isContactConnected;

	QString _uuid;

};

#endif //OWQtUMSendFile_H
