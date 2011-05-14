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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtIMProfileWidget.h"

#include "ui_IMProfileWidget.h"

#include "QtIMAccountPresenceMenuManager.h"
#include <presentation/qt/QtPresenceMenuManager.h>
#include <presentation/qt/QtPresenceIMMenuManager.h>
#include <presentation/qt/QtPresenceSIPMenuManager.h>
#include <presentation/qt/QtIMAccountMonitor.h>
#include <presentation/qt/QtIMAccountHelper.h>
#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/QtVoxWindowManager.h>

#include <presentation/qt/profile/QtProfileDetails.h>
#include <presentation/qt/imaccount/QtIMAccountManager.h>
#include <presentation/PFactory.h>

#include <presentation/qt/config/QtWengoConfigDialog.h>
#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <thread/ThreadEvent.h>
#include <imwrapper/QtEnumIMProtocol.h>

#include <model/profile/UserProfile.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/webservices/url/WsUrl.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/PixmapMerging.h>

#include <qtutil/VoxOxFrame.h>

#include <qtutil/VoxOxToolTipLineEdit.h>

#include <QtGui/QMenu>

static const QString GLOBAL_STATUS_MESSAGE  = QString("Global");//VOXOX CHANGE by Rolando - 2009.06.29 
static const QString HISTORY_STATUS_MESSAGE = QString("History");//VOXOX CHANGE by Rolando - 2009.06.29 
static const QString VALID_STATUS_MESSAGE   = QString("valid");//VOXOX CHANGE by Rolando - 2009.06.29 
static const QString INVALID_STATUS_MESSAGE = QString("invalid");//VOXOX CHANGE by Rolando - 2009.06.29 

QtIMProfileWidget::QtIMProfileWidget(QWidget * parent)
	: QWidget(parent)
	, _cUserProfile(0)
	, _qtImAccountMonitor(0)
	, _qtWengoPhone(0){

	_ui = new Ui::IMProfileWidget();
	_ui->setupUi(this);	

	setVisible(false);
	
	SAFE_CONNECT(_ui->statusLabel,   SIGNAL(clicked()), SLOT(statusIMLabelClickedSlot()));//VOXOX CHANGE Rolando 03-20-09
	SAFE_CONNECT(_ui->dropDownLabel, SIGNAL(clicked()), SLOT(statusIMLabelClickedSlot()));//VOXOX CHANGE Rolando 03-20-09
	SAFE_CONNECT(_ui->usernameLabel, SIGNAL(clicked()), SLOT(statusIMLabelClickedSlot()));//VOXOX CHANGE Rolando 03-20-09

	SAFE_CONNECT(_ui->sipDropDownLabel,   SIGNAL(clicked()), SLOT(sipDropDownLabelClickedSlot()));//VOXOX CHANGE Rolando 03-20-09
//	SAFE_CONNECT(_ui->sipIconstatusLabel, SIGNAL(clicked()), SLOT(statusSIPLabelClickedSlot()));//VOXOX CHANGE Rolando 03-20-09
	SAFE_CONNECT(_ui->sipStatusLabel,     SIGNAL(clicked()), SLOT(statusSIPLabelClickedSlot()));//VOXOX CHANGE Rolando 03-20-09
	SAFE_CONNECT(_ui->voxoxPointsLabel,   SIGNAL(clicked()), SLOT(voxoxPointsLabelClickedSlot()));//VOXOX CHANGE Rolando 03-20-09

	SAFE_CONNECT(_ui->socialNetworkDropDownLabel, SIGNAL(clicked()), SLOT(socialNetworkDropDownLabelClickedSlot()));//VOXOX CHANGE Rolando 03-20-09
	SAFE_CONNECT(_ui->socialNetworkStatusLabel, SIGNAL(clicked()), SLOT(socialNetworkDropDownLabelClickedSlot()));//VOXOX CHANGE by Rolando - 2009.07.07 
	//SAFE_CONNECT(_ui->socialNetworkFrame, SIGNAL(mouseLeave()), SLOT(socialNetworkFrameLeaveSlot()));//VOXOX CHANGE by Rolando - 2009.07.07  
	//SAFE_CONNECT(_ui->socialNetworkFrame, SIGNAL(mouseEntered()), SLOT(socialNetworkFrameEnteredSlot()));//VOXOX CHANGE by Rolando - 2009.07.07 
	
	//_ui->socialNetworkFrame->hideBorder();//VOXOX CHANGE by Rolando - 2009.07.07 - by default we hide the border

	//SAFE_CONNECT(_ui->socialNetworkLineEdit, SIGNAL(currentTextChanged(QString)), SLOT(socialNetworkLineEditChangedSlot(QString)));//VOXOX CHANGE Rolando 03-20-09
	
	setNickNameLabel("Connecting...");//VOXOX CHANGE Rolando 03-20-09
	setVoxoxPointsLabel("");//VOXOX CHANGE Rolando 03-20-09
	setSipStatusLabel("");//VOXOX CHANGE Rolando 03-20-09

	_ui->voxoxPointsLabel->setToolTip(QString("This is your current balance. Click to add more credit."));//VOXOX CHANGE by Rolando - 2009.10.20 
	_ui->voxoxPointsLabel->setMinimumWidth(1);//VOXOX CHANGE by Rolando - 2009.05.13  - To be able to set ellipsis at the end of text when it is cropped by new size of window

	//LANGUAGE_CHANGE(this);	
	
	SAFE_CONNECT(_ui->socialNetworkLineEdit, SIGNAL(returnPressed()), SLOT(statusMessageTextChanged()));//VOXOX CHANGE Rolando 03-20-09
	SAFE_CONNECT(_ui->avatarButton, SIGNAL(clicked()), SLOT(changeAvatarClicked()));//VOXOX CHANGE Rolando 03-20-09

}

QtIMProfileWidget::~QtIMProfileWidget() {
	OWSAFE_DELETE(_ui);	
}

