/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* CLASS DESCRIPTION 
* @author Chris Jimenez C 
* @date 2009.08.14
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtUMWindow.h"
#include "QtUMChat.h"	//VOXOX - JRT - 2009.10.12 - For access to Chat-specific methods.
#include "ui_UMWindow.h"

#include <qtutil/SafeConnect.h>
#include "QtUMHelper.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/contactlist/ContactProfile.h>

#include <model/profile/AvatarList.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/contactlist/QtContactList.h>
#include <presentation/qt/toaster/QtChatToaster.h>
#include <presentation/qt/statusbar/QtStatusBar.h>
#include <presentation/qt/contactlist/QtContactActionManager.h>
#include <presentation/qt/QtToolBar.h>
#include <presentation/qt/QtVoxWindowManager.h>
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.05.29 

#include <qtutil/CloseEventFilter.h>	
#include <qtutil/WidgetUtils.h>
#include <qtutil/Widget.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
//
#include <sound/Sound.h>
#include <sound/AudioDeviceManager.h>


#include <presentation/qt/messagebox/QtVoxMessageBox.h>	//VOXOX - CJC - 2009.10.06 

#if defined(OS_WINDOWS)
	#include <windows.h>
#endif


//VOXOX CHANGE CJC Chat window other constructor to be able to support multiple chat widgets
QtUMWindow::QtUMWindow(QWidget * parent, CChatHandler & cChatHandler, QtWengoPhone & qtWengoPhone)
	: QMainWindow(parent),
	_cChatHandler(cChatHandler),
	_qtWengoPhone(qtWengoPhone) 
{
	

	_ui = new Ui::UMWindow();
	_ui->setupUi(this);

	createToolButtons();//VOXOX - CJC - 2009.08.17 
	
	_qtStatusBar = new QtStatusBar(_ui->statusBar);	//Setup StatusBar

	_qtUMHelper = new QtUMHelper(_cChatHandler,_cChatHandler.getCUserProfile().getCContactList());

	SAFE_CONNECT(_qtUMHelper, SIGNAL(addUMItemSignal(QtUMBase *,bool)),    SLOT(addUMItemSlot(QtUMBase *,bool )));
	SAFE_CONNECT(_qtUMHelper, SIGNAL(setTyping(QtUMBase*,bool)),    SLOT(setTypingSlot(QtUMBase*,bool)));
	SAFE_CONNECT(_qtUMHelper, SIGNAL(messageReceivedSignal(QtUMBase *)),    SLOT(messageReceivedSlot(QtUMBase *)));
	SAFE_CONNECT(_qtUMHelper, SIGNAL(statusChangedSignal(QtUMBase *)),    SLOT(statusChangedSlot(QtUMBase *)));

	SAFE_CONNECT(_qtUMHelper, SIGNAL(contactAddedSignal(QtUMBase *,const QString &,const QString &)),    SLOT(contactAddedSlot(QtUMBase *,const QString &,const QString &)));

	SAFE_CONNECT(_qtUMHelper, SIGNAL(contactRemovedSignal(QtUMBase *,const QString &,const QString &)),    SLOT(contactRemovedSlot(QtUMBase *,const QString &,const QString &)));
	
	_qtUMItemList = new QtUMItemList(_qtWengoPhone.getCWengoPhone());//VOXOX CHANGE by Rolando - 2009.08.17
	SAFE_CONNECT(_qtUMItemList, SIGNAL(itemClosed(QString)), SLOT(closeCurrentUMItemSlot(QString)));//VOXOX CHANGE by Rolando - 2009.08.27 
	SAFE_CONNECT(_qtUMItemList, SIGNAL(itemClicked(QString)), SLOT(itemClickedSlot(QString)));//VOXOX CHANGE by Rolando - 2009.08.28 
	SAFE_CONNECT(_qtUMItemList, SIGNAL(groupClicked(QString)), SLOT(groupClickedSlot(QString)));//VOXOX - CJC - 2009.09.24 
	SAFE_CONNECT(_qtUMItemList, SIGNAL(groupClosed(QString)), SLOT(groupClosedSlot(QString)));//VOXOX CHANGE by Rolando - 2009.08.27 

	CloseEventFilter * closeEventFilter = new CloseEventFilter(this, SLOT(closeWindow()));
	this->installEventFilter(closeEventFilter);

	if(_qtUMItemList){
		_ui->dockWidget->setWidget(_qtUMItemList);
		LOG_DEBUG("QtUMItemList added");
	}
	_windowsHasClose = true;

	buildChatStatusMenus();			//VOXOX - JRT - 2009.06.12 

	_qtWengoPhone.setChatWindow(this);


#if defined(OS_MACOSX)
	fixMacOsChatMenu();
#endif

	
}

QtUMWindow::~QtUMWindow() 
{

	OWSAFE_DELETE(_qtUMHelper);
	OWSAFE_DELETE(_qtUMItemList);
}


void QtUMWindow::createToolButtons(){


	readMainWindowSettings();

	#ifdef OS_WINDOWS//VOXOX CHANGE by Rolando - 2009.07.10 
		_ui->leftButtonFrame->layout()->setSpacing(0);//VOXOX CHANGE by Rolando - 2009.07.10 
		_ui->leftButtonFrame->layout()->setMargin(0);//VOXOX CHANGE by Rolando - 2009.07.10
		_ui->smsButton->setMinimumSize(58,38);//VOXOX CHANGE by Rolando - 2009.07.10 
		_ui->smsButton->setMaximumSize(58,38);//VOXOX CHANGE by Rolando - 2009.07.10 
		_ui->sendFileButton->setMinimumSize(78,38);//VOXOX CHANGE by Rolando - 2009.07.10 
		_ui->sendFileButton->setMaximumSize(78,38);//VOXOX CHANGE by Rolando - 2009.07.10 
	#endif

	//VOXOX CHANGE CJC - add images to tool buttons
	_ui->chatButton->setImages		   (":/pics/chat/btnChatButton.png",	  ":/pics/chat/btnChatButtonPress.png",		 "", "", ":/pics/chat/btnChatButtonCheck.png"		);
	_ui->callButton->setImages		   (":/pics/chat/btnCallButton.png",	  ":/pics/chat/btnCallButtonPress.png",		 "", "", ":/pics/chat/btnCallButtonCheck.png"		);
	_ui->smsButton->setImages		   (":/pics/chat/btnSMSButton.png",		  ":/pics/chat/btnSMSButtonPress.png",		 "", "", ":/pics/chat/btnSMSButtonCheck.png"		);
	_ui->sendFileButton->setImages	   (":/pics/chat/btnSendFileButton.png",  ":/pics/chat/btnSendFileButtonPress.png",	 "", "", ":/pics/chat/btnSendFileButtonCheck.png"	);

	_ui->emailButton->setImages		   (":/pics/chat/btnEmailButton.png",	  ":/pics/chat/btnEmailButtonPress.png",	 "", "", ":/pics/chat/btnEmailButtonCheck.png"		);
	_ui->faxButton->setImages		   (":/pics/chat/btnFaxButton.png",		  ":/pics/chat/btnFaxButtonPress.png",		 "", "", ":/pics/chat/btnFaxButtonCheck.png"		);
	_ui->phoneSettingsButton->setImages(":/pics/chat/btnPhoneSettings.png",	  ":/pics/chat/btnPhoneSettingsPress.png",	 "", "", ":/pics/chat/btnPhoneSettingsCheck.png"	);
	_ui->profileButton->setImages	   (":/pics/chat/btnProfileSettings.png", ":/pics/chat/btnProfileSettingsPress.png", "", "", ":/pics/chat/btnProfileSettingsCheck.png"	);

	_ui->buttonFrame->setStyleSheet(QString("QFrame{background-image: url(:/pics/chat/btnBg.png); border:0;}"));

	SAFE_CONNECT(_ui->chatButton,	  SIGNAL(clicked()), SLOT(startChat())		);
	SAFE_CONNECT(_ui->smsButton,	  SIGNAL(clicked()), SLOT(startSMS())		);
	SAFE_CONNECT(_ui->emailButton,	  SIGNAL(clicked()), SLOT(startEmail())		);
	SAFE_CONNECT(_ui->callButton,	  SIGNAL(clicked()), SLOT(startCall())		);
	SAFE_CONNECT(_ui->sendFileButton, SIGNAL(clicked()), SLOT(startSendFile())	);
	SAFE_CONNECT(_ui->faxButton, SIGNAL(clicked()), SLOT(startSendFax())	);
	SAFE_CONNECT(_ui->profileButton,  SIGNAL(clicked()), SLOT(startProfile())	);
	SAFE_CONNECT(_ui->phoneSettingsButton,  SIGNAL(clicked()), SLOT(startPhoneSettings())	);

}




