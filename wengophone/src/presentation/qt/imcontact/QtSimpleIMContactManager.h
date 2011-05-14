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

#ifndef OWQTSIMPLEIMCONTACTMANAGER_H
#define OWQTSIMPLEIMCONTACTMANAGER_H

#include <presentation/qt/imcontact/QtIMContactManagerInterface.h>

#include <imwrapper/EnumIMProtocol.h>

#include <set>

class IMContact;
class ContactProfile;
class CUserProfile;
class MousePressEventFilter;

class QLineEdit;
class QLabel;
class QWidget;
namespace Ui { class SimpleIMContactManager; }

/**
 * Simplified widget for adding an IM contact to a given contact.
 *
 * @see QtIMContactManager
 * @see QtSimpleIMAccountManager
 *
 * @author Tanguy Krotoff
 * @author Xavier Desjardins
 */
class QtSimpleIMContactManager : public QtIMContactManagerInterface {
	Q_OBJECT
public:

	QtSimpleIMContactManager(ContactProfile & contactProfile,
		CUserProfile & cUserProfile, QWidget * parent);

	~QtSimpleIMContactManager();
	
	virtual bool couldBeSaved();

	virtual void saveIMContacts();
	
	virtual QtIMContactManagerInterface::contactManagerMode getContactManagerMode() {
		return QtIMContactManagerInterface::contactManagerModeSimple;
	}

	virtual void setWengoName(const QString & wengoName);

	virtual void setSipAddress(const QString & sipAddress);

private Q_SLOTS:

	void msnLineEditClicked();

	void aimLineEditClicked();

	void icqLineEditClicked();

	void yahooLineEditClicked();

	void jabberLineEditClicked();

private:

	void loadIMContacts();

	/**
	 * Code factorization.
	 */
	std::set<IMContact *> findIMContactsOfProtocol(EnumIMProtocol::IMProtocol imProtocol) const;

	/**
	 * Code factorization.
	 */
	QString getIMContactsOfProtocol(EnumIMProtocol::IMProtocol imProtocol) const;

	/**
	 * Code factorization.
	 */
	void addIMContactsOfProtocol(EnumIMProtocol::IMProtocol imProtocol, const QString & text);

	/**
	 * Code factorization.
	 */
	void changeIMProtocolPixmaps(EnumIMProtocol::IMProtocol imProtocol,
		QLabel * imProtocolLabel, const char * connectedPixmap, QLineEdit * imProtocolLineEdit);

	Ui::SimpleIMContactManager * _ui;

	MousePressEventFilter * _msnLineEditMouseEventFilter;

	MousePressEventFilter * _aimLineEditMouseEventFilter;

	MousePressEventFilter * _icqLineEditMouseEventFilter;

	MousePressEventFilter * _yahooLineEditMouseEventFilter;

	MousePressEventFilter * _jabberLineEditMouseEventFilter;
};

#endif	//OWQTSIMPLEIMCONTACTMANAGER_H
