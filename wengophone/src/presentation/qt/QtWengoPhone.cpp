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
#include "QtWengoPhone.h"

#include "ui_WengoPhoneWindow.h"

#include <presentation/PFactory.h>
#include <presentation/qt/chat/QtUMWindow.h>
#include <presentation/qt/login/QtVoxOxLogin.h>
#include <presentation/qt/login/QtVoxOxLoginMessage.h>
#include <presentation/qt/login/QtVoxOxLoadingLogin.h>
 
#include <presentation/qt/callbar/customCallBar/QtVoxOxCallBarFrame.h>
#include <presentation/qt/chat/umtreewidget/QtUMItemListStyle.h>
#include <presentation/qt/profilebar/QtIMProfileWidget.h>


#include <cutil/global.h>

#include "QtSystray.h"
#include "QtHttpProxyLogin.h"
#include "dialpad/QtDialpad.h"//VOXOX CHANGE by Rolando 04-29-09, now QtDialpad is in dialpad folder
#include "QtIdle.h"
#include "QtLanguage.h"
#include "QtToolBar.h"
#include "QtVoxWindowManager.h"
#include "QtBrowserWidget.h"
#include "QtIMAccountMonitor.h"
#include "callbar/QtCallBar.h"
#include "callbar/QtPhoneComboBox.h"
#include "contactlist/QtContactList.h"

#include "contactlist/QtContactListStyle.h"

#include "filetransfer/QtFileTransfer.h"
#include "history/QtHistoryWidget.h"
#include "imaccount/QtIMAccountManager.h"

#include "phonecall/QtPhoneCall.h"
#include "profile/QtProfileDetails.h"
#include "profilebar/QtProfileBar.h"
#include "statusbar/QtStatusBar.h"
#include "webservices/sms/QtSms.h"


#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	#include "webdirectory/QtWebDirectory.h"
#endif

#include <control/CWengoPhone.h>
#include <control/history/CHistory.h>
#include <control/phoneline/CPhoneLine.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/WengoPhone.h>
#include <model/commandserver/ContactInfo.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/connect/ConnectHandler.h>
#include <model/contactlist/ContactList.h>
#include <model/contactlist/Contact.h>
#include <model/history/History.h>
#include <model/phonecall/PhoneCall.h>
#include <model/phonecall/SipAddress.h>
#include <model/phonecall/ConferenceCall.h>
#include <model/phoneline/IPhoneLine.h>
#include <model/profile/AvatarList.h>
#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileHandler.h>

#include <imwrapper/EnumPresenceState.h>

#include <qtutil/CloseEventFilter.h>
#include <qtutil/Widget.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/VoxOxToolTipLineEdit.h>

#include <cutil/global.h>
#include <thread/Thread.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <webcam/WebcamDriver.h>
//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>

#include <QtGui/QtGui>

#include <QtWebKit/QWebView>

#if defined(OS_MACOSX)
	#include <Carbon/Carbon.h>
	#include <qtutil/VoxOxToolTipLineEdit.h>	
	#include "callbar/customCallBar/QtVoxOxCallBarFrame.h"
	#include "profilebar/QtIMProfileWidget.h"
#endif

static const char * CSS_DIR = "css";

using namespace std;

QtWengoPhone::QtWengoPhone(CWengoPhone & cWengoPhone)
	: QObjectThreadSafe(NULL),
	_cWengoPhone(cWengoPhone) 
{
	_wengoPhoneWindow	 = NULL;
	_qtSystray			 = NULL;
	_qtSms				 = NULL;
	_qtContactList		 = NULL;
	_qtHistoryWidget	 = NULL;
	_qtIdle				 = NULL;
	_qtLanguage			 = NULL;
	_qtStatusBar		 = NULL;
	_qtFileTransfer		 = NULL;
	_qtBrowserWidget	 = NULL;
	_chatWindow			 = NULL;
	_activeTabBeforeCall = NULL;
	_qtVoxOxLogin		 = NULL;
	_qtVoxOxLoginMessage = NULL;
	_qtVoxOxLoadingLogin = NULL;
	_qtDialpad			 = NULL;	//VOXOX - JRT - 2009.07.12 

	_qtToolBar			 = NULL;
	_ui					 = NULL;
	_qtBrowserWidgetTabIndex = -1;	//VOXOX - JRT - 2009.09.21 


	_qtVoxWindowManager = NULL;//VOXOX - CJC - 2009.07.11 
	_userIsLogged = false;	//VOXOX - JRT - 2009.07.12 


#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	_qtWebDirectory = NULL;
#endif

	NetworkProxyDiscovery::getInstance().proxyNeedsAuthenticationEvent += boost::bind(&QtWengoPhone::proxyNeedsAuthenticationEventHandler, this, _1, _2);
	NetworkProxyDiscovery::getInstance().wrongProxyAuthenticationEvent += boost::bind(&QtWengoPhone::wrongProxyAuthenticationEventHandler, this, _1, _2);

	//Check if the event has not already been sent
	if (NetworkProxyDiscovery::getInstance().getState() == NetworkProxyDiscovery::NetworkProxyDiscoveryStateNeedsAuthentication) 
	{
		proxyNeedsAuthenticationEventHandler(NetworkProxyDiscovery::getInstance(), NetworkProxyDiscovery::getInstance().getNetworkProxy());
	}

	qRegisterMetaType<QVariant>("QVariant");

	//Initialize libwebcam for the Qt thread.
	WebcamDriver::apiInitialize();
	////

	initUi();

	typedef PostEvent0<void ()> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtWengoPhone::initThreadSafe, this));
	postEvent(event);

//VOXOX CHANGE by ASV 06-25-2009: VoxOx should become front-most app when launched on the Mac.
#ifdef OS_MACOSX	
	bringMainWindowToFront();
#endif

	initQtSettings();	//VOXOX -ASV- 2009.09.17
}

QtWengoPhone::~QtWengoPhone() {
	//TODO delete created objects
	//VOXOX - JRT - 2009.04.13 - delete created objects.
//JRT-XXX	OWSAFE_DELETE(_qtBrowserWidget);		//	QtBrowserWidget * 
//JRT-XXX	OWSAFE_DELETE(_ui);						//	Ui::WengoPhoneWindow *
//JRT-XXX	OWSAFE_DELETE(_qtSystray);				//	QtSystray * 
//JRT-XXX	OWSAFE_DELETE(_qtSms);					//	QtSms * 
//JRT-XXX	OWSAFE_DELETE(_qtContactCallListWidget);//	QtContactCallListWidget * 
//JRT-XXX	OWSAFE_DELETE(_qtIdle);					//	QtIdle * 
//JRT-XXX	OWSAFE_DELETE(_qtLanguage);				//	QtLanguage * 
//JRT-XXX	OWSAFE_DELETE(_qtToolBar);				//	QtToolBar * 
//JRT-XXX	OWSAFE_DELETE(_qtStatusBar);			//	QtStatusBar * 
//JRT-XXX	OWSAFE_DELETE(_qtFileTransfer);			//	QtFileTransfer * 
//JRT-XXX	OWSAFE_DELETE(_chatWindow);				//	QWidget * 
//JRT-XXX	OWSAFE_DELETE(_activeTabBeforeCall);	//	QWidget * 

//	OWSAFE_DELETE(_qtContactList);			//	QtContactList *		//Deleted elsewhere
//	OWSAFE_DELETE(_qtHistoryWidget);		//	QtHistoryWidget * 

//Crash	OWSAFE_DELETE(_wengoPhoneWindow);		//	QMainWindow *
	//End VoxOx
}

