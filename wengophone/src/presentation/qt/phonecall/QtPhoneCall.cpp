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
#include "QtPhoneCall.h"

#include "ui_PhoneCallWidget.h"

#ifdef XV_HWACCEL
	#include "QtVideoXV.h"
#endif

#include "QtVideoQt.h"
#include "QtPhoneCallEventFilter.h"
#include "../toaster/QtCallToaster.h"
#include "../dialpad/QtDialpad.h"//VOXOX CHANGE by Rolando 04-29-09, now QtDialpad is in dialpad folder

#include "QtAudioSmileysWindow.h"

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/QtToolBar.h>
#include <presentation/qt/statusbar/QtStatusBar.h>
#include <presentation/qt/contactlist/QtContactList.h>
#include <presentation/qt/profile/QtProfileDetails.h>
#include <presentation/qt/conference/QtConferenceCallWidget.h>
#include <presentation/qt/callbar/QtCallBar.h>
#include <presentation/qt/callbar/customCallBar/QtVoxOxCallBarFrame.h>

#include <sound/AudioDeviceManager.h>

#include <sound/VolumeControl.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/phonecall/CPhoneCall.h>

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/contactlist/Contact.h>
#include <model/phonecall/ConferenceCall.h>
#include <model/phonecall/PhoneCall.h>
#include <model/phoneline/PhoneLine.h>
#include <model/profile/AvatarList.h>

#include <sipwrapper/CodecList.h>
#include <sipwrapper/SipWrapper.h>

#include <cutil/global.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/WidgetRatioEnforcer.h>
#include <qtutil/WidgetUtils.h>
#include <qtutil/PixmapMerging.h>

#include <QtCore/QMutex>
#include <QtCore/QTime>
#include <QtCore/QTimer>

#include <QtGui/QMenu>
#include <QtGui/QPainter>

//VOXOX CHANGE by Rolando - 2009.06.16 
#include <presentation/qt/messagebox/QtVoxPhoneNumberMessageBox.h>


// If a PSTN number is shorter than this length, we assume it's a special sip
// account, like 333 for Wengo
static const int PSTN_NUMBER_MIN_LENGTH = 4;
static const std::string AVATAR_PHONECALL_BACKGROUND = ":/pics/avatar_background.png";

static const int LOCAL_FRAME_WIDTH = 64;
static const int LOCAL_FRAME_HEIGHT = 64;
static const int LOCAL_FRAME_MARGIN = 7;

// See wifo/phapi/phmedia-video.c
static const int NOWEBCAM_FILL_VALUE = 128;

static const int NOWEBCAM_SAMPLE_COUNT = 200;

// Minimum width of buttons shown when we are in "talking" state. Defined to avoid square buttons
// under MacOS X
static const int TALKING_BUTTON_MIN_WIDTH = 50;

/**
 * Helper function to determine if image is a real webcam or not:
 * If the remote user has no webcam, then wifo will send us a yuv image filled
 * with NOWEBCAM_FILL_VALUE bytes.
 */
static bool isRealWebcamVideoFrame(piximage* image) {
	if (image->palette != PIX_OSI_YUV420P) {
		return true;
	}

	int step = (image->width * image->height * 3 / 2) / NOWEBCAM_SAMPLE_COUNT;

	for (int sample=0; sample < NOWEBCAM_SAMPLE_COUNT; ++sample) {
		if (image->data[sample * step] != NOWEBCAM_FILL_VALUE) {
			return true;
		}
	}

	return false;
}

static void setButtonAction(QPushButton* button, QAction* action) {
	button->disconnect();
	SAFE_CONNECT_RECEIVER(button, SIGNAL(clicked()), action, SLOT(trigger()));
	button->setIcon(action->icon());
	button->setEnabled(action->isEnabled());
	button->setToolTip(action->toolTip());
}

static void flashMainWindow(QWidget* window) {
	if (window->isActiveWindow()) {
		return;
	}

	if (!window->isVisible()) {
		// Make sure the window appears on the taskbar,
		// otherwise it won't flash...
		window->showMinimized();
	}

	WidgetUtils::flashWindow(window);
}

