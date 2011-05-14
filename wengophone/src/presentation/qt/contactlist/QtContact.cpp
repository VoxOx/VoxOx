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


//VOXOX - JRT - 2009.04.14 
//	WAY too much business logic here.  This is a contact list entry!
//	We should NOT be calling this class to retrieve data or make calls, etc.

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtContact.h"
#include "QtContactListStyle.h"
#include "QtContactPixmap.h"
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfileHandler.h>
#include <model/profile/UserProfile.h>

#include <control/CWengoPhone.h>
#include <control/contactlist/CContactList.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/filetransfer/QtFileTransfer.h>
#include <presentation/qt/webservices/sms/QtSms.h>
#include <presentation/qt/QtNoWengoAlert.h>

#include <imwrapper/QtEnumIMProtocol.h>

#include <util/Logger.h>

#include <QtGui/QtGui>

#include "QtContactWidget.h"


//VOXOX - JRT - 2009.05.04 - TODO: these Ht/Wd defines are same as enum in QtContact.h.  We should use the enum.
const int AVATAR_HEIGHT		 = 28;
const int AVATAR_WIDTH		 = 28;
const int CONTACT_MARGIN	 = 16;
const int AVATAR_TEXT_MARGIN =  4;
const int AVATAR_MARGIN_TOP	 =  4;

QtContact::QtContact(const std::string& contactId, const std::string& groupId, CWengoPhone & cWengoPhone, QObject * parent, bool bGroup )
	:  QObject(parent),
	_cWengoPhone(cWengoPhone) 
{
	initVars();

	_isGroup   = bGroup;
	_contactId = contactId;
	_groupKey  = groupId;					//VOXOX - JRT - 2009.08.09 - Multiple groups.
	_key	   = contactId + groupId;		//VOXOX - JRT - 2009.08.10 - Multiple groups.

	updateDataPtr();
}

void QtContact::initVars()
{
	_isGroup	  = false;
	_contactId	  = "";
	_groupKey	  = "";
	_key		  = "";

	_contact	  = NULL;
	_contactGroup = NULL;

	_item		 = NULL;
	_mouseOn	 = false;
	_mouseButton = Qt::NoButton;

	setIsOpen( false );

	_posXNetworkStatus = 0;
}

void QtContact::paint(QPainter * painter, const QStyleOptionViewItem & option) 
{
	if ( getTreeWidgetItem() ) {// VOXOX -ASV- 2009.10.19
		getTreeWidgetItem()->setSizeHint( 0, QSize(-1, getHeight() ) );	//VOXOX - JRT - 2009.10.15 
		QtContactListStyle * contactListStyle = QtContactListStyle::getInstance(); // VOXOX -ASV- 2009.10.19
		
		// VOXOX -ASV- 2009.10.19: Because of some problem on Mac when trying to detect if the item was 
		// selected or not using QStyleOptionViewItem, I decided to get the background color "manually" 
		// from the contactListStyle according to the specific situation (selected / non selected) and 
		// not to trust the selection state of the item from the object of QStyleOptionViewItem. This is why
		// I removed function 'getBackgroundColor()'
		if ( this->isOpen() )
		{
			// VOXOX -ASV- 2009.10.19: We are calling the instance of this object before this 'if', 
			// so if the intention was to prevent a possible crash this will not prevent it. I am 
			// moving this 'if' up so we prevent a crash in case 'getTreeWidgetItem()' returns NULL
			//if ( getTreeWidgetItem() )
			//{
				painter->fillRect(option.rect, contactListStyle->getContactSelectedBackgroundColorTop());// VOXOX -ASV- 2009.10.19

				QTreeWidget*	 tree   = getTreeWidgetItem()->treeWidget();
				QtContactWidget* widget = new QtContactWidget( getId().toStdString(), getKey().toStdString(), _cWengoPhone, NULL);

				tree->setItemWidget( getTreeWidgetItem(), 0, widget );	
			//}
		}
		else
		{
			painter->fillRect(option.rect, contactListStyle->getContactNonSelectedBackgroundColor() );// VOXOX -ASV- 2009.10.19
			paintForeground(painter, option);
		}
	}
}

