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


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtIMAccountHelper.h"
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.05.29 

#include <util/Logger.h>

namespace QtIMAccountHelper 
{

void copyListToPtrVector(const IMAccountList& list, QtIMAccountPtrVector* vector) {
	IMAccountList::const_iterator
		it = list.begin(),
		end = list.end();

	for (; it!=end; ++it) 
	{
		vector->push_back(&(it->second) );
	}
}

bool compareIMAccountPtrs(const IMAccount* account1, const IMAccount* account2) 
{
//	int protocol1Order = QtEnumIMProtocolMap::getInstance().findByModelProtocol( account1->getProtocol() )->getOrder();
//	int protocol2Order = QtEnumIMProtocolMap::getInstance().findByModelProtocol( account2->getProtocol() )->getOrder();
	int protocol1Order = QtEnumIMProtocolMap::getInstance().findByQtProtocol( account1->getQtProtocol() )->getOrder();	//VOXOX - JRT - 2009.06.28 
	int protocol2Order = QtEnumIMProtocolMap::getInstance().findByQtProtocol( account2->getQtProtocol() )->getOrder();
	
	if (protocol1Order != protocol2Order) 
	{
		return protocol1Order < protocol2Order;
	}
	else
	{//VOXOX CHANGE Rolando 03-20-09
		if(account1->isVoxOxAccount())
		{
			return true;
		}
		else
		{
			return account1->getLogin() < account2->getLogin();
		}
	}	
}

} // namespace
