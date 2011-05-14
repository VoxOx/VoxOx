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

//VOXOX - JRT - 2009.04.15 - NOTE: this class is only used to sort ContactList
//	If sort options are added, we need to:
//		1) Add any newly needed info to this class
//		2) Modify the operator< to sort as desired.

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtContactInfo.h"

#include "QtContactListManager.h"
#include "QtContact.h"

#include "QtEnumSortOption.h"			//VOXOX - JRT - 2009.05.03
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <QtGui/QtGui>

#ifdef _WIN32
#define random rand
#endif

QtContactInfo::QtContactInfo(QTreeWidgetItem * item, QTreeWidgetItem * parentItem, 
							 const QString & contactId, const QString& userName, int index,
							 EnumPresenceState::PresenceState eState, int sortOrder, bool isGroup,
							 EnumGroupType::GroupType grpType )
{
	_contactId	= contactId;
	_userName	= userName.toLower();
	_item		= item;
	_parentItem = parentItem;
	_index		= index;
	_clear		= false;
	_eState		= eState; 
	_sortOrder  = sortOrder;
	_isGroup    = isGroup;
	_grpType	= grpType;

	_randomKey	= random();

	normalizePresence();
}

QtContactInfo::QtContactInfo(const QtContactInfo & other) 
{
	copy( other );
}

QtContactInfo::~QtContactInfo() 
{
}

QtContactInfo & QtContactInfo::operator=(const QtContactInfo & other) {

	if ( this != &other )		//VOXOX - JRT - 2009.04.15 
	{
		copy( other );
	}
	return *this;
}

void QtContactInfo::copy(const QtContactInfo & other) 
{
	_contactId	= other._contactId;
	_userName	= other._userName;	//VOXOX - JRT - 2009.04.15 
	_item		= other._item;
	_parentItem = other._parentItem;
	_index		= other._index;
	_clear		= other._clear;
	_eState		= other._eState;
	_sortOrder  = other._sortOrder;
	_isGroup    = other._isGroup;
	_grpType	= other._grpType;

	_randomKey  = other._randomKey;
	_normalizedPresence = other._normalizedPresence;
}

bool QtContactInfo::operator<(const QtContactInfo & other) const 
{
	//VOXOX - JRT - 2009.04.26 
	//Converted toLower() for every compare is too inefficent.
	//The ctor has been changed to handle this.
	bool    bRet   = false;
	Config&	config = ConfigManager::getInstance().getCurrentConfig();

	if ( isGroup() )
	{
		if ( ( _grpType		  == EnumGroupType::GroupType_Filter ) || 
			 ( other._grpType == EnumGroupType::GroupType_Filter ) )	//VOXOX - JRT - 2009.05.30 - Always at top
		{

			if ( _grpType == EnumGroupType::GroupType_Filter )
			{
				bRet =  (other._grpType != EnumGroupType::GroupType_Filter);
			}
			else
			{
				bRet =  (_grpType == EnumGroupType::GroupType_Filter);
			}
		}
		else
		{
			QtEnumSortOption::SortOption eSort  = static_cast<QtEnumSortOption::SortOption>(config.getGroupSort());
		
			switch ( eSort )
			{
			case QtEnumSortOption::SortGroupAlpha:
				bRet = (getUserName() < other.getUserName());
				break;

			case QtEnumSortOption::SortGroupManual:
				bRet = (getSortOrder() < other.getSortOrder());
				break;

			default:
				bRet = (getUserName() < other.getUserName());
				break;
			}
		}
	}
	else
	{
		QtEnumSortOption::SortOption eSort  = static_cast<QtEnumSortOption::SortOption>(config.getContactSort());

		switch ( eSort )
		{
		case QtEnumSortOption::SortRandom:
			bRet = (getRandomKey() < other.getRandomKey());
			break;

		case QtEnumSortOption::SortPresence:
			if ( getNormalizedPresence() == other.getNormalizedPresence() )
			{
				bRet = (getUserName() < other.getUserName());
			}
			else
			{
				bRet = (getNormalizedPresence() < other.getNormalizedPresence());
			}
			break;

		case QtEnumSortOption::SortAlpha:
		default:
			bRet = (getUserName() < other.getUserName());
			break;
		}
}

	return bRet;
}

//-----------------------------------------------------------------------------

void QtContactInfo::normalizePresence()
{
	int result = 0;

	switch( _eState )
	{
	case EnumPresenceState::PresenceStateOnline:
		result = 0;
		break;

	case EnumPresenceState::PresenceStateAway:
	case EnumPresenceState::PresenceStateDoNotDisturb:
		result = 1;
		break;

	case EnumPresenceState::PresenceStateOffline:
	case EnumPresenceState::PresenceStateInvisible:
	case EnumPresenceState::PresenceStateUnavailable:
		result = 2;
		break;

	case EnumPresenceState::PresenceStateUserDefined:
	case EnumPresenceState::PresenceStateUnknown:
	case EnumPresenceState::PresenceStateMulti:
	default:
		result = 3;
	}

	_normalizedPresence = result;
}