//VOXOX - CJC - 2009.05.05 Events gets triggered when user close the chat window, it will close all the tabs
void QtUMWindow::closeWindow() {


	QStringList itemListKeys = _qtUMItemList->getItemListKeys();//VOXOX - CJC - 2009.09.02
	QStringList::iterator iterator = itemListKeys.begin();
	for (iterator = itemListKeys.begin(); iterator != itemListKeys.end(); ++iterator){
		QString key = *iterator;
		QtUMBase * item = _qtUMHelper->getUMItemByKey(key);//VOXOX - CJC - 2009.09.04 This is the same as the close window slot, but we cant call it, because it could be recoursive
		if(item){

			if(item->getMode()!=QtEnumUMMode::UniversalMessageCall && item->getMode()!=QtEnumUMMode::UniversalMessageConferenceCall){
				if(!item->isGroupChat()){
					_qtUMItemList->removeItem(key);
				}else{
					_qtUMItemList->removeGroup(key);
				}
				_ui->centralStakedWidget->removeWidget(item->getWidget());
				_qtUMHelper->deleteUMItemByKey(key);
			}else{
				_qtUMHelper->hangUpCall(key);
			}

		}
	}

	saveMainWindowSettings();
	saveDockWidgetSettings();

	////VOXOX - CJC - 2009.09.03 IF widget is floating and window will close, dock back to widget
	if(_ui->dockWidget->isFloating()){
		_ui->dockWidget->setFloating(false);
		 addDockWidget(Qt::LeftDockWidgetArea, _ui->dockWidget);
	}
	_windowsHasClose = true;
	 
}

void QtUMWindow::readDockWidgetSettings(){

	 QSettings settings("Telcentris", "VoxOx");
	 settings.beginGroup("DockWidget");
	 _ui->dockWidget->setFloating(settings.value("docked").toBool());
	 _ui->dockWidget->move(settings.value("pos", QPoint(200, 200)).toPoint());
	 addDockWidget((Qt::DockWidgetArea)settings.value("dockarea", Qt::LeftDockWidgetArea).toInt(), _ui->dockWidget);
	 settings.endGroup();

}

void QtUMWindow::saveDockWidgetSettings(){

	  QSettings settings("Telcentris", "VoxOx");
	  settings.beginGroup("DockWidget");
	  settings.setValue("dockarea", dockWidgetArea(_ui->dockWidget));
	  settings.setValue("docked", _ui->dockWidget->isFloating());
	  settings.setValue("pos", _ui->dockWidget->pos());
	  settings.endGroup();

}

void QtUMWindow::readMainWindowSettings(){

	 QSettings settings("Telcentris", "VoxOx");
	 settings.beginGroup("UMWindow");
	 resize(settings.value("size", QSize(596, 387)).toSize());
	 move(settings.value("pos", QPoint(300, 300)).toPoint());
	 settings.endGroup();

}

void QtUMWindow::saveMainWindowSettings(){

	 QSettings settings("Telcentris", "VoxOx");
	 settings.beginGroup("UMWindow");
	 settings.setValue("size", size());
	 settings.setValue("pos", pos());
	 settings.endGroup();

}

void QtUMWindow::hangUpCurrentCall()
{
	QString key = _qtUMItemList->getCurrentItemId();
	QtUMBase * base =  _qtUMHelper->getUMItemByKey(key);
	if(base){
		if(base->getMode() == QtEnumUMMode::UniversalMessageCall || base->getMode() == QtEnumUMMode::UniversalMessageConferenceCall){
			_qtUMHelper->hangUpCall(key);
		}
	}
}



void QtUMWindow::startChat()
{
	QString key = _qtUMItemList->getCurrentItemId();
	QString contactId =  _qtUMHelper->getContactIdByKey(key);
	if(contactId!=""){
		//Start chat
		QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_qtWengoPhone.getCWengoPhone());
		contactActionManager->chatContact(contactId);
	}
	
}
//VOXOX - CJC - 2009.06.16 
void QtUMWindow::startEmail(){
		
	QString key = _qtUMItemList->getCurrentItemId();
	QString contactId =  _qtUMHelper->getContactIdByKey(key);
	if(contactId!=""){
		QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_qtWengoPhone.getCWengoPhone());
		contactActionManager->emailContact(contactId);
	}
		
}

//VOXOX - CJC - 2009.06.16 
void QtUMWindow::startSMS(){
		
	QString key = _qtUMItemList->getCurrentItemId();
	QString contactId =  _qtUMHelper->getContactIdByKey(key);
	if(contactId!=""){
		QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_qtWengoPhone.getCWengoPhone());
		contactActionManager->smsContact(contactId);
	}

}


//VOXOX CHANGE CJC START CALL
void QtUMWindow::startCall(){
		
	QString key = _qtUMItemList->getCurrentItemId();
	QString contactId =  _qtUMHelper->getContactIdByKey(key);
	if(contactId!=""){
		//Start call
		QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_qtWengoPhone.getCWengoPhone());
		contactActionManager->callContact(contactId);
	}

	
}

void QtUMWindow::startSendFile()
{
	QString key = _qtUMItemList->getCurrentItemId();
	QString contactId =  _qtUMHelper->getContactIdByKey(key);
	if(contactId!=""){
		QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_qtWengoPhone.getCWengoPhone());
		contactActionManager->sendFileContact(contactId);
	}

}

