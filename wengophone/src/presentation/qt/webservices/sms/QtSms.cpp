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
#include "QtSms.h"

#include "ui_SmsWindow.h"

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/contactlist/QtContactMenu.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/profile/UserProfile.h>

#include <cutil/global.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/CloseEventFilter.h>

#include <QtGui/QtGui>

QtSms::QtSms(CSms & cSms)
	: QObjectThreadSafe(NULL),
	_cSms(cSms) {

	_cSms.smsStatusEvent += boost::bind(&QtSms::smsStatusEventHandler, this, _1, _2, _3);
	_qtWengoPhone = (QtWengoPhone *) _cSms.getCWengoPhone().getPresentation();

	typedef PostEvent0<void ()> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtSms::initThreadSafe, this));
	postEvent(event);
}

QtSms::~QtSms() {
	_qtWengoPhone->setQtSms(NULL);
	OWSAFE_DELETE(_ui);
}

void QtSms::initThreadSafe() {

	_smsWindow = new QDialog(_qtWengoPhone->getWidget());
	_ui = new Ui::SmsWindow();
	_ui->setupUi(_smsWindow);

	_mobilePhoneMenu = new QMenu(_smsWindow);

	SAFE_CONNECT(_ui->sendButton, SIGNAL(clicked()), SLOT(sendButtonClicked()));
	SAFE_CONNECT(_ui->smsText, SIGNAL(textChanged()), SLOT(updateCounter()));
	SAFE_CONNECT(_ui->signatureLineEdit, SIGNAL(textChanged(const QString &)), SLOT(updateCounter()));

	SAFE_CONNECT( _ui->addContactToolButton, SIGNAL(clicked()), SLOT(addContactToolButtonClicked()));	
	SAFE_CONNECT(_mobilePhoneMenu, SIGNAL(triggered(QAction *)), SLOT(updatePhoneNumberLineEdit(QAction *)));

	CloseEventFilter * closeEventFilter = new CloseEventFilter(this, SLOT(CloseEventFilterSlot()));
	_smsWindow->installEventFilter(closeEventFilter);

	loadSignature();
	_qtWengoPhone->setQtSms(this);
}

void QtSms::clear() {

	setText(QString());
}

void QtSms::setPhoneNumber(const QString & phoneNumber) {

	_ui->phoneComboBox->clear();
	if (!phoneNumber.isEmpty()) {
		_ui->phoneComboBox->lineEdit()->setText(phoneNumber);
	}
}

void QtSms::addPhoneNumber(const QString & phoneNumber) {

	if (!phoneNumber.isEmpty()) {
		if (_ui->phoneComboBox->lineEdit()->text().isEmpty()) {
			_ui->phoneComboBox->lineEdit()->setText(phoneNumber);
		} else {
			_ui->phoneComboBox->lineEdit()->setText(_ui->phoneComboBox->lineEdit()->text() + "; " + phoneNumber);
		}
	}
}

void QtSms::setSignature(const QString & signature) {

	_ui->signatureLineEdit->setText(signature);
}

void QtSms::setText(const QString & text) {

	int pos = text.lastIndexOf(SIGNATURE_SEPARATOR);
	QString mess = text;
	if (pos != -1) {
		//extract the signature
		setSignature(text.right(text.length() - pos - SIGNATURE_LENGTH));
		mess = text.left(pos);
	}

	_ui->smsText->clear();

	QString fontSize = "9";
#if defined(OS_MACOSX)
	fontSize = "13";
#endif

	_ui->smsText->setHtml("<html><head><meta name=\"qrichtext\" content=\"1\" /></head>"
		"<body style=\" white-space: pre-wrap; font-family:Sans Serif; font-size:"
		+ fontSize
		+ "pt; font-weight:400; font-style:normal; text-decoration:none;\">"
		"<p style=\" margin-top:0px; margin-bottom:0px; "
		"margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
		+ mess
		+ "</p></body></html>");
	updateCounter();
}

void QtSms::updateCounter() {

	int textLength = _ui->smsText->toPlainText().length();
	int signatureLength = _ui->signatureLineEdit->text().length();

	QString counterText = QString::number(textLength + signatureLength) + "/";
	if (signatureLength) {
		counterText += QString::number(MAX_LENGTH - SIGNATURE_LENGTH);
	} else {
		counterText += QString::number(MAX_LENGTH);
	}
	_ui->counterLabel->setText(counterText);

	// update counterLabel color
	QPalette palette = _ui->counterLabel->palette();
	if (!isSmsLengthOk()) {
		palette.setColor(QPalette::WindowText, Qt::red);
	} else {
		palette.setColor(QPalette::WindowText, Qt::black);
	}
	_ui->counterLabel->setPalette(palette);
}

