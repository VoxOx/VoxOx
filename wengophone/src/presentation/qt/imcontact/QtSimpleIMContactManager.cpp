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
#include "QtSimpleIMContactManager.h"

#include "ui_SimpleIMContactManager.h"

#include <presentation/qt/imaccount/QtIMAccountSettings.h>
#include <presentation/qt/QtWengoPhone.h>

#include <control/CWengoPhone.h>
#include <control/contactlist/CContactList.h>
#include <control/profile/CUserProfile.h>

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/account/SipAccount.h>
#include <model/contactlist/ContactProfile.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/IMAccount.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/MouseEventFilter.h>
#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

static const QString IMCONTACT_TEXT_SEPARATOR = ";";

QtSimpleIMContactManager::QtSimpleIMContactManager(ContactProfile & contactProfile,
	CUserProfile & cUserProfile, QWidget * parent)
	: QtIMContactManagerInterface(contactProfile, cUserProfile, parent) {

	_imContactManagerWidget = new QWidget(NULL);

	_ui = new Ui::SimpleIMContactManager();
	_ui->setupUi(_imContactManagerWidget);

	_msnLineEditMouseEventFilter = new MousePressEventFilter(this, SLOT(msnLineEditClicked()));
	_ui->msnLineEdit->installEventFilter(_msnLineEditMouseEventFilter);
	_aimLineEditMouseEventFilter = new MousePressEventFilter(this, SLOT(aimLineEditClicked()));
	_ui->aimLineEdit->installEventFilter(_aimLineEditMouseEventFilter);
	_icqLineEditMouseEventFilter = new MousePressEventFilter(this, SLOT(icqLineEditClicked()));
	_ui->icqLineEdit->installEventFilter(_icqLineEditMouseEventFilter);
	_yahooLineEditMouseEventFilter = new MousePressEventFilter(this, SLOT(yahooLineEditClicked()));
	_ui->yahooLineEdit->installEventFilter(_yahooLineEditMouseEventFilter);
	_jabberLineEditMouseEventFilter = new MousePressEventFilter(this, SLOT(jabberLineEditClicked()));
	_ui->jabberLineEdit->installEventFilter(_jabberLineEditMouseEventFilter);

	SAFE_CONNECT(_ui->msnLineEdit, SIGNAL(textChanged(const QString &)), SIGNAL(contentHasChanged()));
	SAFE_CONNECT(_ui->aimLineEdit, SIGNAL(textChanged(const QString &)), SIGNAL(contentHasChanged()));
	SAFE_CONNECT(_ui->icqLineEdit, SIGNAL(textChanged(const QString &)), SIGNAL(contentHasChanged()));
	SAFE_CONNECT(_ui->yahooLineEdit, SIGNAL(textChanged(const QString &)), SIGNAL(contentHasChanged()));
	SAFE_CONNECT(_ui->jabberLineEdit, SIGNAL(textChanged(const QString &)), SIGNAL(contentHasChanged()));
	SAFE_CONNECT(_ui->wengoLineEdit, SIGNAL(textChanged(const QString &)), SIGNAL(contentHasChanged()));
	SAFE_CONNECT(_ui->sipLineEdit, SIGNAL(textChanged(const QString &)), SIGNAL(contentHasChanged()));

//VOXOX CHANGE for VoxOx by Rolando 01-09-09, we don't need it
//#ifndef CUSTOM_ACCOUNT
//	_ui->wengoLabel->hide();
//	_ui->label_2->hide();
//	_ui->wengoLineEdit->hide();
//#endif

	loadIMContacts();
}

QtSimpleIMContactManager::~QtSimpleIMContactManager() {
	OWSAFE_DELETE(_msnLineEditMouseEventFilter);
	OWSAFE_DELETE(_aimLineEditMouseEventFilter);
	OWSAFE_DELETE(_icqLineEditMouseEventFilter);
	OWSAFE_DELETE(_yahooLineEditMouseEventFilter);
	OWSAFE_DELETE(_jabberLineEditMouseEventFilter);
	OWSAFE_DELETE(_ui);
}

