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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "ConfigImporter.h"

#include "Config.h"
#include "ConfigManager.h"

#include <model/account/wengo/WengoAccount.h>
#include <model/contactlist/Contact.h>
#include <model/contactlist/ContactList.h>
#include <model/contactlist/ContactListXMLSerializer.h>
#include <model/contactlist/ContactListXMLSerializer1.h>
#include <model/profile/EnumSex.h>
#include <model/profile/StreetAddress.h>
#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileHandler.h>
#include <model/profile/UserProfileFileStorage1.h>
#include <model/profile/UserProfileXMLSerializer.h>

#include <imwrapper/IMAccountListXMLSerializer.h>
#include <imwrapper/IMAccountListXMLSerializer1.h>

#include <cutil/global.h>

#include <util/Base64.h>
#include <util/File.h>
#include <util/Logger.h>
#include <util/Path.h>
#include <util/String.h>

#include <settings/SettingsXMLSerializer.h>

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

typedef struct telNumber_s {
	string key;
	string value;
} telNumber_t;

typedef std::list<telNumber_t *> telNumberList;
typedef std::list<telNumber_t *>::iterator telNumberIt;

typedef struct address_s {
	string street;
	string city;
	string post_code;
	string state;
	string country;
} address_t;

typedef struct birthday_s {
	int day;
	int month;
	int year;
} birthday_t;

#define MALE 0
#define FEMALE 1
#define UNKNOWN 2

typedef struct vcard_s {
	string id;
	string fname;
	string lname;
	int gender;
	string company;
	string website;
	birthday_t birthday;
	string note;
	address_t address;
	telNumberList numbers;
	StringList emails;
	string owner;
	bool blocked;
} vcard_t;

typedef std::list<vcard_t *> vcardList;
typedef std::list<vcard_t *>::iterator vcardIt;

static const char * HOME_NUMBER_KEY = "home";
static const char * WORK_NUMBER_KEY = "work";
static const char * CELL_NUMBER_KEY = "cell";
static const char * FAX_NUMBER_KEY = "fax";
static const char * OTHER_NUMBER_KEY = "other";

telNumber_t * createNewNodeNumber(const std::string & key, const std::string & value) {
	telNumber_t * number = new telNumber_t();

	memset(number, 0, sizeof(telNumber_t));
	number->key = key;
	number->value = value;

	return number;
}

StringList mySplit(const std::string & str, char sep) {
	string word;
	StringList wordList;
	istringstream strStream(str);

	while (std::getline(strStream, word, sep)) {
		wordList += word;
	}

	return wordList;
}

static void initVcard(vcard_t * mVcard) {
	if (mVcard) {
		mVcard->gender = UNKNOWN;
		mVcard->blocked = false;
		mVcard->birthday.day = 1;
		mVcard->birthday.month = 1;
		mVcard->birthday.year = 1900;
	}
}

static const int CONFIG_UNKNOWN = 0;
static const int CONFIG_VERSION1 = 1;
static const int CONFIG_VERSION2 = 2;
static const int CONFIG_VERSION3 = 3;
static const int CONFIG_VERSION4 = 4;
static const int CONFIG_VERSION5 = 5;
static const int CONFIG_VERSION6 = 6;
static const int CONFIG_VERSION7 = 7;
static const int CONFIG_VERSION8 = 8;
static const int CONFIG_VERSION9 = 9;

static const std::string USERPROFILE_FILENAME = "userprofile.xml";
static const std::string USERCONFIG_FILENAME = "user.config";
static const std::string IMACCOUNTS_FILENAME = "imaccounts.xml";
static const std::string CONTACTLIST_FILENAME = "contactlist.xml";
static const std::string NEW_HISTORY_FILENAME = "history.xml";
static const std::string OLD_HISTORY_FILENAME = "_history";
static const std::string CONFIG_FILENAME = "config.xml";
static const std::string FAKE_LOGIN = "**fake_identity**";
static const std::string FAKE_PASSWORD = "**fake_password**";
static const std::string CONTACT_PREFIX = "#IMPORTED_CONTACT#";