void QtWengoPhone::initUi() {
	// Init parts of the UI which do not rely on Control layer
	QApplication::setQuitOnLastWindowClosed(false);
#if defined OS_WINDOWS
	QApplication::setVoxToolTipDelay(20);//VOXOX - CJC - 2009.11.06 
#endif
	loadStyleSheets();					//About 25,000 memory allocations!

	//Translation
	_qtLanguage = new QtLanguage(this);

	_wengoPhoneWindow = new QMainWindow(NULL);

	_ui = new Ui::WengoPhoneWindow();
	_ui->setupUi(_wengoPhoneWindow);	//About 140,000 memory allocations!

#ifndef CUSTOM_ACCOUNT
	_ui->actionShowWengoAccount->setVisible(false);
#endif

//VOXOX CHANGE by ASV 05-25-2009: this eliminates the QSizeGrip image on the Mac version so it uses the default form the OS
//on Mac and the and specified design on Windows.
#if defined OS_WINDOWS
	_ui->statusBar->setStyleSheet(QString("QSizeGrip {background-image: url(:/pics/statusbar/resizer.png); width: 13px; height: 25px;}"));
#endif

    LANGUAGE_CHANGE(_wengoPhoneWindow);

#ifdef OS_LINUX
	std::string data = AvatarList::getInstance().getDefaultAvatarPicture().getData();
	QPixmap defaultAvatar;
	defaultAvatar.loadFromData((uchar*) data.c_str(), data.size());
	_wengoPhoneWindow->setWindowIcon(QIcon(defaultAvatar));
#endif

	//Install the close event filter
	CloseEventFilter * closeEventFilter = new CloseEventFilter(this, SLOT(closeWindow()));
	_wengoPhoneWindow->installEventFilter(closeEventFilter);

	//QtCallBar
	//_ui->callBar->getQtPhoneComboBox()->setQtWengoPhone(this);
	

	
	//VOXOX - CJC - 2009.07.11 
	_qtVoxWindowManager = new QtVoxWindowManager(*this, _wengoPhoneWindow);//VOXOX - CJC - 2009.07.09 
	//QtToolBar
	_qtToolBar = new QtToolBar(*this, _ui, _wengoPhoneWindow);
	//VOXOX CHANGE Rolando 03-20-09
	SAFE_CONNECT(_qtToolBar, SIGNAL(showOrHideProfileBar(bool)), SLOT(showOrHideProfileBarSlot(bool)));

	//VOXOX CHANGE Rolando 03-20-09
	SAFE_CONNECT(_qtToolBar, SIGNAL(showOrHideCallBar(bool)), SLOT(showOrHideCallBarSlot(bool)));
	//phoneComboBox
	//SAFE_CONNECT(_ui->callBar, SIGNAL(phoneComboBoxClicked()), SLOT(phoneComboBoxClicked()));

	SAFE_CONNECT(_ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(tabIndexChange(int)));
	//Buttons initialization
	initCallButtons();

//	Config & config = ConfigManager::getInstance().getCurrentConfig();
	//QtStatusBar
	_qtStatusBar = new QtStatusBar(_ui->statusBar);
	//VOXOX CHANGE CJC ADD MENU SUPPORT
	_qtStatusBar->setAddMenu(_qtToolBar->getStatusBarAddMenu(),QString("Add Contacts and Networks"));
	//_qtStatusBar->setSettingsMenu(_qtToolBar->getStatusBarSettingsMenu(_ui->tabWidget->currentIndex()),QString("Settings"));
	changeGearMenu();
	SAFE_CONNECT(_ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(changeGearMenu())); 


//#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
//	_qtWebDirectory = new QtWebDirectory(0);
//#endif

	installQtBrowserWidget();

	
	mainWindowGeometry( getConfig() );		//VOXOX - JRT - 2009.10.08 
}

//VOXOX - SEMR - 2009.07.06 change the menu according to the TABs
void QtWengoPhone::changeGearMenu(){
	int tabIndex = _ui->tabWidget->currentIndex();

	switch (tabIndex){
		case 0:
			_qtStatusBar->setSettingsMenu(_qtToolBar->getContactsTabMenu(), "Contacts");
			break;
		case 1:
			_qtStatusBar->setSettingsMenu(_qtToolBar->getKeypadTabMenu(), "Keypad");
			break;
		case 2:
			_qtStatusBar->setSettingsMenu(_qtToolBar->getAppsTabMenu(), "Apps");
			break;
		case 3:
			_qtStatusBar->setSettingsMenu(_qtToolBar->getHistoryTabMenu(), "History");
			break;
	}
}

void QtWengoPhone::initThreadSafe() {
	// The code in this method relies on the Control layer
	_qtSystray = new QtSystray(this);

	updatePresentation();
}

//VOXOX -ASV- 2009.09.17: we initialize some Qt settings here.
//This includes settings where we add plugins for webkit browser
void QtWengoPhone::initQtSettings() {
	//Enable some QtWebKit plugins and sets some settings
	QWebSettings::globalSettings()->setAttribute( QWebSettings::PluginsEnabled,						true );
	QWebSettings::globalSettings()->setAttribute( QWebSettings::JavascriptEnabled,					true );
	QWebSettings::globalSettings()->setAttribute( QWebSettings::PrivateBrowsingEnabled,				true );
	QWebSettings::globalSettings()->setAttribute( QWebSettings::LocalStorageDatabaseEnabled,		false);
	QWebSettings::globalSettings()->setAttribute( QWebSettings::OfflineWebApplicationCacheEnabled,	false);
	QWebSettings::globalSettings()->setAttribute( QWebSettings::OfflineStorageDatabaseEnabled,		false);	

	readMainWindowSettings();
}

void QtWengoPhone::readMainWindowSettings(){

	 QSettings settings("Telcentris", "VoxOx");
	 settings.beginGroup("VoxOxMainWindow");
	 _wengoPhoneWindow->resize(settings.value("size", QSize(300, 520)).toSize());
	 _wengoPhoneWindow->move(settings.value("pos", QPoint(300, 300)).toPoint());
	 settings.endGroup();

}

void QtWengoPhone::saveMainWindowSettings(){

	 QSettings settings("Telcentris", "VoxOx");
	 settings.beginGroup("VoxOxMainWindow");
	 settings.setValue("size", _wengoPhoneWindow->size());
	 settings.setValue("pos", _wengoPhoneWindow->pos());
	 settings.endGroup();

}


void QtWengoPhone::loadStyleSheets() {
	Config & config = getConfig();		//VOXOX - JRT - 2009.10.08 
	QDir dir(QString::fromStdString(config.getResourcesDir()) + CSS_DIR);

	QStringList filters;
	filters << "*.css";
	QStringList cssList;
	Q_FOREACH(QFileInfo fileInfo, dir.entryInfoList(filters)) {
		QString path = fileInfo.absoluteFilePath();
		QFile file(path);
		if (!file.open(QIODevice::ReadOnly)) {
			LOG_WARN("Can't open " + path.toStdString());
			continue;
		}
		QByteArray content = file.readAll();
		cssList << QString::fromUtf8(content);
	}
	QString styleSheet = cssList.join("\n");
	qApp->setStyleSheet(styleSheet);

		
	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();
	contactListStyle->loadStyleConfig(QString::fromStdString(config.getResourcesDir()) + contactListStyle->getContactStyleFolder() + "dark");//VOXOX - CJC - 2009.09.17 Hardcode this for now, lets change it back when we support multiple styles
	
	
	QtUMItemListStyle * qtUMItemListStyle = QtUMItemListStyle::getInstance();
	qtUMItemListStyle->loadStyleConfig(QString::fromStdString(config.getResourcesDir()) + qtUMItemListStyle->getStyleFolder() + qtUMItemListStyle->getCurrentStyle());


}
//VOXOX - CJC - 2009.05.01 Save last tab index on config. Task #51
void QtWengoPhone::tabIndexChange(int index) {

	getConfig().set(Config::LAST_OPEN_TAB_INDEX, index);	//VOXOX - JRT - 2009.10.08 
	getQtToolBar()->updateMenuActions();//VOXOX - PUTA CHECHO PONGA COMENTARIOS< ESTA MIERDA LA HIZO USTED!!
}

void QtWengoPhone::mainWindowGeometry(Config & config) {
	//default position and size given by Qt
/*	QPoint defaultPos = _wengoPhoneWindow->pos();
	QSize defaultSize = _wengoPhoneWindow->size();

	int profileWidth = config.getProfileWidth();
	if (profileWidth == 0) {
		// Make sure the toolbar is fully visible
		profileWidth = qMax(
			_ui->toolBar->sizeHint().width(),
			_wengoPhoneWindow->sizeHint().width()
			);
	}

	int profileHeight = config.getProfileHeight();

	//Main window size and position saved
	_wengoPhoneWindow->resize(profileWidth, profileHeight);
	_wengoPhoneWindow->move(QPoint(config.getProfilePosX(), config.getProfilePoxY()));

	//tests if the Wengophone is visible, if not sends it back to its default position and size
	QDesktopWidget* desktop = QApplication::desktop();
	if (desktop->screenNumber(_wengoPhoneWindow) == -1) {
		LOG_DEBUG("Main window is NOT visible !!");
		_wengoPhoneWindow->resize(defaultSize);
	 	_wengoPhoneWindow->move(defaultPos);
	}*/
}

QWidget * QtWengoPhone::getWidget() const {
	return _wengoPhoneWindow;
}

QtDialpad * QtWengoPhone::getQtDialpad(){
	return _qtDialpad;
}


QtProfileBar * QtWengoPhone::getQtProfileBar() const {
	return _ui->profileBar;
}

QtBrowserWidget * QtWengoPhone::getQtBrowserWidget() const {
	return _qtBrowserWidget;
}

CWengoPhone & QtWengoPhone::getCWengoPhone() const {
	return _cWengoPhone;
}

#if defined(OS_MACOSX)
// VOXOX -ASV- 10.09.2009
QToolBar * QtWengoPhone::getToolBar() const{
	return _toolBarToggle;
}
#endif

void QtWengoPhone::setQtSms(QtSms * qtSms) {
	_qtSms = qtSms;
}

QtSms * QtWengoPhone::getQtSms() const {
	return _qtSms;
}

#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
QtWebDirectory * QtWengoPhone::getQtWebDirectory() const {
	return _qtWebDirectory;
}
#endif

QtToolBar * QtWengoPhone::getQtToolBar() const {
	return _qtToolBar;
}

QtVoxWindowManager * QtWengoPhone::getQtVoxWindowManager() const {//VOXOX - CJC - 2009.07.09 
	return _qtVoxWindowManager;
}