QtPhoneCall::QtPhoneCall(CPhoneCall & cPhoneCall)
	: QObject(NULL),
	_cPhoneCall(cPhoneCall) {

	_qtWengoPhone = (QtWengoPhone *) _cPhoneCall.getCWengoPhone().getPresentation();
	_videoWindow = NULL;
	_closed = false;
	_duration = 0;

	_callToaster = NULL;
	_remoteVideoFrame = NULL;
	_localVideoFrame = NULL;
	_hold = false;
	_videoMode = VM_None;

	_mutex = new QMutex(QMutex::Recursive);
	_isReadyToTalk = false;//VOXOX CHANGE by Rolando - 2009.05.22 - this variable is used to avoid that the user clicks on hold button when a conference call is not ready
	_phoneCallWidget = new QWidget(NULL);
	_muteButtonOn = false;//VOXOX CHANGE by Rolando - 2009.06.04 

	_qtMiniDialpad = NULL;

	_ui = new Ui::PhoneCallWidget();
	_ui->setupUi(_phoneCallWidget);

	//VOXOX CHANGE by Rolando 04-07-09	
	_phoneCallWidget->setStyleSheet(QString("QLabel#phoneNumberLabel{ font-family: Tahoma; font-size: 12px;color: #dce602; } ")
		+ QString("QFrame#backgroundOpaqueFrame{ background-image: url(:/pics/phonecall/bg_opaque1.png); border: none; } ")		
		+ QString(" QLabel#durationLabel{ font-family: Tahoma; font-size: 15px;	color: #ffffff;} ")
		+ QString("QLabel#topImageLabel{ background-image: url(:/pics/phonecall/bgtopfade.png); background-repeat: repeat-x; border: 1px transparent; } ")
		+ QString(" QLabel#statusLabel{ font-family: Tahoma; font-size: 15px;	color: #ffffff;} "));

	//VOXOX CHANGE by Rolando 04-07-09	
	_ui->acceptButton->setImages(QString(":/pics/phonecall/btn_lrg_call.png"),QString(":/pics/phonecall/btn_lrg_call_press.png"));
	_ui->rejectButton->setImages(QString(":/pics/phonecall/btn_lrg_hangup.png"),QString(":/pics/phonecall/btn_lrg_hangup_press.png"));
	
	//VOXOX CHANGE by Rolando 04-07-09	
	_ui->hangupButton->setImages(QString(":/pics/phonecall/btn_opt_hangup"),QString(":/pics/phonecall/btn_opt_hangup_press.png"));
	_ui->muteButton->setImages(QString(":/pics/phonecall/btn_opt_mute.png"),QString(":/pics/phonecall/btn_opt_mute_press.png"));
	_ui->smileysButton->setImages(QString(":/pics/phonecall/btn_opt_fun.png"),QString(":/pics/phonecall/btn_opt_fun_press.png"));
	_ui->dialpadButton->setImages(QString(":/pics/phonecall/btn_opt_keypad.png"),QString(":/pics/phonecall/btn_opt_keypad_press.png"));
	_ui->transferCallButton->setImages(QString(":/pics/phonecall/btn_opt_transfer.png"),QString(":/pics/phonecall/btn_opt_transfer_press.png"));
	_ui->holdButton->setImages(QString(":/pics/phonecall/btn_opt_hold.png"),QString(":/pics/phonecall/btn_opt_hold_press.png"),QString(":/pics/phonecall/btn_opt_hold_on.png"));
	
	//_ui->encryptionFrame->hide();
	//_ui->videoContainer->hide();
	_ui->labelStack->setCurrentWidget(_ui->avatarPage);
	_ui->buttonStack->setCurrentWidget(_ui->ringingPage);

	_phoneNumber = QString::fromStdString(_cPhoneCall.getPhoneCall().getPeerSipAddress().getUserName());//VOXOX CHANGE by Rolando 04-29-09 gets phonenumber of caller
	
	loadDisplayNameByPhoneNumber(_phoneNumber);//VOXOX CHANGE by Rolando - 2009.10.06 

	QPixmap avatarPixmap = loadContactAvatar(_phoneNumber);//VOXOX CHANGE by Rolando - 2009.07.09 
	initAvatarLabel(avatarPixmap);


	Config & config = ConfigManager::getInstance().getCurrentConfig();

	// On MacOSX getVideoEnable causes quite a lot of work, like listing the
	// available devices. Therefore we keep the result instead of asking it
	// everytime it's needed.
	_videoEnabledInConfig = config.getVideoEnable();

	//init flip
	PhoneLine & phoneLine = dynamic_cast < PhoneLine & > (_cPhoneCall.getPhoneCall().getPhoneLine());
	phoneLine.flipVideoImage(config.getVideoFlipEnable());
	////

	//VOXOX CHANGE by Rolando 04-07-09	
//#ifndef OS_MACOSX
//	//With Qt4.2, the button background is set to _phoneCallWidget background
//	//color on Windows and Linux, setting autoFillBackground to true prevents
//	//this.
//	//We don't want to do this on OSX, because on this platform setting
//	//autoFillBackground to true paints horizontal stripes behind the buttons.
//	Q_FOREACH(QPushButton* button, _phoneCallWidget->findChildren<QPushButton*>()) {
//		button->setAutoFillBackground(true);
//	}
//	////
//#endif
//
//	Q_FOREACH(QPushButton* button, _ui->talkingPage->findChildren<QPushButton*>()) {
//		button->setMinimumWidth(TALKING_BUTTON_MIN_WIDTH);
//	}

	//VOXOX CHANGE by Rolando 04-07-09	
	// Set bold ourself: if we do it from Designer it alters the font name (at
	// least with Qt 4.1)
	/*QFont font(_ui->nickNameLabel->font());
	font.setBold(true);
	_ui->nickNameLabel->setFont(font);

	_ui->nickNameLabel->setText(userName);*/
	updateNickNameToolTip();

	//_ui->statusLabel->setToolTip(tr("Status"));

	// Accept call
	SAFE_CONNECT(_ui->acceptButton, SIGNAL(clicked()), SLOT(acceptCall()));

	// Reject call
	SAFE_CONNECT(_ui->rejectButton, SIGNAL(clicked()), SLOT(rejectCall()));

	// Tweak ui when we are the caller
	if (!isIncoming()) {
		_ui->acceptButton->hide();
		/*_ui->rejectButton->setText(tr("Ca&ncel"));*/
	}

	//VOXOX CHANGE by Rolando 04-07-09	
	// Hang up
	/*_actionHangupCall = new QAction(QIcon(":/pics/actions/hangup-phone.png"), tr("Hang-up"), _phoneCallWidget);	
	SAFE_CONNECT(_actionHangupCall, SIGNAL(triggered()), SLOT(rejectCall()));
	setButtonAction(_ui->hangupButton, _actionHangupCall);*/

	SAFE_CONNECT(_ui->hangupButton, SIGNAL(clicked()), SLOT(rejectCall()));

	//VOXOX CHANGE by Rolando 04-07-09	
	//Hold
	/*_actionHold = new QAction(_phoneCallWidget);
	SAFE_CONNECT(_actionHold, SIGNAL(triggered()), SLOT(holdOrResume()));*/

	SAFE_CONNECT(_ui->holdButton, SIGNAL(clicked()), SLOT(holdOrResume()));

	
	SAFE_CONNECT(_ui->transferCallButton, SIGNAL(clicked()), SLOT(transferCallButtonClicked()));
	SAFE_CONNECT(_ui->muteButton, SIGNAL(clicked()), SLOT(muteButtonClicked()));

	//VOXOX CHANGE by Rolando 04-07-09	
	//Add contact
	/*_actionAddContact = new QAction(QIcon(":/pics/actions/add-contact.png"), tr("Add contact"), _phoneCallWidget);
	SAFE_CONNECT(_actionAddContact, SIGNAL(triggered()), SLOT(addContact()));
	setButtonAction(_ui->addContactButton, _actionAddContact);*/
	////

	/*SAFE_CONNECT(_ui->dialpadButton, SIGNAL(toggled(bool)), SLOT(toggleDialpad(bool)) );//VOXOX CHANGE by Rolando - 2009.06.18 
	SAFE_CONNECT(_ui->smileysButton, SIGNAL(toggled(bool)), SLOT(toggleSmileys(bool)) );*///VOXOX CHANGE by Rolando - 2009.06.18 

	SAFE_CONNECT(_ui->dialpadButton, SIGNAL(clicked()), SLOT(dialpadClicked()) );
	SAFE_CONNECT(_ui->smileysButton, SIGNAL(clicked()), SLOT(smileysClicked()) );


	//Computes the call duration
	_callTimer = new QTimer(_phoneCallWidget);
	SAFE_CONNECT(_callTimer, SIGNAL(timeout()), SLOT(updateCallDuration()));

	QtPhoneCallEventFilter * filter = new QtPhoneCallEventFilter(_phoneCallWidget);
	_phoneCallWidget->installEventFilter(filter);

	flashMainWindow(_qtWengoPhone->getWidget());

	if(_displayName != ""){//VOXOX CHANGE by Rolando - 2009.10.06 
		showToaster(_displayName, avatarPixmap);//VOXOX CHANGE by Rolando - 2009.10.06 
	}
	else{//VOXOX CHANGE by Rolando - 2009.10.06
		showToaster(_phoneNumber, avatarPixmap);//VOXOX CHANGE by Rolando - 2009.10.06 
	}

	if (!_cPhoneCall.getPhoneCall().getConferenceCall()) {
		_ui->transferCallButton->setEnabled(true);
		_qtWengoPhone->addPhoneCall(this);
	} else {
		_ui->transferCallButton->setEnabled(false);
		_qtWengoPhone->addToConference(this);
	}

	updateStatusLabel();
	updateHoldAction();

	//update toolbar (menus)
	if (_qtWengoPhone) {
		_qtWengoPhone->getQtToolBar()->updateMenuActions();
	}
	////
	_ui->smileysButton->setEnabled(true);//VOXOX CHANGE by Rolando - 2009.06.24 - disabled according task "Disable Fun button in Call/Keypad Tab"

	_ui->acceptButton->setFocus();
	_ui->stackedWidget->setCurrentWidget(_ui->callPage);
}

