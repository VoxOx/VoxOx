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

#include "PhApiIMConnect.h"

#include "PhApiWrapper.h"

PhApiIMConnect::PhApiIMConnect(IMAccount & account, PhApiWrapper & phApiWrapper)
	: IMConnect(account),
	_phApiWrapper(phApiWrapper) {

	_phApiWrapper.connectedEvent		  += boost::bind(&PhApiIMConnect::connectedEventHandler,		  this, _1);
	_phApiWrapper.disconnectedEvent		  += boost::bind(&PhApiIMConnect::disconnectedEventHandler,		  this, _1, _2, _3);
	_phApiWrapper.connectionProgressEvent += boost::bind(&PhApiIMConnect::connectionProgressEventHandler, this, _1, _2, _3, _4);
}

PhApiIMConnect::~PhApiIMConnect() {
}

void PhApiIMConnect::connect() {
	_phApiWrapper.connect();
}

void PhApiIMConnect::disconnect(bool force) {
	_phApiWrapper.disconnect(force);
}

void PhApiIMConnect::connectedEventHandler(PhApiWrapper & sender) {
	_imAccount.setPresenceState( _imAccount.getInitialPresenceState() );//VOXOX - JRT - 2009.09.14 - Should always be 'online'?
	
	connectedEvent(*this);
}

void PhApiIMConnect::disconnectedEventHandler(PhApiWrapper & sender, bool connectionError, const std::string & reason) {
	disconnectedEvent(*this, connectionError, reason, false );//VOXOX - JRT - 2009.07.13 - TODO: Do we need 'fatal' parameter for this?
}

void PhApiIMConnect::connectionProgressEventHandler(PhApiWrapper & sender, int currentStep, int totalSteps, const std::string & infoMessage) {
	connectionProgressEvent(*this, currentStep, totalSteps, infoMessage);
}
