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
#include "QtContactManager.h"

#include "QtContactWidget.h"
#include "QtContactList.h"
#include "QtContactPixmap.h"
#include "QtContactInfo.h"
#include "QtContactListManager.h"
#include "QtContactTreeKeyFilter.h"
#include "QtConferenceAction.h"

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/profile/QtProfileDetails.h>
#include <presentation/qt/contactlist/QtContactListManager.h>
#include <presentation/qt/contactlist/QtContactActionManager.h>
#include <presentation/qt/contactlist/QtEnumSortOption.h>
#include <presentation/qt/QtVoxWindowManager.h>			//VOXOX - JRT - 2009.09.19 

#include <presentation/qt/messagebox/QtVoxPhoneNumberMessageBox.h>
#include <presentation/qt/messagebox/QtVoxEmailMessageBox.h>

#include <control/CWengoPhone.h>
#include <control/contactlist/CContactList.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/phoneline/PhoneLine.h>
#include <model/phonecall/PhoneCall.h>
#include <model/profile/UserProfile.h>
#include <model/contactlist/ContactList.h>
#include <model/contactlist/ContactProfile.h>

#include <presentation/qt/QtEnumPhoneType.h>
#include <presentation/qt/QtEnumEmailType.h>
#include <sipwrapper/EnumPhoneCallState.h>

#include <util/WebBrowser.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/LanguageChangeEventFilter.h>

#include <QtGui/QtGui>

//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>

#ifdef _WINDOWS		//VOXOX - JRT - 2009.04.08 
#include "windows.h"
#endif

int	QtContactManager::_refreshInterval	= 500;		//in milliseconds
int	QtContactManager::_reloadInterval	= 500;		//in milliseconds

QtContactManager::QtContactManager(CUserProfile & cUserProfile, CWengoPhone & cWengoPhone,
	QtContactList & qtContactList, QObject * parent, QTreeWidget * target)
	: QObject(parent),
	_cWengoPhone(cWengoPhone),
	_cUserProfile(cUserProfile),
	_qtContactList(qtContactList) {

	LANGUAGE_CHANGE(this);
	languageChanged();

	_tree		= dynamic_cast<VoxContactListTreeWidget*>(target);
	_sortContacts = true;
	_menu		= NULL;
	_itemSize	= NULL;

	_sortTimerId = -1;
	_canSort	= true;
	_wantSort	= false;

	_showTimerId = -1;
	_canShow	= true;
	_wantShow	= false;

	_needsRefresh	= true;
	_needsReload	= true;
	_isDrawing		= false;

	_topQtContact = NULL;

	_initialLoad = false;

	QtContactListManager::getInstance()->setTreeWidget(target);
	target->setMouseTracking(true);
	QtContactTreeKeyFilter * keyFilter = new QtContactTreeKeyFilter(this, target);

//	SAFE_CONNECT(target,	SIGNAL(itemSelectionChanged()),				 SLOT(treeViewSelectionChanged()		  ));
//	SAFE_CONNECT(target,	SIGNAL(itemClicked(QTreeWidgetItem *, int)), SLOT(itemClicked(QTreeWidgetItem *, int) ));

	//VOXOX - JRT - 2009.05.14 - With new VoxContactTreeWidget class we can generate our own itemRightClicked event.  
	SAFE_CONNECT(target,	SIGNAL(itemLeftClicked  (QTreeWidgetItem *, int)), SLOT(itemLeftClicked  (QTreeWidgetItem *, int) ));
	SAFE_CONNECT(target,	SIGNAL(itemRightClicked (QTreeWidgetItem *, int)), SLOT(itemRightClicked (QTreeWidgetItem *, int) ));
	SAFE_CONNECT(target,	SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(itemDoubleClicked(QTreeWidgetItem *, int) ));

	SAFE_CONNECT(keyFilter, SIGNAL(openItem    (QTreeWidgetItem *)), SLOT(openContactInfo(QTreeWidgetItem *)));
	SAFE_CONNECT(keyFilter, SIGNAL(closeItem   (QTreeWidgetItem *)), SLOT(closeContactInfo()				));
	SAFE_CONNECT(keyFilter, SIGNAL(deleteItem  (QTreeWidgetItem *)), SLOT(deleteContact()					));
	SAFE_CONNECT(keyFilter, SIGNAL(enterPressed(QTreeWidgetItem *)), SLOT(defaultAction(QTreeWidgetItem *)	));

	QtWengoPhone * qtWengoPhone = (QtWengoPhone *) _cWengoPhone.getPresentation();

	SAFE_CONNECT_RECEIVER(this, SIGNAL(inviteToConferenceClicked(QString, PhoneCall *)), qtWengoPhone, SLOT(addToConference(QString, PhoneCall *)));

	restartSortTimer();		//VOXOX - JRT - 2009.10.14 
}

void QtContactManager::startSMS() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->smsContact(QString::fromStdString(getCurrentItemContactKey()));

	/*std::string contactId = getCurrentItemContactId();
	_cUserProfile.startIMAndSendMessage( contactId, "VoxOx automated message test");*/
}

void QtContactManager::startChat() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->chatContact(QString::fromStdString(getCurrentItemContactKey()));

	/*std::string contactId = getCurrentItemContactKey();
	_cUserProfile.startIM(contactId);*/
}

void QtContactManager::sendFile() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->sendFileContact(QString::fromStdString(getCurrentItemContactKey()));	
	
	/*std::string contactId = getCurrentItemContactId();
	_cUserProfile.sendFile(contactId);*/
}

void QtContactManager::editContact() {
	QTreeWidgetItem * item = _tree->currentItem();
	if (item) {
		editContact(item->text(0));
	}
}

void QtContactManager::editContact(QString contactId) {
	//TODO JEFF ADD HERE THE CONTACT MANAGER WINDOW
	/*ContactProfile contactProfile = _cUserProfile.getCContactList().getContactProfile(contactId.toStdString());
	QtProfileDetails qtProfileDetails(_cUserProfile, contactProfile, _tree, tr("Edit Contact"));
	if (qtProfileDetails.show()) {*/
//		_cUserProfile.getCContactList().updateContact(contactProfile);
		//_cUserProfile.updateContact(contactProfile);	//VOXOX - JRT - 2009.05.25 
	//}
}

void QtContactManager::deleteContact() {
	QTreeWidgetItem * item = _tree->currentItem();
	if (!item) {
		return;
	}
	//VOXOX - CJC - 2009.06.10 
	QtVoxMessageBox box(_tree);
	box.setWindowTitle("VoxOx - Delete contact");
	box.setText(tr("Do you really want to delete this contact?"));
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No );
	if(box.exec() == QMessageBox::Yes)
	{
		QString contactId = item->text(0);

		_cUserProfile.getCContactList().removeContact( contactId.toStdString());
		
		//VOXOX - JRT - 2009.09.19 - If CM is open, we need to remove from CM.
		QtWengoPhone *qtWengoPhone = dynamic_cast<QtWengoPhone *>(_cWengoPhone.getPresentation());
		qtWengoPhone->getQtVoxWindowManager()->showContactManagerWindow(contactId, QtContactProfileWidget::Action_Delete );
	}
	/*if (QMessageBox::question(_tree,
		tr("Delete contact"),
		tr("Do you really want to delete this contact?"),
		tr("&Yes"),
		tr("&No"),
		QString(),
		0, 1) == 0) {

		_cUserProfile.getCContactList().removeContact(item->text(0).toStdString());
	}*/
}

void QtContactManager::reRequestAuthorization() {//VOXOX - CJC - 2009.07.31 
	QTreeWidgetItem * item = _tree->currentItem();
	if (!item) {
		return;
	}
	_cUserProfile.reRequestAuthorization(item->text(0).toStdString());
}


///VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
void QtContactManager::blockContact() {
	QTreeWidgetItem * item = _tree->currentItem();
	if (!item) {
		return;
	}

	//VOXOX - CJC - 2009.06.10 
	QtVoxMessageBox box(_tree);
	box.setWindowTitle("VoxOx - Block contact");
	box.setText(tr("Do you really want to block this contact?"));
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No );
	if(box.exec() == QMessageBox::Yes){
		_cUserProfile.getCContactList().blockContact(item->text(0).toStdString());
	}
}
///VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
void QtContactManager::unblockContact() {
	QTreeWidgetItem * item = _tree->currentItem();
	if (!item) {
		return;
	}

	//VOXOX - CJC - 2009.06.10 
	QtVoxMessageBox box(_tree);
	box.setWindowTitle("VoxOx - Unblock contact");
	box.setText(tr("Do you really want to unblock this contact?"));
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No );
	if(box.exec() == QMessageBox::Yes){
		_cUserProfile.getCContactList().unblockContact(item->text(0).toStdString());
	}
}

