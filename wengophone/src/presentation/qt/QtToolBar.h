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

#ifndef OWQTTOOLBAR_H
#define OWQTTOOLBAR_H

#include <QtCore/QObject>
#include <util/NonCopyable.h>
#include <model/webservices/url/WsUrl.h>

//VOXOX CHANGE by ASV 07-03-2009: We are not using QtContactMenu2 anymore
//#include "contactlist/QtContactMenu2.h"			//VOXOX - JRT - 2009.05.03
#include "contactlist/QtEnumSortOption.h"		//VOXOX - JRT - 2009.05.03

class QtWengoPhone;
class CWengoPhone;
class QtHistoryWidget;

class QAction;
class QMenu;
class QWidget;
class QTextEdit;
class QLineEdit;
class QComboBox;
class QWebView;
class QShortcut;

//class QtContactProfileWidget;	//VOXOX - JRT - 2009.06.29 

class Config;	//VOXOX - JRT - 2009.05.03 - for convenience method.

namespace Ui { class WengoPhoneWindow; }

/**
 * Tool bar and menu actions from the main window.
 *
 * @author Tanguy Krotoff
 */
class QtToolBar : public QObject, NonCopyable {
	Q_OBJECT
public:

	QtToolBar(QtWengoPhone & qtWengoPhone, Ui::WengoPhoneWindow * qtWengoPhoneUi, QWidget * parent);

	~QtToolBar();

	QWidget * getWidget() const;


	/**
	 * Makes sure the toggle actions (like show/hide hidden contacts and
	 * show/hide groups) are correctly translated
	 */
	void retranslateUi();

	QMenu * getStatusBarAddMenu();

	QMenu * getContactsTabMenu();
	QMenu * getKeypadTabMenu();
	QMenu * getHistoryTabMenu();
	QMenu * getAppsTabMenu();

	//QMenu * getNetworkAddMenu()

	QMenu * getStatusBarSettingsMenu();

	bool getUserIsLogged(){
		return _userIsLogged;
	};
	
	void fixMacOSXMenus(); //VOXOX CHANGE by ASV 06-29-2009

	// VOXOX CHANGE -ASV- 07-14-2009
	QMenu *getMenuWengo();
	QMenu *getMenuEdit();
	QMenu *getMenuContacts();
	QMenu *getMenuActions();	
	QMenu *getMenuTools();
	QMenu *getMenuWindow();	
	QMenu *getMenuHelp();	
	
	/**
	 * Configure toolbar according to the settings from the config file
	 */
//VOXOX CHANGE by ASV 04-24-2009: we are not using this anymore
//	void updateToolBar();

Q_SIGNALS:
	void configChanged(QString key);
	//VOXOX CHANGE Rolando 03-20-09
	void showOrHideProfileBar(bool);

	void showOrHideCallBar(bool);

public Q_SLOTS:

	void showWengoAccount();

	void editMyProfile();

	//VOXOX CHANGE Rolando 03-20-09
	void profileBarActionTriggered();

	void callBarActionTriggered();

	void addContact();
	void addCOntactSlot();

	//void editContact();

	void showConfig();
	void showPhoneConfig();
	void showNetworkConfig();		//VOXOX - SEMR - 2009.05.27 Open config window on Networks
	void showAppearanceConfig();	//VOXOX - SEMR - 2009.05.27 Open appearance window of config

	//VOXOX CHANGE CJC OPEN TABS
	void showContactsTab();

	void showKeyTab();
	
	//VOXOX CHANGE by Rolando - 2009.09.22 
	void showApps();

	void showHistory();
	//VOXOX - CJC - 2009.06.03 
	void showWizard();

	void showStore();//VOXOX CHANGE by Rolando - 2009.09.22 

	void showContactManager();// VOXOX -ASV- 07-09-2009
	void showContactManager( QString contactId );	//VOXOX - JRT - 2009.06.29 

	void showWengoForum();

	void showAbout();
	void checkForUpdates() ;		//VOXOX - JRT - 2009.10.05 

	//VOXOX - SEMR - 2009.05.19 New option Help Menu	
	void showReportBug();
	//VOXOX - SEMR - 2009.05.19 New option Help Menu
	void showSuggestFeature();
	//VOXOX - SEMR - 2009.05.19 New option Help Menu
	void showBillingFeature();

