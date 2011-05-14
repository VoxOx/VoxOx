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
#include "QtAddIMContact.h"

#include "ui_AddIMContact.h"

#include <control/contactlist/CContactList.h>
#include <control/profile/CUserProfile.h>

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/account/SipAccount.h>
#include <model/contactlist/ContactProfile.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/IMAccount.h>

#include <util/SafeDelete.h>

#include <QtGui/QtGui>

QtAddIMContact::QtAddIMContact(ContactProfile & contactProfile,
	CUserProfile & cUserProfile, QWidget * parent)
	: QObject(parent),
	_contactProfile(contactProfile),
	_cUserProfile(cUserProfile) {

	_addIMContactWindow = new QDialog(parent);

	_ui = new Ui::AddIMContact();
	_ui->setupUi(_addIMContactWindow);

#ifdef CUSTOM_ACCOUNT
	_ui->protocolComboBox->addItem(QIcon(":pics/protocols/wengo.png"),
		QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolWengo)));
#endif

	_ui->protocolComboBox->addItem(QIcon(":pics/protocols/msn.png"),
		QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolMSN)));
	_ui->protocolComboBox->addItem(QIcon(":pics/protocols/aim.png"),
		QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolAIM)));
	_ui->protocolComboBox->addItem(QIcon(":pics/protocols/icq.png"),
		QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolICQ)));
	_ui->protocolComboBox->addItem(QIcon(":pics/protocols/yahoo.png"),
		QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolYahoo)));
	_ui->protocolComboBox->addItem(QIcon(":pics/protocols/jabber.png"),
		QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolJabber)));
	_ui->protocolComboBox->addItem(QIcon(":pics/protocols/sip.png"),
		QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolSIP)));
	connect(_ui->protocolComboBox, SIGNAL(currentIndexChanged(const QString &)),
		SLOT(imProtocolChanged(const QString &)));

	connect(_ui->addIMContactButton, SIGNAL(clicked()), SLOT(addIMContact()));

	show();
}

QtAddIMContact::~QtAddIMContact() {
	delete _ui;
}

int QtAddIMContact::show() {
	imProtocolChanged(_ui->protocolComboBox->currentText());
	return _addIMContactWindow->exec();
}

void QtAddIMContact::addIMContact() {
	QString contactId = _ui->contactIdLineEdit->text().trimmed();
	
	if (contactId.isEmpty()) {
		return;
	}
	
	QString protocolName = _ui->protocolComboBox->currentText();

	EnumIMProtocol::IMProtocol imProtocol = EnumIMProtocol::toIMProtocol(protocolName.toStdString());
	
	// sip contact should know their domain
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString wengoSuffix = "@" + QString::fromStdString( config.getWengoRealm() );
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
	
	IMContact imContact(imProtocol, contactId.toStdString());

	IMAccountList imAccounts = getSelectedIMAccounts(imProtocol);

	if (imAccounts.empty()) {
		_contactProfile.addIMContact(imContact);
	}

	for (IMAccountList::const_iterator it = imAccounts.begin();
		it != imAccounts.end(); ++it) {

		IMAccount * imAccount =
//			_cUserProfile.getUserProfile().getIMAccountManager().getIMAccount((*it).getUUID());	//VOXOX - JRT - 2009.04.09 
//			_cUserProfile.getUserProfile().getIMAccountManager().getIMAccount( it->second.getUUID());
			_cUserProfile.getUserProfile().getIMAccountManager().getIMAccount( it->second );	//VOXOX - JRT - 2009.04.24
		imContact.setIMAccount(imAccount);
		_contactProfile.addIMContact(imContact);
		OWSAFE_DELETE(imAccount);
	}
}

void QtAddIMContact::imProtocolChanged(const QString & protocolName) {
	loadIMAccounts(EnumIMProtocol::toIMProtocol(protocolName.toStdString()));
}

void QtAddIMContact::loadIMAccounts(EnumIMProtocol::IMProtocol imProtocol) {
	_ui->treeWidget->clear();

	IMAccountList imAccounts = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccountsOfProtocol(imProtocol);

	for (IMAccountList::const_iterator it = imAccounts.begin();
		it != imAccounts.end(); ++it) {

		QTreeWidgetItem * item = new QTreeWidgetItem(_ui->treeWidget);

		//By default, check the first element only
		if (it == imAccounts.begin()) {
			item->setCheckState(0, Qt::Checked);
		} else {
			item->setCheckState(0, Qt::Unchecked);
		}

//		item->setText(1, QString::fromStdString((*it).getLogin()));	//VOXOX - JRT - 2009.04.09 
		item->setText(1, QString::fromStdString( it->second.getLogin()));
	}
}

IMAccountList QtAddIMContact::getSelectedIMAccounts(EnumIMProtocol::IMProtocol imProtocol) const {
	IMAccountList result;

	IMAccountList imAccounts = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccountsOfProtocol(imProtocol);

	for (IMAccountList::const_iterator it = imAccounts.begin();
		it != imAccounts.end(); ++it) {

//		QList<QTreeWidgetItem *> list = _ui->treeWidget->findItems(QString::fromStdString((*it).getLogin()), Qt::MatchExactly, 1);	//VOXOX - JRT - 2009.04.09 
		QList<QTreeWidgetItem *> list = _ui->treeWidget->findItems(QString::fromStdString( it->second.getLogin()), Qt::MatchExactly, 1);

		if (list.size() > 0) {
			//There should be only one item
			QTreeWidgetItem * item = list[0];
			if (item->checkState(0) == Qt::Checked) {
//				result.push_back(*it);	//VOXOX - JRT - 2009.04.09 
				result.Add( const_cast<IMAccount&>(it->second) );
			}
		}
	}

	return result;
}