//VOXOX - JRT - 2009.05.20  - Moved to VoxContactTreeWidget
//void QtContactManager::treeViewSelectionChanged() 
//{
////	if (_button == Qt::NoButton) {
//		closeContactInfo();
////	}
//}

void QtContactManager::openContactInfo( QTreeWidgetItem * item ) 
{
	_tree->openContactInfo( item );
}

void QtContactManager::closeContactInfo() 
{
	_tree->closeContactInfo();
}

void QtContactManager::itemDoubleClicked(QTreeWidgetItem * item, int)
{
	defaultAction(item);
}

void QtContactManager::itemRightClicked(QTreeWidgetItem * item, int)
{
	itemRightClicked(item);
}

void QtContactManager::itemLeftClicked(QTreeWidgetItem * item, int)	
{

	if (item->parent())			//This is contact
	{
		//VOXOX - JRT - 2009.05.20 - This is now handled in VoxContactTreeWidget
	}
	else						//This is group
	{
		groupClicked( item );
	}

	_tree->viewport()->update();
//	_button = Qt::NoButton;
}

void QtContactManager::itemRightClicked(QTreeWidgetItem * item) 
{
	if ( item->parent() == NULL )	//Is group item.
	{
		//VOXOX - JRT - 2009.05.15 - TODO: Why is contact menu in QtContactManager and
		//		Group menu is in QtContactList?
		groupRightClicked(item->text(0));
	}
	else
	{
		OWSAFE_DELETE(_menu);
		_menu = createMenu();
		_menu->popup(QCursor::pos());

	//	_tree->clearSelection();	// is bugged ! We have to clear the selection ourself
		clearTreeSelection();
		_tree->setItemSelected(item, true);
//		_button = Qt::NoButton;
	}
}

void QtContactManager::groupClicked(QTreeWidgetItem * item) 
{
	//VOXOX - JRT - 2009.05.15 - Moved to itemRightClicked()

	//setGroupOpen( item->text(0), bOpen );
//	setGroupOpen( item->text(0), _tree->toggleExpanded( item ) );
	setGroupOpen( item->text(0), item->isExpanded() );
}

void QtContactManager::clearTreeSelection() {
	QList <QTreeWidgetItem * > selectedList = _tree->selectedItems();
	QList <QTreeWidgetItem * >::iterator it;
	for (it = selectedList.begin(); it != selectedList.end(); it ++) {
		_tree->setItemSelected((*it), false);
	}
}

void QtContactManager::defaultAction(QTreeWidgetItem * item) 
{
	if (!item) 
	{
		return;
	}

	QString userId = item->text(0);

	//VOXOX - JRT - 2009.07.26 - This is all business logic.  Moved to CUserProfile.
	_cUserProfile.doDefaultAction( userId.toStdString() );

//	Config & config = ConfigManager::getInstance().getCurrentConfig();
////	QtContactListManager * ul = QtContactListManager::getInstance();
//
//	if (config.getGeneralClickStartChat()) 
//	{
//		_cUserProfile.startIM(userId.toStdString());
//
//		return;
//	}
//
//	ContactProfile contactProfile = _qtContactList.getCContactList().getContactProfile(userId.toStdString());
//
//	// Start free call by default
//	if (contactProfile.hasFreeCall() || contactProfile.hasVoiceMail()) 
//	{
//		std::string phoneNumber = contactProfile.getFirstFreePhoneNumber();
//		if (phoneNumber.empty()) 
//		{
//			phoneNumber = contactProfile.getFirstVoiceMailNumber();
//		}
//
//		if (phoneNumber.empty()) 
//		{
//			LOG_WARN("Could find neither a free phone number, nor a voice mail number");
//		}
//		else
//		{
//			_cUserProfile.makeCall(phoneNumber);
//		}
//		//End VoxOx
//	} 
//	else if (config.getGeneralClickCallCellPhone() && contactProfile.hasCall()) 
//	{
//		_cUserProfile.makeCall( userId.toStdString() );
//	}
}

void QtContactManager::safeUserStateChanged() {
	OWSAFE_DELETE(_menu);
	redrawContacts();
}

void QtContactManager::userStateChanged() {
	safeUserStateChanged();
}

//-----------------------------------------------------------------------------
//Begin Sort related methods
//-----------------------------------------------------------------------------

void QtContactManager::restartSortTimer()
{
	_canSort = false;

	if (_sortTimerId != -1) 
	{
		killTimer(_sortTimerId);
	}

	_sortTimerId = startTimer(1000);
	_wantSort    = false;

	_showTimerId = startTimer(500);		//JRT-XXX

}

void QtContactManager::sortContacts(bool bypassTimer) {
	safeSortContacts(bypassTimer);
}

bool QtContactManager::shouldSortContacts(bool bypassTimer) 
{
	bool bSort = true;

	if (_sortContacts) 
	{
		if (!bypassTimer) 
		{
			if (_canSort) 
			{
				restartSortTimer();
			} 
			else 
			{
				_wantSort = true;
				bSort     = false;
			}
		}
	}
	else
	{
		bSort = false;
	}

	return bSort;
}

	
void QtContactManager::safeSortContacts(bool bypassTimer)
{
	if ( shouldSortContacts( bypassTimer ) )
	{
		RecursiveMutex::ScopedLock lock(_mutex);
		_tree->lock();

		bool saved = saveTopQtContact();			//VOXOX - JRT - 2009.10.15 

		setInitialLoad( true );

		int topCount = _tree->topLevelItemCount();

		if ( topCount > 1 ) 
		{
			QTreeWidgetItem* root = _tree->invisibleRootItem();
			sortChildren( root, true );
			restoreGroupExpandedStatus();
		}

		for (int topIndex = 0; topIndex < topCount; ++topIndex) 
		{
			QTreeWidgetItem* groupItem = _tree->topLevelItem(topIndex);

			sortChildren( groupItem, false );
		}

		if ( saved )		//VOXOX - JRT - 2009.10.15 
		{
			restoreTopQtContact();
		}

		_tree->unlock();

		setInitialLoad( false );
	}
}

void QtContactManager::sortChildren( QTreeWidgetItem* parentItem, bool bTopLevel )
{
	int count = parentItem->childCount();

	QtContact* parQtContact = getQtContactFromItem( parentItem );

	QtContactInfoList contactInfoList;
		
	int				 sortOrder = 0;
	bool			 isGroup   = 0;
	QString			 userName  = "";
	QTreeWidgetItem* item	   = NULL;
	QtContact*		 pContact  = NULL;
	EnumGroupType::GroupType  grpType = EnumGroupType::GroupType_User;

	EnumPresenceState::PresenceState presence = EnumPresenceState::PresenceStateUnknown;

	for (int index = 0; index < count; ++index) 
	{
		item	 = parentItem->child(index);
		pContact = getQtContactFromItem( item );

		if ( bTopLevel )
		{
			userName = (pContact ? QString(pContact->getGroupName() ) : "" );
			presence = EnumPresenceState::PresenceStateUnknown;
			sortOrder = (pContact ? pContact->getSortOrder() : 0);
			isGroup   = true;
			grpType   = (pContact ? pContact->getGroupType() : EnumGroupType::GroupType_User );
		}
		else
		{
			userName  = (pContact ? QString(pContact->getDisplayName() ) : "" );
			presence  = (pContact ? pContact->getPresenceState()         : EnumPresenceState::PresenceStateUnknown);
			sortOrder = 0;
			isGroup   = false;
		}

		QtContactInfo info = QtContactInfo(item, parentItem, item->text(0), userName, index, presence, sortOrder, isGroup, grpType );
		contactInfoList.append(info);
		//End VoxOx
	}

	qSort(contactInfoList);

	Q_FOREACH(QtContactInfo info, contactInfoList) 
	{
		QTreeWidgetItem* item = info.getItem();
		parentItem->takeChild  ( parentItem->indexOfChild(item));
		parentItem->insertChild( parentItem->childCount(), item);
	}
}

void QtContactManager::restoreGroupExpandedStatus()
{
	int topCount = _tree->topLevelItemCount();

	for ( int x = 0; x < topCount; x++ )
	{
		QTreeWidgetItem* item = _tree->topLevelItem(x);
		QString contactGroupId = QString( item->text(0) );
		bool bOpen = isGroupOpen( contactGroupId );

		_tree->setItemExpanded( item, bOpen );
	}
}

