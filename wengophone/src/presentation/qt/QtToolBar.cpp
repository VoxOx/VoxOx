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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtToolBar.h"

#include "ui_WengoPhoneWindow.h"
#include <cutil/global.h>

#include "QtWengoPhone.h"
#include "QtVoxWindowManager.h"
#include "QtAbout.h"
#include "QtNoWengoAlert.h"
#include "dialpad/QtDialpad.h"//VOXOX CHANGE by Rolando 04-29-09, now QtDialpad is in dialpad folder
#include "profile/QtProfileDetails.h"
#include "profile/QtUserProfileHandler.h"
#include "config/QtWengoConfigDialog.h"
#include "wizard/QtWizardDialog.h"//VOXOX - CJC - 2009.06.01 
#include "webservices/sms/QtSms.h"
#include "phonecall/QtContactCallListWidget.h"
#include "phonecall/QtPhoneCall.h"
#include "contactlist/QtContactList.h"
#include "contactlist/QtEnumSortOption.h"
#include "filetransfer/QtFileTransfer.h"
#include "filetransfer/QtFileTransferWidget.h"
#include "conference/QtConferenceCallWidget.h"

//#include "chat/QtContactProfileWidget.h"	//VOXOX - JRT - 2009.08.31 - not needed


#include <presentation/qt/chat/QtUMWindow.h>//VOXOX CHANGE by Rolando - 2009.05.27 

#include "imcontact/QtFastAddIMContact.h"

#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	#include "webdirectory/QtWebDirectory.h"
#endif

#include <control/CWengoPhone.h>
#include <control/history/CHistory.h>
#include <control/phonecall/CPhoneCall.h>
#include <control/phoneline/CPhoneLine.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/contactlist/ContactProfile.h>
#include <model/phonecall/ConferenceCall.h>
#include <model/account/wengo/WengoAccount.h>
#include <model/profile/UserProfile.h>
#include <model/history/History.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/config/EnumToolBarMode.h>

#include <util/Logger.h>
#include <presentation/qt/imaccount/QtIMAccountSettings.h>
#include <presentation/qt/history/QtHistoryWidget.h> //VOXOX - SEMR - 2009.07.10 
#include <presentation/qt/contactlist/QtRenameGroup.h>
#include <presentation/qt/dialpad/QtDialpad.h>
#include <presentation/qt/contactlist/QtContactManager.h>
#include <presentation/qt/callbar/customCallBar/QtVoxOxCallBarFrame.h>
#include <presentation/qt/profilebar/QtIMProfileWidget.h> //VOXOX CHANGE by ASV 07-02-2009
#include <presentation/qt/chat/QtChatHistoryWidget.h> //VOXOX -ASV- 07-23-2009

#include <qtutil/WidgetBackgroundImage.h>
#include <qtutil/SafeConnect.h>
#include <util/SafeDelete.h>
#include <qtutil/Object.h>
#include <qtutil/WidgetFactory.h>
#include <qtutil/VoxOxToolTipLineEdit.h>//VOXOX CHANGE by ASV 07-02-2009

//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>
#include <presentation/qt/messagebox/QtVoxEmailMessageBox.h>
#include <presentation/qt/messagebox/QtVoxTextMessageBox.h>
#include <presentation/qt/messagebox/QtVoxPhoneNumberMessageBox.h>
//#include <QtGui/QToolButton>

static const int TOOLBAR_BUTTON_MINIMUM_WIDTH = 60;

void QtToolBar::connectOrHide(bool condition, QAction* action, QObject* receiver, const char* slot) {
	if (condition) {
		SAFE_CONNECT_RECEIVER(action, SIGNAL(triggered()), receiver, slot);
	} else {
#if QT_VERSION >= 0x040200
		Q_FOREACH(QWidget* widget, action->associatedWidgets()) {
			widget->removeAction(action);
		}
#else
		Q_FOREACH(QMenu* menu, _ui->menuBar->findChildren<QMenu*>()) {
			menu->removeAction(action);
		}
		_ui->toolBar->removeAction(action);
#endif
	}
}


void QtToolBar::connectOrHide(WsUrl::Page page, QAction* action, QObject* receiver, const char* slot) {
	connectOrHide(WsUrl::hasPage(page), action, receiver, slot);
}


QtToolBar::QtToolBar(QtWengoPhone & qtWengoPhone, Ui::WengoPhoneWindow * qtWengoPhoneUi, QWidget * parent)
	: QObject(parent),
	_qtWengoPhone(qtWengoPhone),
	_cWengoPhone(_qtWengoPhone.getCWengoPhone()) {

	_ui = qtWengoPhoneUi;

	Config& config		 = getConfig();	//ConfigManager::getInstance().getCurrentConfig(); //VOXOX - JRT - 2009.05.03 
	bool canSendSms		 = config.getSmsFeatureEnabled();
	bool canDoConference = config.getAudioConferenceFeatureEnabled();

	SAFE_CONNECT(&_qtWengoPhone, SIGNAL(userProfileDeleted()), SLOT(userProfileDeleted()));

	// actions initialization
	_userIsLogged = false;
	updateActionsAccordingToUserStatus();
	
	//ASV 2009.11.03: we can't do this here because it crashes on Mac since the main UI is not loaded
	//updateShowHideProfileBarAction();//VOXOX CHANGE Rolando 03-20-09 VOXOX CHANGE 
	updateShowHideCallBarAction();//VOXOX CHANGE Rolando 03-25-09

	//VOXOX - SEMR - 2009.05.21 menuWengo actions
//	SAFE_CONNECT(_ui->actionLogOff, SIGNAL(triggered()), SLOT(logOff()));		//VOXOX - JRT - 2009.07.12
	//SAFE_CONNECT(_ui->actionCloseTab, SIGNAL(triggered()), SLOT(closeTab()));
	SAFE_CONNECT_RECEIVER(_ui->actionLogOff, SIGNAL(triggered()), &_qtWengoPhone, SLOT(logOff()					  ));	//VOXOX - JRT - 2009.07.12 
//	SAFE_CONNECT_RECEIVER(_ui->actionQuit,	 SIGNAL(triggered()), &_qtWengoPhone, SLOT(prepareToExitApplication() ));
	SAFE_CONNECT_RECEIVER(_ui->actionQuit,	 SIGNAL(triggered()), &_qtWengoPhone, SLOT(exitApplication() ));			//VOXOX - JRT - 2009.07.13 
	

	//VOXOX - SEMR - 2009.05.22 actionsMenu
	SAFE_CONNECT(_ui->actionAddContact, SIGNAL(triggered()), SLOT(addContact()));
	//Add Group
	addNetworkMenu();
	SAFE_CONNECT(_ui->actionManageNetworks, SIGNAL(triggered()), SLOT(showNetworkConfig()));
	SAFE_CONNECT(_ui->actionEditMyProfile, SIGNAL(triggered()), SLOT(editMyProfile()));
	SAFE_CONNECT(_ui->actionPlaceCall, SIGNAL(triggered()), SLOT(showKeyTab()));
	connectOrHide(canDoConference, _ui->actionCreateConferenceCall, this, SLOT(createConferenceCall()));
	//Send Instant Message
	_ui->actionGroupChat->setDisabled(true);
	SAFE_CONNECT(_ui->actionSendSms, SIGNAL(triggered()), SLOT(sendSMS()));
	SAFE_CONNECT(_ui->actionSendEmail, SIGNAL(triggered()), SLOT(sendEmail()));
	//Send Group Email
	//Send File
	SAFE_CONNECT(_ui->actionSendFax, SIGNAL(triggered()), SLOT(showKeyTab()));

	//VOXOX - SEMR - 2009.05.21 menuContacts
	connectOrHide(WsUrl::Account, _ui->actionShowWengoAccount, this, SLOT(showWengoAccount()) );
	SAFE_CONNECT(_ui->actionProfileBar, SIGNAL(triggered()), SLOT(profileBarActionTriggered()));//VOXOX CHANGE Rolando 03-20-09
	SAFE_CONNECT(_ui->actionCallBar, SIGNAL(triggered()), SLOT(callBarActionTriggered()));//VOXOX CHANGE Rolando 03-25-09
	SAFE_CONNECT(_ui->actionShowAppearance, SIGNAL(triggered()), SLOT(showAppearanceConfig()));//VOXOX CHANGE Rolando 03-25-09
	connectOrHide(WsUrl::CallOut, _ui->actionWengoServices, this, SLOT(showWengoServices()) );

	//VOXOX - SEMR - 2009.05.22 toolsMenu
	SAFE_CONNECT(_ui->actionShowSetupWizard, SIGNAL(triggered()), SLOT(showWizard()));
	//Store	
	SAFE_CONNECT(_ui->actionShowStore, SIGNAL(triggered()), SLOT(showStore()));//VOXOX CHANGE by Rolando - 2009.09.22 
	SAFE_CONNECT(_ui->actionToggleVideo, SIGNAL(triggered()), SLOT(toggleVideo()));
	//menuClearHistory
	SAFE_CONNECT(_ui->actionClearOutgoingCalls, SIGNAL(triggered()), SLOT(clearHistoryOutgoingCalls()));
	SAFE_CONNECT(_ui->actionClearIncomingCalls, SIGNAL(triggered()), SLOT(clearHistoryIncomingCalls()));
	SAFE_CONNECT(_ui->actionClearMissedCalls, SIGNAL(triggered()), SLOT(clearHistoryMissedCalls()));
	SAFE_CONNECT(_ui->actionClearRejectedCalls, SIGNAL(triggered()), SLOT(clearHistoryRejectedCalls()));
	SAFE_CONNECT(_ui->actionClearChatSessions, SIGNAL(triggered()), SLOT(clearHistoryChatSessions()));
	connectOrHide(canSendSms, _ui->actionClearSms, this, SLOT(clearHistorySms()));
	SAFE_CONNECT(_ui->actionClearAll, SIGNAL(triggered()), SLOT(clearHistoryAll()));
	SAFE_CONNECT(_ui->actionShowConfig, SIGNAL(triggered()), SLOT(showConfig()));

	//menuView
	//replaceContactMenu();		//VOXOX - JRT - 2009.05.05 
	SAFE_CONNECT(_ui->actionShowOnlineContacts, SIGNAL(triggered()), SLOT(showHideOfflineContacts()));

	SAFE_CONNECT(_ui->actionAlphabetically, SIGNAL(triggered()), SLOT(sortGroupByAlphabetically()));
	SAFE_CONNECT(_ui->actionPresence, SIGNAL(triggered()), SLOT(sortGroupByPresence()));
	SAFE_CONNECT(_ui->actionRandom, SIGNAL(triggered()), SLOT(sortGroupByRandom()));

	SAFE_CONNECT(_ui->actionGroups, SIGNAL(triggered()), SLOT(sortContactByGroup()));
	SAFE_CONNECT(_ui->actionNetworks, SIGNAL(triggered()), SLOT(sortContactByNetwork()));
	SAFE_CONNECT(_ui->actionNone, SIGNAL(triggered()), SLOT(sortContactByNone()));

	
	_ui->actionShowCallBar->setChecked(config.getShowCallBar());
	SAFE_CONNECT(_ui->actionShowCallBar, SIGNAL(triggered()), SLOT(callBarActionTriggered()));
	//ASV 2009.11.03: we can't do this here because it crashes on Mac since the main UI is not loaded
	//updateShowHideProfileBarAction();
	SAFE_CONNECT(_ui->actionShowProfileBar, SIGNAL(triggered()), SLOT(profileBarActionTriggered()));
	/*
	profileBarActionTriggered()
	SAFE_CONNECT(_ui->actionClearChatSessions, SIGNAL(triggered()), SLOT(clearHistoryChatSessions()));
	*/

	//menuActions
	connectOrHide(canSendSms, _ui->actionSendSms, this, SLOT(sendSms()));
	SAFE_CONNECT(_ui->actionAcceptCall,     SIGNAL(triggered()), SLOT(acceptCall()));
	SAFE_CONNECT(_ui->actionHangUpCall,     SIGNAL(triggered()), SLOT(hangUpCall()));
	SAFE_CONNECT(_ui->actionHoldResumeCall, SIGNAL(triggered()), SLOT(holdResumeCall()));

	//menuHelp
	connectOrHide(WsUrl::Forum, _ui->actionShowWengoForum, this, SLOT(showWengoForum()));
	connectOrHide(WsUrl::FAQ,   _ui->actionWengoFAQ,       this, SLOT(showWengoFAQ())  );

	SAFE_CONNECT(_ui->actionReportBug,		 SIGNAL(triggered()), SLOT(showReportBug()));
	SAFE_CONNECT(_ui->actionSuggetsFeature,  SIGNAL(triggered()), SLOT(showSuggestFeature()));
	SAFE_CONNECT(_ui->actionBillingQuestion, SIGNAL(triggered()), SLOT(showBillingFeature()));
	SAFE_CONNECT(_ui->actionShowAbout,		 SIGNAL(triggered()), SLOT(showAbout()));
	SAFE_CONNECT(_ui->actionCheckForUpdate,  SIGNAL(triggered()), SLOT(checkForUpdates()));
	
	config.valueChangedEvent += boost::bind(&QtToolBar::configChangedEventHandler, this, _1);
	SAFE_CONNECT(this, SIGNAL(configChanged(QString)), SLOT(slotConfigChanged(QString)) );

	SAFE_CONNECT(_ui->menuActions, SIGNAL(aboutToShow()), SLOT(updateMenuActions()));

#ifndef CUSTOM_ACCOUNT
	_ui->actionSearchWengoContact->setVisible(false);
#endif

//#ifdef DISABLE_SMS
//	_ui->actionClearSms->setVisible(false);
	//_ui->actionSendSms->setVisible(false);
//#endif

	fixMacOSXMenus();
		
	SAFE_CONNECT(_ui->actionCloseWindow, SIGNAL(triggered()), SLOT(handleCloseWindow())); //VOXOX -ASV- 07-13-2009
		
    //VOXOX - SEMR - 2009.07.07 Gear MENU ACTIONS
	_setting = new QAction(tr("Settings"), this);
	SAFE_CONNECT(_setting, SIGNAL(triggered()), SLOT(showConfig()));

	SAFE_CONNECT(static_cast<QApplication *>(QCoreApplication::instance()),
			SIGNAL(focusChanged (QWidget *, QWidget *)), SLOT(updateClipboardActions(QWidget *, QWidget *)));
		
	createContactsTabGearMenu();
	createHistoryTabGearMenu();
	createKeypadTabGearMenu();
	createAppsTabGearMenu();
		
	_activeTextEdit = NULL;	//VOXOX -ASV- 08-17-2009
	_activeLineEdit = NULL; //VOXOX -ASV- 08-17-2009
	_activeComboBox = NULL; //VOXOX -ASV- 08-17-2009
}

