/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* Custom Tooltip Manager
* @author Chris Jimenez C 
* @date 2009.05.11
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtContactTooltipManager.h"

#include "QtContactTooltip.h"
#include "QtContact.h"

#include <imwrapper/QtEnumIMProtocol.h>	

#include <qtutil/SafeConnect.h>
#include <control/CWengoPhone.h>
#include <presentation/qt/QtWengoPhone.h>
#include <QtGui/QtGui>


QtContactTooltipManager * QtContactTooltipManager::instance = NULL;



QtContactTooltipManager::QtContactTooltipManager(CWengoPhone & cWengoPhone)
	:QObject(),
	_cWengoPhone(cWengoPhone) {
	_currentTooltip = NULL;
	_currentTooltipKey = "";

}

QtContactTooltipManager::~QtContactTooltipManager() {
	closeCurrentTooltip();
	
}


QtContactTooltipManager *QtContactTooltipManager::getInstance(CWengoPhone & cWengoPhone) {

	if (!instance) {
		instance = new QtContactTooltipManager(cWengoPhone);
	}

	return instance;
}
void QtContactTooltipManager::closeCurrentTooltip(){
	if(isTooltipActive()){
		_currentTooltip->close();
		delete _currentTooltip;
		_currentTooltip = NULL;
		_currentTooltipKey = "";
	}
}

bool QtContactTooltipManager::isTooltipActive(){//VOXOX - CJC - 2009.06.26 
	if(_currentTooltip!=NULL){
		if(_currentTooltipKey!=""){
			return true;
		}else{
			return false;
		}
	}else{
		return false;
	}
}

void QtContactTooltipManager::openTooltip(QtContact * contact,QWidget * parent)
{
	if(contact->getKey() != _currentTooltipKey)	//VOXOX - JRT - 2009.08.10 - Multiple groups
	{
		closeCurrentTooltip();
		QtContactTooltip * tooltip = new QtContactTooltip(contact->getId(), parent,_cWengoPhone);	//VOXOX - JRT - 2009.08.10 
		tooltip->setDisplayName(contact->getDisplayName());
		tooltip->setStatusMessage(contact->getStatusMessage());
		//tooltip->setStatusPixmap(contact->getStatusPixmap());//VOXOX CHANGE by Rolando - 2009.10.23 
		tooltip->setStatusPixmapPath(contact->getNormalStatusPixmapPath(),contact->getHoverStatusPixmapPath());//VOXOX CHANGE by Rolando - 2009.10.23 
		tooltip->setAvatarPixmap(contact->getAvatarPixmap());
		tooltip->setProtocol(contact->getContact()->getPreferredIMContact()->getQtProtocol());
		SAFE_CONNECT(tooltip, SIGNAL(closeMe()), SLOT(closeCurrentTooltip()));//VOXOX - CJC - 2009.06.26 
		_currentTooltip = tooltip;
		_currentTooltipKey = contact->getKey();	//VOXOX - JRT - 2009.08.10 
		tooltip->showTooltip();
	}

}