QtPhoneCall::~QtPhoneCall() {
	if (_remoteVideoFrame) {
		pix_free(_remoteVideoFrame);
	}
	if (_localVideoFrame) {
		pix_free(_localVideoFrame);
	}

	if (_callToaster) {
		_callToaster->close();
	}

	if(_qtMiniDialpad){//VOXOX CHANGE by Rolando - 2009.05.22 - if in the moment that QtContactCallListWidget was destroyed and was shown _qtMiniDialpad then delete it
		OWSAFE_DELETE(_qtMiniDialpad);	
	}

	OWSAFE_DELETE(_ui);
	OWSAFE_DELETE(_mutex);
}

void QtPhoneCall::initAvatarLabel(const QPixmap& pixmap) {

	_ui->avatarLabel->setPixmap(pixmap);
	_ui->labelStack->setCurrentWidget(_ui->avatarPage);
}

//VOXOX CHANGE by Rolando - 2009.10.06 
void QtPhoneCall::loadDisplayNameByPhoneNumber(const QString& phoneNumber) {

	QtContactList * qtContactList = _qtWengoPhone->getQtContactList();//VOXOX CHANGE by Rolando - 2009.10.06 
	if (qtContactList)//VOXOX CHANGE by Rolando - 2009.10.06 
	{
		CContactList & cContactList = qtContactList->getCContactList();//VOXOX CHANGE by Rolando - 2009.10.06 
		Contact *		contact		= cContactList.getContactByNumber(phoneNumber.toStdString());//VOXOX CHANGE by Rolando - 2009.10.06 

		if(contact)//VOXOX CHANGE by Rolando - 2009.10.06 
		{
			_displayName = QString::fromStdString(contact->getDisplayName());//VOXOX CHANGE by Rolando - 2009.10.06 
		}
		
	}
	
}