void QtIMProfileWidget::init(CUserProfile* cUserProfile, QtIMAccountMonitor* qtImAccountMonitor, QtWengoPhone * qtWengoPhone) {
	_cUserProfile = cUserProfile;
	_qtImAccountMonitor = qtImAccountMonitor;
	_qtWengoPhone = qtWengoPhone;

	//UserProfile changed event connection
	_cUserProfile->getUserProfile().profileChangedEvent += boost::bind(&QtIMProfileWidget::profileChangedEventHandler, this);

	updateIMStatusLabel();
	updateSIPStatusLabel();

	// IMAccountMonitor connections
	SAFE_CONNECT_TYPE(_qtImAccountMonitor, SIGNAL(imAccountAdded(QString)),   SLOT(addIMAccount(QString)),    Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(_qtImAccountMonitor, SIGNAL(imAccountRemoved(QString)), SLOT(removeIMAccount(QString)), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(_qtImAccountMonitor, SIGNAL(imAccountUpdated(QString)), SLOT(updateIMAccount(QString)), Qt::QueuedConnection);

	updateDefaultMessage();//VOXOX CHANGE by Rolando - 2009.07.03 

	updateLastStatusMessage();//VOXOX CHANGE by Rolando - 2009.07.03 
	
	updateWidgets();
}

void QtIMProfileWidget::updateDefaultMessage(){

	_ui->socialNetworkLineEdit->setToolTipDefaultText("What's on your mind?");
	
	//VOXOX CHANGE CJC STATUS MESSAGE
	if (!_cUserProfile->getUserProfile().getStatusMessage().empty()) {
		QString statusMessage = QString::fromUtf8(_cUserProfile->getUserProfile().getStatusMessage().c_str());
		_ui->socialNetworkLineEdit->setText(statusMessage);		
		if(!_statusMessageList.contains(statusMessage) && statusMessage != ""){
			_statusMessageList << statusMessage;		
		}
	}

	_ui->socialNetworkStatusLabel->setPixmap(QString(":/pics/profilebar/global_status_message.png"));//VOXOX CHANGE by Rolando - 2009.07.07 
	

}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::addIMAccount(QString imAccountId) {
	
	QString id = imAccountId;

	IMAccountToId::const_iterator imIter = _imAccountToId.find(id);
	if (imIter != _imAccountToId.end()) {
		LOG_WARN("there is already added this account id:" + id.toStdString());
		return;
	}

	SipAccountToId::const_iterator sipIter = _sipAccountToId.find(id);
	if (sipIter != _sipAccountToId.end()) {
		LOG_WARN("there is already added this account id:" + id.toStdString());
		return;
	}

	const IMAccount * account = _cUserProfile->getUserProfile().getIMAccountManager().getIMAccount(imAccountId.toStdString());

	if ( EnumIMProtocol::isSip( account->getProtocol() ) )	//VOXOX - JRT - 2009.07.02 
	{
		_sipAccountToId[id] = *account;
		updateSIPStatusLabel();
	}
	else
	{
		_imAccountToId[id] = *account;	
		updateIMStatusLabel();
	}
}
//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::addIMAccount(IMAccount account) {

	QString id = QString::fromStdString(account.getKey());

	IMAccountToId::const_iterator imIter = _imAccountToId.find(id);
	SipAccountToId::const_iterator sipIter = _sipAccountToId.find(id);

	if (imIter != _imAccountToId.end() || sipIter != _sipAccountToId.end()) {
		LOG_WARN("there is already an IMAccount with id:" + id.toStdString());
		return;
	}

	if ( EnumIMProtocol::isSip( account.getProtocol() ) )
	{
		_sipAccountToId[id] = account;
		updateSIPStatusLabel();
	}
	else
	{
		_imAccountToId[id] = account;
		updateIMStatusLabel();
	}
	
}
//VOXOX CHANGE Rolando 03-20-09
QPixmap QtIMProfileWidget::getStatus(IMAccount account, EnumPresenceState::PresenceState presenceState)
{
	QtEnumIMProtocolMap& rMap = QtEnumIMProtocolMap::getInstance();
	QString strPath = rMap.findByQtProtocol( account.getQtProtocol() )->getNetworkIconPath( presenceState ).c_str();	//VOXOX - JRT - 2009.05.29 

	return QPixmap( strPath );
}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::removeIMAccount(QString imAccountId) {

	IMAccountToId::iterator imIter = _imAccountToId.find(imAccountId);
	SipAccountToId::iterator sipIter = _sipAccountToId.find(imAccountId);

	if (imIter == _imAccountToId.end()) {
		
		if (sipIter == _sipAccountToId.end()) {
			LOG_WARN("Neither IM nor SIP Account removed because not exists one with id " + imAccountId.toStdString());
		}
		else{			
			_sipAccountToId.erase(sipIter);
			updateSIPStatusLabel();	
		}
	}
	else{		
		_imAccountToId.erase(imIter);
		updateIMStatusLabel();
	}

}
//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::updateIMAccount(QString imAccountId) {
	IMAccountToId::iterator imIter   = _imAccountToId.find(imAccountId);
	SipAccountToId::iterator sipIter = _sipAccountToId.find(imAccountId);

	if(_cUserProfile){
		const IMAccount * account = _cUserProfile->getUserProfile().getIMAccountManager().getIMAccount(imAccountId.toStdString());

		if (imIter == _imAccountToId.end()) {
			if (sipIter == _sipAccountToId.end()) {
				LOG_WARN("No updated, neither a SIP nor IM account exist with id " + imAccountId.toStdString());
				return;
			}
			else{//if was found a SIP account with this id

				//VOXOX CHANGE Marin add presence to sip account
				EnumPresenceState::PresenceState sipPresenceState;	
				if(_cUserProfile->getUserProfile().isSIPConnected()){
					sipPresenceState = _cUserProfile->getUserProfile().getSIPPresenceState();		
				}
				else{
					sipPresenceState = EnumPresenceState::PresenceStateOffline;
				}

				
				_sipAccountToId[imAccountId] = *account;
				updateSIPStatusLabel();
				return;
			}
			
		}
		else{//if was found a IMAccount with this id
			
			_imAccountToId[imAccountId] = *account;
			updateIMStatusLabel();
		}
	}
	
}
void QtIMProfileWidget::changeAvatarClicked() {

	//VOXOX CHANGE by Rolando - 2009.10.20 - added for task "Update: clicking on your avatar in the profile bar"
	QtProfileDetails * qtProfileDetails =  new QtProfileDetails(*_cUserProfile, _cUserProfile->getUserProfile(), this, tr("Edit My Profile"));
	////TODO UserProfile must be updated if QtProfileDetails was accepted	
	//qtProfileDetails->show();

	//_qtWengoPhone->getQtVoxWindowManager()->showProfileWindow();//VOXOX CHANGE by Rolando - 2009.10.20 - commented for task "Update: clicking on your avatar in the profile bar"
	qtProfileDetails->changeUserProfileAvatar();//VOXOX CHANGE by Rolando - 2009.10.20 - added for task "Update: clicking on your avatar in the profile bar"
	updateAvatar();
}

void QtIMProfileWidget::updateWidgets() {
	if (!_cUserProfile) {
		return;
	}
	
	updateAvatar();

	createIMAccounts();//VOXOX CHANGE Rolando 03-20-09
}

void QtIMProfileWidget::updateLastStatusMessage(){
	//VOXOX CHANGE by Rolando - 2009.06.29 
	QtEnumIMProtocolMap& rMap = QtEnumIMProtocolMap::getInstance();//VOXOX CHANGE by Rolando - 2009.06.29
	Config & config = ConfigManager::getInstance().getCurrentConfig();//VOXOX CHANGE by Rolando - 2009.06.29 
	QPixmap statusMessageProtocolPixmap;
	std::string lastKey = config.getLastStatusMessageAccountKey();
	std::string lastStatusMessage;
	bool lastKeyFound = false;//VOXOX CHANGE by Rolando - 2009.06.30 

	if(lastKey != ""){//VOXOX CHANGE by Rolando - 2009.06.29
		
		IMAccountList imAccountList = _cUserProfile->getUserProfile().getIMAccountManager().getIMAccountListCopy();		
		QtIMAccountHelper::QtIMAccountPtrVector imAccountPtrVector;
		QtIMAccountHelper::copyListToPtrVector(imAccountList, &imAccountPtrVector);
	//	std::sort(imAccountPtrVector.begin(), imAccountPtrVector.end(), QtIMAccountHelper::compareIMAccountPtrs);


		// Updates IMAccounts icons status
		QtIMAccountHelper::QtIMAccountPtrVector::iterator//VOXOX CHANGE by Rolando - 2009.06.29 
			it = imAccountPtrVector.begin(),//VOXOX CHANGE by Rolando - 2009.06.29 
			end = imAccountPtrVector.end();

		for (; it!=end; ++it) {		//VOXOX CHANGE by Rolando - 2009.06.29 
//			if((*it)->getProtocol() != EnumIMProtocol::IMProtocolWengo && (*it)->getProtocol() != EnumIMProtocol::IMProtocolSIP)
			if(!EnumIMProtocol::isSip( (*it)->getProtocol() ) )//VOXOX CHANGE by Rolando - 2009.07.15  
			{
				if((*it)->getKey() == lastKey){
					statusMessageProtocolPixmap = QPixmap(QString::fromStdString(rMap.findByQtProtocol( (*it)->getQtProtocol() )->getIconPath()));//VOXOX CHANGE by Rolando - 2009.06.29 
					_ui->socialNetworkStatusLabel->setPixmap(statusMessageProtocolPixmap);
					_currentIMAccountKey = QString::fromStdString(lastKey);
					lastKeyFound = true;
					break;
				}
			}
		}

		//VOXOX CHANGE by Rolando - 2009.06.29 
		lastStatusMessage = config.getLastStatusMessageDisplayed();//VOXOX CHANGE by Rolando - 2009.06.29 
		if(lastStatusMessage != "" && lastKeyFound){//VOXOX CHANGE by Rolando - 2009.06.29
			_ui->socialNetworkLineEdit->setText(QString::fromStdString(lastStatusMessage));
		}
	}
}