void ConfigImporter::importConfig() {
	Config& config = ConfigManager::getInstance().getCurrentConfig();
	int systemVersion = boost::any_cast<int>( config.getDefaultValue(Config::CONFIG_VERSION_KEY) );
	int localVersion = detectLastVersion();

	if (localVersion != CONFIG_UNKNOWN && localVersion < systemVersion) {
		if (localVersion == CONFIG_VERSION1) {
			importConfigFromV1toV3();
			localVersion = CONFIG_VERSION3;
		}

		if (localVersion == CONFIG_VERSION2) {
			importConfigFromV2toV3();
			localVersion = CONFIG_VERSION3;
		}

		if (localVersion == CONFIG_VERSION3) {
			importConfigFromV3toV4();
			localVersion = CONFIG_VERSION4;
		}

		if (localVersion == CONFIG_VERSION4) {
			importConfigFromV4toV5();
			localVersion = CONFIG_VERSION5;
		}

		if (localVersion == CONFIG_VERSION5) {
			importConfigFromV5toV6();
			localVersion = CONFIG_VERSION6;
		}

		if (localVersion == CONFIG_VERSION6) {
			importConfigFromV6toV7();
			localVersion = CONFIG_VERSION7;
		}

		if (localVersion == CONFIG_VERSION7) {
			importConfigFromV7toV8();
			localVersion = CONFIG_VERSION8;
		}

		if (localVersion == CONFIG_VERSION8) {
			// Nothing to do, just bump the version number
			localVersion = CONFIG_VERSION9;
		}

		if (localVersion < systemVersion) {
			LOG_FATAL("importer: config=" + String::fromNumber(localVersion)
				+ " failed to reach config=" + String::fromNumber(systemVersion));
		}

		// Set the new version number
		Settings settings = readConfigFile();
		settings.set(Config::CONFIG_VERSION_KEY, systemVersion);
		writeConfigFile(settings);
	}
}

unsigned ConfigImporter::detectLastVersion() {
	string ConfigPathV1 = getWengoClassicConfigPath();
	string ConfigPathV2 = Config::getConfigDir();

	bool dirV1Exists = !ConfigPathV1.empty()
		&& File::exists(ConfigPathV1.substr(0, ConfigPathV1.size() - 1));
	bool dirV2Exists = !ConfigPathV2.empty()
		&& File::exists(ConfigPathV2.substr(0, ConfigPathV2.size() - 1));

	if (dirV2Exists) {
		if (File::exists(ConfigPathV2 + "profiles")) {
			
			std::string configFilename = ConfigPathV2 + CONFIG_FILENAME;
			if (!File::exists(configFilename)) {
				return CONFIG_UNKNOWN;
			}
			Settings settings = readConfigFile();
			if (settings.contains(Config::CONFIG_VERSION_KEY)) {
				return settings.getIntegerKeyValue(Config::CONFIG_VERSION_KEY);
			} else {
				// If the config version does not exist, it means that the
				// config is older than version 4.
				return CONFIG_VERSION3;
			}
		} else if (File::exists(ConfigPathV2 + "user.config")) {
			return CONFIG_VERSION2;
		} else if (dirV1Exists) {
			return CONFIG_VERSION1;
		}
	} else if (dirV1Exists) {
		return CONFIG_VERSION1;
	}

	return CONFIG_UNKNOWN;
}

string ConfigImporter::getWengoClassicConfigPath() {
	string result;

#if defined(OS_WINDOWS)
	result = File::convertPathSeparators(Path::getHomeDirPath() + "wengo/");
#elif defined(OS_LINUX)
	result = File::convertPathSeparators(Path::getHomeDirPath() + ".wengo/");
#endif

	return result;
}

/**
 * Works like std::getline() but will remove any ending '\r'. This is necessary
 * because the WengoPhone Classic creates files ending with '\r\n', regardless
 * of the platform
 */
static void getStrippedLine(std::ifstream& stream, std::string& line) {
	std::getline(stream, line);
	if (line[line.size() - 1] == '\r') {
		line = line.substr(0, line.size() - 1);
	}
}

