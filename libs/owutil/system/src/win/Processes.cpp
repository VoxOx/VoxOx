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

#include <system/Processes.h>

#include <util/Logger.h>

#include <windows.h>
#include <tlhelp32.h>

bool Processes::isRunning(const std::string & processName) {
	//Check if another instance is already running
	int instance_counter = 0;
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(PROCESSENTRY32);

	if(!Process32First(hSnapShot, &processInfo)) {
		CloseHandle(hSnapShot);
		LOG_WARN("Could not get system process snapshot");
		return false;
	}

	do {
		std::string s(processInfo.szExeFile);
		if (s == processName + ".exe" || s == processName + ".ex") {
			instance_counter++;
		}
	} while (Process32Next(hSnapShot, &processInfo));

	CloseHandle(hSnapShot);

	//If another instance is detected, exit
	if (instance_counter >= 2) {
		return true;
	}
	return false;
}

bool Processes::killProcess(const std::string & processName) {
 	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 * processInfo = new PROCESSENTRY32;
	processInfo->dwSize = sizeof(PROCESSENTRY32);

	//Retrieve the pid
	while (Process32Next(hSnapShot,processInfo) != FALSE) {
		std::string s(processInfo->szExeFile);
		if (s == processName) {
			HANDLE ps = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, processInfo->th32ProcessID);
			delete processInfo;		//VOXOX - JRT - 2009.04.13 - Fix memory leak.
			return TerminateProcess(ps, 0);
		}
	}
	delete processInfo;		//VOXOX - JRT - 2009.04.13 - Fix memory leak.
	return false;
}