QtToolBar::~QtToolBar() {
//VOXOX CHANGE by ASV 07-03-2009: We are using the actions from the UI not these	
/*	OWSAFE_DELETE(_sortContactByGroup);
	OWSAFE_DELETE(_sortContactByNetwork);
	OWSAFE_DELETE(_sortContactByNone);
	OWSAFE_DELETE(_sortGroupAlphabetically);
	OWSAFE_DELETE(_sortGroupPresence);
	OWSAFE_DELETE(_sortGroupRandom);
	OWSAFE_DELETE(_groupContactsBy);
	OWSAFE_DELETE(_sortContactsBy);
*/
	//OWSAFE_DELETE(_contactProfileWidget);	//VOXOX - JRT - 2009.06.29 
	#if defined(OS_MACOSX)
		OWSAFE_DELETE(_closeWindowShortCut);
	#endif
}
void QtToolBar::createContactsTabGearMenu(){
	_contactsTabGearMenu = new QMenu(_qtWengoPhone.getWidget());

	_showOnlineContacts= new QAction(tr("Show Contacts Online"), this);
	SAFE_CONNECT(_showOnlineContacts, SIGNAL(triggered()), SLOT(showHideOfflineContacts()));
	_contactsTabGearMenu->addAction(_showOnlineContacts);

	_sortContactsby = new QMenu("Sort Contacts By", _contactsTabGearMenu);

	_sortAlpha = new QAction(tr("Alphabetically"), this);
	_sortAlpha->setCheckable(true);
	_sortContactsby->addAction(_sortAlpha);
	SAFE_CONNECT(_sortAlpha, SIGNAL(triggered()), SLOT(sortGroupByAlphabetically()));

	_sortPresence = new QAction(tr("Presence"), this);
	_sortPresence->setCheckable(true);
	_sortContactsby->addAction(_sortPresence);
	SAFE_CONNECT(_sortPresence, SIGNAL(triggered()), SLOT(sortGroupByPresence()));

	_sortRandom = new QAction(tr("Random"), this);
	_sortRandom->setCheckable(true);
	_sortContactsby->addAction(_sortRandom);
	SAFE_CONNECT(_sortRandom, SIGNAL(triggered()), SLOT(sortGroupByRandom()));

	_contactsTabGearMenu->addMenu(_sortContactsby);

	_groupContactsby = new QMenu("Group Contacts By", _contactsTabGearMenu);

	_groupGroups = new QAction(tr("Groups"), this);
	_groupGroups->setCheckable(true);
	_groupContactsby->addAction(_groupGroups);
	SAFE_CONNECT(_groupGroups, SIGNAL(triggered()), SLOT(sortContactByGroup()));

	_groupNetwork = new QAction(tr("Networks"), this);
	_groupNetwork->setCheckable(true);
	_groupContactsby->addAction(_groupNetwork);
	SAFE_CONNECT(_groupNetwork, SIGNAL(triggered()), SLOT(sortContactByNetwork()));

	_groupNone = new QAction(tr("None"), this);
	_groupNone->setCheckable(true);
	_groupContactsby->addAction(_groupNone);
	SAFE_CONNECT(_groupNone, SIGNAL(triggered()), SLOT(sortContactByNone()));

	_contactsTabGearMenu->addMenu(_groupContactsby);

	_manageNetworks = new QAction(tr("Manage Networks"), this);
	_contactsTabGearMenu->addAction(_manageNetworks);
	SAFE_CONNECT(_manageNetworks, SIGNAL(triggered()), SLOT(showNetworkConfig()));

	_editSelection = new QAction(tr("Edit Selection"), this);
	_contactsTabGearMenu->addAction(_editSelection);
	SAFE_CONNECT(_editSelection, SIGNAL(triggered()), SLOT(editSelection()));

	_deleteSelection = new QAction(tr("Delete Selection"), this);
	_contactsTabGearMenu->addAction(_deleteSelection);
	SAFE_CONNECT(_deleteSelection, SIGNAL(triggered()), SLOT(deleteSelection()));
	_contactsTabGearMenu->addSeparator();
	_contactsTabGearMenu->addAction(_setting);
}
void QtToolBar::createHistoryTabGearMenu(){
	_historyTabGearMenu = new QMenu(_qtWengoPhone.getWidget());

	_clearHistoryBy = new QMenu("Clear History", _historyTabGearMenu);

	_outgoingCalls = new QAction(tr("Outgoing Calls"), this);
	_clearHistoryBy->addAction(_outgoingCalls);
	_incomingCalls = new QAction(tr("Incoming Calls"), this);
	_clearHistoryBy->addAction(_incomingCalls);
	_missedCalls = new QAction(tr("Missed Calls"), this);
	_clearHistoryBy->addAction(_missedCalls);
	_rejectedCalls = new QAction(tr("Rejected Calls"), this);
	_clearHistoryBy->addAction(_rejectedCalls);
	_chatSessions = new QAction(tr("Chat Sessions"), this);
	_clearHistoryBy->addAction(_chatSessions);
	_textMessages = new QAction(tr("Text Messages"), this);
	_clearHistoryBy->addAction(_textMessages);
	_clearAllHistory = new QAction(tr("Clear All History"), this);
	_clearHistoryBy->addAction(_clearAllHistory);

	_historyTabGearMenu->addMenu(_clearHistoryBy);

	_showLog = new QAction(tr("Show Log..."), this);
	_historyTabGearMenu->addAction(_showLog);
	SAFE_CONNECT(_showLog, SIGNAL(triggered()), SLOT(showLogHistoryCurrentItem()));
	_restartSelection = new QAction(tr("Restart Selection"), this);
	_historyTabGearMenu->addAction(_restartSelection);
	SAFE_CONNECT(_restartSelection, SIGNAL(triggered()), SLOT(replayHistoryCurrentItem()));
	_eraseSelection = new QAction(tr("Erase Selection"), this);
	_historyTabGearMenu->addAction(_eraseSelection);
	SAFE_CONNECT(_eraseSelection, SIGNAL(triggered()), SLOT(removeHistoryCurrentItem()));

	_historyTabGearMenu->addSeparator();
	_historyTabGearMenu->addAction(_setting);
}

