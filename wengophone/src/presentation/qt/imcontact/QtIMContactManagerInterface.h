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

#ifndef OWQTIMCONTACTMANAGERINTERFACE_H
#define OWQTIMCONTACTMANAGERINTERFACE_H

#include <util/NonCopyable.h>

#include <QtCore/QObject>

class ContactProfile;
class CUserProfile;

class QWidget;

/**
 * Interface for classes handling IM accounts of a contact (IMContact).
 *
 * @see Contact
 * @see IMContact
 * @see QtSimpleIMContactManager
 * @see QtAdvancedIMContactManager
 *
 * @author Xavier Desjardins
 */
class QtIMContactManagerInterface : public QObject, NonCopyable {
	Q_OBJECT
public:

	QtIMContactManagerInterface(ContactProfile & contactProfile,
		CUserProfile & cUserProfile, QWidget * parent);

	virtual ~QtIMContactManagerInterface();

	QWidget * getWidget() const ;	
	
	/**
	 *	Is there at least one valid contact to be saved
	 */
	virtual bool couldBeSaved() = 0;
	
	/**
	 *	save IMContacts
	 */
	virtual void saveIMContacts() = 0;
	
	enum contactManagerMode {
		contactManagerModeUndefined,
		contactManagerModeSimple,
		contactManagerModeAdvanced
	};
	
	/**
	 * @return contactManagerMode
	 */
	 virtual QtIMContactManagerInterface::contactManagerMode getContactManagerMode();
	 
	virtual void setWengoName(const QString & wengoName) = 0;

	virtual void setSipAddress(const QString & sipAddress) = 0;

Q_SIGNALS:

	/** 
	 * something has changed
	 */
	void contentHasChanged();
	
protected:

	QWidget * _imContactManagerWidget;

	ContactProfile & _contactProfile;

	CUserProfile & _cUserProfile;
};

#endif //OWQTIMCONTACTMANAGERINTERFACE_H