bool ConfigImporter::classicVcardParser(const string & vcardFile, void * structVcard) {
	vcard_t * mVcard = (vcard_t *) structVcard;
	std::ifstream fileStream;
	string lastLine;

	fileStream.open(vcardFile.c_str());
	if (!fileStream) {
		LOG_ERROR("cannot open the file: " + vcardFile);
		return false;
	}

	getStrippedLine(fileStream, lastLine);
	if (lastLine != "BEGIN:VCARD") {
		return false;
	}

	String key, value, tmp;

	getStrippedLine(fileStream, tmp);

	while (!lastLine.empty()) {
		int pos = lastLine.find(":", 0);
		key = lastLine.substr(0, pos);
		value = lastLine.substr(pos + 1, lastLine.length() - (pos + 1));

		if (!value.empty()) {
			if (!key.compare("N")) {
				StringList mList = mySplit(value, ';');
				mVcard->lname = mList[0];
				mVcard->fname = mList[1];

				if (!mList[4].compare("Mme.")) {
					mVcard->gender = FEMALE;
				}
				else if (!mList[4].compare("Mr.")) {
					mVcard->gender = MALE;
				}
				else {
					mVcard->gender = UNKNOWN;
				}
			}
			else if (!key.compare("TEL;TYPE=home")) {
				mVcard->numbers.push_back(createNewNodeNumber(HOME_NUMBER_KEY, value));
			}

			else if (!key.compare("TEL;TYPE=work")) {
				mVcard->numbers.push_back	(createNewNodeNumber(WORK_NUMBER_KEY, value));
			}

			else if (!key.compare("TEL;TYPE=cell")) {
				mVcard->numbers.push_back(createNewNodeNumber(CELL_NUMBER_KEY, value));
			}

			else if (!key.compare("TEL;TYPE=pref")) {
				mVcard->id = CONTACT_PREFIX + value;
			}

			else if (!key.compare("TEL;TYPE=fax")) {
				mVcard->numbers.push_back(createNewNodeNumber(FAX_NUMBER_KEY, value));
			}

			else if (!key.compare("TEL;TYPE=other")) {
				mVcard->numbers.push_back(createNewNodeNumber(OTHER_NUMBER_KEY, value));
			}

			else if (!key.compare("EMAIL")) {
				mVcard->emails += value;
			}

			else if (!key.compare("ORG")) {
				mVcard->company = value;
			}

			else if (!key.compare("URL")) {
				mVcard->website = value;
			}

			else if (!key.compare("BDAY")) {
				StringList mList = mySplit(value, '-');
				mVcard->birthday.year = atoi(mList[0].c_str());
				mVcard->birthday.month = atoi(mList[1].c_str());
				mVcard->birthday.day = atoi(mList[2].c_str());
			}
			else if (!key.compare("NOTE")) {
				mVcard->note = value;
			}

			else if (!key.compare("ADR;TYPE=home;TYPE=pref")) {
				StringList mList = mySplit(value, ';');
				mVcard->address.street = mList[2];
				mVcard->address.city = mList[3];
				mVcard->address.state = mList[4];
				mVcard->address.post_code = mList[5];
				mVcard->address.country = mList[6];
			}
			else {
				LOG_DEBUG("KEY " + key + " not supported");
			}
		}

		lastLine = tmp.trim();
		getStrippedLine(fileStream, tmp);

		if (!tmp.empty()) {
			if (tmp.find(":", 0) == string::npos) {
				lastLine += tmp.trim();
				getStrippedLine(fileStream, tmp);
			}
		}
	}

	return true;
}

bool ConfigImporter::classicXMLParser(const string & xmlFile, void *structVcard) {
	vcard_t *mVcard = (vcard_t *) structVcard;
	std::ifstream fileStream;
	String lastLine;

	mVcard->blocked = false;

	fileStream.open(xmlFile.c_str());
	if (!fileStream) {
		LOG_ERROR("cannot open the file: " + xmlFile);
		return false;
	}

	std::getline(fileStream, lastLine);

	while (!lastLine.empty()) {
		lastLine = lastLine.trim();

		if (!strncmp(lastLine.c_str(), "<blocked>", 9)) {
			int pos1 = lastLine.find_first_of('>');
			int pos2 = lastLine.find_last_of('<');
			string resp = ((String)lastLine.substr(pos1 + 1, pos2 - (pos1 + 1))).trim();

			if (resp == "true") {
				mVcard->blocked = true;
			}
		}

		std::getline(fileStream, lastLine);
	}

	return true;
}

