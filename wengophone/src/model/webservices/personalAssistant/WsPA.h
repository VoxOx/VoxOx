/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* WS TO SEND Personal Assistand Data
* @author Chris Jimenez C 
* @date 2009.06.27
*/


#ifndef OWWSPA_H
#define OWWSPA_H

#include <model/webservices/WengoWebService.h>

class UserProfile;

/**
 * Wengo SMS web service.
 *
 * Permits to send SMS.
 *
 * @author Mathieu Stute
 */
class WsPA : public WengoWebService {
public:

	
	WsPA(WengoAccount * wengoAccount, UserProfile & userProfile);

	virtual ~WsPA() {}

	
	int sendInfo(const std::string & name, const std::string & gender, int paGender);

private:

	void answerReceived(const std::string & answer, int requestId);

	UserProfile & _userProfile;

	std::string _did;//VOXOX - CJC - 2009.07.29 
};

#endif //OWWsPA_H
