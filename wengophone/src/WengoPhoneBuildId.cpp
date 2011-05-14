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

#include "WengoPhoneBuildId.h"

#include <cutil/global.h>
#include <util/String.h>

#include <curl/curl.h>
#if !defined(OS_MACOSX) && defined(OWSOUND_PORTAUDIO_SUPPORT)
	#include <portaudio.h>
#endif
#include <boost/version.hpp>
#include <tinyxml.h>
extern "C" {
#include <avcodec.h>
#include <glib.h>
#include <libpurple/core.h>
}

#define stringize2(x) #x
#define stringize(x) stringize2(x)

std::string	WengoPhoneBuildId::_fullVersion = "";	//VOXOX - JRT - 2009.08.12 
std::string	WengoPhoneBuildId::_nameAndFullVersion = "";	//VOXOX - JRT - 2009.11.03  

const unsigned long long WengoPhoneBuildId::getBuildId() {
	return DD_BUILDID;
}

const char * WengoPhoneBuildId::getVersion() {
	return stringize(DD_VERSION);
}
//VOXOX - JRT - 2009.08.12 
const char * WengoPhoneBuildId::getFullVersion() {
	if ( _fullVersion.empty() )
	{
		_fullVersion = getVersion();
		_fullVersion += ".";
		_fullVersion += getSvnRevision();
	}

	return _fullVersion.c_str();
}

//VOXOX - JRT - 2009.11.03 
const char * WengoPhoneBuildId::getNameAndFullVersion()
{
	if ( _nameAndFullVersion.empty() )
	{
		_nameAndFullVersion = getSoftphoneName();
		_nameAndFullVersion += "-";
		_nameAndFullVersion += getFullVersion();
	}

	return _nameAndFullVersion.c_str();
}


const long WengoPhoneBuildId::getBuildNumber() {
	return DD_BUILDNUMBER;
}

const char * WengoPhoneBuildId::getSvnRevision() {
	return stringize(DD_REVISION);
}

const char * WengoPhoneBuildId::getSoftphoneName() {
	return stringize(DD_SOFTPHONE_NAME);
}

const char * WengoPhoneBuildId::getBoostVersion() {
	static std::string version(BOOST_LIB_VERSION);
	return version.c_str();
}

const char * WengoPhoneBuildId::getPurpleVersion() {
	return purple_core_get_version();
}

const char * WengoPhoneBuildId::getGLibVersion() {
	static std::string version(String::fromNumber(GLIB_MAJOR_VERSION) + "." +
		String::fromNumber(GLIB_MINOR_VERSION) + "." +
		String::fromNumber(GLIB_MICRO_VERSION));
	return version.c_str();
}

const char * WengoPhoneBuildId::getCurlVersion() {
	return curl_version();
}

const char * WengoPhoneBuildId::getPortaudioVersion() {
#if !defined(OS_MACOSX) && defined(OWSOUND_PORTAUDIO_SUPPORT)
	return Pa_GetVersionText();
#endif
	return "";
}

const char * WengoPhoneBuildId::getAvcodecVersion() {
	static std::string version(String::fromNumber(avcodec_version()));
	return version.c_str();
}

const char * WengoPhoneBuildId::getTinyXmlVersion() {
	static std::string version(String::fromNumber(TIXML_MAJOR_VERSION) + "." +
		String::fromNumber(TIXML_MINOR_VERSION) + "." +
		String::fromNumber(TIXML_PATCH_VERSION));
	return version.c_str();
}
