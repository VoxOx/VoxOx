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
* CLASS QtContactNetworkMenu 
* @author Rolando 
* @date 2009.10.23
*/


#ifndef OWQTCONTACTNETWORKMENU_H
#define OWQTCONTACTNETWORKMENU_H

#include <QtCore/QString>
#include <QtGui/QMenu>
#include <QtGui/QWidget>
#include <control/CWengoPhone.h>//VOXOX CHANGE by Rolando - 2009.10.23 

class QWidget;


class QtContactNetworkMenu : public QMenu {
	Q_OBJECT
public:

	QtContactNetworkMenu(const std::string & contactId, CWengoPhone & cWengoPhone, QWidget * parent);

	~QtContactNetworkMenu();

	std::string getContactId() const;
	void setContactId(const std::string & contactId);

private Q_SLOTS:
	void chatButtonClicked();
	void webProfileContactClicked();
	void callButtonClicked();
	void profileContactClicked();

private:

	void buildContactMenuNetwork();

	std::string _contactId;
	CWengoPhone & _cWengoPhone;
};

#endif	//OWQTCONTACTNETWORKMENU_H
