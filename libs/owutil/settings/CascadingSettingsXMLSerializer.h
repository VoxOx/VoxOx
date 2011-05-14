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

#ifndef OWCASCADINGSETTINGSXMLSERIALIZER_H
#define OWCASCADINGSETTINGSXMLSERIALIZER_H

#include <settings/owsettingsdll.h>

#include <string>

class CascadingSettings;

class CascadingSettingsXMLSerializer {
public:
	OWSETTINGS_API CascadingSettingsXMLSerializer(CascadingSettings & settings);
	OWSETTINGS_API virtual ~CascadingSettingsXMLSerializer();

	/**
	 * Serialize the user data
	 */
	OWSETTINGS_API virtual std::string serialize();

	/**
	 * Unserialize system and user data
	 */
	OWSETTINGS_API virtual bool unserialize(const std::string & systemData, const std::string & userData);

	/**
	 * Unserialize system data
	 */
	OWSETTINGS_API virtual bool unserializeSystemSettings(const std::string & data);

	/**
	 * Unserialize user data
	 */
	OWSETTINGS_API virtual bool unserializeUserSettings(const std::string & data);

protected:

	CascadingSettings & _settings;
};

#endif	//OWCASCADINGSETTINGSXMLSERIALIZER_H