//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::createIMAccounts() {
	// Remove existing IMAccounts
	resetIMAccounts();

	// Sort accounts, using the same order as the meta presence
	IMAccountList imAccountList = _cUserProfile->getUserProfile().getIMAccountManager().getIMAccountListCopy();
	QtIMAccountHelper::QtIMAccountPtrVector imAccountPtrVector;
	QtIMAccountHelper::copyListToPtrVector(imAccountList, &imAccountPtrVector);
//JRT-XXX	std::sort(imAccountPtrVector.begin(), imAccountPtrVector.end(), QtIMAccountHelper::compareIMAccountPtrs);

	// Updates IMAccounts icons status
	QtIMAccountHelper::QtIMAccountPtrVector::iterator
		it = imAccountPtrVector.begin(),
		end = imAccountPtrVector.end();

	for (; it!=end; ++it) {		
//		if((*it)->getProtocol() == EnumIMProtocol::IMProtocolWengo || (*it)->getProtocol() == EnumIMProtocol::IMProtocolSIP)
		if( EnumIMProtocol::isSip( (*it)->getProtocol() ) )	//VOXOX - JRT - 2009.07.02 
		{
			_sipAccountToId[QString::fromStdString((*it)->getKey())] = *(*it);
		}
		else{
			_imAccountToId[QString::fromStdString((*it)->getKey())] = *(*it);
		}
	}
}

void QtIMProfileWidget::updateAvatar() {

	//std::string backgroundPixmapFilename = ":/pics/profilebar/avatar_background.png";
	//std::string foregroundPixmapData = _cUserProfile->getUserProfile().getIcon().getData();

	//_ui->avatarButton->setIcon(PixmapMerging::merge(foregroundPixmapData, backgroundPixmapFilename,Qt::KeepAspectRatio));//merge two images

	std::string foregroundPixmapData = _cUserProfile->getUserProfile().getIcon().getData();
	QPixmap foregroundPixmap;
	foregroundPixmap.loadFromData((uchar*) foregroundPixmapData.c_str(), foregroundPixmapData.size());
	_ui->avatarButton->setIcon(foregroundPixmap);

}

void QtIMProfileWidget::showImAccountManager() {
	QtIMAccountManager imAccountManager(_cUserProfile->getUserProfile(),
		true, this);
}

//VOXOX CHANGE by Rolando - 2009.06.29 
void QtIMProfileWidget::statusMessageTextChanged() {


	std::string statusMessage(_ui->socialNetworkLineEdit->realText().toUtf8().constData());

	if(_currentIMAccountKey != ""){//VOXOX CHANGE by Rolando - 2009.06.29 
		if(_currentIMAccountKey == GLOBAL_STATUS_MESSAGE || _currentIMAccountKey == HISTORY_STATUS_MESSAGE){//VOXOX CHANGE by Rolando - 2009.06.29 
			_cUserProfile->getUserProfile().setStatusMessage(statusMessage, NULL);//VOXOX CHANGE by Rolando - 2009.06.29 
		}
		else{
			IMAccountToId::const_iterator iter    = _imAccountToId.begin();//VOXOX CHANGE by Rolando - 2009.06.29 
			IMAccountToId::const_iterator iterEnd = _imAccountToId.end();//VOXOX CHANGE by Rolando - 2009.06.29 
			while (iter != iterEnd) //VOXOX CHANGE by Rolando - 2009.06.29 
			{
				if(iter->second.getKey() == _currentIMAccountKey.toStdString()){//VOXOX CHANGE by Rolando - 2009.06.29 
					IMAccount * account = _cUserProfile->getUserProfile().getIMAccountManager().getIMAccount(iter->second);//VOXOX CHANGE by Rolando - 2009.06.29 
					_cUserProfile->getUserProfile().setStatusMessage(statusMessage, account);//VOXOX CHANGE by Rolando - 2009.06.29 
				}
				++iter;
			}			
		}
	}
	else{
		_cUserProfile->getUserProfile().setStatusMessage(statusMessage, NULL);//VOXOX CHANGE by Rolando - 2009.06.29 
	}

	//_currentIMAccountKey = "";//VOXOX CHANGE by Rolando - 2009.06.29 
	//_ui->socialNetworkStatusLabel->setPixmap(QString(":/pics/profilebar/global_status_message.png"));//VOXOX CHANGE by Rolando - 2009.06.29 
	//

	Config & config = ConfigManager::getInstance().getCurrentConfig();//VOXOX CHANGE by Rolando - 2009.06.29 
	config.set(Config::LAST_STATUS_MESSAGE_ACCOUNT_KEY,_currentIMAccountKey.toStdString());//VOXOX CHANGE by Rolando - 2009.06.29 
	config.set(Config::LAST_STATUS_MESSAGE_DISPLAYED, statusMessage);//VOXOX CHANGE by Rolando - 2009.06.29 

	QString newText = QString::fromStdString(statusMessage);

	if(!_statusMessageList.contains(newText) && newText != ""){
		_statusMessageList << newText;		
	}
	//_ui->socialNetworkFrame->hideBorder();//VOXOX CHANGE by Rolando - 2009.07.07 - by default we hide the border
	_ui->socialNetworkStatusLabel->setFocus();
	
}

void QtIMProfileWidget::profileChangedEventHandler() {
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&QtIMProfileWidget::updateWidgets, this));
	PFactory::postEvent(event);
}

void QtIMProfileWidget::languageChanged() {
	_ui->retranslateUi(this);
	updateWidgets();
}

void QtIMProfileWidget::resetIMAccounts() //VOXOX CHANGE Rolando 03-20-09
{
	_imAccountToId.clear();	//VOXOX CHANGE Rolando 03-20-09
	_sipAccountToId.clear();//VOXOX CHANGE Rolando 03-20-09
	
}

