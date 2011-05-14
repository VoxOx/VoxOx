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

#include <imwrapper/IMAccountListXMLSerializer.h>

#include <imwrapper/IMAccountList.h>
#include <imwrapper/IMAccountXMLSerializer.h>
#include <imwrapper/IMAccount.h>

#include <tinyxml.h>

using namespace std;

IMAccountListXMLSerializer::IMAccountListXMLSerializer(IMAccountList & imAccountList)
	: _imAccountList(imAccountList) 
{
}

void IMAccountListXMLSerializer::setIMAccountParametersData(const std::string & data) 
{
	_imAccountParametersData = data;
}

std::string IMAccountListXMLSerializer::serialize() 
{
	string result;

	result += "<imaccounts>\n";

	for (IMAccountList::const_iterator it = _imAccountList.begin() ; it != _imAccountList.end() ; it++) 
	{
		IMAccountXMLSerializer serializer( const_cast<IMAccount&>(it->second) );
		result += serializer.serialize();
	}

	result += "</imaccounts>\n";

	return result;	
}

bool IMAccountListXMLSerializer::unserialize(const std::string & data) 
{
	TiXmlDocument doc;
	doc.Parse(data.c_str());
	
	TiXmlHandle docHandle(& doc);
	TiXmlNode * imaccounts = docHandle.FirstChild("imaccounts").Node();
	
	if (imaccounts) 
	{
		TiXmlNode * lastChild = NULL;
		while ((lastChild = imaccounts->IterateChildren("account", lastChild))) 
		{
			string accountData;
			accountData << *lastChild;
			
			IMAccount account;
			IMAccountXMLSerializer serializer(account);
			serializer.setIMAccountParametersData(_imAccountParametersData);
			serializer.unserialize(accountData);

			//VOXOX - JRT - 2009.07.13 - HACK to remove old voxox domain accounts.
			if ( !account.isOldVoxOxDomain() )	//Just throw this away.
			{
				_imAccountList.Add( account );
			}
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------

bool IMAccountListXMLSerializer::unserializeWebService( TiXmlHandle& networkElt, IMAccountParameters& imAccountParameters )
{
	if ( networkElt.Node() )
	{
		//VOXOX - JRT - 2009.07.14 - We need to use 'new', so we generate IMAccount objects with unique UUID
		IMAccount* imAccount;

		TiXmlNode*  networkNode = NULL;

		while ( networkNode = networkElt.Node()->IterateChildren(networkNode) )
		{
			imAccount = new IMAccount();	//VOXOX - JRT - 2009.07.14 

			IMAccountXMLSerializer serializer( *imAccount );
			TiXmlHandle h(networkNode);
			serializer.unserializeWebService( h, imAccountParameters );	

			//VOXOX - JRT - 2009.06.02  we currently get empty node if no UserNetworks exist on server.
			if ( imAccount->getProtocol() != EnumIMProtocol::IMProtocolUnknown )
			{
				//VOXOX - JRT - 2009.07.13 - HACK to remove old voxox domain accounts.
				if ( !imAccount->isOldVoxOxDomain() )	//Just throw this away.
				{
					_imAccountList.Add( imAccount );
				}
			}
			else
			{
				int xxx = 1;
			}

			delete imAccount;
			imAccount = NULL;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