//VOXOX CHANGE by Rolando - 2009.07.09 
QPixmap QtPhoneCall::loadContactAvatar(const QString& phoneNumber) {

	QPixmap pixmap;

	QtContactList * qtContactList = _qtWengoPhone->getQtContactList();//VOXOX CHANGE by Rolando - 2009.07.09 
	if (qtContactList) //VOXOX CHANGE by Rolando - 2009.07.09 
	{
		CContactList & cContactList = qtContactList->getCContactList();//VOXOX CHANGE by Rolando - 2009.07.09 
		Contact *		contact		= cContactList.getContactByNumber(phoneNumber.toStdString());//VOXOX CHANGE by Rolando - 2009.07.09 

		if(contact)//VOXOX CHANGE by Rolando - 2009.07.09 
		{
			//std::string contactId = contact->getKey();//VOXOX CHANGE by Rolando - 2009.07.09 
			//ContactProfile contactProfile = cContactList.getContactProfile(contactId);//VOXOX CHANGE by Rolando - 2009.07.09 
			//std::string data = contactProfile.getIcon().getData();//VOXOX CHANGE by Rolando - 2009.07.09 
			std::string data = contact->getIcon().getData();	//VOXOX - JRT - 2009.07.21 
			if (!data.empty()) 
			{
				pixmap.loadFromData((uchar *) data.c_str(), data.size());
			}
		}
		else
		{
			LOG_WARN("no contact found with phoneNumber:" + phoneNumber.toStdString());
		}

		
	}

	if (pixmap.isNull()) {
		// User is not in contact list, or has no avatar defined, use default
		// avatar instead
		std::string data = AvatarList::getInstance().getDefaultAvatarPicture().getData();
		pixmap.loadFromData((uchar*) data.c_str(), data.size());
	}

	pixmap = pixmap.scaled(_ui->avatarLabel->size(),Qt::KeepAspectRatio, Qt::SmoothTransformation);//VOXOX CHANGE by Rolando - 2009.07.09 

	//VOXOX CHANGE by Rolando - 2009.07.09 
	pixmap = PixmapMerging::mergeFromPixmap(pixmap, QPixmap(QString::fromStdString(AVATAR_PHONECALL_BACKGROUND)));
	return pixmap;
}


void QtPhoneCall::updateHoldAction() {
	//VOXOX CHANGE by Rolando 04-07-09	
	/*if (_hold) {
		_actionHold->setToolTip(tr("Resume"));
		_actionHold->setIcon(QIcon(":/pics/phonecall/resume-phone.png"));
	} else {
		_actionHold->setToolTip(tr("Hold"));
		_actionHold->setIcon(QIcon(":/pics/phonecall/hold-phone.png"));
	}
	setButtonAction(_ui->holdButton, _actionHold);*/

	//VOXOX CHANGE by Rolando 04-07-09

	if(_phoneCallWidget){//VOXOX CHANGE by Rolando - 2009.07.10 
		if (_hold) {
			_ui->holdButton->setToolTip(tr("Resume"));
			_ui->holdButton->setImages(QString(":/pics/phonecall/btn_opt_hold.png"),QString(":/pics/phonecall/btn_opt_hold_press.png"),QString(":/pics/phonecall/btn_opt_hold_on.png"));
		} else {
			_ui->holdButton->setToolTip(tr("Hold"));
			_ui->holdButton->setImages(QString(":/pics/phonecall/btn_opt_resume.png"),QString(":/pics/phonecall/btn_opt_resume_press.png"),QString(":/pics/phonecall/btn_opt_resume_on.png"));
		}
	}
	//VOXOX CHANGE by Rolando 04-07-09	
	//setButtonAction(_ui->holdButton, _actionHold);
}


void QtPhoneCall::updateNickNameToolTip() {
	if(_ui){//VOXOX CHANGE by Rolando - 2009.07.10 
		QString toolTip = QString::fromStdString(_cPhoneCall.getPeerSipAddress());

		if(_displayName == ""){//VOXOX CHANGE by Rolando - 2009.10.06 
			_ui->phoneNumberLabel->setText(getPhoneNumberNoSuffix(toolTip));
		}
		else{//VOXOX CHANGE by Rolando - 2009.10.06 
			_ui->phoneNumberLabel->setText(_displayName + " (" + _phoneNumber + ")");//VOXOX CHANGE by Rolando - 2009.10.06 	
		}

		if (!_codecs.isEmpty()) {
			toolTip += QString(" (%1)").arg(_codecs);
		}
		//VOXOX CHANGE by Rolando 04-07-09	
		//_ui->nickNameLabel->setToolTip(toolTip);
		_ui->phoneNumberLabel->setToolTip(toolTip);
	}
}

//VOXOX CHANGE by Rolando 04-29-09 updates phonenumber variable according cPhoneCall phonenumber
void QtPhoneCall::updatePhoneNumber() {
	_phoneNumber = getPhoneNumberNoSuffix(QString::fromStdString(_cPhoneCall.getPeerSipAddress()));
}

//VOXOX CHANGE by Rolando 04-29-09 gets displayname @realm address
QString QtPhoneCall::getDisplayName(QString str) {
	QString tmp;

	int begin = str.indexOf("sip:", 0, Qt::CaseInsensitive);
	if (begin == -1) {
		// Not found, return ...
		return str;
	}
	begin += 4;
	int end = str.indexOf("@", begin, Qt::CaseInsensitive);
	if (end == -1) {
		//Not found, return ...
		return str;
	}
	tmp = str.mid(begin, end - begin);
	return tmp;
}

//VOXOX CHANGE by Rolando - 2009.06.03
QString QtPhoneCall::getPhoneNumber(){
	return _phoneNumber;
}

