/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* Custom Tooltip Manager
* @author Chris Jimenez C 
* @date 2009.05.11
*/


#include "stdafx.h"	
#include "QtContactActionManager.h"
//#include "../chat/QtContactProfileWidget.h"	//VOXOX - JRT - 2009.08.31 - Not needed

#include "QtContactTooltip.h"
#include "QtContact.h"

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMContact.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/profile/CUserProfile.h>
#include <qtutil/SafeConnect.h>
#include <control/CWengoPhone.h>
#include <control/contactlist/CContactList.h>
#include <model/profile/UserProfile.h>
#include <presentation/qt/messagebox/QtVoxMessageBox.h>
#include <presentation/qt/messagebox/QtVoxPhoneNumberMessageBox.h>
#include <presentation/qt/messagebox/QtVoxEmailMessageBox.h>
#include <presentation/qt/QtEnumPhoneType.h>
#include <presentation/qt/QtEnumEmailType.h>
#include <util/WebBrowser.h>
#include <QtGui/QtGui>
#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/QtVoxWindowManager.h>

QtContactActionManager * QtContactActionManager::instance = NULL;



QtContactActionManager::QtContactActionManager(CWengoPhone & cWengoPhone)
	:QObject(),
	_cWengoPhone(cWengoPhone) {

}

QtContactActionManager::~QtContactActionManager() {
	
}


QtContactActionManager *QtContactActionManager::getInstance(CWengoPhone & cWengoPhone) {

	if (!instance) {
		instance = new QtContactActionManager(cWengoPhone);
	}

	return instance;
}


void QtContactActionManager::callContact(const QString & contactIdIn )
{
	std::string contactId = contactIdIn.toStdString();

	//VOXOX - JRT - 2009.07.26
	if ( getCUserProfile()->getCContactList().canCallContact( contactId ) )
	{
		getCUserProfile()->callContact( contactId );
	}
	else
	{
		QString number = addNumberToContact(contactIdIn);

		if(number !="" )
		{
//			getCUserProfile()->makeCall(number.toStdString());	//VOXOX - CJC - 2009.06.11 
			getCUserProfile()->callContact( contactId );	
		}
	}

	////VOXOX - JRT - 2009.07.26 - Move this business logic to UserProfile and Contact.
	//ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
	//if(contactProfile.getKey()!="")
	//{
	//	//VOXOX - CJC - 2009.06.23 Fix bug not calling voxox contact
	//	if(contactProfile.getPreferredIMContact()->isIMAccountVoxOx())
	//	{
	//		if(!contactProfile.getVoxOxPhone().empty())
	//		{
	//			getCUserProfile()->makeCall(contactProfile.getPreferredNumber());	//VOXOX - CJC - 2009.06.11 
	//		}
	//		else
	//		{
	//			getCUserProfile()->makeContactCall(contactId.toStdString());
	//		}
	//	}
	//	else
	//	{
	//		if(contactProfile.getTelephones().hasValid())
	//		{
	//			getCUserProfile()->makeCall(contactProfile.getPreferredNumber());	//VOXOX - CJC - 2009.06.11 
	//		}
	//		else
	//		{
	//			QString number = addNumberToContact(contactId);
	//			if(number!="")
	//			{
	//				getCUserProfile()->makeCall(number.toStdString());	//VOXOX - CJC - 2009.06.11 
	//			}
	//		}
	//	}
	//}
}