//-----------------------------------------------------------------------------
//End sort related methods
//-----------------------------------------------------------------------------


bool QtContactManager::canShowUser(const std::string& contactId )
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if (config.getShowOfflineContacts()) 
	{
		return true;
	}

	bool	 bShow   = true;	//If anything goes wrong, show the contact.
	Contact* contact = _qtContactList.getCContactList().getContactByKey( contactId );

	if ( contact )
	{
		// Enforce rules from http://dev.openwengo.org/trac/openwengo/trac.cgi/wiki/ContactPresenceGrid version 9
		EnumPresenceState::PresenceState presenceState = contact->getPresenceState();

		if ( presenceState == EnumPresenceState::PresenceStateOffline     ||
			 presenceState == EnumPresenceState::PresenceStateUnavailable ||
			 presenceState == EnumPresenceState::PresenceStateUnknown		)
		{
			bool						hasSipNumber  = contact->hasPstnCall();
			bool						blocked		  = contact->isBlocked();		//VOXOX - CJC - 2009.05.05 Add blocked property
			EnumIMProtocol::IMProtocol	protocol	  = contact->getIMProtocol();
			QtEnumIMProtocol::IMProtocol qtImProtocol = QtEnumIMProtocolMap::getInstance().toQtIMProtocol( protocol, contact->getIsIMAccountVoxox(), 
																												  contact->getIsIMAccountGtalk() ); 

			QtContactPixmap::ContactPixmap pixmap = QtContactPixmap::determinePixmap( qtImProtocol, presenceState, hasSipNumber,blocked );

			//VOXOX - JRT - 2009.04.21 - I question the use of QtContactPixmap to determine if we should show a contact.  REVIEW this.
			switch (pixmap) 
			{
			case QtContactPixmap::ContactNoStatusSIP:
			case QtContactPixmap::ContactNoStatus:
			case QtContactPixmap::ContactWDeal:
			case QtContactPixmap::ContactTestCall:
				bShow = true;
				break;

			case QtContactPixmap::ContactUnknown:
			case QtContactPixmap::ContactOffline:

			case QtContactPixmap::ContactMSNOffline:
			case QtContactPixmap::ContactMYSPACEOffline:
			case QtContactPixmap::ContactFacebookOffline:
			case QtContactPixmap::ContactTwitterOffline:
			case QtContactPixmap::ContactSkypeOffline:
			case QtContactPixmap::ContactYahooOffline:
			case QtContactPixmap::ContactAIMOffline:
			case QtContactPixmap::ContactICQOffline:
			case QtContactPixmap::ContactVoxOxOffline:
			case QtContactPixmap::ContactGTalkOffline:
			case QtContactPixmap::ContactJabberOffline:
				bShow = false;
				break;

			default:
				LOG_WARN("Contact status for user '%s' should not be %d (presenceState=%s)",
					contact->getDisplayName().c_str(),
					pixmap,
					EnumPresenceState::toString(presenceState).c_str());
				bShow = true;
				break;
			}
		}
	}

	return bShow;
}

bool QtContactManager::showTimerOk() 
{
	bool result = false;

	if (_canShow) 
	{
		_canShow = false;

		if (_showTimerId != -1) 
		{
			killTimer(_showTimerId);
		}

		_showTimerId = startTimer(500);
		_wantShow = false;
	} 
	else 
	{
		_wantShow = true;
	}

	return !_wantShow;
}

void QtContactManager::redrawContacts() 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	_tree->lock();

	//Save top most visible item so we can scroll back to it.
	bool saved = saveTopQtContact();

//	if ( !showTimerOk() )
//		return;

	_isDrawing = true;

	Contact*		contact = NULL;

	setInitialLoad( true );

	//First, remove all item from the treeview and all users from userlist
	clear();

	//VOXOX - JRT - 2009.08.21 - We now want to show empty groups, so we add a separate pass.
	//Second, add groups.
	ContactGroupSet& cgs = _qtContactList.getCContactList().getContactGroupSet();
	for ( ContactGroupSet::const_iterator it = cgs.begin(); it != cgs.end(); it++ )
	{
		bool isFilterGroup = false;
		if ( shouldUseGroup( (*it).second, isFilterGroup ) )
		{
			QString contactGroupId( (*it).second.getKey().c_str() );
			QTreeWidgetItem* item = createGroupItem( contactGroupId, isFilterGroup) ;
		}
	}

	//Third, add needed contacts
	ContactList& contacts = _qtContactList.getCContactList().getContactList();
	for ( Contacts::const_iterator it = contacts.getContacts().begin(); it != contacts.getContacts().end(); it++) 
	{
		std::string contactId = const_cast<Contact&>((*it).second).getKey();
		std::string groupId   = "";
		bool		isFilter  = false;

		//VOXOX - JRT - 2009.08.09 - Multiple Group support
		const ContactGroupInfoSet& cgis = (*it).second.getContactGroupInfoSetConst();

		for ( ContactGroupInfoSet::const_iterator it2 = cgis.begin(); it2 != cgis.end(); it2++ )
		{
			bool validGroup = true;

			if ( (*it2).second.isFilter() )				//VOXOX - JRT - 2009.08.13 - Always display Filter group
			{
				groupId = (*it2).second.getKey();
			}
			else if ( groupsAreHidden() )
			{
				groupId = "";
			}
			else if ( shouldUseGroup( (*it2).second, (*it2).second.getKey(), isFilter ) )
			{
				groupId = (*it2).second.getKey();
			}
			else
			{
				validGroup = false;
			}

			if ( validGroup )
			{
				handleContactChanged( contactId, groupId, false );
			}
		}
	}

	_tree->reExpandItems();

	//Fourth, sort
	safeSortContacts(true);

	//Reposition top visible item.
	if ( saved )
	{
		restoreTopQtContact();
	}

	setInitialLoad( false );

	_isDrawing = false;

	_tree->unlock();
}

bool QtContactManager::saveTopQtContact()
{
	bool saved = false;

	if ( _topQtContact == NULL )
	{
		QTreeWidgetItem* topItem = _tree->itemAt( 0, 0 );
		
		if ( topItem )
		{
			QtContact* temp = getQtContactFromItem( topItem );

			if ( temp )
			{
				_topQtContact = new QtContact( temp->getId().toStdString(), temp->getGroupKey(), _cWengoPhone, NULL, temp->isGroup() );
				saved = true;
			}
		}
	}

	return saved;
}

void QtContactManager::restoreTopQtContact()
{
	if ( _topQtContact )
	{
		_tree->positionItemAtTop( _topQtContact );
		delete _topQtContact;
		_topQtContact = NULL;
	}
}

void QtContactManager::clear()
{
	QtContactListManager * ul = QtContactListManager::getInstance();
	_tree->clear();
	ul->clear();
}

QMenu * QtContactManager::createConferenceMenu() 
{
	//VOXOX - JRT - 2009.04.15 - TODO - Why does this not use the _cUserProfile memvar?
	PhoneLine * phoneLine = dynamic_cast <PhoneLine *>
		(_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getActivePhoneLine());

	QMenu * menu = new QMenu(_trStringInviteToConference);
	if (phoneLine) {
		PhoneLine::PhoneCallList phoneCallList = phoneLine->getPhoneCallList();
		PhoneLine::PhoneCallList::iterator it;
		for (it = phoneCallList.begin(); it != phoneCallList.end(); it++) {
			if ((*it)->getState() != EnumPhoneCallState::PhoneCallStateClosed) 
			{
				QtConferenceAction * action = new QtConferenceAction(QString::fromStdString((*it)->getPeerSipAddress().getUserName()), menu);
				action->setPhoneCall((*it));
				SAFE_CONNECT(action, SIGNAL(triggered(bool)), SLOT(inviteToConference()));
				menu->addAction(action);
			}
		}
	}
	return menu;
}

void QtContactManager::inviteToConference() 
{
	QObject * source = sender();

	if (source) 
	{
		QtConferenceAction* action	 = dynamic_cast < QtConferenceAction * > (source);
		Contact*			pContact = getCurrentContact();

		if ( pContact )
		{
			std::string phone = pContact->getPreferredNumber();

			if ( !phone.empty() )
			{
				inviteToConferenceClicked( QString(phone.c_str()), action->getPhoneCall());
			}
		}
	} 
	else 
	{
		LOG_FATAL("don't call this function directly");
	}
}