void QtIMProfileWidget::reset() 
{
	resetIMAccounts();//VOXOX CHANGE Rolando 03-20-09

	_cUserProfile = 0;
	_qtImAccountMonitor = 0;
	_qtWengoPhone = 0;

	setNickNameLabel("Connecting...");//VOXOX CHANGE Rolando 03-20-09
	setVoxoxPointsLabel("");//VOXOX CHANGE Rolando 03-20-09
	setSipStatusLabel("");//VOXOX CHANGE Rolando 03-20-09
	
	_ui->socialNetworkLineEdit->setText(QString(""));//VOXOX CHANGE Rolando 03-20-09
	_statusMessageList.clear();//VOXOX CHANGE Rolando 03-20-09
}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::setNickNameLabel(QString displayName){
	if(displayName != ""){
		_nickName = displayName;	
	}
	
	updateText();
}
//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::setSipStatusLabel(QString sipStatusNumber){
	
	_sipNumber = sipStatusNumber;
	updateText();
}
//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::setVoxoxPointsLabel(QString voxoxPoints){
	_voxoxPoints = voxoxPoints;
	updateText();	
}
//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::statusIMLabelClickedSlot(){	

	// no presence when user got a SIPAccount without presence and no IMAccount
	if (_cUserProfile) {
	
		UserProfile & userProfile = _cUserProfile->getUserProfile();
		SipAccount * sipAccount = userProfile.getSipAccount();
		
		if (userProfile.getIMAccountManager().getIMAccountListCopy().size() <= 1) {
			if (!sipAccount || !sipAccount->isPIMEnabled()) {
				return;
			}
		}
	}
	////
	//createIMStatusMenu();//VOXOX CHANGE by Rolando - 2009.06.16 - deleted to show all the accounts including sipaccounts
	createGeneralAccountsMenu();//VOXOX CHANGE by Rolando - 2009.06.16 - added to show all the accounts including sipaccounts
	
}
//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::statusSIPLabelClickedSlot(){	

	sipDropDownLabelClickedSlot();//VOXOX CHANGE by Rolando - 2009.06.16 - now we show an action to go to phone settings window instead of show a menu with sip accounts

	//VOXOX CHANGE by Rolando - 2009.06.16 - commented lines to show a profile settings button
	/*// no presence when user got a SIPAccount without presence and no IMAccount
	if (_cUserProfile) {
	
		UserProfile & userProfile = _cUserProfile->getUserProfile();
		SipAccount * sipAccount = userProfile.getSipAccount();
		
		if (userProfile.getIMAccountManager().getIMAccountListCopy().size() <= 1) {
			if (!sipAccount || !sipAccount->isPIMEnabled()) {
				return;
			}
		}
	}
	////
	
	// by default we create the menu
	createSIPStatusMenu();*/
	//VOXOX CHANGE by Rolando - 2009.06.16 - end of commented lines

}

//VOXOX CHANGE by Rolando - 2009.06.11 
void QtIMProfileWidget::sipDropDownLabelClickedSlot(){
	QMenu * dropDownMenu = new QMenu(this);	
	
	QAction* action = dropDownMenu->addAction(tr("Phone Settings"), this, SLOT(openPhoneSettingsWindow()));

	QPoint p = _ui->sipDropDownLabel->pos();	
	p.setY(p.y() + _ui->sipDropDownLabel->height());
	dropDownMenu->popup(this->mapToGlobal(p));
}

//VOXOX CHANGE by Rolando - 2009.06.11 
void QtIMProfileWidget::openPhoneSettingsWindow(){
	QtWengoConfigDialog * settings = new QtWengoConfigDialog(_qtWengoPhone->getWidget(), _qtWengoPhone->getCWengoPhone());
	settings->showPhoneSettingsPage();
	settings->show();//VOXOX - CJC - 2009.07.03 
}

void QtIMProfileWidget::voxoxPointsLabelClickedSlot(){
	//WsUrl::showRatesPage();//VOXOX CHANGE by Rolando - 2009.10.20 
	if(_qtWengoPhone){//VOXOX CHANGE by Rolando - 2009.10.20 
		_qtWengoPhone->showHomeTab();//VOXOX CHANGE by Rolando - 2009.09.22 //VOXOX CHANGE by Rolando - 2009.10.20 
	}
}

//VOXOX CHANGE by Rolando - 2009.07.07 
//void QtIMProfileWidget::socialNetworkFrameEnteredSlot(){
//	/*_ui->socialNetworkFrame->setStyleSheet(QString("VoxOxFrame{ border: 1px solid #ababab; border-radius: 5px; }"));*/
//	_ui->socialNetworkFrame->showBorder();//VOXOX CHANGE by Rolando - 2009.07.07 
//}

//VOXOX CHANGE by Rolando - 2009.07.07 
//void QtIMProfileWidget::socialNetworkFrameLeaveSlot(){
//	_ui->socialNetworkFrame->hideBorder();//VOXOX CHANGE by Rolando - 2009.07.07
//}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::socialNetworkDropDownLabelClickedSlot(){	

	// by default we create the menu
	createSocialStatusMenu();

}
//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::createSIPStatusMenu() {
	QMenu * sipStatusMenu = new QMenu(this);	
	EnumPresenceState::PresenceState presenceState = _cUserProfile->getUserProfile().getSIPPresenceState();
	QtPresenceSIPMenuManager* manager = new QtPresenceSIPMenuManager(sipStatusMenu, _cUserProfile,_qtWengoPhone);
	QAction* action = sipStatusMenu->addAction(tr("Global Presence"));
	QPoint p = _ui->sipDropDownLabel->pos();	

	action->setEnabled(false);	

	manager->addPresenceActions(sipStatusMenu,presenceState);

	sipStatusMenu->addSeparator();
	addAccountsToMenu(sipStatusMenu, true);//true because we are going to add a sipaccount
	
	p.setY(p.y() + _ui->sipDropDownLabel->height());
	sipStatusMenu->popup(this->mapToGlobal(p));

}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::createIMStatusMenu() {
	QMenu * imStatusMenu = new QMenu(this);
	QtPresenceIMMenuManager* manager = new QtPresenceIMMenuManager(imStatusMenu, _cUserProfile,_qtWengoPhone);
	EnumPresenceState::PresenceState presenceState = _cUserProfile->getUserProfile().getIMPresenceState();

	QPoint p = _ui->dropDownLabel->pos();
	QAction* action = imStatusMenu->addAction(tr("Global Presence"));	
	action->setEnabled(false);	

	manager->addPresenceActions(imStatusMenu, presenceState);

	imStatusMenu->addSeparator();	
	addAccountsToMenu(imStatusMenu, false);//false because we are going to add a IMAccount
	
	p.setY(p.y() + _ui->dropDownLabel->height());
	imStatusMenu->popup(this->mapToGlobal(p));
	
}