void QtToolBar::replayHistoryCurrentItem(){
	QtHistoryWidget * qtHistoryWidget = _qtWengoPhone.getHistoryWidget();
	qtHistoryWidget->replayCurrentItem();
}
void QtToolBar::removeHistoryCurrentItem(){
	QtHistoryWidget * qtHistoryWidget = _qtWengoPhone.getHistoryWidget();
	qtHistoryWidget->removeCurrentItem();
}
void QtToolBar::showLogHistoryCurrentItem(){
	QtHistoryWidget * qtHistoryWidget = _qtWengoPhone.getHistoryWidget();
	qtHistoryWidget->showLogCurrentItem();
}

void QtToolBar::createKeypadTabGearMenu(){
	_keypadTabGearMenu = new QMenu(_qtWengoPhone.getWidget());

	_voicemail = new QAction(tr("Voicemail"), this);
	_keypadTabGearMenu->addAction(_voicemail);
	SAFE_CONNECT(_voicemail, SIGNAL(triggered()), SLOT(voicemailKeypad()));

	_redial = new QAction(tr("Redial"), this);
	_keypadTabGearMenu->addAction(_redial);
	SAFE_CONNECT(_redial, SIGNAL(triggered()), SLOT(redialKeypad()));

	_conferenceCall = new QAction(tr("Conference Call"), this);
	_keypadTabGearMenu->addAction(_conferenceCall);
	SAFE_CONNECT(_conferenceCall, SIGNAL(triggered()), SLOT(conferenceKeypad()));

	_rates = new QAction(tr("Rates"), this);
	_keypadTabGearMenu->addAction(_rates);
	SAFE_CONNECT(_rates, SIGNAL(triggered()), SLOT(showRatesPage()));

	_changeBakcground = new QAction(tr("Change Background"), this);
	_keypadTabGearMenu->addAction(_changeBakcground);

	_phoneSettings = new QAction(tr("Phone Settings"), this);
	_keypadTabGearMenu->addAction(_phoneSettings);
	SAFE_CONNECT(_phoneSettings, SIGNAL(triggered()), SLOT(showPhoneConfig()));

	_keypadTabGearMenu->addSeparator();
	_keypadTabGearMenu->addAction(_setting);
}
void QtToolBar::createAppsTabGearMenu(){
	_appsTabGearMenu = new QMenu(_qtWengoPhone.getWidget());
	_developerPlatform = new QAction(tr("Developer Platform"), this);
	SAFE_CONNECT(_developerPlatform, SIGNAL(triggered()), SLOT(showDeveloperPlatform()));

	_appsTabGearMenu->addAction(_developerPlatform);
	_appsTabGearMenu->addSeparator();
	_appsTabGearMenu->addAction(_setting);
	setChecksStausMenu();
}
//VOXOX - JRT - 2009.05.03 
Config& QtToolBar::getConfig()
{
	return ConfigManager::getInstance().getCurrentConfig();
}
//End VoxOx

QWidget * QtToolBar::getWidget() const {
	return _qtWengoPhone.getWidget();
}
	
	
	
	
QMenu * QtToolBar::getContactsTabMenu(){
	if(_contactsTabGearMenu->isEmpty()){
		createContactsTabGearMenu();
	}
	return _contactsTabGearMenu;
}

QMenu * QtToolBar::getKeypadTabMenu(){
	if(_keypadTabGearMenu->isEmpty()){
		createKeypadTabGearMenu();
	}
	return _keypadTabGearMenu;
}

QMenu * QtToolBar::getHistoryTabMenu(){
	if(_historyTabGearMenu->isEmpty()){
		createHistoryTabGearMenu();
	}
	return _historyTabGearMenu;
}

QMenu * QtToolBar::getAppsTabMenu(){
	if(_appsTabGearMenu->isEmpty()){
		createAppsTabGearMenu();
	}
	return _appsTabGearMenu;
}

QMenu * QtToolBar::getStatusBarSettingsMenu(){
	QMenu * statusGearMenu = new QMenu(_qtWengoPhone.getWidget());
	setChecksStausMenu();	
	
	QAction * manageNetworks =  new QAction("Manage Networks...", this);
	SAFE_CONNECT(manageNetworks, SIGNAL(triggered()), SLOT(showNetworkConfig()));
	statusGearMenu->addAction(manageNetworks);

	statusGearMenu->addMenu(_ui->menuSortContactsBy);
	statusGearMenu->addMenu(_ui->menuGroupContactsBy);

	QAction * settings =  new QAction("Settings", this);
	SAFE_CONNECT(settings, SIGNAL(triggered()), SLOT(showConfig()));	

	return statusGearMenu;
	
}


void QtToolBar::sendEmail(){
	QtVoxEmailMessageBox box(_qtWengoPhone.getWidget());
	box.setWindowTitle("VoxOx - Send Email");
	box.setDialogText(tr("Please enter the email address"));

	int ret = box.exec();
	if(ret == QMessageBox::RejectRole)  {
		QString email = box.getEmail();
		if(email!=""){
			_qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile()->startIMToEmail(email.toStdString());
		}
	}
}

void QtToolBar::sendSMS(){
	QtVoxPhoneNumberMessageBox box(0);

	box.setTitle(tr("VoxOx - Send SMS"));
	box.setDialogText(tr("Please enter the mobile phone number"));
	box.setNumberTypeVisible(false);
	
	if(box.exec() == QDialog::Accepted){
		QString number = box.getNumber();
		if(number!=""){
			_qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile()->startIMToSMS(number.toStdString());
		}
	}
}


//VOXOX - SEMR - 2009.06.26 Group Contacts by Group
//VOXOX - JRT - 2009.10.08 - Two problems here:
//				1) Most of this code is redundant so it should be factored
//				2) getting the qtContactList does not ensure it stays valid.  Move that code to QtWengoPhone which OWNS qtContactList
void QtToolBar::sortContactByGroup()
{
	_qtWengoPhone.UpdateViewGroupBy( QtEnumSortOption::GroupByUserGroup );
	setChecksStausMenu();
}

void QtToolBar::sortContactByNetwork()
{
	_qtWengoPhone.UpdateViewGroupBy( QtEnumSortOption::GroupByNetwork );
	setChecksStausMenu();
}

void QtToolBar::sortContactByNone()
{
	_qtWengoPhone.UpdateViewGroupBy( QtEnumSortOption::GroupByNone );
	setChecksStausMenu();
}

void QtToolBar::sortGroupByAlphabetically()
{
	_qtWengoPhone.UpdateViewSortBy( QtEnumSortOption::SortAlpha );
	setChecksStausMenu();
}
void QtToolBar::sortGroupByPresence()
{
	_qtWengoPhone.UpdateViewSortBy( QtEnumSortOption::SortPresence );
	setChecksStausMenu();
}

void QtToolBar::sortGroupByRandom()
{
	_qtWengoPhone.UpdateViewSortBy( QtEnumSortOption::SortRandom );
	setChecksStausMenu();
}
//VOXOX - SEMR - 2009.06.26 set check marks when its needed
//VOXOX CHANGE by ASV 07-03-2009: changed the actions to the ones in the UI
void QtToolBar::setChecksStausMenu(){
	int contactSortKey = getConfig().getContactSort(); //GENERAL_CONTACT_SORT_KEY
	int contactGroupingKey = getConfig().getContactGrouping();//GENERAL_CONTACT_GROUPING_KEY

	switch(contactGroupingKey){
		case QtEnumSortOption::GroupByUserGroup:
			_ui->actionGroups->setChecked( true );
			_groupGroups->setChecked( true );
		break;

		case QtEnumSortOption::GroupByNetwork:
			_ui->actionNetworks->setChecked( true );
			_groupNetwork->setChecked( true );
		break;

		/*case QtEnumSortOption::GroupByMostComm:
			_sortContactByNetwork->setChecked( true );;
			break;*/

		case QtEnumSortOption::GroupByNone:
			_ui->actionNone->setChecked( true );
			_groupNone->setChecked( true );
		break;

		default:
			assert(false);		//New type?
			_ui->actionGroups->setChecked( true );
			_groupGroups->setChecked( true );
		break;
	}

	switch(contactSortKey  ){
		case QtEnumSortOption::SortPresence:
			_ui->actionPresence->setChecked( true );;
			_sortPresence->setChecked( true );
		break;

		case QtEnumSortOption::SortRandom:
			_ui->actionRandom->setChecked( true );;
			_sortRandom->setChecked( true );
		break;

		case QtEnumSortOption::SortAlpha:
		default:
			_ui->actionAlphabetically->setChecked( true );
			_sortAlpha->setChecked( true );
		break;
	}
	updateContactsStausMenu();
	updateGroupsStausMenu();
}
//VOXOX - SEMR - 2009.06.26 change the mark for each selection	
void QtToolBar::updateGroupsStausMenu(){
	int contactGroupingKey = getConfig().getContactGrouping();//GENERAL_CONTACT_GROUPING_KEY
	switch(contactGroupingKey){
		case QtEnumSortOption::GroupByUserGroup:
			_ui->actionNetworks->setChecked(false);
			_groupNetwork->setChecked( false );
			_ui->actionNone->setChecked( false );
			_groupNone->setChecked( false );
		break;

		case QtEnumSortOption::GroupByNetwork:
			_ui->actionGroups->setChecked( false );
			_groupGroups->setChecked( false );
			_ui->actionNone->setChecked( false );
			_groupNone->setChecked( false );
			
		break;

		/*case QtEnumSortOption::GroupByMostComm:
			_sortContactByNetwork->setChecked( true );;
			break;*/

		case QtEnumSortOption::GroupByNone:
			_ui->actionNetworks->setChecked( false );
			_groupNetwork->setChecked( false );
			_ui->actionGroups->setChecked( false );
			_groupGroups->setChecked( false );
		break;

		default:
			assert(false);		//New type?
			_ui->actionGroups->setChecked( true );
		break;
	}

}
//VOXOX - SEMR - 2009.06.26 change the mark for each selection	
void QtToolBar::updateContactsStausMenu(){
	int contactSortKey = getConfig().getContactSort(); //GENERAL_CONTACT_SORT_KEY
	switch(contactSortKey  ){
		case QtEnumSortOption::SortPresence:
			_ui->actionAlphabetically->setChecked( false );
			_sortAlpha->setChecked( false );
			_ui->actionRandom->setChecked( false );	
			_sortRandom->setChecked( false );
		break;

		case QtEnumSortOption::SortRandom:
			_ui->actionAlphabetically->setChecked( false );
			_sortAlpha->setChecked( false );
			_ui->actionPresence->setChecked( false );
			_sortPresence->setChecked( false );
		break;

		case QtEnumSortOption::SortAlpha:
		default:
			_ui->actionPresence->setChecked( false );
			_sortPresence->setChecked( false );
			_ui->actionRandom->setChecked( false );
			_sortRandom->setChecked( false );
		break;
	}
}

