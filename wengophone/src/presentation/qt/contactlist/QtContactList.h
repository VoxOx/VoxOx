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

#ifndef OWQTCONTACTLIST_H
#define OWQTCONTACTLIST_H

#include <presentation/PContactList.h>

#include <QtCore/QObject>
#include <QtCore/QString>

class CContactList;
class CWengoPhone;

class Contact;
class ContactPopupMenu;
class ContactGroupPopupMenu;

class QtContactManager;
class QtWengoPhone;

class QTreeWidgetItem;

namespace Ui { class ContactList; }

/**
 * Qt Presentation component for ContactList.
 *
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class QtContactList : public QObject, public PContactList {
	Q_OBJECT
public:

	static const QString DEFAULT_GROUP_NAME;
	static const QString STATUS_UNKNOW_PIXMAP;
	static const QString STATUS_NOSTATUS_PIXMAP;
	static const QString STATUS_NOSTATUS_SIP_PIXMAP;
	static const QString STATUS_ONLINE_PIXMAP;
	static const QString STATUS_OFFLINE_PIXMAP;
	static const QString STATUS_DND_PIXMAP;
	static const QString STATUS_INVISIBLE_PIXMAP;
	static const QString STATUS_AWAY_PIXMAP;
	static const QString STATUS_WDEAL_PIXMAP;
	static const QString STATUS_TESTCALL_PIXMAP;
	static const QString STATUS_CONTACT_IM_PIXMAP;
	static const QString STATUS_CONTACT_CALL_PIXMAP;
	static const QString STATUS_CONTACT_VIDEO_PIXMAP;
	static const QString STATUS_GROUP_OPEN_PIXMAP;
	static const QString STATUS_GROUP_CLOSE_PIXMAP;

	static const QString STATUS_MSN_ONLINE_PIXMAP;
	static const QString STATUS_MSN_OFFLINE_PIXMAP;
	static const QString STATUS_MSN_DND_PIXMAP;
	static const QString STATUS_MSN_INVISIBLE_PIXMAP;
	static const QString STATUS_MSN_AWAY_PIXMAP;
	static const QString STATUS_MSN_BLOCK_PIXMAP;
	static const QString STATUS_MSN_MOBILE_PIXMAP;

	static const QString STATUS_MYSPACE_ONLINE_PIXMAP;
	static const QString STATUS_MYSPACE_OFFLINE_PIXMAP;
	static const QString STATUS_MYSPACE_DND_PIXMAP;
	static const QString STATUS_MYSPACE_INVISIBLE_PIXMAP;
	static const QString STATUS_MYSPACE_AWAY_PIXMAP;
	static const QString STATUS_MYSPACE_BLOCK_PIXMAP;
	static const QString STATUS_MYSPACE_MOBILE_PIXMAP;

	static const QString STATUS_FACEBOOK_ONLINE_PIXMAP;
	static const QString STATUS_FACEBOOK_OFFLINE_PIXMAP;
	static const QString STATUS_FACEBOOK_DND_PIXMAP;
	static const QString STATUS_FACEBOOK_INVISIBLE_PIXMAP;
	static const QString STATUS_FACEBOOK_AWAY_PIXMAP;
	static const QString STATUS_FACEBOOK_BLOCK_PIXMAP;
	static const QString STATUS_FACEBOOK_MOBILE_PIXMAP;

	static const QString STATUS_TWITTER_ONLINE_PIXMAP;
	static const QString STATUS_TWITTER_OFFLINE_PIXMAP;
	static const QString STATUS_TWITTER_DND_PIXMAP;
	static const QString STATUS_TWITTER_INVISIBLE_PIXMAP;
	static const QString STATUS_TWITTER_AWAY_PIXMAP;
	static const QString STATUS_TWITTER_BLOCK_PIXMAP;
	static const QString STATUS_TWITTER_MOBILE_PIXMAP;

	static const QString STATUS_SKYPE_ONLINE_PIXMAP;
	static const QString STATUS_SKYPE_OFFLINE_PIXMAP;
	static const QString STATUS_SKYPE_DND_PIXMAP;
	static const QString STATUS_SKYPE_INVISIBLE_PIXMAP;
	static const QString STATUS_SKYPE_AWAY_PIXMAP;
	static const QString STATUS_SKYPE_BLOCK_PIXMAP;
	static const QString STATUS_SKYPE_MOBILE_PIXMAP;

	static const QString STATUS_YAHOO_ONLINE_PIXMAP;
	static const QString STATUS_YAHOO_OFFLINE_PIXMAP;
	static const QString STATUS_YAHOO_DND_PIXMAP;
	static const QString STATUS_YAHOO_INVISIBLE_PIXMAP;
	static const QString STATUS_YAHOO_AWAY_PIXMAP;
	static const QString STATUS_YAHOO_BLOCK_PIXMAP;
	static const QString STATUS_YAHOO_MOBILE_PIXMAP;

	static const QString STATUS_JABBER_ONLINE_PIXMAP;
	static const QString STATUS_JABBER_OFFLINE_PIXMAP;
	static const QString STATUS_JABBER_DND_PIXMAP;
	static const QString STATUS_JABBER_INVISIBLE_PIXMAP;
	static const QString STATUS_JABBER_AWAY_PIXMAP;
	static const QString STATUS_JABBER_BLOCK_PIXMAP;
	static const QString STATUS_JABBER_MOBILE_PIXMAP;

	static const QString STATUS_AIM_ONLINE_PIXMAP;
	static const QString STATUS_AIM_OFFLINE_PIXMAP;
	static const QString STATUS_AIM_DND_PIXMAP;
	static const QString STATUS_AIM_INVISIBLE_PIXMAP;
	static const QString STATUS_AIM_AWAY_PIXMAP;
	static const QString STATUS_AIM_BLOCK_PIXMAP;
	static const QString STATUS_AIM_MOBILE_PIXMAP;

	static const QString STATUS_ICQ_ONLINE_PIXMAP;
	static const QString STATUS_ICQ_OFFLINE_PIXMAP;
	static const QString STATUS_ICQ_DND_PIXMAP;
	static const QString STATUS_ICQ_INVISIBLE_PIXMAP;
	static const QString STATUS_ICQ_AWAY_PIXMAP;
	static const QString STATUS_ICQ_BLOCK_PIXMAP;
	static const QString STATUS_ICQ_MOBILE_PIXMAP;

	static const QString STATUS_VOXOX_ONLINE_PIXMAP;
	static const QString STATUS_VOXOX_OFFLINE_PIXMAP;
	static const QString STATUS_VOXOX_DND_PIXMAP;
	static const QString STATUS_VOXOX_INVISIBLE_PIXMAP;
	static const QString STATUS_VOXOX_AWAY_PIXMAP;
	static const QString STATUS_VOXOX_BLOCK_PIXMAP;
	static const QString STATUS_VOXOX_MOBILE_PIXMAP;

	static const QString STATUS_GTALK_ONLINE_PIXMAP;
	static const QString STATUS_GTALK_OFFLINE_PIXMAP;
	static const QString STATUS_GTALK_DND_PIXMAP;
	static const QString STATUS_GTALK_INVISIBLE_PIXMAP;
	static const QString STATUS_GTALK_AWAY_PIXMAP;
	static const QString STATUS_GTALK_BLOCK_PIXMAP;
	static const QString STATUS_GTALK_MOBILE_PIXMAP;

	static const QString STATUS_MSN_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23 
	static const QString STATUS_MSN_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23 
	static const QString STATUS_MSN_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23 
	static const QString STATUS_MSN_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23 
	static const QString STATUS_MSN_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23 
	static const QString STATUS_MSN_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_MSN_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	static const QString STATUS_MYSPACE_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_MYSPACE_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_MYSPACE_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_MYSPACE_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_MYSPACE_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_MYSPACE_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_MYSPACE_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	static const QString STATUS_FACEBOOK_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_FACEBOOK_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_FACEBOOK_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_FACEBOOK_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_FACEBOOK_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_FACEBOOK_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_FACEBOOK_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	static const QString STATUS_TWITTER_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_TWITTER_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_TWITTER_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_TWITTER_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_TWITTER_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_TWITTER_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_TWITTER_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	static const QString STATUS_SKYPE_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_SKYPE_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_SKYPE_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_SKYPE_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_SKYPE_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_SKYPE_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_SKYPE_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	static const QString STATUS_YAHOO_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_YAHOO_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_YAHOO_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_YAHOO_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_YAHOO_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_YAHOO_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_YAHOO_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	static const QString STATUS_JABBER_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_JABBER_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_JABBER_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_JABBER_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_JABBER_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_JABBER_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_JABBER_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	static const QString STATUS_AIM_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_AIM_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_AIM_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_AIM_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_AIM_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_AIM_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_AIM_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	static const QString STATUS_ICQ_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_ICQ_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_ICQ_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_ICQ_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_ICQ_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_ICQ_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_ICQ_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	static const QString STATUS_VOXOX_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_VOXOX_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_VOXOX_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_VOXOX_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_VOXOX_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_VOXOX_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_VOXOX_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	static const QString STATUS_GTALK_ONLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_GTALK_OFFLINE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_GTALK_DND_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_GTALK_INVISIBLE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_GTALK_AWAY_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_GTALK_BLOCK_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23
	static const QString STATUS_GTALK_MOBILE_HOVER_PIXMAP;//VOXOX CHANGE by Rolando - 2009.10.23

	QtContactList(CContactList & cContactList, CWengoPhone & cWenghoPhone);

	virtual ~QtContactList();

	QWidget * getWidget() const {
		return _contactListWidget;
	}

	QtContactManager * getContactManager() const {
		return _contactManager;
	}

	void updatePresentation();

	//VOXOX - JRT - 2009.04.11 
	void setInitialLoadEvent(const int nOnOff);
	void setInitialLoad( bool bSet );										//VOXOX - JRT - 2009.04.07 
	bool getInitialLoad()		const			{ return _initialLoad;	}	//VOXOX - JRT - 2009.04.12 
	//End VoxOx

	void contactGroupAddedEvent  (const std::string & groupId);
	void contactGroupRemovedEvent(const std::string & groupId);
	void contactGroupRenamedEvent(const std::string & groupId);
	void contactGroupsReorderedEvent();				//VOXOX - JRT - 2009.05.11 

	void contactAddedEvent  (const std::string& contactId, const std::string& groupId );	//VOXOX - JRT - 2009.08.09 - Multiple group support
	void contactRemovedEvent(const std::string& contactId);
	void contactMovedEvent  (const std::string& dstGroupId, const std::string& srcGroupId, const std::string & contactId);
	void contactChangedEvent(const std::string& contactId,  const std::string& groupId );	//VOXOX - JRT - 2009.08.09 - Multiple group support

	CContactList & getCContactList() const;


Q_SIGNALS:

	void contactChangedEventSignal(QString contactId);

private Q_SLOTS:

	void groupRightClickedSlot(const QString & groupName);

	void mergeContactsSlot( QString dstContact,   QString srcContact   );
	void orderGroupsSlot  ( QString dragGroupKey, QString dropGroupKey );	//VOXOX - JRT - 2009.05.11 

private:

	/**
	 * Initializes the Content of the QtContactList.
	 */
	void initContent();
	void		  initContactPixmap();
	QtWengoPhone* getQtWengoPhone();	//VOXOX - JRT - 2009.09.23 
	void		  notifyJsContactManagerContactAdded( const std::string& contactId, int qtId );	//VOXOX - JRT - 2009.09.23 

	void addGroup(QString contactGroupId);

	QtContactManager * _contactManager;

	CContactList & _cContactList;

	Ui::ContactList * _ui;

	QWidget * _contactListWidget;

	ContactPopupMenu * _contactPopupMenu;

	ContactGroupPopupMenu * _contactGroupPopupMenu;

	/** True when model is doing some things on contacts. */
	bool _waitingForModel;

	CWengoPhone & _cWengoPhone;

	bool			_initialLoad;	//VOXOX - JRT - 2009.04.12 
};

#endif	//OWQTCONTACTLIST_H