string ConfigImporter::classicVCardToString(void *structVcard) {
	vcard_t * mVcard = (vcard_t *) structVcard;

	//VOXOX CHANGE by Rolando 01-13-09
	string res = "<wgcard version=\"1.0\" xmlns=\"http://www.voxox.com/wgcard/1.0\">\n";

	//Todo: look at ProfileXMLSerializer and try to use the same serializer
	if (!mVcard->id.empty()) {
		res += ("<wengoid>" + mVcard->id + "</wengoid>\n");
	}

	res += "<name>\n";
	if (!mVcard->fname.empty()) {
		res += ("<first><![CDATA[" + mVcard->fname + "]]></first>\n");
	}
	if (!mVcard->lname.empty()) {
		res += ("<last><![CDATA[" + mVcard->lname + "]]></last>\n");
	}
	res += "</name>\n";

	if (mVcard->gender == MALE) {
		res += ("<sex>male</sex>\n");
	}
	else if (mVcard->gender == FEMALE) {
		res += ("<sex>female</sex>\n");
	}

	if (!mVcard->website.empty()) {
		res += ("<url type=\"website\">" + mVcard->website + "</url>\n");
	}

	if (mVcard->birthday.day && mVcard->birthday.month && mVcard->birthday.year) {
		res += "<birthday>\n<date>\n";
		res += ("<day>" + String::fromNumber(mVcard->birthday.day) + "</day>\n");
		res += ("<month>" + String::fromNumber(mVcard->birthday.month) + "</month>\n");
		res += ("<year>" + String::fromNumber(mVcard->birthday.year) + "</year>\n");
		res += "</date>\n</birthday>\n";
	}

	if (!mVcard->company.empty()) {
		res += "<organization>" + mVcard->company + "</organization>\n";
	}

	telNumberIt it;
	for (it = mVcard->numbers.begin(); it != mVcard->numbers.end(); it++) {
		if (!(*it)->key.compare("home") && !((*it)->value.empty())) {
			res += "<tel type=\"home\">" + (*it)->value + "</tel>\n";
		}
		else if (!(*it)->key.compare("work") && !((*it)->value.empty())) {
			res += "<tel type=\"work\">" + (*it)->value + "</tel>\n";
		}
		else if (!(*it)->key.compare("cell") && !((*it)->value.empty())) {
			res += "<tel type=\"cell\">" + (*it)->value + "</tel>\n";
		}
		else if (!(*it)->key.compare("fax") && !((*it)->value.empty())) {
			res += "<fax type=\"home\">" + (*it)->value + "</fax>\n";
		}
	}

	res += "<address type=\"home\">\n";
	if (!mVcard->address.street.empty()) {
		res += ("<street><![CDATA[" + mVcard->address.street + "]]></street>\n");
	}
	if (!mVcard->address.city.empty()) {
		res += ("<locality><![CDATA[" + mVcard->address.city + "]]></locality>\n");
	}
	if (!mVcard->address.state.empty()) {
		res += ("<region><![CDATA[" + mVcard->address.state + "]]></region>\n");
	}
	if (!mVcard->address.post_code.empty()) {
		res += ("<postcode><![CDATA[" + mVcard->address.post_code + "]]></postcode>\n");
	}
	if (!mVcard->address.country.empty()) {
		res += ("<country><![CDATA[" + mVcard->address.country + "]]></country>\n");
	}
	res += "</address>\n";

	if (!mVcard->id.empty()) {
		res += "<im protocol=\"SIP/SIMPLE\">\n";
		res += ("<id>" + mVcard->id + "</id>\n");
		res += ("<account>" + mVcard->owner + "</account>");
		res += "</im>\n";
	}

	if (mVcard->emails.size() >= 1 && !mVcard->emails[0].empty()) {
		res += ("<email type=\"home\">" + mVcard->emails[0] + "</email>\n");
	}
	if (mVcard->emails.size() >= 2 && !mVcard->emails[1].empty()) {
		res += ("<email type=\"work\">" + mVcard->emails[1] + "</email>\n");
	}
	if (mVcard->emails.size() >= 3 && !mVcard->emails[2].empty()) {
		res += ("<email type=\"other\">" + mVcard->emails[2] + "</email>\n");
	}

	if (!mVcard->note.empty()) {
		res += ("<notes><![CDATA[" + mVcard->note + "]]></notes>\n");
	}

	res += ("<group><![CDATA[Wengo]]></group>\n");

	res += "</wgcard>\n";
	return res;
}

void ConfigImporter::addContactDetails(Contact & contact, void * structVcard) {
	vcard_t * mVcard = (vcard_t *) structVcard;

	contact.setFirstName(mVcard->fname);
	contact.setLastName(mVcard->lname);
	contact.setCompany(mVcard->company);
	contact.setBirthdate(Date(mVcard->birthday.day, mVcard->birthday.month, mVcard->birthday.year));
	contact.setNotes(mVcard->note);
	contact.setWebsite(mVcard->website);
	if (!mVcard->address.city.empty()) {
		StreetAddress adress;
		adress.setStreet1(mVcard->address.street);
		adress.setCity(mVcard->address.city);
		adress.setPostalCode(mVcard->address.post_code);
		adress.setStateProvince(mVcard->address.state);
		adress.setCountry(mVcard->address.country);
		contact.setStreetAddress(adress);
	}

	if (mVcard->gender != UNKNOWN) {
		contact.setSex(mVcard->gender == MALE ? EnumSex::SexMale : EnumSex::SexFemale);
	}

	telNumberIt it;
	for (it = mVcard->numbers.begin(); it != mVcard->numbers.end(); it++) {
		if (!(*it)->key.compare(HOME_NUMBER_KEY) && !((*it)->value.empty())) {
			contact.setHomePhone((*it)->value);
		}
		else if (!(*it)->key.compare(WORK_NUMBER_KEY) && !((*it)->value.empty())) {
			contact.setWorkPhone((*it)->value);
		}
		else if (!(*it)->key.compare(CELL_NUMBER_KEY) && !((*it)->value.empty())) {
			contact.setMobilePhone((*it)->value);
		}
		else if (!(*it)->key.compare(FAX_NUMBER_KEY) && !((*it)->value.empty())) {
			contact.setFax((*it)->value);
		}
		else if (!(*it)->key.compare(OTHER_NUMBER_KEY) && !((*it)->value.empty())) {
			contact.setOtherPhone((*it)->value);
		}
	}

	if (mVcard->emails.size() >= 1 && !mVcard->emails[0].empty()) {
		contact.setPersonalEmail(mVcard->emails[0]);
	}
	if (mVcard->emails.size() >= 2 && !mVcard->emails[1].empty()) {
		contact.setWorkPhone(mVcard->emails[1]);
	}
	if (mVcard->emails.size() >= 3 && !mVcard->emails[2].empty()) {
		contact.setOtherPhone(mVcard->emails[2]);
	}
}

