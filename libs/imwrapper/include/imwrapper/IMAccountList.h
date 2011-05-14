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

#ifndef OWIMACCOUNTLIST_H
#define OWIMACCOUNTLIST_H

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMContactSet.h>									//VOXOX - JRT - 2009.04.09 
#include "../../../wengophone/src/model/profile/VoxListTemplate.h"	//VOXOX - JRT - 2009.04.09 

/**
 * Instant Messaging account list.
 *
 * @author Philippe Bernery
 * Modified by J R Theinert
 */
class IMAccountList : public VoxMapTemplate<std::string, IMAccount> 
{
public:
	IMAccount*  findByAccount  ( const IMAccount& rAcct  );
	IMAccount*  findByKey	   ( const std::string& key );
	IMAccount*	findByLoginInfo( const std::string& loginValue, EnumIMProtocol::IMProtocol protocol );
	IMAccount*	findByServerProtocolId( QtEnumIMProtocol::ServerProtocolId nid );
	IMAccount*	findByUserNetworkId  ( int tgtUserNetworkId );
	IMAccount*	getFirstIMAccount();
	IMAccount*  findFirstValidAccount( const IMContactSet& rImContactSet );
	IMAccount*  getVoxOxAccount() const;
	IMAccount*  findByKeyAndClone( const std::string& imAccountId );
	IMAccount*	findByUUID( const std::string& uuid );
	IMAccount*  findByKeyNoClone( const std::string& imAccountId );//VOXOX CHANGE by Rolando - 2009.07.14 


	bool		update( IMAccount& imAccount );
	bool		contains( const IMAccount& rAcct );
	bool		containsVoxOxAccount();

	EnumPresenceState::PresenceState getPresenceState()			const;
	EnumPresenceState::PresenceState getIMPresenceState()		const;
	EnumPresenceState::PresenceState getVoxOxIMPresenceState()	const;
	EnumPresenceState::PresenceState getSIPPresenceState()		const;

	bool isConnected()		const;
	bool isIMConnected()	const;
	bool isSIPConnected()	const;
	bool isVoxOxConnected() const;

	void getIMAccountsOfProtocol	  ( EnumIMProtocol::IMProtocol              protocol,  IMAccountList& rResult );
	void getIMAccountsOfQtProtocol	  ( QtEnumIMProtocol::IMProtocol              protocol,  IMAccountList& rResult );//VOXOX - CJC - 2009.12.17 
	void getIMAccountsOfProtocolVector( std::vector<EnumIMProtocol::IMProtocol> protocols, IMAccountList& rResult );
};

#endif //OWIMACCOUNTLIST_H
