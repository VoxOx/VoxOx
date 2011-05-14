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
#include "dialpad/QtDialpad.h"//VOXOX CHANGE by Rolando 04-29-09, now QtDialpad is in dialpad folder

#include "ui_DialpadWidget.h"

#include <presentation/qt/dialpad/QtSVGDialpad.h>//VOXOX CHANGE by Rolando 04-29-09, now QtSVGDialpad is in dialpad folder
#include <presentation/qt/conference/QtConferenceCallWidget.h>
#include <presentation/qt/phonecall/QtContactCallListWidget.h>
#include "QtWengoPhone.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/dtmf/DtmfTheme.h>
#include <model/webservices/url/WsUrl.h>

#include <presentation/qt/phonecall/QtPhoneCall.h>
#include <presentation/qt/contactlist/QtContactActionManager.h>
#include <presentation/qt/messagebox/QtVoxMessageBox.h>//VOXOX CHANGE by Rolando - 2009.09.08 
#include <presentation/qt/QtBrowserWidget.h>

#include <control/dtmf/CDtmfThemeManager.h>

#include <model/profile/UserProfile.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/CWengoPhone.h>

#include <sound/Sound.h>
#include <sound/AudioDevice.h>

#include <cutil/global.h>
#include <util/File.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/StringList.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/UpQComboBox.h>

#include <QtGui/QtGui>

#include <qtutil/Widget.h>

//VOXOX CHANGE by Rolando 04-02-09
const char* DEFAULT_THEME = "default";
const char* JUNGLE_THEME = "terminator";//VOXOX - CJC - 2009.06.28 
const int MINIMUM_HEIGHT_SIZE = 360;//VOXOX CHANGE by Rolando - 2009.06.18
const QString INTERNATIONAL_PREFIX_NUMBER = QString("011");//VOXOX CHANGE by Rolando - 2009.06.23
const QString PLUS_CHARACTER = QString("+");//VOXOX CHANGE by Rolando - 2009.06.23

