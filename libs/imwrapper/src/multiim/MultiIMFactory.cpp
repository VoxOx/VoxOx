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

#include "multiim/MultiIMFactory.h"

#include <imwrapper/IMAccount.h>
#include <imwrapper/EnumIMProtocol.h>
#include <PhApiFactory.h>
#include <PurpleIMFactory.h>

MultiIMFactory::MultiIMFactory(PhApiFactory & phApiFactory, PurpleIMFactory & purpleIMFactory)
	: _phApiFactory(phApiFactory), _purpleIMFactory(purpleIMFactory) 
{
}

void MultiIMFactory::init(const std::string & pathToProfile) 
{
	_phApiFactory.init(pathToProfile);
	_purpleIMFactory.init(pathToProfile);
}

void MultiIMFactory::terminate() 
{
	_phApiFactory.terminate();
	_purpleIMFactory.terminate();
}

IMConnect * MultiIMFactory::createIMConnect(IMAccount & account) 
{
	if (EnumIMProtocol::isSip( account.getProtocol() ) )
	{
		return _phApiFactory.createIMConnect(account);
	} 
	else 
	{
		return _purpleIMFactory.createIMConnect(account);
	}
}

IMChat * MultiIMFactory::createIMChat(IMAccount & account) 
{
	if (EnumIMProtocol::isSip( account.getProtocol() ) )
	{
		return _phApiFactory.createIMChat(account);
	} 
	else 
	{
		return _purpleIMFactory.createIMChat(account);
	}
}

IMPresence * MultiIMFactory::createIMPresence(IMAccount & account) 
{
	if (EnumIMProtocol::isSip( account.getProtocol() ) )
	{
		return _phApiFactory.createIMPresence(account);
	} 
	else 
	{
		return _purpleIMFactory.createIMPresence(account);
	}
}

IMContactList * MultiIMFactory::createIMContactList(IMAccount & account) 
{
	if (EnumIMProtocol::isSip( account.getProtocol() ) )
	{
		return _phApiFactory.createIMContactList(account);
	} 
	else 
	{
		return _purpleIMFactory.createIMContactList(account);
	}
}

void MultiIMFactory::removeIMAccount(IMAccount imAccount) 
{ 
	if (EnumIMProtocol::isSip( imAccount.getProtocol() ) )
	{
		return _phApiFactory.removeIMAccount(imAccount); 
	} 
	else 
	{ 
		return _purpleIMFactory.removeIMAccount(imAccount);
	}
}

void MultiIMFactory::imAccountUpdated(IMAccount imAccount) 
{
	if (EnumIMProtocol::isSip( imAccount.getProtocol() ) )
	{
		return _phApiFactory.imAccountUpdated(imAccount); 
	} 
	else 
	{ 
		return _purpleIMFactory.imAccountUpdated(imAccount);
	}
}

//VOXOX CHANGE CJC ADD FRIEND FROM MYSPACE SERVER
void MultiIMFactory::importContactsFromMySpaceServer(IMAccount imAccount) 
{ 
	if (imAccount.getProtocol() == EnumIMProtocol::IMProtocolMYSPACE)
	{ 
		//This only apply for myspace protocols
		return _purpleIMFactory.importContactsFromMySpaceServer(imAccount);
	}
}