QMenu * QtContactManager::createCallMenu(){
	Contact * pContact = this->getCurrentContact();
	QMenu * callMenu = new QMenu(tr("Call Contact"));

	SAFE_CONNECT(callMenu, SIGNAL(triggered(QAction *)), SLOT(callNumber(QAction *)));

	Telephones& rPhones = pContact->getTelephones();

	if(rPhones.hasValid()){
	
		for ( Telephones::iterator it = rPhones.begin(); it != rPhones.end(); it++ )
		{
			if ( (*it).isValid() )
			{
				QString number = QString::fromStdString((*it).getNumber());
				QtEnumPhoneType::Type type = QtEnumPhoneType::toPhoneType(QString::fromStdString((*it).getType()));
				//QIcon icon(QtEnumPhoneType::getTypeIconPath(type));
				callMenu->addAction(number);
			}
		}
		callMenu->addSeparator();
	}
	
	QAction * addNumber =  new QAction("Add Number +", this);
	SAFE_CONNECT(addNumber, SIGNAL(triggered()), SLOT(addNumber()));

	callMenu->addAction(addNumber);
	
	return callMenu;
}

QMenu * QtContactManager::createEmailMenu(){
	Contact * pContact = this->getCurrentContact();
	QMenu * emailMenu = new QMenu(tr("Send Email"));

	SAFE_CONNECT(emailMenu, SIGNAL(triggered(QAction *)), SLOT(sendEmailContact(QAction *)));

	EmailAddresses& rEmail = pContact->getEmailAddresses();

	if(rEmail.hasValid()){
		
		for ( EmailAddresses::iterator it = rEmail.begin(); it != rEmail.end(); it++ )
		{

			if ( (*it).isValid() )
			{
				QString email = QString::fromStdString((*it).getAddress());
				QtEnumEmailType::Type type = QtEnumEmailType::toEmailType(QString::fromStdString((*it).getType()));
				//QIcon icon(QtEnumEmailType::getTypeIconPath(type));
				emailMenu->addAction(email);
			}
		}
		emailMenu->addSeparator();
	}
	
	QAction * addEmail =  new QAction("Add Email Address +", this);
	SAFE_CONNECT(addEmail, SIGNAL(triggered()), SLOT(addEmail()));

	emailMenu->addAction(addEmail);
	return emailMenu;
	
}
QMenu * QtContactManager::createSMSMenu(){
	Contact * pContact = this->getCurrentContact();
	QMenu * smsMenu = new QMenu(tr("Send SMS"));

	SAFE_CONNECT(smsMenu, SIGNAL(triggered(QAction *)), SLOT(sendSMSContact(QAction *)));

	Telephones& rPhones = pContact->getTelephones();

	if(rPhones.hasValid()){
	
		for ( Telephones::iterator it = rPhones.begin(); it != rPhones.end(); it++ )
		{
			if ( (*it).isValid() && (*it).getType() == QtEnumPhoneType::toString(QtEnumPhoneType::Mobile).toStdString())
			{
				QString number = QString::fromStdString((*it).getNumber());
				QtEnumPhoneType::Type type = QtEnumPhoneType::toPhoneType(QString::fromStdString((*it).getType()));
				//QIcon icon(QtEnumPhoneType::getTypeIconPath(type));
				smsMenu->addAction(number);
			}
		}
		smsMenu->addSeparator();
	}
	
	QAction * addNumber =  new QAction(tr("Add Mobile Number +"), this);
	SAFE_CONNECT(addNumber, SIGNAL(triggered()), SLOT(addMobileNumber()));

	smsMenu->addAction(addNumber);

	return smsMenu;

}

QMenu * QtContactManager::createFaxMenu(){
	Contact * pContact = this->getCurrentContact();
	QMenu * faxMenu = new QMenu(tr("Send Fax"));

	SAFE_CONNECT(faxMenu, SIGNAL(triggered(QAction *)), SLOT(sendFaxContact(QAction *)));

	Telephones& rPhones = pContact->getTelephones();

	if(rPhones.hasValid()){
	
		for ( Telephones::iterator it = rPhones.begin(); it != rPhones.end(); it++ )
		{
			if ( (*it).isValid() && (*it).getType() == QtEnumPhoneType::toString(QtEnumPhoneType::Fax).toStdString())
			{
				QString number = QString::fromStdString((*it).getNumber());
				QtEnumPhoneType::Type type = QtEnumPhoneType::toPhoneType(QString::fromStdString((*it).getType()));
				//QIcon icon(QtEnumPhoneType::getTypeIconPath(type));
				faxMenu->addAction(number);
			}
		}
		faxMenu->addSeparator();
	}
	
	QAction * addNumber =  new QAction(tr("Add Fax Number +"), this);
	SAFE_CONNECT(addNumber, SIGNAL(triggered()), SLOT(addFaxNumber()));

	faxMenu->addAction(addNumber);

	return faxMenu;

}

QMenu * QtContactManager::createMenu() {
	QMenu * menu = new QMenu(dynamic_cast <QWidget *> (parent()));
	Contact * pContact = this->getCurrentContact();
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);


	if(pContact){//VOXOX CHANGE by Rolando - 2009.10.15 
		QString contactName = QString::fromStdString(pContact->getDisplayName());//VOXOX CHANGE by Rolando - 2009.10.15 
		QAction* contactNameAction = menu->addAction(contactName);//VOXOX CHANGE by Rolando - 2009.10.15 
		contactNameAction->setEnabled(false);//VOXOX CHANGE by Rolando - 2009.10.15
		menu->addSeparator();//VOXOX CHANGE by Rolando - 2009.10.15 
	}


	//VOXOX - SEMR - 2009.06.16 CALL MENU
	QMenu * callMenu = createCallMenu();
	menu->addMenu(callMenu);

	//VOXOX - SEMR - 2009.06.18 SEND IM MENU
	//QMenu * sendIM = new QMenu(tr("Send IM"));
	//menu->addMenu(sendIM);
	QAction * sendIM = new QAction("Send IM", this);
	SAFE_CONNECT(sendIM, SIGNAL(triggered(bool)), SLOT(startChat()));
	if ( pContact && !pContact->hasIM() ) {
		sendIM->setEnabled(false);
	}
	menu->addAction(sendIM);

	//VOXOX - SEMR - 2009.06.18 SEND SMS MENU
	QMenu * sendSMS = createSMSMenu();
	menu->addMenu(sendSMS);

	//VOXOX - SEMR - 2009.06.18 SEND EMAIL MENU
	QMenu * sendEmail = createEmailMenu();
	menu->addMenu(sendEmail);

	//VOXOX - SEMR - 2009.06.18 VIEW SOCIAL PROFILE ACTION
	std::string socialPage = pContact->getContactSocialAddress();

	//VOXOX - SEMR - 2009.06.23 IF CONTACT HAVE SOCIAL PAGE
	if (!socialPage.empty()){
		QAction * viewSocialProfile =  new QAction("View Social Profile", this);
		SAFE_CONNECT(viewSocialProfile, SIGNAL(triggered(bool)), SLOT(openSocialPageContact()));
		menu->addAction(viewSocialProfile);
	}
	//VOXOX - SEMR - 2009.06.23 SEND FILE ACTION
	QAction * sendFile =  new QAction("Send File", this);
	SAFE_CONNECT(sendFile, SIGNAL(triggered(bool)), SLOT(sendFileContact()));
	menu->addAction(sendFile);
	
	//VOXOX - SEMR - 2009.06.23 SEND FAX ACTION
	QMenu * sendFax = createFaxMenu();
	menu->addMenu(sendFax);

	//VOXOX - SEMR - 2009.06.23 EDIT CONTACT ACTION
	QAction * editContact =  new QAction("Edit Contact", this);
	SAFE_CONNECT(editContact, SIGNAL(triggered(bool)), SLOT(editProfileContact()));
	menu->addAction(editContact);

	

	//VOXOX - SEMR - 2009.06.23 BLOCK / UNBLOCK ACTION
	QAction * unblockContact =  new QAction("Unblock Contact", this);
	SAFE_CONNECT(unblockContact, SIGNAL(triggered(bool)), SLOT(unblockContact()));

	QAction * blockContact =  new QAction("Block Contact", this);
	SAFE_CONNECT(blockContact, SIGNAL(triggered(bool)), SLOT(blockContact()));




	if (!pContact->getIsIMAccountVoxox()){//VOXOX - SEMR - 2009.06.23 Block not working on voxox
		bool isBlocked = pContact->isBlocked();
		if (isBlocked){
			menu->addAction(unblockContact);
		}
		else{
			menu->addAction(blockContact);
		}
	}

	//VOXOX - SEMR - 2009.06.23 DELETE CONTACT ACTION
	QAction * deleteContact =  new QAction("Delete Contact", this);
	SAFE_CONNECT(deleteContact, SIGNAL(triggered(bool)), SLOT(deleteContact()));
	menu->addAction(deleteContact);

	if (pContact->getIsIMAccountVoxox()){
		if(pContact->getStatusMessage()=="Not Authorized"){//VOXOX - CJC - 2009.07.31 TODO make this better, its not the best way to validate that the contact is not authorized, will need to research if Libpurple can return this info
			//VOXOX - CJC - 2009.07.31 
			QAction * reRequestAuthorizationAction =  new QAction(_trStringRERequestAuthorizationContact, this);
			SAFE_CONNECT(reRequestAuthorizationAction, SIGNAL(triggered(bool)), SLOT(reRequestAuthorization()));
			menu->addAction(reRequestAuthorizationAction);
		}
	}

	
	return menu;
}

