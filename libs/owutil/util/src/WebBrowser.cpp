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

//#include "stdafx.h"		//VOXOX - JRT - 2009.04.03 - For precompiled headers.
#include <util/WebBrowser.h>

#include <util/Logger.h>
#include <cutil/global.h>

static std::string defaultBrowser;

#ifdef OS_WINDOWS

#include <windows.h>

bool windows_open_url(std::string url) {

	static const char * browserCommand = "open";

	//Uses shell to invoke the web browser
	//typedef void * HINSTANCE;
	int ret = (int) ::ShellExecuteA(NULL, browserCommand, url.c_str(),
				NULL, NULL, SW_SHOWNORMAL);

	if (ret > 32) {
		return true;
	}

	return false;
}

#elif defined (OS_LINUX)

#include <unistd.h>
#include <stdio.h>

void linux_open_url(const char * url) {
	if (!fork()) {

		if (!defaultBrowser.empty()) {
			LOG_DEBUG("use browser=" + defaultBrowser);
			execlp(defaultBrowser.c_str(), defaultBrowser.c_str(), url, NULL);
		}

		//xdg-open: http://portland.freedesktop.org/xdg-utils-1.0/xdg-open.html
		// from the Portland project, will use the default setup browser
		// regardless of the desktop used.
		execlp("xdg-open", "xdg-open", url, NULL);

		//use $BROWSER if it exists
		char * browser = getenv("BROWSER");
		if (browser) {
			LOG_DEBUG("use browser=" + std::string(browser));
			execlp(browser, browser, url, NULL);
		}

		//sensible-browser script
		execlp("sensible-browser", "sensible-browser", url, NULL);

		//firefox
		execlp("firefox", "firefox", url, NULL);

		//kfmclient from KDE
		execlp("kfmclient", "kfmclient" , "openURL", url, NULL);

		//gnome-open
		execlp("gnome-open", "gnome-open" , url, NULL);

		//mozilla
		execlp("mozilla", "mozilla", url, NULL);

		//galeon
		execlp("galeon", "galeon", url, NULL);

		//epiphany
		execlp("epiphany", "epiphany", url, NULL);

		//lynx
		execlp("lynx", "lynx", url, NULL);

		perror(NULL);
		exit(1);
	}
}

#elif defined OS_MACOSX

#include <Carbon/Carbon.h>

bool mac_open_url(std::string url) {
	// If no protocol has been specified, assume this is http. This is
	// consistent with other platforms
	// See ticket #980
	if (url.find("://") == std::string::npos) {
		url = "http://" + url;
	}
	CFStringRef urlString = CFStringCreateWithCString(NULL, url.c_str(), 0);
	CFURLRef urlRef = CFURLCreateWithString(NULL, urlString, NULL);
	return (LSOpenCFURLRef(urlRef, NULL) == noErr);
}

#endif

void WebBrowser::setBrowser(const std::string & browser) {
	defaultBrowser = browser;
}

bool WebBrowser::openUrl(const std::string & url) {
#ifdef OS_WINDOWS
	return windows_open_url(url);
#elif defined OS_LINUX
	linux_open_url(url.c_str());
#elif defined OS_MACOSX
	return mac_open_url(url);
#endif
	return false;
}

