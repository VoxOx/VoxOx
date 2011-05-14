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

#include "stdafx.h"	
#include "QtChatRoomInviteDlg.h"
#include "QtChatRoomTreeWidgetItem.h"
#include "QtChatRoomListWidgetItem.h"

#include "ui_ChatRoomInviteWidget.h"

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/WidgetFactory.h>
#include <qtutil/Object.h>

#include <model/contactlist/ContactList.h>
#include <model/contactlist/ContactProfile.h>
#include <model/contactlist/ContactGroup.h>
#include <presentation/qt/messagebox/QtVoxMessageBox.h>	//VOXOX - CJC - 2009.10.06 
#include <QtGui/QHeaderView>
#include <QtGui/QtGui>

//-----------------------------------------------------------------------------

QtChatRoomInviteDlg::QtChatRoomInviteDlg(IMChatSession & chatSession, CContactList & cContactList, QWidget * parent , Qt::WFlags f ) :
	QDialog(parent,f), _cContactList(cContactList), _chatSession(chatSession) 
{
	_includeOnlyOnline = false;
	_sameNetworkOnly   = true;

	_chatRoomInviteWidget = new QWidget(this);
	_ui = new Ui::ChatRoomInviteWidget();
	_ui->setupUi(_chatRoomInviteWidget);

	QGridLayout * layout = new QGridLayout();
	layout->addWidget(_chatRoomInviteWidget);
	layout->setMargin(0);
	setLayout(layout);
	setupGui();

	initData();
}

//-------------------------------------------------------------------------

QtChatRoomInviteDlg::~QtChatRoomInviteDlg() 
{
	OWSAFE_DELETE(_ui);
	OWSAFE_DELETE(_chatRoomInviteWidget);
}

//-----------------------------------------------------------------------------

void QtChatRoomInviteDlg::setupGui() {
	_contactListTreeWidget	= _ui->contactListTreeWidget;
	_inviteListWidget		= _ui->inviteListWidget;
	_lineEditChatRoomName	= _ui->lineEditConvName;
	_addPushButton			= _ui->addPushButton;
	_removePushButton		= _ui->removePushButton;
	_startPushButton		= _ui->startPushButton;
	_cancelPushButton		= _ui->cancelPushButton;

	SAFE_CONNECT (_addPushButton,	  SIGNAL(clicked()), SLOT(addToConference()));
	SAFE_CONNECT (_removePushButton,  SIGNAL(clicked()), SLOT(removeFromConference()));
	SAFE_CONNECT (_startPushButton,   SIGNAL(clicked()), SLOT(startConference()));
	SAFE_CONNECT (_cancelPushButton,  SIGNAL(clicked()), SLOT(cancel()));
	
	_contactListTreeWidget->header()->hide();	// Remove the column header

	resize( 528,354 );
}

//-----------------------------------------------------------------------------

void QtChatRoomInviteDlg::startConference() 
{
	if ( validData() )
	{
		_chatSession.getGroupChatInfo().setAlias( _chatRoomName );

		//VOXOX - JRT - 2009.06.13 - Caller can access the Participant list and handle invites.
	//	// _chatSession
	//	QList<QListWidgetItem *>selectList =  _inviteListWidget->findItems("*",Qt::MatchWildcard);
	//	QList<QListWidgetItem *>::iterator it;
	//
	//	for (it = selectList.begin(); it!= selectList.end(); it++) 
	//	{
	//		QtChatRoomListWidgetItem * item = dynamic_cast<QtChatRoomListWidgetItem *> (*it);
	////		_chatSession.addIMContact(*(item->getContact().getFirstAvailableIMContact(_chatSession)));
	////		_selectedContact.append((item->getContact()));
	//	}

		accept();
	}
 }

//-----------------------------------------------------------------------------

void QtChatRoomInviteDlg::cancel() 
{
	reject();
}

//-----------------------------------------------------------------------------

void QtChatRoomInviteDlg::addToConference() 
{
	QList<QTreeWidgetItem *> selectList =  _contactListTreeWidget->selectedItems ();
	QList<QTreeWidgetItem *>::iterator it;

	for (it = selectList.begin(); it!= selectList.end(); it++) 
	{
		if ( (*it)->childCount() == 0 ) 
		{
			QtChatRoomTreeWidgetItem* item = dynamic_cast<QtChatRoomTreeWidgetItem *>((*it));

			Participant p = item->getParticipant();
			_candidates.setInvitedToChat( p, true );
		}
	}

	updateListCtrls();
}

//-----------------------------------------------------------------------------

