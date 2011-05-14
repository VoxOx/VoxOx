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

#ifndef OWQTCONTACTMANAGER_H
#define OWQTCONTACTMANAGER_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QMetaType>
	
#include <thread/RecursiveMutex.h>

#include <util/VoxElapsedTimer.h>	//VOXOX - JRT - 2009.10.14 

#include "VoxContactTreeWidget.h"	//VOXOX - JRT - 2009.05.15 

class ContactProfile;
class Contact;
class ContactGroup;
class ContactGroupInfo;
class CUserProfile;
class CWengoPhone;
class PhoneCall;

class QtContactList;
class QtContactInfo;
class QtContact;

class QString;
class QTreeWidgetItem;
class QTimerEvent;
class QMenu;
class QTreeWidget;
class QSize;
class QAction;

Q_DECLARE_METATYPE(QtContact*)	//VOXOX - JRT - 2009.04.15 

/**
 * Qt Presentation manager for the ContactList.
 *
 * @author Philippe Bernery
 */
class QtContactManager : public QObject 
{
	Q_OBJECT
public:
	QtContactManager(CUserProfile & cUserProfile, CWengoPhone & cWengoPhone, QtContactList & qtContactList, QObject * parent, QTreeWidget * target);

	/**
	 * Removes a Contact from the QtContactManager.
	 *
	 * @param contactId the Id of the Contact to remove
	 */
	void removeContact(const QString & contactId); 

	/**
	 * Moves a Contact from one group to another.
	 *
	 * @param dstGroupId the Id of the destination group
	 * @param srcGroupId the Id of the source group
	 * @param contactId the Id of the Contact to move
	 */
//	void moveContact(const std::string & dstGroupId, const std::string & srcGroupId, const std::string & contactId);	//VOXOX - JRT - 2009.08.10 - Not needed after implementing Multiple Groups.

	/**
	 * @return true if groups are hidden (not displayed)
	 */
	bool groupsAreHidden() const;

	void handleContactChanged(const std::string& contactId, const std::string& groupId, bool bMoved );
	void setInitialLoad( bool bSet );	//VOXOX - JRT - 2009.04.07 - To improve initial data load.

	void deleteCurrentContact();
	void editCurrentContact();


public Q_SLOTS:
	void userStateChanged();
	void editContact(QString contactId);

private Q_SLOTS:

	QString addNumber();
	QString addEmail();
	QString addMobileNumber();
	QString addFaxNumber();

	void sendEmailContact(QAction * action);
	void sendSMSContact(QAction * action);
	void sendFaxContact(QAction * action);

	void openSocialPageContact();
	void sendFileContact();
	void editProfileContact();

	void defaultAction(QTreeWidgetItem * item);

	void deleteContact();
	void editContact();

	void blockContact();
	void unblockContact();


	void reRequestAuthorization();//VOXOX - CJC - 2009.07.31 

	void inviteToConference();
	void callNumber(QAction * action);


	//VOXOX - JRT - 2009.05.15 
//	void itemClicked(QTreeWidgetItem * item, int column);
	void itemLeftClicked  ( QTreeWidgetItem* item, int column );
	void itemRightClicked ( QTreeWidgetItem* item, int column );
	void itemDoubleClicked( QTreeWidgetItem* item, int column );

	/**
	 * Opens the contact panel inside the contact list.
	 */
	void openContactInfo(QTreeWidgetItem * item);

	/**
	 * Closes the contact panel inside the contact list.
	 */
	void closeContactInfo();

	void redrawContacts();

	void sortContacts(bool bypassTimer = false);

	void startChat();

	void sendFile();

	void startHomeCall();

	void startMobileCall();

	void startSMS();

	void startWengoCall();

	void startWorkCall();

	void goToWebsite();

//	void treeViewSelectionChanged();	//VOXOX - JRT - 2009.05.20 

	void languageChanged();

Q_SIGNALS:

	void groupRightClicked(const QString & groupName);

	void inviteToConferenceClicked(QString phone, PhoneCall * target);

private:
//	void	lock()											{ _mutex.lock();	}	//VOXOX - JRT - 2009.06.24 
//	void	unlock()										{ _mutex.unlock();	}	//VOXOX - JRT - 2009.06.24 

	bool	isGroupOpen( const QString& contactGroupId );
	void	setGroupOpen( const QString& groupId, bool bOpen );
	QString getGroupName( const QString& contactGroupId );

	typedef QList <QtContactInfo> QtContactInfoList;

	void safeSortContacts(bool bypassTimer);

	void safeUserStateChanged();

	void clearTreeSelection();

