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
* @date 2009.06.01
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtWizardDialog.h"

#include "ui_WizardDialog.h"

#include "QtWizardProgress.h"
#include "QtWizardSelectNetworks.h"
#include "QtWizardIMAccountContainer.h"
#include "QtWizardPhones.h"
#include "QtWizardAudio.h"
#include "QtWizardVideo.h"
#include "QtWizardProfile.h"
#include "QtWizardPA.h"
//#include "QtEventsSettings.h"
//#include "QtNotificationSettings.h"
//#include "QtGeneralSettings.h"
//#include "QtAccountSettings.h"
//#include "QtPrivacySettings.h"
//#include "QtAppearanceSettings.h"
//#include "QtAudioSettings.h"
//#include "QtVideoSettings.h"
//#include "QtAdvancedSettings.h"
//#include "QtCallForwardSettings.h"
//#include "QtLanguagesSettings.h"
//#include "QtVoicemailSettings.h"
//#include "QtSecuritySettings.h"

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfileHandler.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/WidgetUtils.h>
#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <QtGui/QtGui>
#include <QtCore/QPointer>

#include <qtutil/CloseEventFilter.h>
#include <qtutil/SafeConnect.h>

//VOXOX CHANGE by ASV 07-03-2009: Added window modality to prevent this window from being on top of the main window
QtWizardDialog::QtWizardDialog(QWidget *parent, CWengoPhone & cWengoPhone)
: _cWengoPhone(cWengoPhone),QtVoxWindowInfo(parent,QtEnumWindowType::WizardWindow, Qt::Window | Qt::WindowTitleHint) {

	_ui = new Ui::WizardDialog();
	_ui->setupUi(this);
	_ui->retranslateUi(this);

	CloseEventFilter * closeEventFilter = new CloseEventFilter(this, SLOT(closeEvent()));
	this->installEventFilter(closeEventFilter);

	_wizardList += new QtWizardProgress(cWengoPhone, 0);
	//Important the select netroks should always be on the index 1
	_wizardList += new QtWizardSelectNetworks(cWengoPhone, 0);
	_wizardList += new QtWizardPhones(cWengoPhone, 0);
	_wizardList += new QtWizardPA(cWengoPhone, 0);
	_wizardList += new QtWizardAudio(cWengoPhone, 0);
	_wizardList += new QtWizardVideo(cWengoPhone, 0);
	_wizardList += new QtWizardProfile(cWengoPhone, 0);
//	if (cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
//		_settingsList += new QtAccountSettings(cWengoPhone, 0);
//	}
//	_settingsList += new QtAppearanceSettings(cWengoPhone,0);
//	_settingsList += new QtPhoneSettings(cWengoPhone, 0);
//	_settingsList += new QtAudioSettings(cWengoPhone, 0);
//	_settingsList += new QtVideoSettings(cWengoPhone, 0);
//	_settingsList += new QtMyAccountSettings(cWengoPhone, 0);
//	_settingsList += new QtEventsSettings(0);
//	_settingsList += new QtNotificationSettings(0);
////#ifndef DISABLE_CALL_FORWARD
////	_settingsList += new QtCallForwardSettings(cWengoPhone, 0);
////#endif
//	
////#ifndef DISABLE_VOICE_MAIL
////	_settingsList += new QtVoicemailSettings(cWengoPhone, 0);
////#endif
////	_settingsList += new QtSecuritySettings(cWengoPhone, 0);
////	
////	_settingsList += new QtAdvancedSettings(0);
//
//	//stackedWidget
//
	Q_FOREACH(QtIWizard* wizardItem, _wizardList) {
		QWidget* widget = wizardItem->getWidget();
		//widget->layout()->setMargin(0);
		_ui->stackedWidget->addWidget(widget);
	}

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	_ui->chkShowStartup->setChecked(config.getWizardAutoStart());
//	_ui->listWidget->setCurrentRow(0);
	showWizardPage(0);
//
//	//int minWidth = WidgetUtils::computeListViewMinimumWidth(_ui->listWidget);
//	_ui->listWidget->setFixedWidth(117);
//
	
	//saveButton
	connect(_ui->continueButton, SIGNAL(clicked()), SLOT(continuePage()));

	connect(_ui->backButton, SIGNAL(clicked()), SLOT(previewsPage()));

	//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#if defined(OS_MACOSX)
		_closeWindowShortCut = new QShortcut(QKeySequence("Ctrl+Shift+W"), this);
		SAFE_CONNECT(_closeWindowShortCut, SIGNAL(activated()), SLOT(close()));
	#endif
	
}

QtWizardDialog::~QtWizardDialog() {
	
	#if defined(OS_MACOSX)
		OWSAFE_DELETE(_closeWindowShortCut);//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#endif

	OWSAFE_DELETE(_ui);

}

