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
* Custom Contacts tooltips
* @author Chris Jimenez C 
* @date 2009.05.11
*/


#ifndef QTCONTACTTOOLTIP_H
#define QTCONTACTTOOLTIP_H

#include <imwrapper/QtEnumIMProtocol.h>	

#include <presentation/qt/contactlist/QtContactNetworkMenu.h>//VOXOX CHANGE by Rolando - 2009.10.27 

#include <QtGui/QWidget>
#include <QtCore/QString>
class QPixmap;
class QString;
class CWengoPhone;
class QTimer;
class QPoint;
class QMenu;
class QtContactNetworkMenu;//VOXOX CHANGE by Rolando - 2009.10.27 

namespace Ui { class ContactTooltip; }


class QtContactTooltip : public QWidget {
	Q_OBJECT
public:

	QtContactTooltip(QString contactId,QWidget * parent,CWengoPhone & cWengoPhone);

	~QtContactTooltip();


	void setDisplayName(QString displayName);
	void setStatusMessage(QString statusMessage);
	//void setStatusPixmap(QPixmap statusPixmap);//VOXOX CHANGE by Rolando - 2009.10.23 
	void setStatusPixmapPath(QString normalStatusPixmapPath, QString hoverStatusPixmapPath);//VOXOX CHANGE by Rolando - 2009.10.23 
	void setAvatarPixmap(QPixmap avatarPixmap);
	void setProtocol(QtEnumIMProtocol::IMProtocol protocol);

	QString getDisplayName(){return _displayName;}
	QString getStatusMessage(){return _statusMessage;}
	//QPixmap getStatusPixmap(){return _statusPixmap;}//VOXOX CHANGE by Rolando - 2009.10.23 

	QPixmap getNormalStatusPixmapPath(){return _normalStatusPixmapPath;}//VOXOX CHANGE by Rolando - 2009.10.23 
	QPixmap getHoverStatusPixmapPath(){return _normalStatusPixmapPath;}//VOXOX CHANGE by Rolando - 2009.10.23
	QPixmap getAvatarPixmap(){return _avatarPixmap;}

	void showTooltip();
	void closeThis();

Q_SIGNALS:

	void closeMe();

	
public Q_SLOTS:

	void closeTooltip();

	void chatContact();

	void callContact();

	void smsContact();

	void sendFileContact();
	//VOXOX - CJC - 2009.06.23 
	void sendFaxContact();

	void emailContact();

	void profileContact();

	void socialContact();

	void networkIconClicked();//VOXOX CHANGE by Rolando - 2009.09.23 

private:

	QString checkLink(QString text);

	void buildMenuNetworkIcon();//VOXOX CHANGE by Rolando - 2009.09.23 
	void enterEvent ( QEvent * event );//VOXOX CHANGE by Rolando - 2009.09.23
	void leaveEvent ( QEvent * event );//VOXOX CHANGE by Rolando - 2009.09.23

	QString _contactId;
	QString _displayName;
	QString _statusMessage;
	//QPixmap _statusPixmap;//VOXOX CHANGE by Rolando - 2009.10.23 

	QString _normalStatusPixmapPath;//VOXOX CHANGE by Rolando - 2009.10.23 
	QString _hoverStatusPixmapPath;//VOXOX CHANGE by Rolando - 2009.10.23 
	QPixmap _avatarPixmap;
	QString _socialAddress;

	QtEnumIMProtocol::IMProtocol _imProtocol;//VOXOX CHANGE by Rolando - 2009.09.23 
	bool _isASocialNetwork;//VOXOX CHANGE by Rolando - 2009.09.23 
	//QMenu * _networkIconMenu;//VOXOX CHANGE by Rolando - 2009.10.27

	QtContactNetworkMenu * _contactNetworkMenu;//VOXOX CHANGE by Rolando - 2009.10.27 
	bool _isMouseHover;//VOXOX CHANGE by Rolando - 2009.09.23 

	CWengoPhone & _cWengoPhone;

	QTimer * _timer;

	Ui::ContactTooltip * _ui;
};

#endif	//QtContactTooltip_H