//VOXOX - CJC - 2009.06.23 
void QtUMWindow::startSendFax()
{
	QString key = _qtUMItemList->getCurrentItemId();
	QString contactId =  _qtUMHelper->getContactIdByKey(key);
	if(contactId!=""){
		QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_qtWengoPhone.getCWengoPhone());
		contactActionManager->sendFaxContact(contactId);
	}

}

void QtUMWindow::startProfile()
{	
	QString key = _qtUMItemList->getCurrentItemId();
	QString contactId =  _qtUMHelper->getContactIdByKey(key);
	if(contactId!=""){
		QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_qtWengoPhone.getCWengoPhone());
		contactActionManager->profileContact(contactId);
	}


}

//VOXOX CHANGE by Rolando - 2009.07.09 
void QtUMWindow::startPhoneSettings(){
	QString key = _qtUMItemList->getCurrentItemId();
	QString contactId =  _qtUMHelper->getContactIdByKey(key);
	if(contactId!=""){
		QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_qtWengoPhone.getCWengoPhone());
		//TODO: FIX this when exists a way to open directly to edit phone settings.
		contactActionManager->profileContact(contactId);
	}
}

//VOXOX - CJC - 2009.09.08 TODO DONT REALLY KNOW WHAT DID DOES< LET JUST LEAVE IT HERE FOR NOW

void QtUMWindow::handleAddToChat()
{
	QString key = _qtUMItemList->getCurrentItemId();
	_qtUMHelper->handleAddToChat(key);	
}



//VOXOX CHANGE CJC - to support different menus on the status bar.
void QtUMWindow::setAddMenu(QMenu * addMenu)
{
	_qtStatusBar->setAddMenu(addMenu,"");
}

void QtUMWindow::setSettingsMenu(QMenu * settingsMenu){

	_qtStatusBar->setSettingsMenu(settingsMenu,"");
}




void QtUMWindow::showMinimized() {
	if(_windowsHasClose){
		readDockWidgetSettings();
	}
#if !defined(OS_MACOSX)

#ifdef OS_WINDOWS
	HWND topWindow = GetForegroundWindow();
#endif
	QMainWindow::showMinimized();
#ifdef OS_WINDOWS
	SetForegroundWindow(topWindow);
#endif

#else
	showNormal();
#endif
	_windowsHasClose = false;
}

void QtUMWindow::show() {
	if(_windowsHasClose){
		readDockWidgetSettings();
	}
	activateWindow();
	showNormal();
	_windowsHasClose = false;

}


void QtUMWindow::removeCallTabWidget(QString key){

	QtUMBase * item = _qtUMHelper->getUMItemByKey(key);
	if(item){	
			_qtUMItemList->removeItem(key);
			_ui->centralStakedWidget->removeWidget(item->getWidget());
			_qtUMHelper->deleteUMItemByKey(key);
	
	}
	if(checkWindowHasToClose()){
		close();
	}
}

// VOXOX -ASV- 2009.10.09: we need this to be an slot so we can trigger it when
// the shortcut key is pressed.
void QtUMWindow::closeActiveTab() {
	QString key = _qtUMItemList->getCurrentItemId();
	closeCurrentUMItemSlot(key);
}

// VOXOX -ASV- 07-14-2009
void QtUMWindow::fixMacOsChatMenu(){
	
	SAFE_CONNECT(this, SIGNAL(tabNumberChangedSignal(int)), SLOT(handleTabNumberChanged(int)));
	// Close action for the Chat Window
	_actionCloseChatWindow = new QAction(tr("Close &Window"), this);
	_actionCloseChatWindow->setShortcut(QKeySequence(tr("CTRL+Shift+W")));
	SAFE_CONNECT(_actionCloseChatWindow, SIGNAL(triggered()), SLOT(closeWindow()));	
	
	// Close action for the tabs inside the chat window
	_actionCloseTab = new QAction(tr("Close &Tab"), this);
	_actionCloseTab->setShortcut(QKeySequence(tr("CTRL+W")));
	SAFE_CONNECT(_actionCloseTab, SIGNAL(triggered()), SLOT(closeActiveTab()));	
	SAFE_CONNECT(this, SIGNAL(tabNumberChangedSignal(int)), SLOT(handleTabNumberChanged(int)));
	
	// We can log off from the chat window
	_actionLogOff = new QAction(tr("Sign &Out"), this);
	
	_actionLogOff->setShortcut(QKeySequence(tr("CTRL+Alt+Q")));
	
	SAFE_CONNECT(_actionLogOff, SIGNAL(triggered()), SLOT(logOff()));	
	
	// About menu
	_actionAbout = new QAction(tr("About"), this);
	SAFE_CONNECT(_actionAbout, SIGNAL(triggered()), SLOT(showAboutWindow()));
	
	_actionSettings = new QAction(tr("Settings"), this);
	SAFE_CONNECT(_actionSettings, SIGNAL(triggered()), SLOT(showSettingsWindow()));
	
	//VOXOX -ASV- 2009.12.03
	_actionUpdate = new QAction(tr("Check for Updates..."), this);
	_actionUpdate->setMenuRole(QAction::ApplicationSpecificRole);
	SAFE_CONNECT(_actionUpdate, SIGNAL(triggered()), SLOT(checkForUpdates()));	
	
	_chatMenu = new QMenuBar(this);
	
	_menuWengo = new QMenu(tr("File"), 0);
	_menuWengo->addAction(_actionCloseChatWindow);
	_menuWengo->addAction(_actionCloseTab);
	_menuWengo->addSeparator();
	_menuWengo->addAction(_actionLogOff);
	_menuWengo->addSeparator();	
	_menuWengo->addAction(_actionAbout);
	_menuWengo->addSeparator();	
	_menuWengo->addAction(_actionSettings);
	_menuWengo->addAction(_actionUpdate);
	
	_chatMenu->addMenu(_menuWengo);	
	_chatMenu->addMenu(_qtWengoPhone.getQtToolBar()->getMenuEdit());
	_chatMenu->addMenu(_qtWengoPhone.getQtToolBar()->getMenuContacts());
	_chatMenu->addMenu(_qtWengoPhone.getQtToolBar()->getMenuActions());
	_chatMenu->addMenu(_qtWengoPhone.getQtToolBar()->getMenuTools());
	_chatMenu->addMenu(_qtWengoPhone.getQtToolBar()->getMenuWindow());	
	_chatMenu->addMenu(_qtWengoPhone.getQtToolBar()->getMenuHelp());
	
}