bool ConfigImporter::importContactsFromV1toV3(const string & fromDir, UserProfile & userProfile) {
	File mDir(fromDir);
	StringList fileList = mDir.getFileList();
	ContactList & contactList = userProfile.getContactList();

	ContactGroup contactGroup( "Classic", EnumGroupType::GroupType_User);	//JRT-GRPS
	contactList.addContactGroup( contactGroup );	///VOXOX - JRT - 2009.05.07 - JRT-GRPS

	IMAccountList imAccountList =
		userProfile.getIMAccountManager().getIMAccountsOfProtocol(EnumIMProtocol::IMProtocolWengo);

	if (!imAccountList.size()) {
		return false;
	}

	for (unsigned i = 0; i < fileList.size(); i++) {
		File mFile(fromDir + fileList[i]);
		string Id = fileList[i].substr(0, fileList[i].find("_", 0));
		vcard_t mVcard;

		initVcard(&mVcard);
		if (!mFile.getExtension().compare("vcf")) {
			if (classicVcardParser(fromDir + fileList[i], &mVcard) == false) {
				continue;
			}

			int extPos = fileList[i].find_last_of('.');
			string fileWoExt = fileList[i].substr(0, extPos + 1);
			classicXMLParser(fromDir + fileWoExt + "xml", &mVcard);

//			IMContact imContact(*imAccountList.begin(), mVcard.id);	//VOXOX - JRT - 2009.04.09 
			IMContact imContact(imAccountList.begin()->second, mVcard.id);
			
			//VOXOX - JRT - 2009.04.26 
//			Contact & contact = contactList.createContact();
//			contact.setGroupId(contactList.getContactGroupIdFromName("Classic"));
//			addContactDetails(contact, &mVcard);
//			contact.addIMContact(imContact);

			Contact* contact = contactList.createContact();
//			contact->setGroupId(contactList.getContactGroupIdFromName("Classic"));
			contact->addToGroup(contactList.getContactGroupIdFromName("Classic"), EnumGroupType::GroupType_User);	//VOXOX - JRT - 2009.05.06 JRT-GRPS
			addContactDetails(*contact, &mVcard);
			contact->addIMContact(imContact);
			contactList.addContact( contact );	//ContactList will delete contact
			//End Voxox
		}
	}
	return true;
}

typedef struct last_user_s {
	string login;
	string password;
	bool auto_login;
} last_user_t;

void * ConfigImporter::getLastWengoUser(const std::string & configUserFile, int version) {
	std::ifstream fileStream;
	String lastLine;

	last_user_t * lastUser = new last_user_t();
	fileStream.open(configUserFile.c_str());
	if (!fileStream) {
		LOG_ERROR("cannot open the file: " + configUserFile);
		return NULL;
	}

	std::getline(fileStream, lastLine);

	while (!lastLine.empty()) {
		lastLine = lastLine.trim();

		if (!strncmp(lastLine.c_str(), "<login>", 7)) {
			int pos1, pos2;
			if (version == CONFIG_VERSION2) {
				pos1 = lastLine.find_first_of('>');
				pos2 = lastLine.find_last_of('<');
			} else {
				pos2 = lastLine.find_first_of(']');
				pos1 = lastLine.find_last_of('[');
			}

			lastUser->login = ((String)lastLine.substr(pos1 + 1, pos2 - (pos1 + 1))).trim();
		}
		else if (!strncmp(lastLine.c_str(), "<password>", 10)) {
			int pos1, pos2;
			if (version == CONFIG_VERSION2) {
				pos1 = lastLine.find_first_of('>');
				pos2 = lastLine.find_last_of('<');
			} else {
				pos2 = lastLine.find_first_of(']');
				pos1 = lastLine.find_last_of('[');
			}

			lastUser->password = ((String) lastLine.substr(pos1 + 1, pos2 - (pos1 + 1))).trim();
		}
		else if (!strncmp(lastLine.c_str(), "<autoLogin>", 11)) {
			int pos1 = lastLine.find_first_of('>');
			int pos2 = lastLine.find_last_of('<');
			string resp = ((String) lastLine.substr(pos1 + 1, pos2 - (pos1 + 1))).trim();

			if (resp == (version == CONFIG_VERSION2 ? "1" : "true")) {
				lastUser->auto_login = true;
			} else {
				lastUser->auto_login = false;
			}
		}

		std::getline(fileStream, lastLine);
	}

	return lastUser;
}

