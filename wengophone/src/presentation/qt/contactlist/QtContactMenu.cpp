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
#include "QtContactMenu.h"

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/profile/UserProfile.h>
#include <model/contactlist/ContactProfile.h>
#include <model/contactlist/ContactList.h>			//VOXOX - JRT - 2009.07.27 -ContactInfo2

#include <util/Logger.h>

#include <QtGui/QtGui>

void QtContactMenu::populateMobilePhoneMenu(QMenu * menu, CWengoPhone & cWengoPhone) 
{
	CUserProfile * currentCUserProfile = cWengoPhone.getCUserProfileHandler().getCUserProfile();

	if (currentCUserProfile) 
	{
		QString			 menuText  = "";
		QAction*		 tmpAction = NULL;
		ContactInfoList2 infoList;

		currentCUserProfile->getCContactList().getContactsWithMobileNumber( infoList );

		for (ContactInfoList2::iterator it = infoList.begin(); it != infoList.end(); ++it) 
		{
			std::string mobileNumber = (*it).getTelephones().getMobileNumber();

			if ( !mobileNumber.empty() )
			{
				QString mobilePhone  = mobileNumber.c_str();
				QString displayName  = QString::fromUtf8( (*it).getDisplayName().c_str() );
//				QString mobilePhone2 = mobileNumber.c_str();

				if (displayName == "") 
				{
					menuText = mobilePhone;
				} 
				else 
				{
					menuText = displayName + ": " + mobilePhone;
				}

				tmpAction = menu->addAction( menuText );
				tmpAction->setData(QVariant(mobilePhone));
			}
		}
	}
}

void QtContactMenu::populateHomePhoneMenu(QMenu * menu, CWengoPhone & cWengoPhone) 
{
	CUserProfile * currentCUserProfile = cWengoPhone.getCUserProfileHandler().getCUserProfile();

	if (currentCUserProfile) 
	{
		QString			 menuText  = "";
		QAction*		 tmpAction = NULL;
		ContactInfoList2 infoList;

		currentCUserProfile->getCContactList().getContactsWithHomeNumber( infoList );

		for (ContactInfoList2::iterator it = infoList.begin(); it != infoList.end(); ++it) 
		{
			std::string number = (*it).getTelephones().getHomeNumber();

			if ( !number.empty() )
			{
				QString homePhone   = number.c_str();
				QString displayName = QString::fromUtf8( (*it).getDisplayName().c_str() );

				if (displayName == "") 
				{
					menuText = homePhone;
				} 
				else 
				{
					menuText = displayName + ": " + homePhone;
				}

				tmpAction = menu->addAction( menuText );
				tmpAction->setData(QVariant(homePhone));
			}
		}
	}
}

void QtContactMenu::populateWengoCallMenu(QMenu * menu, CWengoPhone & cWengoPhone) 
{
	CUserProfile * currentCUserProfile = cWengoPhone.getCUserProfileHandler().getCUserProfile();

	if (currentCUserProfile) 
	{
		QString			 menuText  = "";
		QAction*		 tmpAction = NULL;
		ContactInfoList2 infoList;

		currentCUserProfile->getCContactList().getContactsWithWengoCall( infoList );

		for (ContactInfoList2::iterator it = infoList.begin(); it != infoList.end(); ++it) 
		{
			QString displayName     = QString::fromUtf8( (*it).getDisplayName().c_str() );
			QString freePhoneNumber = QString::fromStdString( (*it).getTelephones().getVoxOxNumber() );

			tmpAction = menu->addAction(displayName);
			tmpAction->setData(QVariant(freePhoneNumber));

			setPresenceIcon(tmpAction, (*it).getPresenceState());
		}
	}
}

void QtContactMenu::populateSipCallMenu(QMenu * menu, CWengoPhone & cWengoPhone) 
{
	CUserProfile * currentCUserProfile = cWengoPhone.getCUserProfileHandler().getCUserProfile();

	if (currentCUserProfile) 
	{
		QString			 menuText  = "";
		QAction*		 tmpAction = NULL;
		ContactInfoList2 infoList;

		currentCUserProfile->getCContactList().getContactsWithSipNumber( infoList );

		for (ContactInfoList2::iterator it = infoList.begin(); it != infoList.end(); ++it) 
		{
			QString displayName     = QString::fromUtf8( (*it).getDisplayName().c_str() );
			QString freePhoneNumber = QString::fromStdString( (*it).getTelephones().getVoxOxNumber() );

			tmpAction = menu->addAction(displayName);
			tmpAction->setData(QVariant(freePhoneNumber));

			if ( (*it).getPresenceState() == EnumPresenceState::PresenceStateUnknown) 
			{
				setPresenceIcon(tmpAction, QIcon(":/pics/status/unknown-sip.png"));
			} 
			else 
			{
				setPresenceIcon( tmpAction, (*it).getPresenceState() );
			}
		}
	}
}

