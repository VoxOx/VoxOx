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
* CLASS QtUMItemInfo 
* @author Rolando 
* @date 2009.09.01
*/


//NOTE: this class is only used to sort QtUMItemList
//	If sort options are added, we need to:
//		1) Add any newly needed info to this class
//		2) Modify the operator< to sort as desired.

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtUMItemInfo.h"

#include "QtUMItem.h"

#ifdef _WIN32
#define random rand
#endif

QtUMItemInfo::QtUMItemInfo(QTreeWidgetItem * item, QTreeWidgetItem * parentItem, 
							 const QString & id, const QString& userName, int index, bool isGroup)
{
	_id	= id;
	_userName	= userName.toLower();
	_item		= item;
	_index		= index;	
	_isGroup    = isGroup;
	_randomKey	= random();
}

QtUMItemInfo::QtUMItemInfo(const QtUMItemInfo & other) 
{
	copy( other );
}

QtUMItemInfo::~QtUMItemInfo() 
{
}

QtUMItemInfo & QtUMItemInfo::operator=(const QtUMItemInfo & other) {

	if ( this != &other )
	{
		copy( other );
	}
	return *this;
}

void QtUMItemInfo::copy(const QtUMItemInfo & other) 
{
	_id	= other._id;
	_userName	= other._userName;
	_item		= other._item;
	_index		= other._index;
	_isGroup    = other._isGroup;
	_randomKey  = other._randomKey;
}

bool QtUMItemInfo::operator<(const QtUMItemInfo & other) const 
{
	
	/*return getUserName() < other.getUserName();*///VOXOX CHANGE by Rolando - 2009.09.07 
	return _index < other._index;//VOXOX CHANGE by Rolando - 2009.09.07 
}

//-----------------------------------------------------------------------------


