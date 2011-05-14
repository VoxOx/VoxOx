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

#ifndef OWWENGOPHONEBUILDID_H
#define OWWENGOPHONEBUILDID_H

#include <string>
/**
 * Identifies uniquely a revision of WengoPhone.
 *
 * @author Tanguy Krotoff
 */
class WengoPhoneBuildId {
public:

	/**
	 * Build id is the WengoPhone compilation date/time (e.g 2005/12/07 - 18:18:09)
	 *
	 * This is a unique id for each release.
	 * It permits WengoPhone to check if a more recent version of itself exist.
	 * Format: year/month/day hour:minutes:seconds
	 *
	 * Do not modify this to a string, keep it as a number!
	 */
	static const unsigned long long getBuildId();

	/**
	 * Version name (e.g "1.2.1")
	 * Human readable representation of the build id.
	 *
	 * This is a unique string for each release.
	 */
	static const char * getVersion();
	static const char * getFullVersion();	//VOXOX - JRT - 2009.08.12 
	static const char * getNameAndFullVersion();	//VOXOX - JRT - 2009.11.03 


	static const long getBuildNumber();		//VOXOX - JRT - 2009.07.23 

	/**
	 * Revision number (subversion revision).
	 *
	 * Check console command 'svnversion -n':
	 *
	 * <pre>
	 * The version number will be a single number if the working
	 * copy is single revision, unmodified, not switched and with
	 * an URL that matches the TRAIL_URL argument.  If the working
	 * copy is unusual the version number will be more complex:
	 *
	 * 4123:4168     mixed revision working copy
	 * 4168M         modified working copy
	 * 4123S         switched working copy
	 * 4123:4168MS   mixed revision, modified, switched working copy
	 *
	 * If invoked on a directory that is not a working copy, an
	 * exported directory say, the program will output 'exported'.
	 * </pre>
	 */
	static const char * getSvnRevision();

	/**
	 * Softphone name when connecting on the Wengo platform.
	 * WL_TAG
	 */
	static const char * getSoftphoneName();

	/**
	 * Boost version number.
	 *
	 * Cannot be named BOOST_VERSION because of a define inside Boost.
	 */
	static const char * getBoostVersion();

	/** LibPurple version number. */
	static const char * getPurpleVersion();

	/** GLib version number. */
	static const char * getGLibVersion();

	/** cURL version number. */
	static const char * getCurlVersion();

	/** PortAudio version number. */
	static const char * getPortaudioVersion();

	/** FFmpeg' libavcodec version number. */
	static const char * getAvcodecVersion();

	/** TinyXML version number. */
	static const char * getTinyXmlVersion();

private:

	/** Avoid copying of this object. */

	WengoPhoneBuildId() { }
	WengoPhoneBuildId(const WengoPhoneBuildId &);
	WengoPhoneBuildId& operator=(const WengoPhoneBuildId &);

	static std::string	_fullVersion;
	static std::string	_nameAndFullVersion;	//VOXOX - JRT - 2009.11.03 
};

#endif	//OWWENGOPHONEBUILDID_H
