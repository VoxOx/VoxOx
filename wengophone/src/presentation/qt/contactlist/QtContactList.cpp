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
#include "QtContactList.h"

#include "ui_ContactList.h"

#include "ContactGroupPopupMenu.h"
#include "QtContactListManager.h"
#include "QtContactManager.h"
#include "QtContactPixmap.h"
//#include "QtContactTreeMouseFilter.h"		//VOXOX - JRT - 2009.08.09 - Not used
#include "QtTreeViewDelegate.h"				//VOXOX - JRT - 2009.10.15 
#include "QtContactListStyle.h"

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/contactlist/QtEnumSortOption.h>	//VOXOX - JRT - 2009.05.11 

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/contactlist/CContactList.h>

#include <model/contactlist/Contact.h>
#include <model/contactlist/ContactGroup.h>
#include <model/config/ConfigManager.h>		//VOXOX - JRT - 2009.05.11 - To auto set the Group Sort.
#include <model/config/Config.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/StringList.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>

const QString QtContactList::DEFAULT_GROUP_NAME = "VOXOXLIST";

//TODO: these should come from QtEnum... class
const QString QtContactList::STATUS_NOSTATUS_PIXMAP			= ":/pics/status/unknown-wengo.png";
const QString QtContactList::STATUS_NOSTATUS_SIP_PIXMAP		= ":/pics/status/unknown-sip.png";
const QString QtContactList::STATUS_UNKNOW_PIXMAP			= ":/pics/status/unknown.png";
const QString QtContactList::STATUS_ONLINE_PIXMAP			= ":/pics/status/online.png";
const QString QtContactList::STATUS_OFFLINE_PIXMAP			= ":/pics/status/offline.png";
const QString QtContactList::STATUS_DND_PIXMAP				= ":/pics/status/donotdisturb.png";
const QString QtContactList::STATUS_INVISIBLE_PIXMAP		= ":/pics/status/invisible.png";
const QString QtContactList::STATUS_AWAY_PIXMAP				= ":/pics/status/away.png";
const QString QtContactList::STATUS_WDEAL_PIXMAP			= ":/pics/status/wdeal.png";
const QString QtContactList::STATUS_TESTCALL_PIXMAP			= ":/pics/status/testcall.png";

const QString QtContactList::STATUS_GROUP_OPEN_PIXMAP		= ":/pics/group_open.png";
const QString QtContactList::STATUS_GROUP_CLOSE_PIXMAP		= ":/pics/group_close.png";

const QString QtContactList::STATUS_MSN_ONLINE_PIXMAP		= ":/pics/status/networks/msn_online.png";
const QString QtContactList::STATUS_MSN_OFFLINE_PIXMAP		= ":/pics/status/networks/msn_offline.png";
const QString QtContactList::STATUS_MSN_DND_PIXMAP			= ":/pics/status/networks/msn_donotdisturb.png";
const QString QtContactList::STATUS_MSN_INVISIBLE_PIXMAP	= ":/pics/status/networks/msn_offline.png";
const QString QtContactList::STATUS_MSN_AWAY_PIXMAP			= ":pics/status/networks/msn_away.png";
const QString QtContactList::STATUS_MSN_BLOCK_PIXMAP		= ":pics/status/networks/msn_block.png";
const QString QtContactList::STATUS_MSN_MOBILE_PIXMAP		= ":pics/status/networks/msn_mobile.png";

const QString QtContactList::STATUS_MYSPACE_ONLINE_PIXMAP	= ":/pics/status/networks/myspaceim_online.png";
const QString QtContactList::STATUS_MYSPACE_OFFLINE_PIXMAP	= ":/pics/status/networks/myspaceim_offline.png";
const QString QtContactList::STATUS_MYSPACE_DND_PIXMAP		= ":/pics/status/networks/myspaceim_donotdisturb.png";
const QString QtContactList::STATUS_MYSPACE_INVISIBLE_PIXMAP = ":/pics/status/networks/myspaceim_offline.png";
const QString QtContactList::STATUS_MYSPACE_AWAY_PIXMAP		= ":pics/status/networks/myspaceim_away.png";
const QString QtContactList::STATUS_MYSPACE_BLOCK_PIXMAP	= ":pics/status/networks/myspaceim_block.png";
const QString QtContactList::STATUS_MYSPACE_MOBILE_PIXMAP	= ":pics/status/networks/myspaceim_mobile.png";

const QString QtContactList::STATUS_FACEBOOK_ONLINE_PIXMAP	= ":/pics/status/networks/facebook_online.png";
const QString QtContactList::STATUS_FACEBOOK_OFFLINE_PIXMAP = ":/pics/status/networks/facebook_offline.png";
const QString QtContactList::STATUS_FACEBOOK_DND_PIXMAP		= ":/pics/status/networks/facebook_donotdisturb.png";
const QString QtContactList::STATUS_FACEBOOK_INVISIBLE_PIXMAP = ":/pics/status/networks/facebook_offline.png";
const QString QtContactList::STATUS_FACEBOOK_AWAY_PIXMAP	= ":/pics/status/networks/facebook_away.png";
const QString QtContactList::STATUS_FACEBOOK_BLOCK_PIXMAP	= ":/pics/status/networks/facebook_block.png";
const QString QtContactList::STATUS_FACEBOOK_MOBILE_PIXMAP	= ":/pics/status/networks/facebook_mobile.png";

const QString QtContactList::STATUS_TWITTER_ONLINE_PIXMAP	= ":/pics/status/networks/twitter_online.png";
const QString QtContactList::STATUS_TWITTER_OFFLINE_PIXMAP	= ":/pics/status/networks/twitter_offline.png";
const QString QtContactList::STATUS_TWITTER_DND_PIXMAP		= ":/pics/status/networks/twitter_donotdisturb.png";
const QString QtContactList::STATUS_TWITTER_INVISIBLE_PIXMAP = ":/pics/status/networks/twitter_offline.png";
const QString QtContactList::STATUS_TWITTER_AWAY_PIXMAP		= ":/pics/status/networks/twitter_away.png";
const QString QtContactList::STATUS_TWITTER_BLOCK_PIXMAP	= ":/pics/status/networks/twitter_block.png";
const QString QtContactList::STATUS_TWITTER_MOBILE_PIXMAP	= ":/pics/status/networks/twitter_mobile.png";

const QString QtContactList::STATUS_SKYPE_ONLINE_PIXMAP		= ":/pics/status/networks/skype_online.png";
const QString QtContactList::STATUS_SKYPE_OFFLINE_PIXMAP	= ":/pics/status/networks/skype_offline.png";
const QString QtContactList::STATUS_SKYPE_DND_PIXMAP		= ":/pics/status/networks/skype_donotdisturb.png";
const QString QtContactList::STATUS_SKYPE_INVISIBLE_PIXMAP	= ":/pics/status/networks/skype_offline.png";
const QString QtContactList::STATUS_SKYPE_AWAY_PIXMAP		= ":/pics/status/networks/skype_away.png";
const QString QtContactList::STATUS_SKYPE_BLOCK_PIXMAP		= ":/pics/status/networks/skype_block.png";
const QString QtContactList::STATUS_SKYPE_MOBILE_PIXMAP		= ":/pics/status/networks/skype_mobile.png";

