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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "Presence.h"

#include <model/WengoPhone.h>
#include <model/contactlist/ContactList.h>
#include <model/presence/MyPresenceState.h>
#include <model/presence/MyPresenceStateOffline.h>

#include <imwrapper/IMWrapperFactory.h>

#define LOGGER_COMPONENT "Presence"
#include <util/Logger.h>
#include <util/SafeDelete.h>	

Presence::Presence(IMAccount imAccount)
	: _imAccount(imAccount) {

	_imPresence = IMWrapperFactory::getFactory().createIMPresence(_imAccount);

	_imPresence->presenceStateChangedEvent	+= boost::bind(&Presence::presenceStateChangedEventHandler, this, _1, _2, _3, _4,_5);
	_imPresence->myPresenceStatusEvent		+= boost::bind(&Presence::myPresenceStatusEventHandler,		this, _1, _2, _3);
	_imPresence->subscribeStatusEvent		+= boost::bind(&Presence::subscribeStatusEventHandler,		this, _1, _2, _3);
	_imPresence->authorizationRequestEvent	+= boost::bind(&Presence::authorizationRequestEventHandler, this, _1, _2, _3);
	_imPresence->contactIconChangedEvent	+= boost::bind(&Presence::contactIconChangedEventHandler,	this, _1, _2, _3);
	_imPresence->incomingSubscribeEvent		+= boost::bind(&Presence::incomingSubscribeEventHandler,	this, _1, _2, _3, _4);

	_state = MyPresenceStateOffline::getInstance();
}

Presence::~Presence() {
	OWSAFE_DELETE(_imPresence);
}

void Presence::changeMyPresence(EnumPresenceState::PresenceState state, const std::string & note) {
	setState(state);

	_imPresence->changeMyPresence(state, note);
}

void Presence::changeMyAlias(const std::string & alias) {
	_imPresence->changeMyAlias(alias);
}

void Presence::changeMyIcon(const OWPicture & picture) {
	_imPresence->changeMyIcon(picture);
}

void Presence::subscribeToPresenceOf(const std::string & contactId) {
	_imPresence->subscribeToPresenceOf(contactId);
}

void Presence::unsubscribeToPresenceOf(const std::string & contactId) {
	_imPresence->unsubscribeToPresenceOf(contactId);
}

void Presence::blockContact(const std::string & contactId) {
	_imPresence->blockContact(contactId);
}

void Presence::unblockContact(const std::string & contactId) {
	_imPresence->unblockContact(contactId);
}

void Presence::authorizeContact(const std::string & contactId, bool authorized, const std::string message) {
	_imPresence->authorizeContact(contactId, authorized, message);
}

void Presence::reRequestAuthorization(const std::string & contactId) {//VOXOX - CJC - 2009.07.31 
	_imPresence->reRequestAuthorization(contactId);//VOXOX - CJC - 2009.07.31 
}


void Presence::acceptSubscription(int sid)
{
	_imPresence->acceptSubscription(sid);
}

void Presence::rejectSubscription(int sid)
{
	_imPresence->rejectSubscription(sid);
}

void Presence::setState(EnumPresenceState::PresenceState state) {
	switch(state) {
	case EnumPresenceState::PresenceStateUnknown:
		//Do nothing
		break;
	case EnumPresenceState::PresenceStateOnline:
		_state = _state->online();
		break;
	case EnumPresenceState::PresenceStateOffline:
		_state = _state->offline();
		break;
	case EnumPresenceState::PresenceStateAway:
		_state = _state->away();
		break;
	case EnumPresenceState::PresenceStateDoNotDisturb:
		_state = _state->doNotDisturb();
		break;
	case EnumPresenceState::PresenceStateInvisible:
		_state = _state->invisible();
		break;
	default:
		LOG_FATAL("unknown presence state=" + String::fromNumber(state));
	}
}
//VOXOX CHANGE CJC SUPPORT STATUS MESSAGE
void Presence::presenceStateChangedEventHandler(IMPresence & sender, EnumPresenceState::PresenceState state,
	const std::string & alias, const std::string & statusMessage, const std::string & from) {

	LOG_DEBUG("presence of=" + from + " changed=" + EnumPresenceState::toString(state));

	presenceStateChangedEvent(sender, state, alias, statusMessage, from);
}

void Presence::myPresenceStatusEventHandler(IMPresence & sender, EnumPresenceState::MyPresenceStatus status,
	std::string note) {
	
	myPresenceStatusEvent(sender, status, note);
}

void Presence::subscribeStatusEventHandler(IMPresence & sender, std::string contactId,
	IMPresence::SubscribeStatus status) {

	subscribeStatusEvent(sender, contactId, status);
}

void Presence::authorizationRequestEventHandler(IMPresence & sender, std::string contactId,
	std::string message) {

	authorizationRequestEvent(sender, contactId, message);
}

void Presence::incomingSubscribeEventHandler(IMPresence & sender,  int sid, const std::string& from, const std::string& evtType)
{
	incomingSubscribeEvent(sender,sid,from,evtType);
}

//void Presence::contactIconChangedEventHandler(IMPresence & sender, std::string contactId, OWPicture icon) 
void Presence::contactIconChangedEventHandler(IMPresence & sender, std::string contactId, const OWPicture& icon)	//VOXOX - JRT - 2009.08.18 
{
	contactIconChangedEvent(sender, contactId, icon);
}

void Presence::unregisterAllEvents() {
	_imPresence->presenceStateChangedEvent	-= boost::bind(&Presence::presenceStateChangedEventHandler, this, _1, _2, _3, _4, _5);
	_imPresence->myPresenceStatusEvent		-= boost::bind(&Presence::myPresenceStatusEventHandler,		this, _1, _2, _3);
	_imPresence->subscribeStatusEvent		-= boost::bind(&Presence::subscribeStatusEventHandler,		this, _1, _2, _3);
	_imPresence->authorizationRequestEvent	-= boost::bind(&Presence::authorizationRequestEventHandler, this, _1, _2, _3);
	_imPresence->contactIconChangedEvent	-= boost::bind(&Presence::contactIconChangedEventHandler,	this, _1, _2, _3);
	_imPresence->incomingSubscribeEvent		-= boost::bind(&Presence::incomingSubscribeEventHandler,	this, _1, _2, _3, _4);
}
