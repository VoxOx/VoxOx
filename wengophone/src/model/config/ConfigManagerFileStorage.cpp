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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "ConfigManagerFileStorage.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/config/ConfigXMLSerializer.h>

#include <util/File.h>
#include <util/Logger.h>

static const std::string CONFIG_FILENAME = "config.xml";

ConfigManagerFileStorage::ConfigManagerFileStorage(ConfigManager & configManager)
: _configManager(configManager) {
}

ConfigManagerFileStorage::~ConfigManagerFileStorage() {
}

bool ConfigManagerFileStorage::loadSystemConfig(const std::string& systemDir) {
	std::string systemData;

	FileReader systemFile(systemDir + CONFIG_FILENAME);
	if (!systemFile.open()) {
		LOG_ERROR("Could not open " + systemDir + CONFIG_FILENAME);
		return false;
	}
	systemData = systemFile.read();
	ConfigXMLSerializer serializer(_configManager.getCurrentConfig());
	return serializer.unserializeSystemSettings(systemData);
}


bool ConfigManagerFileStorage::loadUserConfig(const std::string& userDir) {
	std::string userData;
	FileReader userFile(userDir + CONFIG_FILENAME);
	if (userFile.open()) {
		userData = userFile.read();
	}

	ConfigXMLSerializer serializer(_configManager.getCurrentConfig());
	return serializer.unserializeUserSettings(userData);
}

bool ConfigManagerFileStorage::save() {
	std::string userDir = _configManager.getCurrentConfig().getConfigDir();
	FileWriter file(userDir + CONFIG_FILENAME);
	ConfigXMLSerializer serializer(_configManager.getCurrentConfig());

	file.write(serializer.serialize());
	return true;
}
