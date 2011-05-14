/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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
#include "AvatarList.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <util/File.h>
#include <util/Path.h>

AvatarList::AvatarList() 
{
	//Default avatar
	_defaultAvatarPicture = OWPicture::pictureFromFile(getAvatarPath() + "default-avatar.png");

	//Protocol-based avatars
	QtEnumIMProtocolMap& rMap = QtEnumIMProtocolMap::getInstance();

	for ( QtEnumIMProtocolMap::iterator it = rMap.begin(); it != rMap.end(); it++ )
	{
		if ( it->second.isIMProtocol() )
		{
			addAvatar( it->second.getQtProtocol(), it->second.getDefaultImageFile() );
		}
	}
}

AvatarList::~AvatarList() 
{
}

void AvatarList::addAvatar( QtEnumIMProtocol::IMProtocol protocol, const String& defaultImageFile )
{
	OWPicture pic = OWPicture::pictureFromFile(getAvatarPath() + defaultImageFile);
	_pictures.insert( std::pair<QtEnumIMProtocol::IMProtocol, OWPicture>(protocol, pic) );
}

Avatar AvatarList::getDefaultAvatar() const 
{
	return Avatar(getAvatarPath() + "default-avatar.png");
}

std::string AvatarList::getAvatarPath() const 
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return File::convertPathSeparators(config.getResourcesDir() + "pics/avatars/");
}

OWPicture AvatarList::getDefaultAvatarPicture() const 
{
	return _defaultAvatarPicture;
}

//VOXOX - CJC - 2009.05.06 Get This is just mantained for consistensy, it is not call inside this class
//Avatar AvatarList::getDefaultContactAvatar(EnumIMProtocol::IMProtocol protocol) const 
Avatar AvatarList::getDefaultContactAvatar(QtEnumIMProtocol::IMProtocol protocol) const 
{
	std::string	avatarFile = QtEnumIMProtocolMap::getInstance().getDefaultImageFile( protocol );

	return Avatar(getAvatarPath() + avatarFile);
}

//OWPicture AvatarList::getDefaultContactAvatarPicture(QtEnumIMProtocol::IMProtocol protocol)
const OWPicture& AvatarList::getDefaultContactAvatarPicture(QtEnumIMProtocol::IMProtocol protocol)	//VOXOX - JRT - 2009.08.17 
{
//	OWPicture result;
		
	PictureMap::const_iterator it = _pictures.find( protocol );

	if ( it == _pictures.end() )
	{
//		result = _pictures[QtEnumIMProtocol::IMProtocolVoxOx];	//Default to VoxOx if unexpected protocol.
		return _pictures[QtEnumIMProtocol::IMProtocolVoxOx];	//VOXOX - JRT - 2009.08.17 
	}
	else
	{
//		result = it->second;
		return it->second;	//VOXOX - JRT - 2009.08.17 
	}

//	return result;
}
