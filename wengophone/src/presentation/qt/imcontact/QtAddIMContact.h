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

#ifndef OWQTADDIMCONTACT_H
#define OWQTADDIMCONTACT_H

#include <util/NonCopyable.h>

#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMAccountList.h>

#include <QtCore/QObject>

#include <set>

class IMAccount;
class ContactProfile;
class CUserProfile;

class QWidget;
class QDialog;
class QString;
namespace Ui { class AddIMContact; }

/**
 * Window for adding an IM contact to a given contact.
 *
 * @author Tanguy Krotoff
 */
class QtAddIMContact : public QObject, NonCopyable {
	Q_OBJECT
public:

	QtAddIMContact(ContactProfile & contactProfile, CUserProfile & cUserProfile, QWidget * parent);

	~QtAddIMContact();

	QDialog * getWidget() const {
		return _addIMContactWindow;
	}

private Q_SLOTS:

	void addIMContact();

	void imProtocolChanged(const QString & protocolName);

private:

	int show();

	void loadIMAccounts(EnumIMProtocol::IMProtocol imProtocol);

	IMAccountList getSelectedIMAccounts(EnumIMProtocol::IMProtocol imProtocol) const;

	Ui::AddIMContact * _ui;

	QDialog * _addIMContactWindow;

	ContactProfile & _contactProfile;

	CUserProfile & _cUserProfile;
};

#endif	//OWQTADDIMCONTACT_H