std::set<IMContact *> QtSimpleIMContactManager::findIMContactsOfProtocol(EnumIMProtocol::IMProtocol imProtocol) const {
	std::set<IMContact *> result;

	const IMContactSet & imContactSet = _contactProfile.getIMContactSet();
	for (IMContactSet::const_iterator it = imContactSet.begin(); it != imContactSet.end(); it++) {
		if ((*it).getProtocol() == imProtocol) {		//VOXOX - JRT - 2009.07.02 - OK?
			result.insert((IMContact *)(&(*it)));
		}
	}

	return result;
}

QString QtSimpleIMContactManager::getIMContactsOfProtocol(EnumIMProtocol::IMProtocol imProtocol) const {
	QString imContacts;

	std::set<IMContact *> imContactSet = findIMContactsOfProtocol(imProtocol);
	for (std::set<IMContact *>::const_iterator it = imContactSet.begin(); it != imContactSet.end(); it++) {
		IMContact * imContact = *it;
		imContacts += QString::fromStdString(imContact->getDisplayContactId());
		imContacts += IMCONTACT_TEXT_SEPARATOR;
	}

	//Remove the last IMCONTACT_TEXT_SEPARATOR
	int index = imContacts.lastIndexOf(IMCONTACT_TEXT_SEPARATOR);
	imContacts.remove(index, 1);

	return imContacts;
}

void QtSimpleIMContactManager::changeIMProtocolPixmaps(EnumIMProtocol::IMProtocol imProtocol,
	QLabel * imProtocolLabel, const char * connectedPixmap, QLineEdit * imProtocolLineEdit) {

	IMAccountList imAccounts = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccountsOfProtocol(imProtocol);
	if (!imAccounts.empty()) {
		imProtocolLineEdit->setEnabled(true);
		imProtocolLabel->setPixmap(QPixmap(connectedPixmap));
	}
}

void QtSimpleIMContactManager::loadIMContacts() {
	//Update presentation depending if IM accounts exist
	changeIMProtocolPixmaps(EnumIMProtocol::IMProtocolMSN,
		_ui->msnLabel, ":pics/protocols/msn.png", _ui->msnLineEdit);
	changeIMProtocolPixmaps(EnumIMProtocol::IMProtocolAIM,
		_ui->aimLabel, ":pics/protocols/aim.png", _ui->aimLineEdit);
	changeIMProtocolPixmaps(EnumIMProtocol::IMProtocolICQ,
		_ui->icqLabel, ":pics/protocols/icq.png", _ui->icqLineEdit);
	changeIMProtocolPixmaps(EnumIMProtocol::IMProtocolYahoo,
		_ui->yahooLabel, ":pics/protocols/yahoo.png", _ui->yahooLineEdit);
	changeIMProtocolPixmaps(EnumIMProtocol::IMProtocolJabber,
		_ui->jabberLabel, ":pics/protocols/jabber.png", _ui->jabberLineEdit);

	//Loads all the IM contacts
	_ui->wengoLineEdit->setText(getIMContactsOfProtocol(EnumIMProtocol::IMProtocolWengo));
	if (_ui->msnLineEdit->isEnabled()) {
		_ui->msnLineEdit->setText(getIMContactsOfProtocol(EnumIMProtocol::IMProtocolMSN));
	}
	if (_ui->aimLineEdit->isEnabled()) {
		_ui->aimLineEdit->setText(getIMContactsOfProtocol(EnumIMProtocol::IMProtocolAIM));
	}
	if (_ui->icqLineEdit->isEnabled()) {
		_ui->icqLineEdit->setText(getIMContactsOfProtocol(EnumIMProtocol::IMProtocolICQ));
	}
	if (_ui->yahooLineEdit->isEnabled()) {
		_ui->yahooLineEdit->setText(getIMContactsOfProtocol(EnumIMProtocol::IMProtocolYahoo));
	}
	if (_ui->jabberLineEdit->isEnabled()) {
		_ui->jabberLineEdit->setText(getIMContactsOfProtocol(EnumIMProtocol::IMProtocolJabber));
	}

	_ui->sipLineEdit->setText(getIMContactsOfProtocol(EnumIMProtocol::IMProtocolSIP));
}

