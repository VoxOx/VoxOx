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

#ifndef OWCOMMANDLINEPARSER_H
#define OWCOMMANDLINEPARSER_H

#include <string>

/**
 * Parse command line option.
 *
 * @ingroup model
 * @author Mathieu Stute
 * @euthor Xavier Desjardins
 * @euthor Aurelien Gateau
 */
class CommandLineParser {
public:

	CommandLineParser(const std::string& binaryName, int argc, char * argv[]);

	~CommandLineParser();
	
	bool isSeveralWengoPhoneAllowed() const {
		return _severalWengoPhoneAllowed;
	}

	bool getRunInBackground() const {
		return _runInBackground;
	}

	std::string getResourcesDir() const {
		return _resourcesDir;
	}

	std::string getUserConfigDir() const {
		return _userConfigDir;
	}

	std::string getCommand() const {
		return _command;
	}

	std::string getExecutableName() const {
		return _executableName;
	}

private:
	bool _severalWengoPhoneAllowed;
	bool _runInBackground;
	std::string _resourcesDir;
	std::string _userConfigDir;
	std::string _command;
	std::string _executableName;
};

#endif	//OWCOMMANDLINEPARSER_H