void QtContactManager::deleteCurrentContact(){
	deleteContact();
}
void QtContactManager::editCurrentContact(){
	editProfileContact();
}
//VOXOX - SEMR - 2009.06.23 make the call
void QtContactManager::callNumber(QAction * action) {
	QString number = action->text();
	if(number!="Add Number +"){
		//VOXOX - CJC - 2009.06.26 THIS THING WILL CALL DEFAULT NUMBER< THE FUNCTIONALLY IS SUPOSED TO CALL THE NUMBER YOU SELECTED NOT THE DEFAULT ONE
		/*QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
		contactActionManager->callContact(QString::fromStdString(getCurrentItemContactKey()));*/
			//VOXOX - CJC - 2009.06.11 
		_cUserProfile.makeCall(number.toStdString());
	}
}
//VOXOX - SEMR - 2009.06.23 add new phone munber to contact
QString QtContactManager::addNumber() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	return contactActionManager->addNumberToContact(QString::fromStdString(getCurrentItemContactKey()));
}
//VOXOX - SEMR - 2009.06.23 send email to contact
void QtContactManager::sendEmailContact(QAction * action) {
	QString email = action->text();
	if(email!="Add Email Address +"){
	//VOXOX - CJC - 2009.06.26 THIS THING WILL CALL DEFAULT NUMBER< THE FUNCTIONALLY IS SUPOSED TO CALL THE NUMBER YOU SELECTED NOT THE DEFAULT ONE
		/*QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
		contactActionManager->callContact(QString::fromStdString(getCurrentItemContactKey()));*/
			//VOXOX - CJC - 2009.06.11 
		_cUserProfile.startIMToEmail(email.toStdString());
	}
}
//VOXOX - SEMR - 2009.06.23 add email to contact
QString QtContactManager::addEmail() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	return contactActionManager->addEmailToContact(QString::fromStdString(getCurrentItemContactKey()));
}
//VOXOX - SEMR - 2009.06.23 send sms to contact
void QtContactManager::sendSMSContact(QAction * action) {
	QString number = action->text();
	if(number!="Add Mobile Number +"){
		//VOXOX - CJC - 2009.06.26 THIS THING WILL CALL DEFAULT NUMBER< THE FUNCTIONALLY IS SUPOSED TO CALL THE NUMBER YOU SELECTED NOT THE DEFAULT ONE
		/*QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
		contactActionManager->callContact(QString::fromStdString(getCurrentItemContactKey()));*/
			//VOXOX - CJC - 2009.06.11 
		_cUserProfile.startIMToSMS(number.toStdString());
	}
}
//VOXOX - SEMR - 2009.06.23 add mobile to send sms to contact
QString QtContactManager::addMobileNumber() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	return contactActionManager->addMobileNumberToContact(QString::fromStdString(getCurrentItemContactKey()));
}
//VOXOX - SEMR - 2009.06.23 open social network page
void QtContactManager::openSocialPageContact() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->openSocialContactPage(QString::fromStdString(getCurrentItemContactKey()));
}
//VOXOX - SEMR - 2009.06.23 
void QtContactManager::sendFileContact() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->sendFileContact(QString::fromStdString(getCurrentItemContactKey()));
}
//VOXOX - SEMR - 2009.06.23 send fax to contact
void QtContactManager::sendFaxContact(QAction * action) {
	QString number = action->text();
	if(number!="Add Fax Number +"){
	//VOXOX - CJC - 2009.06.26 THIS THING WILL CALL DEFAULT NUMBER< THE FUNCTIONALLY IS SUPOSED TO CALL THE NUMBER YOU SELECTED NOT THE DEFAULT ONE
		/*QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
		contactActionManager->callContact(QString::fromStdString(getCurrentItemContactKey()));*/
		//VOXOX - CJC - 2009.06.11 
		_cUserProfile.sendFax(number.toStdString());
	}
}
//VOXOX - SEMR - 2009.06.23 add fax number to contact
QString QtContactManager::addFaxNumber() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	return contactActionManager->addFaxNumberToContact(QString::fromStdString(getCurrentItemContactKey()));
}
//VOXOX - SEMR - 2009.06.23 edit contact profile
void QtContactManager::editProfileContact() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->profileContact(QString::fromStdString(getCurrentItemContactKey()));
}

void QtContactManager::startMobileCall() 
{
	makeCall( getCurrentItemMobilePhone() );
}

void QtContactManager::startHomeCall() 
{
	makeCall( getCurrentItemHomePhone() );
}

void QtContactManager::startWorkCall() 
{
	makeCall( getCurrentItemWorkPhone() );
}

void QtContactManager::startWengoCall() 
{
	makeCall( getCurrentItemWengoPhoneNumber() );
}

Contact* QtContactManager::getContactById( const std::string contactId )
{
	return _cUserProfile.getCContactList().getContactById( contactId );
}

Contact* QtContactManager::getCurrentContact()
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 

	Contact* pContact = NULL;

	QTreeWidgetItem * item = _tree->currentItem();
	if ( item ) 
	{
		std::string contactId = item->text(0).toStdString();
		pContact = getContactById( contactId );
	}

	return pContact;
}
//VOXOX - SEMR - 2009.06.23 get contact key from current contact
std::string QtContactManager::getCurrentItemContactKey()
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 

	std::string result = "";
	Contact* pContact = getCurrentContact();

	if ( pContact )
	{
		result = pContact->getKey();
	}

	return result;
}

std::string QtContactManager::getCurrentItemContactId()
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 

	std::string result = "";
	Contact* pContact = getCurrentContact();

	if ( pContact )
	{
		result = pContact->getContactId();	//VOXOX - JRT - 2009.05.07 TODO: Is this an OK use of getContactId() or should it be getKey()?
	}

	return result;
}

std::string QtContactManager::getCurrentItemMobilePhone()
{
	std::string result = "";
	Contact* pContact = getCurrentContact();

	if ( pContact )
	{
		result = pContact->getMobilePhone();
	}

	return result;
}

std::string QtContactManager::getCurrentItemHomePhone()
{
	std::string result = "";
	Contact* pContact = getCurrentContact();

	if ( pContact )
	{
		result = pContact->getHomePhone();
	}

	return result;
}

std::string QtContactManager::getCurrentItemWorkPhone()
{
	std::string result = "";
	Contact* pContact = getCurrentContact();

	if ( pContact )
	{
		result = pContact->getWorkPhone();
	}

	return result;
}

std::string QtContactManager::getCurrentItemWengoPhoneNumber()
{
	std::string result = "";
	Contact* pContact = getCurrentContact();

	if ( pContact )
	{
		result = pContact->getWorkPhone();
	}

	return result;
}

std::string QtContactManager::getCurrentItemWebsite()
{
	std::string result = "";
	Contact* pContact = getCurrentContact();

	if ( pContact )
	{
		result = pContact->getWebsite();
	}

	return result;
}

void QtContactManager::makeCall( const std::string& number )
{
	_cUserProfile.makeCall( number );
}



void QtContactManager::goToWebsite() 
{
	std::string website = getCurrentItemWebsite();
	if (!website.find("http://")) 
	{
		website = "http://" + website;
	}

	WebBrowser::openUrl(website);
}