QtStatusBar & QtWengoPhone::getQtStatusBar() const {
	return *_qtStatusBar;
}

QtSystray & QtWengoPhone::getQtSystray() const {
	return *_qtSystray;
}

QtCallBar & QtWengoPhone::getQtCallBar() const {
	return *_ui->callBar;
}

void QtWengoPhone::setChatWindow(QtUMWindow * chatWindow) {
	if (!chatWindow) {
		_chatWindow = NULL;
		/*_ui->actionOpenChatWindow->setEnabled(false);*/ //VOXOX - SEMR - 2009.05.18 not in use
	}
	else {
		_chatWindow = chatWindow;
		/*_ui->actionOpenChatWindow->setEnabled(true);*/ //VOXOX - SEMR - 2009.05.18 not in use
	}
}

QtUMWindow * QtWengoPhone::getChatWindow() const {
	return _chatWindow;
}

void QtWengoPhone::closeChatWindow() {
        if (_chatWindow)
        {
            _chatWindow->close();
        }
}

void QtWengoPhone::setQtContactList(QtContactList * qtContactList) {
	_qtContactList = qtContactList;



	if (!_ui->tabContactList->layout()) {
		Widget::createLayout(_ui->tabContactList);
	}

	_ui->tabContactList->layout()->addWidget(_qtContactList->getWidget());

	


	LOG_DEBUG("QtContactList added");
}

QtContactList * QtWengoPhone::getQtContactList() const 
{
	if ( _userIsLogged )			//VOXOX - JRT - 2009.10.08 - Indicates _qtContactList is being destroyed.
	return _qtContactList;
	else
		return NULL;
}

void QtWengoPhone::setQtHistoryWidget(QtHistoryWidget * qtHistoryWidget) {
	_qtHistoryWidget = qtHistoryWidget;
	_qtHistoryWidget->setQtToolBar(_qtToolBar);
	_qtHistoryWidget->setQtCallBar(_ui->callBar);

	if (!_ui->tabHistory->layout()) {
		Widget::createLayout(_ui->tabHistory);
	}

	_qtHistoryWidget->getWidget()->setParent(_ui->tabHistory);
	_ui->tabHistory->layout()->addWidget(_qtHistoryWidget->getWidget());

	LOG_DEBUG("QtHistoryWidget added");
}

void QtWengoPhone::setQtDialpad(QtDialpad * qtDialpad) {
	_qtDialpad = qtDialpad;
	Widget::createLayout(_ui->tabDialpad)->addWidget(qtDialpad);
	LOG_DEBUG("QtDialpad added");
}

void QtWengoPhone::initCallButtons() {
	//callButton
	SAFE_CONNECT(_ui->callBar, SIGNAL(callButtonClicked()), SLOT(callButtonClicked()));
	//enableCallButton();
	_ui->callBar->setEnabledCallButton(false);

	//hangUpButton
	SAFE_CONNECT(_ui->callBar, SIGNAL(hangUpButtonClicked()), SLOT(hangUpButtonClicked()));
	//_ui->callBar->setEnabledHangUpButton(false);
	_ui->callBar->setCallPushButtonImage();

	//phoneComboBox
	//VOXOX CHANGE Rolando 03-25-09
	SAFE_CONNECT(_ui->callBar, SIGNAL(phoneComboBoxReturnPressed()),					SLOT(callButtonClicked()));
	SAFE_CONNECT(_ui->callBar, SIGNAL(phoneComboBoxEditTextChanged(const QString &)),	SLOT(enableCallButton()));
	SAFE_CONNECT(_ui->callBar, SIGNAL(phoneComboBoxEditTextChanged(const QString &)),	SLOT(populateFilterGroup()));	//VOXOX - JRT - 2009.05.30 
}

void QtWengoPhone::enableCallButton() {
	std::string phoneNumber = _ui->callBar->getPhoneComboBoxCurrentText();
	_ui->callBar->setEnabledCallButton(!phoneNumber.empty());//VOXOX CHANGE Rolando 03-25-09

	QString fullPhoneNumber = _ui->callBar->getFullPhoneNumberText();//VOXOX CHANGE by Rolando - 2009.06.23 - necessary to implement the sync between text in callbar and keypad window 
	_qtDialpad->setPhoneNumber(fullPhoneNumber);//VOXOX CHANGE by Rolando - 2009.06.23 - necessary to implement the sync between text in callbar and keypad window  
}

//VOXOX - JRT - 2009.05.30 
void QtWengoPhone::populateFilterGroup()
{
	std::string phoneNumber = _ui->callBar->getPhoneComboBoxCurrentText();

	//VOXOX - JRT - 2009.10.08 
	if ( getCContactList() )
	{
		getCContactList()->populateFilterGroup( phoneNumber, "Search Results" );
	}

	QString fullPhoneNumber = _ui->callBar->getFullPhoneNumberText();//VOXOX CHANGE by Rolando - 2009.06.23 - necessary to implement the sync between text in callbar and keypad window 
	_qtDialpad->setPhoneNumber(fullPhoneNumber);//VOXOX CHANGE by Rolando - 2009.06.23 - necessary to implement the sync between text in callbar and keypad window 
}

//VOXOX CHANGE Rolando 03-25-09
void QtWengoPhone::aboutToHangUp(){
	setCallPushButtonImage();
}

//VOXOX CHANGE Rolando 03-25-09
void QtWengoPhone::setCallPushButtonImage(){
	_ui->callBar->setCallPushButtonImage();
}

//VOXOX CHANGE by Rolando - 2009.05.22 - this method is used to get a QtContactCallListWidget according its widget
//QtContactCallListWidget * QtWengoPhone::getQtContactCallListWidget(QWidget * widget){
//
//	QList<QtContactCallListWidget*>::iterator iter;
//	for (iter = _qtContactCallListWidgetMap.begin(); iter != _qtContactCallListWidgetMap.end(); iter++) {
//		if((*iter)->getWidget() == widget){
//			return *iter;
//		}
//	}
//
//	return NULL;
//}

//VOXOX CHANGE by Rolando - 2009.05.22 - this method hangs up the call according its active tabwidget
void QtWengoPhone::hangUpButtonClicked() {

	//VOXOX CHANGE by Rolando - 2009.05.22 - finds the QtContactCallListWidget in active tabwidget
	//QList<QtContactCallListWidget*>::iterator iter;
	//for (iter = _qtContactCallListWidgetMap.begin(); iter != _qtContactCallListWidgetMap.end(); iter++) {
	//	if((*iter)->getWidget() == _ui->tabWidget->currentWidget()){
	//		(*iter)->hangup();//VOXOX CHANGE by Rolando - 2009.05.22 - hangs up the call
	//	}
	//}

	//VOXOX CHANGE by Rolando - 2009.06.04 
	if(_chatWindow){
		_chatWindow->hangUpCurrentCall();//VOXOX CHANGE by Rolando - 2009.06.04 - hangs up the current call
	}

	if(_qtContactCallListWidgetMap.empty()){//VOXOX CHANGE by Rolando - 2009.05.22 - if there are not more calls tabs then updates buttons
		_ui->callBar->setCallPushButtonImage();//VOXOX CHANGE Rolando 03-25-09
		std::string phoneNumber = _ui->callBar->getPhoneComboBoxCurrentText();
		_ui->callBar->setEnabledCallButton(!phoneNumber.empty());//VOXOX CHANGE Rolando 03-25-09	
	}
	else{
		//VOXOX CHANGE by Rolando - 2009.05.22 - if there are more calls then updates buttons
		_ui->callBar->setEnabledCallButton(true);
		_ui->callBar->setHangUpPushButtonImage();//VOXOX CHANGE by Rolando - 2009.05.19 
	}
}

void QtWengoPhone::callButtonClicked() {

	//VOXOX CHANGE Rolando 04-29-09
	QString phoneNumber = _ui->callBar->getFullPhoneNumberText();
	phoneNumber = phoneNumber.trimmed();
	makeCall(phoneNumber.toStdString());	
}

//VOXOX CHANGE Rolando 04-29-09, call button in keypad was clicked
void QtWengoPhone::callButtonDialpadClicked(std::string phoneNumber) {
	QString currentAreaCode = QString::fromStdString(_ui->callBar->getCurrentAreaCode());
	QString fullPhoneNumber = QString::fromStdString(phoneNumber);

	if(fullPhoneNumber != ""){//VOXOX CHANGE by Rolando - 2009.05.15 - if phone number is not empty
		if(currentAreaCode != ""){//VOXOX CHANGE by Rolando - 2009.05.15 - if area code is selected
			if(currentAreaCode != "1"){//VOXOX CHANGE by Rolando - 2009.05.15 - if area code is not USA then adds prefix 011 + phone number
				fullPhoneNumber = QString("011") + currentAreaCode + fullPhoneNumber;
			}			
		}

		makeCall(fullPhoneNumber.toStdString());
	}
}