QtDialpad::QtDialpad(CDtmfThemeManager & cDtmfThemeManager, QtWengoPhone * qtWengoPhone)
	: QWidget(NULL),
	_cDtmfThemeManager(cDtmfThemeManager)
	{

	_qtWengoPhone = qtWengoPhone;

	_ui = new Ui::DialpadWidget();
	_ui->setupUi(this);

	_qtConferenceCallWidget = NULL;
	
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	setMinimumHeight(MINIMUM_HEIGHT_SIZE);//VOXOX CHANGE by Rolando - 2009.06.18 

	SAFE_CONNECT(_ui->zeroButton, SIGNAL(leftMouseButtonPress()), SLOT(zeroButtonClicked()));
	SAFE_CONNECT(_ui->oneButton, SIGNAL(leftMouseButtonPress()), SLOT(oneButtonClicked()));
	SAFE_CONNECT(_ui->twoButton, SIGNAL(leftMouseButtonPress()), SLOT(twoButtonClicked()));
	SAFE_CONNECT(_ui->threeButton, SIGNAL(leftMouseButtonPress()), SLOT(threeButtonClicked()));
	SAFE_CONNECT(_ui->fourButton, SIGNAL(leftMouseButtonPress()), SLOT(fourButtonClicked()));
	SAFE_CONNECT(_ui->fiveButton, SIGNAL(leftMouseButtonPress()), SLOT(fiveButtonClicked()));
	SAFE_CONNECT(_ui->sixButton, SIGNAL(leftMouseButtonPress()), SLOT(sixButtonClicked()));
	SAFE_CONNECT(_ui->sevenButton, SIGNAL(leftMouseButtonPress()), SLOT(sevenButtonClicked()));
	SAFE_CONNECT(_ui->eightButton, SIGNAL(leftMouseButtonPress()), SLOT(eightButtonClicked()));
	SAFE_CONNECT(_ui->nineButton, SIGNAL(leftMouseButtonPress()), SLOT(nineButtonClicked()));
	SAFE_CONNECT(_ui->starButton, SIGNAL(leftMouseButtonPress()), SLOT(starButtonClicked()));
	SAFE_CONNECT(_ui->poundButton, SIGNAL(leftMouseButtonPress()), SLOT(poundButtonClicked()));

	SAFE_CONNECT(_ui->backButton, SIGNAL(leftMouseButtonPress()), SLOT(backButtonClicked()));
	SAFE_CONNECT(_ui->callButton, SIGNAL(leftMouseButtonPress()), SLOT(callButtonClicked()));
	SAFE_CONNECT(_ui->smsButton, SIGNAL(leftMouseButtonPress()), SLOT(smsButtonClicked()));//VOXOX CHANGE by Rolando - 2009.06.17 
	SAFE_CONNECT(_ui->faxButton, SIGNAL(leftMouseButtonPress()), SLOT(faxButtonClicked()));//VOXOX CHANGE by Rolando - 2009.06.17 

	SAFE_CONNECT(_ui->ratesButton, SIGNAL(leftMouseButtonPress()), SLOT(ratesButtonClicked()));
	SAFE_CONNECT(_ui->conferenceButton, SIGNAL(leftMouseButtonPress()), SLOT(conferenceButtonClicked()));
	SAFE_CONNECT(_ui->redialButton, SIGNAL(leftMouseButtonPress()), SLOT(redial()));
	SAFE_CONNECT(_ui->voiceMailButton, SIGNAL(leftMouseButtonPress()), SLOT(voiceMailButtonClicked()));

	setKeypadStyleSheet(QString::fromStdString(config.getKeypadBackgroundPath()));//VOXOX - SEMR - 2009.07.31 new style sheet method

	//VOXOX CHANGE by Rolando 04-02-09
	_ui->zeroButton->setImages(QString(":/pics/dialpad/11_on.png"),QString(":/pics/dialpad/11_on_press.png"));
	_ui->oneButton->setImages(QString(":/pics/dialpad/1_on.png"),QString(":/pics/dialpad/1_on_press.png"));
	_ui->twoButton->setImages(QString(":/pics/dialpad/2_on.png"),QString(":/pics/dialpad/2_on_press.png"));
	_ui->threeButton->setImages(QString(":/pics/dialpad/3_on.png"),QString(":/pics/dialpad/3_on_press.png"));
	_ui->fourButton->setImages(QString(":/pics/dialpad/4_on.png"),QString(":/pics/dialpad/4_on_press.png"));
	_ui->fiveButton->setImages(QString(":/pics/dialpad/5_on.png"),QString(":/pics/dialpad/5_on_press.png"));
	_ui->sixButton->setImages(QString(":/pics/dialpad/6_on.png"),QString(":/pics/dialpad/6_on_press.png"));
	_ui->sevenButton->setImages(QString(":/pics/dialpad/7_on"),QString(":/pics/dialpad/7_on_press.png"));
	_ui->eightButton->setImages(QString(":/pics/dialpad/8_on.png"),QString(":/pics/dialpad/8_on_press.png"));
	_ui->nineButton->setImages(QString(":/pics/dialpad/9_on.png"),QString(":/pics/dialpad/9_on_press.png"));
	_ui->starButton->setImages(QString(":/pics/dialpad/10_on.png"),QString(":/pics/dialpad/10_on_press.png"));
	_ui->poundButton->setImages(QString(":/pics/dialpad/12_on.png"),QString(":/pics/dialpad/12_on_press.png"));

	//VOXOX CHANGE by Rolando 04-02-09
	_ui->conferenceButton->setImages(QString(":/pics/dialpad/btn_conference.png"));
	_ui->ratesButton->setImages(QString(":/pics/dialpad/btn_rates.png"));
	_ui->voiceMailButton->setImages(QString(":/pics/dialpad/btn_voicemail.png"));
	_ui->redialButton->setImages(QString(":/pics/dialpad/btn_redial.png"));

	//VOXOX CHANGE by Rolando 04-02-09
	_ui->callButton->setImages(QString(":/pics/dialpad/btn_call.png"),QString(":/pics/dialpad/btn_call_press.png"));
	_ui->smsButton->setImages(QString(":/pics/dialpad/btn_sms.png"),QString(":/pics/dialpad/btn_sms_press.png"));//VOXOX CHANGE by Rolando - 2009.06.17 
	_ui->faxButton->setImages(QString(":/pics/dialpad/btn_fax.png"),QString(":/pics/dialpad/btn_fax_press.png"));//VOXOX CHANGE by Rolando - 2009.06.17 
	_ui->backButton->setImages(QString(":/pics/dialpad/btn_back.png"),QString(":/pics/dialpad/btn_back_press.png"));	
	
	//add svg viewer
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keyZeroSelected()), SLOT(zeroButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keyOneSelected()), SLOT(oneButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keyTwoSelected()), SLOT(twoButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keyThreeSelected()), SLOT(threeButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keyFourSelected()), SLOT(fourButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keyFiveSelected()), SLOT(fiveButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keySixSelected()), SLOT(sixButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keySevenSelected()), SLOT(sevenButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keyEightSelected()), SLOT(eightButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keyNineSelected()), SLOT(nineButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keyStarSelected()), SLOT(starButtonClicked()));
	SAFE_CONNECT(_ui->qtSVGDialpad, SIGNAL(keyPoundSelected()), SLOT(poundButtonClicked()));
	//VOXOX CHANGE by Rolando 04-02-09
	setThemeMode(ThemeModeDefaultOnly);
	
	//VOXOX CHANGE ASV 04-06-09: we need a margin on top for Mac so the tab bar doesn't get on top of the widget content.
	#if defined(OS_MACOSX)
		layout()->setContentsMargins(0, 11, 0, 0);
	#endif

	_defaultPage = _ui->classicDialpadPage;
	
}