//VOXOX CHANGE by ASV 06-29-2009
void QtToolBar::fixMacOSXMenus() {
#if defined(OS_MACOSX)	
	// VOXOX CHANGE by ASV 05-20-2009: Hack for fixing the settings and about menu not active on the Mac.
	_ui->actionShowConfig->setText("Preferences");
	_ui->actionShowAbout->setText("About");

	// Assign shortcuts specific to the Mac version
	_ui->actionLogOff->setShortcut(QKeySequence(tr("CTRL+Alt+Q")));

	// VOXOX CHANGE by ASV 06-29-2009: added to implement the close tab functionality
	// The Close Tab action should only be available when a chat tab can be closed
	//_ui->actionCloseTab->setEnabled(false);
	
	//VOXOX -ASV- 07-08-2009: set the hot keys for the edit menu actions
	_ui->actionCut->setShortcuts(QKeySequence::Cut);
	_ui->actionCopy->setShortcuts(QKeySequence::Copy);
	_ui->actionPaste->setShortcuts(QKeySequence::Paste);
	_ui->actionSelect_All->setShortcuts(QKeySequence::SelectAll);
	
	// VOXOX CHANGE by ASV 06-05-2009: added to implement the functionality of the Edit Menu
	SAFE_CONNECT(_ui->actionCut, SIGNAL(triggered()), SLOT(cutSlot()));
	SAFE_CONNECT(_ui->actionCopy, SIGNAL(triggered()), SLOT(copySlot()));
	SAFE_CONNECT(_ui->actionPaste, SIGNAL(triggered()), SLOT(pasteSlot()));
	SAFE_CONNECT(_ui->actionSelect_All, SIGNAL(triggered()), SLOT(selectAllSlot()));
	
	// VOXOX -ASV- 07-09-2009: added to implement the Window menu for Mac
	SAFE_CONNECT(_ui->actionMinimize, SIGNAL(triggered()), SLOT(handleMinimize()));
	SAFE_CONNECT(_ui->actionZoom, SIGNAL(triggered()), SLOT(handleZoom()));
	SAFE_CONNECT(_ui->actionContact_Manager, SIGNAL(triggered()), SLOT(showContactManager()));	
	
	SAFE_CONNECT(_ui->actionVoxOx, SIGNAL(triggered()), SLOT(showMainWindow()));

	//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed regardles of the window we are at
	_closeWindowShortCut = new QShortcut(QKeySequence("Ctrl+Shift+W"), getWidget());
	SAFE_CONNECT(_closeWindowShortCut, SIGNAL(activated()), SLOT(handleCloseWindow()));

	//VOXOX -ASV- 2009.12.03: Hack!! for setting the Check for Updates menu below the about menu
	// on the application menu for Mac. This was the only way I found to do this.
	// These menus are getting worst everyday! Qt doesn't seems to make it easy for Mac.
	// I think we should forget about the .ui and code all the menus and actions by hand.
	_actionAbout = new QAction(tr("About"), this);
	SAFE_CONNECT(_actionAbout, SIGNAL(triggered()), SLOT(showAbout()));
		
	_actionUpdate = new QAction(tr("Check for Updates..."), this);
	_actionUpdate->setMenuRole(QAction::ApplicationSpecificRole);
	SAFE_CONNECT(_actionUpdate, SIGNAL(triggered()), SLOT(checkForUpdates()));
	
	_ui->menuHelp->removeAction(_ui->actionShowAbout);
	_ui->menuHelp->removeAction(_ui->actionCheckForUpdate);	
	_ui->menuHelp->addAction(_actionAbout);
	_ui->menuHelp->addAction(_actionUpdate);
	// end Hack
#endif
	
#if defined OS_WINDOWS
	// Assign shortcuts specific to the Windows version
	_ui->actionLogOff->setShortcut(QKeySequence(tr("CTRL+O")));

	_ui->menuBar->removeAction( _ui->menuEdit->menuAction()); // VOXOX CHANGE by ASV 07-08-2009: this menu should not appear under Windows
	_ui->menuBar->removeAction( _ui->menuWindow->menuAction()); // VOXOX CHANGE by ASV 07-08-2009: this menu should not appear under Windows	
#endif

}
void QtToolBar::redialKeypad(){
	QtDialpad * dialpad = _qtWengoPhone.getQtDialpad();
	dialpad->redial();
}
void QtToolBar::conferenceKeypad(){
	QtDialpad * dialpad = _qtWengoPhone.getQtDialpad();
	dialpad->conferenceButtonClicked();
}

void QtToolBar::voicemailKeypad(){
	QtDialpad * dialpad = _qtWengoPhone.getQtDialpad();
	dialpad->voiceMailButtonClicked();
}
void QtToolBar::editSelection(){
	QtContactManager * qtContactManager = _qtWengoPhone.getQtContactList()->getContactManager();
	qtContactManager->editCurrentContact();
}

void QtToolBar::deleteSelection(){
	QtContactManager * qtContactManager = _qtWengoPhone.getQtContactList()->getContactManager();
	qtContactManager->deleteCurrentContact();
}

// VOXOX CHANGE -ASV- 07-14-2009
QMenu *QtToolBar::getMenuWengo(){
	return _ui->menuWengo;
}
// VOXOX CHANGE -ASV- 07-14-2009
QMenu *QtToolBar::getMenuEdit(){
	return _ui->menuEdit;
}
// VOXOX CHANGE -ASV- 07-14-2009
QMenu *QtToolBar::getMenuContacts(){
	return _ui->menuContacts;
}
// VOXOX CHANGE -ASV- 07-14-2009
QMenu *QtToolBar::getMenuActions(){
	return _ui->menuActions;
}
// VOXOX CHANGE -ASV- 07-14-2009
QMenu *QtToolBar::getMenuTools(){
	return _ui->menuTools;
}
// VOXOX CHANGE -ASV- 07-14-2009
QMenu *QtToolBar::getMenuWindow(){
	return _ui->menuWindow;
}
// VOXOX CHANGE -ASV- 07-14-2009
QMenu *QtToolBar::getMenuHelp(){
	return _ui->menuHelp;
}

/*// VOXOX CHANGE by ASV 06-05-2009: added to implement the functionality of the Edit Menu
void QtToolBar::cutSlot() {
	if (_qtWengoPhone.getQtCallBar().getCallBarFrame()->getVoxOxToolTipLineEdit()->hasFocus())
		_qtWengoPhone.getQtCallBar().getCallBarFrame()->getVoxOxToolTipLineEdit()->cut();
	if (_qtWengoPhone.getQtProfileBar()->getQtImProfileWidget()->getVoxOxToolTipLineEdit()->hasFocus())
		_qtWengoPhone.getQtProfileBar()->getQtImProfileWidget()->getVoxOxToolTipLineEdit()->cut();
}
// VOXOX CHANGE by ASV 06-05-2009: added to implement the functionality of the Edit Menu
void QtToolBar::copySlot() {
	if (_qtWengoPhone.getQtCallBar().getCallBarFrame()->getVoxOxToolTipLineEdit()->hasFocus())
		_qtWengoPhone.getQtCallBar().getCallBarFrame()->getVoxOxToolTipLineEdit()->copy();
	if (_qtWengoPhone.getQtProfileBar()->getQtImProfileWidget()->getVoxOxToolTipLineEdit()->hasFocus())
		_qtWengoPhone.getQtProfileBar()->getQtImProfileWidget()->getVoxOxToolTipLineEdit()->copy();
}
// VOXOX CHANGE by ASV 06-05-2009: added to implement the functionality of the Edit Menu
void QtToolBar::pasteSlot() {
	if (_qtWengoPhone.getQtCallBar().getCallBarFrame()->getVoxOxToolTipLineEdit()->hasFocus())
		_qtWengoPhone.getQtCallBar().getCallBarFrame()->getVoxOxToolTipLineEdit()->paste();
	if (_qtWengoPhone.getQtProfileBar()->getQtImProfileWidget()->getVoxOxToolTipLineEdit()->hasFocus())
		_qtWengoPhone.getQtProfileBar()->getQtImProfileWidget()->getVoxOxToolTipLineEdit()->paste();
}
// VOXOX CHANGE by ASV 06-05-2009: added to implement the functionality of the Edit Menu
void QtToolBar::selectAllSlot() {
	if (_qtWengoPhone.getQtCallBar().getCallBarFrame()->getVoxOxToolTipLineEdit()->hasFocus())
		_qtWengoPhone.getQtCallBar().getCallBarFrame()->getVoxOxToolTipLineEdit()->selectAll();
	if (_qtWengoPhone.getQtProfileBar()->getQtImProfileWidget()->getVoxOxToolTipLineEdit()->hasFocus())
		_qtWengoPhone.getQtProfileBar()->getQtImProfileWidget()->getVoxOxToolTipLineEdit()->selectAll();
}*/

static QWidget* getActiveTextEdit() {
	return QApplication::focusWidget();
}