QTreeWidgetItem* QtContactManager::createGroupItem(const QString & contactGroupId, bool isFilterGroup) 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 

	//VOXOX - JRT - 2009.05.02 - NOTE: If you change setText(), be sure to adjst QtTreeViewDelegate::drawGroup().
	QString			 groupName = getGroupName( contactGroupId );		//VOXOX - JRT - 2009.05.01 
	QTreeWidgetItem* item = NULL;
	QList < QTreeWidgetItem * > list;

	if (groupsAreHidden() && !isFilterGroup )	//VOXOX - JRT - 2009.06.05 
	{
		list = _tree->findItems(QtContactList::DEFAULT_GROUP_NAME, Qt::MatchExactly);
	} 
	else 
	{
		list =_tree->findItems(contactGroupId, Qt::MatchExactly);
	}

	//If no group exists, create the group
	if (list.isEmpty()) 
	{
		item = new QTreeWidgetItem(_tree);

		//VOXOX - JRT - 2009.05.01 
		QtContactListManager* lm		= QtContactListManager::getInstance();
		QtContact*			  qtContact = new QtContact( contactGroupId.toStdString(), "", _cWengoPhone, _tree, true);
		bool				  bAdded	= lm->addContact(qtContact);
		setItemData( item, qtContact );
		//End VoxOx


//		if (groupsAreHidden()) 
		if (groupsAreHidden() && !isFilterGroup )	//VOXOX - JRT - 2009.06.05 
		{
			item->setText(0, QtContactList::DEFAULT_GROUP_NAME);
		} 
		else 
		{
			item->setText(0, contactGroupId);
		}

		bool bOpen = isGroupOpen( contactGroupId );

		_tree->setItemExpanded( item, bOpen );
	}
	else 
	{
		item = list[0];
	}

	return item;
}

//VOXOX - JRT - 2009.04.07 - To improve initial data load.
void QtContactManager::setInitialLoad( bool bSet )	
{ 
	if ( _initialLoad != bSet )
	{
		_initialLoad = bSet;

		if ( _tree )
		{
			_tree->setUpdatesEnabled(!bSet);

			if ( !bSet )
			{
				//VOXOX - JRT - 2009.08.20 - We are getting blank contactlist intermittently.
				_tree->update();
				_tree->repaint();
//				redrawContacts();	//VOXOX - JRT - 2009.08.20 - contains setInitialLoad() so causes unnecesssary redraw.
			}
		}
	}
}

QString QtContactManager::getGroupName( const QString& contactGroupId )
{
	QString groupName = "";
	
	ContactGroup* pGroup = _cUserProfile.getCContactList().getContactGroup( contactGroupId.toStdString() );

	if ( pGroup )
	{
		groupName = QString( pGroup->getName().c_str() );
	}

	return groupName;
}

bool QtContactManager::isGroupOpen( const QString& contactGroupId )
{
	bool bOpen = true;
	
	ContactGroup* pGroup = _cUserProfile.getCContactList().getContactGroup( contactGroupId.toStdString() );

	if ( pGroup )
	{
		bOpen = pGroup->isOpen();
	}

	return bOpen;
}

//VOXOX - JRT - 2009.05.20 - This should move to VoxContactTreeWidget
void QtContactManager::setGroupOpen( const QString& groupId, bool bOpen )
{
	ContactGroup* pGroup = _cUserProfile.getCContactList().getContactGroup( groupId.toStdString() );

	if ( pGroup )
	{
		pGroup->setIsOpen( bOpen );
	}
}

void QtContactManager::addContact( QtContact* qtContact ) 
{
	Contact* pContact1 = qtContact->getContact();

	if ( pContact1 )
	{
		std::string groupId  = qtContact->getGroupKey();
		bool		isFilter = pContact1->getContactGroupInfoSet().IsFilter( groupId );

		addContact( qtContact, groupId, isFilter );
	}
	else
	{
		assert(false);
	}
}

bool QtContactManager::shouldUseGroup( const ContactGroup& cg, bool& isFilter )
{
	Config&						 config  = ConfigManager::getInstance().getCurrentConfig();
	QtEnumSortOption::SortOption eOption = (QtEnumSortOption::SortOption)config.getContactGrouping();
	EnumGroupType::GroupType	 grpType = cg.getType();

	bool bAddUserGroup = (grpType == EnumGroupType::GroupType_User);
	bool bAdd = false;

	//VOXOX - JRT - 2009.09.03 
	if ( bAddUserGroup )
	{
		int nTotal  = 0;
		int nOnline = 0;

		const_cast<ContactGroup&>(cg).getCounts( nTotal, nOnline );

		//If Online contact count == 0, only show the group if there ZERO contact in it.
		//This ensures user will see empty groups, but not those that are only empty because
		// they are not viewing offline contacts.
		if ( nOnline == 0 )
		{
			bAddUserGroup = (nTotal == 0);
		}
	}

	if ( cg.isFilter() )
	{
		//Use the Filter group only if we have an active filter.
		bAdd	 = 	_cUserProfile.getCContactList().getContactList().isFiltering();
		isFilter = true;
	}
	else
	{
		isFilter = false;

		switch( eOption )
		{
		case QtEnumSortOption::GroupByNetwork:
			bAdd = (grpType == EnumGroupType::GroupType_OrigNetwork);
			break;

	//	case QtEnumSortOption::GroupByMostComm:
	//		bAdd = (grpType == EnumGroupType::GroupType_MostComm);
	//		break;

		case QtEnumSortOption::GroupByUserGroup:
			bAdd = bAddUserGroup;
			break;

		case QtEnumSortOption::GroupByNone:
			bAdd = bAddUserGroup;
			break;

		//If you get any of these options, your config.xml has old/invalid values.
		//  So default to a GroupByUser and reset Config value
		case QtEnumSortOption::SortAlpha:
		case QtEnumSortOption::SortPresence:
		case QtEnumSortOption::SortRandom:
		case QtEnumSortOption::SortGroupAlpha:
		case QtEnumSortOption::SortGroupManual:
		case QtEnumSortOption::ViewUnavailable:
		case QtEnumSortOption::AddContact:
			config.set( Config::GENERAL_CONTACT_GROUPING_KEY, QtEnumSortOption::GroupByUserGroup);
			bAdd = bAddUserGroup;
			break;

		default:			//New type
			assert(false);
			bAdd = bAddUserGroup;
			break;
		}
	}

	return bAdd;
}

bool QtContactManager::shouldUseGroup( const ContactGroupInfo& cgi, const std::string& groupKey, bool& isFilter )
{
	Config& config = ConfigManager::getInstance().getCurrentConfig();
	QtEnumSortOption::SortOption eOption = (QtEnumSortOption::SortOption)config.getContactGrouping();

	bool bAddUserGroup = false;
		
	if (cgi.getType() == EnumGroupType::GroupType_User)
	{
		bAddUserGroup = (cgi.getKey() == groupKey);
	}


	bool bAdd = false;

	if ( cgi.isFilter() )
	{
		bAdd	 = true;		//Always use the Filter group.
		isFilter = true;
	}
	else
	{
		isFilter = false;

		switch( eOption )
		{
		case QtEnumSortOption::GroupByNetwork:
			bAdd = cgi.getType() == EnumGroupType::GroupType_OrigNetwork;
			break;

	//	case QtEnumSortOption::GroupByMostComm:
	//		bAdd = cgi.getType() == EnumGroupType::GroupType_Network;
	//		break;

		case QtEnumSortOption::GroupByUserGroup:
			bAdd = bAddUserGroup;
			break;

		case QtEnumSortOption::GroupByNone:
			bAdd = bAddUserGroup;
			break;

		//If you get any of these options, your config.xml has old/invalid values.
		//  So default to a GroupByUser and reset Config value
		case QtEnumSortOption::SortAlpha:
		case QtEnumSortOption::SortPresence:
		case QtEnumSortOption::SortRandom:
		case QtEnumSortOption::SortGroupAlpha:
		case QtEnumSortOption::SortGroupManual:
		case QtEnumSortOption::ViewUnavailable:
		case QtEnumSortOption::AddContact:
			config.set( Config::GENERAL_CONTACT_GROUPING_KEY, QtEnumSortOption::GroupByUserGroup);
			bAdd = bAddUserGroup;
			break;

		default:			//New type
			assert(false);
			bAdd = bAddUserGroup;
			break;
		}
	}

	return bAdd;
}

void QtContactManager::addContact( QtContact* qtContact, const std::string& groupId, bool isFilterGroup ) 
{
	//Debug code - I'll remove later.
//	if ( groupId == "group1user" )
//		int xxx = 1;			//JRT-XXX
//
//	if ( groupId == "group2user" )
//		int xxx = 1;			//JRT-XXX


	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 

	QTreeWidgetItem* groupItem  = createGroupItem(QString::fromStdString( groupId ), isFilterGroup);

	QTreeWidgetItem * item = new QTreeWidgetItem();
	item->setText(0, qtContact->getId()  );				//VOXOX - JRT - 2009.08.10 OK use of getId().
	item->setFlags(item->flags() | Qt::ItemIsEditable);

	setItemData( item, qtContact );

	if ( _qtContactList.getInitialLoad() )
	{
		groupItem->addChild( item );
	}
	else
	{
		int insertIndex = findInsertPoint( groupItem, qtContact );	
		groupItem->insertChild(insertIndex, item);
	}
}