void QtSms::sendButtonClicked() {

	//check phoneNumber.
	if (_ui->phoneComboBox->currentText().isEmpty()) {

		QMessageBox::warning(_smsWindow,
			tr("@company@ SMS service"),
			tr("Please fill in a phone number.")
		);
		return;
	}

	//check smsText.
	if (_ui->smsText->toPlainText().isEmpty()) {

		QMessageBox::warning(_smsWindow,
			tr("@company@ SMS service"),
			tr("You can not send an empty message.")
		);
		return;
	}

	//Validate SMS length before sending
	if (!isSmsLengthOk()) {

		int neededMessages = getNeededMessages();

		int ret = QMessageBox::question(_smsWindow,
			tr("@company@ SMS service"),
			tr("Your message is too long.\n"
			"This will send ") + QString::number(neededMessages) + tr(" SMS.\n") + tr("Do you want to continue?"),
			QMessageBox::Yes,
			QMessageBox::No
		);

		if (ret == QMessageBox::No) {
			return;
		}
	}

	_ui->sendButton->setEnabled(false);
	memorizeSignature();
	sendSms();
}

void QtSms::smsStatusEventHandler(WsSms & sender, int smsId, EnumSmsState::SmsState state) {

	typedef PostEvent0<void ()> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtSms::smsStatusEventHandlerThreadSafe, this, state));
	postEvent(event);
}

void QtSms::smsStatusEventHandlerThreadSafe(EnumSmsState::SmsState state) {

	QString smsStatus = String::null;
	switch (state) {
	case EnumSmsState::SmsStateError:
		smsStatus = tr("Your SMS has not been sent");
		break;
	case EnumSmsState::SmsStateOk:
		smsStatus = tr("Your SMS has been sent");
		break;
	default:
		LOG_FATAL("unknown SmsStatus=" + String::fromNumber(state));
	}

	_ui->sendButton->setEnabled(true);
	QMessageBox::information(_smsWindow, tr("@company@ SMS service"), smsStatus);
}

QString QtSms::getCompleteMessage() const {

	QString completeMessage = _ui->smsText->toPlainText();
	QString signature = _ui->signatureLineEdit->text();
	if (!signature.isEmpty()) {
		completeMessage += SIGNATURE_SEPARATOR;
		completeMessage += _ui->signatureLineEdit->text();
	}

	return completeMessage;
}

int QtSms::getMessageLength() const {

	return getCompleteMessage().length();
}

void QtSms::sendSms() {

	// format Converts to UTF-8
	std::string numbers;
	QString temp = _ui->phoneComboBox->currentText();
	QStringList phoneNumbers = temp.split(QRegExp(",|;|&"));
	for (int i = 0; i < phoneNumbers.size(); i++) {
		QString number = phoneNumbers[i].remove(" ");
		if (i == 0) {
			numbers += std::string(number.toUtf8().constData());
		} else {
			numbers += ";" + std::string(number.toUtf8().constData());
		}
	}

	QStringList messages = splitMessage();
	for (int i = 0; i < messages.size(); i++) {

		if (!messages[i].isEmpty()) {
			std::string sms(messages[i].toUtf8().constData());
			_cSms.sendSMS(numbers, sms);
		}
	}
}

QStringList QtSms::splitMessage() const {

	QStringList toReturn;
	QString message = getCompleteMessage();
	int i;

	for (i = 0; i < getNeededMessages(); i++) {
		QString temp = message.mid(i * MAX_LENGTH, MAX_LENGTH);
		toReturn += temp;
	}

	return toReturn;
}

int QtSms::getNeededMessages() const {

	return ((getMessageLength() / MAX_LENGTH) + 1);
}

void QtSms::memorizeSignature() {

	CUserProfile * cUserProfile = _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
	QString signature = _ui->signatureLineEdit->text();

	if (cUserProfile && (!signature.isEmpty())) {

		UserProfile & userProfile = cUserProfile->getUserProfile();
		userProfile.setSmsSignature(signature.toStdString());
	}
}

void QtSms::loadSignature() {

	CUserProfile * cUserProfile = _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
	QString signature = _ui->signatureLineEdit->text();

	if (cUserProfile) {

		UserProfile & userProfile = cUserProfile->getUserProfile();
		std::string signature = userProfile.getSmsSignature();

		if (!signature.empty()) {

			_ui->signatureLineEdit->setText(QString::fromStdString(signature));
		}
	}
}

void QtSms::addContactToolButtonClicked() {
	_mobilePhoneMenu->clear();
	QtContactMenu::populateMobilePhoneMenu(_mobilePhoneMenu, _qtWengoPhone->getCWengoPhone());
	QPoint point = _ui->addContactToolButton->pos();
	point.setX(point.x() + _ui->addContactToolButton->rect().width());
	_mobilePhoneMenu->popup(_smsWindow->mapToGlobal(point));
}

void QtSms::updatePhoneNumberLineEdit(QAction * action) {

	if (action) {
		QString data = action->data().toString();
		addPhoneNumber(data);
	}
}
