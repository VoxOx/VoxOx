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

#ifndef OWCONTACTINFO_H
#define OWCONTACTINFO_H

#include "QtContactPixmap.h"
#include <imwrapper/EnumPresenceState.h>
#include <model/contactlist/EnumGroupType.h>

class QTreeWidgetItem;
class QtContact;

/**
 * Store contact informations for sorting
 *
 * @author Mr K
 */
class QtContactInfo {
public:

	QtContactInfo( QTreeWidgetItem * item, QTreeWidgetItem * parentItem, const QString & contactId, 
				   const QString& userName, int index, EnumPresenceState::PresenceState eState,
				   int sortOrder, bool isGroup, EnumGroupType::GroupType grpType );

	QtContactInfo(const QtContactInfo & other);

	~QtContactInfo();

	QTreeWidgetItem * getItem() const			{ return _item;			}
	QTreeWidgetItem * getParentItem() const		{ return _parentItem;	}

	QString getContactId() const				{ return _contactId;}
	QString getUserName()  const				{ return _userName;	}
	int		getSortOrder() const				{ return _sortOrder;}
	int		getRandomKey() const				{ return _randomKey;}
	int		getNormalizedPresence() const		{ return _normalizedPresence;	}
	bool	isGroup()		const				{ return _isGroup;	}

	EnumPresenceState::PresenceState getPresenceState()	{ return _eState;	}

	int getIndex()   const						{ return _index;	}
	bool isCleared() const						{ return _clear;	}

	void clear()								{ _clear = true;	}

	bool			operator<(const QtContactInfo & other) const;
	QtContactInfo&	operator=(const QtContactInfo & other);

private:
	void copy(const QtContactInfo & other);
	void normalizePresence();

	QTreeWidgetItem * _item;
	QTreeWidgetItem * _parentItem;

	bool	_isGroup;							//VOXOX - JRT - 2009.05.11 
	QString _contactId;
	QString	_userName;							//VOXOX - JRT - 2009.04.15 
	int		_sortOrder;							//VOXOX - JRT - 2009.05.11 
	int		_randomKey;							//VOXOX - JRT - 2009.05.03 
	int		_normalizedPresence;				//VOXOX - JRT - 2009.05.03 
	EnumPresenceState::PresenceState _eState;	//VOXOX - JRT - 2009.05.03 
	EnumGroupType::GroupType		 _grpType;	//VOXOX - JRT - 2009.05.30 

	int		_index;
	bool	_clear;
};

#endif	//OWCONTACTINFO_H
