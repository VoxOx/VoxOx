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
#include "QtPhoneComboBox.h"
#include <presentation/qt/QtWengoPhone.h>

#include <control/CWengoPhone.h>
#include <control/history/CHistory.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/history/History.h>
#include <model/phonecall/SipAddress.h>
#include <model/profile/UserProfile.h>
#include <model/contactlist/ContactList.h>
#include <model/contactlist/Contact.h>
#include <model/contactlist/ContactProfile.h>

#include <util/SafeDelete.h>
#include <util/Logger.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

QtPhoneComboBox::QtPhoneComboBox(QWidget * parent)
	: UpQComboBox(parent) {

	setLineEdit(new QtPhoneComboBoxLineEdit(this));

	SAFE_CONNECT(this, SIGNAL(activated(const QString &)), SLOT(comboBoxActivated()));
	SAFE_CONNECT(this, SIGNAL(popupDisplayed()), SLOT(popupDisplayedSlot()));
	SAFE_CONNECT(this, SIGNAL(popupHidden()), SLOT(popupHiddenSlot()));
	SAFE_CONNECT(this, SIGNAL(highlighted(const QString &)), SLOT(itemhighlighted(const QString &)));
		
	init();
}

void QtPhoneComboBox::init() {
	_cleared = false;
	_toolTipTextDone = false;
	_originalPalette = palette();

	//Grey color
	_greyPalette.setColor(QPalette::Text, Qt::gray);

	setAutoCompletion(false);
}

QtPhoneComboBox::~QtPhoneComboBox() {
}

void QtPhoneComboBox::setEditText(const QString & text) {

	if (!_toolTipTextDone) {
		_toolTipTextDone = true;
		_toolTip = text;
		setToolTipText();
	} else {
		clearLineEdit();
		QComboBox::setEditText(text);
	}
}

QString QtPhoneComboBox::currentText() const {
	if (QComboBox::currentText() == _toolTip) {
		return QString::null;
	} else {
		return QComboBox::currentText();
	}
}

void QtPhoneComboBox::setToolTipText() {
	//Text color is grey
	lineEdit()->setPalette(_greyPalette);

	_cleared = false;
	QComboBox::setEditText(_toolTip);
}

void QtPhoneComboBox::clearLineEdit() {
	//Text color back to original color
	lineEdit()->setPalette(_originalPalette);

	clearEditText();
	_cleared = true;
}

void QtPhoneComboBox::insertPhoneNumber(QStringList * phoneList, std::string toBeAdded) {
	if (!toBeAdded.empty()) {
		QString phone(toBeAdded.c_str());
		(*phoneList) << phone;
	}
}

void QtPhoneComboBox::clearList() {
	//Text color back to original color
	lineEdit()->setPalette(_originalPalette);

	QStringList tobeinserted = QStringList();
	clear();
	_totalCompletionList.clear();
	_restrainedCompletionList.clear();

	//completion of contact
	CUserProfile* cuserprofile = _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
	if(cuserprofile) {
		UserProfile & userProfile = cuserprofile->getUserProfile();
//		for (	std::list<Contact>::const_iterator it = userProfile.getContactList().getContacts().begin(); 
		//VOXOX - JRT - 2009.04.05 - Because we now have (or soon will have) user-defined phone type,
		// this iterator cannot be constant (const).  This is because the phone list will automatically
		// create an empty entry for any non-existing type that is requested, as we do here.
		//Going forward, this control may have to change when we implement user-defined types.

		//VOXOX - JRT - 2009.04.07 - Use of an iterator here seems odd.  How many Contact's phones are we using?
//		for (	std::list<Contact>::iterator it = const_cast<ContactList::Contacts&>(userProfile.getContactList().getContacts()).begin(); 
//				it != userProfile.getContactList().getContacts().end(); 
//				++it) {
//			insertPhoneNumber(&tobeinserted, it->getFirstAvailableWengoIMContact().getDisplayContactId());
//			insertPhoneNumber(&tobeinserted, it->getFirstAvailableSIPNumber());
//			insertPhoneNumber(&tobeinserted, it->getMobilePhone());
//			insertPhoneNumber(&tobeinserted, it->getHomePhone());
//			insertPhoneNumber(&tobeinserted, it->getWorkPhone());
//			insertPhoneNumber(&tobeinserted, it->getOtherPhone());
//		}

		//VOXOX - JRT - 2009.04.07 - use new map-based Contacts class
		for (	Contacts::iterator it = const_cast<Contacts&>(userProfile.getContactList().getContacts()).begin(); 
				it != userProfile.getContactList().getContacts().end(); 
				++it) {
			Contact& rContact = (*it).second;
			insertPhoneNumber(&tobeinserted, rContact.getFirstAvailableWengoIMContact().getDisplayContactId());
			insertPhoneNumber(&tobeinserted, rContact.getFirstAvailableSIPNumber());
			insertPhoneNumber(&tobeinserted, rContact.getMobilePhone());
			insertPhoneNumber(&tobeinserted, rContact.getHomePhone());
			insertPhoneNumber(&tobeinserted, rContact.getWorkPhone());
			insertPhoneNumber(&tobeinserted, rContact.getOtherPhone());
		}
		//End Voxox
	}
	////

	if (tobeinserted.size() != 0) {
		tobeinserted.sort();

		//no duplicated entries
		_totalCompletionList << tobeinserted[0];
		for (int i = 1; i < tobeinserted.size(); ++i) {
			if( tobeinserted[i] != tobeinserted[i-1] ) {
				_totalCompletionList << tobeinserted[i];
			}
		}
	}
	_restrainedCompletionList.clear();

	clearEditText();
	_cleared = true;
}

