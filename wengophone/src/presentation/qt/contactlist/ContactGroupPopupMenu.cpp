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
#include "ContactGroupPopupMenu.h"
#include "QtRenameGroup.h"

#include <control/contactlist/CContactList.h>

#include <model/contactlist/ContactGroup.h>

#include <util/Logger.h>
#include <util/StringList.h>

#include <QtGui/QtGui>

#include <presentation/qt/messagebox/QtVoxMessageBox.h>		//VOXOX - CJC - 2009.06.10 
#include <iostream>

using namespace std;

ContactGroupPopupMenu::ContactGroupPopupMenu(CContactList & cContactList, QWidget * parent)
	: QMenu(parent),
	_cContactList(cContactList) 
{

	addAction(tr("Remove Contact Group"), this, SLOT(removeContactGroup()));
	addAction(tr("Rename Contact Group"), this, SLOT(renameContactGroup()));
	//addAction(tr("Send SMS to group"), this, SLOT(sendSms()));
}

void ContactGroupPopupMenu::showMenu(const QString & groupId) 
{
	_groupId = groupId;

	//VOXOX - JRT - 2009.09.01 - Only show menu for User groups.
	ContactGroup* group = _cContactList.getContactGroupSet().FindByKey( groupId.toStdString() );

	if ( group && group->isUser() )
	{
		exec(QCursor::pos());
	}
}

void ContactGroupPopupMenu::removeContactGroup() const 
{
	//VOXOX - JRT - 2009.09.03 - Add prompt about contacts that will be moved.
	int         willBeMoved = _cContactList.getHowManyContactsMustMove( _groupId.toStdString() );
	std::string thisGroup   = _cContactList.getContactGroupName( _groupId.toStdString() );

	QString msg = "";

	if ( willBeMoved > 0 )
	{
		std::string newGroup = _cContactList.getContactList().geDefaultMoveToGroupName();

		std::string temp = "There are " + String::fromNumber( willBeMoved ) + " contacts that will be moved to '" + newGroup + "'.";

		msg += QString(temp.c_str());
		msg += "\n\n";
	}

//	msg += tr("Do you really want to delete this group?");
	msg += "Do you really want to delete group '" + QString(thisGroup.c_str()) + "'?";

	//VOXOX - CJC - 2009.06.10 
	QtVoxMessageBox box(qobject_cast<QWidget *>(parent()));

	box.setWindowTitle("VoxOx - Delete group");
	box.setText( msg );
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No );

	if(box.exec() == QMessageBox::Yes)
	{
		_cContactList.removeContactGroup(_groupId.toStdString());
	}
}

void ContactGroupPopupMenu::renameContactGroup() const 
{
	std::string		tmp		  = _cContactList.getContactGroupName(_groupId.toStdString());
	QString			groupName = QString::fromUtf8(tmp.c_str());
	QtRenameGroup	dialog(groupName, qobject_cast<QWidget *>(parent()));

	if (dialog.exec()) 
	{
		_cContactList.renameContactGroup(_groupId.toStdString(), dialog.getGroupName().toUtf8().data());
	}
}

//void ContactGroupPopupMenu::sendSms() {	//VOXOX - JRT - 2009.09.01 - empty method
/*
	StringList list = _contactGroup->getMobilePhoneList();
	for(unsigned int i = 0; i < list.size(); i++) {
		LOG_DEBUG("mobile number: " + list[i] + "\n");
	}
*/
//}
