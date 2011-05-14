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

#ifndef OWIRECEIVEFILESESSION_H
#define OWIRECEIVEFILESESSION_H

#include "IFileSession.h"

#include <imwrapper/IMContact.h>

#include <string>

/**
 *
 *
 * @author Philippe Bernery
 */
class IReceiveFileSession : public IFileSession {
public:

	IReceiveFileSession();

	IReceiveFileSession(const IReceiveFileSession & iReceiveFileSession);

	virtual IReceiveFileSession * clone() const = 0;

	virtual ~IReceiveFileSession();

	/**
	 * Gets the IMContact who sends the File.
	 */
	IMContact getIMContact() const { return _imContact; }

	/**
	 * Sets the path for saving the File.
	 */
	void setFilePath(const std::string & path) { _filePath = path; }

	/**
	 * Gets the path to the File to be saved.
	 */
	std::string getFilePath() const { return _filePath; }

	/**
	 * Gets the name of the file to receive.
	 */
	std::string getFileName() const { return _fileName; }

	/**
	 * Gets the size of the file to receive.
	 */
	unsigned getFileSize() const { return _fileSize; }

protected:

	/** Path to save place of the file. */
	std::string _filePath;

	/** Name of the file to receive. */
	std::string _fileName;

	/** Size of the file to receive. */
	unsigned _fileSize;

	/** Sender of File. */
	IMContact _imContact;

};

#endif //OWIRECEIVEFILESESSION_H