bool ConfigImporter::importConfigFromV1toV3() {
	string classicPath = getWengoClassicConfigPath();
	File mDir(classicPath);
	last_user_t * lastUser = (last_user_t *) getLastWengoUser(classicPath + USERCONFIG_FILENAME, CONFIG_VERSION1);
	if (lastUser) {
		UserProfile userProfile;
		WengoAccount wAccount(lastUser->login, lastUser->password, true);
		userProfile.setSipAccount(wAccount, false);
	
		// An SSO request was made here before.
		// The SSO request was made to get the SIP identity of the user and link Wengo contacts to this id.
		// Faking this SSO request.
		IMAccount imAccount(FAKE_LOGIN, FAKE_PASSWORD, EnumIMProtocol::IMProtocolWengo);
		userProfile.addIMAccount(imAccount);

		string sep = mDir.getPathSeparator();
		String oldPath = classicPath + lastUser->login + sep + "contacts" + sep;
		importContactsFromV1toV3(oldPath, userProfile);

		String accountDir(userProfile.getProfileDirectory());
		File::createPath(accountDir);
		UserProfileFileStorage1 fStorage(userProfile);
		fStorage.save(userProfile.getName());

		Settings settings = readConfigFile();
		settings.set(Config::PROFILE_LAST_USED_NAME_KEY, userProfile.getName());
		writeConfigFile(settings);
	}

	return true;
}

bool ConfigImporter::importConfigFromV2toV3() {
	String configDir = Config::getConfigDir();

	FileReader file(configDir + USERPROFILE_FILENAME);
	if (file.open()) {
		string data = file.read();

		last_user_t * lastUser = (last_user_t *) getLastWengoUser(configDir + USERCONFIG_FILENAME, CONFIG_VERSION2);
		if (lastUser == NULL) {
			return false;
		}

		UserProfile userProfile;

		UserProfileXMLSerializer serializer(userProfile);
		serializer.unserialize(data);

		WengoAccount wAccount(lastUser->login, Base64::decode(lastUser->password), true);
		userProfile.setSipAccount(wAccount, false);
	
		// An SSO request was made here before.
		// The SSO request was made to get the SIP identity of the user and link Wengo contacts to this id.
		// Faking this SSO request.
		IMAccount imAccount(FAKE_LOGIN, FAKE_PASSWORD, EnumIMProtocol::IMProtocolWengo);
		userProfile.addIMAccount(imAccount);

		//remove user.config and userprofile.xml from the main directory
		File userConfigFile(configDir + USERCONFIG_FILENAME);
		userConfigFile.remove();
		File userProfileFile(configDir + USERPROFILE_FILENAME);
		userProfileFile.remove();

		String accountDir(userProfile.getProfileDirectory());
		File::createPath(accountDir);
		UserProfileFileStorage1 fStorage(userProfile);
		fStorage.save(userProfile.getName());

		File mFile1(configDir + IMACCOUNTS_FILENAME);
		mFile1.move(accountDir + IMACCOUNTS_FILENAME, true);

		File mFile2(configDir + CONTACTLIST_FILENAME);
		mFile2.move(accountDir + CONTACTLIST_FILENAME, true);

		File mDir(configDir);
		StringList dirList = mDir.getFileList();
		for (unsigned i = 0; i < dirList.size(); i++) {
			if (dirList[i].length() > OLD_HISTORY_FILENAME.length()) {
				if (dirList[i].substr(dirList[i].length() - OLD_HISTORY_FILENAME.length()) == OLD_HISTORY_FILENAME) {
					File mFile3(configDir + dirList[i]);
					mFile3.move(accountDir + NEW_HISTORY_FILENAME, true);
					break;
				}
			}
		}

		Settings settings = readConfigFile();
		settings.set(Config::PROFILE_LAST_USED_NAME_KEY, userProfile.getName());
		writeConfigFile(settings);
	}

	return true;
}

bool ConfigImporter::importConfigFromV3toV4() {
	StringList list = UserProfileHandler::getUserProfileNames();
	std::string sipSimpleProtocol = EnumIMProtocol::toString(EnumIMProtocol::IMProtocolSIPSIMPLE);
	std::string wengoProtocol = EnumIMProtocol::toString(EnumIMProtocol::IMProtocolWengo);

	for (StringList::const_iterator it = list.begin();
		it != list.end();
		++it) {

		std::string userProfileDirectory = File::convertPathSeparators(Config::getConfigDir() + "profiles/" + *it + "/");

		//Replacing in imaccounts.xml
		FileReader iIMAccountsFile(userProfileDirectory + "imaccounts.xml");
		iIMAccountsFile.open();
		String data = iIMAccountsFile.read();

		data.replace(sipSimpleProtocol, wengoProtocol);

		FileWriter oIMAccountsFile(userProfileDirectory + "imaccounts.xml");
		oIMAccountsFile.write(data);
		////

		//Replacing in contactlist.xml
		FileReader iContactListFile(userProfileDirectory + "contactlist.xml");
		iContactListFile.open();
		data = iContactListFile.read();

		data.replace(sipSimpleProtocol, wengoProtocol);

		FileWriter oContactListFile(userProfileDirectory + "contactlist.xml");
		oContactListFile.write(data);
		////
	}

	return true;
}