void QtUMWindow::statusChangedSlot(QtUMBase * item)
{
	ContactProfile contactProfile = getCContactList().getContactProfile(item->getContactId().toStdString());

	QtContactPixmap::ContactPixmap normalStatus = getNormalContactPixmap( item->getContactId().toStdString() );//VOXOX CHANGE by Rolando - 2009.10.26 
	QtContactPixmap::ContactPixmap hoverStatus = getHoverContactPixmap( item->getContactId().toStdString() );//VOXOX CHANGE by Rolando - 2009.10.26 
	if(normalStatus != QtContactPixmap::ContactUnknown){//VOXOX CHANGE by Rolando - 2009.10.26 
		QPixmap normalPixmap = QtContactPixmap::getInstance()->getPixmap(normalStatus);//VOXOX CHANGE by Rolando - 2009.10.26 	
		QPixmap hoverPixmap = QtContactPixmap::getInstance()->getPixmap(hoverStatus);//VOXOX CHANGE by Rolando - 2009.10.26 	
		
		_qtUMItemList->updateItemStatusPixmap(item->getKey(),normalPixmap, hoverPixmap);//VOXOX CHANGE by Rolando - 2009.10.26 
	}
}

//VOXOX CHANGE by Rolando - 2009.10.26 - modified method name
//VOXOX - JRT - 2009.07.26 
QtContactPixmap::ContactPixmap QtUMWindow::getNormalContactPixmap( const std::string& contactId )
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;

	bool							 hasAvailableSipNumber	= false;
	bool							 blocked				= false;
	EnumPresenceState::PresenceState eState					= EnumPresenceState::PresenceStateUnknown;
	QtEnumIMProtocol::IMProtocol	 qtImProtocol			= QtEnumIMProtocol::IMProtocolUnknown; 

	Contact* contact = getCUserProfile()->getCContactList().getContactByKey( contactId );

	if ( contact )
	{
		hasAvailableSipNumber	= contact->hasPstnCall();
		blocked					= contact->isBlocked();
		eState					= contact->getPresenceState();
		qtImProtocol			= contact->getQtIMProtocol(); 

		status					= QtContactPixmap::determinePixmap( qtImProtocol, eState, hasAvailableSipNumber, blocked );
	}

	return status;
}

//VOXOX CHANGE by Rolando - 2009.10.26 - added method to get hover pixmap
QtContactPixmap::ContactPixmap QtUMWindow::getHoverContactPixmap( const std::string& contactId )
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;

	bool							 hasAvailableSipNumber	= false;
	bool							 blocked				= false;
	EnumPresenceState::PresenceState eState					= EnumPresenceState::PresenceStateUnknown;
	QtEnumIMProtocol::IMProtocol	 qtImProtocol			= QtEnumIMProtocol::IMProtocolUnknown; 

	Contact* contact = getCUserProfile()->getCContactList().getContactByKey( contactId );

	if ( contact )
	{
		hasAvailableSipNumber	= contact->hasPstnCall();
		blocked					= contact->isBlocked();
		eState					= contact->getPresenceState();
		qtImProtocol			= contact->getQtIMProtocol(); 

		status					= QtContactPixmap::determineHoverPixmap( qtImProtocol, eState, hasAvailableSipNumber, blocked );
	}

	return status;
}


void QtUMWindow::updateToolBarActions(QtEnumUMMode::Mode mode) {
	
	 _ui->chatButton->setChecked(false);
	 _ui->callButton->setChecked(false);
	 _ui->smsButton->setChecked(false);
	 _ui->sendFileButton->setChecked(false);
	 _ui->faxButton->setChecked(false);
	 _ui->emailButton->setChecked(false);
	 _ui->profileButton->setChecked(false);

	 	switch (mode) {
			case QtEnumUMMode::UniversalMessageChat:
				 _ui->chatButton->setChecked(true);
				break;
			case QtEnumUMMode::UniversalMessageFileTransfer:
				  _ui->sendFileButton->setChecked(true);
				break;
			case QtEnumUMMode::UniversalMessageChatToEmail:
				_ui->emailButton->setChecked(true);
				break;
			case QtEnumUMMode::UniversalMessageChatToSMS:
				_ui->smsButton->setChecked(true);
				break;
			case QtEnumUMMode::UniversalMessageCall:
				_ui->callButton->setChecked(true);
				break;
			case QtEnumUMMode::UniversalMessageConferenceCall:
				_ui->callButton->setChecked(true);
				break;
			case QtEnumUMMode::UniversalMessageFax:
				 _ui->faxButton->setChecked(true);
				break;
			default:
				break;
		}

	
}



//void QtUMWindow::incomingGroupChatInviteEventHandler( IMChatSession& sender, GroupChatInfo& gcInfo )
//{
////	incomingGroupChatInviteSignal( &sender, &gcInfo );
////	typedef ThreadEvent2<void (std::string contactId), std::string> MyThreadEvent;
////	typedef ThreadEvent2<void (IMChatSession sender, GroupChatInfo gcInfo), IMChatSession, GroupChatInfo> MyThreadEvent;
////	typedef ThreadEvent1<void (GroupChatInfo gcInfo), GroupChatInfo> MyThreadEvent;
//	typedef PostEvent1<void (GroupChatInfo gcInfo), GroupChatInfo> MyPostEvent;
//	MyPostEvent * event = new MyPostEvent(boost::bind(&QtUMWindow::incomingGroupChatInviteSlot, this, _1), gcInfo);
//	QCoreApplication::postEvent( this, event);
//}

//VOXOX - JRT - 2009.06.16  - I don't this belongs here, but will put it here for now.
//This needs be in UI thread.
//void QtUMWindow::incomingGroupChatInviteSlot( const IMChatSession sender, const GroupChatInfo gcInfo )
void QtUMWindow::incomingGroupChatInvite( const GroupChatInfo& gcInfo )
{


	QString invitationText = QString("%1 has invited you to a group chat.  Would you like to join").arg(QString::fromStdString(gcInfo.getWho().c_str()));
	QtVoxMessageBox box(_qtWengoPhone.getWidget());
	box.setWindowTitle("VoxOx - Chat Room  invitation");
	box.setText(invitationText);
	box.addButton(tr("&Accept"), QMessageBox::AcceptRole);
	box.addButton(tr("&Deny"), QMessageBox::RejectRole);
	int buttonClicked = box.exec();


	if (buttonClicked == QMessageBox::AcceptRole) 
	{
		const_cast<GroupChatInfo&>(gcInfo).setAcceptedInvite( true );
	}else{
		const_cast<GroupChatInfo&>(gcInfo).setAcceptedInvite( false );
	}
		
	gcInfo.respondToInviteEvent( gcInfo );
}


QString QtUMWindow::getPixmapByMode(QtEnumUMMode::Mode mode) {
	QString path = "";
	switch (mode) {	
		case QtEnumUMMode::UniversalMessageFileTransfer:
				path =  ":/pics/chat/tabs/tab_icon_sendfile_on.png";
			break;
		case QtEnumUMMode::UniversalMessageChatToEmail:
				path =  ":/pics/chat/tabs/tab_icon_email_on.png";
			break;
		case QtEnumUMMode::UniversalMessageChatToSMS:
				path =  ":/pics/chat/tabs/tab_icon_sms_on.png";
			break;
		case QtEnumUMMode::UniversalMessageCall:
				path =  ":/pics/chat/tabs/tab_icon_phone_on.png";
			break;
		case QtEnumUMMode::UniversalMessageConferenceCall:
				path =  ":/pics/chat/tabs/tab_icon_phone_on.png";
			break;
		case QtEnumUMMode::UniversalMessageFax:
			path =   ":/pics/chat/tabs/tab_icon_sendfax_on.png";
			break;
		default:
			break;
	}
	return path;
}



