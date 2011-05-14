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
#include "WsUrl.h"

#include <WengoPhoneBuildId.h>

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/account/wengo/WengoAccount.h>

#include <util/WebBrowser.h>
#include <util/Logger.h>

#include <map>

WengoAccount * WsUrl::_wengoAccount = NULL;

static const char KEYWORD_DELIMITER = '@';

class KeywordMap {
public:
	void addKeyword(const std::string& keyword, const std::string& value) {
		_map[KEYWORD_DELIMITER + keyword + KEYWORD_DELIMITER] = value;
	}

	std::string replaceKeywords(const std::string& text) {
		String tmp = text;
		Map::const_iterator
			it = _map.begin(),
			end = _map.end();
		for (; it!=end; ++it) {
			tmp.replace(it->first, it->second);
		}
		return tmp;
	}

private:
	typedef std::map<std::string, std::string> Map;
	Map _map;
};

static std::string keyFromPage(WsUrl::Page page) {
	static std::map<WsUrl::Page, std::string> map;
	if (map.empty()) {
		map[WsUrl::Account] = Config::URL_ACCOUNT_KEY;
		map[WsUrl::AccountCreation] = Config::URL_ACCOUNTCREATION_KEY;
		map[WsUrl::FAQ] = Config::URL_FAQ_KEY;
		map[WsUrl::Forum] = Config::URL_FORUM_KEY;
		map[WsUrl::SmartDirectory] = Config::URL_INTERNALDIRECTORY_KEY;
		map[WsUrl::CallOut] = Config::URL_CALLOUT_KEY;
		map[WsUrl::SMS] = Config::URL_SMS_KEY;
		map[WsUrl::VoiceMail] = Config::URL_VOICEMAIL_KEY;
		map[WsUrl::BuyWengos] = Config::URL_BUYWENGOS_KEY;
		map[WsUrl::PhoneNumber] = Config::URL_PHONENUMBER_KEY;
		map[WsUrl::Wiki] = Config::URL_WIKI_KEY;
		map[WsUrl::LostPassword] = Config::URL_LOSTPASSWORD_KEY;
		map[WsUrl::Launchpad] = Config::URL_LAUNCHPAD_KEY;
		map[WsUrl::Directory] = Config::URL_EXTERNALDIRECTORY_KEY;
	}
	return map[page];
}

std::string WsUrl::getPageUrl(Page page) {
	std::string key = keyFromPage(page);
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return config.getStringKeyValue(key);
}

bool WsUrl::hasPage(Page page) {
	std::string url = getPageUrl(page);
	return !url.empty();
}

std::string WsUrl::getUrlWithoutAuth(const std::string & url) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	KeywordMap map;
	map.addKeyword("lang", config.getLanguage());

	std::string finalUrl = map.replaceKeywords(url);
	return finalUrl;
}

std::string WsUrl::getUrlWithAuth(const std::string & url) {
	if (!_wengoAccount) {
		LOG_WARN("no WengoAccount set, this method should not be called");
		return url;
	}
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	KeywordMap map;
	map.addKeyword("lang", config.getLanguage());
	map.addKeyword("login", _wengoAccount->getWengoLogin());
	map.addKeyword("password", _wengoAccount->getWengoPassword());

	std::string finalUrl = map.replaceKeywords(url);
	return finalUrl;
}

void WsUrl::openWengoUrlWithoutAuth(const std::string & url) {
	WebBrowser::openUrl(getUrlWithoutAuth(url));
}

void WsUrl::openWengoUrlWithAuth(const std::string & url) {
	WebBrowser::openUrl(getUrlWithAuth(url));
}

std::string WsUrl::getWengoDirectoryUrl() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return getUrlWithoutAuth(config.getInternalDirectoryUrl());
}

void WsUrl::showWengoAccount() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithAuth(config.getAccountUrl());
}

void WsUrl::showWengoFAQ() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getFaqUrl());
}

void WsUrl::showWengoForum() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getForumUrl());
}
//VOXOX - SEMR - 2009.05.19 New option Help Menu
void WsUrl::showReportBug(){
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getBugReportUrl());
}
//VOXOX - SEMR - 2009.05.19 New option Help Menu
void WsUrl::showSuggestFeature(){
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getSuggestionsUrl());
}
//VOXOX - SEMR - 2009.05.19 New option Help Menu
void WsUrl::showBillingFeature(){
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getBillingUrl());
}

void WsUrl::showWengoSmartDirectory() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getExternalDirectoryUrl());
}

void WsUrl::showWengoAccountCreation() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getAccountCreationUrl());
}

void WsUrl::showWengoCallOut() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getCalloutUrl());
}

void WsUrl::showWengoSMS() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getSmsUrl());
}

void WsUrl::showWengoVoiceMail() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getVoiceMailUrl());
}

void WsUrl::showWengoBuyWengos() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithAuth(config.getBuyWengosUrl());
}

void WsUrl::showWengoDownload() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithAuth(config.getDownloadUrl());
}

void WsUrl::showWengoPhoneNumber() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithAuth(config.getPhoneNumberUrl());
}

void WsUrl::showWikiPage() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getWikiUrl());
}

void WsUrl::showRatesPage(){
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getRatesUrl());
}

void WsUrl::showDeveloperPlatform(){
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getDeveloperPlatformUrl());
}

void WsUrl::showLostPasswordPage() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	openWengoUrlWithoutAuth(config.getLostPasswordUrl());
}

void WsUrl::showLauchpadPage() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	WebBrowser::openUrl(config.getLaunchpadUrl());
}
