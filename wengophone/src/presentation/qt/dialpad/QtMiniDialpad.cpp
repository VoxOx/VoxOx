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
#include "QtMiniDialpad.h"

#include "ui_MiniDialpadWidget.h"

#include <presentation/qt/dialpad/QtDialpad.h>//VOXOX CHANGE by Rolando 04-29-09, now QtDialpad is in dialpad folder
#include <presentation/qt/dialpad/QtSVGDialpad.h>//VOXOX CHANGE by Rolando 04-29-09, now QtSVGDialpad is in dialpad folder
#include "conference/QtConferenceCallWidget.h"
#include "QtWengoPhone.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/dtmf/DtmfTheme.h>
#include <model/webservices/url/WsUrl.h>
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

//VOXOX CHANGE by Rolando 04-02-09
const char* MINI_DEFAULT_THEME = "default";
const char* MINI_JUNGLE_THEME = "terminator";

const int MINIMUM_SIZE = 307;

QtMiniDialpad::QtMiniDialpad(CDtmfThemeManager & cDtmfThemeManager, QtWengoPhone * qtWengoPhone, QString firstParticipantPhoneNumber, QString currentDuration, QString secondParticipantPhoneNumber)
	: QObject(NULL),
	_cDtmfThemeManager(cDtmfThemeManager),
	_firstParticipantPhoneNumber(firstParticipantPhoneNumber),
	_secondParticipantPhoneNumber(secondParticipantPhoneNumber),
	_currentDuration(currentDuration)
	{

	_qtWengoPhone = qtWengoPhone;

	_miniDialpadWidget = new QWidget(0);
	_ui = new Ui::MiniDialpadWidget();
	_ui->setupUi(_miniDialpadWidget);

//VOXOX CHANGE by Rolando 2009.05.05 - DialpadWidget.ui is not showing as it should on Windows Version
//	that's why we fixed it setting a margin and spacing to 0
#if (defined OS_WINDOWS)
	_ui->frame->layout()->setMargin(0);
	_ui->frame->layout()->setSpacing(0);
#endif	

	_miniDialpadWidget->setMinimumHeight(MINIMUM_SIZE);

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
	SAFE_CONNECT(_ui->hangUpButton, SIGNAL(leftMouseButtonPress()), SLOT(hangUpButtonClicked()));
	
	//VOXOX CHANGE by Rolando 04-02-09
	_miniDialpadWidget->setStyleSheet(QString("QLabel#firstParticipantPhoneNumberLabel{ color: #dce602; font-size: 12px; font-family: \"Tahoma\"; } ")
				  + QString("QLabel#secondParticipantPhoneNumberLabel{ color: #dce602; font-size: 12px; font-family: \"Tahoma\"; } ")
				  + QString("QLabel#labelSVG{ color: #dce602; font-size: 12px; font-family: \"Tahoma\"; } ")
				  + QString("QLabel#durationLabel{ color: #ffffff; font-size: 15px; font-family: \"Tahoma\"; } ")
				  + QString("QLabel#plusSignLabel{ color: #ffffff; font-size: 12px; font-weight: bold; font-family: \"Tahoma\"; } "));

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
	_ui->hangUpButton->setImages(QString(":/pics/dialpad/btn_hangup.png"),QString(":/pics/dialpad/btn_hangup_press.png"));
	_ui->backButton->setImages(QString(":/pics/dialpad/btn_nokeypad.png"),QString(":/pics/dialpad/btn_nokeypad_press.png"));
	_ui->backButtonFun->setImages(QString(":/pics/phonecall/btn_back_arrow_up.png"),QString(":/pics/phonecall/btn_back_arrow_down.png"));

	if(_secondParticipantPhoneNumber != ""){
		_ui->secondParticipantPhoneNumberLabel->setText(_secondParticipantPhoneNumber);
	}
	else{
		_ui->secondParticipantFrame->setVisible(false);
	}

	if(_firstParticipantPhoneNumber != ""){
		_ui->firstParticipantPhoneNumberLabel->setText(_firstParticipantPhoneNumber);
	}
	
	if(_currentDuration != ""){
		
		_duration = _currentDuration.toInt();
		QTime time;
		time = time.addSecs(_duration);
		_ui->durationLabel->setText(time.toString(Qt::TextDate));	

		_callTimer = new QTimer(this);
		_callTimer->start(1000);
		SAFE_CONNECT(_callTimer, SIGNAL(timeout()), SLOT(updateCallDuration()));
		
	}
		
	
	
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
	SAFE_CONNECT(_ui->backButtonFun, SIGNAL(clicked()), SLOT(backButtonFunClickedSlot()));
	
	//VOXOX CHANGE by Rolando 04-02-09
	setThemeMode(ThemeModeDefaultOnly);
	
	//VOXOX CHANGE ASV 04-06-09: we need a margin on top for Mac so the tab bar doesn't get on top of the widget content.
	#if defined(OS_MACOSX)
		_miniDialpadWidget->layout()->setContentsMargins(0, 11, 0, 0);
	#endif
	
}

