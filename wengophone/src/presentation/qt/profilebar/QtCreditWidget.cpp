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
#include "QtCreditWidget.h"

#include "ui_CreditWidget.h"

#include <presentation/qt/QtNoWengoAlert.h>
#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/config/QtWengoConfigDialog.h>

#include <model/account/SipAccount.h>
#include <model/profile/UserProfile.h>
#include <model/phoneline/IPhoneLine.h>
#include <model/webservices/url/WsUrl.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>



#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/SafeConnect.h>

#include <util/SafeDelete.h>

#include <QtGui/QtGui>

QtCreditWidget::QtCreditWidget(QWidget * parent)
	: QObject(parent)
	, _cWengoPhone(0)
	, _userProfile(0) {

	_creditWidget = new QWidget(parent);

	_ui = new Ui::CreditWidget();
	_ui->setupUi(_creditWidget);

	SAFE_CONNECT(_ui->callForwardButton, SIGNAL(clicked()), SLOT(callForwardModeClicked()));
	SAFE_CONNECT(_ui->buyCreditsButton, SIGNAL(clicked()), SLOT(buyCreditsClicked()));

	//voiceMailButton
	SAFE_CONNECT(_ui->voiceMailButton, SIGNAL(clicked()), SLOT(voiceMailClicked()));

	LANGUAGE_CHANGE(_creditWidget);
}

QtCreditWidget::~QtCreditWidget() {
	OWSAFE_DELETE(_ui);
}

void QtCreditWidget::init(CWengoPhone* cWengoPhone, UserProfile* userProfile) {
	_cWengoPhone = cWengoPhone;
	_userProfile = userProfile;
	bool isWengo = _userProfile->hasWengoAccount();
	_ui->callForwardLabel->setVisible(isWengo);
	_ui->callForwardButton->setVisible(isWengo);
	_ui->buyCreditsButton->setVisible(isWengo);
}

QWidget * QtCreditWidget::getWidget() const {
	return _creditWidget;
}

void QtCreditWidget::updatePresentation() {
	if (!_callForwardMode.isEmpty()) {
		_ui->callForwardLabel->setText(_callForwardMode);
	}
}

void QtCreditWidget::setCallForwardMode(const QString & callForwardMode) {
	_callForwardMode = callForwardMode;
	updatePresentation();
}

void QtCreditWidget::buyCreditsClicked() {
	Q_ASSERT(_userProfile->hasWengoAccount());
	WsUrl::showWengoBuyWengos();
}

void QtCreditWidget::callForwardModeClicked() {
	Q_ASSERT(_userProfile->hasWengoAccount());
	//VOXOX - SEMR - 2009.05.13 MEMORY ISSUE: Config/Settings Window
	//QtWengoConfigDialog* dialog = QtWengoConfigDialog::showInstance(*_cWengoPhone);
	//dialog->showCallForwardPage();
}

void QtCreditWidget::languageChanged() {
	_ui->retranslateUi(_creditWidget);
	updatePresentation();
}

void QtCreditWidget::voiceMailClicked() {
	IPhoneLine * phoneLine = _userProfile->getActivePhoneLine();
	if (phoneLine) {
		phoneLine->makeCall(_userProfile->getSipAccount()->getVoicemailNumber());
	}
}
