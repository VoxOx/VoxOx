/*
 * VoxOx
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

#ifndef VOX_WS_ACCOUNT_H
#define VOX_WS_ACCOUNT_H

#include <model/webservices/WengoWebService.h>
#include <serializer/VoxQueryResultXMLSerializer.h>
#include <thread/RecursiveMutex.h>
#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountList.h>

/**
 * Sends info to webservices about Use IM Accounts.
 *
 * @author Christopher Jimenez, Jeff Theinert
 */

class UserProfile;

class WsAccount : public WengoWebService 
{
public:
	WsAccount( const WsAccount& src );	//Needed for boost signal/slot
	WsAccount(UserProfile & userProfile );

	virtual ~WsAccount() {}

	void setUserNetwork	  ( const IMAccount& imAccount );
	void removeUserNetwork( int serverKey );
	void getUserNetworks  ();

	WsAccount& operator=( const WsAccount& src );

	Event<void ( WsAccount& sender, const IMAccountList imAccountList )> wsAccountFetchedEvent;
	Event<void ( WsAccount& sender, const IMAccount& imAccount )>		 wsAccountAddedEvent;
	Event<void ( WsAccount& sender, int userNetworkId )>				 wsAccountRemovedEvent;

private:
	/**
	 * @see WengoWebService
	 */
	void answerReceived(const std::string & answer, int requestId );

//	void setUserNetwork( const std::string& method, int serverKey, const std::string& xml ) ;
	void setUserNetwork( const std::string& method, const std::string& xml ) ;

	void handleGet	 ( const IMAccountList& imAccountList );
	void handleSet   ( const IMAccountList& imAccountList );
	void handleRemove();

	void parseGet   ( TiXmlNode* syncNode, IMAccountList& imAccountList );
	void parseSet   ( TiXmlNode* syncNode, IMAccountList& imAccountList );
	void parseRemove( TiXmlNode* syncNode, int&unid, bool& bDeleted );

	TiXmlNode*  findSyncNode( TiXmlHandle docHandle );			//TODO: part of base class as pure virtual.
	std::string getServerProtocolId  ( const IMAccount& imAccount );
	std::string initParameters( const std::string& method );	//TODO: move to base class
	std::string getSecurityKey();								//TODO: move to base class

	std::string toXml( const IMAccount& imAccount,     const std::string& method );
	void fromXml     ( IMAccountList&   imAccountList, const std::string& xml );


	UserProfile&	_userProfile;		//TODO: move to base class
	QueryResult		_queryResult;		//TODO: move to base class
	bool			_bDeleted;
	int				_userNetworkId;
	RecursiveMutex	_mutex;
};

#endif //VOX_WS_ACCOUNT_H

