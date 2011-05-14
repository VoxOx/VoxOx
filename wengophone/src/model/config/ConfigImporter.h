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

#ifndef OWCONFIGIMPORTER_H
#define OWCONFIGIMPORTER_H

#include <model/account/SipAccount.h>
#include <model/account/wengo/WengoAccount.h>

#include <thread/Condition.h>
#include <thread/Mutex.h>

#include <string>

class Contact;
class UserProfile;
class Settings;

/**
 * Imports the config from a previous WengoPhone version.
 *
 * This class should not use Config getters and setters because at the time
 * it's used the user config has not been loaded in Config yet. Any value set
 * using a Config setter will be overwritten when user config is loaded.
 *
 * Instead of the Config getters and setters, you should use the static methods
 * readConfigFile() and writeConfigFile() to read and write the *user* config
 * to/from a Settings instance.
 *
 * @ingroup model
 * @author Julien Bossart
 * @author Philippe Bernery
 */
class ConfigImporter {
public:

	/**
	 * Import configuration from previous version of WengoPhone
	 * If this method fails it makes a LOG_FATAL() -> assertion failed
	 */
	static void importConfig();

private:
	static Settings readConfigFile();
	static void writeConfigFile(Settings&);

	/**
	 * Detects the last available version.
	 *
	 * @return the version of the last found version.
	 */
	static unsigned detectLastVersion();

	static std::string getWengoClassicConfigPath();

	static bool classicVcardParser(const std::string & vcardFile, void * structVcard);

	static bool classicXMLParser(const std::string & xmlFile, void * structVcard);

	static std::string classicVCardToString(void * structVcard);

	static void * getLastWengoUser(const std::string & configUserFile, int version);

	static bool importConfigFromV1toV3();

	static bool importConfigFromV2toV3();

	static bool importConfigFromV3toV4();

	static bool importConfigFromV4toV5();

	static bool importConfigFromV5toV6();

	static bool importConfigFromV6toV7();

	static bool importConfigFromV7toV8();

	static bool importContactsFromV1toV3(const std::string & fromDir, UserProfile & userProfile);

	static void addContactDetails(Contact & contact, void * structVcard);

};

#endif	//OWCONFIGIMPORTER_H
