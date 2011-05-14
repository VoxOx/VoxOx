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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtIMContactManagerInterface.h"

#include <control/profile/CUserProfile.h>

#include <model/contactlist/ContactProfile.h>

#include <QtGui/QtGui>

QtIMContactManagerInterface::QtIMContactManagerInterface(ContactProfile & contactProfile,
	CUserProfile & cUserProfile, QWidget * parent)
	: QObject(parent),
	_contactProfile(contactProfile),
	_cUserProfile(cUserProfile) {

	_imContactManagerWidget = new QWidget(NULL);
}

QtIMContactManagerInterface::~QtIMContactManagerInterface() {
}

QWidget * QtIMContactManagerInterface::getWidget() const {
	return _imContactManagerWidget;
}

QtIMContactManagerInterface::contactManagerMode QtIMContactManagerInterface::getContactManagerMode() {
	return QtIMContactManagerInterface::contactManagerModeUndefined;
}