	void showRatesPage();//VOXOX - SEMR - 2009.07.08 open rates pages of voxox

	void showDeveloperPlatform();//VOXOX - SEMR - 2009.07.08 open rates pages of voxox

	void sendSms();

	void showWengoFAQ();

	void showWengoServices();

//	void showHideContactGroups();	//VOXOX - JRT - 2009.05.05 

	void showHideOfflineContacts();	//VOXOX - JRT - 2009.05.05 

	void createConferenceCall();

	void clearHistoryOutgoingCalls();

	void clearHistoryIncomingCalls();

	void clearHistoryMissedCalls();

	void clearHistoryRejectedCalls();

	void clearHistoryChatSessions();

	void clearHistorySms();

	void clearHistoryAll();

	void searchWengoContact();

//	void logOff();		//VOXOX - JRT - 2009.07.12 - Moved to QtWengoPhone.

	void closeTab();//VOXOX - SEMR - 2009.06.01 Close tab from menu if exists

	void acceptCall();

	void holdResumeCall();

	void hangUpCall();

	void showChatWindow();

	void showFileTransferWindow();

	void toggleVideo();

	void slotConfigChanged(QString key);
	
	void userProfileIsInitialized();

	void enableMenuBar();

	void updateMenuActions();

	void tryingToConnect();
	//VOXOX CHANGE CJC SUPPORT ACCOUNT ADDING
	void addIMAccount(QAction * action);

	void addContactGroup();

	//VOXOX CHANGE by ASV 07-03-2009: We are not using QtContactMenu2 anymore
	//void replaceContactMenu();					//VOXOX - JRT - 2009.05.05 

	//VOXOX - JRT - 2009.07.03 - Then you won't need this either.
//	void handleContactMenu( QAction* action );	//VOXOX - JRT - 2009.05.03

	//VOXOX - SEMR - 2009.06.24 SORT OPTION FUNCTION
	void sortContactByGroup();
	void sortContactByNetwork();
	void sortContactByNone();
	//VOXOX - SEMR - 2009.06.24 GROUP SORT SLOTS
	void sortGroupByAlphabetically();
	void sortGroupByPresence();
	void sortGroupByRandom();

	void deleteSelection(); //VOXOX - SEMR - 2009.06.25 Delete selected user
	void editSelection(); //VOXOX - SEMR - 2009.06.25 Edit selected user
	
	void redialKeypad();
	void conferenceKeypad();
	void voicemailKeypad();

	// VOXOX CHANGE by ASV 06-05-2009: added to implement the functionality of the Edit Menu
	void cutSlot();
	void copySlot();
	void pasteSlot();
	void selectAllSlot();
	void sendEmail();//VOXOX - SEMR - 2009.07.02 send email function
	void sendSMS();
	
	void showMainWindow(); //VOXOX -ASV- 07-12-2009: activates the Main Window after clicking the VoxOx menu from Windows menu. Only Mac
	
private Q_SLOTS:
	/**
	 * the userProfile has been deleted
	 */
	void userProfileDeleted();
	void handleMinimize();// VOXOX -ASV- 07-09-2009: added to implement the Window menu for Mac
	void handleZoom();// VOXOX -ASV- 07-09-2009: added to implement the Window menu for Mac
	// VOXOX -ASV- 07-09-2009: the close window option from the menu should work with the active window
	// and not just the main window
	void handleCloseWindow();	
	void replayHistoryCurrentItem(); //VOXOX - SEMR - 2009.07.10 
	void removeHistoryCurrentItem(); //VOXOX - SEMR - 2009.07.10 
	void showLogHistoryCurrentItem(); //VOXOX - SEMR - 2009.07.10 

	void updateClipboardActions(QWidget *, QWidget *); //VOXOX -ASV- 07-22-2009

	void handleEditMenu(); //VOXOX -ASV- 08-17-2009
	
private:
	Config& getConfig();				//VOXOX - JRT - 2009.05.03 - Add convenience method.


	void updateActionsAccordingToUserStatus();

	void updateGroupsStausMenu(); //VOXOX - SEMR - 2009.06.24 update group selected

