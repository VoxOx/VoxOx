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

#ifndef OWCPHONELINE_H
#define OWCPHONELINE_H

#include <sipwrapper/EnumPhoneLineState.h>
#include <util/Trackable.h>

#include <string>
#include <map>

class IPhoneLine;
class PPhoneLine;
class PhoneCall;
class CWengoPhone;
class CPhoneCall;

/**
 *
 * @ingroup control
 * @author Tanguy Krotoff
 */
class CPhoneLine : public Trackable {
public:

	CPhoneLine(IPhoneLine & phoneLine, CWengoPhone & cWengoPhone);

	~CPhoneLine();

	PPhoneLine * getPresentation() const {
		return _pPhoneLine;
	}

	CWengoPhone & getCWengoPhone() const {
		return _cWengoPhone;
	}

	/**
	 * @see IPhoneLine::hasPendingcalls
	 */
	bool hasPendingCalls() const;

	/**
	 * @see IPhoneLine::closePendingCalls
	 */
	void closePendingCalls();

	int makeCall(const std::string & phoneNumber);

private:

	void stateChangedEventHandler(IPhoneLine & sender, EnumPhoneLineState::PhoneLineState state);

	void phoneCallCreatedEventHandler(IPhoneLine & sender, PhoneCall & phoneCall);

	void phoneCallClosedEventHandler(IPhoneLine & sender, PhoneCall & phoneCall);

	IPhoneLine & _phoneLine;

	PPhoneLine * _pPhoneLine;

	CWengoPhone & _cWengoPhone;

	typedef std::map<PhoneCall *, CPhoneCall *> CPhoneCallMap;

	CPhoneCallMap _cPhoneCallMap;
};

#endif	//OWCPHONELINE_H
