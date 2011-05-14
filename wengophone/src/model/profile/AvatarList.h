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

#ifndef OWAVATARLIST_H
#define OWAVATARLIST_H

#include <util/OWPicture.h>
#include <util/Singleton.h>
#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/QtEnumIMProtocol.h>
#include "Avatar.h"
#include <map>
/**
 * Provides service to get the list of avatar available in $RESOURCES_DIR/pics/avatars
 */
class AvatarList : public Singleton<AvatarList> {
	friend class Singleton<AvatarList>;
public:

	/**
	 * Get the Default Avatar.
	 *
	 * Thread-safety: reentrant.
	 */
	Avatar getDefaultAvatar() const;

	Avatar getDefaultContactAvatar(QtEnumIMProtocol::IMProtocol protocol) const;

	/**
	 * Get the Default Avatar picture.
	 *
	 * Thread-safety: reentrant.
	 */
	OWPicture getDefaultAvatarPicture() const;

//	OWPicture getDefaultContactAvatarPicture(QtEnumIMProtocol::IMProtocol protocol);
	const OWPicture& getDefaultContactAvatarPicture(QtEnumIMProtocol::IMProtocol protocol);	//VOXOX - JRT - 2009.08.17 

private:
	AvatarList();
	~AvatarList();

	/**
	 * Get the path to the Avatars.
	 *
	 * Thread-safety: reentrant.
	 */
	std::string getAvatarPath() const;
	void addAvatar( QtEnumIMProtocol::IMProtocol protocol, const String& defaultImageFile );	//VOXOX - JRT - 2009.06.11 

	OWPicture _defaultAvatarPicture;

	typedef std::map<QtEnumIMProtocol::IMProtocol, OWPicture> PictureMap;
	PictureMap _pictures;
};

#endif //OWAVATARLIST_H