	void updateContactsStausMenu(); //VOXOX - SEMR - 2009.06.24 update sort option selected

	void setChecksStausMenu(); //VOXOX - SEMR - 2009.06.24 update the marks for the menus in case of sort/group from toolbar

	/**
	 * Ugly code to remove.
	 */
	int findFirstCallTab();

	void addNetworkMenu();//VOXOX - SEMR - 2009.05.20 new networks menu for Actions Menu

	void updateShowHideContactGroupsAction();
	//VOXOX CHANGE Rolando 03-20-09
	void updateShowHideProfileBarAction();

	void updateShowHideCallBarAction();

	void updateShowHideOfflineContactsAction();

	void updateToggleVideoAction();

	void configChangedEventHandler(const std::string & key);

	QWidget *getDefaultParent();// VOXOX CHANGE by ASV 07-08-2009: added to clean up the code a little
	
	/**
	 * If page exists, connect action to receiver::slot, otherwise, remove
	 * action from any widget it's connected to (menu, toolbar...)
	 */
	void connectOrHide(WsUrl::Page page, QAction* action, QObject* receiver, const char* slot);


	/**
	 * If condition is true, connect action to receiver::slot, otherwise,
	 * remove action from any widget it's connected to (menu, toolbar...)
	 */
	void connectOrHide(bool condition, QAction* action, QObject* receiver, const char* slot);

	QtWengoPhone & _qtWengoPhone;

	CWengoPhone & _cWengoPhone;

	Ui::WengoPhoneWindow * _ui;

	bool _userIsLogged;
	
	void createContactsTabGearMenu();
	void createHistoryTabGearMenu();
	void createKeypadTabGearMenu();
	void createAppsTabGearMenu();
	
	//VOXOX - SEMR - 2009.07.07 Action for gear menus
	QAction * _setting;
	QMenu * _groupContactsby;
	QMenu * _sortContactsby;

	//VOXOX - SEMR - 2009.07.07 CONTACTS TAB
	QMenu * _contactsTabGearMenu;
	QAction * _showOnlineContacts;
	QMenu * _sortContactby;
	QMenu * _gorupContactsby;
	QAction * _sortAlpha;
	QAction * _sortPresence;
	QAction * _sortRandom;
	QAction * _groupGroups;
	QAction * _groupNetwork;
	/*QAction * _groupMostContacted;  WAITING FOR IMPLEMENTATIO
	QAction * _groupRecentlyContact;
	QAction * _groupRecebtlyLoggedIn;*/
	QAction * _groupNone;
	QAction * _manageNetworks;
	QAction * _editSelection;
	QAction * _deleteSelection;

	//VOXOX - SEMR - 2009.07.07 HISTORY TAB
	QMenu * _historyTabGearMenu;
	QMenu * _clearHistoryBy;
	QAction * _outgoingCalls;
	QAction * _incomingCalls;
	QAction * _missedCalls;
	QAction * _rejectedCalls;
	QAction * _chatSessions;
	QAction * _textMessages;
	QAction * _clearAllHistory;
	QAction * _showLog;
	QAction * _restartSelection;
	QAction * _eraseSelection;
	
	//VOXOX - SEMR - 2009.07.07 KEYPAD TAB
	QMenu * _keypadTabGearMenu;
	//QMenu * _recentCalls;
	QAction * _voicemail;
	QAction * _redial;
	QAction * _conferenceCall;
	QAction * _rates;
	QAction * _changeBakcground;
	//QMenu * _soundBoards;
	QAction * _phoneSettings;

	//VOXOX - SEMR - 2009.07.07 APPS TAB
	QMenu * _appsTabGearMenu;
	QAction * _developerPlatform;
	
	//VOXOX -ASV- 08-17-2009
	QTextEdit* _activeTextEdit;
	QLineEdit* _activeLineEdit;	
	QComboBox* _activeComboBox;
	QWebView* _activeWebView;

	QAction *_actionAbout;//VOXOX -ASV- 2009.12.03
	QAction *_actionUpdate;//VOXOX -ASV- 2009.12.03
	
	//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed regardles of the window we are at
	#if defined(OS_MACOSX)
		QShortcut *_closeWindowShortCut;
	#endif
};

#endif	//OWQTTOOLBAR_H
