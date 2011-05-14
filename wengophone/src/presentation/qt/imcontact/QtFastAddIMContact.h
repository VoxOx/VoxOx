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

#ifndef OWQTFASTADDIMCONTACT_H
#define OWQTFASTADDIMCONTACT_H

#include <util/NonCopyable.h>
#include <QtGui/QDialog>
#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMAccountList.h>

#include <QtCore/QObject>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <set>

class IMAccount;
class ContactProfile;
class CUserProfile;

class QWidget;
class QDialog;
class QString;
class QVariant;
namespace Ui { class FastAddIMContact; }

/**
 * Window for adding an IM contact to a given contact.
 *
 * @author Tanguy Krotoff
 */
class QtFastAddIMContact : public QDialog{
	Q_OBJECT
public:

	QtFastAddIMContact(ContactProfile & contactProfile, CUserProfile & cUserProfile, QWidget * parent,QVariant & groupId,std::string & groupName);

	~QtFastAddIMContact();
/*
	QDialog * getWidget() const {
		return _addIMContactWindow;
	}*/
	int show();

private Q_SLOTS:

	void addIMContact();

	void addGroup();

	void imProtocolChanged(const QString & protocolName);

private:

	void loadIMAccounts(QtEnumIMProtocol::IMProtocol imProtocol);

	IMAccountList getSelectedIMAccounts(QtEnumIMProtocol::IMProtocol imProtocol) const;

	Ui::FastAddIMContact * _ui;

	/*QDialog * _addIMContactWindow;*/

	ContactProfile & _contactProfile;

	CUserProfile & _cUserProfile;

	QVariant & _groupId;

	std::string & _groupName;
};

#endif	//OWQtFastAddIMContact_H