QtDialpad::~QtDialpad() {
	OWSAFE_DELETE(_ui);
	OWSAFE_DELETE(_defaultPage);
	
}
void QtDialpad::setKeypadStyleSheet(QString keypadBackgroundPath){//VOXOX - SEMR - 2009.07.31 change the background of the keypad
	QString background = keypadBackgroundPath;
	if (background  == ""){
		background  = ":/keypad/voxox_keypad_background.jpg";		
	}
	background =background.replace("\\","/");

	setStyleSheet(QString("QLabel#phoneNumberLabel{ color: #dce602; font-size: 17px; font-family: \"Tahoma\"; } ")
					  + QString("QLabel#labelSVG{ color: #ffffff; font-size: 12px; font-family: \"Tahoma\"; } ")
					  + QString("QFrame#generalDialpadFrame{ background-image: url(%1); } ").arg(background)
					  + QString("QFrame#dialpadFrame{ background-image: url(:/pics/dialpad/bg_opaque1.png); border: none; } ")
					  + QString("QLabel#topImageLabel{ background-image: url(:/pics/dialpad/bgtopfade.png); background-repeat: repeat-x; border: 1px transparent; } "));

	update();
}

//VOXOX CHANGE by Rolando 04-02-09
void QtDialpad::backButtonClicked(){
	QString currentText = _ui->phoneNumberLabel->text();
	if(currentText.length() > 0){
		currentText = currentText.remove(currentText.length() - 1, 1);
		_ui->phoneNumberLabel->setText(currentText);
	}
	_qtWengoPhone->dialpadBackButtonPressed();
}

//VOXOX CHANGE by Rolando 04-02-09
void QtDialpad::callButtonClicked(){
	QString phoneNumber = _ui->phoneNumberLabel->text();
	if(phoneNumber != ""){
		CUserProfile * cUserProfile = _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
		if (cUserProfile){
			if(phoneNumber.startsWith(INTERNATIONAL_PREFIX_NUMBER)){//VOXOX CHANGE by Rolando - 2009.06.23 - added to replace "011" by "+"
				phoneNumber.replace(0, INTERNATIONAL_PREFIX_NUMBER.length(),PLUS_CHARACTER);//VOXOX CHANGE by Rolando - 2009.06.23 - added to replace "011" by "+"
			}
			//cUserProfile->makeCall(phoneNumber.toStdString());//VOXOX CHANGE by Rolando - 2009.06.23 
			if(_qtWengoPhone->textIsPhoneNumber(phoneNumber)){//VOXOX CHANGE by Rolando - 2009.06.23 

				phoneNumber = getPhoneNumberOnlyDigits(phoneNumber);
				cUserProfile->makeCall(phoneNumber.toStdString());//VOXOX CHANGE by Rolando - 2009.05.27 - if this is a call to non contact
			}
			else{
				cUserProfile->makeContactCall(phoneNumber.toStdString());//VOXOX CHANGE by Rolando - 2009.06.23 
			}
		}
	}
}