//VOXOX - CJC - 2009.06.14 
QString QtContactActionManager::addNumberToContact(const QString & contactId) 
{
	QString number = "";
	
//	ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
		
//	if(contactProfile.getKey()!="")
//	{
		QtVoxPhoneNumberMessageBox box(0);

		box.setTitle(tr("VoxOx - Enter Number"));
		box.setDialogText(tr("Please enter the contact phone number"));
		box.setNumberTypeVisible(true);
		
		if(box.exec() == QDialog::Accepted)
		{
			number = box.getNumber();

			if ( number != "" )						//VOXOX - JRT - 2009.07.26 - TODO: Accept should not be allowed with an empty number.
			{
				Telephones phones;

				//Save number to contact
				QtEnumPhoneType::Type type = box.getPhoneType();

				switch(type)
				{
					case QtEnumPhoneType::Mobile:
//						contactProfile.setMobilePhone(number.toStdString());
						phones.addMobileNumber( number.toStdString() );		//VOXOX - JRT - 2009.07.26 
						break;
					case QtEnumPhoneType::Work:
//						contactProfile.setWorkPhone(number.toStdString());
						phones.addWorkNumber( number.toStdString() );		//VOXOX - JRT - 2009.07.26 
						break;
					case QtEnumPhoneType::Home:
//						contactProfile.setHomePhone(number.toStdString());
						phones.addHomeNumber( number.toStdString() );		//VOXOX - JRT - 2009.07.26 
						break;
					case QtEnumPhoneType::Fax:
//						contactProfile.setFax(number.toStdString());
						phones.addFaxNumber( number.toStdString() );		//VOXOX - JRT - 2009.07.26 
						break;
					case QtEnumPhoneType::Other:
//						contactProfile.setOtherPhone(number.toStdString());
						phones.addOtherNumber( number.toStdString() );		//VOXOX - JRT - 2009.07.26 
						break;
				}

//				getCUserProfile()->getCContactList().updateContact(contactProfile);
				getCUserProfile()->getCContactList().addContactPhones( contactId.toStdString(), phones );
			}
		}
//	}

	return number;
}


//VOXOX - CJC - 2009.06.16 
void QtContactActionManager::smsContact(const QString & contactId) 
{
	//VOXOX - JRT - 2009.07.26 - If contact cannot receive SMS, allow user to add mobile number
	if ( !getCUserProfile()->getCContactList().canContactReceiveSMS( contactId.toStdString() ) )
	{
		QString number = addMobileNumberToContact( contactId );
	}

	//Now try to actually send SMS, 
	if ( getCUserProfile()->getCContactList().canContactReceiveSMS( contactId.toStdString() ) )
	{
		if ( isVoxOxIMConnected() )
		{
//			getCUserProfile()->startIMToSMS(contactProfile.getMobilePhone());
			getCUserProfile()->startIMToSMS( contactId.toStdString(), "" );	//Use first mobile if empty.
		}
	}

	//VOXOX - JRT - 2009.07.26 - Business logic
	//ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
	//if(contactProfile.getKey()!="")
	//{
	//	if(contactProfile.getMobilePhone()!="")
	//	{
	//		//VOXOX - CJC - 2009.06.11 
	//		getCUserProfile()->startIMToSMS(contactProfile.getMobilePhone());
	//	}
	//	else
	//	{
	//		QString number = addMobileNumberToContact(contactId);
	//		if(number!="")
	//		{
	//			if(getCUserProfile()->getUserProfile().getVoxOxAccount()->isConnected())
	//			{
	//				getCUserProfile()->startIMToSMS(number.toStdString());
	//			}
	//			else
	//			{
	//				QtVoxMessageBox box(0);
	//				box.setWindowTitle(tr("Your VoxOx IM account is offline"));
	//				box.setText(tr("Your VoxOx IM account is offline. To send text messages, please reconnect and try again."));
	//				box.setStandardButtons(QMessageBox::Ok);
	//				box.exec();
	//			}
	//		}
	//	}
	//}
}

bool QtContactActionManager::isVoxOxIMConnected()
{
	bool result = false;

	if(getCUserProfile()->getUserProfile().getVoxOxAccount()->isConnected())
	{
		result = true;
	}
	else
	{
		QtVoxMessageBox box(0);
		box.setWindowTitle(tr("Your VoxOx IM account is offline"));
		box.setText(tr("Your VoxOx IM account is offline. To send text messages, please reconnect and try again."));
		box.setStandardButtons(QMessageBox::Ok);
		box.exec();
	}

	return result;
}

//VOXOX - CJC - 2009.06.23 
void QtContactActionManager::sendFaxContact(const QString & contactId) 
{
	//VOXOX - JRT - 2009.07.26 
	if ( !getCUserProfile()->getCContactList().canContactReceiveFax( contactId.toStdString() ) )
	{
		QString number = addFaxNumberToContact(contactId);
	}

	if ( getCUserProfile()->getCContactList().canContactReceiveFax( contactId.toStdString() ) )
	{
		getCUserProfile()->sendFax( contactId.toStdString(), "" );
	}

	//ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
	//if(contactProfile.getKey()!="")
	//{
	//	if(contactProfile.getFax()!="")
	//	{
	//		//VOXOX - CJC - 2009.06.11 
	//		getCUserProfile()->sendFax(contactProfile.getFax());
	//	}
	//	else
	//	{
	//		QString number = addFaxNumberToContact(contactId);
	//		if(number!="")
	//		{
	//			getCUserProfile()->sendFax(number.toStdString());
	//		}
	//	}
	//}
}


