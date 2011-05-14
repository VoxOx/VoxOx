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

#ifndef OWPATH_H
#define OWPATH_H

#include <cutil/global.h>

#include <util/owutildll.h>
#include <util/NonCopyable.h>

#include <string>

/**
 * Path class.
 *
 * Give following services:
 * - application path
 * - resources path
 * - config path
 *
 * Returned paths includes a trailing separator.
 *
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class OWUTIL_API Path : NonCopyable {
public:

	/**
	 * Gets the directory that contains the application executable.
	 *
	 * The executable path returned always finished by "/" or "\".
	 *
	 * On MacOS X, returns full path to exe. e.g: /Application/WengoPhone.app/Contents/MacOS/WengoPhone
	 *
	 * @return application executable path
	 */
	 static std::string getApplicationDirPath();

	/**
	 * Gets the configuration directory, typically under user's
         * home directory.
	 *
	 * @return the configuration directory path
	 */
	 static std::string getConfigurationDirPath();
	 static std::string getVoxOxConfigurationDirPath( bool includeTrailingSeparator = true );	//VOXOX - JRT - 2009.12.10 


	/**
	 * @see File::getPathSeparator()
	 */
	 static std::string getPathSeparator();

	/**
	 * Gets the home directory path.
	 *
	 * @return the home dir of the current user.
	 */
	 static std::string getHomeDirPath();

#ifdef OS_MACOSX
	//Special extensions for MacOS X

	/**
	 * Gets the application bundle path.
	 *
	 * e.g: /Application/WengoPhone.app/
	 */
	 static std::string getApplicationBundlePath();

	/**
	 * Gets the Private Frameworks path.
	 *
	 * e.g: full-path-tp-application.app/Contents/Frameworks
	 */
	 static std::string getApplicationPrivateFrameworksDirPath();

	/**
	 * Gets the Resources path.
	 *
	 * e.g: full-path-tp-application.app/Contents/Resources
	 */
	 static std::string getApplicationResourcesDirPath();
#endif

	 //VOXOX - JRT - 2009.12.15 
#ifdef OS_WINDOWS
	static std::string Exe();
	static std::string ExePath();
	static std::string WinDesktop();
	static std::string MyDocs();
	static std::string MyPictures();
	static std::string AppDataDir();	//Roaming
	static std::string LocalAppDataDir();
	static std::string UserProfileDir();
	static std::string WindowsDir();
	static std::string SystemDir();		//C:\Windows\System32
	static std::string WindowsDefaultPicsDir();

private:
	static bool		   ShellGetFolderPath( int nFolder, char* pszPath );

#endif

	 static std::string _VoxOxAppDataDir;		//VOXOX - JRT - 2009.12.10 

};

#endif	//OWPATH_H
