/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include "PhApiIMPresence.h"

#include "PhApiWrapper.h"

#include <util/Logger.h>
#include <util/OWPicture.h>
#include <util/Path.h>
#include <util/File.h>
#include <cutil/global.h>

#ifdef OS_MACOSX
	#include <CoreFoundation/CoreFoundation.h>
#endif

PhApiIMPresence::PhApiIMPresence(IMAccount & account, PhApiWrapper & phApiWrapper)
	: IMPresence(account),
	_phApiWrapper(phApiWrapper) {

	boost::signals::connection connection = _phApiWrapper.presenceStateChangedEvent +=
		boost::bind(&PhApiIMPresence::presenceStateChangedEventHandler, this, _1, _2, _3, _4);

	_phApiWrapper.myPresenceStatusEvent   += boost::bind(&PhApiIMPresence::myPresenceStatusEventHandler,   this, _1, _2, _3);
	_phApiWrapper.subscribeStatusEvent    += boost::bind(&PhApiIMPresence::subscribeStatusEventHandler,    this, _1, _2, _3);
	_phApiWrapper.contactIconChangedEvent += boost::bind(&PhApiIMPresence::contactIconChangedEventHandler, this, _1, _2, _3);
	_phApiWrapper.incomingSubscribeEvent  += boost::bind(&PhApiIMPresence::incomingSubscribeEventHandler,  this, _1, _2, _3, _4);
}

PhApiIMPresence::~PhApiIMPresence() {
}

void PhApiIMPresence::changeMyPresence(EnumPresenceState::PresenceState state, const std::string & note) {
	_phApiWrapper.changeMyPresence(state, note);
}

void PhApiIMPresence::changeMyAlias(const std::string & nickname) {
	//Hack: we use the user defined presence state of phApi to set the alias
	_phApiWrapper.changeMyPresence(EnumPresenceState::PresenceStateUserDefined, nickname);
}

void PhApiIMPresence::changeMyIcon(const OWPicture & picture) {
	_iconFilename = picture.getFilename();
	_phApiWrapper._iconFilename = _iconFilename;

	std::set<std::string>::iterator it;
	for (it = _phApiWrapper.getSubscribedContacts().begin(); 
		it != _phApiWrapper.getSubscribedContacts().end(); 
		it++) {
		_phApiWrapper.sendMyIcon(*it, _iconFilename);
	}
}

void PhApiIMPresence::subscribeToPresenceOf(const std::string & contactId) {
	_phApiWrapper.subscribeToPresenceOf(contactId);
	_phApiWrapper.getSubscribedContacts().insert(contactId);
}

void PhApiIMPresence::unsubscribeToPresenceOf(const std::string & contactId) {
	_phApiWrapper.unsubscribeToPresenceOf(contactId);
	std::set<std::string>::iterator it = _phApiWrapper.getSubscribedContacts().find(contactId);
	if (it != _phApiWrapper.getSubscribedContacts().end()) {
		_phApiWrapper.getSubscribedContacts().erase(it);
	}
}

void PhApiIMPresence::blockContact(const std::string & contactId) {
	_phApiWrapper.blockContact(contactId);
}

void PhApiIMPresence::unblockContact(const std::string & contactId) {
	_phApiWrapper.unblockContact(contactId);
}

void PhApiIMPresence::authorizeContact(const std::string & contactId, bool auhorized, const std::string message) {
}

void PhApiIMPresence::reRequestAuthorization(const std::string & contactId) {
}

void PhApiIMPresence::acceptSubscription(int sid)
{
	_phApiWrapper.acceptSubscription(sid);
}

void PhApiIMPresence::rejectSubscription(int sid)
{
	_phApiWrapper.rejectSubscription(sid);
}

void PhApiIMPresence::presenceStateChangedEventHandler(PhApiWrapper & sender, EnumPresenceState::PresenceState state, const std::string & note, const std::string & from) {
	LOG_DEBUG("contact=" + from + " presence=" + EnumPresenceState::toString(state));
	//VOXOX CHANGE CJC PHAPI SIP CONTACTS NOT SUPPORT STATUS MESSAGE
	presenceStateChangedEvent(*this, state, note,"", from);
	//presenceStateChangedEvent(*this, state, note, from);

	// Send the icon to contacts that perhaps were offline
	// FIXME: no need to send the icon to contacts that were already online
	if ((state != EnumPresenceState::PresenceStateOffline) &&
		(state != EnumPresenceState::PresenceStateUnknown) &&
		(state != EnumPresenceState::PresenceStateUnavailable)) {
		_phApiWrapper.sendMyIcon(from, _iconFilename);
	}
}

void PhApiIMPresence::myPresenceStatusEventHandler(PhApiWrapper & sender, EnumPresenceState::MyPresenceStatus status, const std::string & note) {
	LOG_DEBUG("my presence status=" + String::fromNumber(status));
	myPresenceStatusEvent(*this, status, note);
}

void PhApiIMPresence::subscribeStatusEventHandler(PhApiWrapper & sender, const std::string & contactId, IMPresence::SubscribeStatus status) {
	subscribeStatusEvent(*this, contactId, status);
}

void PhApiIMPresence::incomingSubscribeEventHandler(SipWrapper & sender, int sid, const std::string& from, const std::string& evtType)
{
	incomingSubscribeEvent(*this,sid,from,evtType);
}

const std::string PhApiIMPresence::getRessourcePath() {
	std::string resourcesPath;

// FIXME: for linux, change it to the right directory (ressource directory)
#if defined(OS_WINDOWS) || defined(OS_LINUX)
	resourcesPath = Path::getApplicationDirPath();
#elif defined(OS_MACOSX)
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	if (mainBundle) {
		CFURLRef url = CFBundleCopyResourcesDirectoryURL(mainBundle);
		char applicationPath[1024];

		if (CFURLGetFileSystemRepresentation(url, true, (UInt8 *)applicationPath, sizeof(applicationPath))) {
			resourcesPath = (std::string(applicationPath) + File::getPathSeparator());
		}

		CFRelease(url);
	}
#endif

	return resourcesPath;
}

void PhApiIMPresence::contactIconChangedEventHandler(PhApiWrapper & sender, const std::string & contactId, const std::string & filename) {
	OWPicture picture = OWPicture::pictureFromFile(getRessourcePath() + "pics/avatars/" + filename);
	contactIconChangedEvent(*this, contactId, picture);
}
