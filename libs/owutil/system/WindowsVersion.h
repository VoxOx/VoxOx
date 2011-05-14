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

#ifndef OWWINDOWSVERSION_H
#define OWWINDOWSVERSION_H

#include <system/owsystemdll.h>

#include <util/NonCopyable.h>
#include <string>

/**
 * Simple library to retrieve a human readable
 * string representing the windows version.
 *
 * String is something like:
 *
 * - Microsoft Windows Vista
 * - Microsoft Server Longhorn
 * - Microsoft Windows Server 2003 R2
 * - Microsoft Windows XP Professional x64 Edition
 * - Microsoft Windows Server 2003
 * - Microsoft Windows XP
 * - Microsoft Windows 2000
 * - Microsoft Windows NT
 *
 * + Various editions:
 *   - Workstation 4.0
 *   - Home Edition
 *   - Professional
 *   - Datacenter Edition for Itanium-based Systems
 *   - Datacenter x64 Edition
 *   - Enterprise x64 Edition
 *   - Standard x64 Edition
 *   - Datacenter Edition
 *   - Enterprise Edition
 *   - Web Edition
 *   - Standard Edition
 *   - Datacenter Server
 *   - Advanced Server
 *   - Server
 *   - Server 4.0, Enterprise Edition
 *   - Server 4.0
 *   - Workstation
 *   - Server
 *   - Advanced Server
 *   - Service Pack 6a (Build N)
 *   - (Build N)
 *
 * - Microsoft Windows 95
 *   - OSR2
 * - Microsoft Windows 98
 *   - A
 *   - B
 *   - SE
 * - Microsoft Windows Millennium Edition
 * - Microsoft Win32s
 *
 * @author Mathieu Stute
 */
class WindowsVersion : NonCopyable {
public:

	OWSYSTEM_API static char * getVersion();
	static std::string _version;
};

#endif	//OWWINDOWSVERSION_H
