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
#include "QtFastAddIMContact.h"

#include "ui_FastAddIMContact.h"

#include <control/contactlist/CContactList.h>
#include "../contactlist/QtRenameGroup.h"
#include <control/profile/CUserProfile.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/account/SipAccount.h>
#include <model/contactlist/ContactProfile.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/IMAccount.h>

#include <util/SafeDelete.h>

#include <QtGui/QtGui>
//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>
const QString DEFAULT_GROUP_NAME = "VoxOx";

QtFastAddIMContact::QtFastAddIMContact(ContactProfile & contactProfile,
	CUserProfile & cUserProfile, QWidget * parent,QVariant & groupId,std::string & groupName)
	: QDialog(0),
	_contactProfile(contactProfile),
	_cUserProfile(cUserProfile),
	_groupId(groupId),
	_groupName(groupName){



	_ui = new Ui::FastAddIMContact();
	_ui->setupUi(this);

	/*resize(QSize(394,300));
	setMinSize(QSize(394,300));
	setHasMenu(false);*/

	//VOXOX - CJC - 2009.12.17 
	QtEnumIMProtocolMap& qtProtocolMap = QtEnumIMProtocolMap::getInstance();

	for ( QtEnumIMProtocolMap::iterator it = qtProtocolMap.begin(); it != qtProtocolMap.end(); it++ )
	{
		if ( (*it).second.isIMProtocol() )
		{
			QIcon icon( (*it).second.getIconPath().c_str() );
			QString strText = (*it).second.getName().c_str();

			_ui->protocolComboBox->addItem(icon,strText);

			//addIMAccountMenu->addAction( icon, strText );
		}
	}





	//_ui->protocolComboBox->addItem(QIcon(QString::fromStdString(":pics/protocols/voxox.png")),
	//	QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolWengo)));
	//_ui->protocolComboBox->addItem(QIcon(QString::fromStdString(":pics/protocols/msn.png")),
	//	QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolMSN)));
	//_ui->protocolComboBox->addItem(QIcon(QString::fromStdString(":pics/protocols/aim.png")),
	//	QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolAIM)));
	//_ui->protocolComboBox->addItem(QIcon(QString::fromStdString(":pics/protocols/icq.png")),
	//	QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolICQ)));
	//_ui->protocolComboBox->addItem(QIcon(QString::fromStdString(":pics/protocols/yahoo.png")),
	//	QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolYahoo)));
	//_ui->protocolComboBox->addItem(QIcon(QString::fromStdString(":pics/protocols/gtalk.png")),
	//	"Gtalk");
	//_ui->protocolComboBox->addItem(QIcon(QString::fromStdString(":pics/protocols/jabber.png")),
	//	QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolJabber)));
	///*_ui->protocolComboBox->addItem(QIcon(QString::fromStdString(ConfigManager::getInstance().getCurrentStylePath())+"/img/pics/protocols/sip.png"),
	//	QString::fromStdString(EnumIMProtocol::toString(EnumIMProtocol::IMProtocolSIP)));*/
	connect(_ui->protocolComboBox, SIGNAL(currentIndexChanged(const QString &)),
		SLOT(imProtocolChanged(const QString &)));

	connect(_ui->saveButton, SIGNAL(clicked()), SLOT(addIMContact()));

	connect(_ui->newButton, SIGNAL(clicked()), SLOT(addGroup()));

	//VOXOX - JRT - 2009.05.09 - TODO: this same QComboBox is loaded in 2-3 places.  We need a derived class for this so groups are loaded consistently.
	ContactGroupSet& rGroups = _cUserProfile.getCContactList().getContactGroupSet();
	for ( ContactGroupSet::const_iterator it = rGroups.begin(); it != rGroups.end(); it++ )
	{
		if ( (*it).second.isUser() )
		{
			_ui->groupComboBox->addItem(QString::fromUtf8((*it).second.getName().c_str()), QString::fromStdString((*it).first.c_str()));
			std::string str1 = (*it).first;
			std::string str2 = contactProfile.getFirstUserGroupId();	//VOXOX - JRT - 2009.05.06 JRT-GRPS - TODO: How would multiple user groups affect this?

			if (str1 == str2) 
			{
				_ui->groupComboBox->setCurrentIndex(_ui->groupComboBox->findText(QString::fromUtf8((*it).second.getName().c_str())));	//VOXOX - JRT - 2009.05.09 
			}
		}
	}
	//End VoxOx

	if(_ui->groupComboBox->count() == 0){
		ContactGroup contactGroup( DEFAULT_GROUP_NAME.toStdString(), EnumGroupType::GroupType_User);	//VOXOX - JRT - 2009.05.07 JRT-GRPS
		_cUserProfile.getCContactList().addContactGroup( contactGroup );	//VOXOX - JRT - 2009.05.07 JRT-GRPS
		_ui->groupComboBox->addItem(DEFAULT_GROUP_NAME);
		_ui->groupComboBox->setCurrentIndex(0);
	}
}