QString QtContactActionManager::addMobileNumberToContact(const QString & contactId) 
{
	QString number = "";

//	ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
//	if(contactProfile.getKey()!="")
//	{
		QtVoxPhoneNumberMessageBox box(0);

		box.setTitle(tr("VoxOx - Enter Mobile Number"));
		box.setDialogText(tr("Please enter the mobile phone number for the contact"));
		box.setNumberTypeVisible(false);
		
		if(box.exec() == QDialog::Accepted)
		{
			number = box.getNumber();		//VOXOX - JRT - 2009.07.26 - TODO: Accept should not be valid option if number is empty.

			if(number!="")
			{
				//VOXOX - JRT - 2009.07.26 
				Telephones phones;
				phones.addMobileNumber( number.toStdString() );
				getCUserProfile()->getCContactList().addContactPhones( contactId.toStdString(), phones );

//				contactProfile.setMobilePhone(number.toStdString());
//				_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().updateContact(contactProfile);
			}
		}
//	}

	return number;
}
//VOXOX - CJC - 2009.06.23 
QString QtContactActionManager::addFaxNumberToContact(const QString & contactId) 
{
	QString number = "";
//	ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
//	if(contactProfile.getKey()!="")
//	{
		QtVoxPhoneNumberMessageBox box(0);

		box.setTitle(tr("VoxOx - Enter Fax Number"));
		box.setDialogText(tr("Please enter the fax phone number for the contact"));
		box.setNumberTypeVisible(false);
		
		if(box.exec() == QDialog::Accepted)
		{
			number = box.getNumber();	//VOXOX - JRT - 2009.07.26 - TODO: Accept should not be valid if number is empty.

			if(number!="")
			{
				Telephones phones;
				phones.addFaxNumber( number.toStdString() );
				getCUserProfile()->getCContactList().addContactPhones( contactId.toStdString(), phones );

//				contactProfile.setFax(number.toStdString());
//				_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().updateContact(contactProfile);
			}
		}
//	}

	return number;
}

//VOXOX - CJC - 2009.06.16 
void QtContactActionManager::chatContact(const QString & contactId) 
{
	//VOXOX - JRT - 2009.07.26 - This makes no sense.
	//ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
	//if(contactProfile.getKey()!="")
	//{
	//	getCUserProfile()->startIM(contactId.toStdString());
	//}

	getCUserProfile()->startIM(contactId.toStdString());
}

//VOXOX - CJC - 2009.06.16 
void QtContactActionManager::emailContact(const QString & contactId) 
{
	//VOXOX - JRT - 2009.07.26 - If user cannot receive email, prompt user.
	if ( !getCUserProfile()->getCContactList().canContactReceiveEmail( contactId.toStdString() ) )
	{
		QString email = addEmailToContact(contactId);
	}

	//VOXOX - JRT - 2009.07.26 - if user can now receive email, then send it.
	if ( getCUserProfile()->getCContactList().canContactReceiveEmail( contactId.toStdString() ) )
	{
		if ( isVoxOxIMConnected() )
		{
			getCUserProfile()->startIMToEmail( contactId.toStdString(), "" );
		}
	}

	//ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
	//if(contactProfile.getKey()!="")
	//{
	//	QString email;
	//	if(!contactProfile.getEmailAddresses().hasValid())
	//	{
	//		//Send email to contact
	//		email = addEmailToContact(contactId);
	//		getCUserProfile()->startIMToEmail(email.toStdString());
	//	}
	//	else
	//	{
	//		if(getCUserProfile()->getUserProfile().getVoxOxAccount()->isConnected())
	//		{
	//			getCUserProfile()->startIMToEmail(contactProfile.getPreferredEmail());
	//		}
	//		else
	//		{
	//			QtVoxMessageBox box(0);
	//			box.setWindowTitle(tr("Your VoxOx IM account is offline"));
	//			box.setText(tr("Your VoxOx IM account is offline. To send emails, please reconnect and try again."));
	//			box.setStandardButtons(QMessageBox::Ok);
	//			box.exec();
	//		}
	//	}
	//}
}


