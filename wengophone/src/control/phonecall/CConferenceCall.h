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

#ifndef OWCCONFERENCECALL_H
#define OWCCONFERENCECALL_H

#include <sipwrapper/EnumConferenceCallState.h>

#include <control/Control.h>

class CWengoPhone;
class CPhoneCall;
class PhoneCall;
class ConferenceCall;
class Presentation;
class PConferenceCall;

/**
 * Control layer for a conference call.
 *
 * @see ConferenceCall
 * @ingroup control
 * @author Tanguy Krotoff
 */
class CConferenceCall : public Control {
public:

	CConferenceCall(ConferenceCall & conferenceCall, CWengoPhone & cWengoPhone);

	~CConferenceCall();

	Presentation * getPresentation() const;

	CWengoPhone & getCWengoPhone() const;

	/**
	 * @see ConferenceCall::addPhoneCall()
	 */
	void addPhoneCall(CPhoneCall & cPhoneCall);

	/**
	 * @see ConferenceCall::removePhoneCall()
	 */
	void removePhoneCall(CPhoneCall & cPhoneCall);

	/**
	 * @see ConferenceCall::addPhoneNumber()
	 */
	void addPhoneNumber(const std::string & phoneNumber);

	/**
	 * @see ConferenceCall::removePhoneNumber()
	 */
	void removePhoneNumber(const std::string & phoneNumber);

private:

	void initPresentationThreadSafe();

	void stateChangedEventHandler(ConferenceCall & sender, EnumConferenceCallState::ConferenceCallState state);

	void phoneCallAddedEventHandler(ConferenceCall & sender, PhoneCall & phoneCall);

	void phoneCallRemovedEventHandler(ConferenceCall & sender, PhoneCall & phoneCall);

	ConferenceCall & _conferenceCall;

	PConferenceCall * _pConferenceCall;

	CWengoPhone & _cWengoPhone;
};

#endif	//OWCCONFERENCECALL_H