//TODO: VOXOX CHANGE Rolando 04-29-09, move this method to a utilitary class. 
/*This method returns true if parameter text is a phone number */
bool QtWengoPhone::textIsPhoneNumber(QString text){
//  formats allowed: 
//  800-555-1212
//  (800) 555-1212
//  (80) 555-1212
//  80-555-1212
//  800-555-12123
//  800 555 1212
//  800/555/1212
//  +1 <number>
//  +39 <number>
//  *<number>

	return text.indexOf(QRegExp("^(\\+\\d\\d?)?[\\-\\s\\/\\.]?[\\(]?(\\d){2,}[\\)]?[\\-\\s\\/\\.]?\\d\\d\\d[\\-\\s\\/\\.]?(\\d){3,}\\b|\\*\\d+$")) >= 0;
}

void QtWengoPhone::makeCall(std::string phoneNumber){
	//VOXOX CHANGE Rolando 04-29-09
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {		
		if (!phoneNumber.empty()) {
			CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
			//cUserProfile->makeCallErrorEvent += boost::bind(&QtWengoPhone::makeCallErrorEventHandler, this, _2, _3);//VOXOX CHANGE by Rolando - 2009.06.02 - as we are using chat to insert calls we are handling errors in QtChatWindow

			if(textIsPhoneNumber(QString::fromStdString(phoneNumber))){
				cUserProfile->makeCall(phoneNumber);//VOXOX CHANGE by Rolando - 2009.05.27 - if this is a call to non contact
			}
			else{
				cUserProfile->makeContactCall(phoneNumber);
			}
			_ui->callBar->setEnabledCallButton(true);
			_ui->callBar->setHangUpPushButtonImage();//VOXOX CHANGE Rolando 03-25-09
			_ui->callBar->addComboBoxItem(QString::fromStdString(phoneNumber));
		}		
	}
}

void QtWengoPhone::setActiveTabBeforeCallCurrent() {
	if(_activeTabBeforeCall){
		_activeTabBeforeCall->setFocus();
		_ui->tabWidget->setCurrentWidget(_activeTabBeforeCall);
	}
}

//VOXOX CHANGE by Rolando - 2009.05.29 - added parameter phoneNumber to check what call failed
void QtWengoPhone::makeCallErrorEventHandler(EnumMakeCallError::MakeCallError error, std::string phoneNumber) {
	typedef PostEvent2<void (EnumMakeCallError::MakeCallError error, std::string phoneNumber), EnumMakeCallError::MakeCallError, std::string> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtWengoPhone::makeCallErrorEventHandlerThreadSafe, this, _1, _2), error, phoneNumber);
	postEvent(event);
}

//VOXOX CHANGE by Rolando - 2009.05.29 - added parameter phoneNumber to check what call failed
void QtWengoPhone::makeCallErrorEventHandlerThreadSafe(EnumMakeCallError::MakeCallError error, std::string phoneNumber) {

	showCallErrorMessage(error, phoneNumber);

}

//VOXOX CHANGE by Rolando - 2009.06.02 - shows a message indicating that a call failed
void QtWengoPhone::showCallErrorMessage(EnumMakeCallError::MakeCallError error, std::string phoneNumber) {
	QString message;
	switch (error) {
	case EnumMakeCallError::CallNotHeld:
		message = QString("Please hold all the phone calls before placing a new one");
		break;
	case EnumMakeCallError::NotConnected:
		message = QString("Your phone account is not connected.\nPlease reconnect and try your call again.");
		break;
	case EnumMakeCallError::EmptyPhoneNumber:
		message = QString("You must enter a phone number");
		break;
	case EnumMakeCallError::SipError:
		message = QString("An error has occured trying to place the call\nwith phone number: %1.").arg(QString::fromStdString(phoneNumber));
		break;	
	case EnumMakeCallError::ContactNotFound:
		message = QString("Contact's phone number not found");
		break;
	case EnumMakeCallError::NoError:
		LOG_DEBUG("No error happened");
		break;

	case EnumMakeCallError::CallAlreadyStarted://VOXOX CHANGE by Rolando - 2009.10.05 
		message = QString("You already have an active call with this phone number: %1.").arg(QString::fromStdString(phoneNumber));//VOXOX CHANGE by Rolando - 2009.10.05 
		break;//VOXOX CHANGE by Rolando - 2009.10.05 

	default://VOXOX CHANGE by Rolando - 2009.10.05 
		message = QString("An Unknown error happened (code: %1) ").arg(error);//VOXOX CHANGE by Rolando - 2009.10.05 

	}
	
	//VOXOX CHANGE by Rolando - 2009.05.28 - close the corresponding call tab in QtChatWindow
	if(_chatWindow){
		_chatWindow->removeCallTabWidget(QString::fromStdString(phoneNumber));			
	}


	QtVoxMessageBox box(getWidget());
	box.setWindowTitle(tr("VoxOx - Call Error"));
	box.setText(message);
	box.setStandardButtons(QMessageBox::Ok);
	box.exec();

	setCallPushButtonImage();//VOXOX CHANGE Rolando 03-25-09
}

void QtWengoPhone::updateStyle() {

	if(_qtContactList){
		_qtContactList->updatePresentation();
	}

	QSize winsize = _wengoPhoneWindow->size();
	//Giggle the widget so the contact group refreshs
	_wengoPhoneWindow->resize(winsize.width()-1, winsize.height()-1);
	_wengoPhoneWindow->resize(winsize.width()+1, winsize.height()+1);
}


//VOXOX CHANGE by Rolando - 2009.05.22 - creates a new call tab in tabwidget
void QtWengoPhone::addPhoneCall(QtPhoneCall * qtPhoneCall) {
	_activeTabBeforeCall = _ui->tabWidget->currentWidget();
	QtContactCallListWidget * qtContactCallListWidget = new QtContactCallListWidget(getCWengoPhone(), NULL);
	qtContactCallListWidget->addPhoneCall(qtPhoneCall);
	QtEnumUMMode::Mode mode = QtEnumUMMode::UniversalMessageCall;
	QString key = generateCallKey(qtPhoneCall->getPhoneNumber(),mode);
	qtContactCallListWidget->setKey(key);
	qtContactCallListWidget->setMode(mode);
	_qtContactCallListWidgetMap[key] = qtContactCallListWidget;
	

	SAFE_CONNECT(qtContactCallListWidget, SIGNAL(isClosing(QString)),SLOT(slotQtContactCallListWidgetIsClosing(QString)));
	
	//VOXOX CHANGE by Rolando - 2009.05.29 - inserts a call tab inside the chat window
	if(_chatWindow){
		_chatWindow->addCall(qtPhoneCall->getPhoneNumber(),qtContactCallListWidget);
		/*_chatWindow->addCallTab(qtPhoneCall->getPhoneNumber().toStdString());
		_chatWindow->insertContactCallListWidget(qtContactCallListWidget);*/
	}
	else{
		qtContactCallListWidget->hangup();
	}

	_ui->callBar->setEnabledCallButton(true);
	_ui->callBar->setHangUpPushButtonImage();//VOXOX CHANGE Rolando 03-25-09

	if (qtPhoneCall->isIncoming()) {
		_ui->callBar->setCallPushButtonImage();//VOXOX CHANGE Rolando 03-25-09
	}	

}

QString QtWengoPhone::generateCallKey(const QString & number, QtEnumUMMode::Mode mode){
	QString key = number + "@" +QtEnumUMMode::toString(mode);
	return key;
}


//VOXOX CHANGE by Rolando 04-29-09, this method is called when _qtContactCallListWidget is destroyed so it removes this widget from stackedwidget
void QtWengoPhone::slotQtContactCallListWidgetIsClosing(QString key){

	QtContactCallListWidgetMap::iterator iter = _qtContactCallListWidgetMap.find(key);//Check if item exists

	if (iter != _qtContactCallListWidgetMap.end()) {
			
			//VOXOX CHANGE by Rolando - 2009.05.29 - removes a call tab inside the chat window according the call destroyed
			if(_chatWindow){
				_chatWindow->removeCallTabWidget(key);
			}
			_qtContactCallListWidgetMap.erase(key);

	}

	
	//QList<QtContactCallListWidget*>::iterator iter;
	//QtContactCallListWidget * tmp = NULL;
	//for (iter = _qtContactCallListWidgetMap.begin(); iter != _qtContactCallListWidgetMap.end(); iter++) {
	//	tmp = (*iter);
	//	if (tmp == object) {

	//		//VOXOX CHANGE by Rolando - 2009.05.29 - removes a call tab inside the chat window according the call destroyed
	//		if(_chatWindow){
	//			QString key = tmp->getKey();
	//			_chatWindow->removeCallTabWidget(tmp->getWidget());
	//		}
	//		_qtContactCallListWidgetMap.removeAll(tmp);
	//		break;
	//	}
	//}

	if(!_qtContactCallListWidgetMap.empty()){
		_ui->callBar->setEnabledCallButton(true);
		_ui->callBar->setHangUpPushButtonImage();//VOXOX CHANGE by Rolando - 2009.05.19
	}
	else{
		_ui->callBar->setCallPushButtonImage();//VOXOX CHANGE Rolando 03-25-09
		std::string phoneNumber = _ui->callBar->getPhoneComboBoxCurrentText();
		_ui->callBar->setEnabledCallButton(!phoneNumber.empty());//VOXOX CHANGE Rolando 03-25-09
	}
	

	setActiveTabBeforeCallCurrent();
}

