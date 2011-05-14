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
#include "CWenboxPlugin.h"

#include <presentation/PFactory.h>
#include <presentation/PWenboxPlugin.h>

#include <model/wenbox/WenboxPlugin.h>

#include <thread/ThreadEvent.h>

CWenboxPlugin::CWenboxPlugin(WenboxPlugin & wenboxPlugin, CWengoPhone & cWengoPhone)
	: _wenboxPlugin(wenboxPlugin),
	_cWengoPhone(cWengoPhone) {

	_pWenboxPlugin = NULL;
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CWenboxPlugin::initPresentationThreadSafe, this));
	PFactory::postEvent(event);
}

CWenboxPlugin::~CWenboxPlugin() {
//JRT-XXX	delete _pWenboxPlugin;	//VOXOX - JRT - 2009.04.13 - Uncommented to fix memory leak.
	_pWenboxPlugin = NULL;
}

Presentation * CWenboxPlugin::getPresentation() const {
	return _pWenboxPlugin;
}

CWengoPhone & CWenboxPlugin::getCWengoPhone() const {
	return _cWengoPhone;
}

void CWenboxPlugin::initPresentationThreadSafe() {
	_pWenboxPlugin = PFactory::getFactory().createPresentationWenboxPlugin(*this);

	_wenboxPlugin.phoneNumberBufferUpdatedEvent += boost::bind(&CWenboxPlugin::phoneNumberBufferUpdatedEventHandler, this, _1, _2);
}

void CWenboxPlugin::phoneNumberBufferUpdatedEventHandler(WenboxPlugin & sender, const std::string & phoneNumberBuffer) {
	typedef ThreadEvent1<void (std::string), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CWenboxPlugin::phoneNumberBufferUpdatedEventHandlerThreadSafe, this, _1), phoneNumberBuffer);
	PFactory::postEvent(event);
}

void CWenboxPlugin::phoneNumberBufferUpdatedEventHandlerThreadSafe(std::string phoneNumberBuffer) {
	_pWenboxPlugin->phoneNumberBufferUpdatedEvent(phoneNumberBuffer);
}