QString QtDialpad::getPhoneNumberOnlyDigits(QString phoneNumber){
	QString unFormattedPhoneNumber;
	unFormattedPhoneNumber = phoneNumber;
	unFormattedPhoneNumber.remove(QRegExp(QString("[/\\- \\(\\)]+")));

	return unFormattedPhoneNumber;

}

//VOXOX CHANGE by Rolando - 2009.06.17 
void QtDialpad::smsButtonClicked() {
	QString phoneNumber = _ui->phoneNumberLabel->text();
	phoneNumber = getPhoneNumberOnlyDigits(phoneNumber);//VOXOX CHANGE by Rolando - 2009.06.30 
	if(phoneNumber != ""){
		CUserProfile * cUserProfile = _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
		if (cUserProfile){
			if(phoneNumber.startsWith(PLUS_CHARACTER)){//VOXOX CHANGE by Rolando - 2009.06.23 - added to replace "+" by "011"
				phoneNumber.replace(0, PLUS_CHARACTER.length(),INTERNATIONAL_PREFIX_NUMBER);//VOXOX CHANGE by Rolando - 2009.06.23 - added to replace "+" by "011"
			}
			cUserProfile->startIMToSMS(phoneNumber.toStdString());
		}
	}	
}


//VOXOX CHANGE by Rolando - 2009.06.17 
void QtDialpad::faxButtonClicked() {
	//TODO: uncomment these sentences when fax functionality is ready and working fine.//VOXOX CHANGE by Rolando - 2009.06.30 
	QString phoneNumber = _ui->phoneNumberLabel->text();//VOXOX CHANGE by Rolando - 2009.06.30
	phoneNumber = getPhoneNumberOnlyDigits(phoneNumber);//VOXOX CHANGE by Rolando - 2009.06.30 

	if(phoneNumber != ""){//VOXOX CHANGE by Rolando - 2009.06.30 
		CUserProfile * cUserProfile = _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();//VOXOX CHANGE by Rolando - 2009.06.30 
		if (cUserProfile){//VOXOX CHANGE by Rolando - 2009.06.30 
			if(phoneNumber.startsWith(PLUS_CHARACTER)){//VOXOX CHANGE by Rolando - 2009.06.30 
				phoneNumber.replace(0, PLUS_CHARACTER.length(),INTERNATIONAL_PREFIX_NUMBER);//VOXOX CHANGE by Rolando - 2009.06.30 
			}
			cUserProfile->sendFax(phoneNumber.toStdString());//VOXOX CHANGE by Rolando - 2009.06.30 
		}
	}
}

//VOXOX CHANGE by Rolando 04-29-09, in conference window was clicked start or cancel button so remove conference window
void QtDialpad::hideConferenceWidget() {
	_ui->stackedWidget->removeWidget(_qtConferenceCallWidget);
	setDefaultPage();
	OWSAFE_DELETE(_qtConferenceCallWidget);
	//_qtWengoPhone->setActiveTabBeforeCallCurrent();//VOXOX CHANGE by Rolando - 2009.07.08 
	
}

void QtDialpad::setDefaultPage(){
	_ui->stackedWidget->setCurrentWidget(_defaultPage);
}

//VOXOX CHANGE by Rolando 04-02-09
void QtDialpad::ratesButtonClicked(){
	WsUrl::showRatesPage();
}

//VOXOX CHANGE by Rolando 04-07-09
void QtDialpad::redial() { 
	CUserProfile * cUserProfile = _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
	if (cUserProfile) {
		IPhoneLine * phoneLine = cUserProfile->getUserProfile().getActivePhoneLine();
		if (phoneLine) {
			if (!phoneLine->getActivePhoneCall()) {
				phoneLine->redial();//VOXOX CHANGE by Rolando - 2009.05.28 - sentence commented
			} 
		}
	}
}

//VOXOX CHANGE by Rolando 04-07-09
void QtDialpad::voiceMailButtonClicked() { //VOXOX - CJC - 2009.06.29 
	/*CUserProfile * cUserProfile = _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
	if (cUserProfile) {
		IPhoneLine * phoneLine = cUserProfile->getUserProfile().getActivePhoneLine();
		if (phoneLine) {
			if (!phoneLine->getActivePhoneCall()) {
				phoneLine->makeCall(cUserProfile->getUserProfile().getSipAccount()->getVoicemailNumber(),NULL);				
			} 
		}
	}*/
	
	_qtWengoPhone->getQtBrowserWidget()->openVoiceMail();//VOXOX - CJC - 2009.06.29 

	_qtWengoPhone->showHomeTab();//VOXOX - CJC - 2009.06.29 

}

