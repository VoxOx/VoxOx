
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

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <util/File.h>

bool Processes::isRunning(const std::string & searchName) {
	struct dirent * ent;
	DIR * proc;
	char buffer[512];

	uid_t userId = getuid();

	proc = opendir("/proc");
	int count = 0;
	while ((ent = readdir (proc))) {
		// Skip files in /proc which do not represent processes.
		if (!isdigit(ent->d_name[0])) {
			continue;
		}

		std::string procPidPath = std::string("/proc/") + ent->d_name;

		// /proc/$pid is owned by the user who started the process. Skip
		// any process which do not belong to us.
		struct stat statBuffer;
		stat(procPidPath.c_str(), &statBuffer);
		if (statBuffer.st_uid != userId) {
			continue;
		}

		// /proc/$pid/exe is an absolute symlink to the full path of the
		// binary
		std::string procPidExePath = procPidPath + "/exe";
		ssize_t size = readlink(procPidExePath.c_str(), buffer, sizeof(buffer) - 1);
		if (size == -1) {
			continue;
		}
		buffer[size] = '\0';

		std::string binaryPath = std::string(buffer);
		File binaryFile(binaryPath);

		if (binaryFile.getFileName() == searchName) {
			count++;
			if (count > 1) {
				break;
			}
		}
	}
	closedir(proc);

	return ( count > 1);
}

bool Processes::killProcess(const std::string & processName) {
	return false;
}