//VOXOX CHANGE by Rolando 04-29-09 gets phoneNumber without @realm address
QString QtPhoneCall::getPhoneNumberNoSuffix(QString str) {
	QString tmp;

	int begin = str.indexOf("sip:", 0, Qt::CaseInsensitive);
	if (begin == -1) {
		// Not found, return ...
		return str;
	}
	begin += 4;
	int end = str.indexOf("@", begin, Qt::CaseInsensitive);
	if (end == -1) {
		//Not found, return ...
		return str;
	}
	tmp = str.mid(begin, end - begin);
	return tmp;
}

//VOXOX CHANGE by Rolando 04-29-09 this method updates state of call
void QtPhoneCall::stateChangedEvent(EnumPhoneCallState::PhoneCallState state) {
	std::string codecs;
	if (_cPhoneCall.getAudioCodecUsed() != CodecList::AudioCodecError) {
		codecs += CodecList::toString(_cPhoneCall.getAudioCodecUsed());
	}
	if (_cPhoneCall.getVideoCodecUsed() != CodecList::VideoCodecError) {
		codecs += "/" + CodecList::toString(_cPhoneCall.getVideoCodecUsed());
	}
	_codecs = QString::fromStdString(codecs);

	if(_ui){//VOXOX CHANGE by Rolando - 2009.06.19 
		updateNickNameToolTip();

		updateStatusLabel();

		//update toolbar (menus)
		if (_qtWengoPhone) {
			_qtWengoPhone->getQtToolBar()->updateMenuActions();
		}

		switch (state) {
		case EnumPhoneCallState::PhoneCallStateTalking:
			_ui->buttonStack->setCurrentWidget(_ui->talkingPage);
			_duration = 0;
			_callTimer->start(1000);
			/*_actionHangupCall->setEnabled(true);*/
			_ui->hangupButton->setEnabled(true);//VOXOX CHANGE by Rolando 04-07-09
			_isReadyToTalk = true;//VOXOX CHANGE by Rolando - 2009.05.22 - this variable is used to avoid that the user clicks on hold button when a conference call is not ready

			//VOXOX CHANGE by Rolando 04-07-09	
			//_ui->encryptionFrame->setVisible(isCallEncrypted());

			startedTalking(this);
			break;

		case EnumPhoneCallState::PhoneCallStateDialing:
		case EnumPhoneCallState::PhoneCallStateRinging:
		case EnumPhoneCallState::PhoneCallStateRingingStart:
		case EnumPhoneCallState::PhoneCallStateRingingStop:
		case EnumPhoneCallState::PhoneCallStateIncoming:
			_isReadyToTalk = false;//VOXOX CHANGE by Rolando - 2009.05.22 - this variable is used to avoid that the user clicks on hold button when a conference call is not ready
			/*_actionHangupCall->setEnabled(true);*///VOXOX CHANGE by Rolando 04-07-09	
			_ui->hangupButton->setEnabled(true);//VOXOX CHANGE by Rolando 04-07-09	
			break;

		case EnumPhoneCallState::PhoneCallStateHold:
			_isReadyToTalk = true;//VOXOX CHANGE by Rolando - 2009.05.22 - this variable is used to avoid that the user clicks on hold button when a conference call is not ready
			_hold = true;
			updateHoldAction();
			break;

		case EnumPhoneCallState::PhoneCallStateResumed:
			_isReadyToTalk = true;//VOXOX CHANGE by Rolando - 2009.05.22 - this variable is used to avoid that the user clicks on hold button when a conference call is not ready
			_hold = false;
			updateHoldAction();
			break;

		case EnumPhoneCallState::PhoneCallStateUnknown:
		case EnumPhoneCallState::PhoneCallStateError:
		case EnumPhoneCallState::PhoneCallStateMissed:
		case EnumPhoneCallState::PhoneCallStateRedirected:
			_isReadyToTalk = false;//VOXOX CHANGE by Rolando - 2009.05.22 - this variable is used to avoid that the user clicks on hold button when a conference call is not ready
			break;

		case EnumPhoneCallState::PhoneCallStateClosed:
			_isReadyToTalk = false;//VOXOX CHANGE by Rolando - 2009.05.22 - this variable is used to avoid that the user clicks on hold button when a conference call is not ready
			LOG_FATAL("should never reach this case since PPhoneCall::close() is done for this purpose");
			break;

		default:
			_isReadyToTalk = false;//VOXOX CHANGE by Rolando - 2009.05.22 - this variable is used to avoid that the user clicks on hold button when a conference call is not ready
			LOG_FATAL("unknown PhoneCallState=" + EnumPhoneCallState::toString(state));
		}
	}
}

void QtPhoneCall::initVideo(piximage* remoteVideoFrame, piximage* localVideoFrame) {
	_remoteVideoFrame = remoteVideoFrame;
	_localVideoFrame = localVideoFrame;

	if (!isRealWebcamVideoFrame(remoteVideoFrame)) {
		if (_videoEnabledInConfig) {
			_videoMode = VM_LocalOnly;
		} else {
			_videoMode = VM_None;
		}
		return;
	}

	// We only get there if we receive a real video from the remote user

	if (_videoEnabledInConfig) {
		_videoMode = VM_Both;
	} else {
		_videoMode = VM_RemoteOnly;
	}
	// Hide avatar label to save space for the video
	//_ui->avatarLabel->hide();

#ifdef XV_HWACCEL
	if (config.getXVideoEnable()) {
		_videoWindow = new QtVideoXV(_phoneCallWidget, remoteVideoFrame->width, remoteVideoFrame->height,
			localVideoFrame->width, localVideoFrame->height);
		if (!_videoWindow->isInitialized()) {
			OWSAFE_DELETE(_videoWindow);
		}
	}
#endif

	// We can fallback from a failure in QtVideoXV
	if (!_videoWindow) {
		_videoWindow = new QtVideoQt(_ui->videoContainer);
		SAFE_CONNECT(_videoWindow, SIGNAL(toggleFlipVideoImageSignal()), SLOT(toggleFlipVideoImage()));
	}

	// Init videoContainer
	double ratio = remoteVideoFrame->width / double(remoteVideoFrame->height);
	_ui->videoContainer->setRatio(ratio);
	_ui->videoContainer->setChild(_videoWindow->getWidget());
	QTimer::singleShot(0, _videoWindow->getWidget(), SLOT(show()));
	//_ui->videoContainer->show();
	_ui->labelStack->setCurrentWidget(_ui->videoPage);
}