void QtPhoneComboBox::mousePressEvent(QMouseEvent * event) {
	QComboBox::mousePressEvent(event);
}

void QtPhoneComboBox::keyPressEvent(QKeyEvent * event) {
	if (!_cleared) {
		clearList();
	}
	QComboBox::keyPressEvent(event);
}

void QtPhoneComboBox::focusOutEvent(QFocusEvent * event) {
	if (currentText().isEmpty()) {
		setToolTipText();
	}
	QComboBox::focusOutEvent(event);
}

void QtPhoneComboBox::comboBoxActivated() {
	//Text color back to original color
	lineEdit()->setPalette(_originalPalette);
}

void QtPhoneComboBox::popupDisplayedSlot() {
	QStringList tobeinserted = QStringList();
	clear();

	CUserProfile * cUserProfile =
		_qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
	if (!cUserProfile) {
		return;
	}

	//completion of history
	//if _qtHistoryWidget is set it means that History has been created
	
	bool isWengoAccountConnected = cUserProfile->getUserProfile().hasWengoAccount();
	
	CHistory* chistory = cUserProfile->getCHistory();
	if (chistory) {
		HistoryMementoCollection * mementos = chistory->getMementos(HistoryMemento::OutgoingCall, 10);
		Config & config = ConfigManager::getInstance().getCurrentConfig();
		QString wengoSuffix = "@" + QString::fromStdString( config.getWengoRealm() );
		for (HistoryMap::iterator it = mementos->begin(); it != mementos->end(); it++) {
			HistoryMemento * memento = (*it).second;
			SipAddress sipAddress(memento->getPeer());
			
			QString username(sipAddress.getSipAddress().c_str());
			if (isWengoAccountConnected) {
				username.remove(wengoSuffix);
			}
			username.remove("sip:");	

			if( !username.isEmpty() && (memento->getState()==HistoryMemento::OutgoingCall) && !tobeinserted.contains(username) ) {
				tobeinserted << username;
			}
		}
		OWSAFE_DELETE(mementos);
	}
	////

	tobeinserted.sort();
	insertItems(0, tobeinserted);
	clearLineEdit();
}

void QtPhoneComboBox::popupHiddenSlot() {
	insertItem(0,_lastHighlighted);
}

void QtPhoneComboBox::itemhighlighted(const QString & text) {
	_lastHighlighted = text;
}

void QtPhoneComboBox::languageChanged() {
	init();
}

void QtPhoneComboBox::setQtWengoPhone(QtWengoPhone* newQtWengoPhone) {
	_qtWengoPhone = newQtWengoPhone;
}

void QtPhoneComboBox::setTextWithCompletion(const QString & newText) {
	
	if (newText.isEmpty()) {
		return;
	}

	// compute new restrained list
	_restrainedCompletionList.clear();
    QStringList::const_iterator i;
    for (i = _totalCompletionList.constBegin(); i != _totalCompletionList.constEnd(); ++i) {
		if (i->startsWith(newText)) {
			break;
		} 
    }
    for (; i != _totalCompletionList.constEnd(); ++i) {
		if (i->startsWith(newText)) {
			_restrainedCompletionList << (*i);
		} else {
			break;
		}
    }
	////
	
	if (_restrainedCompletionList.isEmpty()) {
		return;
	}
	
	// use the restrained list
	clear();
	addItems(_restrainedCompletionList);
	////
	
	// selected added text
	lineEdit()->setSelection(newText.length(), _restrainedCompletionList[0].length());
	////
}

//////////////////////////////////////////////////////////////////////////////////
//////////              QtPhoneComboBoxLineEdit                 //////////////////
//////////////////////////////////////////////////////////////////////////////////

QtPhoneComboBoxLineEdit::QtPhoneComboBoxLineEdit(QtPhoneComboBox * qtPhoneComboBox) {
	_qtPhoneComboBox = qtPhoneComboBox;
	
	SAFE_CONNECT(this, SIGNAL(textEdited(QString)), SLOT(textEditedSlot(QString)));
}

void QtPhoneComboBoxLineEdit::mousePressEvent(QMouseEvent * event) {
	if (!_qtPhoneComboBox->_cleared) {
		_qtPhoneComboBox->clearList();
	}
	QLineEdit::mousePressEvent(event);
}

void QtPhoneComboBoxLineEdit::keyPressEvent(QKeyEvent * event) {
	if (!_qtPhoneComboBox->_cleared) {
		_qtPhoneComboBox->clearList();
	}
	QLineEdit::keyPressEvent(event);
}


void QtPhoneComboBoxLineEdit::textEditedSlot(const QString & newText) {
	if (newText != _enteredText) {
		_enteredText = newText;
		_qtPhoneComboBox->setTextWithCompletion(_enteredText);
	}
}

