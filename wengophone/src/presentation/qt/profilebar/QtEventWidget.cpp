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
#include "QtEventWidget.h"

#include "ui_EventWidget.h"

#include <presentation/PWengoPhone.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>

#include <model/profile/UserProfile.h>
#include <model/phoneline/IPhoneLine.h>

#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/SafeConnect.h>

#include <util/SafeDelete.h>

#include <QtGui/QtGui>
//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>
static const char * VOICE_MAIL_CALL = "123";

QtEventWidget::QtEventWidget(CWengoPhone & cWengoPhone, CUserProfile & cUserProfile, QWidget * parent)
	: QObjectThreadSafe(NULL),
	_cUserProfile(cUserProfile),
	_cWengoPhone(cWengoPhone) {

	_voiceMailCount = 0;
	_missedCallCount = 0;

	typedef PostEvent0<void ()> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtEventWidget::initThreadSafe, this));
	postEvent(event);
}

QtEventWidget::~QtEventWidget() {
	OWSAFE_DELETE(_ui);
}

void QtEventWidget::initThreadSafe() {
	_eventWidget = new QWidget();

	_ui = new Ui::EventWidget();
	_ui->setupUi(_eventWidget);

	//missedCallButton
	SAFE_CONNECT(_ui->missedCallButton, SIGNAL(clicked()), SLOT(missedCallClicked()));

	//voiceMailButton
	SAFE_CONNECT(_ui->voiceMailButton, SIGNAL(clicked()), SLOT(voiceMailClicked()));

	LANGUAGE_CHANGE(_eventWidget);

	updatePresentation();
}

QWidget * QtEventWidget::getWidget() const {
	return _eventWidget;
}

void QtEventWidget::updatePresentation() {
	typedef PostEvent0<void ()> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtEventWidget::updatePresentationThreadSafe, this));
	postEvent(event);
}

void QtEventWidget::updatePresentationThreadSafe() {
	_ui->voiceMailCountLabel->setText(QString("%1").arg(_voiceMailCount));
	_ui->missedCallCountLabel->setText(QString("%1").arg(_missedCallCount));
}

void QtEventWidget::setVoiceMail(int count) {
	_voiceMailCount = count;
	updatePresentation();
}

void QtEventWidget::setMissedCall(int count) {
	_missedCallCount = count;
	updatePresentation();
}

void QtEventWidget::voiceMailClicked() {

	QtVoxMessageBox box(_eventWidget);
	box.setWindowTitle("VoxOx - - Voice Mail");
	box.setText(tr("Do you want to call your voice mail?"));
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No );
	
	if (box.exec() == QMessageBox::Yes) {
		IPhoneLine * phoneLine = _cUserProfile.getUserProfile().getActivePhoneLine();
		if (phoneLine) {
			phoneLine->makeCall(VOICE_MAIL_CALL);
		}
	}
}

void QtEventWidget::missedCallClicked() {
	//Shows the history tab
	PWengoPhone * pWengoPhone = (PWengoPhone *) _cWengoPhone.getPresentation();
	pWengoPhone->showHistory();
}

void QtEventWidget::languageChanged() {
	_ui->retranslateUi(_eventWidget);
	updatePresentationThreadSafe();
}
