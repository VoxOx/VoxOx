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
#include "StartupSettingListener.h"

#include "Config.h"
#include "ConfigManager.h"

#include <system/Startup.h>

#include <cutil/global.h>
#include <util/Logger.h>
#include <util/Path.h>

#include <iostream>

StartupSettingListener::StartupSettingListener(const std::string & executableName) {
	std::string appPath;

#if defined(OS_WINDOWS)
	//add the -b option to start in background mode
	appPath = Path::getApplicationDirPath() + executableName + " -b";
#elif defined (OS_MACOSX)
	(void)executableName; // Avoid gcc warning
	appPath = Path::getApplicationBundlePath();
#else
	(void)executableName; // Avoid gcc warning
#endif
	_startup = new Startup("VoxOxNG", appPath);//VOXOX CHANGE by Rolando 01-20-09, change Wengophone to VoxOx
	_startup->setStartup(ConfigManager::getInstance().getCurrentConfig().getGeneralAutoStart());

	ConfigManager::getInstance().getCurrentConfig().valueChangedEvent +=
		boost::bind(&StartupSettingListener::startupSettingChanged, this, _1);
}

StartupSettingListener::~StartupSettingListener() {
	if (_startup) {
		delete _startup;
	}
}

void StartupSettingListener::startupSettingChanged(const std::string & key) const {
	if (key == Config::GENERAL_AUTOSTART_KEY) {
		_startup->setStartup(ConfigManager::getInstance().getCurrentConfig().getGeneralAutoStart());
	}
}