//VOXOX CHANGE by Rolando 04-29-09 shows Conference Window in keypad tab
void QtWengoPhone::showConferenceWidget() {
	_activeTabBeforeCall = _ui->tabWidget->currentWidget();
	
	_ui->tabWidget->setCurrentWidget(_ui->tabDialpad);
	_qtDialpad->showInitiateConferenceCallWindow();

}

void QtWengoPhone::addToConference(QString phoneNumber, PhoneCall * targetCall) {
	//TODO: VOXOX CHANGE by Rolando 04-29-09, by the moment it is not needed but it has to be fixed to add phonecalls to keypad
	//FIXME conference has to be 100% rewritten...
	/*bool conferenceAlreadyStarted = false;

	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
		int nbtab = _ui->tabWidget->count();

		for (int i = 0; i < nbtab; i++) {
			if (_ui->tabWidget->tabText(i) == QString(tr("Conference"))) {
				return;
			}

			for (int j = 0; j < _ui->tabWidget->count(); j++) {
				QtContactCallListWidget * qtContactCallListWidget = dynamic_cast<QtContactCallListWidget *>(_ui->tabWidget->widget(j));
				if (qtContactCallListWidget) {
					if (qtContactCallListWidget->hasPhoneCall(targetCall)) {
						_ui->tabWidget->setTabText(j, tr("Conference"));
						IPhoneLine * phoneLine = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getActivePhoneLine();

						if (phoneLine != NULL) {
							if (!conferenceAlreadyStarted) {
								conferenceAlreadyStarted = true;
								ConferenceCall * confCall = new ConferenceCall(*phoneLine);
								confCall->addPhoneCall(*targetCall);
								confCall->addPhoneNumber(phoneNumber.toStdString());
							}
						} else {
							LOG_DEBUG("phoneLine is NULL");
						}
					}
				}
			}
		}
	}*/

	//if (!_cWengoPhone.getCUserProfileHandler().getCUserProfile())
	//	return;

	//IPhoneLine * phoneLine = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getActivePhoneLine();
	//if(!phoneLine)
	//	return;

	//for (int i = 0; i < _ui->tabWidget->count(); i++) 
	//{
	//	if (_ui->tabWidget->tabText(i) == QString(tr("Conference"))) 
	//		return;
	//}

	//ConferenceCall * confCall = new ConferenceCall(*phoneLine);
	//targetCall->setConferenceCall(confCall);

	//PhoneCall * phoneCall2 = confCall->getPhoneCall(phoneNumber.toStdString());

	//QtContactCallListWidget * qtContactCallListWidgetTarget = 0;
	//QtPhoneCall * qtPhoneTarget = 0;
	//QtPhoneCall * qtPhoneCall2 = 0;
	//int index2 = -1;

	//for (int j = 0; j < _ui->tabWidget->count(); j++) 
	//{
	//	QtContactCallListWidget * qtContactCallListWidget = dynamic_cast<QtContactCallListWidget *>(_ui->tabWidget->widget(j));
	//	if (qtContactCallListWidget) 
	//	{
	//		SAFE_CONNECT(qtContactCallListWidget, SIGNAL(removeCallTab()), SLOT(removeCallTab()));

	//		if (!qtContactCallListWidgetTarget && qtContactCallListWidget->hasPhoneCall(targetCall)) 
	//		{
	//			_ui->tabWidget->setTabText(j, tr("Conference"));

	//			qtContactCallListWidgetTarget = qtContactCallListWidget;
	//			qtPhoneTarget = qtContactCallListWidgetTarget->takeQtPhoneCall(targetCall);
	//		}
	//		else if(!qtPhoneCall2 && qtContactCallListWidget->hasPhoneCall(phoneCall2))
	//		{
	//			qtPhoneCall2 = qtContactCallListWidget->takeQtPhoneCall(phoneCall2);
	//			index2 = j;
	//			
	//		}
	//	}
	//}

	//if( qtContactCallListWidgetTarget)
	//{
	//	if(qtPhoneCall2)
	//	{
	//		//qtPhoneCall2->hideConfButton();
	//		qtContactCallListWidgetTarget->addPhoneCall(qtPhoneCall2);
	//		_ui->tabWidget->removeTab(index2);
	//	}
	//	if(qtPhoneTarget)
	//	{
	//		//qtPhoneTarget->hideConfButton();
	//		qtContactCallListWidgetTarget->addPhoneCall(qtPhoneTarget);
	//	}
	//}

	//QString address = QString::fromStdString(targetCall->getPeerSipAddress().getSipAddress());
	//
	//if(address.contains("@"))
	//	address = address.section("@",0,0);

	//if (address.startsWith("sip:")) {
	//	address = address.mid(4);
	//}

	//confCall->addPhoneNumber(address.toStdString());
	//confCall->addPhoneNumber(phoneNumber.toStdString());
	//
	//if(phoneCall2)
	//	phoneCall2->setConferenceCall(confCall);

	//if(qtContactCallListWidgetTarget)
	//	qtContactCallListWidgetTarget->slotStartedTalking(qtPhoneTarget);

	
}

//VOXOX CHANGE by Rolando 04-29-09, now we create a conference call in dialpad (keypad tab)
void QtWengoPhone::addToConference(QtPhoneCall * qtPhoneCall) {

	if(_chatWindow){
		QtContactCallListWidget * contactCallListWidget = getCurrentConferenceCall();
		if(contactCallListWidget){
			if(!contactCallListWidget->hasConferenceStarted()){//VOXOX CHANGE by Rolando - 2009.05.18 - if already exists a conference tab in chat window
				contactCallListWidget->addPhoneCall(qtPhoneCall);//VOXOX CHANGE by Rolando - 2009.05.22 - adds the phonecall
			}else{
				//TODO NOTIFI USER ONLY ONE CONFERENCE AT A TIME
			}
		}else{
			//VOXOX CHANGE by Rolando - 2009.05.18 - if not exists a conference tab then creates a conference call tab
			_activeTabBeforeCall = _ui->tabWidget->currentWidget();

			QtContactCallListWidget * qtContactCallListWidget = new QtContactCallListWidget(getCWengoPhone(), NULL);
			qtContactCallListWidget->addPhoneCall(qtPhoneCall);
			QtEnumUMMode::Mode mode = QtEnumUMMode::UniversalMessageConferenceCall;
			QString key = generateCallKey(qtPhoneCall->getPhoneNumber(),mode);
			qtContactCallListWidget->setKey(key);
			qtContactCallListWidget->setMode(mode);
			_qtContactCallListWidgetMap[key] = qtContactCallListWidget;
			

			SAFE_CONNECT(qtContactCallListWidget, SIGNAL(isClosing(QString)),SLOT(slotQtContactCallListWidgetIsClosing(QString)));
			
			_chatWindow->addCall(qtPhoneCall->getPhoneNumber(),qtContactCallListWidget);

			_ui->callBar->setEnabledCallButton(true);
			_ui->callBar->setHangUpPushButtonImage();//VOXOX CHANGE Rolando 03-25-09		

			if (qtPhoneCall->isIncoming()) {
				_ui->callBar->setCallPushButtonImage();//VOXOX CHANGE Rolando 03-25-09
			}

		}
	}
}


//VOXOX CHANGE by Rolando - 2009.05.29 - slot associated when happened an error during a conference call
void QtWengoPhone::cancelConferenceCall(std::string phoneNumber){
	if(_chatWindow){
		_chatWindow->removeCallTabWidget(QString::fromStdString(phoneNumber));			
	}
}

QtContactCallListWidget * QtWengoPhone::getCurrentConferenceCall(){

	QtContactCallListWidgetMap::const_iterator iter    = _qtContactCallListWidgetMap.begin();
	QtContactCallListWidgetMap::const_iterator iterEnd = _qtContactCallListWidgetMap.end();

	while (iter != iterEnd)
	{
		if(iter->second->getMode() == QtEnumUMMode::UniversalMessageConferenceCall){
			return iter->second;	
		}
	}
	return NULL;
}



QWidget * QtWengoPhone::getCurrentTabWidget(){
	return _ui->tabWidget->currentWidget();
}

QtFileTransfer * QtWengoPhone::getFileTransfer() const {
	return _qtFileTransfer;
}

void QtWengoPhone::updatePresentation() {
	typedef PostEvent0<void ()> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtWengoPhone::updatePresentationThreadSafe, this));
	postEvent(event);
}

void QtWengoPhone::updatePresentationThreadSafe() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
		//disabled some actions if no SIP Account is used
		bool hasSipAccount = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().hasSipAccount();

		_ui->actionShowWengoAccount->setEnabled(_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().hasWengoAccount());
		_ui->actionSendSms->setEnabled(hasSipAccount);
		_ui->actionCreateConferenceCall->setEnabled(hasSipAccount);
		_ui->actionSearchWengoContact->setEnabled(hasSipAccount);
	}
}