const QString QtContactList::STATUS_YAHOO_ONLINE_PIXMAP		= ":/pics/status/networks/yahoo_online.png";
const QString QtContactList::STATUS_YAHOO_OFFLINE_PIXMAP	= ":/pics/status/networks/yahoo_offline.png";
const QString QtContactList::STATUS_YAHOO_DND_PIXMAP		= ":/pics/status/networks/yahoo_donotdisturb.png";
const QString QtContactList::STATUS_YAHOO_INVISIBLE_PIXMAP	= ":/pics/status/networks/yahoo_offline.png";
const QString QtContactList::STATUS_YAHOO_AWAY_PIXMAP		= ":/pics/status/networks/yahoo_away.png";
const QString QtContactList::STATUS_YAHOO_BLOCK_PIXMAP		= ":/pics/status/networks/yahoo_block.png";
const QString QtContactList::STATUS_YAHOO_MOBILE_PIXMAP		= ":/pics/status/networks/yahoo_mobile.png";

const QString QtContactList::STATUS_JABBER_ONLINE_PIXMAP	= ":/pics/status/networks/jabber_online.png";
const QString QtContactList::STATUS_JABBER_OFFLINE_PIXMAP	= ":/pics/status/networks/jabber_offline.png";
const QString QtContactList::STATUS_JABBER_DND_PIXMAP		= ":/pics/status/networks/jabber_donotdisturb.png";
const QString QtContactList::STATUS_JABBER_INVISIBLE_PIXMAP = ":/pics/status/networks/jabber_offline.png";
const QString QtContactList::STATUS_JABBER_AWAY_PIXMAP		= ":/pics/status/networks/jabber_away.png";
const QString QtContactList::STATUS_JABBER_BLOCK_PIXMAP		= ":/pics/status/networks/jabber_block.png";
const QString QtContactList::STATUS_JABBER_MOBILE_PIXMAP	= ":/pics/status/networks/jabber_mobile.png";

const QString QtContactList::STATUS_AIM_ONLINE_PIXMAP		= ":/pics/status/networks/aim_online.png";
const QString QtContactList::STATUS_AIM_OFFLINE_PIXMAP		= ":/pics/status/networks/aim_offline.png";
const QString QtContactList::STATUS_AIM_DND_PIXMAP			= ":/pics/status/networks/aim_donotdisturb.png";
const QString QtContactList::STATUS_AIM_INVISIBLE_PIXMAP	= ":/pics/status/networks/aim_offline.png";
const QString QtContactList::STATUS_AIM_AWAY_PIXMAP			= ":/pics/status/networks/aim_away.png";
const QString QtContactList::STATUS_AIM_BLOCK_PIXMAP		= ":/pics/status/networks/aim_block.png";
const QString QtContactList::STATUS_AIM_MOBILE_PIXMAP		= ":/pics/status/networks/aim_mobile.png";

const QString QtContactList::STATUS_ICQ_ONLINE_PIXMAP		= ":/pics/status/networks/icq_online.png";
const QString QtContactList::STATUS_ICQ_OFFLINE_PIXMAP		= ":/pics/status/networks/icq_offline.png";
const QString QtContactList::STATUS_ICQ_DND_PIXMAP			= ":/pics/status/networks/icq_donotdisturb.png";
const QString QtContactList::STATUS_ICQ_INVISIBLE_PIXMAP	= ":/pics/status/networks/icq_offline.png";
const QString QtContactList::STATUS_ICQ_AWAY_PIXMAP			= ":/pics/status/networks/icq_away.png";
const QString QtContactList::STATUS_ICQ_BLOCK_PIXMAP		= ":/pics/status/networks/icq_block.png";
const QString QtContactList::STATUS_ICQ_MOBILE_PIXMAP		= ":/pics/status/networks/icq_mobile.png";

const QString QtContactList::STATUS_VOXOX_ONLINE_PIXMAP		= ":/pics/status/networks/voxox_online.png";
const QString QtContactList::STATUS_VOXOX_OFFLINE_PIXMAP	= ":/pics/status/networks/voxox_offline.png";
const QString QtContactList::STATUS_VOXOX_DND_PIXMAP		= ":/pics/status/networks/voxox_donotdisturb.png";
const QString QtContactList::STATUS_VOXOX_INVISIBLE_PIXMAP	= ":/pics/status/networks/voxox_offline.png";
const QString QtContactList::STATUS_VOXOX_AWAY_PIXMAP		= ":/pics/status/networks/voxox_away.png";
const QString QtContactList::STATUS_VOXOX_BLOCK_PIXMAP		= ":/pics/status/networks/voxox_block.png";
const QString QtContactList::STATUS_VOXOX_MOBILE_PIXMAP		= ":/pics/status/networks/voxox_mobile.png";

const QString QtContactList::STATUS_GTALK_ONLINE_PIXMAP		= ":/pics/status/networks/googletalk_online.png";
const QString QtContactList::STATUS_GTALK_OFFLINE_PIXMAP	= ":/pics/status/networks/googletalk_offline.png";
const QString QtContactList::STATUS_GTALK_DND_PIXMAP		= ":/pics/status/networks/googletalk_donotdisturb.png";
const QString QtContactList::STATUS_GTALK_INVISIBLE_PIXMAP	= ":/pics/status/networks/googletalk_offline.png";
const QString QtContactList::STATUS_GTALK_AWAY_PIXMAP		= ":/pics/status/networks/googletalk_away.png";
const QString QtContactList::STATUS_GTALK_BLOCK_PIXMAP		= ":/pics/status/networks/googletalk_block.png";
const QString QtContactList::STATUS_GTALK_MOBILE_PIXMAP		= ":/pics/status/networks/googletalk_mobile.png";

//VOXOX CHANGE by Rolando - 2009.10.22 
const QString QtContactList::STATUS_MSN_ONLINE_HOVER_PIXMAP		= ":/pics/status/networks/msn_online_hover.png";
const QString QtContactList::STATUS_MSN_OFFLINE_HOVER_PIXMAP		= ":/pics/status/networks/msn_offline_hover.png";
const QString QtContactList::STATUS_MSN_DND_HOVER_PIXMAP			= ":/pics/status/networks/msn_donotdisturb_hover.png";
const QString QtContactList::STATUS_MSN_INVISIBLE_HOVER_PIXMAP	= ":/pics/status/networks/msn_offline_hover.png";
const QString QtContactList::STATUS_MSN_AWAY_HOVER_PIXMAP			= ":pics/status/networks/msn_away_hover.png";
const QString QtContactList::STATUS_MSN_BLOCK_HOVER_PIXMAP		= ":pics/status/networks/msn_block_hover.png";
const QString QtContactList::STATUS_MSN_MOBILE_HOVER_PIXMAP		= ":pics/status/networks/msn_mobile_hover.png";

const QString QtContactList::STATUS_MYSPACE_ONLINE_HOVER_PIXMAP	= ":/pics/status/networks/myspaceim_online_hover.png";
const QString QtContactList::STATUS_MYSPACE_OFFLINE_HOVER_PIXMAP	= ":/pics/status/networks/myspaceim_offline_hover.png";
const QString QtContactList::STATUS_MYSPACE_DND_HOVER_PIXMAP		= ":/pics/status/networks/myspaceim_donotdisturb_hover.png";
const QString QtContactList::STATUS_MYSPACE_INVISIBLE_HOVER_PIXMAP = ":/pics/status/networks/myspaceim_offline_hover.png";
const QString QtContactList::STATUS_MYSPACE_AWAY_HOVER_PIXMAP		= ":pics/status/networks/myspaceim_away_hover.png";
const QString QtContactList::STATUS_MYSPACE_BLOCK_HOVER_PIXMAP	= ":pics/status/networks/myspaceim_block_hover.png";
const QString QtContactList::STATUS_MYSPACE_MOBILE_HOVER_PIXMAP	= ":pics/status/networks/myspaceim_mobile_hover.png";

