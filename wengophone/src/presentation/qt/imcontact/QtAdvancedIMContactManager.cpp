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
#include "QtAdvancedIMContactManager.h"

#include "ui_IMContactManager.h"

#include <presentation/qt/imcontact/QtIMContactItem.h>
#include <presentation/qt/imcontact/QtAddIMContact.h>

#include <control/contactlist/CContactList.h>
#include <control/profile/CUserProfile.h>

#include <model/contactlist/ContactProfile.h>

#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>
//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>
QtAdvancedIMContactManager::QtAdvancedIMContactManager(ContactProfile & contactProfile,
	CUserProfile & cUserProfile, QWidget * parent)
	: QtIMContactManagerInterface(contactProfile, cUserProfile, parent) {

	_imContactManagerWidget = new QWidget(NULL);

	_ui = new Ui::IMContactManager();
	_ui->setupUi(_imContactManagerWidget);

	SAFE_CONNECT(_ui->addIMContactButton, SIGNAL(clicked()), SLOT(addIMContact()));
	SAFE_CONNECT(_ui->deleteIMContactButton, SIGNAL(clicked()), SLOT(deleteIMContact()));
	
	SAFE_CONNECT(_ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem* )), 
		SLOT(currentItemChangedSlot(QTreeWidgetItem*, QTreeWidgetItem*)));
	
	loadIMContacts();
	_ui->deleteIMContactButton->setEnabled(false);
}

QtAdvancedIMContactManager::~QtAdvancedIMContactManager() {
	OWSAFE_DELETE(_ui);
}

void QtAdvancedIMContactManager::loadIMContacts() {
	_ui->treeWidget->clear();

	const IMContactSet & imContactSet = _contactProfile.getIMContactSet();

	for (IMContactSet::const_iterator it = imContactSet.begin(); it != imContactSet.end(); it++) {

		IMContact * imContact = (IMContact *) &(*it);
		QStringList imContactStrList;
		imContactStrList << QString::fromStdString(imContact->getContactId());
		EnumIMProtocol::IMProtocol imProtocol = imContact->getProtocol();		//VOXOX - JRT - 2009.07.02 - OK?

		if (imProtocol == EnumIMProtocol::IMProtocolSIP) {
			//This protocol is internal to WengoPhone, should not be shown to the user
			//continue;
		}

		imContactStrList << QString::fromStdString(EnumIMProtocol::toString(imProtocol));
		imContactStrList << QString::null;

		QtIMContactItem * item = new QtIMContactItem(_ui->treeWidget, imContactStrList);
		item->setIMContact(imContact);
	}
	
	contentHasChanged();
}

void QtAdvancedIMContactManager::addIMContact() {
	QtAddIMContact(_contactProfile, _cUserProfile, _imContactManagerWidget); //Blocking until window closed
	loadIMContacts();
}

void QtAdvancedIMContactManager::deleteIMContact() {
	QtIMContactItem * imContactItem = (QtIMContactItem *) _ui->treeWidget->currentItem();
	if (imContactItem) {
		IMContact * imContact = imContactItem->getIMContact();

		//VOXOX - CJC - 2009.06.10 
		QtVoxMessageBox box(_imContactManagerWidget);
		box.setWindowTitle("VoxOx");
		box.setText("Are sure you want to delete this IM contact?\n" + QString::fromStdString(imContact->getContactId()));

		box.addButton(tr("&Delete"), QMessageBox::AcceptRole);
		box.addButton(tr("&Cancel"), QMessageBox::RejectRole);

		int ret = box.exec();



		/*int buttonClicked = QMessageBox::question(_imContactManagerWidget,
					tr("@product@"),
					tr("Are sure you want to delete this IM contact?\n") +
					QString::fromStdString(imContact->getContactId()),
					tr("&Delete"), tr("Cancel"));*/

		//Button delete clicked
		if (ret == QMessageBox::AcceptRole) {
			_contactProfile.removeIMContact(*imContact);
		}
	}

	loadIMContacts();
}

void QtAdvancedIMContactManager::currentItemChangedSlot(QTreeWidgetItem * current, QTreeWidgetItem * previous) {
	_ui->deleteIMContactButton->setEnabled(current);
}

bool QtAdvancedIMContactManager::couldBeSaved() {
	return (!_contactProfile.getIMContactSet().empty());
}

void QtAdvancedIMContactManager::saveIMContacts() {
	// nothing to do here, contacts are already in _contactProfile
}
