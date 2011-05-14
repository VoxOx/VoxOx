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

#ifndef OWQTCHATROOMINVITEDLG_H
#define OWQTCHATROOMINVITEDLG_H

#include <QtGui/QDialog>

#include <presentation/qt/contactlist/QtContactPixmap.h>

#include <control/contactlist/CContactList.h>
#include <model/contactlist/Contact.h>

#include <imwrapper/IMContact.h>
#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMChat.h>

class Contacts;
class QLineEdit;

class Participant
{
public:
	Participant();
	virtual ~Participant();

	bool isCandidate()    const;
	bool isInvitee()      const;
	bool canBeUninvited() const;

	IMContact	getIMContact()	  const									{ return _imContact;		}
	std::string getDisplayName()  const									{ return _displayName;		}
	bool		isAlreadyInChat() const									{ return _isAlreadyInChat;	}
	bool		isInvitedToChat() const									{ return _invitedToChat;	}
	QtContactPixmap::ContactPixmap getPixmap() const					{ return _pixmap;			}

	void setIMContact      ( const IMContact&   val )					{ _imContact		= val;	}
	void setDisplayName	   ( const std::string& val )					{ _displayName		= val;	}
	void setIsAlreadyInChat( bool val )									{ _isAlreadyInChat	= val;	}
	void setInvitedToChat  ( bool val )									{ _invitedToChat	= val;	}
	void setPixmap		   ( QtContactPixmap::ContactPixmap val )		{ _pixmap			= val;	}

	Participant& operator= ( const Participant& src );
	bool         operator==( const Participant& src );

protected:
	void initVars();

private:
	IMContact	_imContact;
	std::string	_displayName;
	bool		_isAlreadyInChat;
	bool		_invitedToChat;

	QtContactPixmap::ContactPixmap	_pixmap;
};

//=============================================================================

class ParticipantList : public VoxListTemplate<Participant>
{
public:
	ParticipantList();

	void populate  ( const Contacts& rContacts, IMChatSession& imChatSession );
	void setInvitedToChat( const Participant& participant, bool bInvited );

	void toIMContactSet( IMContactSet& imContactSet );

	void setIncludeOnlyOnline( bool val )			{ _includeOnlyOnline = val;	}
	void setSameNetworkOnly  ( bool val )			{ _sameNetworkOnly   = val;	}

protected:
	bool includeInCandidateList( const ContactProfile& contactProfile, const IMChatSession& chatSession, bool& bInChatSession );
	QtContactPixmap::ContactPixmap determineStatus( Contact& contact ) ;

private:
	bool _includeOnlyOnline;
	bool _sameNetworkOnly;
};

//============================================================================

namespace Ui { class ChatRoomInviteWidget; }

class QListWidget;
class QTreeWidget;
class QTreeWidgetItem;

class Contact;
/**
 *
 * @ingroup presentation
 * @author Mr K.
 * modified by J. R. Theinert
 */
class QtChatRoomInviteDlg : public QDialog
{
	Q_OBJECT

public:
	QtChatRoomInviteDlg(IMChatSession & chatSession, ContactList&  contactList,  QWidget* parent = 0, Qt::WFlags f = 0);
	QtChatRoomInviteDlg(IMChatSession & chatSession, CContactList& cContactList, QWidget* parent = 0, Qt::WFlags f = 0);

	virtual ~QtChatRoomInviteDlg();

	ParticipantList&	getParticipants()						{ return _candidates;	}

protected Q_SLOTS:
	void startConference();
	void addToConference();
	void removeFromConference();
	void cancel();

protected:
	void setupGui();

	void initData();
	bool validData();
	void populateCandidates();
	void updateListCtrls();
	void updateCandidateListCtrl();
	void updateInviteeListCtrl();

	QTreeWidgetItem* createGroup( const QString& groupName );

	Ui::ChatRoomInviteWidget * _ui;

	QWidget*		_chatRoomInviteWidget;
	QTreeWidget*	_contactListTreeWidget;
	QListWidget*	_inviteListWidget;
	QLineEdit*		_lineEditChatRoomName;

	QPushButton*	_addPushButton;
	QPushButton*	_removePushButton;

	QPushButton*	_startPushButton;
	QPushButton*	_cancelPushButton;

	CContactList&	_cContactList;
	IMChatSession&	_chatSession;
	bool			_includeOnlyOnline;
	bool			_sameNetworkOnly;
	std::string		_chatRoomName;

	ParticipantList	_candidates;
};

//=============================================================================

#endif //OWQTCHATROOMINVITEDLG_H