//VOXOX CHANGE - Add support for ContactList style and change paint structure.
void QtContact::paintForeground(QPainter * painter, const QStyleOptionViewItem & option) 
{
	QMutexLocker locker(&_mutex);

	if ( ! updateDataPtr() )
	{
		LOG_WARN( QString("Cannot find ContactList entry for %1").arg(getKey()).toStdString() );	//VOXOX - JRT - 2009.08.10 
		return;
	}

	QtContactPixmap * spx = QtContactPixmap::getInstance();
	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();

	if ((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
		painter->setPen(contactListStyle->getContactUserNameSelectedFontColor());
	} else {
		painter->setPen(contactListStyle->getContactUserNameNonSelectedFontColor());
	}

	// Draw the status pixmap
	QtContactPixmap::ContactPixmap status = getStatus();

	QPixmap px = spx->getPixmap(status);

	QRect painterRect = option.rect;

	//painting the status icon
	int x = painterRect.right() - px.width() - 2;
	_posXNetworkStatus = x;

	int centeredPx_y = ((painterRect.bottom() - painterRect.top()) - px.size().height()) / 2;
	
	painter->drawPixmap(x, painterRect.top() + centeredPx_y, px);	

	
	std::string foregroundPixmapData = getContact()->getIcon().getData();

	QPixmap avatar;
	avatar.loadFromData ((uchar*) foregroundPixmapData.c_str(), foregroundPixmapData.size());
	avatar = avatar.scaled ( AVATAR_HEIGHT, AVATAR_WIDTH, Qt::KeepAspectRatio, Qt::SmoothTransformation) ;
	
	painter->drawPixmap(CONTACT_MARGIN, painterRect.top() + AVATAR_MARGIN_TOP, avatar);		

	painterRect.setLeft(CONTACT_MARGIN+AVATAR_WIDTH+AVATAR_TEXT_MARGIN);

	// Draw the text
	painter->setFont(option.font);

	// Center the text vertically
	QRect textRect = painterRect;
	int textY = painterRect.top()+AVATAR_MARGIN_TOP+3;
	textRect.setTop(textY);

	QString text = fixHtmlString(QString::fromUtf8(getContact()->getDisplayName().c_str()));
		
	text = verifyText(textRect,option.font,text);
	painter->drawText(textRect, Qt::AlignLeft, text, 0);

	if ((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
		painter->setPen(contactListStyle->getContactSocialStatusSelectedFontColor());
	} else {
		painter->setPen(contactListStyle->getContactSocialStatusNonSelectedFontColor());
	}
	int fontHeight = QFontMetrics(option.font).height()-3;
	textY +=fontHeight;
	textRect.setTop(textY);

	text = fixHtmlString(QString::fromUtf8(getContact()->getStatusMessage().c_str()));
	text = verifyText(textRect,option.font,text);
	painter->drawText(textRect, Qt::AlignLeft, text, 0);

}

QString QtContact::verifyText(QRect & painterRect, QFont font, QString text){
	int xText = painterRect.left();
	int textWidth = QFontMetrics(font).width(text);//gets new "x" position according width's text	
	
	bool fixedText = false;

	while(xText + textWidth > _posXNetworkStatus){//we need to check that we can paint on the QtContactWidget but not on the vertical line
		text = text.remove(text.length()- 1,text.length());
		textWidth = QFontMetrics(font).width(text);//gets new "x" position according width's text
		fixedText = true;
	}
	
	if(fixedText){
		if(text.length() >= 3){			
			text = text.remove(text.length()- 3,text.length() );
			text += "...";
		}		
	}
	return text;
}

QString QtContact::getKey() const 
{
	return QString::fromStdString(_key);
}

QString QtContact::getId() const 
{
	return QString::fromStdString(_contactId);
}

QString QtContact::getUserName() const 
{
	return QString::fromUtf8(getContact()->getDisplayName().c_str());
}

QString QtContact::getGroupName() const 
{
	std::string groupName = "";
	CUserProfile* cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();

	if ( cUserProfile )
	{
		if ( isGroup() )
		{
			groupName = cUserProfile->getCContactList().getContactGroupName(getId().toStdString());	//VOXOX - JRT - 2009.08.10 - OK use of getId()
		}
		else
		{
//			std::string groupId = getContact()->getFirstUserGroupId();			//VOXOX - JRT - 2009.05.06 JRT-GRPS 
			std::string groupId = getGroupKey();								//VOXOX - JRT - 2009.08.07  

			//VOXOX - JRT - 2009.06.08 - Getting intermittent crashes here which look like CUserProfile is NULL (which is odd).
			//							 They appear to originate from a QtChatWindow::statusChangedSlot().
			groupName = cUserProfile->getCContactList().getContactGroupName(groupId);
		}
	}

	return QString::fromUtf8(groupName.c_str());
}

QString	QtContact::getContactGroupContactsOnline() const
{
	std::string onlineInfo = "";

	if ( isGroup() )
	{
		//VOXOX - JRT - 2009.08.10 - OK use of getId().
		onlineInfo = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().getContactGroupContactsOnline( getId().toStdString() );	
	}

	return QString::fromUtf8(onlineInfo.c_str());
}

bool QtContact::isTestCall() const 
{
	std::string sipAddress = getContact()->getFirstFreePhoneNumber();
	if (sipAddress.empty()) 
	{
		return false;
	}

	// Check if the sipAddress is one of the test calls. We check all test
	// calls because we don't have the information about whether we are
	// connected with a SIP or a Wengo account here.
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if (sipAddress == config.getWengoAudioTestCall()) {
		return true;
	}
	if (sipAddress == config.getWengoVideoTestCall()) {
		return true;
	}
	if (sipAddress == config.getSipAudioTestCall()) {
		return true;
	}
	if (sipAddress == config.getSipVideoTestCall()) {
		return true;
	}
	return false;
}
///VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
bool QtContact::isBlocked(){
	return _contact->getPreferredIMContact()->isBlocked();
}

QtContactPixmap::ContactPixmap QtContact::getStatus() {	//JRT - 2009.04.05 - Removed const
	// Rules are defined here:
	// http://dev.openwengo.org/trac/openwengo/trac.cgi/wiki/ContactPresenceGrid
	// version 9
	QtContactPixmap::ContactPixmap status =  QtContactPixmap::ContactUnknown;

	//VOXOX - JRT - 2009.04.12 - Some crashes here with recent changes.
	IMContact* contact = getContact()->getPreferredIMContact();
	EnumIMProtocol::IMProtocol imProtocol = EnumIMProtocol::IMProtocolUnknown;

	if ( contact )
	{
		imProtocol = contact->getProtocol();	//VOXOX - JRT - 2009.07.02 - OK use of getProtocol()?
	}
	//End Voxox

	bool hasAvailableSipNumber = getContact()->hasPstnCall();
	bool blocked			   = getContact()->isBlocked();

	//VOXOX - JRT - 2009.04.14 - Let's move all this logic to the QtContactPixMap class where it belongs.
	EnumPresenceState::PresenceState eState = getContact()->getPresenceState();
	QtEnumIMProtocol::IMProtocol qtImProtocol = QtEnumIMProtocolMap::getInstance().toQtIMProtocol( imProtocol, getContact()->getIsIMAccountVoxox(), 
																											   getContact()->getIsIMAccountGtalk() ); 

	status = QtContactPixmap::determinePixmap( qtImProtocol, eState, hasAvailableSipNumber,blocked );

	return status;
}

//VOXOX CHANGE by Rolando - 2009.10.23 - TODO: factorize - Returns ContactPixmap if it was Hover status
QtContactPixmap::ContactPixmap QtContact::getHoverStatus() {
	// Rules are defined here:
	// http://dev.openwengo.org/trac/openwengo/trac.cgi/wiki/ContactPresenceGrid
	// version 9
	QtContactPixmap::ContactPixmap status =  QtContactPixmap::ContactUnknown;

	//VOXOX - JRT - 2009.04.12 - Some crashes here with recent changes.
	IMContact* contact = getContact()->getPreferredIMContact();
	EnumIMProtocol::IMProtocol imProtocol = EnumIMProtocol::IMProtocolUnknown;

	if ( contact )
	{
		imProtocol = contact->getProtocol();	//VOXOX - JRT - 2009.07.02 - OK use of getProtocol()?
	}
	//End Voxox

	bool hasAvailableSipNumber = getContact()->hasPstnCall();
	bool blocked			   = getContact()->isBlocked();

	//VOXOX - JRT - 2009.04.14 - Let's move all this logic to the QtContactPixMap class where it belongs.
	EnumPresenceState::PresenceState eState = getContact()->getPresenceState();
	QtEnumIMProtocol::IMProtocol qtImProtocol = QtEnumIMProtocolMap::getInstance().toQtIMProtocol( imProtocol, getContact()->getIsIMAccountVoxox(), 
																											   getContact()->getIsIMAccountGtalk() ); 

	status = QtContactPixmap::determineHoverPixmap( qtImProtocol, eState, hasAvailableSipNumber,blocked );

	return status;
}

void QtContact::setButton(const Qt::MouseButton button) {
	_mouseButton = button;
}

Qt::MouseButton QtContact::getButton() const {
	return _mouseButton;
}

int QtContact::getHeight() const 
{
	if ( isGroup() )
	{
		return GroupSize;
	}
	else
	{
		if ( isOpen() ) 
		{
			return UserOpenSize;
		}
		else
		{
			return UserSize;
		}
	}
}

void QtContact::resetMouseStatus() 
{
	setButton(Qt::NoButton);
}

//VOXOX - JRT - 2009.04.05 - Because we now have (or soon will have) user-defined phone type,
// these getXxxPhone() methods cannot be const.  This is because the phone list will automatically
// create an empty entry for any non-existing type that is requested, as we do here.
//Going forward, this control may have to change when we implement user-defined types.
QString QtContact::getMobilePhone()
{
	return QString::fromStdString(getContact()->getMobilePhone());
}

QString QtContact::getHomePhone()
{
	return QString::fromStdString(getContact()->getHomePhone());
}

QString QtContact::getWebsite()
{
	return QString::fromStdString(getContact()->getWebsite());
}

QString QtContact::getWorkPhone()
{
	return QString::fromStdString(getContact()->getWorkPhone());
}

QString QtContact::getWengoPhoneNumber()
{
	return QString::fromStdString(getContact()->getFirstAvailableWengoId());
}

QString QtContact::getPreferredNumber()
{
	return QString::fromStdString(getContact()->getPreferredNumber());
}

QString QtContact::getDisplayName() const 
{
	if ( isGroup() )
	{
		return getGroupName();
	}
	else
	{
		return QString::fromUtf8( getContact()->getDisplayName().c_str());
	}
}

QString QtContact::getEscapedDisplayName()
{
	return fixHtmlString( getDisplayName() );
}

QString QtContact::getStatusMessage() const 
{
	return QString::fromUtf8( getContact()->getStatusMessage().c_str());
}

QString QtContact::getEscapedStatusMessage()
{
	return fixHtmlString( getStatusMessage() ); 
}

//VOXOX CHANGE by Rolando - 2009.10.23 
QString QtContact::getNormalStatusPixmapPath() 
{
	QtContactPixmap * spx = QtContactPixmap::getInstance();

	// Draw the status pixmap
	QtContactPixmap::ContactPixmap status = getStatus();
	
	QString pxPath = spx->getPixmapPath(status);
	
	return pxPath;
}

//VOXOX CHANGE by Rolando - 2009.10.23 
QString QtContact::getHoverStatusPixmapPath() 
{
	QtContactPixmap * spx = QtContactPixmap::getInstance();

	// Draw the status pixmap
	QtContactPixmap::ContactPixmap status = getHoverStatus();

	QString pxPath = spx->getPixmapPath(status);
	
	return pxPath;
}

//QPixmap QtContact::getStatusPixmap() 
//{
//	QtContactPixmap * spx = QtContactPixmap::getInstance();
//
//	// Draw the status pixmap
//	QtContactPixmap::ContactPixmap status = getStatus();
//	
//		//Set Status Pixmap
//	QPixmap px = spx->getPixmap(status);
//	
//	return px;
//}

QPixmap QtContact::getAvatarPixmap() const 
{
	std::string foregroundPixmapData = getContact()->getIcon().getData();	
	QPixmap avatar;
	avatar.loadFromData ((uchar*) foregroundPixmapData.c_str(), foregroundPixmapData.size());
	return avatar;
}
EnumPresenceState::PresenceState QtContact::getPresenceState() const 
{
	return getContact()->getPresenceState();
}

int QtContact::getSortOrder() const
{
	int sortOrder = 0;

	ContactGroup* pGroup = getContactGroup();

	if ( pGroup )
	{
		sortOrder = pGroup->getSortOrder();
	}

	return sortOrder;
}

EnumGroupType::GroupType QtContact::getGroupType() const
{
	EnumGroupType::GroupType grpType = EnumGroupType::GroupType_User;

	ContactGroup* pGroup = getContactGroup();

	if ( pGroup )
	{
		grpType = pGroup->getType();
	}

	return grpType;
}

ContactGroup* QtContact::getContactGroup() const
{
	ContactGroup* pRet = NULL;

	if ( isGroup() )
	{
		//VOXOX - JRT - 2009.08.10 - OK use of getId().
		pRet = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().getContactGroup( getId().toStdString() );

//		assert(pRet);	//VOXOX - JRT - 2009.06.05 - We get here when grouping == None.  TODO; make NOGROUP an actual EnumGroupType.
	}

	return pRet;
}
ContactGroupInfo* QtContact::getContactGroupInfo() const
{
	ContactGroupInfo* cgi = NULL;
	
	if ( !isGroup() )
	{
		if ( getContact() )
		{
			cgi = getContact()->getContactGroupInfoSet().FindByKey( getGroupKey() );
		}
	}

	return cgi;
}

bool QtContact::updateDataPtr()
{
	bool bSuccess = false;

	_contact        = NULL;
	_contactGroup   = NULL;

	if ( _isGroup )
	{
		_contactGroup = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().getContactGroup(_contactId);

		bSuccess = (_contactGroup != NULL);
	}
	else
	{
		_contact = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().getContactByKey(_contactId);

		bSuccess = (_contact != NULL);
	}

	return bSuccess;
}

//VOXOX CHANGE CJC, remove unescaped chars from sting
//TODO: QT does NOT provide an unescape() but they do provide an escape().  Move this to QtUtil project.
//TODO: I think String class does this.
QString QtContact::fixHtmlString(QString text)
{
    QString result = text;

	result = result.replace("&quot;", "\"");
	result = result.replace("&amp;",  "&" );
	result = result.replace("&gt;",   ">" );
	result = result.replace("&lt;",   "<" );
	result = result.replace("&apos;", "\'");
        
    return result;
}

//-----------------------------------------------------------------------------

bool QtContact::shouldExpand()
{
	bool bShould = true;

	QString groupName = getGroupName();
		
	//Does not expand if contact is part of the WDeal contact group
	if ( groupName == "WDeal")
	{
		bShould = false;
	}

	return bShould;
}

//-----------------------------------------------------------------------------

bool QtContact::canDrag()
{
	bool result = false;
	
	if ( isGroup() )
	{
		result = false;
	}
	else
	{
		if ( getContact() )
		{
			result = getContact()->getContactGroupInfoSet().CanDrag( getGroupKey() );
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