void QtUMWindow::messageReceivedSlot(QtUMBase * item) {
			
	if (!isActiveWindow()) {
		if (!isVisible()) {
			// Make sure the window appears on the taskbar,
			// otherwise it won't flash...
			if(_ui->centralStakedWidget->count() > 0 ){//VOXOX CHANGE by Rolando - 2009.06.11 
				//TODO: CHECK IF WE HAVE SOMETHING ONTHE WINDOW
				showMinimized();//VOXOX CHANGE by Rolando - 2009.06.11 
			}
		}
		#if defined (OS_WINDOWS)
			WidgetUtils::flashWindow(this);//VOXOX CHANGE by Rolando - 2009.06.10 
		#endif
		//VOXOX - CJC - 2009.05.26 
		playIncomingChatSound();
	}
	if(_ui->centralStakedWidget->currentWidget()!= item->getWidget()){
		if (isMinimized()) {
				_ui->centralStakedWidget->setCurrentWidget(item->getWidget());
		} else {
			_qtUMItemList->startBlinkingItem(item->getKey());//VOXOX CHANGE by Rolando - 2009.08.28
		}
	}			
}


void QtUMWindow::playIncomingChatSound(){
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if(config.getNotificationPlaySoundOnIncomingChat()){
		std::string soundFile = config.getAudioIncomingChatFile();
		Sound::play(soundFile, AudioDeviceManager::getInstance().getDefaultOutputDevice());
	}
}



//VOXOX - CJC - 2009.05.09 This will get trigger when we open a chat sesion manually, or a chat session get opens
// It could be a normal chat session or a chat to email, or chat sms session that is comming from ask.im.voxox.com
void QtUMWindow::imChatSessionCreatedHandler(IMChatSession * imChatSession) {

	_qtUMHelper->handleIMChatSessionCreated(imChatSession);
	
}


void QtUMWindow::sendMessageCreatedHandler(IMChatSession * imChatSession,std::string & message){//VOXOX - CJC - 2009.10.14 

	_qtUMHelper->sendMessageToChat(imChatSession,QString::fromStdString(message));
	
}




//VOXOX - CJC - 2009.05.07 Add support for Chat to Email
void QtUMWindow::imChatToEmailSessionCreatedHandler(IMChatSession * imChatSession,std::string & email) {


	_qtUMHelper->handleUMEmailSessionCreated(imChatSession,QString::fromStdString(email));

}

//VOXOX - CJC - 2009.05.19 Add chat to sms session
void QtUMWindow::imChatToSMSSessionCreatedHandler(IMChatSession * imChatSession,std::string & number) {
	
	_qtUMHelper->handleUMSMSSessionCreated(imChatSession,QString::fromStdString(number));

}

void QtUMWindow::setTypingSlot(QtUMBase * item,bool typing){
	//VOXOX - CJC - 2009.08.26 Verify is item is current
	if(item->getWidget() == _ui->centralStakedWidget->currentWidget()){
		updateWindowTitle(item,typing);
	}	
}

void QtUMWindow::addUMItemSlot(QtUMBase * item, bool userCreated){
	bool showSelected = false;
	if(addUMItem(item)){
		if(_qtUMItemList->getItemListKeys().count() == 0 || userCreated == true){//VOXOX - CJC - 2009.09.22 Check if this is the first one so we always selected it
			showSelected = true;
		}
		addUMItemToUMList(item,showSelected);
		updateWindowTitle(item,false);//VOXOX - JRT - 2009.09.18 
		updateToolBarActions(item->getMode());
		buildPlusMenu(item->getKey());//VOXOX - CJC - 2009.09.23 
		if(userCreated){
			//TODO ADD HERE CODE FOR CONTACT LIST
			_ui->centralStakedWidget->setCurrentWidget(item->getWidget());
//			updateWindowTitle(item,false);		//VOXOX - JRT - 2009.09.18 
			show();
		}else{

			if (isActiveWindow()) {
				
				_qtUMItemList->startBlinkingItem(item->getKey());
				
				playIncomingChatSound();
			} else {
				if (!isVisible()) {
					// Make sure the window appears on the taskbar,
					// otherwise it won't flash...
					showMinimized();
				}

				if (isMinimized()) {
					//CHANGE THIS ITEM TO CURRENT ITEM
				}

				WidgetUtils::flashWindow(this);//VOXOX CHANGE by ASV 07-31-2009: fix icon bounce when any other window is active				
				playIncomingChatSound();
				if(item->getMode()== QtEnumUMMode::UniversalMessageChat){//VOXOX - CJC - 2009.08.30 Do we want only toaster for chat?
					showToaster(item->getContactId(),item->getSenderName());
				}
			}
		}
		item->setVisible(showSelected);
	}

}



bool QtUMWindow::addUMItem(QtUMBase * item){

	if(!existInWindow(item)){
		addUMItemToWindow(item);
		return true;
	}
	return false;
	
}

void QtUMWindow::groupClickedSlot(QString key){//VOXOX - CJC - 2009.09.08 This slot triggers when a group or contact inside group is clicked, TODO: use it when doing group chat functionality
	
	QtUMBase * item = _qtUMHelper->getUMItemByKey(key);
	if(item){
		updateToolBarActions(item->getMode());
		updateWindowTitle(item,false);
		_ui->centralStakedWidget->setCurrentWidget(item->getWidget());
		_qtUMItemList->stopBlinkingItem(item->getKey());
		item->setVisible(true);
		buildPlusMenu(key);//VOXOX - CJC - 2009.09.23 
	}

}

