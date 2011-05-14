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

/*
 * Check http://www.codeproject.com/debug/postmortemdebug_standalone1.asp
 */

#include <memorydump/MSVCMemoryDump.h>

#include <util/Logger.h>
#include <util/String.h>
#include <util/Path.h>

#include <shlwapi.h>

#include <cstring>
#include <ctime>
#include <cstdio>
#include <iostream>

std::wstring str2wstr(std::string& str)
{
	std::wstring wstr;
	wstr.assign(str.begin(), str.end());
	return wstr;
}

std::string wstr2str(std::wstring& wstr)
{
	std::string str;
	str.assign(wstr.begin(), wstr.end());
	return str;
}

MSVCMemoryDump::MSVCMemoryDump(const std::string & applicationName, const std::string & revision, const char* prefix )	//VOXOX - JRT - 2009.07.23 
	: MemoryDump(applicationName, revision, prefix) {

	std::wstring dumpPathw = str2wstr(Path::getApplicationDirPath());

	std::string  temp	   = prefix;
	std::wstring prefixw   = str2wstr(temp);		//VOXOX - JRT - 2009.07.23 

	_exceptionHandler = new ExceptionHandler(dumpPathw,	// dump path
			NULL,								// filter callback
			miniDumpCallBack,					// minidump call back
			NULL,								// call back context
			ExceptionHandler::HANDLER_ALL,		// handler_types
			prefixw			);					// Prefix for output file.		//VOXOX - JRT - 2009.07.23 
}

MSVCMemoryDump::~MSVCMemoryDump() {
	delete _exceptionHandler;
}

bool MSVCMemoryDump::miniDumpCallBack(const wchar_t* dump_path,
								  const wchar_t* minidump_id,
								  void* context,
								  EXCEPTION_POINTERS* exinfo,
								  MDRawAssertionInfo* assertion,
								  bool succeeded)
{
	if(succeeded) {

		//Launches crashreport.exe
		std::string commandLine = "owcrashreport";
		if (!_styleName.empty()) {
			commandLine += " -style=";
			commandLine += _styleName;
		}

		//Name of the memory dump
		//Use current path
		std::string memoryDumpName;

		if ( !_fileNamePrefix.empty() )
		{
			memoryDumpName += _fileNamePrefix;
			memoryDumpName += "-";
		}

		memoryDumpName += wstr2str(std::wstring(minidump_id));
		memoryDumpName += ".dmp";

		//GetModuleFileName retrieves the path of the executable file of the current process.
		std::string memoryDumpFile = wstr2str(std::wstring(dump_path));
		memoryDumpFile += memoryDumpName;

		commandLine += " -d ";
		commandLine += "\"";
		commandLine += memoryDumpFile;
		commandLine += "\"";

		commandLine += " -n ";
		commandLine += "\"";
		commandLine += _applicationName;
		commandLine += "\"";

		if (!_languageFilename.empty()) {
			commandLine += " -l ";
			commandLine += "\"";
			commandLine += _languageFilename;
			commandLine += "\"";
		}
		if (getAdditionalInfo) {
			commandLine += " -i ";
			commandLine += "\"";
			commandLine += getAdditionalInfo();
			commandLine += "\"";
		}
		
		//Flushes the logger file
		//Logger::logger.flush();

		executeProcess(commandLine);
	}	
	
	// force to terminate
	TerminateProcess(GetCurrentProcess(), 0);
	
	return succeeded;
}

BOOL MSVCMemoryDump::executeProcess(const std::string & commandLine) {
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	GetStartupInfoA(&si);

	BOOL success = CreateProcessA(NULL,
		(char *) commandLine.c_str(),	//Name of app to launch
		NULL,	//Default process security attributes
		NULL,	//Default thread security attributes
		FALSE,	//Don't inherit handles from the parent
		0,	//Normal priority
		NULL,	//Use the same environment as the parent
		NULL,	//Launch in the current directory
		&si,	//Startup Information
		&pi);	//Process information stored upon return

	return success;
}