//VOXOX CHANGE by Rolando - 2009.06.16 
void QtIMProfileWidget::createGeneralAccountsMenu(){

	QMenu * generalStatusMenu = new QMenu(this);
	QtPresenceMenuManager* manager = new QtPresenceMenuManager(generalStatusMenu, _cUserProfile,_qtWengoPhone);
	EnumPresenceState::PresenceState presenceState = _cUserProfile->getUserProfile().getPresenceState();

	if(presenceState == EnumPresenceState::PresenceStateMulti){
		if(_cUserProfile->getUserProfile().isVoxOxConnected()){
			presenceState = _cUserProfile->getUserProfile().getVoxOxIMPresenceState();
		}else{
			presenceState = EnumPresenceState::PresenceStateOffline;		
		}
	}
	else{
		if(!_cUserProfile->getUserProfile().isVoxOxConnected()){			
			presenceState = EnumPresenceState::PresenceStateOffline;		
		}
	}

	QPoint p = _ui->dropDownLabel->pos();
	QAction* action = generalStatusMenu->addAction(tr("Global Presence"));	
	action->setEnabled(false);	

	manager->addPresenceActions(generalStatusMenu, presenceState);

	generalStatusMenu->addSeparator();	
	addAccountsToMenu(generalStatusMenu, false);//false because we are going to add a IMAccount
	addAccountsToMenu(generalStatusMenu, true);//true because we are going to add a sipaccount

	p.setY(p.y() + _ui->dropDownLabel->height());
	generalStatusMenu->popup(this->mapToGlobal(p));
	

}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::addAccountsToMenu(QMenu * menu, bool isSIPAccount)
{
	QString login;
	QString key;
	QString errMsg;
	QIcon   icon;	
	QtIMAccountMonitor::IMAccountInfoAutoPtr info;
	QtIMAccountMonitor::ConnectionState connectionState;
	EnumPresenceState::PresenceState presenceState;
	bool imAccountEnable = true;

	if(isSIPAccount)
	{
		SipAccountToId::const_iterator iter    = _sipAccountToId.begin();
		SipAccountToId::const_iterator iterEnd = _sipAccountToId.end();

		while (iter != iterEnd) 
		{
			login			= QString::fromStdString((*iter).second.getLogin());
			key				= QString::fromStdString((*iter).second.getKey());
			errMsg			= "";

			updateSIPStatusLabel();
			presenceState = (*iter).second.getPresenceState();

			cleanLoginName( login );

			if((*iter).second.isConnected())
			{
				//icon = QIcon(getStatus((*iter).second,presenceState ));//VOXOX CHANGE by Rolando - 2009.06.16 
			}
			else
			{
				info			= _qtImAccountMonitor->getIMAccountInfo(key);
				connectionState = info->connectionState();
				presenceState	= EnumPresenceState::PresenceStateOffline;

				switch (connectionState) {
					case QtIMAccountMonitor::StateConnecting:
						login = QString("Connecting %1").arg(login);
						imAccountEnable = false;
						//icon = QIcon();//VOXOX CHANGE by Rolando - 2009.06.16 
						break;

					case QtIMAccountMonitor::StateFailure:
						login  = QString("There was a failure connecting %1").arg(login);
						//icon   = QIcon();//VOXOX CHANGE by Rolando - 2009.06.16 
						errMsg = info->message();
						break;
	
					default:;
						//icon = QIcon(getStatus((*iter).second,presenceState ));//VOXOX CHANGE by Rolando - 2009.06.16 
				}											
				
			}

			icon = QIcon(":/pics/profilebar/phone_icon.png");//VOXOX CHANGE by Rolando - 2009.06.16 

			addSIPAccountMenu(menu,login, key, icon,presenceState, errMsg);
			
			++iter;
		}
	}
	else
	{		
		updateIMStatusLabel();
		IMAccountToId::const_iterator iter    = _imAccountToId.begin();
		IMAccountToId::const_iterator iterEnd = _imAccountToId.end();
		while (iter != iterEnd) 
		{
			login			= QString::fromStdString((*iter).second.getLogin());
			key				= QString::fromStdString((*iter).second.getKey());
			imAccountEnable = true;
			presenceState	= (*iter).second.getPresenceState();
			errMsg			= "";

			cleanLoginName( login );

			if((*iter).second.isConnected())
			{
				QPixmap pix = getStatus((*iter).second, presenceState );
				icon = QIcon( pix );
			}
			else
			{
				info			= _qtImAccountMonitor->getIMAccountInfo(key);
				connectionState = info->connectionState();

				switch (connectionState) {
					case QtIMAccountMonitor::StateConnecting:
						login = QString("Connecting %1").arg(login);
						imAccountEnable = false;
						break;

					case QtIMAccountMonitor::StateFailure:
						errMsg = info->message();
						login  = QString("There was a failure connecting %1").arg(login);
						break;
				}	

				presenceState = EnumPresenceState::PresenceStateOffline;
				icon		  =  QIcon(getStatus((*iter).second,presenceState));
			}

			addIMAccountMenu(menu, login, key, icon, presenceState, imAccountEnable, errMsg);
			
			++iter;
		}
	}
	
}

// VOXOX CHANGE by ASV 06-05-2009: added to implement the functionality of the Edit Menu
VoxOxToolTipLineEdit * QtIMProfileWidget::getVoxOxToolTipLineEdit(){
	return _ui->socialNetworkLineEdit;
}

