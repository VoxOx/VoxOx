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

#ifndef OWPROCESSES_H
#define OWPROCESSES_H

#include <system/owsystemdll.h>

#include <util/NonCopyable.h>

#include <string>

/**
 * Check if a process is running or not + kill a running process.
 *
 * @author Mathieu Stute
 */
class Processes : NonCopyable {
public:

	/**
	 * Checks if a process is running.
	 *
	 * @param processName name of the process
	 * @return true if the process is running; false otherwise
	 */
	OWSYSTEM_API static bool isRunning(const std::string & processName);

	/**
	 * Kills a running process.
	 *
	 * @param processName name of the process
	 * @return true if the process was killed; false otherwise
	 */
	OWSYSTEM_API static bool killProcess(const std::string & processName);
};

#endif	//OWPROCESSES_H