void QtSimpleIMContactManager::addIMContactsOfProtocol(EnumIMProtocol::IMProtocol imProtocol, const QString & text) {
	QString imContactsBefore = getIMContactsOfProtocol(imProtocol);
	QStringList contactIdListBefore = imContactsBefore.split(IMCONTACT_TEXT_SEPARATOR);
	QString imContactsAfter = text;
	QStringList contactIdListAfter = imContactsAfter.split(IMCONTACT_TEXT_SEPARATOR);
	for (int i = 0; i < contactIdListAfter.size(); i++) {
		contactIdListAfter[i] = contactIdListAfter[i].trimmed();
	}

	//Remove IMContacts
	for (int i = 0; i < contactIdListBefore.size(); i++) {
		QString contactId = contactIdListBefore[i];

		if (!contactIdListAfter.contains(contactId)) {
			//Remove the imContact from the contactProfile
			IMContact imContact(imProtocol, contactId.toStdString());
			_contactProfile.removeIMContact(imContact);
		}
	}

	//Check if id is not empty
	if (text.isEmpty()) {
		return;
	}

	//Add IMContacts
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString wengoSuffix = "@" + QString::fromStdString( config.getWengoRealm() );
	for (int i = 0; i < contactIdListAfter.size(); i++) {
		QString contactId = contactIdListAfter[i];

		if (!contactIdListBefore.contains(contactId)) {
			
			// sip contact should know their domain
			if (imProtocol == EnumIMProtocol::IMProtocolWengo) {
				contactId += wengoSuffix;
			} else if (imProtocol == EnumIMProtocol::IMProtocolSIP) {
				if (!contactId.contains("@")) {
					SipAccount * sipaccount = _cUserProfile.getUserProfile().getSipAccount();
					if (sipaccount) {
						contactId += QString("@") + QString::fromStdString(sipaccount->getRealm());
					}
				}
			}
			////
			
			//Add the imContact to the contactProfile
			IMContact imContact(imProtocol, contactId.toStdString());

			//IMAccount to associate with the IMContact
			IMAccountList imAccounts = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccountsOfProtocol(imProtocol);
			IMAccountList::const_iterator it = imAccounts.begin();
			if (it != imAccounts.end()) 
			{
//				IMAccount * imAccount =	_cUserProfile.getUserProfile().getIMAccountManager().getIMAccount((*it).getUUID());	//VOXOX - JRT - 2009.04.09 
//				IMAccount * imAccount =	_cUserProfile.getUserProfile().getIMAccountManager().getIMAccount( it->second.getUUID());
				IMAccount * imAccount =	_cUserProfile.getUserProfile().getIMAccountManager().getIMAccount( (*it).second );	//VOXOX - JRT - 2009.04.24

				imContact.setIMAccount(imAccount);
				OWSAFE_DELETE(imAccount);
			}

			_contactProfile.addIMContact(imContact);
		}
	}
}