const QString QtContactList::STATUS_FACEBOOK_ONLINE_HOVER_PIXMAP	= ":/pics/status/networks/facebook_online_hover.png";
const QString QtContactList::STATUS_FACEBOOK_OFFLINE_HOVER_PIXMAP = ":/pics/status/networks/facebook_offline_hover.png";
const QString QtContactList::STATUS_FACEBOOK_DND_HOVER_PIXMAP		= ":/pics/status/networks/facebook_donotdisturb_hover.png";
const QString QtContactList::STATUS_FACEBOOK_INVISIBLE_HOVER_PIXMAP = ":/pics/status/networks/facebook_offline_hover.png";
const QString QtContactList::STATUS_FACEBOOK_AWAY_HOVER_PIXMAP	= ":/pics/status/networks/facebook_away_hover.png";
const QString QtContactList::STATUS_FACEBOOK_BLOCK_HOVER_PIXMAP	= ":/pics/status/networks/facebook_block_hover.png";
const QString QtContactList::STATUS_FACEBOOK_MOBILE_HOVER_PIXMAP	= ":/pics/status/networks/facebook_mobile_hover.png";

const QString QtContactList::STATUS_TWITTER_ONLINE_HOVER_PIXMAP	= ":/pics/status/networks/twitter_online_hover.png";
const QString QtContactList::STATUS_TWITTER_OFFLINE_HOVER_PIXMAP	= ":/pics/status/networks/twitter_offline_hover.png";
const QString QtContactList::STATUS_TWITTER_DND_HOVER_PIXMAP		= ":/pics/status/networks/twitter_donotdisturb_hover.png";
const QString QtContactList::STATUS_TWITTER_INVISIBLE_HOVER_PIXMAP = ":/pics/status/networks/twitter_offline_hover.png";
const QString QtContactList::STATUS_TWITTER_AWAY_HOVER_PIXMAP		= ":/pics/status/networks/twitter_away_hover.png";
const QString QtContactList::STATUS_TWITTER_BLOCK_HOVER_PIXMAP	= ":/pics/status/networks/twitter_block_hover.png";
const QString QtContactList::STATUS_TWITTER_MOBILE_HOVER_PIXMAP	= ":/pics/status/networks/twitter_mobile_hover.png";

const QString QtContactList::STATUS_SKYPE_ONLINE_HOVER_PIXMAP		= ":/pics/status/networks/skype_online_hover.png";
const QString QtContactList::STATUS_SKYPE_OFFLINE_HOVER_PIXMAP	= ":/pics/status/networks/skype_offline_hover.png";
const QString QtContactList::STATUS_SKYPE_DND_HOVER_PIXMAP		= ":/pics/status/networks/skype_donotdisturb_hover.png";
const QString QtContactList::STATUS_SKYPE_INVISIBLE_HOVER_PIXMAP	= ":/pics/status/networks/skype_offline_hover.png";
const QString QtContactList::STATUS_SKYPE_AWAY_HOVER_PIXMAP		= ":/pics/status/networks/skype_away_hover.png";
const QString QtContactList::STATUS_SKYPE_BLOCK_HOVER_PIXMAP		= ":/pics/status/networks/skype_block_hover.png";
const QString QtContactList::STATUS_SKYPE_MOBILE_HOVER_PIXMAP		= ":/pics/status/networks/skype_mobile_hover.png";

const QString QtContactList::STATUS_YAHOO_ONLINE_HOVER_PIXMAP		= ":/pics/status/networks/yahoo_online_hover.png";
const QString QtContactList::STATUS_YAHOO_OFFLINE_HOVER_PIXMAP	= ":/pics/status/networks/yahoo_offline_hover.png";
const QString QtContactList::STATUS_YAHOO_DND_HOVER_PIXMAP		= ":/pics/status/networks/yahoo_donotdisturb_hover.png";
const QString QtContactList::STATUS_YAHOO_INVISIBLE_HOVER_PIXMAP	= ":/pics/status/networks/yahoo_offline_hover.png";
const QString QtContactList::STATUS_YAHOO_AWAY_HOVER_PIXMAP		= ":/pics/status/networks/yahoo_away_hover.png";
const QString QtContactList::STATUS_YAHOO_BLOCK_HOVER_PIXMAP		= ":/pics/status/networks/yahoo_block_hover.png";
const QString QtContactList::STATUS_YAHOO_MOBILE_HOVER_PIXMAP		= ":/pics/status/networks/yahoo_mobile_hover.png";

const QString QtContactList::STATUS_JABBER_ONLINE_HOVER_PIXMAP	= ":/pics/status/networks/jabber_online_hover.png";
const QString QtContactList::STATUS_JABBER_OFFLINE_HOVER_PIXMAP	= ":/pics/status/networks/jabber_offline_hover.png";
const QString QtContactList::STATUS_JABBER_DND_HOVER_PIXMAP		= ":/pics/status/networks/jabber_donotdisturb_hover.png";
const QString QtContactList::STATUS_JABBER_INVISIBLE_HOVER_PIXMAP = ":/pics/status/networks/jabber_offline_hover.png";
const QString QtContactList::STATUS_JABBER_AWAY_HOVER_PIXMAP		= ":/pics/status/networks/jabber_away_hover.png";
const QString QtContactList::STATUS_JABBER_BLOCK_HOVER_PIXMAP		= ":/pics/status/networks/jabber_block_hover.png";
const QString QtContactList::STATUS_JABBER_MOBILE_HOVER_PIXMAP	= ":/pics/status/networks/jabber_mobile_hover.png";

const QString QtContactList::STATUS_AIM_ONLINE_HOVER_PIXMAP		= ":/pics/status/networks/aim_online_hover.png";
const QString QtContactList::STATUS_AIM_OFFLINE_HOVER_PIXMAP		= ":/pics/status/networks/aim_offline_hover.png";
const QString QtContactList::STATUS_AIM_DND_HOVER_PIXMAP			= ":/pics/status/networks/aim_donotdisturb_hover.png";
const QString QtContactList::STATUS_AIM_INVISIBLE_HOVER_PIXMAP	= ":/pics/status/networks/aim_offline_hover.png";
const QString QtContactList::STATUS_AIM_AWAY_HOVER_PIXMAP			= ":/pics/status/networks/aim_away_hover.png";
const QString QtContactList::STATUS_AIM_BLOCK_HOVER_PIXMAP		= ":/pics/status/networks/aim_block_hover.png";
const QString QtContactList::STATUS_AIM_MOBILE_HOVER_PIXMAP		= ":/pics/status/networks/aim_mobile_hover.png";

const QString QtContactList::STATUS_ICQ_ONLINE_HOVER_PIXMAP		= ":/pics/status/networks/icq_online_hover.png";
const QString QtContactList::STATUS_ICQ_OFFLINE_HOVER_PIXMAP		= ":/pics/status/networks/icq_offline_hover.png";
const QString QtContactList::STATUS_ICQ_DND_HOVER_PIXMAP			= ":/pics/status/networks/icq_donotdisturb_hover.png";
const QString QtContactList::STATUS_ICQ_INVISIBLE_HOVER_PIXMAP	= ":/pics/status/networks/icq_offline_hover.png";
const QString QtContactList::STATUS_ICQ_AWAY_HOVER_PIXMAP			= ":/pics/status/networks/icq_away_hover.png";
const QString QtContactList::STATUS_ICQ_BLOCK_HOVER_PIXMAP		= ":/pics/status/networks/icq_block_hover.png";
const QString QtContactList::STATUS_ICQ_MOBILE_HOVER_PIXMAP		= ":/pics/status/networks/icq_mobile_hover.png";

