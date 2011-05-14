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

#ifndef OWMEMORYDUMP_H
#define OWMEMORYDUMP_H

#include <memorydump/owmemorydumpdll.h>
#include <exception_handler.h>
using namespace google_breakpad;

#include <util/Interface.h>

#include <string>

/**
 * Creates a memory dump of a software after it crashes.
 *
 * Implementations for UNIX, Windows (MSVC) and MacOSX.
 *
 * @author Mathieu Stute
 */
class MemoryDump : public Interface {
public:

	/**
	 * Constructs a MemoryDump object.
	 *
	 * @param applicationName name of the application (e.g WengoPhone)
	 */
	MemoryDump(const std::string & applicationName, const std::string & revision, const char* prefix);	//VOXOX - JRT - 2009.07.23 

	virtual ~MemoryDump();

	/**
	 * Sets the Qt widgets style to use.
	 *
	 * @param styleName name of the style to use
	 */
	OWMEMORYDUMP_API void setStyle(const std::string & styleName);

	/**
	 * Sets the language for translating MemoryDump.
	 *
	 * @param languageFilename path to the Qt translation file (.qm)
	 */
	OWMEMORYDUMP_API void setLanguage(const std::string & languageFilename);

	/**
	 * Sets additional informations via a callback.
	 *
	 * A callback is used because some informations can only be known
	 * when the software crashes.
	 *
	 * @param proc callback function that will be executed
	 */
	OWMEMORYDUMP_API static void setGetAdditionalInfo(std::string (*proc)());

protected:

	/**
	 * Gets the current date time as a 14 characters string: 20050211234613
	 *
	 * @return the current date time as a string
	 */
	static std::string getCurrentDateTime();

	/**
	 * Callback function to get additional informations.
	 */
	static std::string (*getAdditionalInfo)();

	/**
	 * Name of the application that uses MemoryDump.
	 */
	static std::string _applicationName;

	/**
	 * Qt widgets style name.
	 */
	static std::string _styleName;

	/**
	 * Path to the translation file.
	 */
	static std::string _languageFilename;

	/**
	 * Name of the application that uses MemoryDump.
	 */
	static std::string _revision;

	/**
	* Exception handler.
	*/
	static ExceptionHandler* _exceptionHandler;

	static std::string	_fileNamePrefix;		//VOXOX - JRT - 2009.07.23 

};

#endif	//OWMEMORYDUMP_H
