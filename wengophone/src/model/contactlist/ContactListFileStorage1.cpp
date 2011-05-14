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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "ContactListFileStorage1.h"

#include "ContactListXMLSerializer1.h"
#include "ContactList.h"
#include "ContactGroup.h"
#include "Contact.h"

#include <util/File.h>
#include <util/Logger.h>

#include <string>
using namespace std;

static const std::string CONTACTLIST_FILENAME = "contactlist.xml";

ContactListFileStorage1::ContactListFileStorage1(ContactList & contactList, IMAccountList & imAccountList)
	: ContactListStorage(contactList),
	_imAccountList(imAccountList) {
}

ContactListFileStorage1::~ContactListFileStorage1() {
}

bool ContactListFileStorage1::load(const std::string & url) {
	FileReader file(url + CONTACTLIST_FILENAME);
	if (file.open()) {
		string data = file.read();

		ContactListXMLSerializer1 serializer(_contactList, _imAccountList);
		serializer.unserialize(data);
		return true;
	}

	return false;
}

bool ContactListFileStorage1::save(const std::string & url) {
	FileWriter file(url + CONTACTLIST_FILENAME);
	ContactListXMLSerializer1 serializer(_contactList, _imAccountList);

	file.write(serializer.serialize());
	return true;
}