void QtPhoneCall::videoFrameReceivedEvent(piximage * remoteVideoFrame, piximage * localVideoFrame) {

	QMutexLocker locker(_mutex);

	//FIXME hack to prevent a crash
	if (_closed) {
		return;
	}

	if (_videoMode == VM_None) {
		initVideo(remoteVideoFrame, localVideoFrame);
	}

	if (_videoMode == VM_RemoteOnly || _videoMode == VM_Both) {
		_videoWindow->showImage(remoteVideoFrame, localVideoFrame);
	}

	if (_videoMode == VM_LocalOnly) {
		QImage image;
		QSize size(_localVideoFrame->width, _localVideoFrame->height);
		size.scale(LOCAL_FRAME_WIDTH, LOCAL_FRAME_HEIGHT, Qt::KeepAspectRatio);
		if (size.width() & 1) {
			size.setWidth(size.width() + 1);
		}
		if (size.height() & 1) {
			size.setHeight(size.height() + 1);
		}
		QtVideoQt::convertPixImageToQImage(_localVideoFrame, size, &image);
		QPixmap avatarPix = *_ui->avatarLabel->pixmap();
		QPainter painter(&avatarPix);
		painter.drawImage(
			avatarPix.width() - size.width() - LOCAL_FRAME_MARGIN,
			avatarPix.height() - size.height() - LOCAL_FRAME_MARGIN,
			image);
		painter.end();
		_ui->avatarLabel->setPixmap(avatarPix);
	}
}

void QtPhoneCall::acceptCall() {
	_ui->statusLabel->setText(tr("Initialization..."));
	_cPhoneCall.accept();
	if (_callToaster) {
		_callToaster->close();
	}
}

void QtPhoneCall::rejectCall() {
	LOG_DEBUG("phone call hangup");
	switch (_cPhoneCall.getState()) {
	case EnumPhoneCallState::PhoneCallStateClosed:
	case EnumPhoneCallState::PhoneCallStateError:
	case EnumPhoneCallState::PhoneCallStateResumed:
	case EnumPhoneCallState::PhoneCallStateTalking:
	case EnumPhoneCallState::PhoneCallStateDialing:
	case EnumPhoneCallState::PhoneCallStateRinging:
	case EnumPhoneCallState::PhoneCallStateRingingStart:
	case EnumPhoneCallState::PhoneCallStateRingingStop:
	case EnumPhoneCallState::PhoneCallStateIncoming:
	case EnumPhoneCallState::PhoneCallStateHold:
	case EnumPhoneCallState::PhoneCallStateRedirected:
		_cPhoneCall.hangUp();
		break;
	default:
		LOG_DEBUG("call rejected");
	}
}

void QtPhoneCall::holdOrResume() {
	if (_hold) {
		_cPhoneCall.resume();
	} else {
		_cPhoneCall.hold();
	}
	updateHoldAction();
}

//VOXOX CHANGE by Rolando 04-07-09	
void QtPhoneCall::muteButtonClicked() {
	if(!_muteButtonOn){
		Config & config = ConfigManager::getInstance().getCurrentConfig();
		VolumeControl volumeControl(AudioDevice(AudioDeviceManager::getInstance().getDefaultInputDevice()));
		volumeControl.setLevel(0);
		_muteButtonOn = true;
		_ui->muteButton->setToolTip(QString("Mute On"));
	}
	else{
		Config & config = ConfigManager::getInstance().getCurrentConfig();
		VolumeControl volumeControl(AudioDevice(AudioDeviceManager::getInstance().getDefaultInputDevice()));
		volumeControl.setLevel(100);//VOXOX CHANGE by Rolando - 2009.06.16 
		_muteButtonOn = false;
		_ui->muteButton->setToolTip(QString("Mute Off"));
	}
}


//VOXOX CHANGE by Rolando 04-07-09	
void QtPhoneCall::transferCallButtonClicked() {

	QtVoxPhoneNumberMessageBox box(_phoneCallWidget);//VOXOX CHANGE by Rolando - 2009.06.16 

	box.setTitle(tr("VoxOx - Call Transfer"));//VOXOX CHANGE by Rolando - 2009.06.16 
	box.setDialogText(tr("Please type the number you would like to transfer this call\nthen click on \"OK\" button"));//VOXOX CHANGE by Rolando - 2009.06.16 
	box.setNumberTypeVisible(false);//VOXOX CHANGE by Rolando - 2009.06.16 
	
	if(box.exec() == QDialog::Accepted){//VOXOX CHANGE by Rolando - 2009.06.16 
		QString forwardNumber = box.getNumber();//VOXOX CHANGE by Rolando - 2009.06.16 
		if (forwardNumber != ""){//VOXOX CHANGE by Rolando - 2009.06.16 
			//HOLD CALL FIRST
			  _cPhoneCall.hold();//VOXOX CHANGE by Rolando - 2009.06.16 
			 _cPhoneCall.blindTransfer(forwardNumber.toStdString());//VOXOX CHANGE by Rolando - 2009.06.16 
		}
	}

}