void QtWizardDialog::showWindow(){//VOXOX - CJC - 2009.07.09 
	if(isVisible()){
		activateWindow();
		raise();
	}else{
		showNormal();
	}
}

void QtWizardDialog::showWizardPage(int row) {
	if (row<0) {
		return;
	}
	if(row == _wizardList.size()){
		save();
		return;
	}

	if(row == _wizardList.size()-1){
		_ui->continueButton->setText(tr("Complete"));
	}else{
		_ui->continueButton->setText(tr("Continue"));
	}

	_currentWizardPage = row;
	QtIWizard* wizard = _wizardList.at(row);
	_ui->titleLabel->setText(wizard->getTitle());
	if(wizard->getDescription()!=""){
		_ui->lblDescription->setVisible(true);
		_ui->lblDescription->setText(wizard->getDescription());
	}
	else{
		_ui->lblDescription->setVisible(false);
	}
	if(wizard->getPixmapPath()!=""){
		_ui->lblIcon->setVisible(true);
		QPixmap icon = QPixmap(wizard->getPixmapPath());
		_ui->lblIcon->setPixmap(icon);
		_ui->lblIcon->setMinimumSize(icon.size());
		_ui->lblIcon->setMaximumSize(icon.size());
	}else{
		_ui->lblIcon->setVisible(false);
	}
	
	//Hide stuff
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if(config.getWizardStepsComplete()<wizard->getStepNumber()){
		config.set(Config::WIZARD_STEP_COMPLETE_KEY, wizard->getStepNumber());
	}

	QWidget* widget = wizard->getWidget();
	wizard->readConfig();
	_ui->stackedWidget->setCurrentWidget(widget);

	if(wizard->getStepNumber()==0){
		_ui->chkShowStartup->setVisible(true);
		_ui->backButton->setVisible(false);
		_ui->lblStepImages->setVisible(false);
	}else{
		_ui->chkShowStartup->setVisible(false);
		_ui->backButton->setVisible(true);
		_ui->lblStepImages->setVisible(true);
		switch(wizard->getStepNumber()){

			case 1:
					_ui->lblStepImages->setPixmap(QPixmap(":pics/wizard/step1.png"));
				break;
			case 2:
					_ui->lblStepImages->setPixmap(QPixmap(":pics/wizard/step2.png"));
				break;
			case 3:
					_ui->lblStepImages->setPixmap(QPixmap(":pics/wizard/step3.png"));
				break;
			case 4:
					_ui->lblStepImages->setPixmap(QPixmap(":pics/wizard/step4.png"));
				break;
			case 5:
					_ui->lblStepImages->setPixmap(QPixmap(":pics/wizard/step5.png"));
				break;
		}
	}
	wizard->postInitialize();
}

void QtWizardDialog::saveShowOnStartUp()	//VOXOX - JRT - 2009.08.20 
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	config.set(Config::WIZARD_AUTOSTART_KEY, _ui->chkShowStartup->isChecked());
}

void QtWizardDialog::close()
{
	this->accept();
	windowClose(getKey());
}

void QtWizardDialog::save() 
{
	saveShowOnStartUp();

	//VOXOX - JRT - 2009.08.20 - We save each page separately so user can abort at any time.
//	Q_FOREACH(QtIWizard* wizards, _wizardList) 
//	{
//		wizards->saveConfig();
//	}

	//VOXOX - JRT - 2009.08.03 - We do a single update to server.
	//VOXOX - JRT - 2009.08.03 - TODO: detect if there were changes to minimize server hits.
//	_cWengoPhone.getCUserProfileHandler().updateUserProfile();

	close();		//VOXOX - JRT - 2009.08.20 
}

void QtWizardDialog::closeEvent()
{
	//VOXOX - JRT - 2009.08.20 - We have an intermittent issue where UserProfile from server is empty.
	//							 So we don't want to save all these if user exits, because all user profile data will be lost.
	saveShowOnStartUp();

//	save();			//VOXOX - JRT - 2009.08.20 - We are saving individual screens as we go.

	close();
}

void QtWizardDialog::showPage(const QString & pageName) {
	/*int count = _settingsList.count();
	for (int row = 0; row < count; ++row) {
		if (_settingsList[row]->getName() == pageName) {
			_ui->listWidget->setCurrentRow(row);
			return;
		}
	}
	LOG_WARN("unknown page name=" + pageName.toStdString());*/
}

void QtWizardDialog::continuePage() {
	int page = _currentWizardPage+1;
		
	//We are moving from the wizard select networks window, need to check what networks where checked and add the pages of those networks
	if(page == 2)
	{
		addNetworksPages();
	}
	else
	{
		_wizardList[_currentWizardPage]->saveConfig();	//VOXOX - JRT - 2009.08.20 - Save each page as user continues past it.
		showWizardPage(page);
	}
	
}

