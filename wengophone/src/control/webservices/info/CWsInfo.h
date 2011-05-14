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

#ifndef CWSINFO_H
#define CWSINFO_H

#include <model/webservices/info/WsInfo.h>

#include <util/Trackable.h>

#include <string>

class CWengoPhone;

/**
 *
 * @ingroup control
 * @author Mathieu Stute
 */
class CWsInfo : public Trackable {
public:

	/**
	 * @see WsInfo::wsInfoWengosEvent
	 */
	Event<void (WsInfo & sender, int id, WsInfo::WsInfoStatus status,std::string wengos)> wsInfoWengosEvent;

	/**
	 * @see WsInfo::wsInfoSmsCountEvent
	 */
	Event<void (WsInfo & sender, int id, WsInfo::WsInfoStatus status, int sms)> wsInfoSmsCountEvent;

	/**
	 * @see WsInfo::wsInfoActiveMailEvent
	 */
	Event<void (WsInfo & sender, int id, WsInfo::WsInfoStatus status, int activeMail)> wsInfoActiveMailEvent;

	/**
	 * @see WsWengoInfo::wsInfoVoiceMailEvent
	 */
	Event<void (WsInfo & sender, int id, WsInfo::WsInfoStatus status, int voicemail)> wsInfoVoiceMailEvent;

	/**
	 * @see WsWengoInfo::wsInfoLandlineNumberEvent
	 */
	Event<void (WsInfo & sender, int id, WsInfo::WsInfoStatus status, std::string number)> wsInfoLandlineNumberEvent;

	/**
	 * @see WsWengoInfo::wsCallForwardInfoEvent
	 */
	Event<void (WsInfo & sender, int id, WsInfo::WsInfoStatus status,
		WsInfo::WsInfoCallForwardMode mode, bool voicemail, std::string dest1,
		std::string dest2, std::string dest3)> wsCallForwardInfoEvent;

	/**
	 * default constructor.
	 *
	 * @param wsInfo reference to WsWengoInfo.
	 * @param cWengoPhone reference to CWengoPhone.
	 */
	CWsInfo(WsInfo & wsInfo, CWengoPhone & cWengoPhone);

	/**
	 * default destructor.
	 */
	~CWsInfo();

	/**
	 * Get a reference to CWengoPhone.
	 *
	 * @return _cWengoPhone.
	 */
	CWengoPhone & getCWengoPhone() const {
		return _cWengoPhone;
	}

	/**
	 * wengo's request.
	 */
	void getWengosCount();

	/**
	 * sms count request.
	 */
	void getSmsCount();

	/**
	 * active mail request.
	 */
	void getActiveMail();

	/**
	 * voice mail count request.
	 */
	void getUnreadVoiceMail();

	/**
	 * call forward request.
	 */
	void getCallForwardInfo();

	/**
	 * Landline number request.
	 */
	void getLandlineNumber();

private:

	WsInfo & _wsInfo;

	CWengoPhone & _cWengoPhone;
};

#endif	//CWSINFO_H
