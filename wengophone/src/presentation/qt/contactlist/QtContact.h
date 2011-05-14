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

#ifndef OWQTCONTACT_H
#define OWQTCONTACT_H

#include <model/contactlist/ContactProfile.h>
#include <model/contactlist/Contact.h>			//VOXOX - JRT - 2009.04.14 
#include <model/contactlist/ContactGroup.h>		//VOXOX - JRT - 2009.05.01 - To support group sort options.

#include "QtContactPixmap.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMutex>
#include <QtGui/QPixmap>
// VOXOX CHANGE by ASV 04-20-2009: we need to declare this in order to use it in the header file
#include <QtGui/QTreeWidget>

class QPainter;
class CWengoPhone;
class QModelIndex;
class QStyleOptionViewItem;
class QPainter;
class QRect;

/**
 * Qt Presentation component for Contact
 *
 * @author Mr K
 */
class QtContact : QObject {
	Q_OBJECT
public:

	enum SizeHint { UserSize = 32, UserOpenSize = 68, GroupSize = 21 };

//	enum AvatarSize{AvatarWidth = 28, AvatarHeight = 28};

	QtContact( const std::string& contactId, const std::string& groupId, CWengoPhone& cWengoPhone, QObject * parent, bool bGroup );

	CWengoPhone & getCWengoPhone()			{return _cWengoPhone;}

	//VOXOX - JRT - 2009.04.14 - Rearranging methods to make many of them private so
	//		we can ensure QtContact is not used as data retrieval source.
	virtual void	paint(QPainter * painter, const QStyleOptionViewItem & option);

	QString			getId() const;
	QString			getKey() const;
	QString			getDisplayName() const;
	QString			getStatusMessage() const;
	//QPixmap		getStatusPixmap();//VOXOX CHANGE by Rolando - 2009.10.23 
	QString			getNormalStatusPixmapPath();//VOXOX CHANGE by Rolando - 2009.10.23 
	QString			getHoverStatusPixmapPath();//VOXOX CHANGE by Rolando - 2009.10.23 
	QPixmap			getAvatarPixmap() const;
	QString			getGroupName() const;
	QString			getContactGroupContactsOnline() const;
	EnumPresenceState::PresenceState getPresenceState() const;
	int				getSortOrder() const;

	EnumGroupType::GroupType getGroupType() const;	//VOXOX - JRT - 2009.05.30 

	QString			getEscapedDisplayName();
	QString			getEscapedStatusMessage();

	int				getHeight() const;
	Qt::MouseButton getButton() const;
	void			setButton(const Qt::MouseButton button);
	void			setMouseOn(bool value) { _mouseOn = value; }
	void			resetMouseStatus() ;

	void			toggleOpenStatus()								{ _openStatus = !_openStatus;	}
	void			setIsOpen(bool value)							{ _openStatus = value;			}
	bool			isOpen() const									{ return _openStatus;			}

	void			setTreeWidgetItem( QTreeWidgetItem* item )		{ _item		  = item;			}
	QTreeWidgetItem* getTreeWidgetItem()							{ return _item;	}

	bool			isGroup()	 const								{ return _isGroup;	}
	Contact*		getContact() const								{ return _contact;	}	//VOXOX - JRT - 2009.05.08 Made public

	bool			shouldExpand();
	bool			canDrag();

	std::string		getGroupKey() const								{ return _groupKey;	}	//VOXOX - JRT - 2009.08.07 
	void			setGroupKey( const std::string& val )			{ _groupKey = val;	}

	ContactGroupInfo* getContactGroupInfo() const;

	/**
	 * Returns the pixmap to draw for this contact.
	 */
	QtContactPixmap::ContactPixmap getStatus();

	QtContactPixmap::ContactPixmap getHoverStatus();//VOXOX CHANGE by Rolando - 2009.10.23 

	bool isBlocked();

private:
	void initVars();
	ContactGroup*	getContactGroup() const;

	void paintForeground(QPainter * painter, const QStyleOptionViewItem & option);
	QString verifyText(QRect & painterRect, QFont font, QString text);

	
	QString getUserName() const;
	QString getMobilePhone();		//JRT - 2009.04.05 - Removed const
	QString getHomePhone();			//JRT - 2009.04.05 - Removed const
	QString getWorkPhone();			//JRT - 2009.04.05 - Removed const
	QString getWengoPhoneNumber();	//JRT - 2009.04.05 - Removed const
	QString getPreferredNumber();	//JRT - 2009.04.05 - Removed const
	QString getWebsite();			//JRT - 2009.04.05 - Removed const

	QString fixHtmlString(QString text);	//VOXOX - JRT - 2009.05.04 - TODO: move to qtUtil?

public Q_SLOTS:

Q_SIGNALS:

	void clicked(QtContact * qtContact, int prt);

private:
	bool updateDataPtr();
	bool isTestCall() const;

	bool _mouseOn;
	bool _openStatus;

	Qt::MouseButton _mouseButton;

	std::string		_contactId;
	std::string		_groupKey;				//VOXOX - JRT - 2009.08.07 - For multigroup support
	std::string		_key;
	bool			_isGroup;

	Contact*		_contact;
	ContactGroup*	_contactGroup;

	CWengoPhone &	_cWengoPhone;

	QMutex			_mutex;

	int				_posXNetworkStatus;
	QTreeWidgetItem* _item;					//VOXOX - JRT - 2009.04.16 - make connection to Tree easier and faster.
};

#endif	//OWQTCONTACT
