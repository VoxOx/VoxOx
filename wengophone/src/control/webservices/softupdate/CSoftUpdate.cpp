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
#include "CSoftUpdate.h"

#include <model/webservices/softupdate/WsSoftUpdate.h>
#include <presentation/PFactory.h>
#include <presentation/PSoftUpdate.h>
#include <control/CWengoPhone.h>					//VOXOX - JRT - 2009.10.30 
#include <control/profile/CUserProfileHandler.h>	//VOXOX - JRT - 2009.10.30 

#include <util/SafeDelete.h>

CSoftUpdate::CSoftUpdate(WsSoftUpdate & wsSoftUpdate, CWengoPhone & cWengoPhone)
	: _wsSoftUpdate(wsSoftUpdate),
	_cWengoPhone(cWengoPhone) 
{
	_pSoftUpdate = PFactory::getFactory().createPresentationSoftUpdate(*this);

	_wsSoftUpdate.updateWengoPhoneEvent  += boost::bind(&CSoftUpdate::updateWengoPhoneEventHandler,  this, _1, _2);	//VOXOX - JRT - 2009.10.05 
	_wsSoftUpdate.noAvailableUpdateEvent += boost::bind(&CSoftUpdate::noAvailableUpdateEventHandler, this, _1, _2);	//VOXOX - JRT - 2009.10.05 
}

CSoftUpdate::~CSoftUpdate() 
{
	OWSAFE_DELETE(_pSoftUpdate);
}

void CSoftUpdate::updateWengoPhoneEventHandler( WsSoftUpdate & sender, const SoftUpdateInfo& suInfo ) 
{
	_pSoftUpdate->updateWengoPhoneEvent( suInfo );
}

void CSoftUpdate::noAvailableUpdateEventHandler( WsSoftUpdate & sender, const SoftUpdateInfo& suInfo ) 
{
	_pSoftUpdate->noAvailableUpdateEvent( suInfo );
}

void CSoftUpdate::checkForUpdate( bool manualCheck ) 
{
	_wsSoftUpdate.checkForUpdate( manualCheck );
}

void CSoftUpdate::handleUserCanceledMandatory()
{
	_cWengoPhone.handleUserCanceledMandatory();
}

void CSoftUpdate::ignoreVersion( const SoftUpdateInfo& suInfo )
{
	_cWengoPhone.getCUserProfileHandler().ignoreVersion( suInfo );
}
