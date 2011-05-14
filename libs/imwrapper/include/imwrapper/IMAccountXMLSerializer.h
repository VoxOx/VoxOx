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

#ifndef OWIMACCOUNTXMLSERIALIZER_H
#define OWIMACCOUNTXMLSERIALIZER_H

//#include <serialization/Serializable.h>
#include <serializer/VoxXMLSerializerBase.h>	//VOXOX - JRT - 2009.06.01 
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.06.06 
#include <imwrapper/EnumIMProtocol.h>			//VOXOX - JRT - 2009.05.27 
#include <imwrapper/IMAccountParameters.h>	
#include <tinyxml.h>							//VOXOX - JRT - 2009.05.27 


class IMAccount;

/**
 * Serializes an IMAccount object.
 *
 * @author Philippe Bernery
 */
class IMAccountXMLSerializer : public XMLSerializerBase	//public Serializable 
{
public:
	IMAccountXMLSerializer(IMAccount & imAccount);

	void setIMAccountParametersData(const std::string & data);

	std::string serialize();

	bool unserialize(const std::string & data);

	std::string serializeForWebService( std::string& buildId );			//VOXOX - JRT - 2009.06.01 
	bool unserializeWebService( TiXmlHandle & account, IMAccountParameters& imAccountParameters );	//VOXOX - JRT - 2009.05.27 

	void setMethod( const std::string& method )		{ _method = method;	}
	std::string getMethod() const					{ return _method;	}

private:
	void getIMProtocols( const std::string& protocolId, EnumIMProtocol::IMProtocol& imProtocol, QtEnumIMProtocol::IMProtocol& qtImProtocol);
	QtEnumIMProtocol::IMProtocol getQtProtocolFromId(std::string id);
	EnumIMProtocol::IMProtocol	 getModelProtocolFromId(std::string id);					//VOXOX - JRT - 2009.05.27 
	void initImAccountParameters( IMAccount& imAccount );

	IMAccount & _imAccount;
	std::string _imAccountParametersData;
	std::string _method;
};

#endif	//OWIMACCOUNTXMLSERIALIZER_H