void QtDialpad::showInitiateConferenceCallWindow(){

	CUserProfile * cUserProfile = _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
	if (cUserProfile) {

		IPhoneLine * phoneLine = cUserProfile->getUserProfile().getActivePhoneLine();
		if (phoneLine) {
			if (phoneLine->getActivePhoneCall()) {//VOXOX CHANGE by Rolando - 2009.09.08 
				QtVoxMessageBox box(this);//VOXOX CHANGE by Rolando - 2009.09.08 
				QString message = "Conference can't be started while there are active phone calls.\nFinish all phone calls and try again.";//VOXOX CHANGE by Rolando - 2009.09.08 
				box.setWindowTitle(tr("VoxOx - Proceed Conference"));//VOXOX CHANGE by Rolando - 2009.09.08 
				box.setText(message);//VOXOX CHANGE by Rolando - 2009.09.08 
				box.setStandardButtons(QMessageBox::Ok);//VOXOX CHANGE by Rolando - 2009.09.08 
				box.exec();//VOXOX CHANGE by Rolando - 2009.09.08 
				
			} else {
				if(cUserProfile->getUserProfile().getSipAccount()->isConnected()){//VOXOX CHANGE by Rolando - 2009.09.08 
					_qtConferenceCallWidget = new QtConferenceCallWidget(this, _qtWengoPhone->getCWengoPhone(), phoneLine);
					_ui->stackedWidget->insertWidget(_ui->stackedWidget->count(), _qtConferenceCallWidget);
					SAFE_CONNECT(_qtConferenceCallWidget, SIGNAL(startClickedSignal()), SLOT(hideConferenceWidget()));
					SAFE_CONNECT(_qtConferenceCallWidget, SIGNAL(cancelClicked()), SLOT(hideConferenceWidget()));
					SAFE_CONNECT_RECEIVER(_qtConferenceCallWidget, SIGNAL(cancelConferenceCall(std::string)), _qtWengoPhone, SLOT(cancelConferenceCall(std::string)));
					_ui->stackedWidget->setCurrentWidget(_qtConferenceCallWidget);
				}
				else{
					QtVoxMessageBox box(this);//VOXOX CHANGE by Rolando - 2009.09.08 
					QString message = "Your phone account is not connected.\nPlease reconnect and try your conference call again.";//VOXOX CHANGE by Rolando - 2009.09.08 
					box.setWindowTitle(tr("VoxOx - Proceed Conference"));//VOXOX CHANGE by Rolando - 2009.09.08 
					box.setText(message);//VOXOX CHANGE by Rolando - 2009.09.08 
					box.setStandardButtons(QMessageBox::Ok);//VOXOX CHANGE by Rolando - 2009.09.08
					box.exec();//VOXOX CHANGE by Rolando - 2009.09.08 
				}
			}
		}
	}	
}

//VOXOX CHANGE by Rolando 04-29-09, this method shows classicDialpadPage
void QtDialpad::showClassicalDialpadWindow(){
	_ui->stackedWidget->setCurrentWidget(_ui->classicDialpadPage);
}

//VOXOX CHANGE by Rolando 04-02-09
void QtDialpad::conferenceButtonClicked(){
	showInitiateConferenceCallWindow();
}

//VOXOX CHANGE by Rolando 04-02-09
void QtDialpad::playTone(const std::string & tone) {

	switch(_themeMode){
		case ThemeModeDefaultOnly:
			_cDtmfThemeManager.playTone(DEFAULT_THEME,tone);//VOXOX CHANGE by Rolando 04-03-09
			break;
		case ThemeModeAudioSmileysOnly:
			_cDtmfThemeManager.playTone(JUNGLE_THEME,tone);//VOXOX CHANGE by Rolando 04-03-09
			break;
		default:;
	}
	
	_qtWengoPhone->dialpad(tone);
	//_ui->phoneNumberLabel->setText(_ui->phoneNumberLabel->text() + QString::fromStdString(tone));//VOXOX CHANGE by Rolando - 2009.06.23 
}