void QtWizardDialog::previewsPage() {
	int page = _currentWizardPage-1;
	showWizardPage(page);
}

void QtWizardDialog::addNetworksPages(){
	cleanNetworksPages();
	QtWizardSelectNetworks *  wizardSelectNetworks = dynamic_cast<QtWizardSelectNetworks *> (_wizardList[1]);
	int indexToAdd = 2;

	//VOXOX - JRT - 2009.08.20 - Let's make this easier to read and maintain.
	addNetworkPageIfNeeded( wizardSelectNetworks->getAddAIM(),			QtEnumIMProtocol::IMProtocolAIM,		indexToAdd );
	addNetworkPageIfNeeded( wizardSelectNetworks->getAddFacebook(),		QtEnumIMProtocol::IMProtocolFacebook,	indexToAdd );
	addNetworkPageIfNeeded( wizardSelectNetworks->getAddGoogleTalk(),	QtEnumIMProtocol::IMProtocolGoogleTalk, indexToAdd );
	addNetworkPageIfNeeded( wizardSelectNetworks->getAddICQ(),			QtEnumIMProtocol::IMProtocolICQ,		indexToAdd );
	addNetworkPageIfNeeded( wizardSelectNetworks->getAddJabber(),		QtEnumIMProtocol::IMProtocolJabber,		indexToAdd );
	addNetworkPageIfNeeded( wizardSelectNetworks->getAddMSN(),			QtEnumIMProtocol::IMProtocolMSN,		indexToAdd );
	addNetworkPageIfNeeded( wizardSelectNetworks->getAddMySpace(),		QtEnumIMProtocol::IMProtocolMYSPACE,	indexToAdd );
	addNetworkPageIfNeeded( wizardSelectNetworks->getAddSkype(),		QtEnumIMProtocol::IMProtocolSkype,		indexToAdd );
	addNetworkPageIfNeeded( wizardSelectNetworks->getAddTwitter(),		QtEnumIMProtocol::IMProtocolTwitter,	indexToAdd );
	addNetworkPageIfNeeded( wizardSelectNetworks->getAddYahoo(),		QtEnumIMProtocol::IMProtocolYahoo,		indexToAdd );



	//if(wizardSelectNetworks->getAddAIM()){
	//	_wizardList.insert(indexToAdd,new QtWizardIMAccountContainer(_cWengoPhone,QtEnumIMProtocol::IMProtocolAIM, 0));
	//	QWidget* widget = _wizardList[indexToAdd]->getWidget();
	//	_ui->stackedWidget->insertWidget(indexToAdd,widget);
	//	indexToAdd = indexToAdd++;
	//}
	//if(wizardSelectNetworks->getAddFacebook()){
	//	_wizardList.insert(indexToAdd,new QtWizardIMAccountContainer(_cWengoPhone,QtEnumIMProtocol::IMProtocolFacebook, 0));
	//	QWidget* widget = _wizardList[indexToAdd]->getWidget();
	//	_ui->stackedWidget->insertWidget(indexToAdd,widget);
	//	indexToAdd = indexToAdd++;
	//}
	//if(wizardSelectNetworks->getAddGoogleTalk()){
	//	_wizardList.insert(indexToAdd,new QtWizardIMAccountContainer(_cWengoPhone,QtEnumIMProtocol::IMProtocolGoogleTalk, 0));
	//	QWidget* widget = _wizardList[indexToAdd]->getWidget();
	//	_ui->stackedWidget->insertWidget(indexToAdd,widget);
	//	indexToAdd = indexToAdd++;
	//}
	//if(wizardSelectNetworks->getAddICQ()){
	//	_wizardList.insert(indexToAdd,new QtWizardIMAccountContainer(_cWengoPhone,QtEnumIMProtocol::IMProtocolICQ, 0));
	//	QWidget* widget = _wizardList[indexToAdd]->getWidget();
	//	_ui->stackedWidget->insertWidget(indexToAdd,widget);
	//	indexToAdd = indexToAdd++;
	//}
	//if(wizardSelectNetworks->getAddJabber()){
	//	_wizardList.insert(indexToAdd,new QtWizardIMAccountContainer(_cWengoPhone,QtEnumIMProtocol::IMProtocolJabber, 0));
	//	QWidget* widget = _wizardList[indexToAdd]->getWidget();
	//	_ui->stackedWidget->insertWidget(indexToAdd,widget);
	//	indexToAdd = indexToAdd++;
	//}
	//if(wizardSelectNetworks->getAddMSN()){
	//	_wizardList.insert(indexToAdd,new QtWizardIMAccountContainer(_cWengoPhone,QtEnumIMProtocol::IMProtocolMSN, 0));
	//	QWidget* widget = _wizardList[indexToAdd]->getWidget();
	//	_ui->stackedWidget->insertWidget(indexToAdd,widget);
	//	indexToAdd = indexToAdd++;
	//}
	//if(wizardSelectNetworks->getAddMySpace()){
	//	_wizardList.insert(indexToAdd,new QtWizardIMAccountContainer(_cWengoPhone,QtEnumIMProtocol::IMProtocolMYSPACE, 0));
	//	QWidget* widget = _wizardList[indexToAdd]->getWidget();
	//	_ui->stackedWidget->insertWidget(indexToAdd,widget);
	//	indexToAdd = indexToAdd++;
	//}
	//if(wizardSelectNetworks->getAddSkype()){
	//	_wizardList.insert(indexToAdd,new QtWizardIMAccountContainer(_cWengoPhone,QtEnumIMProtocol::IMProtocolSkype, 0));
	//	QWidget* widget = _wizardList[indexToAdd]->getWidget();
	//	_ui->stackedWidget->insertWidget(indexToAdd,widget);
	//	indexToAdd = indexToAdd++;
	//}
	//if(wizardSelectNetworks->getAddTwitter()){
	//	_wizardList.insert(indexToAdd,new QtWizardIMAccountContainer(_cWengoPhone,QtEnumIMProtocol::IMProtocolTwitter, 0));
	//	QWidget* widget = _wizardList[indexToAdd]->getWidget();
	//	_ui->stackedWidget->insertWidget(indexToAdd,widget);
	//	indexToAdd = indexToAdd++;
	//}
	//if(wizardSelectNetworks->getAddYahoo()){
	//	_wizardList.insert(indexToAdd,new QtWizardIMAccountContainer(_cWengoPhone,QtEnumIMProtocol::IMProtocolYahoo, 0));
	//	QWidget* widget = _wizardList[indexToAdd]->getWidget();
	//	_ui->stackedWidget->insertWidget(indexToAdd,widget);
	//	indexToAdd = indexToAdd++;
	//}

	showWizardPage(2);

}