bool ConfigImporter::importConfigFromV4toV5() {
	std::string configFilename = Config::getConfigDir() + CONFIG_FILENAME;
	//Replacing in config.xml
	FileReader iConfigFile(configFilename);
	iConfigFile.open();
	String data = iConfigFile.read();

	// Fix video.quality key (int to string)
	data.replace("<video.quality><int>0</int></video.quality>",
		"<video.quality><string>VideoQualityNormal</string></video.quality>");
	data.replace("<video.quality><int>1</int></video.quality>",
		"<video.quality><string>VideoQualityGood</string></video.quality>");
	data.replace("<video.quality><int>2</int></video.quality>",
		"<video.quality><string>VideoQualityVeryGood</string></video.quality>");
	data.replace("<video.quality><int>3</int></video.quality>",
		"<video.quality><string>VideoQualityExcellent</string></video.quality>");

	// Fix wenbox.enable key (int to string)
	data.replace("<wenbox.enable><bool>0</bool></wenbox.enable>",
		"<wenbox.enable><string>WenboxStatusNotConnected</string></wenbox.enable>");
	data.replace("<wenbox.enable><bool>1</bool></wenbox.enable>",
		"<wenbox.enable><string>WenboxStatusEnable</string></wenbox.enable>");

	FileWriter oConfigFile(configFilename);
	oConfigFile.write(data);
	////

	return true;
}

Settings ConfigImporter::readConfigFile() {
	Settings settings;
	std::string configFilename = Config::getConfigDir() + CONFIG_FILENAME;
	FileReader iConfigFile(configFilename);
	if (iConfigFile.open()) {
		std::string data = iConfigFile.read();
		SettingsXMLSerializer serializer(settings);
		serializer.unserialize(data);
	}
	return settings;
}

void ConfigImporter::writeConfigFile(Settings& settings) {
	std::string configFilename = Config::getConfigDir() + CONFIG_FILENAME;
	SettingsXMLSerializer serializer(settings);
	std::string data = serializer.serialize();

	FileWriter oConfigFile(configFilename);
	oConfigFile.write(data);
}

bool ConfigImporter::importConfigFromV5toV6() {
	Settings settings = readConfigFile();

	// In V5 (WengoPhone 2.0), the values of GENERAL_SHOW_OFFLINE_CONTACTS_KEY
	// and GENERAL_SHOW_GROUPS_KEY were the opposite of their meaning.
	if (settings.contains(Config::GENERAL_SHOW_OFFLINE_CONTACTS_KEY)) {
		settings.set(Config::GENERAL_SHOW_OFFLINE_CONTACTS_KEY,
			!settings.getBooleanKeyValue(Config::GENERAL_SHOW_OFFLINE_CONTACTS_KEY)
			);
	}

	if (settings.contains(Config::GENERAL_SHOW_GROUPS_KEY)) {
		settings.set(Config::GENERAL_SHOW_GROUPS_KEY,
			!settings.getBooleanKeyValue(Config::GENERAL_SHOW_GROUPS_KEY)
			);
	}

	writeConfigFile(settings);
	return true;
}

bool ConfigImporter::importConfigFromV6toV7() {
	StringList list = UserProfileHandler::getUserProfileNames();
	std::map<std::string, EnumIMProtocol::IMProtocol> accountMap;

	for (StringList::const_iterator it = list.begin();
		it != list.end();
		++it) {

		std::string userProfileDirectory = File::convertPathSeparators(Config::getConfigDir() + "profiles/" + *it + "/");

		//Replacing AIM/ICQ in imaccounts.xml
		FileReader iIMAccountsFile(userProfileDirectory + "imaccounts.xml");
		iIMAccountsFile.open();
		String data = iIMAccountsFile.read();
		static const std::string AIMICQACCSTR = "<account protocol=\"AIM/ICQ\">";
		static const std::string LOGINSTR = "<login>";

		std::string::size_type accPos = data.find(AIMICQACCSTR);
		while (accPos != std::string::npos) {
			// Gets login and protocol
			std::string::size_type loginPos = data.find(LOGINSTR, accPos + AIMICQACCSTR.size());
			std::string login = data.substr(loginPos + LOGINSTR.size(),
				data.find("</login>", loginPos) - loginPos - LOGINSTR.size());
			bool beginsWithDigit = (String(login.substr(0, 1)).toInteger() != 0);
			EnumIMProtocol::IMProtocol protocol = EnumIMProtocol::IMProtocolAIM;
			if (beginsWithDigit) {
				protocol = EnumIMProtocol::IMProtocolICQ;
			}
			////

			// Changing protocol line
			data.replaceInRange(accPos, AIMICQACCSTR.size(), "AIM/ICQ", EnumIMProtocol::toString(protocol));
			////

			// Inserting account in map for changing contactlist.xml
			accountMap[login] = protocol;
			////

			accPos += AIMICQACCSTR.size();
			accPos = data.find(AIMICQACCSTR);
		}

		FileWriter oIMAccountsFile(userProfileDirectory + "imaccounts.xml");
		oIMAccountsFile.write(data);
		////

		//Replacing in contactlist.xml
		FileReader iContactListFile(userProfileDirectory + "contactlist.xml");
		iContactListFile.open();
		data = iContactListFile.read();
		static const std::string AIMICQIMSTR = "<im protocol=\"AIM/ICQ\">";
		static const std::string ACCOUNTSTR = "<account>";

		std::string::size_type imPos = data.find(AIMICQIMSTR);
		while (imPos != std::string::npos) {
			std::string::size_type accountPos = data.find(ACCOUNTSTR, imPos + AIMICQIMSTR.size());
			std::string account = data.substr(accountPos + ACCOUNTSTR.size(),
				data.find("</account>", accountPos) - accountPos - ACCOUNTSTR.size());
			data.replaceInRange(imPos, AIMICQIMSTR.size(), "AIM/ICQ", EnumIMProtocol::toString(accountMap[account]));

			imPos += AIMICQIMSTR.size();
			imPos = data.find(AIMICQIMSTR);
		}

		FileWriter oContactListFile(userProfileDirectory + "contactlist.xml");
		oContactListFile.write(data);
		////
	}

	return true;
}

