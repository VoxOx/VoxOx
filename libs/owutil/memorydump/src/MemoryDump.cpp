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

#include <memorydump/MemoryDump.h>

#include <util/Date.h>
#include <util/Time.h>
#include <util/String.h>
#include <util/Path.h>
#include <util/Logger.h>

std::string MemoryDump::_applicationName;
std::string MemoryDump::_styleName;
std::string MemoryDump::_languageFilename;
std::string MemoryDump::_revision;
std::string MemoryDump::_fileNamePrefix;		//VOXOX - JRT - 2009.07.23 
std::string (*MemoryDump::getAdditionalInfo)();
google_breakpad::ExceptionHandler * MemoryDump::_exceptionHandler = NULL;


MemoryDump::MemoryDump(const std::string & applicationName, const std::string & revision, const char* prefix) {	//VOXOX - JRT - 2009.07.23 
	_applicationName = applicationName;
	_revision = revision;
	_fileNamePrefix = prefix;	//VOXOX - JRT - 2009.07.23 
}

MemoryDump::~MemoryDump() {
}

void MemoryDump::setStyle(const std::string & styleName) {
	_styleName = styleName;
}

void MemoryDump::setLanguage(const std::string & languageFilename) {
	_languageFilename = languageFilename;
}

std::string MemoryDump::getCurrentDateTime() {
	String date = Date().toString();
	date.replace("-", "");

	String time = Time().toString();
	time.replace(":", "");

	return date + time;
}

void MemoryDump::setGetAdditionalInfo(std::string (*proc)()) {
	getAdditionalInfo = proc;
}

