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

#ifndef OWMSVCMEMORYDUMP_H
#define OWMSVCMEMORYDUMP_H

#include <memorydump/MemoryDump.h>

#include <windows.h>
#include <string>

/**
 * Creates a mini dump (Crash Dump File .dmp) of the current application running.
 *
 * A mini dump is the equivalent of a core dump under Windows.
 *
 * It creates a file inside the current directory named something like:
 * applicationName-dateTime.dmp
 * Example: WengoPhone-0.93-0602124133.dmp
 * with applicationName = WengoPhone-0.93
 * and dateTime = 0602124133 (month day hour minutes seconds)
 * dateTime is only here in order to be sure to generate a unique file.
 *
 * What to do in order to read memorydumps? (.dmp files)
 * You need:
 * - Microsoft Visual C++
 * - the .exe and .dll files in debug mode
 * - all the .pdb (Program Debug Database) files generated during the compilation process in debug mode
 * - open the .dmp file in Visual C++ it will show you where the crash occured
 * Remember that .exe, .dll and .pdb files should be generated from the same compilation process in debug mode.
 * If a .pdb file does not correspond to the exact .exe or .dll file, the .dmp file is inefficient.
 *
 * MemoryDump calls an external program (crashreport.exe) with the following arguments:
 * crashreport.exe -style=styleName "memoryDumpFile" applicationName languageFile
 * Example:
 * crashreport.exe -style=wengokeramik "C:/Program Files/Wengo/WengoPhone-0.93-0602124133.dmp" WengoPhone wengo_fr.qm
 * crashreport.exe can be wathever you want.
 * Currently it is a Qt application (MemoryDumpWindow) that sends via FTP the crashreport.
 *
 * So it uses MemoryDumpWindow as a separate executable file otherwise
 * some crashes do not open MemoryDumpWindow (probably because of the dependency with the Qt dll).
 * MemoryDump does not rely on Qt.
 *
 * @author Tanguy Krotoff
 */
class MSVCMemoryDump : public MemoryDump {
public:

	/**
	 * Constructs a MemoryDump object.
	 *
	 * @param applicationName name of the application (e.g WengoPhone)
	 */
	OWMEMORYDUMP_API MSVCMemoryDump(const std::string & applicationName, const std::string & revision, const char* prefix);	//VOXOX - JRT - 2009.07.23 

	OWMEMORYDUMP_API ~MSVCMemoryDump();

private:

	static bool miniDumpCallBack(const wchar_t* dump_path,
		const wchar_t* minidump_id,
		void* context,
		EXCEPTION_POINTERS* exinfo,
		MDRawAssertionInfo* assertion,
		bool succeeded);
	/**
	 * Executes a process given a command line.
	 *
	 * Uses CreateProcess() from the win32 API.
	 *
	 * @param commandLine executable path + parameters
	 */
	static BOOL executeProcess(const std::string & commandLine);
};

#endif	//OWMSVCMEMORYDUMP_H