QtFastAddIMContact::~QtFastAddIMContact() {
	delete _ui;
}

int QtFastAddIMContact::show() {
	imProtocolChanged(_ui->protocolComboBox->currentText());
	return this->exec();
}


void QtFastAddIMContact::addGroup(){
		
	QtRenameGroup dialog("", this);
	if (dialog.exec()) {
	    
		_ui->groupComboBox->addItem(dialog.getGroupName().toUtf8().data());
		_ui->groupComboBox->setCurrentIndex(_ui->groupComboBox->count()-1);  
	}
}

void QtFastAddIMContact::addIMContact() {

	if (_ui->groupComboBox->currentText().isEmpty()) {

		//VOXOX - CJC - 2009.06.10 
		QtVoxMessageBox box(this);
		box.setWindowTitle("VoxOx  - No Group Selected");
		box.setText(tr("A contact must have a group, please set a group."));
		box.setStandardButtons(QMessageBox::Ok);
		box.exec();

		//QMessageBox::warning(this,
		//	tr("VoxOx - No Group Selected"),
		//	tr("A contact must have a group, please set a group."),
		//	QMessageBox::NoButton,
		//	QMessageBox::NoButton,
		//	QMessageBox::Ok);

		return;
	}

	QString contactId = _ui->contactIdLineEdit->text().trimmed();
	
	
	if (contactId.isEmpty()) 
	{
		return;
	}

	int index = _ui->groupComboBox->findText(_ui->groupComboBox->currentText());
	_groupId  = _ui->groupComboBox->itemData(index);
	
	//If the group does not exist
//	if (_groupId.isNull())		//VOXOX - JRT - 2009.05.10 - Let's just always get this value.
	/*{*/
	_groupName = std::string(_ui->groupComboBox->currentText().toUtf8().data());
	//}
	
	QString protocolName = _ui->protocolComboBox->currentText();
	/*if(protocolName=="Gtalk")
	{
		protocolName="Jabber";
	}*/
	QtEnumIMProtocolMap& qtProtocolMap = QtEnumIMProtocolMap::getInstance();
	//QtEnumIMProtocol::IMProtocol protocol= qtProtocolMap.toIMProtocol(protocolName.toStdString());//VOXOX - CJC - 2009.12.17
	QtEnumIMProtocol::IMProtocol imProtocol = qtProtocolMap.toIMProtocol(protocolName.toStdString());//VOXOX - CJC - 2009.12.17
	
	//// sip contact should know their domain
	//if (imProtocol != QtEnumIMProtocol::IMProtocolWengo && imProtocol != QtEnumIMProtocol::IMProtocolSIP) 
	//{

	IMAccountList imAccounts = getSelectedIMAccounts(imProtocol);

	if (imAccounts.empty()) 
	{
		QtVoxMessageBox box(this);
		box.setWindowTitle("VoxOx  - Account does not exist");
		box.setText(QString("A %1 account does not exist, please add a %1 account and then add the contact.").arg(protocolName));
		box.setStandardButtons(QMessageBox::Ok);
		box.exec();
		return;
		/*_contactProfile.addIMContact(imContact);*/
	}

	
	if (imProtocol == QtEnumIMProtocol::IMProtocolVoxOx)//VOXOX - CJC - 2009.12.17 Lets add the extention if it is a voxox contact
	{
	//Trim and add jabberserver if voxox
		QString contactName = _ui->contactIdLineEdit->text();


		Config & config = ConfigManager::getInstance().getCurrentConfig();
		if(!contactName.contains("@", Qt::CaseInsensitive))
		{
			contactName = contactName+"@"+QString::fromStdString(config.getJabberVoxoxServer());
		}

		contactId = contactName;
	}

	IMContact imContact(qtProtocolMap.toModelIMProtocol(imProtocol), contactId.toStdString());//VOXOX - CJC - 2009.12.17 


	for (IMAccountList::const_iterator it = imAccounts.begin(); it != imAccounts.end(); ++it) 
	{
		IMAccount * imAccount = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount( it->second );
		imContact.setIMAccount(imAccount);
		_contactProfile.addIMContact(imContact);
		OWSAFE_DELETE(imAccount);
	}
	/*} */
	/*else if (imProtocol == EnumIMProtocol::IMProtocolSIP) 
	{
		if (!contactId.contains("@")) 
		{
			SipAccount * sipaccount = _cUserProfile.getUserProfile().getSipAccount();
			if (sipaccount) 
			{
				contactId += QString("@") + QString::fromStdString(sipaccount->getRealm());
			}
		}

		IMContact imContact(imProtocol, contactId.toStdString());

		IMAccountList imAccounts = getSelectedIMAccounts(imProtocol);

		if (imAccounts.empty()) {
			_contactProfile.addIMContact(imContact);
		}

		for (IMAccountList::const_iterator it = imAccounts.begin(); it != imAccounts.end(); ++it) 
		{

			IMAccount * imAccount = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount( it->second );
			imContact.setIMAccount(imAccount);
			_contactProfile.addIMContact(imContact);
			OWSAFE_DELETE(imAccount);
		}
	}*/
	//else if(imProtocol == EnumIMProtocol::IMProtocolWengo)
	//{
	//	//Trim and add jabberserver if voxox
	//	QString contactName = _ui->contactIdLineEdit->text();


	//	Config & config = ConfigManager::getInstance().getCurrentConfig();
	//	if(!contactName.contains("@", Qt::CaseInsensitive))
	//	{
	//		contactName = contactName+"@"+QString::fromStdString(config.getJabberVoxoxServer());
	//	}

	//	contactId = contactName;

	//	IMContact imContact(EnumIMProtocol::IMProtocolJabber, contactId.toStdString());

	//	//VOXOX - JRT - 2009.04.27 - NOTE: From here to end of method is all business logic and has no place here.
	//	//	Whatever this does should be encapsulated (in _userProfile?).
	//	IMAccount * imAccount = _cUserProfile.getUserProfile().getVoxOxAccount();

	//	//if (imAccounts.empty()) 
	//	//{
	//	//	//_contactProfile.addIMContact(imContact);
	//	//}

	//	//for (IMAccountList::const_iterator it = imAccounts.begin(); it != imAccounts.end(); ++it) 
	//	//{
	//	//	//LOG_DEBUG("In loop");
	//	//	if( it->second.isVoxOxAccount())
	//	//	{
	//			//LOG_DEBUG("Is jabber and voxox account");
	//			//IMAccount * imAccount = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount( it->second );

	//		imContact.setIMAccount(imAccount);
	//		_contactProfile.addIMContact(imContact);
	//		//OWSAFE_DELETE(imAccount);
	//	/*	}
	//	}*/
	//}
	
	this->accept();
}