	void groupClicked(QTreeWidgetItem * item);

	void itemRightClicked(QTreeWidgetItem * item);

	bool canShowUser(const std::string& contactId);

	QTreeWidgetItem * findGroupItem(const QString & groupId) const;

	QTreeWidgetItem * findContactItem(const QString & contactId) const;

//	QtContact * findContactInTreeByKey( const QString & contactKey) const;

	QTreeWidgetItem* createGroupItem(const QString & contactGroupId, bool isFilterGroup );

	void addContact   ( QtContact* qtContact );	
	void addContact   ( QtContact* qtContact, const std::string& groupId, bool isFilterGroup );
	void updateContact( QtContact* qtContact ) ;
	void removeContact( QTreeWidgetItem* item );

	QMenu * createConferenceMenu();

	QMenu * createMenu();
	QMenu * createCallMenu();
	QMenu * createEmailMenu();
	QMenu * createSMSMenu();
	QMenu * createFaxMenu();

	void timerEvent(QTimerEvent * event);

	Contact*	getCurrentContact();
	Contact*	getContactById( const std::string contactId );
	
	std::string getCurrentItemContactKey();
	std::string getCurrentItemContactId();
	std::string getCurrentItemMobilePhone();
	std::string getCurrentItemHomePhone();
	std::string getCurrentItemWorkPhone();
	std::string getCurrentItemWengoPhoneNumber();
	std::string getCurrentItemWebsite();

	void		makeCall( const std::string& number );
	int			findInsertPoint( QTreeWidgetItem* groupItem, QtContact* qtContact );
	QtContact*	getQtContactFromItem( QTreeWidgetItem* item ) const;
	void		setItemData ( QTreeWidgetItem* item, QtContact* qtContact );
	void		clear();

	bool		shouldSortContacts(bool bypassTimer) ;
	void		sortChildren( QTreeWidgetItem* parentItem, bool bTopLevel );
	void		restoreGroupExpandedStatus();

	bool		shouldUseGroup( const ContactGroupInfo& cgi, const std::string& groupKey, bool& isFilter );
	bool		shouldUseGroup( const ContactGroup& cg, bool& isFilter );	//VOXOX - JRT - 2009.08.21 

	void		setNeedsRefresh();			//VOXOX - JRT - 2009.10.14 
	void		setNeedsReload();			//VOXOX - JRT - 2009.10.14 
	bool		showTimerOk() ;				//VOXOX - JRT - 2009.10.14 
//	bool		sortTimerOk() ;				//VOXOX - JRT - 2009.10.14 
	void		restartSortTimer();			//VOXOX - JRT - 2009.10.14 
	bool		isDrawing()					{ return _isDrawing;	}	//VOXOX - JRT - 2009.10.14 

	bool		saveTopQtContact();			//VOXOX - JRT - 2009.10.15 
	void		restoreTopQtContact();


	std::string determineUseableGroupId( const std::string& contactId, const std::string& groupIdIn );

	VoxContactListTreeWidget* _tree;
	QSize*				_itemSize;
	QMenu*				_menu;
	QAction*			_callAction;

	bool				_sortContacts;
	int					_sortTimerId;
	bool				_canSort;
	bool				_wantSort;

	int					_showTimerId;
	bool				_canShow;
	bool				_wantShow;

//	int					_refreshTimerId;	//VOXOX - JRT - 2009.10.14 
	bool				_needsRefresh;
	bool				_needsReload;
	bool				_isDrawing;

	QtContact*			_topQtContact;


	bool				_initialLoad;

	CWengoPhone&		_cWengoPhone;
	CUserProfile&		_cUserProfile;
	QtContactList&		_qtContactList;

	mutable RecursiveMutex _mutex;

	VoxElapsedTimer	_refreshTimer;			//VOXOX - JRT - 2009.10.14 
	static int		_refreshInterval;

	VoxElapsedTimer	_reloadTimer;			//VOXOX - JRT - 2009.10.14 
	static int		_reloadInterval;


	/**
	 * Translated strings
	 */
	QString _trStringCallContact;
	QString _trStringSendEmail;
	QString _trStringSendSMS;
	QString _trStringSendFax;
	QString _trStringSendIM;
	QString _trStringSocialProfile;
	QString _trStringSendFile;
	QString _trStringEditContact;
	QString _trStringDeleteContact;
	QString _trStringBlockContact;
	QString _trStringRERequestAuthorizationContact;//VOXOX - CJC - 2009.07.30 
	QString _trStringUnblockContact;
	QString _trStringInviteToConference;
};

#endif	//OWQTCONTACTMANAGER_H