void QtToolBar::updateClipboardActions(QWidget *oldWidget, QWidget *newWidget)
{
	// Disconnect any connected signals because the widget focuse changed
	if (_activeTextEdit) {
		SAFE_DISCONNECT_SENDER_SIGNAL_RECEIVER_METHOD(_activeTextEdit, SIGNAL(copyAvailable(bool)), this, SLOT(handleEditMenu(bool)));
	} else if (_activeLineEdit) {
		SAFE_DISCONNECT_SENDER_SIGNAL_RECEIVER_METHOD(_activeLineEdit, SIGNAL(selectionChanged()), this, SLOT(handleEditMenu()));
	} /*else if (_activeComboBox) {
		// check if this combobox is editable to fix crash when it doesn't have a LineEdit
		if (_activeComboBox->lineEdit())
			SAFE_DISCONNECT_SENDER_SIGNAL_RECEIVER_METHOD(_activeComboBox->lineEdit(), SIGNAL(selectionChanged()), this, SLOT(handleEditMenu()));
	}*/
	
	// We instance the corresponding widget that has focus,
	// if it doesn't have docus the instance will be null.
	_activeTextEdit = qobject_cast<QTextEdit*>(newWidget);
	_activeLineEdit = qobject_cast<QLineEdit*>(newWidget);
	_activeWebView = qobject_cast<QWebView*>(newWidget);
	_activeComboBox = qobject_cast<QComboBox*>(newWidget);
	
	bool modifiable   = false;
	bool hasSelection = false;
	
	// Setup the menu according to the active widget 
	if (_activeTextEdit) 
	{
		modifiable = !_activeTextEdit->isReadOnly();
		SAFE_CONNECT(_activeTextEdit, SIGNAL(selectionChanged()), SLOT(handleEditMenu()));
		_ui->actionSelect_All->setEnabled(true);
	} else if (_activeLineEdit)
	{
		modifiable = !_activeLineEdit->isReadOnly();
		SAFE_CONNECT(_activeLineEdit, SIGNAL(selectionChanged()), SLOT(handleEditMenu()));
		_ui->actionSelect_All->setEnabled(true);
	} else if (_activeWebView) {
		_ui->actionCopy->setEnabled(true);
		_ui->actionSelect_All->setEnabled(true);
	/*} else if (_activeComboBox) {
		// check if this combobox is editable to fix crash when it doesn't have a LineEdit
		if (_activeComboBox->lineEdit()) { 
			modifiable = !_activeComboBox->lineEdit()->isReadOnly();
			SAFE_CONNECT(_activeComboBox->lineEdit(), SIGNAL(selectionChanged()), SLOT(handleEditMenu()));
			_ui->actionSelect_All->setEnabled(true);		
		} else {
			_ui->actionSelect_All->setEnabled(false);
		}
	}*/
	}else{
		_ui->actionSelect_All->setEnabled(false);	
	}
	
	_ui->actionPaste->setEnabled(modifiable);
}

//VOXOX -ASV- 08-17-2009
void QtToolBar::handleEditMenu() {
	if (_activeTextEdit) {
		bool hasSelection = !_activeTextEdit->textCursor().selectedText().isEmpty();
		bool modifiable = !_activeTextEdit->isReadOnly();
		_ui->actionCopy->setEnabled(hasSelection);
		_ui->actionCut->setEnabled(modifiable && hasSelection);
	} else if (_activeLineEdit) {
		bool enable = _activeLineEdit->hasSelectedText();
		_ui->actionCopy->setEnabled(enable);
		_ui->actionCut->setEnabled(enable);
	} else if (_activeComboBox) {
		bool enable = _activeComboBox->lineEdit()->hasSelectedText();
		_ui->actionCopy->setEnabled(enable);
		_ui->actionCut->setEnabled(enable);		
	}
}

void QtToolBar::cutSlot() {
	if (_activeTextEdit) {
		_activeTextEdit->cut();
	} else if (_activeLineEdit) {
		_activeLineEdit->cut();
	} else if (_activeComboBox) {
		_activeComboBox->lineEdit()->cut();
	}
}

void QtToolBar::copySlot() {
	
	if (_activeTextEdit) {
		_activeTextEdit->copy();
	} else if (_activeLineEdit) {
		_activeLineEdit->copy();
	} else if (_activeWebView) {
		// VOXOX -ASV- 08-26-2009: we set the qstring text to the clipboard instead of 
		// using the triggerAction so we remove the style from the font
		//_activeWebView->triggerPageAction(QWebPage::Copy);		
		QApplication::clipboard()->setText(_activeWebView->page()->selectedText());
	} else if (_activeComboBox) {
		_activeComboBox->lineEdit()->copy();
	}
}

void QtToolBar::pasteSlot() {

	if (_activeTextEdit) {
		_activeTextEdit->paste();
	} else if (_activeLineEdit) {
		_activeLineEdit->paste();
	} else if (_activeComboBox) {
		_activeComboBox->lineEdit()->paste();
	}
}

void QtToolBar::selectAllSlot() {

	if (_activeTextEdit) {
		_activeTextEdit->selectAll();
	} else if (_activeLineEdit) {
		_activeLineEdit->selectAll();
	} else if (_activeWebView){
		_activeWebView->triggerPageAction(QWebPage::SelectAll);
	} else if (_activeComboBox) {
		_activeComboBox->lineEdit()->selectAll();
	}
}

////VOXOX - SEMR - 2009.05.20 Network Menu for Actions Menu
void QtToolBar::addNetworkMenu(){

	SAFE_CONNECT(_ui->menuAddNetwork, SIGNAL(triggered(QAction *)), SLOT(addIMAccount(QAction *)));

	QtEnumIMProtocolMap& qtProtocolMap = QtEnumIMProtocolMap::getInstance();

	for ( QtEnumIMProtocolMap::iterator it = qtProtocolMap.begin(); it != qtProtocolMap.end(); it++ )
	{
		if ( (*it).second.useInIMAccountMenu() )
		{
			QString path = (*it).second.getIconPath().c_str();
			QIcon icon( path );
			QString strText = (*it).second.getName().c_str();

			_ui->menuAddNetwork->addAction( icon, strText );
		}
	}
}

//VOXOX - SEMR - 2009.06.23 PLUS BUTTON NEW MENU
QMenu * QtToolBar::getStatusBarAddMenu(){

	QMenu * addIMAccountMenu = new QMenu(_qtWengoPhone.getWidget());
	SAFE_CONNECT(addIMAccountMenu, SIGNAL(triggered(QAction *)), SLOT(addIMAccount(QAction *)));

	QAction * addContact =  new QAction("Add Contact...", this);
	SAFE_CONNECT(addContact, SIGNAL(triggered()), SLOT(addContact()));
	addIMAccountMenu->addAction(addContact);

	QAction * addGroup =  new QAction("Add Group...", this);
	SAFE_CONNECT(addGroup, SIGNAL(triggered()), SLOT(addContactGroup()));	
	addIMAccountMenu->addAction(addGroup);

	QMenu * networks = new QMenu(tr("Add Network"));
	
	QtEnumIMProtocolMap& qtProtocolMap = QtEnumIMProtocolMap::getInstance();

	for ( QtEnumIMProtocolMap::iterator it = qtProtocolMap.begin(); it != qtProtocolMap.end(); it++ )
	{
		if ( (*it).second.useInIMAccountMenu() )
		{
			QString path = (*it).second.getIconPath().c_str();
			QIcon icon( path );
			QString strText = (*it).second.getName().c_str();

			networks->addAction( icon, strText );
		}
	}

	addIMAccountMenu->addMenu(networks);

	return addIMAccountMenu;
}

void QtToolBar::addIMAccount(QAction * action) {
	QString protocolName = action->text();
	QtEnumIMProtocol::IMProtocol imProtocol = QtEnumIMProtocolMap::getInstance().toIMProtocol(protocolName.toStdString());	//VOXOX - JRT - 2009.04.20

	//VOXOX - JRT - 2009.05.05 - Avoid crashes with unknown protocol ( and '+Add Contact' from addContact menu item ).
	if ( imProtocol != QtEnumIMProtocol::IMProtocolUnknown )	//VOXOX - JRT - 2009.05.05 - TODO: determine this from QtEnumIMProtocol list.
	{
		QtIMAccountSettings(_qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile()->getUserProfile(), imProtocol, _qtWengoPhone.getWidget());
	}
}

void QtToolBar::addContactGroup() {

	QtRenameGroup dialog("", _qtWengoPhone.getWidget());
	if (dialog.exec()) 
	{
		CContactList& cContactList = _qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile()->getCContactList();
		ContactGroup contactGroup( dialog.getGroupName().toUtf8().data(), EnumGroupType::GroupType_User );
		cContactList.addContactGroup( contactGroup );	//VOXOX - JRT - 2009.05.07 JRT-GRPS
	}
}
void QtToolBar::showWengoAccount() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
		//WARNING should not be called when no UserProfile set
		WsUrl::showWengoAccount();
	}
}

void QtToolBar::editMyProfile() {
	//if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
	//	QWidget * parent = qobject_cast<QWidget *>(sender()->parent());

	//	//FIXME this method should not be called if no UserProfile has been set
	//	QtProfileDetails * qtProfileDetails   = new QtProfileDetails(*_cWengoPhone.getCUserProfileHandler().getCUserProfile(),
	//		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile(),
	//		parent, tr("Edit My Profile"));

	//	//TODO UserProfile must be updated if QtProfileDetails was accepted
	//	qtProfileDetails->show();
	//}

	_qtWengoPhone.getQtVoxWindowManager()->showProfileWindow();
}
//VOXOX CHANGE Rolando 03-20-09
void QtToolBar::profileBarActionTriggered(){
//	Config & config = ConfigManager::getInstance().getCurrentConfig();
//	config.set(Config::SHOW_PROFILE_BAR_KEY, !config.getShowProfileBar());
//	showOrHideProfileBar(config.getShowProfileBar());
	showOrHideProfileBar( getConfig().toggle( Config::SHOW_PROFILE_BAR_KEY ) );
	updateShowHideProfileBarAction();		
}