bool ConfigImporter::importConfigFromV7toV8() {
	StringList list = UserProfileHandler::getUserProfileNames();
	std::map<std::string, EnumIMProtocol::IMProtocol> accountMap;

	for (StringList::const_iterator it = list.begin();
		it != list.end();
		++it) {

		std::string userProfileDirectory = File::convertPathSeparators(Config::getConfigDir() + "profiles/" + *it + "/");

		//Loading data
		FileReader iIMAccountsFile(userProfileDirectory + "imaccounts.xml");
		iIMAccountsFile.open();
		String data = iIMAccountsFile.read();

		IMAccountList imAccountList;
		IMAccountListXMLSerializer1 imAccountListSerializer1(imAccountList);
		imAccountListSerializer1.unserialize(data);

		FileReader iContactListFile(userProfileDirectory + "contactlist.xml");
		iContactListFile.open();
		data = iContactListFile.read();

		UserProfile userProfile;
		ContactList contactList(userProfile);
		ContactListXMLSerializer1 contactListSerializer1(contactList, imAccountList);
		contactListSerializer1.unserialize(data);

		// Look for IMAccount and Contact link to the IMAccount with login FAKE_LOGIN. It has been introduced by a previous importation step
		// to avoid the SSO request.
		for (IMAccountList::iterator ait = imAccountList.begin(); ait != imAccountList.end(); ++ait) {
//			if (((*ait).getProtocol() == EnumIMProtocol::IMProtocolWengo)
//				&& ((*ait).getLogin() == FAKE_LOGIN)) {
			if (( ait->second.getProtocol() == EnumIMProtocol::IMProtocolWengo)
				&& ( ait->second.getLogin() == FAKE_LOGIN)) {

				// Update contact links to this IMAccount (update UUID and remove CONTACT_PREFIX).
				//VOXOX - JRT - 2009.04.07 - Use new map-based class.
//				for (ContactList::Contacts::iterator cit = const_cast<ContactList::Contacts &>(contactList.getContacts()).begin();
//					cit != contactList.getContacts().end();
				for ( Contacts::iterator cit = const_cast<Contacts &>(contactList.getContacts()).begin();
					cit != contactList.getContacts().end();
					cit++) {
//					IMContactSet & imContactSet = const_cast<IMContactSet &>((*cit).getIMContactSet());	//VOXOX - JRT - 2009.04.07 
					IMContactSet & imContactSet = const_cast<IMContactSet &>((*cit).second.getIMContactSet());
					for (IMContactSet::iterator imit = imContactSet.begin();
						imit != imContactSet.end();
						imit++) {
						if (String((*imit).getContactId()).beginsWith(CONTACT_PREFIX)) {
							const_cast<IMContact &>(*imit).setContactId((*imit).getContactId().substr(CONTACT_PREFIX.size()));
//							const_cast<IMContact &>(*imit).setIMAccount(&(*ait));	//VOXOX - JRT - 2009.04.09 
							const_cast<IMContact &>(*imit).setIMAccount(&(ait->second));
						}
					}
				}

				// Removing the IMAccount
				imAccountList.erase(ait);
				break;
			}
		}
		////

		IMAccountListXMLSerializer lastIMAccountListSerializer(imAccountList);
		data = lastIMAccountListSerializer.serialize();
		FileWriter oIMAccountsFile(userProfileDirectory + "imaccounts.xml");
		oIMAccountsFile.write(data);

		ContactListXMLSerializer lastContactListSerializer(contactList, imAccountList);
		data = lastContactListSerializer.serialize();
		FileWriter oContactListFile(userProfileDirectory + "contactlist.xml");
		oContactListFile.write(data);
		////	
	}

	return true;
}
