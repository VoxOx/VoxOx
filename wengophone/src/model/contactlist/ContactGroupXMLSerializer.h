/*
	VOXOX !!!!!
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

#ifndef CONTACTGOUP_XMLSERIALIZER_H
#define CONTACTGOUP_XMLSERIALIZER_H

#include <serializer/VoxXMLSerializerBase.h>	//VOXOX - JRT - 2009.03.31
#include <model/contactlist/ContactList.h>		//Declaration of ContactGroupSet

#include <tinyxml.h>
#include <string>

/**
 * Serialize a ContactGroupSet object.
 *
 * @author Jeff Theinert
 */
class ContactGroupSet;

class ContactGroupSetXMLSerializer : public XMLSerializerBase
{
public:
//	ContactGroupSetXMLSerializer( ContactList::ContactGroupSet& groupSet );
	ContactGroupSetXMLSerializer( ContactGroupSet& groupSet );		//VOXOX - JRT - 2009.05.05 

	std::string serialize();
	bool		unserialize(const std::string & data);
	bool		unserialize( TiXmlNode* groupsNode );

private:
//	ContactList::ContactGroupSet& _groupSet;
	ContactGroupSet& _groupSet;			//VOXOX - JRT - 2009.05.05 
};

//=============================================================================

class ContactGroupXMLSerializer : public XMLSerializerBase
{
public:
	ContactGroupXMLSerializer( ContactGroup& group );

	std::string serialize();
	bool		unserialize(const std::string & data);

protected:
	std::string	parseElement( TiXmlHandle& rootElt, const char* childTag );

private:
	ContactGroup& _group;
};


#endif //CONTACTGOUP_XMLSERIALIZER_H
