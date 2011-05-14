/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2007  Wengo
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

#include <memorydump/UNIXMemoryDump.h>
#include <util/Logger.h>
#include <util/Path.h>


UNIXMemoryDump::UNIXMemoryDump(const std::string & applicationName, const std::string & revision)
	: MemoryDump(applicationName, revision) {
		std::string dumpPath = "/tmp/";

		_exceptionHandler = new ExceptionHandler(dumpPath,	// dump path
			NULL,						// filter callback
			miniDumpCallBack,				// minidump call back
			NULL,						// call back context
			true);						// minidump will be written on exception

}

UNIXMemoryDump::~UNIXMemoryDump() {
	delete _exceptionHandler; 
}


bool UNIXMemoryDump::miniDumpCallBack(const char * dump_path,
						const char * minidump_id,
						void* context,
						bool succeeded)
{
        if(succeeded) {
		std::string memoryDumpName (minidump_id);
		memoryDumpName += ".dmp";
		std::string memoryDumpFile(dump_path);
	        memoryDumpFile += memoryDumpName;

                                
		std::string command = Path::getApplicationDirPath() + "owcrashreport";
                char * commandArgs[10];
                int i = 0;
                commandArgs[i++] = (char *)command.c_str();
                commandArgs[i++] = "-d";
                commandArgs[i++] = (char *)memoryDumpFile.c_str();
                commandArgs[i++] = "-n";
                commandArgs[i++] = (char *)_applicationName.c_str();
                
                if (!_languageFilename.empty()) {
			commandArgs[i++] = "-l";
			commandArgs[i++] = (char *)_languageFilename.c_str();
		}
                
                if (getAdditionalInfo) {
			commandArgs[i++] = "-i";
			commandArgs[i++] = (char *)getAdditionalInfo().c_str();
		}
                
                commandArgs[i] = NULL;
            
                //Flushes the logger file
		//Logger::logger.flush();

                if (!fork()) {
			execv(command.c_str(), commandArgs);
		}

	}

	return succeeded;
}