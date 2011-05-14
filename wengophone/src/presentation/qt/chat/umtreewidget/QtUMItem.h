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

#ifndef OWQTUMITEM_H
#define OWQTUMITEM_H

#include <presentation/qt/contactlist/QtContactNetworkMenu.h>//VOXOX CHANGE by Rolando - 2009.10.23 
#include <presentation/qt/contactlist/QtContactPixmap.h>//VOXOX CHANGE by Rolando - 2009.08.30 
#include <control/CWengoPhone.h>//VOXOX CHANGE by Rolando - 2009.10.23 

#include <QtCore/QMutex>				//VOXOX - JRT - 2009.09.09
#include <QtGui/QStyleOptionViewItem>	//VOXOX - JRT - 2009.09.09
class QTreeWidgetItem;					//VOXOX - JRT - 2009.09.09

class QtUMItem : QObject {
	Q_OBJECT
public:

	enum SizeHint { UserSize = 32, UserOpenSize = 68, GroupSize = 21 };

	//VOXOX CHANGE by Rolando - 2009.10.23 - added cWengoPhone and contactId parameters 
	//VOXOX CHANGE by Rolando - 2009.08.31 
	QtUMItem(CWengoPhone & cWengoPhone, const QString & contactId, const  QString &    id,	 const QString & groupId, const QString & displayName, const QString & groupName, QString avatarData, QPixmap normalStatusPixmap, QPixmap hoverStatusPixmap, QObject * parent, bool bGroup, bool isSelected);//VOXOX CHANGE by Rolando - 2009.10.26 

	//VOXOX - JRT - 2009.04.14 - Rearranging methods to make many of them private so
	//		we can ensure QtUMItem is not used as data retrieval source.
	virtual void	paint(QPainter * painter, const QStyleOptionViewItem & option);

	QString			getId() const;
	QString			getKey() const;
	QString			getDisplayName() const;
	QString			getGroupName() const;//VOXOX CHANGE by Rolando - 2009.08.21
	QString			getStatusMessage() const;
	QPixmap			getNormalStatusPixmap(){ return _normalStatusPixmap;}//VOXOX CHANGE by Rolando - 2009.10.26 
	QPixmap			getHoverStatusPixmap(){ return _hoverStatusPixmap;}//VOXOX CHANGE by Rolando - 2009.10.26
	
	QPixmap			getCurrentStatusPixmap();//VOXOX CHANGE by Rolando - 2009.10.26 
	
	
	QPixmap			getAvatarPixmap();
	QString			getEscapedDisplayName();
	
	int				getHeight() const;


	void			toggleOpenStatus()								{ _openStatus = !_openStatus;	}
	void			setOpenStatus(bool value)						{ _openStatus = value;			}
	bool			isOpen() const									{ return _openStatus; }

	void			setIsSelected(bool isSelected)					{ _isSelected = isSelected;		}//VOXOX CHANGE by Rolando - 2009.09.29 
	bool			isSelected()									{ return _isSelected;			}//VOXOX CHANGE by Rolando - 2009.09.29 

	void			setTreeWidgetItem( QTreeWidgetItem* item )		{ _item		  = item;			}
	QTreeWidgetItem* getTreeWidgetItem()							{ return _item;	}

	bool			isAGroupItem()	 const							{ return _isAGroupItem;	}//VOXOX CHANGE by Rolando - 2009.10.01 
	void			setIsAGroupItem(bool isAGroupItem)				{ _isAGroupItem = isAGroupItem;}//VOXOX CHANGE by Rolando - 2009.09.25 

	QString		getGroupKey() const									{ return _groupKey;	}
	void			setGroupKey( const QString & val )				{ _groupKey = val;	}

	QString		getContactId() const									{ return _contactId;	}//VOXOX CHANGE by Rolando - 2009.10.23 
	void		setContactId( const QString & contactId )				{ _contactId = contactId;	}//VOXOX CHANGE by Rolando - 2009.10.23 

	bool mouseHoveringCloseButton(QPoint mousePosition);

	bool mouseHoveringNetworkButton(QPoint mousePosition);//VOXOX CHANGE by Rolando - 2009.10.21 

	void setBlinking(bool isBlinking);//VOXOX CHANGE by Rolando - 2009.08.28 	

	QString getAvatarData(){ return _avatarData;}//VOXOX CHANGE by Rolando - 2009.08.30 
	void setAvatarData(QString avatarData){ _avatarData = avatarData;}//VOXOX CHANGE by Rolando - 2009.08.31 

