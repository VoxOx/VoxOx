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

#ifndef OWQTSMS_H
#define OWQTSMS_H

#include <presentation/PSms.h>

#include <control/webservices/sms/CSms.h>

#include <util/Trackable.h>

#include <qtutil/QObjectThreadSafe.h>

#include <QtCore/QString>

class QtWengoPhone;

class QWidget;
class QPushButton;
class QTextEdit;
class QDialog;
class QMenu;
class QAction;
namespace Ui { class SmsWindow; }

static const QString SIGNATURE_SEPARATOR = " -- ";
static const int SIGNATURE_LENGTH = 4;
static const int MAX_LENGTH = 160;

/**
 * Qt Presentation component for SMS.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class QtSms : public QObjectThreadSafe, public PSms, public Trackable {
	Q_OBJECT
public:

	/**
	 * Constructor.
	 */
	QtSms(CSms & cSms);

	/**
	 * Destructor.
	 */
	virtual ~QtSms();

	/**
	 * Return a pointer to the widget.
	 * 
	 * @return the sms widget.
	 */
	QWidget * getWidget() const { return (QWidget*)_smsWindow; }

	/**
	 * @see Presentation::updatePresentation.
	 */
	void updatePresentation() {}

	/**
	 * Set the phone number(s).
	 * 
	 * @param phoneNumber the phone number.
	 */
	void setPhoneNumber(const QString & phoneNumber);

	/**
	 * Add a phone number.
	 * 
	 * @param phoneNumber the phone number to add.
	 */
	void addPhoneNumber(const QString & phoneNumber);

	/**
	 * Set the message text.
	 * 
	 * @param text the text.
	 */
	void setText(const QString & text);

	/**
	 * Set the signature.
	 * 
	 * @param signature the signature.
	 */
	void setSignature(const QString & signature);

	/**
	 * Clear signature, phone box and textedit.
	 */
	void clear();

private Q_SLOTS:

	/**
	 * Update the counter label.
	 */
	void updateCounter();

	/**
	 * The user has clicked the send button.
	 */
	void sendButtonClicked();

	/**
	 * The user has clicked the add contact button.
	 */
	void addContactToolButtonClicked();

	/**
	 * The user has clicked on a menu entry.
	 */
	void updatePhoneNumberLineEdit(QAction * action);

	/**
	 * Call when the dialog is closed.
	 */
	void CloseEventFilterSlot() { clear(); }

private:

	/**
	 * Send sms(s) via control layer.
	 */
	void sendSms();

	/**
	 * Split the sms into several sms if needed.
	 * TODO: put this in the model layer
	 * 
	 * @return the sms list.
	 */
	QStringList splitMessage() const;

	/**
	 * Return the number of sms(s) needed to send the message.
	 * TODO: put this in the model layer
	 * 
	 * @return the number of sms(s) needed.
	 */
	int getNeededMessages() const;

	/**
	 * Return the length of the sms.
	 *
	 * @return the length of the sms.
	 */
	int getMessageLength() const;

	/**
	 * Gets the complete sms: text + signature.
	 *
	 * @return the complete message.
	 */
	QString getCompleteMessage() const;

	/**
	 * Check if the message could be send via one sms.
	 *
	 * @return the complete message.
	 */
	bool isSmsLengthOk() const { return (getCompleteMessage().length() <= MAX_LENGTH); }

	/**
	 * Init thread safe.
	 */
	void initThreadSafe();

	/**
	 * UpdatePresentation thread safe.
	 */
	void updatePresentationThreadSafe() {}

	/**
	 * Sms status event handler.
	 */
	void smsStatusEventHandler(WsSms & sender, int smsId, EnumSmsState::SmsState state);

	/**
	 * Sms status event handler thread safe.
	 */
	void smsStatusEventHandlerThreadSafe(EnumSmsState::SmsState state);

	/**
	 * Memorize in the current UserProfile.
	 */
	void memorizeSignature();

	/**
	 * Load the signature from the current UserProfile.
	 */
	void loadSignature();

	CSms & _cSms;

	QtWengoPhone * _qtWengoPhone;

	QMenu * _mobilePhoneMenu;

	Ui::SmsWindow * _ui;

	QDialog * _smsWindow;
};

#endif	//OWQTSMS_H