int QtContactManager::findInsertPoint( QTreeWidgetItem* groupItem, QtContact* qtContact )
{
	int insertIndex  = 0;

	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 
	_tree->lock();								//VOXOX - JRT - 2009.07.20 

	if ( groupItem && qtContact )
	{
		int contactCount = groupItem->childCount();
		int contactIndex = 0;
		
		insertIndex  = contactCount;

		QString					contactKey		= qtContact->getKey().toUpper();	//VOXOX - JRT - 2009.08.10 
		QtContactListManager *	ul				= QtContactListManager::getInstance();
		QTreeWidgetItem *		childItem		= NULL;
		QtContact*				childQtContact	= NULL;

		 for (contactIndex = 0; contactIndex < contactCount; ++contactIndex) 
		 {
			childItem      = groupItem->child(contactIndex);
			childQtContact = getQtContactFromItem( childItem );

			if ( childQtContact )		//VOXOX - JRT - 2009.06.24 - Prevent crash.
			{
				if (childQtContact->getKey() > contactKey )	//VOXOX - JRT - 2009.08.10 
				{
					insertIndex = contactCount;
					break;
				}
			}
		}
	}

	 _tree->unlock();						//VOXOX - JRT - 2009.07.20 

	 return insertIndex;
}

void QtContactManager::removeContact(const QString & contactId ) 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 
	_tree->lock();								//VOXOX - JRT - 2009.07.12 

	QTreeWidgetItem* contactItem = findContactItem(contactId);

	if (contactItem) 
	{
		removeContact( contactItem );
	}

	_tree->unlock();
}

//VOXOX - JRT - 2009.05.20 - TODO: move to VoxContactTreeWidget
void QtContactManager::removeContact( QTreeWidgetItem* item )
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 
	_tree->lock();								//VOXOX - JRT - 2009.07.12 

	QtContactListManager*	ul		  = QtContactListManager::getInstance();
	QtContact*				qtContact = this->getQtContactFromItem( item );

	if ( item )
	{
		QTreeWidgetItem*		groupItem = item->parent();

		_tree->removeItemWidget( item, 0 );	//VOXOX - JRT - 2009.04.17 
		delete item;
	}

	if (qtContact) 
	{
		ul->removeContact(qtContact);
	} 
	else 
	{
		LOG_WARN("Could not find contact to remove in QtContactListManager");
	}

#if QT_VERSION < 0x40300
	// This is needed on Qt 4.2 and lower to workaround this bug:
	// Assuming the list looks like this:
	// ( ) A
	// ( ) B
	//     +---+ ...
	//     |   | ...
	//     +---+
	//
	// if 'A' gets hidden, the 'B' line moves up, but its QtContactWidget stays
	// were it was.
	_tree->doItemsLayout();
#endif

	//VOXOX - JRT - 2009.08.26 - We now show empty groups.
//	if (groupItem->childCount() == 0) 
//	{
//		_tree->removeItemWidget( groupItem, 0 );
//		delete groupItem;							//Don't we have to remove from tree?
//	}

	_tree->unlock();								//VOXOX - JRT - 2009.07.12 
}

//VOXOX - JRT - 2009.08.10 - Not needed after implementing Multiple Groups
//void QtContactManager::moveContact(const std::string & dstGroupId, const std::string & srcGroupId, const std::string & contactId) 
//{
//	LOG_DEBUG("moving contact=" + contactId + " from=" + srcGroupId + " to=" + dstGroupId);
//
//	//If groups are hidden, there is nothing to move...
//	if (groupsAreHidden()) 
//	{
//		return;
//	}
//
//	//Looking for the contact in the destination group
//	//If it is inside this group nothing is done
//	QTreeWidgetItem * group = findGroupItem(QString::fromStdString(dstGroupId));
//	if (!group) 
//	{
//		return;
//	}
//
//	if (findContactInGroup(group, QString::fromStdString(contactId))) 
//	{
//		return;
//	}
//
//	//VOXOX - JRT - 2009.08.10 - We never get here anymore.  It appears this is handled in other ways.
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 
//
//	removeContact(QString::fromStdString(contactId));
////	addContact(QString::fromStdString(contactId));
//	handleContactChanged( contactId, dstGroupId, true );		//VOXOX - JRT - 2009.08.09 
//}

QTreeWidgetItem * QtContactManager::findGroupItem(const QString & groupId) const 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 

	QList < QTreeWidgetItem * > list = _tree->findItems(groupId, Qt::MatchExactly);
	if (list.isEmpty()) 
	{
		return NULL;
	}

	return list[0];
}

QTreeWidgetItem * QtContactManager::findContactItem(const QString & contactId) const 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 

	QList < QTreeWidgetItem * > list = _tree->findItems(contactId, Qt::MatchExactly | Qt::MatchRecursive);
	if (list.isEmpty()) 
	{
		return 0;
	}

	return list[0];
}

//QtContact * QtContactManager::findContactInTreeByKey(const QString & contactKey ) const 
//{
//	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 
//
//	QtContact*	result = NULL;
//
//	_tree->lock();
//
//	const QTreeWidgetItem * group, 
//	int count = group->childCount();
//
//	for (int i = 0; i < count; i++) 
//	{
//		QtContact* qtContact = getQtContactFromItem( group->child(i) );	//VOXOX - JRT - 2009.08.10 
//
//		if ( qtContact )
//		{
//			if ( qtContact->getKey() == contactKey() )
//			{
//				result = qtContact;
//				break;
//			}
//		}
//	}
//
//	return qtContact;
//}

void QtContactManager::timerEvent(QTimerEvent * event) 
{
	if (event->timerId() == _sortTimerId) 
	{
//		killTimer(_sortTimerId);
//		_sortTimerId = -1;
//		_canSort     = true;

		bool sortIt = _wantSort;

		if ( !sortIt )
		{
			if ( _needsRefresh )
			{
				sortIt = (_refreshTimer.GetCurrSeconds() * 1000 > _refreshInterval);

				if ( sortIt )
				{
					_needsRefresh = false;
				}
			}
		}

		if (sortIt) 
		{
			safeSortContacts(true);
			_wantSort = false;
		}
	}
	else if (event->timerId() == _showTimerId) 
	{
//		killTimer(_showTimerId);
//		_showTimerId = -1;
//		_canShow     = true;

		bool showIt = _wantShow;

		if ( !showIt )
		{
			if ( _needsReload )
			{
				showIt = (_reloadTimer.GetCurrSeconds() * 1000 > _reloadInterval);

				if ( showIt )
				{
					_needsReload = false;
				}
			}
		}

		if ( showIt ) 
		{
			redrawContacts();
			_wantShow = false;
		}
	}
	else
	{
	QObject::timerEvent(event);
	}
}

bool QtContactManager::groupsAreHidden() const 
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return QtEnumSortOption::groupsAreHidden( config.getContactGrouping() );
}

void QtContactManager::languageChanged() 
{
	_trStringCallContact		= tr("Call Contact");
	_trStringSendEmail			= tr("Send Email");
	_trStringSendSMS			= tr("Send SMS");
	_trStringSendFax			= tr("Send Fax");
	_trStringSendIM				= tr("Send IM");
	_trStringSocialProfile		= tr("View Social Profile");
	_trStringSendFile			= tr("Send File");
	_trStringEditContact		= tr("Edit contact");
	_trStringDeleteContact		= tr("Delete contact");
	_trStringBlockContact		= tr("Block contact");
	_trStringUnblockContact		= tr("Unblock contact");
	_trStringInviteToConference = tr("Invite to Conference");
	_trStringRERequestAuthorizationContact = tr("Re-Request Authorization");//VOXOX - CJC - 2009.07.31 
}