void QtUMWindow::addUMItemToUMList(QtUMBase * item , bool selectedOnInit){
	
	QString avatarData = QString::fromStdString(AvatarList::getInstance().getDefaultContactAvatarPicture(QtEnumIMProtocol::IMProtocolVoxOx).getData());

	QPixmap normalPx = QPixmap();//VOXOX CHANGE by Rolando - 2009.10.26 
	QPixmap hoverPx = QPixmap();//VOXOX CHANGE by Rolando - 2009.10.26 

	Contact * contactFrom = getCContactList().getContactList().getContactByKey(item->getContactId().toStdString());
	if(contactFrom){
		avatarData = QString::fromStdString(contactFrom->getIcon().getData());
		if(item->getMode() == QtEnumUMMode::UniversalMessageChat){//VOXOX - CJC - 2009.09.01 Only if this is chat
			//QtContactPixmap::ContactPixmap status = getContactPixmap( item->getContactId().toStdString() );
			QtContactPixmap::ContactPixmap normalStatus = getNormalContactPixmap( item->getContactId().toStdString() );//VOXOX CHANGE by Rolando - 2009.10.26  - method name modified
			QtContactPixmap::ContactPixmap hoverStatus = getHoverContactPixmap( item->getContactId().toStdString() );//VOXOX CHANGE by Rolando - 2009.10.26  - get hover pixmap
			
			if(normalStatus != QtContactPixmap::ContactUnknown){//VOXOX CHANGE by Rolando - 2009.10.26 
				normalPx = QtContactPixmap::getInstance()->getPixmap(normalStatus);	//VOXOX CHANGE by Rolando - 2009.10.26 
				hoverPx = QtContactPixmap::getInstance()->getPixmap(hoverStatus);//VOXOX CHANGE by Rolando - 2009.10.26 
			}
			else if(!contactFrom->getIMAccount().empty())
			{
				QtEnumIMProtocolMap & rMap = QtEnumIMProtocolMap::getInstance();
				normalPx = QPixmap(QString::fromStdString(rMap.findByQtProtocol(contactFrom->getIMAccount().getQtProtocol())->getIconPath()));//VOXOX CHANGE by Rolando - 2009.10.26  		
				hoverPx = QPixmap(QString::fromStdString(rMap.findByQtProtocol(contactFrom->getIMAccount().getQtProtocol())->getIconPath()));//VOXOX CHANGE by Rolando - 2009.10.26 		
			}
		}else{
			normalPx = QPixmap(getPixmapByMode(item->getMode()));//VOXOX CHANGE by Rolando - 2009.10.26 
			hoverPx = QPixmap(getPixmapByMode(item->getMode()));//VOXOX CHANGE by Rolando - 2009.10.26 
		}
	}
	if(!item->isGroupChat()){
		_qtUMItemList->addItem(item->getContactId(), item->getKey(), "", item->getSenderName(), avatarData, normalPx, hoverPx, selectedOnInit);//VOXOX CHANGE by Rolando - 2009.10.26 
	}else{
		_qtUMItemList->addGroup(item->getKey(),item->getSenderName(),selectedOnInit);//VOXOX - CJC - 2009.09.24 
	}
	if(!selectedOnInit){
		_qtUMItemList->startBlinkingItem(item->getKey());
	}

}

//VOXOX - CJC - 2009.10.02 
void QtUMWindow::contactAddedSlot(QtUMBase * item,const QString & contactId,const QString & contactKey){
	
	QString avatarData = QString::fromStdString(AvatarList::getInstance().getDefaultContactAvatarPicture(QtEnumIMProtocol::IMProtocolVoxOx).getData());

	QPixmap normalPx = QPixmap();//VOXOX CHANGE by Rolando - 2009.10.26 
	QPixmap hoverPx = QPixmap();//VOXOX CHANGE by Rolando - 2009.10.26 
	QString displayName = contactId;
	Contact * contactFrom = getCContactList().getContactList().getContactByKey(contactId.toStdString());
	if(contactFrom){
		avatarData = QString::fromStdString(contactFrom->getIcon().getData());
		displayName = QString::fromStdString(contactFrom->getDisplayName());
		if(item->getMode() == QtEnumUMMode::UniversalMessageChat){//VOXOX - CJC - 2009.09.01 Only if this is chat
			//QtContactPixmap::ContactPixmap status = getContactPixmap( contactId.toStdString() );
			QtContactPixmap::ContactPixmap normalStatus = getNormalContactPixmap( item->getContactId().toStdString() );//VOXOX CHANGE by Rolando - 2009.10.26 - method name modified
			QtContactPixmap::ContactPixmap hoverStatus = getHoverContactPixmap( item->getContactId().toStdString() );//VOXOX CHANGE by Rolando - 2009.10.26 - method name modified
			if(normalStatus != QtContactPixmap::ContactUnknown){
				normalPx = QtContactPixmap::getInstance()->getPixmap(normalStatus);	//VOXOX CHANGE by Rolando - 2009.10.26 
				hoverPx = QtContactPixmap::getInstance()->getPixmap(hoverStatus);	//VOXOX CHANGE by Rolando - 2009.10.26 
			}
			else if(!contactFrom->getIMAccount().empty())
			{
				QtEnumIMProtocolMap & rMap = QtEnumIMProtocolMap::getInstance();
				normalPx = QPixmap(QString::fromStdString(rMap.findByQtProtocol(contactFrom->getIMAccount().getQtProtocol())->getIconPath()));//VOXOX CHANGE by Rolando - 2009.10.26 
				hoverPx = QPixmap(QString::fromStdString(rMap.findByQtProtocol(contactFrom->getIMAccount().getQtProtocol())->getIconPath()));//VOXOX CHANGE by Rolando - 2009.10.26  		
			}
		}else{
			normalPx = QPixmap(getPixmapByMode(item->getMode()));//VOXOX CHANGE by Rolando - 2009.10.26
			hoverPx = QPixmap(getPixmapByMode(item->getMode()));//VOXOX CHANGE by Rolando - 2009.10.26
		}
	}

	if(item->isGroupChat()){
		//VOXOX CHANGE by Rolando - 2009.10.23 - parameter contactId added
		_qtUMItemList->addItemToGroup(contactId, contactKey, item->getKey(), displayName, avatarData, normalPx, hoverPx, true);//VOXOX CHANGE by Rolando - 2009.10.26 
	}
}

//VOXOX - CJC - 2009.10.02 
void QtUMWindow::contactRemovedSlot(QtUMBase * item,const QString & contactId,const QString & contactKey){
	
	//QString avatarData = QString::fromStdString(AvatarList::getInstance().getDefaultContactAvatarPicture(QtEnumIMProtocol::IMProtocolVoxOx).getData());

	//QPixmap px = QPixmap();
	//QString displayName = contactId;
	//Contact * contactFrom = getCContactList().getContactList().getContactByKey(contactId.toStdString());
	//if(contactFrom){
	//	avatarData = QString::fromStdString(contactFrom->getIcon().getData());
	//	displayName = QString::fromStdString(contactFrom->getDisplayName());
	//	if(item->getMode() == QtEnumUMMode::UniversalMessageChat){//VOXOX - CJC - 2009.09.01 Only if this is chat
	//		QtContactPixmap::ContactPixmap status = getContactPixmap( contactId.toStdString() );
	//		if(status != QtContactPixmap::ContactUnknown){
	//			px = QtContactPixmap::getInstance()->getPixmap(status);	
	//		}
	//		else if(!contactFrom->getIMAccount().empty())
	//		{
	//			QtEnumIMProtocolMap & rMap = QtEnumIMProtocolMap::getInstance();
	//			px = QPixmap(QString::fromStdString(rMap.findByQtProtocol(contactFrom->getIMAccount().getQtProtocol())->getIconPath()));//VOXOX CHANGE by Rolando - 2009.08.31 		
	//		}
	//	}else{
	//		px = QPixmap(getPixmapByMode(item->getMode()));
	//	}
	//}

	if(item->isGroupChat()){
		_qtUMItemList->removeItem(contactKey);
	}
}




