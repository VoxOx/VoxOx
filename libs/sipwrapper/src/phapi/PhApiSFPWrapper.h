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

#ifndef OWPHAPISFPWRAPPER_H
#define OWPHAPISFPWRAPPER_H

#include <imwrapper/Account.h>
#include <imwrapper/IMContact.h>

#include <util/Singleton.h>
#include <util/File.h>

/**
 * Class that wraps the calls to the SFP plugin
 *
 * @author Nicolas Couturier
 */
class PhApiSFPWrapper : public Singleton<PhApiSFPWrapper> {

	friend class Singleton<PhApiSFPWrapper>;

public :

	/**
	*
	*
	* @return	the call id if the invitation to transfer could be sent
	*/
	int sendFile(int vlineID, std::string fullIdentity, std::string contactUri, std::string filename, std::string shortFilename, std::string fileType, unsigned int fileSize);

	int receiveFile(int callId, std::string filename);

	int cancelTransfer(int callId);

	int pauseTransfer(int callId);

	int resumeTransfer(int callId);

private :

	PhApiSFPWrapper();

	~PhApiSFPWrapper();

	void setBasePort(const unsigned int basePort);

};

#endif	//OWPHAPISFPWRAPPER_H

