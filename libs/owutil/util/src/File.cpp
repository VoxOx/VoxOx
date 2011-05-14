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
#include <util/File.h>
#include <util/String.h>

#define LOGGER_COMPONENT "File"
#include <util/Logger.h>

#include <cutil/global.h>

#include <string>
#include <iostream>
#include <memory>
using namespace std;

#include <stdio.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(OS_WINDOWS)
#include <sys/param.h>
#endif

#if defined(OS_WINDOWS)
	#ifndef S_ISDIR
		#define S_ISDIR(x) ((x) & _S_IFDIR)
	#endif
	#ifndef S_ISREG
		#define S_ISREG(x) ((x) & _S_IFREG)
	#endif
#endif

#ifdef OS_WINDOWS
	#include <windows.h>
#endif
#ifdef CC_MSVC
	#include <direct.h>
#endif

File::File(const std::string & filename, Encoding encoding)
	: _filename(filename),
	_encoding(encoding) {
}

File::File(const File & file)
	: NonCopyable(),
	_filename(file._filename),
	_encoding(file._encoding) {
}

File & File::operator=(const File & file) {
	_filename = file._filename;
	_encoding = file._encoding;
	return *this;
}

std::string File::getExtension() const {
	/*int posLastElm = _filename.find_last_of(getPathSeparator());

	if ((posLastElm == -1) || (posLastElm == _filename.length())) {
		return String::null;
	}

	string last = _filename.substr(++posLastElm, _filename.length() - posLastElm);
	int posExt = last.find_last_of('.');

	if ((posExt == -1) || (posExt == last.length())) {
		return String::null;
	} else {
		return last.substr(++posExt, last.length() - posExt);
	}*/
	String path = _filename;

	string::size_type pos = path.rfind('.');

	if (pos == string::npos) {
		return String::null;
	} else {
		path = path.substr(pos+1);
		return path;
	}
}

bool File::isDirectory(const std::string & filename) {
	struct stat statinfo;
	bool result = false;
	std::string myFilename = filename;

#ifdef OS_WINDOWS
	if (myFilename.substr(myFilename.size() - 1, 1) == getPathSeparator()) {
		myFilename = myFilename.substr(0, myFilename.size() - 1);
	}
#endif

	if (stat(myFilename.c_str(), &statinfo) == 0) {
		if (S_ISDIR(statinfo.st_mode)) {
			result = true;
		}
	}

	return result;
}

bool File::remove() {
	bool result = false;

	if (isDirectory(_filename)) {
		//Removing all files in dir recursively
		StringList dirList = getDirectoryList();
		for (StringList::const_iterator it = dirList.begin();
			it != dirList.end(); ++it) {
			File subDir(_filename + getPathSeparator() + (*it));
			subDir.remove();
		}

		StringList fileList = getFileList();
		for (StringList::const_iterator it = fileList.begin();
			it != fileList.end(); ++it) {
			File subFile(_filename + getPathSeparator() + (*it));
			subFile.remove();
		}
	}

	if (isDirectory(_filename)) {
		if (!::rmdir(_filename.c_str())) {
			result = true;
		}
	} else {
		if (!::remove(_filename.c_str())) {
			result = true;
		}
	}

	return result;
}

bool File::move(const std::string & newName, bool overwrite) {
	if (exists(newName) && overwrite) {
		File file(newName);
		file.remove();
	}

	if (!rename(_filename.c_str(), newName.c_str())) {
		return true;
	} else {
		return false;
	}
}

bool File::copy(const std::string & path) {
	bool result = false;

	if (!exists(path)) {
		createPath(path);
	}

	if (isDirectory(_filename)) {
		//Creating all directories recursively
		StringList dirList = getDirectoryList();
		for (StringList::const_iterator it = dirList.begin();
			it != dirList.end(); ++it) {
			File subDir(_filename + getPathSeparator() + (*it));
			result = subDir.copy(path + getPathSeparator() + (*it));
		}

		StringList fileList = getFileList();
		for (StringList::const_iterator it = fileList.begin();
			it != fileList.end(); ++it) {
			File subFile(_filename + getPathSeparator() + (*it));
			result = subFile.copy(path + getPathSeparator() + (*it));
		}
	} else {
		result = copyFile(path, _filename);
	}

	return result;
}