void QtWengoPhone::dialpad(const std::string & tone) {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
		_ui->callBar->setPhoneComboBoxEditText(_ui->callBar->getPhoneComboBoxCurrentText() + tone);//VOXOX CHANGE Rolando 03-25-09

		if(_qtDialpad){//VOXOX CHANGE by Rolando - 2009.06.23 - necessary to implement the sync between text in callbar and keypad window 
			_qtDialpad->setPhoneNumber(_ui->callBar->getFullPhoneNumberText());//VOXOX CHANGE by Rolando - 2009.06.23 - necessary to implement the sync between text in callbar and keypad window 
		}
		//_ui->callBar->setPhoneComboBoxEditText(_ui->callBar->getPhoneComboBoxCurrentText() + tone);
	}
}

//VOXOX CHANGE Rolando 04-02-09, delete las character written in callbar
void  QtWengoPhone::dialpadBackButtonPressed(){
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
		QString currentText = QString::fromStdString(_ui->callBar->getPhoneComboBoxCurrentText());
		if(currentText.length() > 0){
			currentText = currentText.remove(currentText.length() - 1, 1);
			_ui->callBar->setPhoneComboBoxEditText(currentText.toStdString());
		}
	}
	
}

//VOXOX - JRT - 2009.07.12 - Moved here from QtToolBar so it can be reused.
//							 This code prevents some crashes on exit by setting a flag in CUserProfile
void QtWengoPhone::logOff() 
{
	doLogOff( false );
}

void QtWengoPhone::exitApplication()
{
	doLogOff( true );
	prepareToExitApplication() ;
}

void QtWengoPhone::doLogOff( bool quitting )
{
	if (_userIsLogged) 
	{
		if ( checkLogOffIsOk() )
		{
//			_cWengoPhone.getCUserProfileHandler().logOff();	//Tell other objects we are logging off.

			// disable menubar and toolbar to avoid crashes
			//_ui->menuBar->setEnabled(false);
			_ui->actionLogOff->setEnabled(false);
			//_ui->toolBar->setEnabled(false); //VOXOX CHANGE by ASV 04-24-2009: we are not using this anymore

			// Close Chat
			closeChatWindow();	//VOXOX - JRT - 2009.07.12 
		}
	}

	// Calling this when the user has already been logged off is harmless and will bring the login dialog.
	_cWengoPhone.getCUserProfileHandler().logOff( quitting );		//Tell other objects we are logging off.
}

void QtWengoPhone::prepareToExitApplication() 
{
	Config & config = getConfig();	//VOXOX - JRT - 2009.10.08 

	// check for pending calls
	//VOXOX - JRT - 2009.07.12 - Done in logOff();
	//CUserProfile *cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	//if (cUserProfile) {
	//	CPhoneLine *cPhoneLine = cUserProfile->getCPhoneLine();
	//	if (cPhoneLine) {
	//		if (cPhoneLine->hasPendingCalls()) {

	//			QtVoxMessageBox box(getWidget());
	//			box.setWindowTitle("VoxOx - Warning");
	//			box.setText(tr("You have unfinished call(s).Are you sure you want to exit the application?"));
	//			box.addButton(tr("&Exit"), QMessageBox::AcceptRole);
	//			box.addButton(tr("&Cancel"), QMessageBox::RejectRole);
	//			int ret = box.exec();
	//			if(ret == QMessageBox::RejectRole)  {//VOXOX - CJC - 2009.06.23 
	//				return;
	//			}
	//		}
	//	}
	//}
	////
	if(_qtVoxWindowManager){
		_qtVoxWindowManager->closeAllWindows();//VOXOX - CJC - 2009.07.09 
	}
	_qtSystray->hide();

	//Save the window size
	QSize winsize = _wengoPhoneWindow->size();
	config.set(Config::PROFILE_WIDTH_KEY, winsize.width());
	config.set(Config::PROFILE_HEIGHT_KEY, winsize.height());

	//Save the window position
	QPoint winpos = _wengoPhoneWindow->pos();
	config.set(Config::PROFILE_POSX_KEY, winpos.x());
	config.set(Config::PROFILE_POSY_KEY, winpos.y());

	saveMainWindowSettings();//VOXOX - CJC - 2009.09.23 


	QApplication::closeAllWindows();
	QCoreApplication::processEvents();

	//destroyed chatWindow so that chats are saved
//	OWSAFE_DELETE(_chatWindow);		//VOXOX - JRT - 2009.07.13 - We don't own the chatWindow!  And I think the new WindowMgr is handling this?
	////

	_cWengoPhone.terminate();
}

bool QtWengoPhone::checkLogOffIsOk()
{
	bool bOK = true;

	// check for pending calls
	CUserProfile *cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();

	if (cUserProfile) 
	{
		CPhoneLine *cPhoneLine = cUserProfile->getCPhoneLine();
		if (cPhoneLine) 
		{
			if (cPhoneLine->hasPendingCalls()) {
				
				QtVoxMessageBox box( this->getWidget());	//VOXOX - JRT - 2009.07.12 
				box.setWindowTitle("VoxOx - Warning");
				box.setText(tr("You have unfinished call(s).  Are you sure you want to exit the application?"));

				box.addButton(tr("&Log Off"), QMessageBox::AcceptRole);
				box.addButton(tr("&Cancel"), QMessageBox::RejectRole);

				int ret = box.exec();
				if(ret == QMessageBox::RejectRole)  
				{
					bOK = false;
				}
			}
		}
	}

	return bOK;
}

//VOXOX CHANGE Rolando 03-20-09
void QtWengoPhone::showOrHideProfileBarSlot(bool showProfileBar){
#if defined(OS_MACOSX)	
	_toolBarToggle->setVisible(showProfileBar); // VOXOX -ASV- 10.09.2009: we handle the show hide functionality at the toolbar
#else
	_ui->profileBar->setVisible(showProfileBar);
#endif
}

//VOXOX CHANGE Rolando 03-25-09
void QtWengoPhone::showOrHideCallBarSlot(bool showCallBar){
	_ui->callBar->setVisible(showCallBar);
}


void QtWengoPhone::phoneComboBoxClicked() {

	/*_ui->callBar->clearPhoneComboBox();
	_ui->callBar->clearPhoneComboBoxEditText();*/
}

void QtWengoPhone::exitEvent() {
	QCoreApplication::exit(EXIT_SUCCESS);
}

void QtWengoPhone::showHistory() {
	_ui->tabWidget->setCurrentWidget(_ui->tabHistory);
}

void QtWengoPhone::showHomeTab() {//VOXOX - CJC - 2009.06.29 
	if ( _qtBrowserWidgetTabIndex >= 0 )	//VOXOX - JRT - 2009.09.21 - Crash when resuming from hibernation.
	{
	_ui->tabWidget->setCurrentIndex(_qtBrowserWidgetTabIndex);
	}
}

void QtWengoPhone::currentUserProfileWillDieEventHandlerSlot() {
	//Signal for re-initializing graphical components
	//when there is no user profile anymore
	if(_qtVoxWindowManager){
		_qtVoxWindowManager->closeAllWindows();
	}
	userProfileDeleted();
	_userIsLogged = false;	//VOXOX - JRT - 2009.07.12 

	// Reset _qtIMAccountMonitor so that it does not emit signals anymore
	_qtIMAccountMonitor.reset(0);
	
	OWSAFE_DELETE(_qtFileTransfer);
	OWSAFE_DELETE(_qtIdle);
//	OWSAFE_DELETE(_qtVoxWindowManager);//VOXOX - CJC - 2009.07.09 
#if defined(OS_MACOSX)	
	_toolBarToggle->clear(); //VOXOX - ASV - 2009.11.09: we clear the toolbar when the user logs off 
#endif
	_ui->profileBar->reset();

	if (_qtContactList) {
		_ui->tabContactList->layout()->removeWidget(_qtContactList->getWidget());

		//VOXOX - JRT - 2009.10.08 - This really should be handled in CContactList, since that is where QtContactList/PContactList is created.
		//OWSAFE_DELETE(_qtContactList);
		_qtContactList = NULL;
	}

	if (_qtHistoryWidget) {
		_ui->tabHistory->layout()->removeWidget(_qtHistoryWidget->getWidget());
		OWSAFE_DELETE(_qtHistoryWidget);
	}

	uninstallQtBrowserWidget();
}