const QString QtContactList::STATUS_VOXOX_ONLINE_HOVER_PIXMAP		= ":/pics/status/networks/voxox_online_hover.png";
const QString QtContactList::STATUS_VOXOX_OFFLINE_HOVER_PIXMAP	= ":/pics/status/networks/voxox_offline_hover.png";
const QString QtContactList::STATUS_VOXOX_DND_HOVER_PIXMAP		= ":/pics/status/networks/voxox_donotdisturb_hover.png";
const QString QtContactList::STATUS_VOXOX_INVISIBLE_HOVER_PIXMAP	= ":/pics/status/networks/voxox_offline_hover.png";
const QString QtContactList::STATUS_VOXOX_AWAY_HOVER_PIXMAP		= ":/pics/status/networks/voxox_away_hover.png";
const QString QtContactList::STATUS_VOXOX_BLOCK_HOVER_PIXMAP		= ":/pics/status/networks/voxox_block_hover.png";
const QString QtContactList::STATUS_VOXOX_MOBILE_HOVER_PIXMAP		= ":/pics/status/networks/voxox_mobile_hover.png";

const QString QtContactList::STATUS_GTALK_ONLINE_HOVER_PIXMAP		= ":/pics/status/networks/googletalk_online_hover.png";
const QString QtContactList::STATUS_GTALK_OFFLINE_HOVER_PIXMAP	= ":/pics/status/networks/googletalk_offline_hover.png";
const QString QtContactList::STATUS_GTALK_DND_HOVER_PIXMAP		= ":/pics/status/networks/googletalk_donotdisturb_hover.png";
const QString QtContactList::STATUS_GTALK_INVISIBLE_HOVER_PIXMAP	= ":/pics/status/networks/googletalk_offline_hover.png";
const QString QtContactList::STATUS_GTALK_AWAY_HOVER_PIXMAP		= ":/pics/status/networks/googletalk_away_hover.png";
const QString QtContactList::STATUS_GTALK_BLOCK_HOVER_PIXMAP		= ":/pics/status/networks/googletalk_block_hover.png";
const QString QtContactList::STATUS_GTALK_MOBILE_HOVER_PIXMAP		= ":/pics/status/networks/googletalk_mobile_hover.png";