//VOXOX - CJC - 2009.06.14 
QString QtContactActionManager::addEmailToContact(const QString & contactId)
{
	QString email = "";

	//VOXOX - JRT - 2009.07.26 
//	ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());

//	if(contactProfile.getKey()!="")
//	{
		QtVoxEmailMessageBox box(0);

		box.setTitle(tr("VoxOx - Enter Email"));
		box.setDialogText(tr("Please enter the contact email address"));
		box.setEmailTypeVisible(true);
		
		if(box.exec() == QDialog::Accepted)
		{
			email = box.getEmail();				//VOXOX - JRT - 2009.07.26 - Accept should be invalid if email is empty

			if( email != "" )
			{
				EmailAddresses emails;

				//Save email to contact
				QtEnumEmailType::Type type = box.getEmailType();

				switch(type)
				{
					case QtEnumEmailType::Personal:
//						contactProfile.setPersonalEmail(email.toStdString());
						emails.addPersonal( email.toStdString() );
						break;
					case QtEnumEmailType::Work:
//						contactProfile.setWorkEmail(email.toStdString());
						emails.addWork( email.toStdString() );
						break;
					case QtEnumEmailType::Other:
//						contactProfile.setOtherEmail(email.toStdString());
						emails.addOther( email.toStdString() );
						break;
				}

//				_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().updateContact(contactProfile);
				getCUserProfile()->getCContactList().addContactEmailAddresses( contactId.toStdString(), emails );
			}
		}
//	}
	
	return email;
}

void QtContactActionManager::profileContact(const QString & contactId) 
{
	QString realContactId = contactId;
//	ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
//	///*if(contactProfile.getKey()!="")
//	//{*/
////		getCUserProfile()->openContactProfile(contactId.toStdString());	//VOXOX - JRT - 2009.06.29 
////		QtContactProfileWidget dlg = QtContactProfileWidget(&_qtWengoPhone, NULL );
//
//// VOXOX CHANGE by ASV 06-30-2009: on Mac we need to have a parent so we don't lose the menu.
//		//VOXOX - JRT - 2009.06.30 - Who deletes this?
//		//	Should work for Windows too, but it keeps CM on top of main window.
//#if defined(OS_MACOSX)
//		QtWengoPhone *qtWengoPhone = dynamic_cast<QtWengoPhone *>(_cWengoPhone.getPresentation());
//		QtContactProfileWidget *dlg = new QtContactProfileWidget(&_cWengoPhone, contactId, qtWengoPhone->getWidget() );
//		dlg->setModal( false );
//#endif
//#if defined OS_WINDOWS
//		QtContactProfileWidget *dlg = new QtContactProfileWidget(&_cWengoPhone, contactId, NULL );		
//		dlg->setModal( false );
//#endif
//		
//		dlg->show();	// VOXOX CHANGE by ASV 06-30-2009: We need to use show in order to get all the top buttons active
	//}

	QtWengoPhone *qtWengoPhone = dynamic_cast<QtWengoPhone *>(_cWengoPhone.getPresentation());
	qtWengoPhone->getQtVoxWindowManager()->showContactManagerWindow(realContactId, QtContactProfileWidget::Action_Edit );

}

void QtContactActionManager::sendFileContact(const QString & contactId) 
{
	//VOXOX - JRT - 2009.07.26 - This makes no sense.
	//ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
	//if(contactProfile.getKey()!="")
	//{
	//	getCUserProfile()->sendFile(contactId.toStdString());	
	//}

	getCUserProfile()->sendFile( contactId.toStdString() );	
}

void QtContactActionManager::openSocialContactPage(const QString & contactId) 
{
	//ContactProfile contactProfile = getCUserProfile()->getCContactList().getContactProfile(contactId.toStdString());
	//if(contactProfile.getKey()!="")
	//{
	//	std::string address = contactProfile.getContactSocialAddress();
	//	if(address!="")
	//	{
	//		WebBrowser::openUrl(address);
	//	}
	//}

	std::string address = getCUserProfile()->getCContactList().getContactSocialAddress( contactId.toStdString() );

	if( !address .empty() )
	{
		WebBrowser::openUrl(address);
	}
}


CUserProfile* QtContactActionManager::getCUserProfile()
{
	return _cWengoPhone.getCUserProfileHandler().getCUserProfile();
}