//VOXOX CHANGE by Rolando - 2009.06.29
void QtIMProfileWidget::createSocialStatusMenu() {

	QMenu *			socialStatusMenu			= new QMenu(this);
	QAction *		globalSocialStatusAction	= new QAction(0);
	QAction *		networkSocialStatusAction	= new QAction(0);
	QActionGroup *	actionsNetworksGroup		= new QActionGroup(this);
	QActionGroup * actionsHistoryGroup			= new QActionGroup(this);

	QtEnumIMProtocolMap& rMap = QtEnumIMProtocolMap::getInstance();
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	bool statusMessageRecoveredIsEmpty    = false;
	bool statusMessageAccountKeyRecovered = false;
	QString statusMessage;
	QString protocolPixmapPath;
	QString currentLineEditText;

	actionsNetworksGroup->setExclusive(true);
	actionsHistoryGroup->setExclusive(true);

	_socialStatusToAction.clear();
	_imContactKeyToAction.clear();
	

	if(_ui->socialNetworkLineEdit->text() != ""){//VOXOX CHANGE by Rolando - 2009.06.29 
		globalSocialStatusAction = socialStatusMenu->addAction(QIcon(QString(":/pics/profilebar/global_status_message.png")),_ui->socialNetworkLineEdit->realText(), this, SLOT(globalSocialStatusClicked()) );
		globalSocialStatusAction->setData(VALID_STATUS_MESSAGE);
		globalSocialStatusAction->setEnabled(true);
		globalSocialStatusAction->setCheckable(true);
	}
	else{//VOXOX CHANGE by Rolando - 2009.06.29 
		globalSocialStatusAction = socialStatusMenu->addAction(QIcon(QString(":/pics/profilebar/global_status_message.png")),"No Global Status Message Set", this, SLOT(globalSocialStatusClicked()) );
		globalSocialStatusAction->setData(INVALID_STATUS_MESSAGE);
		globalSocialStatusAction->setEnabled(true);
		globalSocialStatusAction->setCheckable(true);
	}

	actionsNetworksGroup->addAction(globalSocialStatusAction);

	socialStatusMenu->addSeparator();

	IMAccountToId::const_iterator iter    = _imAccountToId.begin();
	IMAccountToId::const_iterator iterEnd = _imAccountToId.end();
	while (iter != iterEnd) //VOXOX CHANGE by Rolando - 2009.06.29 
	{

		if(iter->second.isConnected()){//VOXOX CHANGE by Rolando - 2009.06.29 
			statusMessage = QString::fromStdString((*iter).second.getStatusMessage());
			//statusMessage = QString::fromStdString(_cUserProfile->getUserProfile().getStatusMessage());//VOXOX CHANGE by Rolando - 2009.06.29 
			protocolPixmapPath = QString::fromStdString(rMap.findByQtProtocol( iter->second.getQtProtocol() )->getIconPath());

			if(statusMessage == ""){//VOXOX CHANGE by Rolando - 2009.06.29 
				if(config.getLastStatusMessageAccountKey() == iter->second.getKey()){//VOXOX CHANGE by Rolando - 2009.06.29 
					statusMessageAccountKeyRecovered = true;
					statusMessage = QString::fromStdString(config.getLastStatusMessageDisplayed());//VOXOX CHANGE by Rolando - 2009.06.29 
					
					if(statusMessage == ""){//VOXOX CHANGE by Rolando - 2009.06.29
						statusMessageRecoveredIsEmpty = true;//VOXOX CHANGE by Rolando - 2009.06.29 
						statusMessage = "No Status Message Set";//VOXOX CHANGE by Rolando - 2009.06.29 
					}
				}
				else{
					statusMessage = "No Status Message Set";//VOXOX CHANGE by Rolando - 2009.06.29 
				}
				networkSocialStatusAction = socialStatusMenu->addAction(QIcon(protocolPixmapPath),statusMessage, this, SLOT(socialStatusClicked()) );
				if(statusMessageRecoveredIsEmpty){//VOXOX CHANGE by Rolando - 2009.06.29 
					networkSocialStatusAction->setData(INVALID_STATUS_MESSAGE);	//VOXOX CHANGE by Rolando - 2009.06.29 			
				}
				else{
					networkSocialStatusAction->setData(VALID_STATUS_MESSAGE);//VOXOX CHANGE by Rolando - 2009.06.29 
				}
				networkSocialStatusAction->setEnabled(true);
				networkSocialStatusAction->setCheckable(true);
				actionsNetworksGroup->addAction(networkSocialStatusAction);//VOXOX CHANGE by Rolando - 2009.06.29 
				/*if(statusMessageAccountKeyRecovered){//VOXOX CHANGE by Rolando - 2009.06.29 					
					networkSocialStatusAction->setChecked(config.getLastStatusMessageAccountKey() == iter->second.getKey());//VOXOX CHANGE by Rolando - 2009.06.29 
				}
				else{
					networkSocialStatusAction->setChecked(_currentIMAccountKey == QString::fromStdString(iter->second.getKey()));//VOXOX CHANGE by Rolando - 2009.06.29 
				}*/

				if(statusMessage == "No Status Message Set"){//VOXOX CHANGE by Rolando - 2009.06.29 
					statusMessage = "";//VOXOX CHANGE by Rolando - 2009.06.29 
				}
				
			}
			else{//VOXOX CHANGE by Rolando - 2009.06.29 
				if(config.getLastStatusMessageAccountKey() == iter->second.getKey()){//VOXOX CHANGE by Rolando - 2009.06.29 
					statusMessageAccountKeyRecovered = true;
					statusMessage = QString::fromStdString(config.getLastStatusMessageDisplayed());//VOXOX CHANGE by Rolando - 2009.06.29 
					
					if(statusMessage == ""){//VOXOX CHANGE by Rolando - 2009.06.29
						statusMessageRecoveredIsEmpty = true;//VOXOX CHANGE by Rolando - 2009.06.29 
						statusMessage = "No Status Message Set";//VOXOX CHANGE by Rolando - 2009.06.29 
					}
				}

				networkSocialStatusAction = socialStatusMenu->addAction(QIcon(protocolPixmapPath),statusMessage, this, SLOT(socialStatusClicked()) );

				if(statusMessageRecoveredIsEmpty){//VOXOX CHANGE by Rolando - 2009.06.29 
					networkSocialStatusAction->setData(INVALID_STATUS_MESSAGE);	//VOXOX CHANGE by Rolando - 2009.06.29 
					statusMessage = "";//VOXOX CHANGE by Rolando - 2009.07.01 
				}
				else{
					networkSocialStatusAction->setData(VALID_STATUS_MESSAGE);//VOXOX CHANGE by Rolando - 2009.06.29 
				}
				
				networkSocialStatusAction->setEnabled(true);
				networkSocialStatusAction->setCheckable(true);
				actionsNetworksGroup->addAction(networkSocialStatusAction);//VOXOX CHANGE by Rolando - 2009.06.29

				/*if(statusMessageAccountKeyRecovered){//VOXOX CHANGE by Rolando - 2009.06.29 					
					networkSocialStatusAction->setChecked(config.getLastStatusMessageAccountKey() == iter->second.getKey());//VOXOX CHANGE by Rolando - 2009.06.29 
				}
				else{
					networkSocialStatusAction->setChecked(_currentIMAccountKey == QString::fromStdString(iter->second.getKey()));//VOXOX CHANGE by Rolando - 2009.06.29 
				}*/
				
			}

			_socialStatusToAction[networkSocialStatusAction] = statusMessage;//VOXOX CHANGE by Rolando - 2009.06.29 
			_imContactKeyToAction[networkSocialStatusAction] = QString::fromStdString(iter->second.getKey());//VOXOX CHANGE by Rolando - 2009.06.29 
			
			
		}
		statusMessageAccountKeyRecovered = false;//VOXOX CHANGE by Rolando - 2009.07.01 
		statusMessageRecoveredIsEmpty = false;//VOXOX CHANGE by Rolando - 2009.07.01 
		++iter;

	}

	if(_statusMessageList.size() > 0){//VOXOX CHANGE by Rolando - 2009.06.29 
		socialStatusMenu->addSeparator();
		QStringList::const_iterator constIterator = _statusMessageList.constBegin();//VOXOX CHANGE by Rolando - 2009.06.29 
		for (constIterator = _statusMessageList.constBegin(); constIterator != _statusMessageList.constEnd();++constIterator){//VOXOX CHANGE by Rolando - 2009.06.29 
			QAction* socialStatusAction = socialStatusMenu->addAction(*constIterator, this, SLOT(socialStatusClicked()) );//VOXOX CHANGE by Rolando - 2009.06.29 
			socialStatusAction->setData(VALID_STATUS_MESSAGE);//VOXOX CHANGE by Rolando - 2009.06.29 
			socialStatusAction->setEnabled(true);//VOXOX CHANGE by Rolando - 2009.06.29 
			socialStatusAction->setCheckable(true);//VOXOX CHANGE by Rolando - 2009.06.29 
			actionsHistoryGroup->addAction(socialStatusAction);//VOXOX CHANGE by Rolando - 2009.06.29 
			_socialStatusToAction[socialStatusAction] = *constIterator;//VOXOX CHANGE by Rolando - 2009.06.29 
		}		
	}

	if(_statusMessageList.size() > 0){//VOXOX CHANGE by Rolando - 2009.07.02  
		socialStatusMenu->addSeparator();//VOXOX CHANGE by Rolando - 2009.07.02 
		QAction* clearHistoryStatusAction = socialStatusMenu->addAction(QString("Clear History"), this, SLOT(clearHistoryStatusClicked()) );//VOXOX CHANGE by Rolando - 2009.07.02 
	}

	if(_imAccountToId.size() > 0){//VOXOX CHANGE by Rolando - 2009.07.02 
		socialStatusMenu->addSeparator();//VOXOX CHANGE by Rolando - 2009.07.02 
		QAction* clearSocialStatusAction = socialStatusMenu->addAction(QString("Clear Status"), this, SLOT(clearSocialStatusClicked()) );//VOXOX CHANGE by Rolando - 2009.07.02 

	}

	QPoint p = _ui->socialNetworkFrame->pos();
	p.setY(p.y() + _ui->socialNetworkFrame->height());
	socialStatusMenu->popup(this->mapToGlobal(p));	
	
}

