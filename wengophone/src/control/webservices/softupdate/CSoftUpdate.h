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

#ifndef CSOFTUPDATE_H
#define CSOFTUPDATE_H

#include <util/Trackable.h>

#include <string>

class WsSoftUpdate;
class SoftUpdateInfo;	//VOXOX - JRT - 2009.10.05 
class CWengoPhone;
class PSoftUpdate;

/**
 *
 * @ingroup control
 * @author Tanguy Krotoff
 */
class CSoftUpdate : public Trackable 
{
public:
	CSoftUpdate( WsSoftUpdate& wsSoftUpdate, CWengoPhone& cWengoPhone);

	virtual ~CSoftUpdate();

	CWengoPhone& getCWengoPhone() const		{ return _cWengoPhone;	}

	void checkForUpdate( bool manualCheck );			//VOXOX - JRT - 2009.10.27 
	void handleUserCanceledMandatory();					//VOXOX - JRT - 2009.10.30 
	void ignoreVersion( const SoftUpdateInfo& suInfo );	//VOXOX - JRT - 2009.10.30 

private:
	void updateWengoPhoneEventHandler ( WsSoftUpdate& sender, const SoftUpdateInfo& suInfo );
	void noAvailableUpdateEventHandler( WsSoftUpdate& sender, const SoftUpdateInfo& suInfo );	//VOXOX - JRT - 2009.10.27 

	WsSoftUpdate&	_wsSoftUpdate;
	CWengoPhone&	_cWengoPhone;
	PSoftUpdate*	_pSoftUpdate;
};

#endif	//CSOFTUPDATE_H
