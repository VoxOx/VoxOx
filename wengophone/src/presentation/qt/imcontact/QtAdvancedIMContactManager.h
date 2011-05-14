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

#ifndef OWQTADVANCEDIMCONTACTMANAGER_H
#define OWQTADVANCEDIMCONTACTMANAGER_H

#include <presentation/qt/imcontact/QtIMContactManagerInterface.h>

class ContactProfile;
class CUserProfile;

class QWidget;
class QTreeWidget;
class QTreeWidgetItem;
namespace Ui { class IMContactManager; }

/**
 * Window that handles IM accounts of a contact (IMContact).
 *
 * Very similar to QtIMAccountManager but for IM contacts.
 *
 * @see QtIMAccountManager
 * @see Contact
 * @see IMContact
 * @see QtSimpleIMContactManager
 * @see QtIMContactManagerInterface
 *
 * @author Tanguy Krotoff
 * @Xavier Desjardins
 */
class QtAdvancedIMContactManager : public QtIMContactManagerInterface {
	Q_OBJECT
public:

	QtAdvancedIMContactManager(ContactProfile & contactProfile,
		CUserProfile & cUserProfile, QWidget * parent);

	virtual ~QtAdvancedIMContactManager();
	
	virtual bool couldBeSaved();

	virtual void saveIMContacts();
	
	virtual QtIMContactManagerInterface::contactManagerMode getContactManagerMode() {
		return QtIMContactManagerInterface::contactManagerModeAdvanced;
	}
	
	virtual void setWengoName(const QString & wengoName) {
		//nothing to do...
	}

	virtual void setSipAddress(const QString & sipAddress) {
		//nothing to do...
	}


private Q_SLOTS:

	void addIMContact();

	void deleteIMContact();

	void currentItemChangedSlot(QTreeWidgetItem * current, QTreeWidgetItem * previous);
	
private:

	void loadIMContacts();

	Ui::IMContactManager * _ui;
};

#endif //OWQTADVANCEDIMCONTACTMANAGER_H