//VOXOX CHANGE by Rolando - 2009.06.23 
void QtDialpad::setPhoneNumber(QString phoneNumber){
	if(phoneNumber.startsWith(INTERNATIONAL_PREFIX_NUMBER)){//VOXOX CHANGE by Rolando - 2009.06.23 - replace QString "011" by "+"
		phoneNumber.replace(0, INTERNATIONAL_PREFIX_NUMBER.length(),PLUS_CHARACTER);
	}
	_ui->phoneNumberLabel->setText(phoneNumber);
}

void QtDialpad::oneButtonClicked() {
	playTone("1");
}

void QtDialpad::twoButtonClicked() {
	playTone("2");
}

void QtDialpad::threeButtonClicked() {
	playTone("3");
}

void QtDialpad::fourButtonClicked() {
	playTone("4");
}

void QtDialpad::fiveButtonClicked() {
	playTone("5");
}

void QtDialpad::sixButtonClicked() {
	playTone("6");
}

void QtDialpad::sevenButtonClicked() {
	playTone("7");
}

void QtDialpad::eightButtonClicked() {
	playTone("8");
}

void QtDialpad::nineButtonClicked() {
	playTone("9");
}

void QtDialpad::zeroButtonClicked() {
	playTone("0");
}

void QtDialpad::starButtonClicked() {
	playTone("*");
}

void QtDialpad::poundButtonClicked() {
	playTone("#");
}

//VOXOX CHANGE by Rolando - 2009.05.28 - added to handle keys pressed from 0-9 and "#", "*" keys
void QtDialpad::keyPressEvent ( QKeyEvent * event ){
	
	switch (event->key()){
		case Qt::Key_0:
			zeroButtonClicked();
			break;
		case Qt::Key_1:
			oneButtonClicked();
			break;
		case Qt::Key_2:
			twoButtonClicked();
			break;
		case Qt::Key_3:
			threeButtonClicked();
			break;
		case Qt::Key_4:
			fourButtonClicked();
			break;
		case Qt::Key_5:
			fiveButtonClicked();
			break;
		case Qt::Key_6:
			sixButtonClicked();
			break;
		case Qt::Key_7:
			sevenButtonClicked();
			break;
		case Qt::Key_8:
			eightButtonClicked();
			break;
		case Qt::Key_9:
			nineButtonClicked();
			break;
		case Qt::Key_Asterisk:
			starButtonClicked();
			break;
		case Qt::Key_NumberSign:			
			poundButtonClicked();
			break;
		case Qt::Key_Delete://VOXOX CHANGE by Rolando - 2009.06.24 
		case Qt::Key_Backspace://VOXOX CHANGE by Rolando - 2009.06.24 
			backButtonClicked();//VOXOX CHANGE by Rolando - 2009.06.24 
			break;
		default:			
			event->ignore();	
	}
}

//VOXOX CHANGE by Rolando 04-02-09
void QtDialpad::setThemeMode(ThemeMode themeMode) {
	_themeMode = themeMode;
	if (_themeMode == ThemeModeDefaultOnly) {
		_ui->stackedWidget->setCurrentWidget(_ui->classicDialpadPage);
	} else {
		std::string newThemeName = JUNGLE_THEME;
		const DtmfTheme * newTheme = _cDtmfThemeManager.getDtmfTheme(newThemeName);

		if (newTheme->getDialpadMode() == DtmfTheme::svg) {

			//change theme
			_ui->qtSVGDialpad->setNewTheme(newTheme->getRepertory());

			//active QtSVGDialpad
			_ui->stackedWidget->setCurrentWidget(_ui->svgDialpadPage);

			//force repaint
			_ui->qtSVGDialpad->update();

		}

	}
	
}

//VOXOX CHANGE by Rolando - 2009.05.28 
bool QtDialpad::isConferenceWindowShown(){

	if(_qtConferenceCallWidget){
		return _ui->stackedWidget->currentWidget() == _qtConferenceCallWidget;
	}
	else{
		return false;
	}
	
}

//VOXOX CHANGE by Rolando - 2009.05.28 
bool QtDialpad::isKeypadWindowShown(){
	return _ui->stackedWidget->currentWidget() == _ui->classicDialpadPage;
}
