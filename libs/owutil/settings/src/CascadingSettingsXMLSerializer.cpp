/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include <settings/CascadingSettings.h>
#include <settings/CascadingSettingsXMLSerializer.h>
#include <settings/SettingsXMLSerializer.h>

#include <util/Logger.h>


CascadingSettingsXMLSerializer::CascadingSettingsXMLSerializer(CascadingSettings & settings)
	: _settings(settings) {
}

CascadingSettingsXMLSerializer::~CascadingSettingsXMLSerializer() {
}

std::string CascadingSettingsXMLSerializer::serialize() {
	SettingsXMLSerializer userSerializer(_settings._userSettings);
	return userSerializer.serialize();
}

bool CascadingSettingsXMLSerializer::unserialize(const std::string & systemData, const std::string & userData) {
	if (!unserializeSystemSettings(systemData)) {
		return false;
	}

	return unserializeUserSettings(userData);
}

bool CascadingSettingsXMLSerializer::unserializeSystemSettings(const std::string & data) {
	SettingsXMLSerializer serializer(_settings._systemSettings);
	if (!serializer.unserialize(data)) {
		LOG_ERROR("Could not unserialize system settings");
		return false;
	}
	return true;
}

bool CascadingSettingsXMLSerializer::unserializeUserSettings(const std::string & data) {
	if (!data.empty()) {
		SettingsXMLSerializer serializer(_settings._userSettings);
		if (!serializer.unserialize(data)) {
			LOG_WARN("Could not unserialize user settings, using only system settings");
			return false;
		}
	}

	// Create missing sticky keys
	std::set<std::string>::const_iterator
		it = _settings._stickyKeySet.begin(),
		end = _settings._stickyKeySet.end();
	for (; it!=end; ++it) {
		std::string key = *it;
		if (_settings._systemSettings.contains(key) && !_settings._userSettings.contains(key)) {
			_settings._userSettings.set(key, _settings._systemSettings.getAny(key));
		}
	}
	return true;
}
