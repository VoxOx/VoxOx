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
#include "QtIMAccountManager.h"

#include "ui_IMAccountManager.h"

#include "QtIMAccountSettings.h"

#include <model/connect/ConnectHandler.h>
#include <model/profile/UserProfile.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>
#include <imwrapper/QtEnumIMProtocol.h>	
//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>

QtIMAccountManager::QtIMAccountManager(UserProfile & userProfile, bool showAsDialog, QWidget * parent)
	: QObject(parent),
	_userProfile(userProfile) {

	if (showAsDialog) {
		_imAccountManagerWidget = new QDialog(parent);
	} else {
		_imAccountManagerWidget = new QWidget(parent);
	}

	_ui = new Ui::IMAccountManager();
	_ui->setupUi(_imAccountManagerWidget);

	QMenu * addIMAccountMenu = new QMenu(_ui->addIMAccountButton);
	SAFE_CONNECT(addIMAccountMenu, SIGNAL(triggered(QAction *)), SLOT(addIMAccount(QAction *)));

	//VOXOX - JRT - 2009.04.21 - get from QtEnumIMProtocolMap settings
	//  TODO: we may have TOO many here.  Add new memvar to QtEnumIMProtocolMap if necessary
	QtEnumIMProtocolMap& qtProtocolMap = QtEnumIMProtocolMap::getInstance();

	for ( QtEnumIMProtocolMap::iterator it = qtProtocolMap.begin(); it != qtProtocolMap.end(); it++ )
	{
		if ( (*it).second.useInIMAccountMenu() )
		{
			QIcon icon( (*it).second.getIconPath().c_str() );
			QString strText = (*it).second.getName().c_str();

			addIMAccountMenu->addAction( icon, strText );
		}
	}
	//End Voxox

	_ui->addIMAccountButton->setMenu(addIMAccountMenu);

	SAFE_CONNECT(_ui->modifyIMAccountButton, SIGNAL(clicked()), SLOT(modifyIMAccount()));
	SAFE_CONNECT(_ui->deleteIMAccountButton, SIGNAL(clicked()), SLOT(deleteIMAccount()));

	if (showAsDialog) 
	{
		SAFE_CONNECT_RECEIVER(_ui->closeButton, SIGNAL(clicked()), _imAccountManagerWidget, SLOT(accept()));
	} 
	else 
	{
		_imAccountManagerWidget->layout()->setMargin(0);
		_imAccountManagerWidget->layout()->setSpacing(0);
		_ui->closeButton->hide();
	}

	SAFE_CONNECT(_ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(itemDoubleClicked(QTreeWidgetItem *, int)));
	SAFE_CONNECT(_ui->treeWidget, SIGNAL(itemSelectionChanged()),					 SLOT(updateButtons()));

	loadIMAccounts();
	updateButtons();
	
	if (showAsDialog) 
	{
		((QDialog *) _imAccountManagerWidget)->exec();
	}
}

QtIMAccountManager::~QtIMAccountManager() 
{
	OWSAFE_DELETE(_ui);
}

void QtIMAccountManager::loadIMAccounts() 
{
	_ui->treeWidget->clear();

	IMAccountList imAccountList = _userProfile.getIMAccountManager().getIMAccountListCopy();
	QtEnumIMProtocolMap& qtProtocolMap = QtEnumIMProtocolMap::getInstance();

	for (IMAccountList::iterator it = imAccountList.begin(); it != imAccountList.end(); it++) 
	{
		IMAccount imAccount = it->second;
		QStringList accountStrList;

		// protocol
//		EnumIMProtocol::IMProtocol imProtocol = imAccount.getProtocol();
		QtEnumIMProtocol::IMProtocol imProtocol = imAccount.getQtProtocol();	//VOXOX - JRT - 2009.07.01 

		if ((imProtocol == QtEnumIMProtocol::IMProtocolWengo) ||
			(imProtocol == QtEnumIMProtocol::IMProtocolSIP) || imAccount.isVoxOxAccount()){
			//only IM account should be shown
			continue;
		}

		std::string protocolName = qtProtocolMap.findByQtProtocol( imProtocol )->getName();

		accountStrList << QString::fromStdString( protocolName );
		accountStrList << QString::fromStdString(imAccount.getLogin());	// login
		accountStrList << QString::null;

		QtEnumIMProtocol* protocol = qtProtocolMap.findByQtProtocol( imProtocol );	//VOXOX - JRT - 2009.07.01 
		
		QTreeWidgetItem * item = new QTreeWidgetItem(_ui->treeWidget, accountStrList);
		item->setIcon(0,QIcon(protocol->getIconPath().c_str()));
		QString id = QString::fromStdString(imAccount.getKey());
		item->setData(0, Qt::UserRole, id);
	}
}