void QtWizardDialog::addNetworkPageIfNeeded( bool shouldAdd, QtEnumIMProtocol::IMProtocol qtProtocol, int& indexToAdd )
{
	if (shouldAdd)
	{
		_wizardList.insert(indexToAdd, new QtWizardIMAccountContainer(_cWengoPhone, qtProtocol, 0));
		QWidget* widget = _wizardList[indexToAdd]->getWidget();
		_ui->stackedWidget->insertWidget(indexToAdd, widget);
		indexToAdd = indexToAdd++;
	}
}

void QtWizardDialog::cleanNetworksPages(){
	for (int row = 0; row < _wizardList.count(); ++row) {
		if (_wizardList[row]->getName() == "IMAccount") {
			QWidget* widget = _wizardList[row]->getWidget();
			_ui->stackedWidget->removeWidget(widget);
			_wizardList.removeAt(row);
			--row;
		}
	}
}


QWidget * QtWizardDialog::getWidget() const {
	return (QWidget*)this;
}

QString QtWizardDialog::getKey() const {
	return QtEnumWindowType::toString(getType());
}

QString QtWizardDialog::getDescription() const {
	return QString("VoxOx Wizard");
}

int QtWizardDialog::getAllowedInstances() const{
	return 1;
}

int QtWizardDialog::getCurrentTab() const{
	return 0;
}

void QtWizardDialog::setCurrentTab(QString tabName){
	//showPage(tabName);
}




//void QtWizardDialog::showGeneralPage() {
//	showPage(tr("General"));
//}
//
//void QtWizardDialog::showLanguagePage() {
//	showPage(tr("Language"));
//}
//
//void QtWizardDialog::showAccountsPage() {
//	showPage(tr("Accounts"));
//}
//
//void QtWizardDialog::showPrivacyPage() {
//	showPage(tr("Privacy"));
//}
//
//void QtWizardDialog::showAudioPage() {
//	showPage(tr("Audio"));
//}
//
//void QtWizardDialog::showVideoPage() {
//	showPage(tr("Video"));
//}
//
//void QtWizardDialog::showNotificationsPage() {
//	showPage(tr("Notifications"));
//}
//
//void QtWizardDialog::showCallForwardPage() {
//	showPage(tr("Call Forward"));
//}
//
//void QtWizardDialog::showAdvancedPage() {
//	showPage(tr("Advanced"));
//}
//
//void QtWizardDialog::showVoicemailPage() {
//	showPage(tr("Voicemail"));
//}
//
//void QtWizardDialog::showSecurityPage() {
//	showPage(tr("security"));
//}