//VOXOX CHANGE by Rolando - 2009.06.29 
void QtIMProfileWidget::socialStatusClicked(){
	QString socialStatusText = _socialStatusToAction[sender()];//VOXOX CHANGE by Rolando - 2009.06.29 
	QtEnumIMProtocolMap& rMap = QtEnumIMProtocolMap::getInstance();//VOXOX CHANGE by Rolando - 2009.06.29 

	_ui->socialNetworkLineEdit->setText(socialStatusText);
	_ui->socialNetworkLineEdit->setFocus();

	QString key = _imContactKeyToAction[sender()];//VOXOX CHANGE by Rolando - 2009.06.29 

	if(key == ""){
		key = HISTORY_STATUS_MESSAGE;//VOXOX CHANGE by Rolando - 2009.06.29 
		_ui->socialNetworkStatusLabel->setPixmap(QString(":/pics/profilebar/global_status_message.png"));//VOXOX CHANGE by Rolando - 2009.06.29 
	}
	else{
		IMAccountToId::const_iterator iter    = _imAccountToId.begin();//VOXOX CHANGE by Rolando - 2009.06.29 
		IMAccountToId::const_iterator iterEnd = _imAccountToId.end();//VOXOX CHANGE by Rolando - 2009.06.29 
		while (iter != iterEnd) 
		{
			if(QString::fromStdString(iter->second.getKey()) == key){	//VOXOX CHANGE by Rolando - 2009.06.29 			
				_ui->socialNetworkStatusLabel->setPixmap(QString::fromStdString(rMap.findByQtProtocol( iter->second.getQtProtocol() )->getIconPath()));//VOXOX CHANGE by Rolando - 2009.07.07 
			}
			++iter;
		}	
	}
	_currentIMAccountKey = key;//VOXOX CHANGE by Rolando - 2009.06.29 

}

//VOXOX CHANGE by Rolando - 2009.06.28 
void QtIMProfileWidget::globalSocialStatusClicked(){
	_ui->socialNetworkStatusLabel->setPixmap(QString(":/pics/profilebar/global_status_message.png"));//VOXOX CHANGE by Rolando - 2009.07.07 
	_ui->socialNetworkLineEdit->setFocus();//VOXOX CHANGE by Rolando - 2009.06.29
	if(_ui->socialNetworkLineEdit->text() == ""){//VOXOX CHANGE by Rolando - 2009.07.01 
		_ui->socialNetworkLineEdit->setText("");//VOXOX CHANGE by Rolando - 2009.07.01 
	}
	_currentIMAccountKey = GLOBAL_STATUS_MESSAGE;//VOXOX CHANGE by Rolando - 2009.06.29 
}


void QtIMProfileWidget::clearHistoryStatusClicked(){
	_statusMessageList.clear();//VOXOX CHANGE by Rolando - 2009.06.29 
	//_ui->socialNetworkLineEdit->setText("");//VOXOX CHANGE by Rolando - 2009.06.29 
	//_ui->socialNetworkLineEdit->setToolTipText();//VOXOX CHANGE by Rolando - 2009.06.29 
	_ui->socialNetworkLineEdit->setFocus();//VOXOX CHANGE by Rolando - 2009.06.29 
}

void QtIMProfileWidget::clearSocialStatusClicked(){
	if(_currentIMAccountKey == GLOBAL_STATUS_MESSAGE){
		_ui->socialNetworkLineEdit->setText("");//VOXOX CHANGE by Rolando - 2009.07.02 
		_ui->socialNetworkStatusLabel->setPixmap(QString(":/pics/profilebar/global_status_message.png"));//VOXOX CHANGE by Rolando - 2009.07.07 
		_ui->socialNetworkLineEdit->setFocus();//VOXOX CHANGE by Rolando - 2009.07.02 
	}
	else{
		_ui->socialNetworkLineEdit->setText("");//VOXOX CHANGE by Rolando - 2009.07.02
		_ui->socialNetworkLineEdit->setFocus();//VOXOX CHANGE by Rolando - 2009.07.02
		
	}
	_ui->socialNetworkLineEdit->displayToolTipMessage();//VOXOX CHANGE by Rolando - 2009.07.02
	statusMessageTextChanged();//VOXOX CHANGE by Rolando - 2009.07.03 
	
}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::addSIPAccountMenu(QMenu * menu, QString login, QString id, QIcon sipIcon,
										  EnumPresenceState::PresenceState currentPresenceState, const QString& errMsg )
{
	// Add menu
	//voxox.jabber.server
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	
	int pos = login.indexOf(QString::fromStdString("@" + config.getJabberVoxoxServer()));
	if(pos != -1){
		login = login.remove(pos, login.length() - pos);
	}

	QMenu* accountsMenu = menu->addMenu(sipIcon, login);		
	
	QtIMAccountPresenceMenuManager* manager = new QtIMAccountPresenceMenuManager(accountsMenu, *_cUserProfile, id.toStdString());
	manager->addPresenceActions(accountsMenu,currentPresenceState, errMsg);	//VOXOX - JRT - 2009.05.21 
	
}

void QtIMProfileWidget::cleanLoginName( QString& login )
{
	//voxox.jabber.server
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	
	int pos = login.indexOf(QString::fromStdString("@" + config.getJabberVoxoxServer()));

	if(pos >= 0){//if was found the character "@" + config.getJabberVoxoxServer()
		login = login.remove(pos, login.length() - pos);
	}

	pos = login.indexOf(QString("/"));
	if(pos >= 0){//if was found the character "/"
		login = login.remove(pos, login.length() - pos);
	}
}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::addIMAccountMenu(QMenu * menu, QString login, QString id, QIcon imIcon, EnumPresenceState::PresenceState currentPresenceState, 
										 bool imAccountEnable, const QString& errMsg )
{

	QMenu* accountsMenu = menu->addMenu(imIcon, login);	

	if(imAccountEnable)
	{		
		QtIMAccountPresenceMenuManager* manager = new QtIMAccountPresenceMenuManager(accountsMenu, *_cUserProfile, id.toStdString());
		manager->addPresenceActions(accountsMenu,currentPresenceState, errMsg );
	}
	else
	{
		accountsMenu->setEnabled(false);
	}
}

