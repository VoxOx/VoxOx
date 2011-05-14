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

//#include "stdafx.h"		//VOXOX - JRT - 2009.04.03 - For precompiled headers.
#include <util/Path.h>

#include <util/File.h>
#include <util/String.h>
#include <util/Logger.h>

#if defined(OS_MACOSX)
	#include <CoreFoundation/CoreFoundation.h>
#elif defined(OS_WINDOWS)
	#include <windows.h>
	#include "ShlObj.h"			//VOXOX - JRT - 2009.12.15 
#elif defined(OS_LINUX)
	#include <stdio.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif

#if !defined(OS_WINDOWS)
	static const int MAX_PATH = 256;
#endif

using namespace std;

std::string Path::_VoxOxAppDataDir = "VoxOx2";

string Path::getApplicationDirPath() 
{
	string result = "";

#if defined(OS_WINDOWS)

	char moduleName[MAX_PATH];
	GetModuleFileNameA(NULL, moduleName, sizeof(moduleName));

	File file(moduleName);
	result = file.getPath();
	result += File::getPathSeparator();

#elif defined(OS_MACOSX)

	CFBundleRef mainBundle = CFBundleGetMainBundle();
	if (mainBundle) {
		CFURLRef execUrl = CFBundleCopyExecutableURL(mainBundle);
		CFURLRef url = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, execUrl);

		char applicationPath[MAX_PATH];

		if (CFURLGetFileSystemRepresentation(url, true, (UInt8 *) applicationPath, sizeof(applicationPath))) {
			result = (string(applicationPath) + File::getPathSeparator());
		}

		CFRelease(execUrl);
		CFRelease(url);
	}

#elif defined(OS_LINUX)

	char procname[MAX_PATH];
	memset(procname, 0, sizeof(procname));
	pid_t pid = getpid();
	if (snprintf(procname, sizeof(procname), "/proc/%i/exe", pid) < 0) {
		return String::null;
	}

	char buff[MAX_PATH];
	memset(buff, 0, sizeof(buff));
	int ret = readlink(procname, buff, sizeof(buff));

	if (ret == -1 || (unsigned int) ret >= sizeof(buff)) {
		return String::null;
	}

	buff[ret] = 0;
	string mstr(buff);
	File f(mstr);
	result = f.getPath() + File::getPathSeparator();

#endif

	return result;
}

string Path::getConfigurationDirPath() 
{
	string result = "";

#if defined(OS_WINDOWS)
	//VOXOX - JRT - 2009.12.15 - Use Windows API calls.
	result  = Path::AppDataDir();
	result += File::getPathSeparator();

	//char *appData = getenv("APPDATA");
	//if(appData){
	//	// remove the last separator to appData
	//	String tmpAppData = String(appData);
	//	if ((!tmpAppData.endsWith("\\")) || (!tmpAppData.endsWith("/"))) {
	//		tmpAppData = tmpAppData + File::getPathSeparator();
	//	}
	//	////

	//	result = tmpAppData;
	//} else {
	//	result = getHomeDirPath() + File::convertPathSeparators("Application Data/");	
	//}

#elif defined(OS_MACOSX)

	result = getHomeDirPath() + File::convertPathSeparators("Library/Application Support/");

#elif defined(OS_POSIX)

	result = getHomeDirPath();

#endif

	return result;
}

//VOXOX - JRT - 2009.12.10 
string Path::getVoxOxConfigurationDirPath( bool includeTrailingSeparator ) 
{
	std::string configDir = Path::getConfigurationDirPath();

#if defined(OS_LINUX)
	configDir += '.';
#endif
	
	configDir += Path::_VoxOxAppDataDir;

	if ( includeTrailingSeparator )
	{
		configDir += Path::getPathSeparator();
	}

	return configDir;
}


std::string Path::getPathSeparator() 
{
	return File::getPathSeparator();
}


string Path::getHomeDirPath() 
{
	string result = "";

#if defined(OS_WINDOWS)
	//VOXOX - JRT - 2009.12.15 - Use Windows API calls.
	result = Path::UserProfileDir();

	//char * homeDrive = getenv("HOMEDRIVE");
	//char * homeDir = getenv("HOMEPATH");

	//if (homeDrive && homeDir) 
	//{
	//	// remove the last separator to homeDrive
	//	String tmpHomeDrive = String(homeDrive);
	//	if ((tmpHomeDrive.endsWith("\\")) || (tmpHomeDrive.endsWith("/"))) {
	//		tmpHomeDrive.substr(0, tmpHomeDrive.size() - 2);
	//	}
	//	////

	//	// add the leading separator to homeDir
	//	String tmpHomeDir = String(homeDir);
	//	if ((!tmpHomeDir.beginsWith("\\")) || (!tmpHomeDir.beginsWith("/"))) {
	//		tmpHomeDir = File::getPathSeparator() + tmpHomeDir;
	//	}
	//	////

	//	result = tmpHomeDrive + tmpHomeDir;
	//}

#elif defined(OS_POSIX)
	char * homeDir = getenv("HOME");
	if (homeDir) {
		result = homeDir;
	}
#endif
	result += File::getPathSeparator();

	return result;
}

#ifdef OS_MACOSX

