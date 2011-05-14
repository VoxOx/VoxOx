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

#ifndef OWIMACCOUNTLISTXMLSERIALIZER_H
#define OWIMACCOUNTLISTXMLSERIALIZER_H

#include <serializer/VoxXMLSerializerBase.h>	//VOXOX - JRT - 2009.06.01 
#include <imwrapper/IMAccountParameters.h>

class IMAccountList;

/**
 * Serializes an IMAccountList object.
 *
 * @author Philippe Bernery
 */
class IMAccountListXMLSerializer : public XMLSerializerBase	//public Serializable 
{
public:

	IMAccountListXMLSerializer(IMAccountList & imAccountList);

	void setIMAccountParametersData(const std::string & data);

	std::string serialize();

	bool unserialize(const std::string & data);

	bool unserializeWebService( TiXmlHandle& accountElt, IMAccountParameters& imAccountParameters );

private:

	IMAccountList & _imAccountList;
	std::string _imAccountParametersData;
};

#endif	//OWIMACCOUNTLISTXMLSERIALIZER_H