void QtPhoneCall::updateCallDuration() {
	if(_ui){//VOXOX CHANGE by Rolando - 2009.06.27 
		_duration++;
		QTime time;
		time = time.addSecs(_duration);
		_ui->durationLabel->setText(time.toString(Qt::TextDate));
	}

	//updateStatusLabel();//VOXOX CHANGE by Rolando 04-07-09	
}

void QtPhoneCall::updateStatusLabel() {
	
	if(_ui){//VOXOX CHANGE by Rolando - 2009.06.27 
		QString text;
		switch (_cPhoneCall.getState()) {
		case EnumPhoneCallState::PhoneCallStateError:
			text = tr("Error");
			break;

		case EnumPhoneCallState::PhoneCallStateResumed:
		case EnumPhoneCallState::PhoneCallStateTalking:
			if (_cPhoneCall.getPhoneCall().getConferenceCall()) {
				text = tr("Talking - Conference");
			} else {
				text = tr("Talking");
			}
			break;

		case EnumPhoneCallState::PhoneCallStateDialing:
			//VOXOX CHANGE by Rolando 04-07-09	
			if(_ui->buttonStack->currentIndex() == _ui->buttonStack->indexOf(_ui->talkingPage) ){
				text = tr("");
			}
			else{
				text = tr("Dialing");
			}
			break;

		case EnumPhoneCallState::PhoneCallStateRinging:
			text = tr("Ringing");
			break;

		case EnumPhoneCallState::PhoneCallStateRingingStart:
			text = tr("Ringing");
			break;

		case EnumPhoneCallState::PhoneCallStateIncoming:
			text = tr("Incoming Call");
			break;

		case EnumPhoneCallState::PhoneCallStateHold:
			text = tr("Hold");		
			break;

		case EnumPhoneCallState::PhoneCallStateRedirected:
			text = tr("Redirected");
			break;

		default:
			break;
		}
		_ui->statusLabel->setText(text);
	}

	//VOXOX CHANGE by Rolando 04-07-09	
	/*QTime time;
	time = time.addSecs(_duration);
	_ui->durationLabel->setText(time.toString(Qt::TextDate));*/
}

bool QtPhoneCall::isIncoming() const {
	return (_cPhoneCall.getState() == EnumPhoneCallState::PhoneCallStateIncoming);
}

void QtPhoneCall::showToaster(const QString & userName, const QPixmap& pixmap) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	if (!isIncoming()) {
		return;
	}

	//Shows toaster for incoming chats?
	if (!config.getNotificationShowToasterOnIncomingCall()) {
		return;
	}

	OWSAFE_DELETE(_callToaster);
	_callToaster = new QtCallToaster();
	SAFE_CONNECT(_callToaster, SIGNAL(destroyed(QObject*)),
		SLOT(slotCallToasterDestroyed(QObject*)));

	_callToaster->setMessage(userName);
	_callToaster->setPixmap(pixmap);

	SAFE_CONNECT(_callToaster, SIGNAL(pickUpButtonClicked()), SLOT(acceptCall()));
	SAFE_CONNECT(_callToaster, SIGNAL(hangUpButtonClicked()), SLOT(rejectCall()));
	_callToaster->show();
}

void QtPhoneCall::close() {
	//FIXME hack to prevent a crash
	_closed = true;

	if (_callToaster) {
		_callToaster->close();
	}

	_callTimer->disconnect();
	_callTimer->stop();
	OWSAFE_DELETE(_callTimer);

	/*_actionHangupCall->setEnabled(false);*/
	_ui->hangupButton->setEnabled(false);
	//_ui->statusLabel->setText(tr("Closed"));
	if (_videoWindow) {
		if (_videoWindow->isFullScreen()) {
			_videoWindow->unFullScreen();
		}
	}

	//update toolbar (menus)
	if (_qtWengoPhone) {
		_qtWengoPhone->getQtToolBar()->updateMenuActions();
	}
	////

	//TODO: disconnect from flipWebcamButtonClicked
	OWSAFE_DELETE(_videoWindow);
	OWSAFE_DELETE(_phoneCallWidget);
	deleteLater();
}

void QtPhoneCall::slotCallToasterDestroyed(QObject * callToaster){
	_callToaster = NULL;
}

void QtPhoneCall::toggleFlipVideoImage() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	PhoneLine & phoneLine = dynamic_cast < PhoneLine & > (_cPhoneCall.getPhoneCall().getPhoneLine());
	bool flip = !config.getVideoFlipEnable();
	phoneLine.flipVideoImage(flip);
	config.set(Config::VIDEO_ENABLE_FLIP_KEY, flip);
}

/**
 * Helper function trying to determine if a username is a PSTN number.
 */
static bool isPSTNUserName(const QString& userName) {
	for (int pos=0; pos < userName.size(); ++pos) {
		if (!userName[0].isDigit()) {
			return false;
		}
	}

	return userName.size() >= PSTN_NUMBER_MIN_LENGTH;
}