//VOXOX CHANGE Rolando 03-25-09
void QtToolBar::callBarActionTriggered(){
//	Config & config = ConfigManager::getInstance().getCurrentConfig();
//	config.set(Config::SHOW_CALL_BAR_KEY, !config.getShowCallBar());
//	showOrHideCallBar(config.getShowCallBar());
	showOrHideCallBar( getConfig().toggle( Config::SHOW_CALL_BAR_KEY ) );
	updateShowHideCallBarAction();		
}
void QtToolBar::addCOntactSlot(){
	addContact();
}
void QtToolBar::addContact() 
{
//	_qtWengoPhone.getQtVoxWindowManager()->showContactManagerWindow( QString(""), QtContactProfileWidget::Action_Add );	//VOXOX - JRT - 2009.09.18 

	//VOXOX - JRT - 2009.08.31 - Use CM.
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) 
	{
		QWidget * parent = qobject_cast<QWidget *>(sender()->parent());

		ContactProfile contactProfile;
		QVariant groupId;
		std::string groupName;

		QtFastAddIMContact qtFastAddIMContact(contactProfile, *_cWengoPhone.getCUserProfileHandler().getCUserProfile(), 0, groupId, groupName);

		if (qtFastAddIMContact.exec()) //VOXOX - CJC - 2009.07.13 
		{
			Group grp( groupName, EnumGroupType::GroupType_User);
			contactProfile.getGroups().Add( &grp );

			_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().addContact(contactProfile);
		}
	}
}
//VOXOX - SEMR - 2009.06.01 Show network manage 
void QtToolBar::showNetworkConfig() {

	_qtWengoPhone.getQtVoxWindowManager()->showConfigWindow(QString("Networks"));

}
//VOXOX - SEMR - 2009.06.01 Show Appearance
void QtToolBar::showAppearanceConfig() {
	_qtWengoPhone.getQtVoxWindowManager()->showConfigWindow(QString("Appearance"));
}


void QtToolBar::showConfig() {	//VOXOX - JRT - 2009.07.02 
	//QtWengoConfigDialog * settings = new QtWengoConfigDialog(getDefaultParent(), _cWengoPhone);//VOXOX CHANGE by ASV 07-08-2009: the parent depends on the OS
	//settings->showPhoneSettingsPageEx( jsAction );
	//settings->show();
	
	_qtWengoPhone.getQtVoxWindowManager()->showConfigWindow(QString("General"));
	//showPhoneSettingsPage
}
void QtToolBar::showPhoneConfig() {	//VOXOX - JRT - 2009.07.02 
	//QtWengoConfigDialog * settings = new QtWengoConfigDialog(getDefaultParent(), _cWengoPhone);//VOXOX CHANGE by ASV 07-08-2009: the parent depends on the OS
	//settings->showPhoneSettingsPageEx( jsAction );
	//settings->show();
	
	_qtWengoPhone.getQtVoxWindowManager()->showConfigWindow(QString("Phone"));
	//showPhoneSettingsPage
}

void QtToolBar::showWizard() {
	_qtWengoPhone.getQtVoxWindowManager()->showWizardWindow();
	// VOXOX CHANGE by ASV 06-27-2009: Fix a bug that deleted the menu when you open a wizard window. This happens only on Mac.
	//QtWizardDialog * wizard = new QtWizardDialog(getDefaultParent(), _cWengoPhone);//VOXOX CHANGE by ASV 07-08-2009: the parent depends on the OS
	//wizard->show();
}

//VOXOX CHANGE by Rolando - 2009.09.22 
void QtToolBar::showStore() {
	showApps();//VOXOX CHANGE by Rolando - 2009.09.22 	
}

// VOXOX -ASV- 07-09-2009
void QtToolBar::showContactManager() {
	showContactManager( "" );
}

void QtToolBar::showContactManager( QString contactId ) {
	
	//if ( _contactProfileWidget == NULL )
	//{
	//	// VOXOX CHANGE by ASV 06-30-2009: on Mac we need to have a parent so we don't lose the menu.
	//	//VOXOX - JRT - 2009.06.30 - Should work for Windows too, but forces CM on top of main window!
	//	_contactProfileWidget = new QtContactProfileWidget(&_cWengoPhone, contactId, getDefaultParent() );//VOXOX CHANGE by ASV 07-08-2009: the parent depends on the OS
	//}


	//_contactProfileWidget->raise();

	//_contactProfileWidget->showNormal();
	//_contactProfileWidget->show();

	_qtWengoPhone.getQtVoxWindowManager()->showContactManagerWindow(contactId, QtContactProfileWidget::Action_Edit);

}

void QtToolBar::updateShowHideOfflineContactsAction() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if (config.getShowOfflineContacts()) {
		_ui->actionShowOnlineContacts->setChecked(true);
	}
	else {
		_ui->actionShowOnlineContacts->setChecked(false);
	}
}

//VOXOX CHANGE Rolando 03-20-09
void QtToolBar::updateShowHideProfileBarAction() {
//	Config & config = ConfigManager::getInstance().getCurrentConfig();	
//	_ui->actionProfileBar->setChecked(config.getShowProfileBar());
	bool showProfileBar = getConfig().getShowProfileBar();
	_ui->actionShowProfileBar->setChecked( showProfileBar );	//VOXOX - JRT - 2009.05.03
#if defined(OS_MACOSX)
	// VOXOX -ASV- 09.08.2009
	_qtWengoPhone.getToolBar()->toggleViewAction()->setEnabled( showProfileBar );
#endif
}

//VOXOX CHANGE Rolando 03-20-09
void QtToolBar::updateShowHideCallBarAction() {
//	Config & config = ConfigManager::getInstance().getCurrentConfig();	
//	_ui->actionCallBar->setChecked(config.getShowCallBar());
	_ui->actionShowCallBar->setChecked( getConfig().getShowCallBar());
}

void QtToolBar::updateShowHideContactGroupsAction() {
//	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString text;
//	if (config.getShowGroups()) {
	if ( getConfig().getShowGroups()) {	//VOXOX - JRT - 2009.05.03 
		text = tr("Hide Groups");
	} else {
		text = tr("Show Groups");
	}

	_ui->actionShowHideContactGroups->setText(text);
}

void QtToolBar::updateToggleVideoAction() {

      bool videoEnabled = getConfig().getVideoEnable();     //VOXOX - JRT - 2009.05.03 
      QString text;
      if (videoEnabled) {
            text = tr("Disable &Video Calls");
      } else {
            text = tr("Enable &Video Calls");
      }

      _ui->actionToggleVideo->setText(text);
      _ui->actionToggleVideo->setEnabled(!getConfig().getVideoWebcamDevice().empty());

}

void QtToolBar::showWengoForum() {
	WsUrl::showWengoForum();
}

void QtToolBar::showWengoServices() {
	WsUrl::showWengoCallOut();
}

void QtToolBar::searchWengoContact() {
#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	QtWebDirectory * qtWebDirectory = _qtWengoPhone.getQtWebDirectory();
	if (qtWebDirectory) {
		qtWebDirectory->raise();
	}
#else
	WsUrl::showWengoSmartDirectory();
#endif
}

void QtToolBar::showAbout() {
/*	QWidget * parent = qobject_cast<QWidget *>(sender()->parent());*/
	QtAbout * qtAbout = new QtAbout(0);
	qtAbout->show();
}

void QtToolBar::checkForUpdates() 
{
	CUserProfile* cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();

	if ( cUserProfile )
	{
		cUserProfile->checkForSoftUpdate( true );		//Manual
	}
}

void QtToolBar::sendSms() {

	CUserProfile * cuserprofile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	if (cuserprofile) {
		if (cuserprofile->getUserProfile().hasWengoAccount()) {
			QtSms * qtSms = _qtWengoPhone.getQtSms();
			if (qtSms) {
				QWidget * parent = qobject_cast<QWidget *>(sender()->parent());
				qtSms->getWidget()->setParent(parent, Qt::Dialog);
				qtSms->getWidget()->show();
			}
			return;
		} else {
			QtNoWengoAlert noWengoAlert(NULL, _qtWengoPhone);
			noWengoAlert.getQDialog()->exec();
		}
	}
}

//VOXOX -ASV- 07-12-2009: activates the Main Window after clicking the VoxOx menu from Windows menu. Only Mac
void QtToolBar::showMainWindow()
{
	if (!_qtWengoPhone.getWidget()->isVisible()){
		_qtWengoPhone.getWidget()->show();
	}
	else if (_qtWengoPhone.getWidget()->isMinimized()) {
		_qtWengoPhone.getWidget()->showNormal();
		_qtWengoPhone.getWidget()->activateWindow();		
	}
	else {
		_qtWengoPhone.getWidget()->activateWindow(); 
	}
}

void QtToolBar::createConferenceCall() {

	CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	if (cUserProfile) {

		IPhoneLine * phoneLine = cUserProfile->getUserProfile().getActivePhoneLine();
		if (phoneLine) {
			if (phoneLine->getActivePhoneCall()) {

				QtVoxMessageBox box(_qtWengoPhone.getWidget());
				box.setWindowTitle(tr("VoxOx - - Proceed Conference"));
				box.setText("Conference can't be started while there are active phone calls. Finish all phone calls and try again.");
				box.setStandardButtons(QMessageBox::Ok);
				box.exec();

			} else {
				_qtWengoPhone.showConferenceWidget();
				/*QtConferenceCallWidget conferenceDialog(_qtWengoPhone.getWidget(), _cWengoPhone, phoneLine);
				conferenceDialog.exec();*/
			}
		}
	}
}


void QtToolBar::showWengoFAQ() {
	WsUrl::showWengoFAQ();
}
void QtToolBar::showRatesPage() {
	WsUrl::showRatesPage();
}
void QtToolBar::showDeveloperPlatform() {
	WsUrl::showDeveloperPlatform();
}

//VOXOX - SEMR - 2009.05.19 New option Help Menu
void QtToolBar::showReportBug(){
	WsUrl::showReportBug();
}
//VOXOX - SEMR - 2009.05.19 New option Help Menu
void QtToolBar::showSuggestFeature(){
	WsUrl::showSuggestFeature();
}
//VOXOX - SEMR - 2009.05.19 New option Help Menu
void QtToolBar::showBillingFeature(){
	WsUrl::showBillingFeature();
}


void QtToolBar::showHideOfflineContacts() {
	QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
	if (qtContactList) {
		getConfig().toggle( Config::GENERAL_SHOW_OFFLINE_CONTACTS_KEY );
		qtContactList->updatePresentation();
		updateShowHideOfflineContactsAction();
	}
}

//void QtToolBar::showHideContactGroups() {
//	QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
//	if (qtContactList) {
////		Config & config = ConfigManager::getInstance().getCurrentConfig();
////		config.set(Config::GENERAL_SHOW_GROUPS_KEY, !config.getShowGroups());
//		getConfig().toggle(Config::GENERAL_SHOW_GROUPS_KEY);		//VOXOX - JRT - 2009.05.03 
//		qtContactList->updatePresentation();
//		updateShowHideContactGroupsAction();
//	}
//}

