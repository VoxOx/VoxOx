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

#ifndef OWSTARTUP_H
#define OWSTARTUP_H

#include <util/owutildll.h>
#include <util/NonCopyable.h>

#include <string>

/**
 * Sets or not an application as a startup application.
 *
 * Statups the application when the Operating System is launched.
 *
 * @author Tanguy Krotoff
 */
class OWUTIL_API Startup : NonCopyable {
public:

	/**
	 * @param applicationName name of the application
	 *        (e.g WengoPhone)
	 * @param executablePath path to the executable
	 *        (e.g C:/Program Files/Wengo/wengophone.exe -background)
	 */
	 Startup(const std::string & applicationName, const std::string & executablePath);

	 ~Startup();

	/**
	 * Sets an executable as an application startup.
	 *
	 * @param startup true if the application should start or not
	 * @return true if success, false otherwise
	 */
	 bool setStartup(bool startup = true);

	/**
	 * Gets if the executable is an application startup.
	 *
	 * @return true if an application startup, false otherwise
	 */
	 bool isStartup();

private:

	/**
	 * Name of the application (e.g WengoPhone).
	 */
	std::string _applicationName;

	/**
	 * Path of the executable (e.g C:/Program Files/Wengo/wengophone.exe)
	 */
	std::string _executablePath;
};

#endif	//OWSTARTUP_H
