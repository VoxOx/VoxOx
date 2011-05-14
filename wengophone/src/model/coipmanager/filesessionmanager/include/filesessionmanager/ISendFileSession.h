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

#ifndef OWISENDFILESESSION_H
#define OWISENDFILESESSION_H

#include "IFileSession.h"

#include <imwrapper/IMContactSet.h>

#include <util/File.h>

#include <vector>

/**
 *
 *
 * @author Philippe Bernery
 */
class ISendFileSession : public IFileSession {
public:

	ISendFileSession();

	ISendFileSession(const ISendFileSession & iSendFileSession);

	virtual ISendFileSession * clone() const = 0;

	virtual ~ISendFileSession();

	/**
	 * Sets the IMContactSet.
	 */
	void setIMContactSet(const IMContactSet & imContactSet) { _imContactSet = imContactSet; }

	/**
	 * Gets the IMContactSet.
	 */
	IMContactSet getIMContactSet() const { return _imContactSet; }

	/**
	 * Sets the File List.
	 */
	void setFileList(const std::vector<File> & fileList) { _fileVector = fileList; }

	/**
	 * Gets the File list.
	 */
	std::vector<File> getFileList() const { return _fileVector; }

protected:

	/** List of Contact to send the files to. */
	IMContactSet _imContactSet;

	/** List of File to send. */
	std::vector<File> _fileVector;

};

#endif //OWISENDFILESESSION_H