void QtWengoPhone::userProfileInitializedEventHandlerSlot() {
	CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	cUserProfile->makeCallErrorEvent += boost::bind(&QtWengoPhone::makeCallErrorEventHandler, this, _2, _3);//VOXOX CHANGE by Rolando - 2009.06.02 - as we are using chat to insert calls we are handling errors in QtChatWindow


	if(!getWidget()->isVisible()){//VOXOX CHANGE by Rolando 2009.05.05 if main window was not visible then set visible true
		getWidget()->setVisible(true);
	}	
	
	Config & config = getConfig();	//VOXOX - JRT - 2009.10.08 

	_ui->tabWidget->setCurrentIndex(config.getLastOpenTabIndex());
	_activeTabBeforeCall = _ui->tabWidget->currentWidget();//VOXOX CHANGE by Rolando - 2009.05.22 - we have to initialize this variable after loads lastOpenTabIndex

	//Idle detection
	//FIXME: QtIdle must not use UserProfile but CUserProfile
	_qtIdle = new QtIdle(cUserProfile->getUserProfile(), _wengoPhoneWindow);

	_ui->callBar->setCUserProfile(cUserProfile);//VOXOX CHANGE Rolando 04-29-09

	// Create account monitor
	_qtIMAccountMonitor.reset( new QtIMAccountMonitor(0, cUserProfile) );

	//Create the profilebar
	_ui->profileBar->init(&_cWengoPhone, cUserProfile, _qtIMAccountMonitor.get());

	//VOXOX CHANGE Rolando 03-20-09
	bool showProfileBar = config.getShowProfileBar();

#if defined(OS_MACOSX)
	// VOXOX -ASV- 09.08.2009: we use a toolbar to handle the profile bar 
	// so the toggle button on Mac is activated
	_toolBarToggle = _wengoPhoneWindow->addToolBar(tr("Profile Bar"));
	_toolBarToggle->addWidget(_ui->profileBar);
	_toolBarToggle->setMovable(false);
	_toolBarToggle->toggleViewAction()->setEnabled(false);
	_toolBarToggle->setVisible( showProfileBar );
	bringMainWindowToFront();
#endif
	
	//VOXOX CHANGE Rolando 03-25-09
	bool showCallBar = config.getShowCallBar();
	_ui->callBar->setVisible(showCallBar);

	// Systray
	SAFE_CONNECT_RECEIVER_TYPE(_qtIMAccountMonitor.get(), SIGNAL(imAccountAdded(QString)),
		_qtSystray, SLOT(updateSystrayIcon()), Qt::QueuedConnection);
	SAFE_CONNECT_RECEIVER_TYPE(_qtIMAccountMonitor.get(), SIGNAL(imAccountUpdated(QString)),
		_qtSystray, SLOT(updateSystrayIcon()), Qt::QueuedConnection);
	SAFE_CONNECT_RECEIVER_TYPE(_qtIMAccountMonitor.get(), SIGNAL(imAccountRemoved(QString)),
		_qtSystray, SLOT(updateSystrayIcon()), Qt::QueuedConnection);

	_qtSystray->setTrayMenu();
	_qtSystray->updateSystrayIcon();

	_qtFileTransfer = new QtFileTransfer(this, _cWengoPhone.getWengoPhone().getCoIpManager());

	//menu
	_qtToolBar->userProfileIsInitialized();
	_userIsLogged = true;	//VOXOX - JRT - 2009.07.12 
	


	//VOXOX CHANGE by Rolando 2009.05.05 if Loading Login Window is still inserted in stacked widget then destroy it
	if(_qtVoxOxLoadingLogin) {		
		_qtVoxOxLoadingLogin->deleteLater();		
	}

	//VOXOX CHANGE by Rolando 2009.05.05 if Error Message Login Window is still inserted in stacked widget then destroy it
	if(_qtVoxOxLoginMessage) {		
		_qtVoxOxLoginMessage->deleteLater();		
	}

	//VOXOX CHANGE by Rolando 2009.05.05 if Login Window is still inserted in stacked widget then destroy it
	if(_qtVoxOxLogin) {		
		_qtVoxOxLogin->deleteLater();		
	}

	//VOXOX CHANGE by Rolando - 2009.06.03 
	if(!_chatWindow){
		cUserProfile->initChat();
	}

	//VOXOX CHANGE by Rolando 2009.05.05 set visible the status bar
	setVisibleStatusBar(true);
	_ui->stackedWidget->setCurrentWidget(_ui->mainPage);


	if(config.getWizardAutoStart()){
		_qtVoxWindowManager->showWizardWindow();//VOXOX - CJC - 2009.07.09 
	}
}

void QtWengoPhone::proxyNeedsAuthenticationEventHandler(NetworkProxyDiscovery & sender, NetworkProxy networkProxy) {
	typedef PostEvent1<void (NetworkProxy networkProxy), NetworkProxy> MyPostEvent;
	MyPostEvent * event =
		new MyPostEvent(boost::bind(&QtWengoPhone::proxyNeedsAuthenticationEventHandlerThreadSafe, this, _1), networkProxy);
	postEvent(event);
}

void QtWengoPhone::wrongProxyAuthenticationEventHandler(NetworkProxyDiscovery & sender, NetworkProxy networkProxy) {
	typedef PostEvent1<void (NetworkProxy networkProxy), NetworkProxy> MyPostEvent;
	MyPostEvent * event =
		new MyPostEvent(boost::bind(&QtWengoPhone::proxyNeedsAuthenticationEventHandlerThreadSafe, this, _1), networkProxy);
	postEvent(event);
}

void QtWengoPhone::proxyNeedsAuthenticationEventHandlerThreadSafe(NetworkProxy networkProxy) {
	static QtHttpProxyLogin * httpProxy =
		new QtHttpProxyLogin(getWidget(),
			networkProxy.getServer(), networkProxy.getServerPort());

	int ret = httpProxy->show();

	if (ret == QDialog::Accepted) {
		NetworkProxy myNetworkProxy;
		myNetworkProxy.setServer(httpProxy->getProxyAddress());
		myNetworkProxy.setServerPort(httpProxy->getProxyPort());
		myNetworkProxy.setLogin(httpProxy->getLogin());
		myNetworkProxy.setPassword(httpProxy->getPassword());

		NetworkProxyDiscovery::getInstance().setProxySettings(myNetworkProxy);
	}
}

void QtWengoPhone::closeWindow() {
	_wengoPhoneWindow->hide();
}

void QtWengoPhone::languageChanged() {
//#if defined(OS_MACOSX)
	//fixMacOSXMenus();
//#endif
	LOG_DEBUG("retranslate main window ui");
	_ui->retranslateUi(_wengoPhoneWindow);
	_qtToolBar->retranslateUi();


	if (_qtBrowserWidget) {
		_ui->tabWidget->setTabText(_qtBrowserWidgetTabIndex, tr("Apps"));
	}


	if (_qtHistoryWidget) {
		_qtHistoryWidget->retranslateUi();
	}
        
         _ui->menuWengo->setTitle(tr("File"));
}

void QtWengoPhone::showAddContact(ContactInfo contactInfo) {

	ensureVisible();

	//TODO THIS SHOULD CALL THE ADD CONTACT WINDOW, NOT THE PROFILE DETAILS WINDOWS

	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {

		//FIXME this method should not be called if no UserProfile has been set
		ContactProfile contactProfile;
		//QtProfileDetails qtProfileDetails(*_cWengoPhone.getCUserProfileHandler().getCUserProfile(),
		//	contactProfile, _wengoPhoneWindow, tr("Add a Contact"));

		////FIXME to remove when wdeal will be able to handle SIP presence
		//if (contactInfo.group == "WDeal") {
		//	qtProfileDetails.setHomePhone(QUrl::fromPercentEncoding(QByteArray(contactInfo.sip.c_str())));
		//} else {
		//	qtProfileDetails.setWengoName(QUrl::fromPercentEncoding(QByteArray(contactInfo.wengoName.c_str())));
		//}
		/////

		//if (contactInfo.group == "WDeal") {
		//	qtProfileDetails.setFirstName(QUrl::fromPercentEncoding(QByteArray(contactInfo.wdealServiceTitle.c_str())));
		//} else {
		//	qtProfileDetails.setFirstName(QUrl::fromPercentEncoding(QByteArray(contactInfo.firstname.c_str())));
		//}

		//qtProfileDetails.setLastName(QUrl::fromPercentEncoding(QByteArray(contactInfo.lastname.c_str())));
		//qtProfileDetails.setCountry(QUrl::fromPercentEncoding(QByteArray(contactInfo.country.c_str())));
		//qtProfileDetails.setCity(QUrl::fromPercentEncoding(QByteArray(contactInfo.city.c_str())));
		//qtProfileDetails.setState(QUrl::fromPercentEncoding(QByteArray(contactInfo.state.c_str())));
		//qtProfileDetails.setGroup(QUrl::fromPercentEncoding(QByteArray(contactInfo.group.c_str())));
		//qtProfileDetails.setWebsite(QUrl::fromPercentEncoding(QByteArray(contactInfo.website.c_str())));

		//if (qtProfileDetails.show()) {
			_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().addContact(contactProfile);
		//}
	}
}

void QtWengoPhone::bringMainWindowToFront() {
	ensureVisible();
}

void QtWengoPhone::ensureVisible() {
	_wengoPhoneWindow->activateWindow();
	_wengoPhoneWindow->showNormal();
	_wengoPhoneWindow->raise();
}

void QtWengoPhone::installQtBrowserWidget() {
	_qtBrowserWidget = new QtBrowserWidget(*this);

	_qtBrowserWidgetTabIndex = _ui->tabWidget->insertTab(_ui->tabWidget->count()-1,_qtBrowserWidget->getWidget(), tr("Apps"));
	_qtBrowserWidget->loadDefaultURL();
}