void QtIMAccountManager::addIMAccount(QAction * action) 
{
	QString protocolName = action->text();
	LOG_DEBUG(protocolName.toStdString());

	QtEnumIMProtocol::IMProtocol imProtocol = QtEnumIMProtocolMap::getInstance().toIMProtocol(protocolName.toStdString());
	QtIMAccountSettings * imAccountSettings = new QtIMAccountSettings(_userProfile, imProtocol, _imAccountManagerWidget);
	loadIMAccounts();
	OWSAFE_DELETE(imAccountSettings);
}

std::auto_ptr<IMAccount> QtIMAccountManager::getIMAccountFromItem(QTreeWidgetItem* item) 
{
	if (!item) 
	{
		return std::auto_ptr<IMAccount>(0);
	}
	std::string id = item->data(0, Qt::UserRole).toString().toStdString();
	return std::auto_ptr<IMAccount>(_userProfile.getIMAccountManager().getIMAccount(id));
}

void QtIMAccountManager::deleteIMAccount() 
{
	QTreeWidgetItem* item = _ui->treeWidget->currentItem();
	std::auto_ptr<IMAccount> imAccount = getIMAccountFromItem(item);
	if (!imAccount.get()) 
	{
		return;
	}

	//VOXOX - CJC - 2009.06.10 
	QtVoxMessageBox box(_imAccountManagerWidget);
	box.setWindowTitle("VoxOx");
	box.setText("Are sure you want to delete this account?\n");

	box.addButton(tr("&Delete"), QMessageBox::AcceptRole);
	box.addButton(tr("&Cancel"), QMessageBox::RejectRole);

	int ret = box.exec();

	//int buttonClicked = 
	//	QMessageBox::question(_imAccountManagerWidget,
	//		tr("@product@"),
	//		tr("Are sure you want to delete this account?\n") +
	//		QString::fromStdString(imAccount->getLogin()),
	//		

	//Button delete clicked
	if (ret == QMessageBox::AcceptRole) 
	{
		_userProfile.removeIMAccount(*imAccount);
	}

	delete item;
}

void QtIMAccountManager::modifyIMAccount() 
{
	QTreeWidgetItem* item = _ui->treeWidget->currentItem();
	std::auto_ptr<IMAccount> imAccount = getIMAccountFromItem(item);
	if (!imAccount.get()) {
		return;
	}
	QtIMAccountSettings * imAccountSettings =
		new QtIMAccountSettings(_userProfile, *imAccount.get(), _imAccountManagerWidget);
	OWSAFE_DELETE(imAccountSettings);

	imAccount = getIMAccountFromItem(item);
	item->setText(1, QString::fromStdString(imAccount->getLogin()));
}

void QtIMAccountManager::itemDoubleClicked(QTreeWidgetItem * item, int column) 
{
	_ui->treeWidget->setCurrentItem(item);
	modifyIMAccount();
}

void QtIMAccountManager::updateButtons() 
{
	bool itemSelected = _ui->treeWidget->selectedItems().count() != 0;
	_ui->modifyIMAccountButton->setEnabled(itemSelected);
	_ui->deleteIMAccountButton->setEnabled(itemSelected);
}