void QtContactMenu::populateChatMenu(QMenu * menu, CWengoPhone & cWengoPhone) 
{
	CUserProfile * currentCUserProfile = cWengoPhone.getCUserProfileHandler().getCUserProfile();

	if (currentCUserProfile) 
	{
		QString			 menuText  = "";
		QAction*		 tmpAction = NULL;
		ContactInfoList2 infoList;

		currentCUserProfile->getCContactList().getContactsThatCanChat( infoList );

		for (ContactInfoList2::iterator it = infoList.begin(); it != infoList.end(); ++it) 
		{
			QString displayName     = QString::fromUtf8( (*it).getDisplayName().c_str() );
			QString freePhoneNumber = QString::fromStdString( (*it).getTelephones().getVoxOxNumber() );
			QString key				= (*it).getKey().c_str();

			tmpAction = menu->addAction(displayName);
			tmpAction->setData(QVariant(key));

			setPresenceIcon(tmpAction, (*it).getPresenceState());
		}
	}
}

void QtContactMenu::setPresenceIcon(QAction * action, QIcon icon) {
	action->setIcon(icon);
}
					
void QtContactMenu::setPresenceIcon(QAction * action, EnumPresenceState::PresenceState presenceState) {

	switch (presenceState) {
	case EnumPresenceState::PresenceStateOnline:
		action->setIcon(QIcon(":/pics/status/online.png"));
		break;
	case EnumPresenceState::PresenceStateOffline:
		action->setIcon(QIcon(":/pics/status/offline.png"));
		break;
	case EnumPresenceState::PresenceStateInvisible:
		action->setIcon(QIcon(":/pics/status/invisible.png"));
		break;
	case EnumPresenceState::PresenceStateAway:
		action->setIcon(QIcon(":/pics/status/away.png"));
		break;
	case EnumPresenceState::PresenceStateDoNotDisturb:
		action->setIcon(QIcon(":/pics/status/donotdisturb.png"));
		break;
	case EnumPresenceState::PresenceStateUnknown:
		action->setIcon(QIcon(":/pics/status/unknown.png"));
		break;
	case EnumPresenceState::PresenceStateUnavailable:
		action->setIcon(QIcon(":/pics/status/unknown-wengo.png"));
		break;
	default:
		LOG_FATAL("unknown presenceState=" + String::fromNumber(presenceState));
		break;
	}
}

//VOXOX - JRT - 2009.07.27 - Not called.
//void QtContactMenu::populateWengoUsersContactId(QMenu * menu, CWengoPhone & cWengoPhone) {
//
//	CUserProfile * currentCUserProfile = cWengoPhone.getCUserProfileHandler().getCUserProfile();
//	if (currentCUserProfile) {
//
//		CContactList & currentCContactList = currentCUserProfile->getCContactList();
//		StringList currentContactsIds = currentCContactList.getContactIds();
//
//		for (StringList::const_iterator it = currentContactsIds.begin(); it != currentContactsIds.end(); ++it) {
//
//			ContactProfile tmpContactProfile = currentCContactList.getContactProfile(*it);
//			QString displayName = QString::fromUtf8(tmpContactProfile.getDisplayName().c_str());
//
//			if (tmpContactProfile.hasAvailableWengoId() &&
//				tmpContactProfile.getPresenceState() != EnumPresenceState::PresenceStateOffline && 
//				tmpContactProfile.getPresenceState() != EnumPresenceState::PresenceStateUnknown && 
//				tmpContactProfile.getPresenceState() != EnumPresenceState::PresenceStateUnavailable) {
//
//				QAction * tmpAction = menu->addAction(displayName);
//				tmpAction->setData(QVariant(QString::fromStdString(*it)));
//				EnumPresenceState::PresenceState presenceState = tmpContactProfile.getPresenceState();
//				setPresenceIcon(tmpAction, presenceState);
//			}
//		}
//	}
//}