void QtFastAddIMContact::imProtocolChanged(const QString & protocolName) {

	QtEnumIMProtocolMap& qtProtocolMap = QtEnumIMProtocolMap::getInstance();//VOXOX - CJC - 2009.12.17 


	QtEnumIMProtocol::IMProtocol protocol= qtProtocolMap.toIMProtocol(protocolName.toStdString());//VOXOX - CJC - 2009.12.17

	loadIMAccounts(protocol);//VOXOX - CJC - 2009.12.17 
}

void QtFastAddIMContact::loadIMAccounts(QtEnumIMProtocol::IMProtocol imProtocol) 
{
	
	_ui->treeWidget->clear();

	QtEnumIMProtocolMap& qtProtocolMap = QtEnumIMProtocolMap::getInstance();//VOXOX - CJC - 2009.12.17 

	IMAccountList imAccounts = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccountsOfQtProtocol(imProtocol);

	for (IMAccountList::const_iterator it = imAccounts.begin(); it != imAccounts.end(); ++it) 
	{
		/*if(!( it->second.isVoxOxAccount()) )
		{*/
			QTreeWidgetItem * item = new QTreeWidgetItem(_ui->treeWidget);

			//By default, check the first element only
			if (it == imAccounts.begin()) 
			{
				item->setCheckState(0, Qt::Checked);
			} 
			else 
			{
				item->setCheckState(0, Qt::Unchecked);
			}

			////If it voxox we need to change it to show the jabber id and not the phone number
			//if(imProtocol== QEnumIMProtocol::IMProtocolJabber)
			//{
			//	std::string jabberLogin = it->second.getLogin();
			//	//Search for the real Jabber voxox account
			//	
			//	{
			//		if(itJabber->second.isVoxOxAccount())
			//		{
			//			jabberLogin = itJabber->second.getLogin();
			//		}
			//	}

			//	item->setText(1, QString::fromStdString(jabberLogin));
			//}
			//else
			//{
				item->setText(1, QString::fromStdString( it->second.getLogin() ));
			//}
		//}
	}
}

IMAccountList QtFastAddIMContact::getSelectedIMAccounts(QtEnumIMProtocol::IMProtocol imProtocol) const 
{
	IMAccountList result;

	IMAccountList imAccounts = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccountsOfQtProtocol(imProtocol);

	for (IMAccountList::const_iterator it = imAccounts.begin(); it != imAccounts.end(); ++it) 
	{

		QList<QTreeWidgetItem *> list = _ui->treeWidget->findItems(QString::fromStdString( it->second.getLogin()), Qt::MatchExactly, 1);

		if (list.size() > 0) 
		{
			//There should be only one item
			QTreeWidgetItem * item = list[0];
			if (item->checkState(0) == Qt::Checked) 
			{
				result.Add( const_cast<IMAccount&>(it->second) );
			}
		}
	}

	return result;
}