//VOXOX - CJC - 2009.09.04 This method should only be call, when the user clicks on the X of the tab.
void QtUMWindow::closeCurrentUMItemSlot(QString itemKey){
	QtUMBase * item = _qtUMHelper->getUMItemByKey(itemKey);
	if(item){
		if(item->getMode()!=QtEnumUMMode::UniversalMessageCall && item->getMode()!=QtEnumUMMode::UniversalMessageConferenceCall){
			_qtUMItemList->removeItem(itemKey);
			_ui->centralStakedWidget->removeWidget(item->getWidget());
			_qtUMHelper->deleteUMItemByKey(itemKey);
		}else{
			_qtUMHelper->hangUpCall(itemKey);
		}
	}
	if(checkWindowHasToClose()){
		close();
	}

}


//VOXOX - CJC - 2009.09.04 This method should only be call, when the user clicks on the X of the tab.
void QtUMWindow::groupClosedSlot(QString itemKey){
	QtUMBase * item = _qtUMHelper->getUMItemByKey(itemKey);
	if(item){
		if(item->isGroupChat()){
			_qtUMItemList->removeGroup(itemKey);
			_ui->centralStakedWidget->removeWidget(item->getWidget());
			_qtUMHelper->deleteUMItemByKey(itemKey);
		}
	}
	if(checkWindowHasToClose()){
		close();
	}

}

void QtUMWindow::itemClickedSlot(QString itemKey){

	QtUMBase * item = _qtUMHelper->getUMItemByKey(itemKey);
	if(item){
		updateToolBarActions(item->getMode());
		updateWindowTitle(item,false);
		_ui->centralStakedWidget->setCurrentWidget(item->getWidget());
		_qtUMItemList->stopBlinkingItem(item->getKey());
		item->setVisible(true);
		buildPlusMenu(itemKey);//VOXOX - CJC - 2009.09.23 
	}
}

bool QtUMWindow::checkWindowHasToClose(){

	QStringList itemListKeys = _qtUMItemList->getItemListKeys();//VOXOX - CJC - 2009.09.02
	QStringList::iterator iterator = itemListKeys.begin();
	return iterator == itemListKeys.end();
}




bool QtUMWindow::addUMItemToWindow(QtUMBase * item){
	
 	_ui->centralStakedWidget->addWidget(item->getWidget());

	return true;
}



bool QtUMWindow::exist(QtUMBase * item){

	return _qtUMHelper->exist(item) || existInWindow(item);
}



bool QtUMWindow::existInWindow(QtUMBase * item){

	if(_ui->centralStakedWidget->indexOf(item->getWidget())==-1){
		return false;
	}

	return true;
}

void QtUMWindow::updateWindowTitle(QtUMBase * item, bool isTyping){
	
	QString tittle = getTittle(item);
	if(isTyping){
		tittle += QString(" is typing");
	}
	setWindowTitle(tittle);
}


QString QtUMWindow::getTittle(QtUMBase * item){//VOXOX - CJC - 2009.08.26 Convenient Method

	QString senderName = item->getSenderName();
	QString type = QtEnumUMMode::toString(item->getMode());
	return type+" "+senderName;

}


void QtUMWindow::addSendFileWidgetTab(const IMContact & from) 
{
	_qtUMHelper->createUMSendFile(from);	
}
//VOXOX - CJC - 2009.06.23 
void QtUMWindow::addSendFaxWidgetTab(std::string faxNumber) 
{
	_qtUMHelper->createUMFax(QString::fromStdString(faxNumber));
}


//VOXOX - CJC - 2009.05.27 
void QtUMWindow::addCall(const QString & phoneNumber,QtContactCallListWidget * qtContactCallListWidget) {

	_qtUMHelper->createUMCall(phoneNumber,qtContactCallListWidget);
	
}


void QtUMWindow::showToaster(const QString & contactId, const QString & contactDisplayName) {
	QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
	if (qtContactList) {
		CContactList & cContactList = qtContactList->getCContactList();
		QPixmap avatar;
		QtEnumIMProtocol * protocol;
		QString message = contactDisplayName;
		QtChatToaster * toaster = NULL;
		Contact * contact =  getCContactList().getContactByKey(contactId.toStdString());
		if (contact) 
		{	
			toaster = new QtChatToaster();
			std::string data = getCContactList().getContactIconData( contactId.toStdString() );	//VOXOX - JRT - 2009.07.26 

			if (!data.empty()) 
			{
				avatar.loadFromData((uchar *) data.c_str(), data.size());
			}
			else
			{
				std::string data = AvatarList::getInstance().getDefaultContactAvatarPicture(contact->getQtIMProtocol()).getData();	//VOXOX - JRT - 2009.06.11 
				avatar.loadFromData((uchar*)data.c_str(), data.size());
			}
			QtEnumIMProtocolMap & rMap = QtEnumIMProtocolMap::getInstance();
			protocol  = rMap.findByQtProtocol(contact->getQtIMProtocol());		//VOXOX - JRT - 2009.07.02 
			toaster->setProtocolImage(QPixmap(protocol->getBigIconPath().c_str()));
		}
		if(toaster){
			toaster->setMessage(message);
			toaster->setPixmap(avatar);
			SAFE_CONNECT(toaster, SIGNAL(chatButtonClicked()), SLOT(show()));
			toaster->show();
		}
	}
}

//VOXOX - SEMR - 2009.06.28 change settings
void QtUMWindow::changeFontSettings(){
	//getChatWidget()->changeFont();
}

void QtUMWindow::spellCheckActive(){
	//VOXOX - SEMR - 2009.06.28  TODO
}
//VOXOX - SEMR - 2009.06.28 change chat theme from wizard Appearance
void QtUMWindow::changeTheme(){
	QtToolBar * qtToolBar = _qtWengoPhone.getQtToolBar();
	qtToolBar->showAppearanceConfig();
}
//VOXOX - SEMR - 2009.06.28 edit contact 
void QtUMWindow::editContact(){
	/*QString contactId = getActiveTabContactId();

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_qtWengoPhone.getCWengoPhone());
	contactActionManager->profileContact(contactId);

	updateToolBarActions();*/
}


void QtUMWindow::deleteContact(){
	//QString contactId = getActiveTabContactId();
	//if (contactId != ""){
	//	closeActiveTab();
	//	_qtWengoPhone.getQtContactList()->getCContactList().removeContact(contactId.toStdString());
	//	//_qtWengoPhone.getQtContactList()->updatePresentation();
	//}
}
void QtUMWindow::openSettings(){
	QtToolBar * qtToolBar = _qtWengoPhone.getQtToolBar();
	qtToolBar->showConfig();
}

void QtUMWindow::logOff() {
	_qtWengoPhone.logOff();
}

void QtUMWindow::showAboutWindow() {
	_qtWengoPhone.getQtToolBar()->showAbout();
}

void QtUMWindow::showSettingsWindow() {
	_qtWengoPhone.getQtVoxWindowManager()->showConfigWindow("");
}

