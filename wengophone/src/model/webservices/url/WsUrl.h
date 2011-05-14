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

#ifndef OWWSURL_H
#define OWWSURL_H

#include <util/NonCopyable.h>

#include <string>

class WengoAccount;

/**
 * Helper class that stores all the url related to Wengo.
 *
 * FIXME
 * This class does not take a WengoAccount, it uses WengoPhone::instance
 * which is a big hack. A real solution must be find for the implementation
 * of openWengoUrlWithAuth(). Check WengoPhone::instance.
 * It has been done in order to keep WsUrl methods static thus easier to use,
 * I would like to keep them static since it is easier to use.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class WsUrl : NonCopyable {
	friend class UserProfileHandler;
public:
	enum Page {
		Account,
		AccountCreation,
		FAQ,
		Forum,
		SmartDirectory,
		CallOut,
		SMS,
		VoiceMail,
		BuyWengos,
		PhoneNumber,
		Wiki,
		LostPassword,
		Launchpad,
		Directory
	};

	static std::string getPageUrl(Page);

	static bool hasPage(Page);

	/**
	 * Opens a web browser and shows the Wengo account informations inside.
	 */
	static void showWengoAccount();

	/**
	 * Opens a web browser and shows the Wengo account creation page.
	 */
	static void showWengoAccountCreation();

	/**
	 * Opens a web browser and shows the Wengo help center page.
	 */
	static void showWengoFAQ();

	/**
	 * Opens a web browser and shows the Wengo forum page.
	 */
	static void showWengoForum();

	/**
	 * Opens a web browser and shows the Report Bug page.
	 */
	//VOXOX - SEMR - 2009.05.19 New option Help Menu
	static void showReportBug();

	/**
	 * Opens a web browser and shows the Suggest Feature page.
	 */
	//VOXOX - SEMR - 2009.05.19 New option Help Menu
	static void showSuggestFeature();

	/**
	 * Opens a web browser and shows the Billing Feature page.
	 */
	//VOXOX - SEMR - 2009.05.19 New option Help Menu
	static void showBillingFeature();

	/**
	 * Opens a web browser and shows the Wengo smart directory page.
	 */
	static void showWengoSmartDirectory();

	/**
	 * Opens a web browser and shows the Wengo callout page.
	 */
	static void showWengoCallOut();

	/**
	 * Opens a web browser and shows the Wengo SMS page.
	 */
	static void showWengoSMS();

	/**
	 * Opens a web browser and shows the Wengo voice mail page.
	 */
	static void showWengoVoiceMail();

	/**
	 * Opens a web browser and shows the Wengo buy page.
	 */
	static void showWengoBuyWengos();

	/**
	 * Opens a web browser and shows the Wengo download page.
	 */
	static void showWengoDownload();

	/**
	 * Opens a web browser and shows the Wengo phone number associated with the Wengo account.
	 */
	static void showWengoPhoneNumber();

	/**
	 * Opens a web browser and shows the Wengo wiki page.
	 */
	static void showWikiPage();

	/**
	 * Opens a web browser and shows the VoxOx rates page.
	 */
	static void showRatesPage();

	/**
	 * Opens a web browser and shows the VoxOx rates page.
	 */
	static void showDeveloperPlatform();

	/**
	 * Opens a web browser and shows the lost password page.
	 */
	static void showLostPasswordPage();

	/**
	 * Opens a web browser and shows launchpad OpenWengo page.
	 */
	static void showLauchpadPage();

	/**
	 * Gets the Wengo directory url.
	 *
	 * @return Wengo directory url
	 */
	static std::string getWengoDirectoryUrl();

private:

	/**
	 * Opens a web browser with the given url.
	 *
	 * @param url the url to open
	 */
	static void openWengoUrlWithoutAuth(const std::string & url);

	/**
	 * Opens a web browser with the given url.
	 *
	 * @param url the url to open
	 */
	static void openWengoUrlWithAuth(const std::string & url);

	/**
	 * Gets url without authentication.
	 *
	 * @param url the url to transform
	 */
	static std::string getUrlWithoutAuth(const std::string & url);

	/**
	 * Gets url with authentication.
	 *
	 * @param url the url to transform
	 */
	static std::string getUrlWithAuth(const std::string & url);

	/**
	 * Sets the value of _wengoAccount.
	 */
	static void setWengoAccount(WengoAccount * wengoAccount) {
		_wengoAccount = wengoAccount;
	}

	/** Link to the current WengoAccount. */
	static WengoAccount * _wengoAccount;
};

#endif	//OWWSURL_H