void QtWengoPhone::uninstallQtBrowserWidget() {

	if(_qtBrowserWidget) 
	{
		if ( _qtBrowserWidgetTabIndex >= 0 )	//VOXOX - JRT - 2009.09.21 
		{
			_ui->tabWidget->widget(_qtBrowserWidgetTabIndex)->layout()->removeWidget(_qtBrowserWidget->getWidget());
			_ui->tabWidget->removeTab(_qtBrowserWidgetTabIndex);
			_qtBrowserWidgetTabIndex = -1;		//VOXOX - JRT - 2009.09.21 
		}

		OWSAFE_DELETE(_qtBrowserWidget);
	}
}

void QtWengoPhone::setCurrentUserProfileEventHandlerSlot()
{
	_qtToolBar->tryingToConnect();
}


//VOXOX CHANGE by Rolando 2009.05.05 this method adds a login window to stackedwidget and shows it
void QtWengoPhone::showLoginWindow(std::string profileName, bool isARetry)
{
	if(!_qtVoxOxLogin){
		_qtVoxOxLogin = new QtVoxOxLogin(getWidget(), this, _cWengoPhone.getCUserProfileHandler());
		int value = _ui->stackedWidget->count();
		_ui->stackedWidget->insertWidget(_ui->stackedWidget->count(), _qtVoxOxLogin);
		value = _ui->stackedWidget->count();
		SAFE_CONNECT(_qtVoxOxLogin, SIGNAL(destroyed(QObject*)), SLOT(slotQtVoxOxLoginDestroyed(QObject*)));
		SAFE_CONNECT(_qtVoxOxLogin, SIGNAL(acceptLoginClicked(QString)), SLOT(acceptLoginButtonClicked(QString)));
		
	}

	if(!getWidget()->isVisible()){//VOXOX CHANGE by Rolando 2009.05.05 it shows main window if this was not visible because of loading automatically
		getWidget()->setVisible(true);
	}
	_qtVoxOxLogin->load(profileName, isARetry);
	setVisibleStatusBar(false);//VOXOX CHANGE by Rolando 2009.05.05 it shows status bar if this was not visible because of login design GUI


	_ui->stackedWidget->setCurrentWidget(_qtVoxOxLogin);	
}

//VOXOX CHANGE by Rolando 2009.05.05 it removes Login Window from StackedWidget when this window is destroyed
void QtWengoPhone::slotQtVoxOxLoginDestroyed(QObject* object){

	int value = _ui->stackedWidget->count();
	_ui->stackedWidget->removeWidget(_qtVoxOxLogin);
	value = _ui->stackedWidget->count();
	_qtVoxOxLogin = NULL;
}

//VOXOX CHANGE by Rolando 2009.05.05 it login button was clicked it destroy Login Window
void QtWengoPhone::acceptLoginButtonClicked(QString profileName){
	_qtVoxOxLogin->deleteLater();
}

//VOXOX CHANGE by Rolando 2009.05.05 it shows loading window until user account was or not validated
void QtWengoPhone::showLoadingLoginMessage(QString profileName){
	
	if(!_qtVoxOxLoadingLogin){
		_qtVoxOxLoadingLogin = new QtVoxOxLoadingLogin(0, this, _cWengoPhone.getCUserProfileHandler());
		_qtVoxOxLoadingLogin->setProfileName(profileName);
		int value = _ui->stackedWidget->count();
		_ui->stackedWidget->insertWidget(_ui->stackedWidget->count(), _qtVoxOxLoadingLogin);
		value = _ui->stackedWidget->count();
		SAFE_CONNECT(_qtVoxOxLoadingLogin, SIGNAL(destroyed(QObject*)), SLOT(slotQtVoxOxLoadingLoginDestroyed(QObject*)));		
		
	}

	setVisibleStatusBar(false);

	if(!getWidget()->isVisible()){//VOXOX CHANGE by Rolando 2009.05.05 it shows main window if this was not visible because of loading automatically
		getWidget()->setVisible(true);
	}
	_ui->stackedWidget->setCurrentWidget(_qtVoxOxLoadingLogin);

}

//VOXOX CHANGE by Rolando 2009.05.05 it shows a error message if ocurred an error when user was validating his account
void QtWengoPhone::showLoginMessageWindow()
{
	if(!_qtVoxOxLoginMessage){
		_qtVoxOxLoginMessage = new QtVoxOxLoginMessage(0, this, _cWengoPhone.getCUserProfileHandler());
		int value = _ui->stackedWidget->count();
		_ui->stackedWidget->insertWidget(_ui->stackedWidget->count(), _qtVoxOxLoginMessage);
		value = _ui->stackedWidget->count();
		SAFE_CONNECT(_qtVoxOxLoginMessage, SIGNAL(destroyed(QObject*)), SLOT(slotQtVoxOxLoginMessageDestroyed(QObject*)));
		SAFE_CONNECT(_qtVoxOxLoginMessage, SIGNAL(tryAgainButton(std::string)), SLOT(tryAgainSlot(std::string)));
		
	}
	setVisibleStatusBar(false);

	if(!getWidget()->isVisible()){
		getWidget()->setVisible(true);
	}

	_ui->stackedWidget->setCurrentWidget(_qtVoxOxLoginMessage);

	if(_qtVoxOxLoadingLogin) {//VOXOX CHANGE by Rolando 2009.05.05 it destroys loading window if happened an error when user was validating like no network or password wrong
		_qtVoxOxLoadingLogin->deleteLater();		
	}
	
}

//VOXOX CHANGE by Rolando 2009.05.05
QtVoxOxLoginMessage * QtWengoPhone::getLoginMessageWindow(){
	if(_qtVoxOxLoginMessage){
		return _qtVoxOxLoginMessage;
	}
	else{
		return NULL;
	}
}

//VOXOX CHANGE by Rolando 2009.05.05
QtVoxOxLogin * QtWengoPhone::getLoginWindow(){
	if(_qtVoxOxLogin){
		return _qtVoxOxLogin;
	}
	else{
		return NULL;
	}
}

//VOXOX CHANGE by Rolando 2009.05.05 if button "Try Again" was clicked then shows a Login Window
void QtWengoPhone::tryAgainSlot(std::string profileName){
	
	if(!_qtVoxOxLogin){
		_qtVoxOxLogin = new QtVoxOxLogin(0, this, _cWengoPhone.getCUserProfileHandler());
		int value = _ui->stackedWidget->count();
		_ui->stackedWidget->insertWidget(_ui->stackedWidget->count(), _qtVoxOxLogin);
		value = _ui->stackedWidget->count();
		SAFE_CONNECT(_qtVoxOxLogin, SIGNAL(destroyed(QObject*)), SLOT(slotQtVoxOxLoginDestroyed(QObject*)));
		SAFE_CONNECT(_qtVoxOxLogin, SIGNAL(acceptLoginClicked()), SLOT(acceptLoginButtonClicked()));	
	}
	_qtVoxOxLogin->load(profileName, true);

	setVisibleStatusBar(false);

	if(!getWidget()->isVisible()){
		getWidget()->setVisible(true);
	}

	_ui->stackedWidget->setCurrentWidget(_qtVoxOxLogin);
}

//VOXOX CHANGE by Rolando 2009.05.05 this method is call when Login Error Message Window was destroyed and removes it from stacked widget
void QtWengoPhone::slotQtVoxOxLoginMessageDestroyed(QObject*){

	int value = _ui->stackedWidget->count();
	_ui->stackedWidget->removeWidget(_qtVoxOxLoginMessage);
	value = _ui->stackedWidget->count();
	_qtVoxOxLoginMessage = NULL;
}

//VOXOX CHANGE by Rolando 2009.05.05 this method is call when Loading Login Window was destroyed and removes it from stacked widget
void QtWengoPhone::slotQtVoxOxLoadingLoginDestroyed(QObject*){
	int value = _ui->stackedWidget->count();
	_ui->stackedWidget->removeWidget(_qtVoxOxLoadingLogin);
	value = _ui->stackedWidget->count();
	_qtVoxOxLoadingLogin = NULL;

}

void QtWengoPhone::setVisibleStatusBar(bool visible){
	_qtStatusBar->setVisible(visible);
}
		
void QtWengoPhone::notifyContactManagerContactAdded( const std::string& contactId, int qtId )	//VOXOX - JRT - 2009.09.23 
{
	getQtVoxWindowManager()->notifyContactManagerContactAdded( contactId, qtId );
}

void QtWengoPhone::UpdateViewGroupBy( QtEnumSortOption::SortOption groupOption )
{
	getConfig().set( Config::GENERAL_CONTACT_GROUPING_KEY, groupOption );

	if ( getQtContactList() )
	{
		getQtContactList()->updatePresentation();
	}
}

void QtWengoPhone::UpdateViewSortBy( QtEnumSortOption::SortOption sortOption )
{
	getConfig().set( Config::GENERAL_CONTACT_SORT_KEY, sortOption );

	if ( getQtContactList() )
	{
		getQtContactList()->updatePresentation();
	}
}

Config& QtWengoPhone::getConfig()
{
	return ConfigManager::getInstance().getCurrentConfig();
}

CContactList* QtWengoPhone::getCContactList()
{
	CContactList* result = false;

	CUserProfile* cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();

	if ( cUserProfile )
	{
		result = &(cUserProfile->getCContactList());
	}

	return result;
}