// VOXOX -ASV- 2009.12.03
void QtUMWindow::checkForUpdates() {
	CUserProfile* cUserProfile = _qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile();

	if ( cUserProfile )
	{
		cUserProfile->checkForSoftUpdate( true );		//Manual
	}
}

//VOXOX - SEMR - 2009.06.28 Universal Status Menu
void QtUMWindow::buildChatStatusMenus() 
{
	buildPlusMenu(QString(""));
	buildGearMenu();
}

void QtUMWindow::addContactSlot(){
	QtToolBar * qtToolBar = _qtWengoPhone.getQtToolBar();
	qtToolBar->addCOntactSlot();
}
void QtUMWindow::buildPlusMenu(const QString & key)
{
	QMenu * plusMenu = new QMenu(this);
	QAction *   addContact			= new QAction("Add Contact",	this);
	SAFE_CONNECT(addContact, SIGNAL(triggered()), SLOT(addContactSlot()));

	QAction * 	callContact			= new QAction("Call", plusMenu);
	SAFE_CONNECT(callContact, SIGNAL(triggered()), SLOT(startCall()));

	QAction * 	sendIM				= new QAction("Send IM", plusMenu);
	SAFE_CONNECT(sendIM, SIGNAL(triggered()), SLOT(startChat()));

	QAction * 	sendTextMessage		= new QAction("Send SMS", plusMenu);
	SAFE_CONNECT(sendTextMessage, SIGNAL(triggered()), SLOT(startSMS()));

	QAction * 	sendEmail			= new QAction("Send Email", plusMenu);
	SAFE_CONNECT(sendEmail, SIGNAL(triggered()), SLOT(startEmail()));

	QAction * 	sendFile			= new QAction("Send File", plusMenu);
	SAFE_CONNECT(sendFile, SIGNAL(triggered()), SLOT(startSendFile()));

	QAction * 	sendFax				= new QAction("Send Fax", plusMenu);
	SAFE_CONNECT(sendFax, SIGNAL(triggered()), SLOT(startSendFax()));

	plusMenu->addAction(addContact);
	plusMenu->addSeparator();
	plusMenu->addAction(callContact);
	plusMenu->addAction(sendIM);
	plusMenu->addAction(sendTextMessage);
	plusMenu->addAction(sendEmail);
	plusMenu->addAction(sendFile);
	plusMenu->addAction(sendFax);

	addGroupChatAction( plusMenu,key );

	setAddMenu(plusMenu);
}

void QtUMWindow::buildGearMenu()
{
	QMenu* gearMenu		= new QMenu(this);
	
	QAction* spellCheck		= new QAction("Spell Check On / Off",	this);
	spellCheck->setEnabled(false);
	
	QAction* changeTheme	= new QAction("Change Theme",	this);
	SAFE_CONNECT(changeTheme, SIGNAL(triggered()), SLOT(changeTheme()));
	
	QAction* editContact	= new QAction("Edit Contact",   this);
	SAFE_CONNECT(editContact, SIGNAL(triggered()), SLOT(editContact()));
	
	QAction* deleteContact	= new QAction("Delete Contact", this);
	SAFE_CONNECT(deleteContact, SIGNAL(triggered()), SLOT(deleteContact()));
	
	
	QAction* settings		= new QAction("Settings",this);
	SAFE_CONNECT(settings, SIGNAL(triggered()), SLOT(openSettings()));

	gearMenu->addAction(spellCheck);
	gearMenu->addAction(changeTheme);
	gearMenu->addAction(editContact);
	gearMenu->addAction(deleteContact);
	gearMenu->addAction(settings);

	setSettingsMenu(gearMenu);
}

//VOXOX - JRT - 2009.07.08 - Add 'group chat', if needed.
void QtUMWindow::addGroupChatAction( QMenu* menu,const QString & key )
{
	QString idKey = key;

	if(idKey=="")
	{
		 idKey = _qtUMItemList->getCurrentItemId();
	}

	QtUMBase * item = _qtUMHelper->getUMItemByKey(idKey);

	if(item)
	{
		if(item->getMode() == QtEnumUMMode::UniversalMessageChat)
		{
			QtUMChat* chatItem = dynamic_cast<QtUMChat*>(item);	//VOXOX - JRT - 2009.10.12 
			QAction* groupChat = NULL;

			if(item->isGroupChat())
			{
				groupChat = new QAction("Add to Chat",this);
			}
			else
			{
				groupChat = new QAction("Start Group Chat",this);
			}

			SAFE_CONNECT(groupChat, SIGNAL(triggered()), SLOT(handleAddToChat()));
			menu->addAction( groupChat );

			groupChat->setEnabled( chatItem->canDoMultiChat() );	//VOXOX - JRT - 2009.10.12 
		}
	}
}


	/*if ( activeTabIsGroupChat() )
	{
		QAction* groupChat = new QAction("Add to Chat",this);
		SAFE_CONNECT(groupChat, SIGNAL(triggered()), SLOT(handleAddToChat()));

		menu->addAction( groupChat );
	}
	else if ( activeTabIsChat() )
	{
		QAction* groupChat = new QAction("Start Group Chat",this);
		SAFE_CONNECT(groupChat, SIGNAL(triggered()), SLOT(handleAddToChat()));

		menu->addAction( groupChat );
	}*/
//}


void QtUMWindow::updateClipboardActions()
{
	/*QTextEdit* textEdit = getActiveTextEdit();
	bool modifiable   = false;
	bool hasSelection = false;

	if (textEdit) 
	{
		hasSelection = !textEdit->textCursor().selectedText().isEmpty();
		modifiable = !textEdit->isReadOnly();
	}

	bool clipboardFilled = !QApplication::clipboard()->text().isEmpty();
	_ui->actionCut->setEnabled(modifiable && hasSelection);
	_ui->actionCopy->setEnabled(hasSelection);
	_ui->actionPaste->setEnabled(modifiable && clipboardFilled)*/;
}

void QtUMWindow::cut() {
	/*QTextEdit* textEdit = getActiveTextEdit();
	if (textEdit) {
		textEdit->cut();
	}*/
}

void QtUMWindow::copy() {
	/*QTextEdit* textEdit = getActiveTextEdit();
	if (textEdit) {
		textEdit->copy();
	}*/
}

void QtUMWindow::paste() {
	/*QTextEdit* textEdit = getActiveTextEdit();
	if (textEdit) {
		textEdit->paste();
	}*/
}

bool QtUMWindow::eventFilter(QObject* object, QEvent* event) {
	//if (!qobject_cast<QTextEdit*>(object)) {
	//	return false;
	//}

	//if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
	//	updateClipboardActions();
	//}
	return false;
}

//VOXOX - JRT - 2009.04.15 - for readability and maintainability
CUserProfile* QtUMWindow::getCUserProfile()
{
	return _qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile();
}

CContactList& QtUMWindow::getCContactList() const
{

	return _qtWengoPhone.getQtContactList()->getCContactList();

}
//End VoxOx

