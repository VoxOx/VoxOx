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

#include <imwrapper/IMAccountListFileStorage.h>

#include <imwrapper/IMAccountListXMLSerializer.h>
#include <imwrapper/IMAccountList.h>

#include <util/File.h>
#include <util/Logger.h>

#include <string>

using namespace std;

static const std::string IMACCOUNT_FILENAME = "imaccounts.xml";

IMAccountListFileStorage::IMAccountListFileStorage(IMAccountList & imAccountList)
	: IMAccountListStorage(imAccountList),
	_imAccountList(imAccountList) {
}

IMAccountListFileStorage::~IMAccountListFileStorage() {
}

void IMAccountListFileStorage::setIMAccountParametersData(const std::string& data) {
	_imAccountParametersData = data;
}

bool IMAccountListFileStorage::load(const string & url) {
	FileReader file(url + IMACCOUNT_FILENAME);

	if (file.open()) {
		string data = file.read();
		file.close();

		IMAccountListXMLSerializer serializer(_imAccountList);
		serializer.setIMAccountParametersData(_imAccountParametersData);
		serializer.unserialize(data);

		return true;
	}

	return false;
}

bool IMAccountListFileStorage::save(const string & url) {
	FileWriter file(url + IMACCOUNT_FILENAME);

	IMAccountListXMLSerializer serializer(_imAccountList);
	file.write(serializer.serialize());
	file.close();

	return true;
}
