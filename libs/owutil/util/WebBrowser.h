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

#ifndef OWWEBBROWSER_H
#define OWWEBBROWSER_H

#include <util/owutildll.h>
#include <util/NonCopyable.h>

#include <string>

/**
 * Opens the default web browser of the Operating System.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class OWUTIL_API WebBrowser : NonCopyable {
public:

	/**
	 * Opens an URL using the default web browser of the OS.
	 *
	 * @param url URL to open inside the web browser
	 * @return true if the web browser was found and the URL opened, false otherwise
	 */
	 static bool openUrl(const std::string & url);
	
	/*
	 * Set a user defined web browser to use on linux
	 */
	 static void setBrowser(const std::string & browser);
private:
};

#endif	//OWWEBBROWSER_H