//VOXOX - JRT - 2009.10.15 - This probably needs more work.
//std::string QtContactManager::determineUseableGroupId( const std::string& contactId, const std::string& groupIdIn )
//{
//	std::string groupId = groupIdIn;
//
//	if ( groupsAreHidden() )
//	{
//		groupId = "";
//	}
//	else
//	{
//		Contact*	  contact		= _qtContactList.getCContactList().getContactByKey( contactId );
//		ContactGroup* pGroup		= _cUserProfile.getCContactList().getContactGroup( groupIdIn );
//		bool		  isFilterGroup = false;
//
//		if ( pGroup )
//		{
//			ContactGroupInfo cgi( pGroup->getKey(), pGroup->getType() );
//
//			if ( pGroup->isFilter() )
//			{
//				groupId = pGroup->getKey();
//			}
//			else if ( !shouldUseGroup( const_cast<ContactGroup&>(*pGroup), isFilterGroup ) )
//			{
//				groupId = determineGroupToUse( 
//				//else if ( ( cgi, groupIdIn, isFilterGroup ) )
//				//{
//				//	groupId = pGroup->getKey();
//				//}
//			}
//			else
//			{
//				int xxx = 1;
//			}
//		}
//	}
//
//	return groupId;
//}


std::string QtContactManager::determineUseableGroupId( const std::string& contactId, const std::string& groupIdIn )
{
	std::string					 groupId = groupIdIn;
	Config&						 config  = ConfigManager::getInstance().getCurrentConfig();
	QtEnumSortOption::SortOption eOption = (QtEnumSortOption::SortOption)config.getContactGrouping();
	
	Contact*					 contact = _qtContactList.getCContactList().getContactByKey( contactId );
	ContactGroup*				 cg		 = _cUserProfile.getCContactList().getContactGroup ( groupIdIn );

	if ( cg && cg->isFilter() )
	{
		//Use the Filter group only if we have an active filter.
		if ( !_cUserProfile.getCContactList().getContactList().isFiltering() )
		{
			groupId = contact->getContactGroupInfoSet().GetFilterGroupKey();
		}
	}
	else
	{
		switch( eOption )
		{
		case QtEnumSortOption::GroupByNetwork:
			groupId = contact->getContactGroupInfoSet().GetOrigNetworkGroupKey();
			break;

	//	case QtEnumSortOption::GroupByMostComm:
	//		bAdd = (grpType == EnumGroupType::GroupType_MostComm);
	//		break;

		case QtEnumSortOption::GroupByUserGroup:
			groupId = groupIdIn;
			break;

		case QtEnumSortOption::GroupByNone:
			groupId = "";		//Not sure this is correct.
			break;

		default:			//New type
			assert(false);
			break;
		}
	}

	return groupId;
}

void QtContactManager::handleContactChanged(const std::string& contactId, const std::string& groupIdIn, bool bMoved ) 
{
	//VOXOX - JRT - 2009.05.15 - These are just static counters to evaluate the number of events triggered.
	static int s_nCount   = 0;
	static int s_nAdds    = 0;
	static int s_nUpdates = 0;
	static int s_nUpdateSkips = 0;
	static int s_nRemoves = 0;
	static int s_nNoShow  = 0;

	static std::string	s_prevContactId = "";

	s_nCount++;

	setNeedsReload();			//VOXOX - JRT - 2009.10.14 
	setNeedsRefresh();

	std::string groupId = determineUseableGroupId( contactId, groupIdIn );		//VOXOX - JRT - 2009.10.15 - Useable groupId depends on group/sort options.
	QtContactListManager* lm = QtContactListManager::getInstance();

	//We only care about this if we are showing this contact.
	if ( canShowUser(contactId) )
	{

		//Add QtContact to QtContactListManager.  
		//	This will determine if we have an 'add' or an 'update'.  
		//  We only have 'remove' in case of canShowUser() == false.
		QtContact*			  qtContact = new QtContact( contactId, groupId, _cWengoPhone, _tree, false);	//Do NOT delete this ptr.  It is owned by lm.
		bool				  bAdded	= lm->addContact(qtContact);

		if ( bAdded )
		{
			setNeedsReload();			//VOXOX - JRT - 2009.10.14 
		}

		//VOXOX - JRT - 2009.08.14 - We may get a new contact while grouped by Networks
		bool isFilterGroup = false;
		ContactGroupInfo* cgi = qtContact->getContactGroupInfo();
		if ( cgi )
		{
			if ( !shouldUseGroup( *cgi, groupId, isFilterGroup ) )
			{
				return;
			}
		}

		//Setting initialLoad before addContact speeds up the Tree inserts immensely.
		//  It also allows us to skip updates which aren't being redrawn any.
		if ( bAdded )
		{
			addContact( qtContact );
			s_nAdds++;
		}
		else
		{
			//VOXOX - JRT - 2009.08.08 - Multiple group support
			QTreeWidgetItem* parentItem = qtContact->getTreeWidgetItem()->parent();
			if ( parentItem )
			{
				QtContact* qtParent = this->getQtContactFromItem( parentItem );
				if ( qtParent )
				{
					qtContact->setGroupKey( qtParent->getId().toStdString() );	//VOXOX - JRT - 2009.08.10 - OK use of getId().
				}
			}

			//This needs to be in lm->addContact(qtContact).
			setItemData( qtContact->getTreeWidgetItem(), qtContact );

			//These updates were captured in lm->addContact() above.
			//So after initialLoad, we should have most current data to display.
			if ( !_initialLoad )
			{
				//The data has been updated already.  This would just redraw the Tree item.
				updateContact( qtContact );
				s_nUpdates++;

				if ( bMoved )
				{
//					redrawContacts();
					setNeedsReload();		//VOXOX - JRT - 2009.10.14 
				}
			}
			else
			{
				s_nUpdateSkips++;
			}
	
			setNeedsRefresh();			//VOXOX - JRT - 2009.10.14 
		}
	}
	else
	{
		s_nNoShow++;

		//Check if we need to remove a previously added contact.
		//This may be because of PresenceStatus change.
		QtContact qtContact( contactId, groupId, _cWengoPhone, _tree, false);
//		QtContact* qtTemp = lm->getContact( qtContact.getKey() );
		//VOXOX - JRT - 2009.08.26 
		QTreeWidgetItem* item = _tree->getItemByContactKey( qtContact.getKey().toStdString(), true );

//		if ( qtTemp )
		if ( item )
		{
//			removeContact( qtTemp->getTreeWidgetItem() );
			removeContact( item );
			s_nRemoves++;
			setNeedsReload();			//VOXOX - JRT - 2009.10.14 
		}
	}
}

void QtContactManager::updateContact( QtContact* qtContact ) 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.06.24 

	//QtContact should contain the TreeWidget item.
	QTreeWidgetItem* item = qtContact->getTreeWidgetItem();

	if ( item )
	{
		assert( item->text(0) == qtContact->getId() );	//VOXOX - JRT - 2009.08.10 - OK use of getId().

		// Repaint only the item. We adjust the rect to avoid repainting neighbour items. 
		// It does not matter because QtContact::paint repaint the whole item, regardless of the area to update.
		QRect rect = _tree->visualItemRect(item);
		rect.adjust(0, 1, 0, -1);
		_tree->viewport()->update(rect);


		//VOXOX - JRT - 2009.04.16 - This always returns NULL.
		QtContactWidget* widget = _tree->findChild<QtContactWidget*>();
		if (widget) 
		{
			widget->updateButtons();
			widget->update();
		}
		else
		{
			int xx = 1;
		}
	}
	else
	{
//		assert(false);
		int xxx = 1;
	}
}

//VOXOX - JRT - 2009.05.15 - Move to new VoxContactListWidget class.
void  QtContactManager::setItemData( QTreeWidgetItem* item, QtContact* qtContact )
{
	RecursiveMutex::ScopedLock lock(_mutex);

	if ( item )
	{
		item->setData( 0, Qt::UserRole, qVariantFromValue(qtContact) );
		qtContact->setTreeWidgetItem( item );
	}
}

QtContact* QtContactManager::getQtContactFromItem( QTreeWidgetItem* item ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	QtContact* qtContact = NULL;

	if ( item )
	{
		qtContact = item->data( 0, Qt::UserRole ).value<QtContact*>();
	}

	return qtContact;
}
//End VoxOx

//VOXOX - JRT - 2009.10.14 - This method set flag indicating we need to update Contact List, 
//							 and starts/restarts an elapsed timer so we can tell how long it has been since most recent update.
void QtContactManager::setNeedsRefresh()
{
	if ( !isDrawing() )
	{
		RecursiveMutex::ScopedLock lock(_mutex);
		
		_needsRefresh = true;

		_refreshTimer.Restart();
	}
}

void QtContactManager::setNeedsReload()
{
	//if ( !isDrawing() )
	//{
	//	RecursiveMutex::ScopedLock lock(_mutex);
	//
	//	_needsReload = true;

	//	_reloadTimer.Restart();
	//}
}
