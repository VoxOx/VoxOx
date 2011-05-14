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

#ifndef OWXPCOMFACTORY_H
#define OWXPCOMFACTORY_H

#include "XPCOMWengoPhone.h"
#include "XPCOMUserProfile.h"
#include "XPCOMPhoneLine.h"
#include "XPCOMPhoneCall.h"
#include "XPCOMSms.h"

#include <presentation/PFactory.h>
#include <control/CWengoPhone.h>
#include <control/phoneline/CPhoneLine.h>
#include <control/phonecall/CPhoneCall.h>

PFactory * PFactory::_factory = 0;

class XPCOMFactory : public PFactory {
public:

	XPCOMFactory() {
	}

	~XPCOMFactory() {
	}

	PWengoPhone * createPresentationWengoPhone(CWengoPhone & cWengoPhone) {
		static XPCOMWengoPhone xpcomWengoPhone(&cWengoPhone);
		return &xpcomWengoPhone;
	}

	PUserProfile * createPresentationUserProfile(CUserProfile & cUserProfile) {
		XPCOMUserProfile * xpcomUserProfile = new XPCOMUserProfile(cUserProfile);
		return xpcomUserProfile;
	}

	PUserProfileHandler * createPresentationUserProfileHandler(CUserProfileHandler & cUserProfileHandler) {
		return NULL;
	}

	PPhoneLine * createPresentationPhoneLine(CPhoneLine & cPhoneLine) {
		XPCOMPhoneLine * xpcomPhoneLine = new XPCOMPhoneLine(cPhoneLine);
		return xpcomPhoneLine;
	}

	PPhoneCall * createPresentationPhoneCall(CPhoneCall & cPhoneCall) {
		XPCOMPhoneCall * xpcomPhoneCall = new XPCOMPhoneCall(cPhoneCall);
		return xpcomPhoneCall;
	}

	PContactList * createPresentationContactList(CContactList & cContactList) {
		return NULL;
	}

	PWenboxPlugin * createPresentationWenboxPlugin(CWenboxPlugin & cWenboxPlugin) {
		return NULL;
	}

	PChatHandler * createPresentationChatHandler(CChatHandler & cChatHandler) {
		return NULL;
	}

	PSms * createPresentationSms(CSms & cSms) {
		XPCOMSms * xpcomSms = new XPCOMSms(cSms);
		return xpcomSms;
	}

	PSoftUpdate * createPresentationSoftUpdate(CSoftUpdate & cSoftUpdate) {
		return NULL;
	}

	PHistory * createPresentationHistory(CHistory & cHistory) {
		return NULL;
	}

	PSubscribe * createPresentationSubscribe(CSubscribe & cSubscribe) {
		return NULL;
	}

	PWsDirectory * createPresentationWsDirectory(CWsDirectory & cWsDirectory) {
		return NULL;
	}

	int exec() {
		return true;
	}

	void processEvents() { }

	void reset() { }

private:
};

#endif	//OWXPCOMFACTORY_H