void QtChatRoomInviteDlg::removeFromConference() 
{
	QList<QListWidgetItem *>selectList = _inviteListWidget->selectedItems();
	QList<QListWidgetItem *>::iterator it;

	for (it= selectList.begin(); it!= selectList.end(); it++ ) 
	{
		QtChatRoomListWidgetItem* item = dynamic_cast<QtChatRoomListWidgetItem *>((*it));

		Participant p = item->getParticipant();

		if ( !p.isAlreadyInChat() )
		{
			_candidates.setInvitedToChat( p, false );
		}
	}

	updateListCtrls();
}

//-----------------------------------------------------------------------------

void QtChatRoomInviteDlg::initData()
{
	//Only reuse chatroom if this is aleady a GroupChat.
	if ( _chatSession.isGroupChat() )
	{
		_chatRoomName = _chatSession.getGroupChatInfo().getChatRoom();
		std::string::size_type pos = _chatRoomName.find_first_of("@");

		if ( pos >= 0 )
		{
			_chatRoomName = _chatRoomName.substr( 0, pos );
		}

		_lineEditChatRoomName->setReadOnly( true );		//Don't allow user to change.
	}
	else
	{
		_chatRoomName = "";
		_lineEditChatRoomName->setReadOnly( false );
	}

	_lineEditChatRoomName->setText( QString(_chatRoomName.c_str() ) );

	populateCandidates();

	updateListCtrls();
}

//-----------------------------------------------------------------------------

bool QtChatRoomInviteDlg::validData()
{
	bool result = false;

	QString tempRoomName = _lineEditChatRoomName->text();
	//VOXOX - CJC - 2009.10.06 Remove spaces and make it lower case
	tempRoomName = tempRoomName.trimmed();
	tempRoomName = tempRoomName.toLower();
	tempRoomName = tempRoomName.replace(" ","_");

	_chatRoomName = tempRoomName.toStdString();
	_lineEditChatRoomName->setText( QString(_chatRoomName.c_str() ) );

	if ( _chatRoomName.empty() )
	{

			QtVoxMessageBox box(this);
			box.setWindowTitle(tr("VoxOx"));
			box.setText(tr("Chat room name cannot be empty"));
			box.setStandardButtons(QMessageBox::Ok);
			box.exec();

		//QMessageBox::warning( this, tr("Voxox"),  );//VOXOX - CJC - 2009.10.06 
	}
	else
	{
		result = true;
	}

	return result;
}

//-----------------------------------------------------------------------------

void QtChatRoomInviteDlg::populateCandidates() 
{
	_candidates.setIncludeOnlyOnline( _includeOnlyOnline );
	_candidates.setSameNetworkOnly  ( _sameNetworkOnly   );

	_candidates.populate( _cContactList.getContactList().getContacts(), _chatSession );
}

//-----------------------------------------------------------------------------

void QtChatRoomInviteDlg::updateListCtrls()
{
	updateCandidateListCtrl();
	updateInviteeListCtrl();
}

//-----------------------------------------------------------------------------

void QtChatRoomInviteDlg::updateCandidateListCtrl()
{
	_contactListTreeWidget->clear();

	//Create Group.
//	QTreeWidgetItem* groupItem = createGroup( "All" );
	QTreeWidgetItem* groupItem = _contactListTreeWidget->invisibleRootItem();

	//Add to Group.
	for ( ParticipantList::const_iterator it = _candidates.begin(); it != _candidates.end(); it++ )
	{
		if ( (*it).isCandidate() )
		{
			QtChatRoomTreeWidgetItem* item = new QtChatRoomTreeWidgetItem ( (*it), groupItem );
			item->setText(0, QString::fromStdString ((*it).getDisplayName()));
			item->setIcon(0, QIcon(QtContactPixmap::getInstance()->getPixmap( (*it).getPixmap() )));
		}
	}
}

//-----------------------------------------------------------------------------

void QtChatRoomInviteDlg::updateInviteeListCtrl()
{
	_inviteListWidget->clear();

	for ( ParticipantList::const_iterator it = _candidates.begin(); it != _candidates.end(); it++ )
	{
		if ( (*it).isInvitee() )
		{
			QtChatRoomListWidgetItem * listItem = new QtChatRoomListWidgetItem((*it), _inviteListWidget);

			listItem->setText( QString::fromStdString( (*it).getDisplayName() ) );
			listItem->setIcon( QIcon(QtContactPixmap::getInstance()->getPixmap( (*it).getPixmap() )));
		}
	}
}

//-----------------------------------------------------------------------------

QTreeWidgetItem* QtChatRoomInviteDlg::createGroup( const QString& groupName )
{
	QTreeWidgetItem* groupItem = new QTreeWidgetItem( _contactListTreeWidget );
	groupItem->setFlags(Qt::ItemIsEnabled);

	groupItem->setText(0, groupName);

	return groupItem;
}

//-----------------------------------------------------------------------------

//=============================================================================

Participant::Participant()
{
	initVars();
}

//-----------------------------------------------------------------------------

Participant::~Participant()
{
}

//-----------------------------------------------------------------------------

