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

#include "PhApiFactory.h"

#include "PhApiWrapper.h"
#include "PhApiIMConnect.h"
#include "PhApiIMChat.h"
#include "PhApiIMPresence.h"

PhApiFactory::PhApiFactory() {
	_phApiWrapperInstance = PhApiWrapper::getInstance();
}

void PhApiFactory::init(const std::string & pathToProfile) {
}

void PhApiFactory::terminate() {
}

SipWrapper * PhApiFactory::createSipWrapper() {
	return _phApiWrapperInstance;
}

IMConnect * PhApiFactory::createIMConnect(IMAccount & account) {
	return new PhApiIMConnect(account, *_phApiWrapperInstance);
}

IMChat * PhApiFactory::createIMChat(IMAccount & account) {
	return  new PhApiIMChat(account, *_phApiWrapperInstance);
}

IMPresence * PhApiFactory::createIMPresence(IMAccount & account) {
	return new PhApiIMPresence(account, *_phApiWrapperInstance);
}

IMContactList * PhApiFactory::createIMContactList(IMAccount & account) {
	return NULL;
}