bool File::copyFile(const std::string & dst, const std::string & src) {
	createPath(dst);

	std::string destination;
	if (isDirectory(dst)) {
		File srcFile(src);
		destination = dst + srcFile.getFileName();
	} else {
		destination = dst;
	}

	ifstream ifile(src.c_str(), ios::binary);
	ofstream ofile(destination.c_str(), ios::binary);

	if (ifile.fail()) {
		LOG_ERROR(src + " does not exist");
		return false;
	}

	if (ofile.fail()) {
		LOG_ERROR("cannot open " + dst + " for writing");
		return false;
	}

	static const unsigned BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];
	while (!ifile.eof()) {
		ifile.read(buffer, BUFFER_SIZE);
		if (ifile.bad()) {
			LOG_ERROR("error while reading data");
			return false;
		}
		ofile.write(buffer, ifile.gcount());
	}

	ifile.close();
	ofile.close();

	return true;
}

std::string File::getPath() const {
	String path = _filename;
	path = convertPathSeparators(path);

	string::size_type pos = path.rfind(getPathSeparator());

	if (pos == string::npos || pos == path.length() - 1) {
		return path;
	} else {
		path = path.substr(0, pos);
		return path;
	}
}

std::string File::getFullPath() const {
	return _filename;
}

std::string File::getFileName() const {
	String path = _filename;

	/*
	 * Under windows Qt gives / as pasth separator which is not
	 * homogeneous with getPathSeparator() that returns \. Therefore
	 * we have to convert / into \ under Windows.
	 * We can't call convertPathSeparators under Linux, because with
	 * a filename like /home/user/tes\"t.txt, it would turn it into
	 * /home/user/tes/t.txt which would result in incorrect t.txt filename.
	 */
#ifdef OS_WINDOWS
	path = convertPathSeparators(path);
#endif /* OS_WINDOWS */

	string::size_type pos = path.rfind(getPathSeparator());

	if (pos == string::npos) {
		return path;
	} else {
		path = path.substr(pos+1);
		return path;
	}
}

StringList File::getDirectoryList() const {
	//Same code as File::getFileList()

	StringList dirList;

	DIR * dp = opendir(_filename.c_str());
	if (dp) {
		struct dirent * ep = NULL;
		while ((ep = readdir(dp))) {
			String dir(ep->d_name);

			if (dir == "." || dir == "..") {
				continue;
			}

			std::string absPath = _filename + getPathSeparator() + dir;
			if (isDirectory(absPath)) {
				dirList += dir;
			}
		}

		closedir(dp);
	}

	return dirList;
}

StringList File::getFileList() const {
	//Same code as File::getDirectoryList()

	StringList fileList;

	DIR * dp = opendir(_filename.c_str());
	if (dp) {
		struct dirent * ep = NULL;
		while ((ep = readdir(dp))) {
			String file(ep->d_name);

			if ((file == ".") || (file == "..")) {
				continue;
			}

			std::string absPath = _filename + file;
			if (!isDirectory(absPath)) {
				fileList += file;
			}
		}
	}

	closedir(dp);

	return fileList;
}

unsigned File::getSize() const {
	struct stat sb;

#ifdef OS_WINDOWS
	wchar_t filename4win[4096];
	struct _stat sb4win;

	if (_encoding == EncodingUTF8) {
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (LPCSTR) _filename.c_str(), -1, (LPWSTR) &filename4win, sizeof(filename4win));
		if (_wstat(filename4win, &sb4win) == 0) {
			return sb4win.st_size;
		}
		return 0;
	}
#endif

	if (_encoding == EncodingDefault || _encoding == EncodingUTF8) {
		if (stat(_filename.c_str(), &sb) == 0) {
			return sb.st_size;
		}
		return 0;
	}

	return 0;
}

std::string File::convertPathSeparators(const std::string & path) {
	String tmp = path;
	tmp.replace("\\", getPathSeparator());
	tmp.replace("/", getPathSeparator());
	return tmp;
}

std::string File::convertToUnixPathSeparators(const std::string & path) {
	String tmp = path;
	tmp.replace("\\", "/");
	return tmp;
}

std::string File::getPathSeparator() {
#ifdef OS_WINDOWS
	static const std::string PATH_SEPARATOR = "\\";
	return PATH_SEPARATOR;
#else
	static const std::string PATH_SEPARATOR = "/";
	return PATH_SEPARATOR;
#endif
}

