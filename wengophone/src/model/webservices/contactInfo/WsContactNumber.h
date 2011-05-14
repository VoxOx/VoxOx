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

#ifndef OWWSCONTACTNUMBER_H
#define OWWSCONTACTNUMBER_H

#include <model/webservices/WengoWebService.h>

/**
 * WengoPhone update web service.
 *
 * Checks for the availability of WengoPhone updates.
 *
 * @author Tanguy Krotoff
 */
class WsContactNumber : public WengoWebService {
public:

	/**
	 * Default constructor.
	 *
	 * @param wengoAccount the WengoAccount used for web services
	 */
	WsContactNumber();

	virtual ~WsContactNumber() {}

	void setContactId(const std::string pContactId);

	std::string getContactId(){ return _contactId;}
	/**
	 * Event WengoPhone should be updated.
	 *
	 * @param sender this class
	 * @param Contact Numer
	 */
	Event<void (WsContactNumber & sender,
			const std::string & contactNumber,const std::string & contactId)> contactNumberEvent;
	

	/**
	 * Checks if a WengoPhone update is available.
	 */
	void execute();

private:

	/**
	 * @see WengoWebService
	 */
	void answerReceived(const std::string & answer, int id);


	std::string _contactId;
};

#endif //OWWSSOFTUPDATE_H