void QtSimpleIMContactManager::saveIMContacts() {
	addIMContactsOfProtocol(EnumIMProtocol::IMProtocolWengo, _ui->wengoLineEdit->text());
	if (_ui->msnLineEdit->isEnabled()) {
		addIMContactsOfProtocol(EnumIMProtocol::IMProtocolMSN, _ui->msnLineEdit->text());
	}
	if (_ui->aimLineEdit->isEnabled()) {
		addIMContactsOfProtocol(EnumIMProtocol::IMProtocolAIM, _ui->aimLineEdit->text());
	}
	if (_ui->icqLineEdit->isEnabled()) {
		addIMContactsOfProtocol(EnumIMProtocol::IMProtocolICQ, _ui->icqLineEdit->text());
	}
	if (_ui->yahooLineEdit->isEnabled()) {
		addIMContactsOfProtocol(EnumIMProtocol::IMProtocolYahoo, _ui->yahooLineEdit->text());
	}
	if (_ui->jabberLineEdit->isEnabled()) {
		addIMContactsOfProtocol(EnumIMProtocol::IMProtocolJabber, _ui->jabberLineEdit->text());
	}

	addIMContactsOfProtocol(EnumIMProtocol::IMProtocolSIP, _ui->sipLineEdit->text());
}

void QtSimpleIMContactManager::msnLineEditClicked() {
	if (!_ui->msnLineEdit->isEnabled()) {
		QtIMAccountSettings(_cUserProfile.getUserProfile(),
			QtEnumIMProtocol::IMProtocolMSN, _imContactManagerWidget);
		loadIMContacts();
	}
}

void QtSimpleIMContactManager::aimLineEditClicked() {
	if (!_ui->aimLineEdit->isEnabled()) {
		QtIMAccountSettings(_cUserProfile.getUserProfile(),
			QtEnumIMProtocol::IMProtocolAIM, _imContactManagerWidget);
		loadIMContacts();
	}
}

void QtSimpleIMContactManager::icqLineEditClicked() {
	if (!_ui->icqLineEdit->isEnabled()) {
		QtIMAccountSettings(_cUserProfile.getUserProfile(),
			QtEnumIMProtocol::IMProtocolICQ, _imContactManagerWidget);
		loadIMContacts();
	}
}

void QtSimpleIMContactManager::yahooLineEditClicked() {
	if (!_ui->yahooLineEdit->isEnabled()) {
		QtIMAccountSettings(_cUserProfile.getUserProfile(),
			QtEnumIMProtocol::IMProtocolYahoo, _imContactManagerWidget);
		loadIMContacts();
	}
}

void QtSimpleIMContactManager::jabberLineEditClicked() {
	if (!_ui->jabberLineEdit->isEnabled()) {
		QtIMAccountSettings(_cUserProfile.getUserProfile(),
			QtEnumIMProtocol::IMProtocolJabber, _imContactManagerWidget);
		loadIMContacts();
	}
}

void QtSimpleIMContactManager::setWengoName(const QString & wengoName) {
	_ui->wengoLineEdit->setText(wengoName);
	_ui->wengoLineEdit->setEnabled(true);
}

void QtSimpleIMContactManager::setSipAddress(const QString & sipAddress) {
	_ui->sipLineEdit->setText(sipAddress);
	_ui->sipLineEdit->setEnabled(true);
}

bool QtSimpleIMContactManager::couldBeSaved() {
	return ( (/*(_ui->wengoLineEdit->isEnabled()) &&*/ (!_ui->wengoLineEdit->text().isEmpty())) ||
			 ((_ui->msnLineEdit->isEnabled()) && (!_ui->msnLineEdit->text().isEmpty())) ||
			 (( _ui->aimLineEdit->isEnabled()) && (! _ui->aimLineEdit->text().isEmpty())) ||
			 (( _ui->icqLineEdit->isEnabled()) && (! _ui->icqLineEdit->text().isEmpty())) ||
			 (( _ui->yahooLineEdit->isEnabled()) && (! _ui->yahooLineEdit->text().isEmpty())) ||
			 (( _ui->jabberLineEdit->isEnabled()) && (! _ui->jabberLineEdit->text().isEmpty())) ||
			 (/*( _ui->sipLineEdit->isEnabled()) &&*/ (!_ui->sipLineEdit->text().isEmpty()))
		   );
}
