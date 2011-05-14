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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "CommandLineParser.h"

#include <boost/program_options.hpp>
#include <iostream>
using namespace boost::program_options;
using namespace std;

#include <util/File.h>
#include <util/Path.h>
#include <util/String.h>

#include <cutil/global.h>

static std::string getDefaultResourcesDir() {
	// Init resourcesDir
	std::string resourcesDir;
#if defined(OS_WINDOWS)
	resourcesDir = Path::getApplicationDirPath();
#elif defined(OS_MACOSX)
	resourcesDir = Path::getApplicationResourcesDirPath();
#elif defined(OS_LINUX) and defined(OW_RESOURCEDIR)
	resourcesDir = OW_RESOURCEDIR + File::getPathSeparator();
#endif

	//Check if resourcesDir exist, if not then Path::getApplicationDirPath()
	if (!File::exists(resourcesDir)) {
		resourcesDir = Path::getApplicationDirPath();
	}

	return resourcesDir;
}

static std::string getDefaultUserConfigDir(const std::string& binaryName) {
	std::string configDir = Path::getConfigurationDirPath();
#if defined(OS_LINUX)
	configDir += '.';
#endif
	//configDir += binaryName;
	configDir += "VoxOx2";//VOXOX - CJC - 2009.07.10 
	configDir += Path::getPathSeparator();
	return configDir;
}

// FIXME: binaryName should get read from buildconfig, but it's only available
// in the qt subdir for now :-(
CommandLineParser::CommandLineParser(const std::string& binaryName, int argc, char * argv[]) {
	_severalWengoPhoneAllowed = false;
	_runInBackground = false;
	_resourcesDir = getDefaultResourcesDir();
	//_userConfigDir = getDefaultUserConfigDir(binaryName);
	_userConfigDir = getDefaultUserConfigDir("VoxOx2");//VOXOX - CJC - 2009.07.10 

	try {
		options_description desc("Allowed options");
		desc.add_options()
		//First parameter describes option name/short name
		//The second is parameter to option
		//The third is description
		("help,h", "print usage message")
		("background,b", "run in background mode")
		("several,s", "allow several VoxOx at the same time")//VOXOX CHANGE by Rolando 01-20-09, change Wengophone to VoxOx
		("configpath,q", value<string>(), "pass a path to load configuration directory")
		("resources", value<string>(), "use a custom resources directory")
		("command,c", value<string>(), "pass a command to the VoxOx")//VOXOX CHANGE by Rolando 01-20-09, change Wengophone to VoxOx
		;

		variables_map vm;
		store(parse_command_line(argc, argv, desc), vm);

		// Handle help first
		if (vm.count("help")) {
			cout << desc << endl;
			exit(0);
		}

		// Now init config
		if (vm.count("resources")) {
			_resourcesDir = vm["resources"].as<string>();
		}

		if (vm.count("configpath")) {
			_userConfigDir = vm["configpath"].as<string>();
			std::string pathSeparator = File::getPathSeparator();
			
			int size = _userConfigDir.size();
			if (_userConfigDir[size-1] != pathSeparator[0]) {
				
				// remove " at the end
				if (_userConfigDir[size-1] == '\"') {
					_userConfigDir = _userConfigDir.substr(0,size-1);
				}
				////
				
				// add path separator
				_userConfigDir += pathSeparator;
				////
			}
		}

		//Set executable name
		std::string executableName;
		if (argv[0]) {
			char * p = strrchr(argv[0], Path::getPathSeparator()[0]);
			_executableName = std::string(p ? p + 1 : argv[0]);
		}

		if (vm.count("background")) {
			_runInBackground = true;
		}
		
		if (vm.count("several")) {
			_severalWengoPhoneAllowed = true;
		}

		if (vm.count("command")) {
			static String commandCall = "call/";
			String command = String(vm["command"].as<string>());

			//FIXME is it right to do that here?
			command.remove("wengo://");

			if (command.beginsWith(commandCall)) {
				_command = command.split("/")[1];
			}
		}

	}
	catch(exception & e) {
		cerr << e.what() << endl;
	}
}

CommandLineParser::~CommandLineParser() {
}