void QtToolBar::toggleVideo() {
//	Config & config = ConfigManager::getInstance().getCurrentConfig();
//	config.set(Config::VIDEO_ENABLE_KEY, !config.getVideoEnable());
	getConfig().toggle( Config::VIDEO_ENABLE_KEY );
}

void QtToolBar::clearHistoryOutgoingCalls() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile() &&
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()) {
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()->clear(HistoryMemento::OutgoingCall);
	}
}

void QtToolBar::clearHistoryIncomingCalls() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile() &&
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()) {
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()->clear(HistoryMemento::IncomingCall);
	}
}

void QtToolBar::clearHistoryMissedCalls() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile() &&
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()) {
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()->clear(HistoryMemento::MissedCall);
	}
}

void QtToolBar::clearHistoryRejectedCalls() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile() &&
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()) {
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()->clear(HistoryMemento::RejectedCall);
	}
}

void QtToolBar::clearHistoryChatSessions() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile() &&
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()) {
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()->clear(HistoryMemento::ChatSession);
	}
}

void QtToolBar::clearHistorySms() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile() &&
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()) {
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()->clear(HistoryMemento::OutgoingSmsOk);
	}
}

void QtToolBar::clearHistoryAll() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile() &&
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()) {
		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCHistory()->clear(HistoryMemento::Any);
	}
}

//VOXOX - JRT - 2009.07.12 - Moved to QtWengoPhone.
//void QtToolBar::logOff() 
//{
//	if (_userIsLogged) {
//		// check for pending calls
//		CUserProfile *cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
//		if (cUserProfile) {
//			CPhoneLine *cPhoneLine = cUserProfile->getCPhoneLine();
//			if (cPhoneLine) {
//				if (cPhoneLine->hasPendingCalls()) {
//					
//					QtVoxMessageBox box(_qtWengoPhone.getWidget());
//					box.setWindowTitle("VoxOx - Warning");
//					box.setText(tr("You have unfinished call(s).  Are you sure you want to exit the application?"));
//
//					box.addButton(tr("&Log Off"), QMessageBox::AcceptRole);
//					box.addButton(tr("&Cancel"), QMessageBox::RejectRole);
//
//					int ret = box.exec();
//					if(ret == QMessageBox::RejectRole)  {
//						return;
//					}
//				}
//			}
//		}
//		////
//                // Close Chat
//                _qtWengoPhone.closeChatWindow ();
//		
//		// disable menubar and toolbar to avoid crashes
//		//_ui->menuBar->setEnabled(false);
//		_ui->actionLogOff->setEnabled(false);
//		//_ui->toolBar->setEnabled(false); //VOXOX CHANGE by ASV 04-24-2009: we are not using this anymore
//		////
//	}
//
//	// Calling this when the user has already been logged off is harmless and
//	// will bring the login dialog.
//	_cWengoPhone.getCUserProfileHandler().logOff();
//}

//VOXOX - SEMR - 2009.06.01 Close Tab option on file Menu and trigger shortcut
void QtToolBar::closeTab(){
	/*if (_qtWengoPhone.getChatWindow()){
		_qtWengoPhone.getChatWindow()->closeActiveTab();
	}*/
}

int QtToolBar::findFirstCallTab() {
	for (int i = 0; i < _ui->tabWidget->count(); i++) {
		if(_ui->tabWidget->tabText(i).toLower() == QString("call") || _ui->tabWidget->tabText(i).toLower() == QString("conference")){
			return i;
		}
	}
	return -1;	
}

//VOXOX CHANGE CJC OPEN TABS
void QtToolBar::showContactsTab() {
    _ui->tabWidget->setCurrentWidget(_ui->tabContactList);
}

void QtToolBar::showKeyTab() {
	_ui->tabWidget->setCurrentWidget(_ui->tabDialpad);
}

void QtToolBar::showHistory() {
	_ui->tabWidget->setCurrentWidget(_ui->tabHistory);
}

//VOXOX CHANGE by Rolando - 2009.09.22 
void QtToolBar::showApps(){
	_qtWengoPhone.showHomeTab();//VOXOX CHANGE by Rolando - 2009.09.22 
}

void QtToolBar::acceptCall() {

	//int callIndex = _ui->tabWidget->currentIndex();//VOXOX CHANGE by Rolando - 2009.05.22 - gets current active tab
	//if (callIndex == -1) {
	//	return;
	//}

	////VOXOX CHANGE by Rolando - 2009.05.22 - checks if current tab is a call or conference tab
	//QtContactCallListWidget * widget = _qtWengoPhone.getQtContactCallListWidget(_ui->tabWidget->widget(callIndex));

	//if (!widget) {
	//	return;
	//}

	//QtPhoneCall * qtPhoneCall = widget->getFirstQtPhoneCall();
	//if (qtPhoneCall) {
	//	qtPhoneCall->acceptCall();
	//}
}

void QtToolBar::holdResumeCall() {

	//int callIndex = _ui->tabWidget->currentIndex();//VOXOX CHANGE by Rolando - 2009.05.22 - gets current active tab
	//if (callIndex == -1) {
	//	return;
	//}

	////VOXOX CHANGE by Rolando - 2009.05.22 - checks if current tab is a call or conference tab
	//QtContactCallListWidget * widget = _qtWengoPhone.getQtContactCallListWidget(_ui->tabWidget->widget(callIndex));
	//if (!widget) {
	//	return;
	//}

	//if(widget->getFirstQtPhoneCall()){
	//	widget->getFirstQtPhoneCall()->holdOrResume();
	//}
	//

}

void QtToolBar::hangUpCall() {

	//VOXOX CHANGE by Rolando - 2009.05.22 - hangs up current active tab - if current tab is a call or conference tab then hangs up
	_qtWengoPhone.hangUpButtonClicked();

}

void QtToolBar::updateMenuActions() {

	//bool accept = false, holdResume = false, hangUp = false;
	//
	//int callIndex = findFirstCallTab();
	////if (callIndex > 0) {
	//if (callIndex != -1) {

	//	//QtContactCallListWidget * widget = (QtContactCallListWidget *) _ui->tabWidget->widget(callIndex);
	//	QtContactCallListWidget * widget = _qtWengoPhone.getQtContactCallListWidget(_ui->tabWidget->widget(callIndex));
	//	if (widget) {

	//		QtPhoneCall * qtPhoneCall = widget->getFirstQtPhoneCall();

	//		if (qtPhoneCall) {

	//			switch (qtPhoneCall->getCPhoneCall().getState()) {
	//				
	//				case EnumPhoneCallState::PhoneCallStateResumed:
	//				case EnumPhoneCallState::PhoneCallStateTalking: {
	//					holdResume = true;
	//					hangUp = true;
	//					break;
	//				}
	//				
	//				case EnumPhoneCallState::PhoneCallStateDialing:
	//				case EnumPhoneCallState::PhoneCallStateRinging: {
	//					hangUp = true;
	//					break;
	//				}
	//				
	//				case EnumPhoneCallState::PhoneCallStateIncoming: {
	//					accept = true;
	//					hangUp = true;
	//					break;
	//				}
	//				
	//				case EnumPhoneCallState::PhoneCallStateHold: {
	//					holdResume= true;
	//					hangUp = true;
	//					break;
	//				}
	//				
	//				case EnumPhoneCallState::PhoneCallStateError: {
	//					hangUp = true;
	//					break;
	//				}
	//				
	//				// other cases -> set all to false
	//				case EnumPhoneCallState::PhoneCallStateMissed :
	//				case EnumPhoneCallState::PhoneCallStateRedirected:
	//				case EnumPhoneCallState::PhoneCallStateClosed:
	//				case EnumPhoneCallState::PhoneCallStateUnknown:
	//				default: {
	//					//accept = holdResume = hangUp = false;
	//				}
	//			}
	//		}
	//	}
	//}

	//_ui->actionAcceptCall->setEnabled(accept);
	//_ui->actionHangUpCall->setEnabled(hangUp);
	//_ui->actionHoldResumeCall->setEnabled(holdResume);


	//bool accept = false, holdResume = false, hangUp = false;
	//
	//int callIndex = _ui->tabWidget->currentIndex();//VOXOX CHANGE by Rolando - 2009.05.22 - gets current active tab
	//
	//if (callIndex >= 0) {

	//	//VOXOX CHANGE by Rolando - 2009.05.22 - checks if current tab is a call or conference tab
	//	QtContactCallListWidget * widget = _qtWengoPhone.getQtContactCallListWidget(_ui->tabWidget->widget(callIndex));
	//	if (widget) {

	//		QtPhoneCall * qtPhoneCall = widget->getFirstQtPhoneCall();

	//		if (qtPhoneCall) {

	//			switch (qtPhoneCall->getCPhoneCall().getState()) {
	//				
	//				case EnumPhoneCallState::PhoneCallStateResumed:
	//				case EnumPhoneCallState::PhoneCallStateTalking: {
	//					holdResume = true;
	//					hangUp = true;
	//					break;
	//				}
	//				
	//				case EnumPhoneCallState::PhoneCallStateDialing:
	//				case EnumPhoneCallState::PhoneCallStateRinging: {
	//					hangUp = true;
	//					break;
	//				}
	//				
	//				case EnumPhoneCallState::PhoneCallStateIncoming: {
	//					accept = true;
	//					hangUp = true;
	//					break;
	//				}
	//				
	//				case EnumPhoneCallState::PhoneCallStateHold: {
	//					holdResume= true;
	//					hangUp = true;
	//					break;
	//				}
	//				
	//				case EnumPhoneCallState::PhoneCallStateError: {
	//					hangUp = true;
	//					break;
	//				}
	//				
	//				// other cases -> set all to false
	//				case EnumPhoneCallState::PhoneCallStateMissed :
	//				case EnumPhoneCallState::PhoneCallStateRedirected:
	//				case EnumPhoneCallState::PhoneCallStateClosed:
	//				case EnumPhoneCallState::PhoneCallStateUnknown:
	//				default: {
	//					//accept = holdResume = hangUp = false;
	//				}
	//			}
	//		}
	//	}
	//}

	//_ui->actionAcceptCall->setEnabled(accept);
	//_ui->actionHangUpCall->setEnabled(hangUp);
	//_ui->actionHoldResumeCall->setEnabled(holdResume);
}