	QString getDisplayName(){ return _displayName;}//VOXOX CHANGE by Rolando - 2009.08.30 
	void setDisplayName(QString displayName){ _displayName = displayName;}//VOXOX CHANGE by Rolando - 2009.08.30

	EnumPresenceState::PresenceState getPresenceState(){return _presenceState;}//VOXOX CHANGE by Rolando - 2009.08.30 
	void setPresenceState(EnumPresenceState::PresenceState presenceState){ _presenceState = presenceState;}//VOXOX CHANGE by Rolando - 2009.08.30 
	void setNormalStatusPixmap(QtContactPixmap::ContactPixmap status);//VOXOX CHANGE by Rolando - 2009.10.26  
	void setNormalStatusPixmap(QPixmap statusPixmap);//VOXOX CHANGE by Rolando - 2009.10.26

	void setHoverStatusPixmap(QtContactPixmap::ContactPixmap status);//VOXOX CHANGE by Rolando - 2009.10.26  
	void setHoverStatusPixmap(QPixmap statusPixmap);//VOXOX CHANGE by Rolando - 2009.10.26 

	void networkIconClicked(QPoint mousePosition);//VOXOX CHANGE by Rolando - 2009.10.26

	void setNetworkIconIsUnderMouse(bool isUnderMouse);//VOXOX CHANGE by Rolando - 2009.10.26 

private:
	void initVars();
	
	void paintForeground(QPainter * painter, const QStyleOptionViewItem & option);
	QString verifyText(QRect & painterRect, QFont font, QString text);
	
	QString getUserName() const;

	QString fixHtmlString(QString text);

public Q_SLOTS:

Q_SIGNALS:

	void clicked(QtUMItem * umItem, int prt);

private:
	bool _mouseOn;
	bool _openStatus;

	Qt::MouseButton _mouseButton;

	QString     _contactId;//VOXOX CHANGE by Rolando - 2009.10.23 
	QString		_id;
	QString		_groupKey;				//VOXOX - JRT - 2009.08.07 - For multigroup support
	QString 	_key;
	QString		_groupName;//VOXOX CHANGE by Rolando - 2009.08.21 
	bool		_isAGroupItem;
	bool		_isSelected;//VOXOX CHANGE by Rolando - 2009.09.29

	bool		_isNetworkIconUnderMouse;//VOXOX CHANGE by Rolando - 2009.10.26 

	QMutex		_mutex;

	int					_posXNetworkStatus;
	int					_posXCloseButton;//VOXOX CHANGE by Rolando - 2009.08.25
	int					_posYCloseButton;//VOXOX CHANGE by Rolando - 2009.08.27
	int					_widthCloseButton;//VOXOX CHANGE by Rolando - 2009.08.27
	int					_heightCloseButton;//VOXOX CHANGE by Rolando - 2009.08.27

	int					_posXNetworkButton;//VOXOX CHANGE by Rolando - 2009.10.21 
	int					_posYNetworkButton;//VOXOX CHANGE by Rolando - 2009.10.21 
	int					_widthNetworkButton;//VOXOX CHANGE by Rolando - 2009.10.21 
	int					_heightNetworkButton;//VOXOX CHANGE by Rolando - 2009.10.21 
	
	QTreeWidgetItem*	_item;	

	bool				_isBlinking;//VOXOX CHANGE by Rolando - 2009.08.28 
	bool				_blinkNow;//VOXOX CHANGE by Rolando - 2009.08.28 
	QColor				_backgroundColor;//VOXOX CHANGE by Rolando - 2009.08.28
	QColor				_fontColor;//VOXOX CHANGE by Rolando - 2009.09.07 	

	QString				_avatarData;//VOXOX CHANGE by Rolando - 2009.08.30
	QPixmap				_normalStatusPixmap;//VOXOX CHANGE by Rolando - 2009.10.26
	QPixmap				_hoverStatusPixmap;//VOXOX CHANGE by Rolando - 2009.10.26 
	QString				_displayName;//VOXOX CHANGE by Rolando - 2009.08.30
	EnumPresenceState::PresenceState _presenceState;

	QtContactNetworkMenu * _contactNetworkMenu;//VOXOX CHANGE by Rolando - 2009.10.23
	CWengoPhone & _cWengoPhone;//VOXOX CHANGE by Rolando - 2009.10.23 
};

#endif	//OWQTUMITEM_H