void QtIMProfileWidget::updateSIPStatusLabel(){

	/*EnumPresenceState::PresenceState presenceState;
	if(_cUserProfile->getUserProfile().isSIPConnected()){
		presenceState = _cUserProfile->getUserProfile().getSIPPresenceState();		
	}
	else{
		presenceState = EnumPresenceState::PresenceStateOffline;
	}

	switch (presenceState) {
		case EnumPresenceState::PresenceStateAway:
			_ui->sipIconstatusLabel->setPixmap(QPixmap(":/pics/status/away.png"));
			break;
		case EnumPresenceState::PresenceStateOnline:
			_ui->sipIconstatusLabel->setPixmap(QPixmap(":/pics/status/online.png"));
			break;
		case EnumPresenceState::PresenceStateOffline:
			_ui->sipIconstatusLabel->setPixmap(QPixmap(":/pics/status/offline.png"));			
			break;
		case EnumPresenceState::PresenceStateInvisible:			
			_ui->sipIconstatusLabel->setPixmap(QPixmap(":/pics/status/invisible.png"));
			break;
		case EnumPresenceState::PresenceStateDoNotDisturb:			
			_ui->sipIconstatusLabel->setPixmap(QPixmap(":/pics/status/donotdisturb.png"));
			break;		
		default:
			_ui->sipIconstatusLabel->setPixmap(QPixmap(":/pics/status/offline.png"));
			LOG_FATAL("unknown presenceState=" + String::fromNumber(presenceState));
			break;
	}*/
}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::updateIMStatusLabel(){

	if(_cUserProfile){

		EnumPresenceState::PresenceState presenceState = _cUserProfile->getUserProfile().getIMPresenceState();	

		if(presenceState == EnumPresenceState::PresenceStateMulti){
			if(_cUserProfile->getUserProfile().isVoxOxConnected()){
				presenceState = _cUserProfile->getUserProfile().getVoxOxIMPresenceState();
			}else{
				presenceState = EnumPresenceState::PresenceStateOffline;		
			}
			
		}
		else{
			if(!_cUserProfile->getUserProfile().isVoxOxConnected()){			
				presenceState = EnumPresenceState::PresenceStateOffline;		
			}
		}

		switch (presenceState) {
			case EnumPresenceState::PresenceStateAway:
				_ui->statusLabel->setPixmap(QPixmap(":/pics/status/away.png"));
				break;
			case EnumPresenceState::PresenceStateOnline:
				_ui->statusLabel->setPixmap(QPixmap(":/pics/status/online.png"));
				break;
			case EnumPresenceState::PresenceStateOffline:
				_ui->statusLabel->setPixmap(QPixmap(":/pics/status/offline.png"));
				break;
			case EnumPresenceState::PresenceStateInvisible:
				_ui->statusLabel->setPixmap(QPixmap(":/pics/status/invisible.png"));
				break;
			case EnumPresenceState::PresenceStateDoNotDisturb:
				_ui->statusLabel->setPixmap(QPixmap(":/pics/status/donotdisturb.png"));
				break;

			default:
				_ui->statusLabel->setPixmap(QPixmap(":/pics/status/offline.png"));
				LOG_FATAL("unknown presenceState=" + String::fromNumber(presenceState));
				break;
		}
	}
}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::socialNetworkLineEditChangedSlot(QString newText){

	//if(!_statusMessageList.contains(newText) && newText != ""){
	//	_statusMessageList << newText;		
	//}
	//
	//std::string statusMessage(newText.toUtf8().constData());
	//_cUserProfile->getUserProfile().setStatusMessage(statusMessage, NULL);

}

//VOXOX CHANGE Rolando 03-20-09
void QtIMProfileWidget::updateText() {

	if((_nickName!="" || _nickName!=NULL)&& (_sipNumber!="" || _sipNumber!=NULL) && (_voxoxPoints!="" || _voxoxPoints!=NULL)){
		_ui->usernameLabel->setText(_nickName);
		_ui->sipStatusLabel->setText(_sipNumber);
		_ui->voxoxPointsLabel->setText(_voxoxPoints);

		


		//VOXOX CHANGE by Rolando - 2009.05.14 - Necessary to show an ellipis at the end of current voxoxPointsLabel value. 
		//Don't change this policy meanwhile we use ellipsis at the end of voxox points value
		_ui->voxoxPointsLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	}else if((_nickName!="" || _nickName!=NULL) && (_sipNumber!="" || _sipNumber!=NULL)&& (_voxoxPoints=="" || _voxoxPoints==NULL)){
		_ui->usernameLabel->setText(_nickName);
		_ui->sipStatusLabel->setText(_sipNumber);

		
	}else if((_nickName!="" || _nickName==NULL) && (_sipNumber=="" || _sipNumber==NULL)&& (_voxoxPoints=="" || _voxoxPoints==NULL)){
		_ui->usernameLabel->setText(_nickName);
	}

	setVisible(true);
}

//VOXOX CHANGE by Rolando - 2009.05.14 - when widget updates its size we should update width of voxoxPointsLabel
void QtIMProfileWidget::resizeEvent(QResizeEvent * event){
	
	QSize oldSize = event->oldSize();
	//VOXOX CHANGE by Rolando - 2009.05.13 - if difference is a positive value that means window reduced its width
	//VOXOX CHANGE by Rolando - 2009.05.13 - otherwise its width was increased
	int difference = oldSize.width() - event->size().width();

	if(_voxoxPoints != ""){//VOXOX CHANGE by Rolando - 2009.05.14 - update label only if exists text from _voxoxPoints variable
		updateVoxOxPointsWidthLabel(difference);
	}

}

//VOXOX CHANGE by Rolando - 2009.05.14 - updates voxoxPointsLabel width size according parameter difference
void QtIMProfileWidget::updateVoxOxPointsWidthLabel(int difference){

	verifyVoxOxPointsText(difference);	

}

//VOXOX CHANGE by Rolando - 2009.05.14 - updates voxoxPointsLabel width size according its width size available
void QtIMProfileWidget::verifyVoxOxPointsText(int difference){

	//VOXOX CHANGE by Rolando - 2009.05.14 - gets maximum width needed according value of variable _voxoxPoints 
	int voxoxPointsPixelsSize = QFontMetrics(_ui->voxoxPointsLabel->font()).width(_voxoxPoints);
	int textWidth = 0;//VOXOX CHANGE by Rolando - 2009.05.14 - temporal variable to get new text to show
	QString text = _voxoxPoints;
	bool fixedText = false;
	int currentTextWidth= _ui->voxoxPointsLabel->width();//VOXOX CHANGE by Rolando - 2009.05.14 - gets current width of label
	int counter = 0;

	
	if(difference < 0){//VOXOX CHANGE by Rolando - 2009.05.14 - if difference is less than 0 is because the aplication increased its width size
		if(difference == -1){//VOXOX CHANGE by Rolando - 2009.05.14 - because we need to increase size of label faster no 1 by 1 pixel we multiply by 3, and it is negative value because we need to convert it to a positive value
			difference *= -3;
		}
		else{
			difference *= -1;
		}
		
		//VOXOX CHANGE by Rolando - 2009.05.14 - if current width of label plus difference is largest than width needed
		if(currentTextWidth + difference > voxoxPointsPixelsSize){
			currentTextWidth = voxoxPointsPixelsSize;
		}
		else{
			currentTextWidth += difference;//VOXOX CHANGE by Rolando - 2009.05.14 - adds difference to current width
		}

		//VOXOX CHANGE by Rolando - 2009.05.14 - resizes the label to new width size
		_ui->voxoxPointsLabel->resize(currentTextWidth,_ui->voxoxPointsLabel->height());
		
	}

	//VOXOX CHANGE by Rolando - 2009.05.14 - if current available width is wider than size needed
	if(currentTextWidth < voxoxPointsPixelsSize){
		text = "";
		while(textWidth < currentTextWidth){//gets part of voxox Points text that fits in width available
			text = text.append(_voxoxPoints.at(counter));
			textWidth = QFontMetrics(_ui->voxoxPointsLabel->font()).width(text);
			fixedText = true;
			counter++;
		}
		
		if(fixedText){
			if(text.length() > 3){//if text is larger than size of ellipsis (...) but it is not enough to fit the whole text
				text = text.remove(text.length()- 3,text.length() );
				text += "...";
			}		
		}
	}
	
	_ui->voxoxPointsLabel->setText(text);	

}