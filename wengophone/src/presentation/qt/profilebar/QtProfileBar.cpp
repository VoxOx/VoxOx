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
 * GNU General Public License for more details.ew
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtProfileBar.h"

#include "QtIMProfileWidget.h"

#include <presentation/qt/QtIMAccountMonitor.h>

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/profile/QtUserProfile.h>

#include <control/CWengoPhone.h>

#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/connect/ConnectHandler.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/IMAccount.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>

#include <QtGui/QGridLayout>


QtProfileBar::QtProfileBar(QWidget * parent)
	: QFrame(parent),
	_cUserProfile(0),
	_cWengoPhone(0),
	_qtImAccountMonitor(0) {

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//	QVBoxLayout* mainLayout = new QVBoxLayout(this);		//VOXOX - JRT - 2009.04.16 - memory leak. Make it a memvar.
	_mainLayout = new QVBoxLayout(this);
	_mainLayout->setMargin(0);
	_mainLayout->setSpacing(0);

	//create internal widgets
	_qtImProfileWidget = new QtIMProfileWidget(this);
	_qtImProfileWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	_mainLayout->addWidget(_qtImProfileWidget);

	
	qRegisterMetaType<IMAccount>("IMAccount");

	SAFE_CONNECT_TYPE(this, SIGNAL(wsInfoWengosEvent(const QString &)),
		SLOT(wsInfoWengosEventSlot(const QString &)), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(wsInfoLandlineNumberEvent(const QString &)),
		SLOT(wsInfoLandlineNumberEventSlot(const QString &)), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(phoneLineCreatedEvent()),
		SLOT(phoneLineCreatedEventSlot()), Qt::QueuedConnection);

	reset();
}

QtProfileBar::~QtProfileBar() 
{
//JRT-XXX	OWSAFE_DELETE( _mainLayout		  );	//VOXOX - JRT - 2009.04.16 
//JRT-XXX	OWSAFE_DELETE( _qtImProfileWidget );	//VOXOX - JRT - 2009.04.16 
}


void QtProfileBar::init(CWengoPhone* cWengoPhone, CUserProfile* cUserProfile, QtIMAccountMonitor* qtImAccountMonitor) {
	_cWengoPhone = cWengoPhone;
	_cUserProfile = cUserProfile;
	_qtImAccountMonitor = qtImAccountMonitor;

	// Init widgets
	_qtImProfileWidget->init(_cUserProfile, _qtImAccountMonitor, dynamic_cast<QtWengoPhone*>(_cWengoPhone->getPresentation()));

	// Connect _cUserProfile
	_cUserProfile->getUserProfile().wsInfoCreatedEvent +=
		boost::bind(&QtProfileBar::wsInfoCreatedEventHandler, this, _1, _2);

	_cUserProfile->getUserProfile().phoneLineCreatedEvent +=
		boost::bind(&QtProfileBar::phoneLineCreatedEventHandler, this, _1, _2);


	//Check if events already occured
	//FIXME: must not use model class
	if (_cUserProfile->getUserProfile().getActivePhoneLine()) {
		phoneLineCreatedEventHandler(_cUserProfile->getUserProfile(), *_cUserProfile->getUserProfile().getActivePhoneLine());
	}

	if (_cUserProfile->getUserProfile().getWsInfo()) {
		wsInfoCreatedEventHandler(_cUserProfile->getUserProfile(), *_cUserProfile->getUserProfile().getWsInfo());
	}
	////
	//setEnabled(true);

	Config & config = ConfigManager::getInstance().getCurrentConfig();
#if defined(OS_MACOSX)	
	// VOXOX -ASV- 10.09.2009: we don't hide the user profile anymore since we use the qtoolbar
	// that contains it to handle this show hide functionality
	setVisible(true);
#else
	if(config.getShowProfileBar()){
		setVisible(true);	
	}
#endif
}

void QtProfileBar::reset() {
	//setEnabled(false);
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if(config.getShowProfileBar()){
		setVisible(false);
	}
	
	_cUserProfile = 0;
	_qtImAccountMonitor = 0;
	_qtImProfileWidget->reset();
	
}

void QtProfileBar::wsInfoCreatedEventHandler(UserProfile & sender, WsInfo & wsInfo) {
	wsInfo.wsInfoWengosEvent += boost::bind(&QtProfileBar::wsInfoWengosEventHandler, this, _1, _2, _3, _4);
	//wsInfo.wsInfoVoiceMailEvent += boost::bind(&QtProfileBar::wsInfoVoiceMailEventHandler, this, _1, _2, _3, _4);
	wsInfo.wsInfoLandlineNumberEvent += boost::bind(&QtProfileBar::wsInfoLandlineNumberEventHandler, this, _1, _2, _3, _4);
	wsInfo.getWengosCount(true);
	//wsInfo.getUnreadVoiceMail(false);
	
	wsInfo.getLandlineNumber(true);
	wsInfo.execute();
}

void QtProfileBar::wsInfoWengosEventHandler(WsInfo & sender, int id, WsInfo::WsInfoStatus status, std::string wengos) {
	if (status == WsInfo::WsInfoStatusOk) {
		wsInfoWengosEvent(QString::fromStdString(wengos));
	}
}

void QtProfileBar::wsInfoVoiceMailEventHandler(WsInfo & sender, int id, WsInfo::WsInfoStatus status, int voicemail) {
	if (status == WsInfo::WsInfoStatusOk) {
		wsInfoVoiceMailEvent(voicemail);
	}
}

void QtProfileBar::wsInfoLandlineNumberEventHandler(WsInfo & sender, int id, WsInfo::WsInfoStatus status, std::string number) {
	if (status == WsInfo::WsInfoStatusOk) {
		wsInfoLandlineNumberEvent(QString::fromStdString(number));
	}
}

void QtProfileBar::phoneLineCreatedEventHandler(UserProfile & sender, IPhoneLine & phoneLine) {
	phoneLineCreatedEvent();
}

void QtProfileBar::phoneLineCreatedEventSlot() {
	if(_cUserProfile)
	{	//VOXOX CHANGE Rolando 03-20-09
		_qtImProfileWidget->setNickNameLabel(QString::fromStdString(_cUserProfile->getUserProfile().getSipAccount()->getDisplayName()));
	}
}

void QtProfileBar::wsInfoWengosEventSlot(const QString & wengos) {
	//VOXOX CHANGE Rolando 03-20-09
	_qtImProfileWidget->setVoxoxPointsLabel( QString(" $ %1").arg(wengos));
}

void QtProfileBar::wsInfoLandlineNumberEventSlot(const QString & number) {
	//VOXOX CHANGE Rolando 03-20-09

	if(_cUserProfile){//VOXOX - CJC - 2009.06.27  Set voxox number
			_cUserProfile->getUserProfile().setVoxOxPhone(number.toStdString());
	}

	_qtImProfileWidget->setSipStatusLabel( QString(" %1 |").arg(number));
	
}