void QtPhoneCall::addContact() {
	//CWengoPhone & cWengoPhone = _cPhoneCall.getCWengoPhone();
	//CUserProfile* cUserProfile =  cWengoPhone.getCUserProfileHandler().getCUserProfile();
	//if (!cUserProfile) {
	//	LOG_WARN("No user profile defined. This should not happen");
	//	return;
	//}

	//ContactProfile contactProfile;
	//QtProfileDetails qtProfileDetails(*cUserProfile, contactProfile, _phoneCallWidget->window(), tr("Add a Contact"));

	//// Fill some QtProfileDetails fields
	//PhoneCall& call = _cPhoneCall.getPhoneCall();
	//SipAddress sipAddress = call.getPeerSipAddress();

	//QString rawSipAddress = QString::fromStdString(sipAddress.getRawSipAddress());
	//Config & config = ConfigManager::getInstance().getCurrentConfig();
	//QString wengoRealm = QString::fromStdString( config.getWengoRealm() );
	//if (rawSipAddress.contains(wengoRealm)) {
	//	// Wengo server
	//	QString userName;
	//	userName = QString::fromUtf8(sipAddress.getDisplayName().c_str());
	//	if (userName.isEmpty()) {
	//		userName = QString::fromUtf8(sipAddress.getUserName().c_str());
	//	}
	//	if (isPSTNUserName(userName)) {
	//		// User name is the phone number
	//		qtProfileDetails.setHomePhone(userName);
	//	} else {
	//		// User name is the name of the Wengo account
	//		qtProfileDetails.setWengoName(userName);
	//	}
	//} else {
	//	// External SIP server
	//	QString address = QString::fromStdString(sipAddress.getSipAddress());
	//	if (address.startsWith("sip:")) {
	//		address = address.mid(4);
	//	}
	//	qtProfileDetails.setSipAddress(address);
	//}
	//////

	//if (qtProfileDetails.show()) {
	//	// WARNING: if the user hang up while the dialog is shown, 'this' won't
	//	// be valid anymore. Therefore we must not use any field of 'this'
	//	// here.
	//	cUserProfile->getCContactList().addContact(contactProfile);
	//}
}

bool QtPhoneCall::isCallEncrypted() {
	return _cPhoneCall.isCallEncrypted();
}

//VOXOX CHANGE by Rolando - 2009.07.24 - method not needed anymore
/*
//VOXOX CHANGE by Rolando - 2009.05.22, this method is called when _qtMiniDialpad is destroyed so it removes this widget from stackedwidget
void QtPhoneCall::slotQtMiniDialpadDestroyed(QObject * object){
	_ui->stackedWidget->removeWidget(_qtMiniDialpad->getWidget());
	_qtMiniDialpad = NULL;
	
}*/

//VOXOX CHANGE by Rolando - 2009.06.18 
void QtPhoneCall::dialpadClicked() {
	showMiniDialpadWindow();
	if(_qtMiniDialpad){
		_qtMiniDialpad->setThemeMode(QtMiniDialpad::ThemeModeDefaultOnly);//VOXOX CHANGE by Rolando 04-24-09
		_qtMiniDialpad->setFirstParticipantPhoneNumber(_phoneNumber);

		_qtMiniDialpad->setDurationTime(getSecondsCallDuration());
	}	
}

//VOXOX CHANGE by Rolando 04-29-09, returns number of seconds of call
int QtPhoneCall::getSecondsCallDuration(){
	return _duration;
}

//VOXOX CHANGE by Rolando - 2009.06.18 
void QtPhoneCall::smileysClicked() {	
	showMiniDialpadWindow();
	if(_qtMiniDialpad){
		_qtMiniDialpad->setThemeMode(QtMiniDialpad::ThemeModeAudioSmileysOnly);//VOXOX CHANGE by Rolando 04-24-09
		_qtMiniDialpad->setFirstParticipantPhoneNumber(_phoneNumber);

		_qtMiniDialpad->setDurationTime(getSecondsCallDuration());
	}	
}

//VOXOX CHANGE by Rolando - 2009.05.22, this method shows QtMiniDialpad window and inserts it to stackedwidget
void QtPhoneCall::showMiniDialpadWindow(){

	if(!_qtMiniDialpad){
		_qtMiniDialpad = new QtMiniDialpad(_qtWengoPhone->getQtDialpad()->getCDtmfThemeManager(), _qtWengoPhone);
		//SAFE_CONNECT(_qtMiniDialpad, SIGNAL(destroyed(QObject*)),SLOT(slotQtMiniDialpadDestroyed(QObject*)));//VOXOX CHANGE by Rolando - 2009.07.24 - no needed anymore
		SAFE_CONNECT(_qtMiniDialpad, SIGNAL(backMiniDialpadClicked()),SLOT(backMiniDialpadClickedSlot()));
		SAFE_CONNECT(_qtMiniDialpad, SIGNAL(backButtonFunClicked()),SLOT(backFunMiniDialpadClickedSlot()));
		SAFE_CONNECT(_qtMiniDialpad, SIGNAL(hangUpMiniDialpadClicked()),SLOT(hangUpMiniDialpadSlot()));
		_ui->stackedWidget->insertWidget(_ui->stackedWidget->count(), _qtMiniDialpad->getWidget());
	}
	_ui->stackedWidget->setCurrentWidget(_qtMiniDialpad->getWidget());
}

void QtPhoneCall::backMiniDialpadClickedSlot(){
	_ui->stackedWidget->setCurrentWidget(_ui->callPage);
}

void QtPhoneCall::backFunMiniDialpadClickedSlot(){
	_ui->stackedWidget->setCurrentWidget(_ui->callPage);
}

void QtPhoneCall::hangUpMiniDialpadSlot(){
	_ui->stackedWidget->setCurrentWidget(_ui->callPage);
	rejectCall();	
}