void File::createPath(const std::string & path) {
	string::size_type index = path.find(File::getPathSeparator(), 0);
	while (index != string::npos) {
#if defined CC_MSVC || defined CC_MINGW
		mkdir(path.substr(0, index).c_str());
#else
		mkdir(path.substr(0, index).c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
#endif
		index = path.find(File::getPathSeparator(), index + 1);
	}
}


#ifdef OS_WINDOWS
File File::createTemporaryFile() {
	return File(tempnam(NULL, NULL));
}
#else
File File::createTemporaryFile() {
	char * tmpDir = getenv("TMPDIR");

	char tempFileName[MAXPATHLEN];
	int fd;
	if (tmpDir) {
		strcpy(tempFileName, tmpDir);
		strcat(tempFileName, "/XXXXXX");

		fd = mkstemp(tempFileName);
		if (fd != -1) {
			close(fd);
			return File(tempFileName);
		}
	}

	strcpy(tempFileName, "/tmp/XXXXXX");
	fd = mkstemp(tempFileName);
	if (fd != -1) {
		close(fd);
		return File(tempFileName);
	}

	LOG_FATAL("Could not create temporary file");
	return File("neverreached");
}
#endif

bool File::exists(const std::string & path) {

	if (path.empty()) {
		return false;
	}

	std::string myPath = path;
	//Checking for ending PathSeparator existance.
	//if the path contains a trailing PathSepartor, 'exists' will not work
	//under Windows
	std::string pathSeparator = File::getPathSeparator();
	if (myPath.substr(myPath.size() - pathSeparator.size()) == pathSeparator) {
		myPath = myPath.substr(0, myPath.size() - pathSeparator.size());
	}

	struct stat statinfo;

	if (stat(myPath.c_str(), &statinfo) == 0) {
		return true;
	} else {
		return false;
	}
}

FileReader::FileReader(const std::string & filename)
	: File(filename) {
}

FileReader::FileReader(const File & file)
	: File(file) {
}

FileReader::FileReader(const FileReader & fileReader)
	: File(fileReader),
	IFile() {
}

FileReader::~FileReader() {
}

bool FileReader::open() {
	LOG_DEBUG("loading " + _filename);
	_file.open(_filename.c_str(), ios::binary);
	return isOpen();
}

bool FileReader::isOpen() {
	return _file.is_open();
}

std::string FileReader::read() {
	static const unsigned int BUFFER_SIZE = 2000;

	if (!isOpen()) {
		LOG_FATAL("you must check the file is open");
	}

	std::string data;
	char tmp[BUFFER_SIZE];
	while (!_file.eof()) {
		_file.read(tmp, BUFFER_SIZE);
		data.append(tmp, _file.gcount());
	}

	return data;
}

void FileReader::close() {
	_file.close();
}


FileWriter::FileWriter(const std::string & filename, bool binaryMode)
	: File(filename) {
	_binaryMode = binaryMode;
	_appendMode = false;
	_fileOpen = false;
}

FileWriter::FileWriter(const File & file, bool binaryMode)
	: File(file),
	IFile() {
	_binaryMode = binaryMode;
	_appendMode = false;
	_fileOpen = false;
}

FileWriter::FileWriter(const FileWriter & fileWriter, bool binaryMode)
	: File(fileWriter),
	IFile() {
	_binaryMode = binaryMode;
	_appendMode = false;
	_fileOpen = false;
}

FileWriter::~FileWriter() {
}

bool FileWriter::open() {
	LOG_DEBUG("saving to " + _filename);
	ios::openmode mode;

	if (_appendMode) {
		mode = ios::app;
	} else {
		mode = ios::out;
	}

	if (_binaryMode) {
		mode |= ios::binary;
	}

	_file.open(_filename.c_str(), mode);
	_fileOpen = true;
	return isOpen();
}

bool FileWriter::isOpen() {
	return _fileOpen;
}

void FileWriter::write(const std::string & data) {
	//See http://www.cplusplus.com/doc/tutorial/files.html

	if (!isOpen()) {
		open();
	}

	if (!data.empty()) {
		_file.write(data.c_str(), data.size());
	}
}

void FileWriter::close() {
	_file.close();
}

bool FileWriter::setAppendMode(bool appendMode) {
	if (!_fileOpen) {
		_appendMode = appendMode;
	}

	return _appendMode;
}