std::string Path::getApplicationBundlePath() {
	std::string result;
	CFBundleRef mainBundle = CFBundleGetMainBundle();

	if (mainBundle) {
		CFURLRef url = CFBundleCopyBundleURL(mainBundle);
		char bundlePath[MAX_PATH];

		if (CFURLGetFileSystemRepresentation(url, true, (UInt8 *) bundlePath, sizeof(bundlePath))) {
			result = (std::string(bundlePath) + File::getPathSeparator());
		}

		CFRelease(url);
	}

	return result;
}

std::string Path::getApplicationPrivateFrameworksDirPath() {
	std::string result;
	CFBundleRef mainBundle = CFBundleGetMainBundle();

	if (mainBundle) {
		CFURLRef url = CFBundleCopyPrivateFrameworksURL(mainBundle);
		char frameworkPath[MAX_PATH];

		if (CFURLGetFileSystemRepresentation(url, true, (UInt8 *) frameworkPath, sizeof(frameworkPath))) {
			result = (std::string(frameworkPath) + File::getPathSeparator());
		}

		CFRelease(url);
	}

	return result;
}

std::string Path::getApplicationResourcesDirPath() {
	std::string result;
	CFBundleRef mainBundle = CFBundleGetMainBundle();

	if (mainBundle) {
		CFURLRef url = CFBundleCopyResourcesDirectoryURL(mainBundle);
		char resPath[MAX_PATH];

		if (CFURLGetFileSystemRepresentation(url, true, (UInt8 *) resPath, sizeof(resPath))) {
			result = (std::string(resPath) + File::getPathSeparator());
		}

		CFRelease(url);
	}

	return result;
}
#endif

//-----------------------------------------------------------------------------

#ifdef OS_WINDOWS
//For details on Window Folder API, see http://msdn.microsoft.com/en-us/library/bb762494(VS.85).aspx

// This method will NOT return the folder name in the pszPath
//		parameter if the Lib is built with either of the following Optimizations:
//			Maximimum Speed (/O2)
//			Full Optimization ( /Ox )
//		It works fine with:
//			Disabled (/Od) or Minimize Size (/O1 ).
// BIG NOTE: You will only see this in Release builds because optimizations
//			are typically disabled (/Od ) for Debug builds.
bool Path::ShellGetFolderPath( int nFolder, char* pszPath )
{
	HRESULT hr = ::SHGetFolderPath( NULL, nFolder, NULL, SHGFP_TYPE_CURRENT, pszPath );
	return (hr == S_OK);
}

//-----------------------------------------------------------------------------

std::string Path::WinDesktop()
{
    TCHAR desktopFolder[MAX_PATH];
    Path::ShellGetFolderPath( CSIDL_DESKTOPDIRECTORY, desktopFolder );
    return desktopFolder;
}

//-----------------------------------------------------------------------------

std::string Path::MyDocs()
{
    TCHAR myDocumentsFolder[MAX_PATH];
    Path::ShellGetFolderPath( CSIDL_PERSONAL, myDocumentsFolder );
    return myDocumentsFolder;
}

//-----------------------------------------------------------------------------

std::string Path::MyPictures()
{
    TCHAR myPicsDir[MAX_PATH];
    Path::ShellGetFolderPath( CSIDL_MYPICTURES, myPicsDir );
    return myPicsDir;
}

//-----------------------------------------------------------------------------

std::string Path::AppDataDir()
{
    TCHAR appDataDir[MAX_PATH];
	Path::ShellGetFolderPath( CSIDL_APPDATA, appDataDir );
    return appDataDir;
}

//-----------------------------------------------------------------------------

std::string Path::LocalAppDataDir()
{
    TCHAR appDataDir[MAX_PATH];
	Path::ShellGetFolderPath( CSIDL_LOCAL_APPDATA , appDataDir );
    return appDataDir;
}

//-----------------------------------------------------------------------------

std::string Path::UserProfileDir()
{
    TCHAR appDataDir[MAX_PATH];
	Path::ShellGetFolderPath( CSIDL_PROFILE, appDataDir );
    return appDataDir;
}

//-----------------------------------------------------------------------------

std::string Path::WindowsDir()
{
    TCHAR strFolder[MAX_PATH];
    Path::ShellGetFolderPath( CSIDL_WINDOWS, strFolder );
    return strFolder;
}

//-----------------------------------------------------------------------------

std::string Path::SystemDir()	//C:\Windows\System32
{
    TCHAR strFolder[MAX_PATH];
    Path::ShellGetFolderPath( CSIDL_SYSTEM, strFolder );
    return strFolder;
}

//-----------------------------------------------------------------------------
// get the path to a directory containing default logon images
std::string Path::WindowsDefaultPicsDir()
{
	TCHAR appData[ MAX_PATH ];
	std::string res;
    Path::ShellGetFolderPath( CSIDL_COMMON_APPDATA, appData );
	res  = appData;
//	res += _T("\\Microsoft\\User Account Pictures\\Default Pictures");
	res += "\\Microsoft\\User Account Pictures\\Default Pictures";
	return res;
}

//-----------------------------------------------------------------------------

//Full file path of current program.
//std::string Path::Exe() 
//{
//    TCHAR	exePath[1024];
//    GetModuleFileName( AfxGetInstanceHandle(), exePath, sizeof(exePath));
//    return exePath;
//}

//-----------------------------------------------------------------------------

//std::string Path::ExePath() 
//{
//	std::string exePath = Path::Exe();
//    return Path::Path( exePath );
//}

//-----------------------------------------------------------------------------

#endif