void QtToolBar::showChatWindow() {
	//VOXOX - SEMR - 2009.05.21 Specify the method a little more
	QtUMWindow * chatWindow = _qtWengoPhone.getChatWindow();
	if (chatWindow) {
		chatWindow->showNormal();
		chatWindow->activateWindow();
	}
}

void QtToolBar::showFileTransferWindow() {

	QtFileTransfer * fileTransfer = _qtWengoPhone.getFileTransfer();
	if (fileTransfer) {
			fileTransfer->getFileTransferWidget()->show();
	}
}

void QtToolBar::retranslateUi() {
	updateShowHideContactGroupsAction();
	updateShowHideOfflineContactsAction();
	updateToggleVideoAction();
	updateActionsAccordingToUserStatus();
}

void QtToolBar::configChangedEventHandler(const std::string & key) {
	configChanged(QString::fromStdString(key));
}

// VOXOX CHANGE by ASV 07-08-2009: added to clean up the code a little
QWidget *QtToolBar::getDefaultParent()
{
	// VOXOX CHANGE by ASV 05-15-2009: we need to have the QMainWindow from QtWengoPhone as the parent for this Window
	// in order to show all the menu items
	QWidget *parent = NULL;
	#if defined(OS_MACOSX)
	parent = _qtWengoPhone.getWidget();
	#endif
	return parent;
}

void QtToolBar::slotConfigChanged(QString qkey) {
	std::string key = qkey.toStdString();
	if (key == Config::VIDEO_ENABLE_KEY || key == Config::VIDEO_WEBCAM_DEVICE_KEY) {
		updateToggleVideoAction();
	}
//VOXOX CHANGE by ASV 04-24-2009: we are not using this anymore
/*	if (key == Config::GENERAL_TOOLBARMODE_KEY) {
		updateToolBar();
	}*/
}

//VOXOX CHANGE by ASV 04-24-2009: we are not using this anymore
/*
void QtToolBar::updateToolBar() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string text = config.getToolBarMode();
	EnumToolBarMode::ToolBarMode toolBarMode = EnumToolBarMode::toToolBarMode(text);


	if (toolBarMode == EnumToolBarMode::ToolBarModeHidden) {
		_ui->toolBar->hide(); 

	} else if (toolBarMode == EnumToolBarMode::ToolBarModeIconsOnly) {
		_ui->toolBar->show();
		_ui->toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

		// Restore icon width in case it was set by TextUnderIcon mode
		QSize size = _ui->toolBar->iconSize();
		size.setWidth(size.height());
		_ui->toolBar->setIconSize(size);

	} else if (toolBarMode == EnumToolBarMode::ToolBarModeTextUnderIcons) {
		_ui->toolBar->show();
		_ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		QSize size = _ui->toolBar->iconSize();
		size.setWidth(TOOLBAR_BUTTON_MINIMUM_WIDTH);
		_ui->toolBar->setIconSize(size);

	} else {
		LOG_WARN("Unknown toolbar mode: " + toolBarMode);
	}
}
*/

void QtToolBar::userProfileDeleted() {
	_userIsLogged = false;
	updateActionsAccordingToUserStatus();
}

// VOXOX -ASV- 07-09-2009: added to implement the Window menu for Mac
void QtToolBar::handleMinimize()
{
	if (_qtWengoPhone.getWidget()->isActiveWindow()) {
		_qtWengoPhone.getWidget()->showMinimized();
	} else if (_qtWengoPhone.getChatWindow()->isActiveWindow()) {
		_qtWengoPhone.getChatWindow()->showMinimized();
	}
}

// VOXOX -ASV- 07-09-2009: added to implement the Window menu for Mac
void QtToolBar::handleZoom()
{
	if (_qtWengoPhone.getWidget()->isActiveWindow()) {
		_qtWengoPhone.getWidget()->showMaximized();
	} else if (_qtWengoPhone.getChatWindow()->isActiveWindow()) {
		_qtWengoPhone.getChatWindow()->showMaximized();
	}	
}

// VOXOX -ASV- 07-09-2009: the close window option from the menu should work with the active window
// and not just the main window
void QtToolBar::handleCloseWindow()
{
	if (_qtWengoPhone.getWidget()->isActiveWindow()) {
		_qtWengoPhone.getWidget()->hide();
	} /*else if (_qtWengoPhone.getChatWindow()->isActiveWindow()) {
		_qtWengoPhone.getChatWindow()->closeWindow();
	}*/ else {
		_qtWengoPhone.getQtVoxWindowManager()->closeActiveWindow();
	}
}


void QtToolBar::userProfileIsInitialized() {
	_userIsLogged = true;
	updateActionsAccordingToUserStatus();
	// VOXOX -ASV- 07-12-2009: Sets the user name to the VoxOx - <Username> menu at the Window menu for Mac
#if defined(OS_MACOSX)	
	_ui->actionVoxOx->setText("VoxOx - " + QString::fromStdString(_qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile()->getUserProfile().getSipAccount()->getDisplayName()));
	updateShowHideProfileBarAction();
#endif
}

void QtToolBar::enableMenuBar() {
	_ui->menuBar->setEnabled(true);
}

void QtToolBar::updateActionsAccordingToUserStatus() {
	// We need to use a boolean to keep trace of the user status because when
	// userProfileDeleted is called, the user profile is still alive, even if
	// it's not for a long time.
	if (_userIsLogged) {
		_ui->actionLogOff->setVisible(true);
		_ui->actionLogOff->setText(tr("Sign &Out")); //VOXOX CHANGE by ASV 06-29-2009: removed "..."
	} else {
		_ui->actionLogOff->setVisible(false);
		_ui->actionLogOff->setText(tr("Sign &On")); //VOXOX CHANGE by ASV 06-29-2009: removed "..."
	}
	//_ui->menuBar->setEnabled(_userIsLogged);
	//_ui->toolBar->setEnabled(_userIsLogged); //VOXOX CHANGE by ASV 04-24-2009: we are not using this anymore

	// This list contains all actions which needs a logged user to work
	QList<QAction*> loggedInActions;
	loggedInActions
//VOXOX - SEMR - 2009.06.01 Actions that change is user log in or log out
	<< _ui->actionAddContact
	<< _ui->actionManageNetworks
	<< _ui->menuAddNetwork->actions()
	<< _ui->actionEditMyProfile
	<< _ui->actionPlaceCall
	<< _ui->actionCreateConferenceCall	
	<< _ui->actionSendInstantMessage
	//<< _ui->actionGroupChat
	<< _ui->actionSendSms
	//<< _ui->actionSendGroupTextMessage
	<< _ui->actionSendEmail
	//<< _ui->actionSendGroupEmail
	//<< _ui->actionSendFile
	<< _ui->actionSendFax
	<< _ui->actionProfileBar
	<< _ui->actionCallBar
	<< _ui->actionShowAppearance
	<< _ui->actionShowSetupWizard
	<< _ui->actionShowStore//VOXOX CHANGE by Rolando - 2009.09.22 
	<< _ui->actionToggleVideo
	//<< _contactMenu2->actions()//VOXOX CHANGE by ASV 07-03-2009: We are not using QtContactMenu2 anymore
	<< _ui->menuClearHistory->actions()
	<< _ui->actionShowConfig
	;
	Q_FOREACH(QAction* action, loggedInActions) {
		action->setEnabled(_userIsLogged);
	}
}

void QtToolBar::tryingToConnect()
{
	_ui->actionLogOff->setText(tr("Log &Off..."));
	_ui->actionLogOff->setEnabled(true);
}

//VOXOX - JRT - 2009.05.03
//VOXOX CHANGE by ASV 07-03-2009: We are not using QtContactMenu2 anymore
/*void QtToolBar::replaceContactMenu()
{
	//QtContactMenu2* contactMenu2 = new QtContactMenu2( this, _ui );	//VOXOX - JRT - 2009.05.27 
	//_ui->menuBar->removeAction( _ui->menuContacts->menuAction() );		//VOXOX - JRT - 2009.05.05 - Remove old Contact menu. TODO: remove from designer.
	//_ui->menuBar->insertMenu  ( _ui->menuTools->menuAction(), _contactMenu2 ); //VOXOX - SEMR - 2009.05.27 Change the order of appearance of the new VIEW menu
	//connect( _contactMenu2, SIGNAL(triggered(QAction*)), this, SLOT( handleContactMenu(QAction*)));
}*/

//VOXOX - JRT - 2009.07.03 - Then you won't need this either
//void QtToolBar::handleContactMenu( QAction* action )
//{
//	if ( action )
//	{
//		int nOption = action->data().value<int>();
//
//		if ( QtEnumSortOption::isContactSort( nOption ) )
//		{
//			getConfig().set( Config::GENERAL_CONTACT_SORT_KEY, nOption );
//		}
//		else if ( QtEnumSortOption::isGrouping( nOption ) )
//		{
//			getConfig().set( Config::GENERAL_CONTACT_GROUPING_KEY, nOption );
//		}
//		else if ( QtEnumSortOption::isGroupSort( nOption ) )
//		{
//			getConfig().set( Config::GENERAL_GROUP_SORT_KEY, nOption );
//		}
//		else if ( QtEnumSortOption::isView( nOption ) )
//		{
//			switch( nOption )
//			{
//			case QtEnumSortOption::ViewUnavailable:
//				getConfig().toggle( Config::GENERAL_SHOW_OFFLINE_CONTACTS_KEY );
//				break;
//
////			case QtEnumSortOption::ViewGroups:	//VOXOX - JRT - 2009.05.10 - TODO: remove GENERAL_SHOW_GROUPS_KEY
////				getConfig().toggle(Config::GENERAL_SHOW_GROUPS_KEY);		//VOXOX - JRT - 2009.05.03 
////				break;
//
//			default:
//				assert(false);			//New view option?
//			}
//		}
//		else
//		{
//			switch( nOption )
//			{
//			case QtEnumSortOption::AddContact:
//				addContact();
//				break;
//
//			default:
//			assert(false);				//Unexpected menu item.
//			}
//		}
//
//		QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
//		if ( qtContactList )
//		{
//			qtContactList->updatePresentation();
//		}
//	}
//	setChecksStausMenu();
//}
//End //VOXOX - JRT - 2009.05.05 