static inline QPixmap scalePixmap(const QString name) {
	return QPixmap(name).scaled(QSize(20, 20), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QtContactList::QtContactList(CContactList & cContactList, CWengoPhone & cWengoPhone)
	: QObject(NULL),
	_cContactList(cContactList),
	_cWengoPhone(cWengoPhone) {

	_waitingForModel = false;

	_contactListWidget = new QWidget(NULL);

	_ui = new Ui::ContactList();
	_ui->setupUi(_contactListWidget);

//VOXOX CHANGE ASV 04-06-09: we need a margin on top for Mac so the tab bar doesn't get on top of the widget content.
#if defined(OS_MACOSX)
	_contactListWidget->layout()->setContentsMargins(0, 11, 0, 0);
#endif

	//treeWidget
	_ui->treeWidget->setCContactList( &cContactList );			//VOXOX - JRT - 2009.05.13 - Needed for new derived class VoxContactTreeWidget.
	_ui->treeWidget->setAcceptDrops(true);
	_ui->treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_ui->treeWidget->setProperty("showDropIndicator", QVariant(true));
	_ui->treeWidget->setDragEnabled(true);
	_ui->treeWidget->setAlternatingRowColors(false);
	_ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_ui->treeWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
	_ui->treeWidget->setIndentation(0);
	_ui->treeWidget->setRootIsDecorated(false);

	//VOXOX - JRT - 2009.05.13 - Move QtContactPixmap creation to separate method (maybe a class?) for easier reading.
	initContactPixmap();

	_contactManager = new QtContactManager(*_cWengoPhone.getCUserProfileHandler().getCUserProfile(),
		_cWengoPhone, *this, _ui->treeWidget, _ui->treeWidget);

//	QtContactTreeMouseFilter * qtContactTreeMouseFilter = new QtContactTreeMouseFilter(_cContactList, _ui->treeWidget, _ui->treeWidget);
//	SAFE_CONNECT_RECEIVER(qtContactTreeMouseFilter, SIGNAL(mouseClicked(Qt::MouseButton)), _contactManager, SLOT(setMouseButton(Qt::MouseButton)));
//	SAFE_CONNECT(qtContactTreeMouseFilter, SIGNAL(mergeContacts(QString, QString)), SLOT(mergeContactsSlot(QString, QString)));
//	SAFE_CONNECT(qtContactTreeMouseFilter, SIGNAL(orderGroups  (QString, QString)), SLOT(orderGroupsSlot  (QString, QString)));	//VOXOX - JRT - 2009.05.11 

//	SAFE_CONNECT_RECEIVER(_ui->treeWidget, SIGNAL(mouseClicked(Qt::MouseButton)), _contactManager, SLOT(setMouseButton(Qt::MouseButton)));	//VOXOX - JRT - 2009.05.18 not needed
	SAFE_CONNECT(_ui->treeWidget,		   SIGNAL(mergeContacts(QString, QString)), SLOT(mergeContactsSlot(QString, QString)));
	SAFE_CONNECT(_ui->treeWidget,		   SIGNAL(orderGroups  (QString, QString)), SLOT(orderGroupsSlot  (QString, QString)));	//VOXOX - JRT - 2009.05.11 

	QtTreeViewDelegate * delegate = new QtTreeViewDelegate(_cWengoPhone, _contactManager, _ui->treeWidget);
	delegate->setParent(_ui->treeWidget->viewport());
	_ui->treeWidget->setItemDelegate(delegate);

	_ui->treeWidget->setUniformRowHeights(false);
	_ui->treeWidget->header()->hide();

	//Popup Menus
	_contactGroupPopupMenu = new ContactGroupPopupMenu(_cContactList, _ui->treeWidget);
	SAFE_CONNECT(_contactManager, SIGNAL(groupRightClicked(const QString &)), SLOT(groupRightClickedSlot(const QString &)));

	QtWengoPhone * qtWengoPhone = getQtWengoPhone();	//VOXOX - JRT - 2009.09.23 
	qtWengoPhone->setQtContactList(this);
	_ui->treeWidget->viewport()->setFocus();

	updatePresentation();

	_initialLoad = false;		//VOXOX - JRT - 2009.04.12 

	initContent();
}

QtContactList::~QtContactList() 
{
	QtContactListManager::getInstance()->clear();
	OWSAFE_DELETE(_contactListWidget);
	OWSAFE_DELETE(_ui);
}

QtWengoPhone* QtContactList::getQtWengoPhone()
{
	return (QtWengoPhone *) _cWengoPhone.getPresentation();
}

void QtContactList::initContent() 
{
//	//VOXOX - JRT - 2009.07.27 - TODO: this contactAddedEvent() seems redundant and costly.
//	StringList contacts = _cContactList.getContactIds();
//	StringList::const_iterator it;
//	Contact*	contact = NULL;	
//
//	setInitialLoad( true );
//
//	//VOXOX - JRT - 2009.08.09 - Add Multiple group support.
//	for (it = contacts.begin(); it != contacts.end(); ++it) 
//	{
//		contact = _cContactList.getContact(  *it );
//
//		if ( contact )
//		{
//			const ContactGroupInfoSet& cgis = (*it).get
//			contactAddedEvent(*it);
//		}
//
////		contactAddedEvent(*it);
//	}	
//
//	setInitialLoad(false);
}

void QtContactList::setInitialLoadEvent( const int nOnOff )
{
	bool bSet = (nOnOff != 0);
	setInitialLoad( bSet );
}

void QtContactList::setInitialLoad( bool bSet )
{
	LOG_INFO( bSet ? "on" : "off"  );
	_initialLoad = bSet;
	_contactManager->setInitialLoad( bSet );
}

void QtContactList::updatePresentation() 
{
	//VOXOX CHANGE CJC MODIFIED CONTACT BACKGROUND COLOR WE WILL GET IT FROM THE CONTACT STYLE AND THEME AND WE WILL ASSIGN IT WITH STYLESHEET TO THE CONTACT STYLE
	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();
	_ui->treeWidget->setStyleSheet(QString("QTreeWidget{background-color: %1;}").arg(contactListStyle->getContactNonSelectedBackgroundColor().name()));
	_contactManager->userStateChanged();
	_ui->treeWidget->viewport()->update();
}

void QtContactList::contactGroupAddedEvent(const std::string & groupId) 
{
	QString tmp = QString::fromStdString(groupId);
	if (_contactManager->groupsAreHidden()) 
	{
		tmp = DEFAULT_GROUP_NAME;
	}

	addGroup(tmp);
	updatePresentation();
}

void QtContactList::contactGroupRemovedEvent(const std::string & groupId) 
{
	QList <QTreeWidgetItem *> list = _ui->treeWidget->findItems(QString::fromStdString(groupId), Qt::MatchExactly);
	if (!list.isEmpty()) {
		_ui->treeWidget->takeTopLevelItem(_ui->treeWidget->indexOfTopLevelItem(list[0]));
	}
}

void QtContactList::contactGroupRenamedEvent(const std::string & groupId) 
{
	updatePresentation();
}

void QtContactList::contactGroupsReorderedEvent() 
{
	updatePresentation();
}

void QtContactList::contactAddedEvent(const std::string& contactId, const std::string& groupId) 
{
	_contactManager->handleContactChanged(contactId, groupId, false);

	//VOXOX - JRT - 2009.09.23 
//	ContactProfile dstContactProfile = _cContactList.getContactProfile( contactId );
	Contact* contact = _cContactList.getContactById( contactId );

	if ( contact )
	{
		int qtId = contact->getId();
		notifyJsContactManagerContactAdded( contactId, qtId );	//VOXOX - JRT - 2009.09.23 
	}

	updatePresentation();
}

void QtContactList::contactRemovedEvent(const std::string & contactId) 
{
	_contactManager->removeContact(QString::fromStdString(contactId));
	updatePresentation();
	_waitingForModel = false;
}

void QtContactList::contactMovedEvent(const std::string & dstGroupId, const std::string & srcGroupId, const std::string & contactId) 
{
//	_contactManager->moveContact(dstGroupId, srcGroupId, contactId);	//VOXOX - JRT - 2009.08.10 - Not needed.
	updatePresentation();
}

void QtContactList::contactChangedEvent(const std::string & contactId, const std::string& groupId)
{
	_contactManager->handleContactChanged(contactId, groupId, false);
	contactChangedEventSignal(QString::fromStdString(contactId));	//Notify other windows of change.
}

void QtContactList::addGroup(QString contactGroupId) 
{
	QList <QTreeWidgetItem *> list;

	list = _ui->treeWidget->findItems(contactGroupId, Qt::MatchExactly);
	if (list.isEmpty()) 
	{
		QTreeWidgetItem * group = new QTreeWidgetItem(_ui->treeWidget);
		group->setText(0, contactGroupId);
		_ui->treeWidget->setItemExpanded(group, true);
	}
}

CContactList & QtContactList::getCContactList() const 
{
	return _cContactList;
}

void QtContactList::groupRightClickedSlot(const QString & groupId) 
{
	_contactGroupPopupMenu->showMenu(groupId);
}

void QtContactList::mergeContactsSlot(QString dstContact, QString srcContact) 
{
	//VOXOX - JRT - 2009.07.26 - We aren't merging yet, so comment for now.
	//if (!_waitingForModel) {
	//	_waitingForModel = true;

	//VOXOX - JRT - 2009.07.26 - TODO: try not to use getContactProfile().
	//	ContactProfile dstContactProfile = _cContactList.getContactProfile(dstContact.toStdString());
	//	ContactProfile srcContactProfile = _cContactList.getContactProfile(srcContact.toStdString());

	//	//VOXOX - CJC - 2009.06.10 
	//	QtVoxMessageBox box(_contactListWidget);
	//	box.setWindowTitle("VoxOx - - Merge Contacts");
	//	box.setText(tr("Merge %1 with %2?")
	//		.arg(QString::fromStdString(dstContactProfile.getDisplayName()))
	//		.arg(QString::fromStdString(srcContactProfile.getDisplayName())));
	//	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No );

	//	if (box.exec() == QMessageBox::Yes) {
	//		_cContactList.merge(dstContact.toStdString(), srcContact.toStdString());	
	//	} 
	//	
	//	_waitingForModel = false;	//VOXOX - JRT - 2009.05.11 
	//}
}

void QtContactList::orderGroupsSlot( QString dragGroupKey, QString dropGroupKey ) 
{
	if (!_waitingForModel) 
	{
		_waitingForModel = true;

//		if (QMessageBox::question(_contactListWidget,
//			tr("@product@ - Reorder Groups"),
//			tr("Order %1 after %2?")
//			.arg( dragGroupKey )
//			.arg( dropGroupKey ),
//			tr("&Yes"), tr("&No"),
//			QString(), 0, 1) == 0) 
		{
			//VOXOX - JRT - 2009.05.11 - Per Bryan M., once a user manual orders groups, it will stay that way.
			//	So, we automatically change the Group Sort order here.
			Config& config = ConfigManager::getInstance().getCurrentConfig();
			config.set( Config::GENERAL_GROUP_SORT_KEY, QtEnumSortOption::SortGroupManual );

			_cContactList.orderGroups(dragGroupKey.toStdString(), dropGroupKey.toStdString());	
		} 

		_waitingForModel = false;
	}
}

void QtContactList::notifyJsContactManagerContactAdded( const std::string& contactId, int qtId )	//VOXOX - JRT - 2009.09.23 
{
	QtWengoPhone* qtWengoPhone = getQtWengoPhone();

	if ( qtWengoPhone )
	{
		qtWengoPhone->notifyContactManagerContactAdded( contactId, qtId );
	}
}


void QtContactList::initContactPixmap()
{
	QtContactPixmap * spx = QtContactPixmap::getInstance();

	spx->setPixmap(QtContactPixmap::ContactUnknown,			scalePixmap(STATUS_UNKNOW_PIXMAP),STATUS_UNKNOW_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactNoStatus,		scalePixmap(STATUS_NOSTATUS_PIXMAP),STATUS_NOSTATUS_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactNoStatusSIP,		scalePixmap(STATUS_NOSTATUS_SIP_PIXMAP),STATUS_NOSTATUS_SIP_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactOnline,			scalePixmap(STATUS_ONLINE_PIXMAP),STATUS_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactOffline,			scalePixmap(STATUS_OFFLINE_PIXMAP ), STATUS_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactDND,				scalePixmap(STATUS_DND_PIXMAP), STATUS_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactInvisible,		scalePixmap(STATUS_INVISIBLE_PIXMAP),STATUS_INVISIBLE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAway,			scalePixmap(STATUS_AWAY_PIXMAP), STATUS_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactWDeal,			scalePixmap(STATUS_WDEAL_PIXMAP), STATUS_WDEAL_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTestCall,		scalePixmap(STATUS_TESTCALL_PIXMAP), STATUS_TESTCALL_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactMSNOnline,		scalePixmap(STATUS_MSN_ONLINE_PIXMAP), STATUS_MSN_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNOffline,		scalePixmap(STATUS_MSN_OFFLINE_PIXMAP),STATUS_MSN_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNInvisible,	scalePixmap(STATUS_MSN_OFFLINE_PIXMAP),STATUS_MSN_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNDND,			scalePixmap(STATUS_MSN_DND_PIXMAP),STATUS_MSN_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNAway,			scalePixmap(STATUS_MSN_AWAY_PIXMAP),STATUS_MSN_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNBlock,		scalePixmap(STATUS_MSN_BLOCK_PIXMAP),STATUS_MSN_BLOCK_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNMobile,		scalePixmap(STATUS_MSN_MOBILE_PIXMAP),STATUS_MSN_MOBILE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactMYSPACEOnline,	scalePixmap(STATUS_MYSPACE_ONLINE_PIXMAP),STATUS_MYSPACE_ONLINE_PIXMAP);		//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEOffline,	scalePixmap(STATUS_MYSPACE_OFFLINE_PIXMAP),STATUS_MYSPACE_OFFLINE_PIXMAP);	//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEInvisible, scalePixmap(STATUS_MYSPACE_OFFLINE_PIXMAP),STATUS_MYSPACE_OFFLINE_PIXMAP);	//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEDND,		scalePixmap(STATUS_MYSPACE_DND_PIXMAP),STATUS_MYSPACE_DND_PIXMAP);		//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEAway,		scalePixmap(STATUS_MYSPACE_AWAY_PIXMAP),STATUS_MYSPACE_AWAY_PIXMAP);		//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEBlock,	scalePixmap(STATUS_MYSPACE_BLOCK_PIXMAP),STATUS_MYSPACE_BLOCK_PIXMAP);		//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEMobile,	scalePixmap(STATUS_MYSPACE_MOBILE_PIXMAP),STATUS_MYSPACE_MOBILE_PIXMAP);	//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactFacebookOnline,	scalePixmap(STATUS_FACEBOOK_ONLINE_PIXMAP),STATUS_FACEBOOK_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookOffline, scalePixmap(STATUS_FACEBOOK_OFFLINE_PIXMAP),STATUS_FACEBOOK_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookInvisible, scalePixmap(STATUS_FACEBOOK_OFFLINE_PIXMAP),STATUS_FACEBOOK_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookDND,		scalePixmap(STATUS_FACEBOOK_DND_PIXMAP),STATUS_FACEBOOK_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookAway,	scalePixmap(STATUS_FACEBOOK_AWAY_PIXMAP),STATUS_FACEBOOK_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookBlock,	scalePixmap(STATUS_FACEBOOK_BLOCK_PIXMAP),STATUS_FACEBOOK_BLOCK_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookMobile,	scalePixmap(STATUS_FACEBOOK_MOBILE_PIXMAP),STATUS_FACEBOOK_MOBILE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactTwitterOnline,	scalePixmap(STATUS_TWITTER_ONLINE_PIXMAP),STATUS_TWITTER_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterOffline,	scalePixmap(STATUS_TWITTER_OFFLINE_PIXMAP),STATUS_TWITTER_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterInvisible, scalePixmap(STATUS_TWITTER_OFFLINE_PIXMAP),STATUS_TWITTER_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterDND,		scalePixmap(STATUS_TWITTER_DND_PIXMAP),STATUS_TWITTER_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterAway,		scalePixmap(STATUS_TWITTER_AWAY_PIXMAP),STATUS_TWITTER_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterBlock,	scalePixmap(STATUS_TWITTER_BLOCK_PIXMAP),STATUS_TWITTER_BLOCK_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterMobile,	scalePixmap(STATUS_TWITTER_MOBILE_PIXMAP),STATUS_TWITTER_MOBILE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactSkypeOnline,		scalePixmap(STATUS_SKYPE_ONLINE_PIXMAP),STATUS_SKYPE_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeOffline,	scalePixmap(STATUS_SKYPE_OFFLINE_PIXMAP),STATUS_SKYPE_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeInvisible,	scalePixmap(STATUS_SKYPE_OFFLINE_PIXMAP),STATUS_SKYPE_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeDND,		scalePixmap(STATUS_SKYPE_DND_PIXMAP),STATUS_SKYPE_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeAway,		scalePixmap(STATUS_SKYPE_AWAY_PIXMAP),STATUS_SKYPE_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeBlock,		scalePixmap(STATUS_SKYPE_BLOCK_PIXMAP),STATUS_SKYPE_BLOCK_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeMobile,		scalePixmap(STATUS_SKYPE_MOBILE_PIXMAP),STATUS_SKYPE_MOBILE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactYahooOnline,		scalePixmap(STATUS_YAHOO_ONLINE_PIXMAP),STATUS_YAHOO_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactYahooOffline,	scalePixmap(STATUS_YAHOO_OFFLINE_PIXMAP),STATUS_YAHOO_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNInvisible,	scalePixmap(STATUS_YAHOO_OFFLINE_PIXMAP),STATUS_YAHOO_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactYahooDND,		scalePixmap(STATUS_YAHOO_DND_PIXMAP),STATUS_YAHOO_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactYahooAway,		scalePixmap(STATUS_YAHOO_AWAY_PIXMAP),STATUS_YAHOO_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactYahooBlock,		scalePixmap(STATUS_YAHOO_BLOCK_PIXMAP),STATUS_YAHOO_BLOCK_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactYahooMobile,		scalePixmap(STATUS_YAHOO_MOBILE_PIXMAP),STATUS_YAHOO_MOBILE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactJabberOnline,	scalePixmap(STATUS_JABBER_ONLINE_PIXMAP),STATUS_JABBER_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberOffline,	scalePixmap(STATUS_JABBER_OFFLINE_PIXMAP),STATUS_JABBER_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberInvisible, scalePixmap(STATUS_JABBER_OFFLINE_PIXMAP),STATUS_JABBER_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberDND,		scalePixmap(STATUS_JABBER_DND_PIXMAP),STATUS_JABBER_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberAway,		scalePixmap(STATUS_JABBER_AWAY_PIXMAP),STATUS_JABBER_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberBlock,		scalePixmap(STATUS_JABBER_BLOCK_PIXMAP),STATUS_JABBER_BLOCK_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberMobile,	scalePixmap(STATUS_JABBER_MOBILE_PIXMAP),STATUS_JABBER_MOBILE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactICQOnline,		scalePixmap(STATUS_ICQ_ONLINE_PIXMAP),STATUS_ICQ_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQOffline,		scalePixmap(STATUS_ICQ_OFFLINE_PIXMAP),STATUS_ICQ_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQInvisible,	scalePixmap(STATUS_ICQ_OFFLINE_PIXMAP),STATUS_ICQ_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQDND,			scalePixmap(STATUS_ICQ_DND_PIXMAP),STATUS_ICQ_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQAway,			scalePixmap(STATUS_ICQ_AWAY_PIXMAP),STATUS_ICQ_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQBlock,		scalePixmap(STATUS_ICQ_BLOCK_PIXMAP),STATUS_ICQ_BLOCK_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQMobile,		scalePixmap(STATUS_ICQ_MOBILE_PIXMAP),STATUS_ICQ_MOBILE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactAIMOnline,		scalePixmap(STATUS_AIM_ONLINE_PIXMAP),STATUS_AIM_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMOffline,		scalePixmap(STATUS_AIM_OFFLINE_PIXMAP),STATUS_AIM_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMInvisible,	scalePixmap(STATUS_AIM_OFFLINE_PIXMAP),STATUS_AIM_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMDND,			scalePixmap(STATUS_AIM_DND_PIXMAP),STATUS_AIM_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMAway,			scalePixmap(STATUS_AIM_AWAY_PIXMAP),STATUS_AIM_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMBlock,		scalePixmap(STATUS_AIM_BLOCK_PIXMAP),STATUS_AIM_BLOCK_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMMobile,		scalePixmap(STATUS_AIM_MOBILE_PIXMAP),STATUS_AIM_MOBILE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactVoxOxOnline,		scalePixmap(STATUS_VOXOX_ONLINE_PIXMAP),STATUS_VOXOX_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxOffline,	scalePixmap(STATUS_VOXOX_OFFLINE_PIXMAP),STATUS_VOXOX_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxInvisible,	scalePixmap(STATUS_VOXOX_OFFLINE_PIXMAP),STATUS_VOXOX_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxDND,		scalePixmap(STATUS_VOXOX_DND_PIXMAP),STATUS_VOXOX_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxAway,		scalePixmap(STATUS_VOXOX_AWAY_PIXMAP),STATUS_VOXOX_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxBlock,		scalePixmap(STATUS_VOXOX_BLOCK_PIXMAP),STATUS_VOXOX_BLOCK_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxMobile,		scalePixmap(STATUS_VOXOX_MOBILE_PIXMAP),STATUS_VOXOX_MOBILE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactGTalkOnline,		scalePixmap(STATUS_GTALK_ONLINE_PIXMAP),STATUS_GTALK_ONLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkOffline,	scalePixmap(STATUS_GTALK_OFFLINE_PIXMAP),STATUS_GTALK_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkInvisible,	scalePixmap(STATUS_GTALK_OFFLINE_PIXMAP),STATUS_GTALK_OFFLINE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkDND,		scalePixmap(STATUS_GTALK_DND_PIXMAP),STATUS_GTALK_DND_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkAway,		scalePixmap(STATUS_GTALK_AWAY_PIXMAP),STATUS_GTALK_AWAY_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkBlock,		scalePixmap(STATUS_GTALK_BLOCK_PIXMAP),STATUS_GTALK_BLOCK_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkMobile,		scalePixmap(STATUS_GTALK_MOBILE_PIXMAP),STATUS_GTALK_MOBILE_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 


	//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNOnlineHover,		scalePixmap(STATUS_MSN_ONLINE_HOVER_PIXMAP), STATUS_MSN_ONLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNOfflineHover,		scalePixmap(STATUS_MSN_OFFLINE_HOVER_PIXMAP),STATUS_MSN_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNInvisibleHover,	scalePixmap(STATUS_MSN_OFFLINE_HOVER_PIXMAP),STATUS_MSN_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNDNDHover,			scalePixmap(STATUS_MSN_DND_HOVER_PIXMAP),STATUS_MSN_DND_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNAwayHover,		scalePixmap(STATUS_MSN_AWAY_HOVER_PIXMAP),STATUS_MSN_AWAY_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNBlockHover,		scalePixmap(STATUS_MSN_BLOCK_HOVER_PIXMAP),STATUS_MSN_BLOCK_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNMobileHover,		scalePixmap(STATUS_MSN_MOBILE_HOVER_PIXMAP),STATUS_MSN_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactMYSPACEOnlineHover,	scalePixmap(STATUS_MYSPACE_ONLINE_HOVER_PIXMAP),STATUS_MYSPACE_ONLINE_HOVER_PIXMAP);		//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEOfflineHover,	scalePixmap(STATUS_MYSPACE_OFFLINE_HOVER_PIXMAP),STATUS_MYSPACE_OFFLINE_HOVER_PIXMAP);	//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEInvisibleHover,scalePixmap(STATUS_MYSPACE_OFFLINE_HOVER_PIXMAP),STATUS_MYSPACE_OFFLINE_HOVER_PIXMAP);	//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEDNDHover,		scalePixmap(STATUS_MYSPACE_DND_HOVER_PIXMAP),STATUS_MYSPACE_DND_HOVER_PIXMAP);		//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEAwayHover,	scalePixmap(STATUS_MYSPACE_AWAY_HOVER_PIXMAP),STATUS_MYSPACE_AWAY_HOVER_PIXMAP);		//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEBlockHover,	scalePixmap(STATUS_MYSPACE_BLOCK_HOVER_PIXMAP),STATUS_MYSPACE_BLOCK_HOVER_PIXMAP);		//TODO: generates Pixmap is null pixmap.  Missing?
	spx->setPixmap(QtContactPixmap::ContactMYSPACEMobileHover,	scalePixmap(STATUS_MYSPACE_MOBILE_HOVER_PIXMAP),STATUS_MYSPACE_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 	

	spx->setPixmap(QtContactPixmap::ContactFacebookOnlineHover,	scalePixmap(STATUS_FACEBOOK_ONLINE_HOVER_PIXMAP),STATUS_FACEBOOK_ONLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookOfflineHover,scalePixmap(STATUS_FACEBOOK_OFFLINE_HOVER_PIXMAP),STATUS_FACEBOOK_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookInvisibleHover,scalePixmap(STATUS_FACEBOOK_OFFLINE_HOVER_PIXMAP),STATUS_FACEBOOK_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookDNDHover,	scalePixmap(STATUS_FACEBOOK_DND_HOVER_PIXMAP),STATUS_FACEBOOK_DND_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookAwayHover,	scalePixmap(STATUS_FACEBOOK_AWAY_HOVER_PIXMAP),STATUS_FACEBOOK_AWAY_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookBlockHover,	scalePixmap(STATUS_FACEBOOK_BLOCK_HOVER_PIXMAP),STATUS_FACEBOOK_BLOCK_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactFacebookMobileHover,	scalePixmap(STATUS_FACEBOOK_MOBILE_HOVER_PIXMAP),STATUS_FACEBOOK_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactTwitterOnlineHover,	scalePixmap(STATUS_TWITTER_ONLINE_HOVER_PIXMAP),STATUS_TWITTER_ONLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterOfflineHover,	scalePixmap(STATUS_TWITTER_OFFLINE_HOVER_PIXMAP),STATUS_TWITTER_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterInvisibleHover,scalePixmap(STATUS_TWITTER_OFFLINE_HOVER_PIXMAP),STATUS_TWITTER_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterDNDHover,		scalePixmap(STATUS_TWITTER_DND_HOVER_PIXMAP),STATUS_TWITTER_DND_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterAwayHover,	scalePixmap(STATUS_TWITTER_AWAY_HOVER_PIXMAP),STATUS_TWITTER_AWAY_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterBlockHover,	scalePixmap(STATUS_TWITTER_BLOCK_HOVER_PIXMAP),STATUS_TWITTER_BLOCK_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactTwitterMobileHover,	scalePixmap(STATUS_TWITTER_MOBILE_HOVER_PIXMAP),STATUS_TWITTER_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactSkypeOnlineHover,	scalePixmap(STATUS_SKYPE_ONLINE_HOVER_PIXMAP),STATUS_SKYPE_ONLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeOfflineHover,	scalePixmap(STATUS_SKYPE_OFFLINE_HOVER_PIXMAP),STATUS_SKYPE_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeInvisibleHover,	scalePixmap(STATUS_SKYPE_OFFLINE_HOVER_PIXMAP),STATUS_SKYPE_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeDNDHover,		scalePixmap(STATUS_SKYPE_DND_HOVER_PIXMAP),STATUS_SKYPE_DND_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeAwayHover,		scalePixmap(STATUS_SKYPE_AWAY_HOVER_PIXMAP),STATUS_SKYPE_AWAY_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeBlockHover,		scalePixmap(STATUS_SKYPE_BLOCK_HOVER_PIXMAP),STATUS_SKYPE_BLOCK_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactSkypeMobileHover,	scalePixmap(STATUS_SKYPE_MOBILE_HOVER_PIXMAP),STATUS_SKYPE_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactYahooOnlineHover,	scalePixmap(STATUS_YAHOO_ONLINE_HOVER_PIXMAP),STATUS_YAHOO_ONLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactYahooOfflineHover,	scalePixmap(STATUS_YAHOO_OFFLINE_HOVER_PIXMAP),STATUS_YAHOO_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactMSNInvisibleHover,	scalePixmap(STATUS_YAHOO_OFFLINE_HOVER_PIXMAP),STATUS_YAHOO_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactYahooDNDHover,		scalePixmap(STATUS_YAHOO_DND_HOVER_PIXMAP),STATUS_YAHOO_DND_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactYahooAwayHover,		scalePixmap(STATUS_YAHOO_AWAY_HOVER_PIXMAP),STATUS_YAHOO_AWAY_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactYahooBlockHover,		scalePixmap(STATUS_YAHOO_BLOCK_HOVER_PIXMAP),STATUS_YAHOO_BLOCK_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactYahooMobileHover,	scalePixmap(STATUS_YAHOO_MOBILE_HOVER_PIXMAP),STATUS_YAHOO_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactJabberOnlineHover,	scalePixmap(STATUS_JABBER_ONLINE_HOVER_PIXMAP),STATUS_JABBER_ONLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberOfflineHover,	scalePixmap(STATUS_JABBER_OFFLINE_HOVER_PIXMAP),STATUS_JABBER_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberInvisibleHover,scalePixmap(STATUS_JABBER_OFFLINE_HOVER_PIXMAP),STATUS_JABBER_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberDNDHover,		scalePixmap(STATUS_JABBER_DND_HOVER_PIXMAP),STATUS_JABBER_DND_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberAwayHover,		scalePixmap(STATUS_JABBER_AWAY_HOVER_PIXMAP),STATUS_JABBER_AWAY_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberBlockHover,	scalePixmap(STATUS_JABBER_BLOCK_HOVER_PIXMAP),STATUS_JABBER_BLOCK_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactJabberMobileHover,	scalePixmap(STATUS_JABBER_MOBILE_HOVER_PIXMAP),STATUS_JABBER_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactICQOnlineHover,		scalePixmap(STATUS_ICQ_ONLINE_HOVER_PIXMAP),STATUS_ICQ_ONLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQOfflineHover,		scalePixmap(STATUS_ICQ_OFFLINE_HOVER_PIXMAP),STATUS_ICQ_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQInvisibleHover,	scalePixmap(STATUS_ICQ_OFFLINE_HOVER_PIXMAP),STATUS_ICQ_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQDNDHover,			scalePixmap(STATUS_ICQ_DND_HOVER_PIXMAP),STATUS_ICQ_DND_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQAwayHover,		scalePixmap(STATUS_ICQ_AWAY_HOVER_PIXMAP),STATUS_ICQ_AWAY_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQBlockHover,		scalePixmap(STATUS_ICQ_BLOCK_HOVER_PIXMAP),STATUS_ICQ_BLOCK_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactICQMobileHover,		scalePixmap(STATUS_ICQ_MOBILE_HOVER_PIXMAP),STATUS_ICQ_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactAIMOnlineHover,		scalePixmap(STATUS_AIM_ONLINE_HOVER_PIXMAP),STATUS_AIM_ONLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMOfflineHover,		scalePixmap(STATUS_AIM_OFFLINE_HOVER_PIXMAP),STATUS_AIM_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMInvisibleHover,	scalePixmap(STATUS_AIM_OFFLINE_HOVER_PIXMAP),STATUS_AIM_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMDNDHover,			scalePixmap(STATUS_AIM_DND_HOVER_PIXMAP),STATUS_AIM_DND_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMAwayHover,		scalePixmap(STATUS_AIM_AWAY_HOVER_PIXMAP),STATUS_AIM_AWAY_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMBlockHover,		scalePixmap(STATUS_AIM_BLOCK_HOVER_PIXMAP),STATUS_AIM_BLOCK_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactAIMMobileHover,		scalePixmap(STATUS_AIM_MOBILE_HOVER_PIXMAP),STATUS_AIM_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactVoxOxOnlineHover,	scalePixmap(STATUS_VOXOX_ONLINE_HOVER_PIXMAP),STATUS_VOXOX_ONLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxOfflineHover,	scalePixmap(STATUS_VOXOX_OFFLINE_HOVER_PIXMAP),STATUS_VOXOX_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxInvisibleHover,	scalePixmap(STATUS_VOXOX_OFFLINE_HOVER_PIXMAP),STATUS_VOXOX_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxDNDHover,		scalePixmap(STATUS_VOXOX_DND_HOVER_PIXMAP),STATUS_VOXOX_DND_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxAwayHover,		scalePixmap(STATUS_VOXOX_AWAY_HOVER_PIXMAP),STATUS_VOXOX_AWAY_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxBlockHover,		scalePixmap(STATUS_VOXOX_BLOCK_HOVER_PIXMAP),STATUS_VOXOX_BLOCK_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactVoxOxMobileHover,	scalePixmap(STATUS_VOXOX_MOBILE_HOVER_PIXMAP),STATUS_VOXOX_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	spx->setPixmap(QtContactPixmap::ContactGTalkOnlineHover,	scalePixmap(STATUS_GTALK_ONLINE_HOVER_PIXMAP),STATUS_GTALK_ONLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkOfflineHover,	scalePixmap(STATUS_GTALK_OFFLINE_HOVER_PIXMAP),STATUS_GTALK_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkInvisibleHover,	scalePixmap(STATUS_GTALK_OFFLINE_HOVER_PIXMAP),STATUS_GTALK_OFFLINE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkDNDHover,		scalePixmap(STATUS_GTALK_DND_HOVER_PIXMAP),STATUS_GTALK_DND_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkAwayHover,		scalePixmap(STATUS_GTALK_AWAY_HOVER_PIXMAP),STATUS_GTALK_AWAY_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkBlockHover,		scalePixmap(STATUS_GTALK_BLOCK_HOVER_PIXMAP),STATUS_GTALK_BLOCK_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 
	spx->setPixmap(QtContactPixmap::ContactGTalkMobileHover,	scalePixmap(STATUS_GTALK_MOBILE_HOVER_PIXMAP),STATUS_GTALK_MOBILE_HOVER_PIXMAP);//VOXOX CHANGE by Rolando - 2009.10.23 

	//Group icons
	//These are not being used anymore
	//spx->setPixmap(QtContactPixmap::ContactGroupOpen,  QPixmap(STATUS_GROUP_OPEN_PIXMAP));
	//spx->setPixmap(QtContactPixmap::ContactGroupClose, QPixmap(STATUS_GROUP_CLOSE_PIXMAP));
}
//-----------------------------------------------------------------------------
