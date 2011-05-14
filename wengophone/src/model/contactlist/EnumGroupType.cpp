/*
 VOXOX !!!
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006 Wengo
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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "EnumGroupType.h"

#include <util/Logger.h>

using namespace std;

//static
std::string EnumGroupType::toString( GroupType eType )
{
	std::string type = "";

	switch( eType )
	{
	case GroupType_OrigNetwork:
		type = "orignet";
		break;

	case GroupType_Network:
		type = "net";
		break;

	case GroupType_Filter:
		type = "filter";
		break;

	case GroupType_User:
	default:
		type = "user";
		break;
	}

	return type;
}

//static
EnumGroupType::GroupType EnumGroupType::toGroupType(const std::string& type ) 
{
	EnumGroupType::GroupType eType = GroupType_User;	//Default
	if ( type == "orignet" )
	{
		eType = GroupType_OrigNetwork;
	}
	else if ( type == "net" )
	{
		GroupType_Network;
	}
	else if ( type == "user" )
	{
		GroupType_User;
	}
	else if ( type == "filter" )
	{
		GroupType_Filter;
	}
	else
	{
//		LOG_WARN( );		//TODO
		GroupType_User;
	}

	return eType;
}