void Participant::initVars()
{
	setDisplayName( "" );
	setIsAlreadyInChat( false );
	setInvitedToChat  ( false );
	setPixmap( QtContactPixmap::ContactUnknown );
}

//-----------------------------------------------------------------------------

Participant& Participant::operator=( const Participant& src )
{
	if ( this != &src )
	{
		setIMContact      ( src.getIMContact()	  );
		setDisplayName	  ( src.getDisplayName()  );
		setPixmap		  ( src.getPixmap()		  );
		setIsAlreadyInChat( src.isAlreadyInChat() );
		setInvitedToChat  ( src.isInvitedToChat() );
	}

	return *this;
}

//-----------------------------------------------------------------------------
	
bool Participant::operator==( const Participant& src )
{
	return ( _imContact == src._imContact );
}

//-----------------------------------------------------------------------------

bool Participant::isCandidate() const
{
	bool bIs = (!isAlreadyInChat() && !isInvitedToChat() );
	return bIs;
}

//-----------------------------------------------------------------------------

bool Participant::isInvitee() const
{
	return !isCandidate();
}

//-----------------------------------------------------------------------------

bool Participant::canBeUninvited() const
{
	return (! isAlreadyInChat());
}

//=============================================================================
	
ParticipantList::ParticipantList()
{
}

//-----------------------------------------------------------------------------

void ParticipantList::populate( const Contacts& rContacts, IMChatSession& imChatSession )
{
	Participant participant;
	IMContact*  imContact = NULL;
	bool		bAlreadyInChat = false;

	for ( Contacts::const_iterator it = rContacts.begin(); it != rContacts.end(); it++ )
	{
		if ( includeInCandidateList( (*it).second, imChatSession, bAlreadyInChat ) )
		{
			Contact c = (*it).second;
			imContact = c.getPreferredIMContact();

			participant.setIMContact      ( *imContact			 );
			participant.setDisplayName	  ( c.getDisplayName()   );
			participant.setIsAlreadyInChat( bAlreadyInChat		 );
			participant.setPixmap		  ( determineStatus( c ) );

			Add( &participant );
		}
	}
}

//-----------------------------------------------------------------------------

bool ParticipantList::includeInCandidateList( const ContactProfile& contactProfile, const IMChatSession& chatSession, bool& bInChatSession )
{
	bool			  bInclude  = true;
	const IMContact*  imContact = NULL;

	bInChatSession = false;

	if ( _includeOnlyOnline )
	{
		imContact = contactProfile.getFirstAvailableIMContact( const_cast<IMChatSession&>(chatSession) );	//This seems wrong.
	}
	else
	{
		imContact = contactProfile.getPreferredIMContact();
	}

	if ( imContact )
	{
		IMContact* imContact2 = chatSession.getIMContactSet().findByContact( *imContact );	//Don't add IMContacts already in chatsession.

		//Jabber IMContacts for Group Chats are not built correctly (missing domain), so try a second find.
		if ( imContact2 == NULL )
		{
			std::string accountId = imContact->getIMAccountId();
			std::string contactId = imContact->getContactId();
			imContact2 = chatSession.getIMContactSet().findBy( accountId, contactId, true );
		}

		bInChatSession = (imContact2 != NULL);

		if ( _sameNetworkOnly )
		{
			bInclude = (imContact->getProtocol() == chatSession.getIMContactSet().begin()->getProtocol() );	//OK use of getProtocol().
		}
	}

	return bInclude;
}

//-----------------------------------------------------------------------------

QtContactPixmap::ContactPixmap ParticipantList::determineStatus( Contact& contact ) 
{
	QtContactPixmap::ContactPixmap status =  QtContactPixmap::ContactUnknown;

	QtEnumIMProtocol::IMProtocol     qtImProtocol = contact.getQtIMProtocol();
	EnumPresenceState::PresenceState eState		  = contact.getPresenceState();

	bool hasAvailableSipNumber = contact.hasPstnCall();
	bool blocked			   = contact.isBlocked();

	status = QtContactPixmap::determinePixmap( qtImProtocol, eState, hasAvailableSipNumber,blocked );

	return status;
}

//-----------------------------------------------------------------------------
	
void ParticipantList::setInvitedToChat( const Participant& participant, bool bInvited )
{
	for ( ParticipantList::iterator it = begin(); it != end(); it++ )
	{
		if ( *it == participant )
		{
			(*it).setInvitedToChat( bInvited );
		}
	}
}

//-----------------------------------------------------------------------------

void ParticipantList::toIMContactSet( IMContactSet& imContactSet )
{
	for ( ParticipantList::iterator it = begin(); it != end(); it++ )
	{
		if ( (*it).isInvitee() )
		{
			imContactSet.insert( (*it).getIMContact() );
		}
	}
}

//-----------------------------------------------------------------------------