QtMiniDialpad::~QtMiniDialpad() {
		
	if(_callTimer){
		if(_callTimer->isActive()){
			_callTimer->stop();
		}
		OWSAFE_DELETE(_callTimer);
	}

	/*if(_miniDialpadWidget){
		OWSAFE_DELETE(_miniDialpadWidget);
	}*/
	OWSAFE_DELETE(_ui);
	
}

void QtMiniDialpad::setFirstParticipantPhoneNumber(QString firstParticipantPhoneNumber){
	if(firstParticipantPhoneNumber != ""){
		_ui->firstParticipantPhoneNumberLabel->setText(firstParticipantPhoneNumber);
	}	
}

void QtMiniDialpad::setSecondParticipantPhoneNumber(QString secondParticipantPhoneNumber){
	_ui->secondParticipantPhoneNumberLabel->setText(secondParticipantPhoneNumber);
	_ui->secondParticipantFrame->setVisible(true);
}

void QtMiniDialpad::setDurationTime(int seconds){
	_duration = seconds;
	_callTimer = new QTimer(this);
	_callTimer->start(1000);
	SAFE_CONNECT(_callTimer, SIGNAL(timeout()), SLOT(updateCallDuration()));
	
}

void QtMiniDialpad::updateCallDuration() {
	if(_miniDialpadWidget){
		_duration++;
		QTime time;
		time = time.addSecs(_duration);
		_ui->durationLabel->setText(time.toString(Qt::TextDate));
	}
	else{
		LOG_WARN("trying to update call duration when ui not exists");//VOXOX CHANGE by Rolando - 2009.06.29 
	}
}
//VOXOX CHANGE by Rolando 04-02-09
void QtMiniDialpad::backButtonClicked(){
	if(_callTimer->isActive()){
		_callTimer->stop();
	}
	backMiniDialpadClicked();
	
}

//VOXOX CHANGE by Rolando 04-02-09
void QtMiniDialpad::hangUpButtonClicked(){
	if(_callTimer->isActive()){
		_callTimer->stop();
	}
	hangUpMiniDialpadClicked();
	
	//deleteLater();//VOXOX CHANGE by Rolando - 2009.06.04 - no necessary anymore
}

//VOXOX CHANGE by Rolando 04-02-09
void QtMiniDialpad::playTone(const std::string & tone) {

	switch(_themeMode){
		case ThemeModeDefaultOnly:
			_cDtmfThemeManager.playTone(MINI_DEFAULT_THEME,tone);//VOXOX CHANGE by Rolando 04-03-09
			break;
		case ThemeModeAudioSmileysOnly:
			_cDtmfThemeManager.playTone(MINI_JUNGLE_THEME,tone);//VOXOX CHANGE by Rolando 04-03-09
			break;
		default:;
	}
	
	_qtWengoPhone->dialpad(tone);
}

void QtMiniDialpad::oneButtonClicked() {
	playTone("1");
}

void QtMiniDialpad::twoButtonClicked() {
	playTone("2");
}

void QtMiniDialpad::threeButtonClicked() {
	playTone("3");
}

void QtMiniDialpad::fourButtonClicked() {
	playTone("4");
}

void QtMiniDialpad::fiveButtonClicked() {
	playTone("5");
}

void QtMiniDialpad::sixButtonClicked() {
	playTone("6");
}

void QtMiniDialpad::sevenButtonClicked() {
	playTone("7");
}

void QtMiniDialpad::eightButtonClicked() {
	playTone("8");
}

void QtMiniDialpad::nineButtonClicked() {
	playTone("9");
}

void QtMiniDialpad::zeroButtonClicked() {
	playTone("0");
}

void QtMiniDialpad::starButtonClicked() {
	playTone("*");
}

void QtMiniDialpad::poundButtonClicked() {
	playTone("#");
}

void QtMiniDialpad::backButtonFunClickedSlot(){
	if(_callTimer->isActive()){
		_callTimer->stop();
	}

	backButtonFunClicked();

}

//VOXOX CHANGE by Rolando - 2009.05.28 - added to handle keys pressed from 0-9 and "#", "*" keys
void QtMiniDialpad::keyPressEvent ( QKeyEvent * event ){
	
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
		default:			
			event->ignore();	
	}
}

//VOXOX CHANGE by Rolando 04-02-09
void QtMiniDialpad::setThemeMode(ThemeMode themeMode) {
	_themeMode = themeMode;
	if (_themeMode == ThemeModeDefaultOnly) {
		_ui->stackedWidget->setCurrentWidget(_ui->classicDialpadPage);
	} else {
		std::string newThemeName = MINI_JUNGLE_THEME;
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
