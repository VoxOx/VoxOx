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

#ifndef OWQTIMPROFILEWIDGET_H
#define OWQTIMPROFILEWIDGET_H

#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMAccount.h>
#include <util/Trackable.h>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtGui/QIcon>
#include <QtGui/QWidget>

class CWengoPhone;
class CUserProfile;
class QtIMAccountMonitor;
class QtWengoPhone;
class QAction;
class QFrame;
class QMenu;
class QHBoxLayout;
class QWidget;
class QStringList;
class QIcon;
class QResizeEvent;
class VoxOxToolTipLineEdit;

namespace Ui { class IMProfileWidget; }

/**
 * Shows a summary of the user profile + its IM accounts.
 *
 * Inside the profile bar at the top of the main window.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class QtIMProfileWidget : public QWidget, public Trackable {
	Q_OBJECT
public:
	QtIMProfileWidget(QWidget * parent);

	~QtIMProfileWidget();

	void init(CUserProfile*, QtIMAccountMonitor*, QtWengoPhone * qtWengoPhone);
	
	void resetIMAccounts();
	//VOXOX CHANGE Rolando 03-20-09
	void setNickNameLabel(QString displayName);
	void setSipStatusLabel(QString sipStatus);//VOXOX CHANGE Rolando 03-20-09
	void setVoxoxPointsLabel(QString voxoxPoints);//VOXOX CHANGE Rolando 03-20-09
	void updateIMStatusLabel();//VOXOX CHANGE Rolando 03-20-09
	void updateSIPStatusLabel();//VOXOX CHANGE Marin add presence to sip account

	void addAccountsToMenu(QMenu * menu, bool isSIPAccount);
	//VOXOX - CJC - 2009.06.05 
	void updateAvatar();
	
	VoxOxToolTipLineEdit *getVoxOxToolTipLineEdit();// VOXOX CHANGE by ASV 06-05-2009: added to implement the functionality of the Edit Menu
		
public Q_SLOTS:

	void addIMAccount(QString imAccountId);//VOXOX CHANGE Rolando 03-20-09

	void removeIMAccount(QString imAccountId);//VOXOX CHANGE Rolando 03-20-09

	void updateIMAccount(QString imAccountId);//VOXOX CHANGE Rolando 03-20-09

	void reset();

	void voxoxPointsLabelClickedSlot();//VOXOX CHANGE Rolando 04-22-09

	void statusSIPLabelClickedSlot();//VOXOX CHANGE Rolando 03-20-09

	void sipDropDownLabelClickedSlot();//VOXOX CHANGE by Rolando - 2009.06.11

	void openPhoneSettingsWindow();//VOXOX CHANGE by Rolando - 2009.06.11 

	void statusIMLabelClickedSlot();//VOXOX CHANGE Rolando 03-20-09

	void socialNetworkDropDownLabelClickedSlot();//VOXOX CHANGE Rolando 03-20-09

	//void socialNetworkFrameLeaveSlot();//VOXOX CHANGE by Rolando - 2009.07.07 

	//void socialNetworkFrameEnteredSlot();//VOXOX CHANGE by Rolando - 2009.07.07 

	void socialStatusClicked();//VOXOX CHANGE Rolando 03-20-09

	void globalSocialStatusClicked();//VOXOX CHANGE by Rolando - 2009.06.28 

	void clearSocialStatusClicked();//VOXOX CHANGE Rolando 03-20-09

	void clearHistoryStatusClicked();//VOXOX CHANGE by Rolando - 2009.07.02 

	void socialNetworkLineEditChangedSlot(QString);	//VOXOX CHANGE Rolando 03-20-09
	//VOXOX - CJC - 2009.06.05 
	void changeAvatarClicked();

private Q_SLOTS:
	
	void statusMessageTextChanged();	//VOXOX CHANGE CJC
	void languageChanged();


private:
	void cleanLoginName( QString& login );	//VOXOX - JRT - 2009.05.21 

	void updateVoxOxPointsWidthLabel(int difference);

	void verifyVoxOxPointsText(int difference);

	void resizeEvent(QResizeEvent * event);

	void updateText();//VOXOX CHANGE Rolando 03-20-09

	QPixmap getStatus(IMAccount account,EnumPresenceState::PresenceState presenceState);//VOXOX CHANGE Rolando 03-20-09
	
	void createSIPStatusMenu();//VOXOX CHANGE Rolando 03-20-09

	void createIMStatusMenu();//VOXOX CHANGE Rolando 03-20-09

	void createGeneralAccountsMenu();//VOXOX CHANGE by Rolando - 2009.06.16 

	void createSocialStatusMenu();//VOXOX CHANGE Rolando 03-20-09

	void addSIPAccountMenu(QMenu * menu, QString login, QString id, QIcon icon,EnumPresenceState::PresenceState currentPresenceState, const QString& errMsg);//VOXOX CHANGE Rolando 03-20-09

	void addIMAccountMenu(QMenu * menu, QString login, QString id, QIcon icon,EnumPresenceState::PresenceState currentPresenceState, bool imAccountEnable, const QString& errMsg);//VOXOX CHANGE Rolando 03-20-09

	void addIMAccount(IMAccount imAccount);//VOXOX CHANGE Rolando 03-20-09

	/**
	 * @see UserProfile::profileChangedEvent
	 */
	void profileChangedEventHandler();

	

	void createIMAccounts();//VOXOX CHANGE Rolando 03-20-09

	void updateLastStatusMessage();//VOXOX CHANGE by Rolando - 2009.07.01

	void updateDefaultMessage();//VOXOX CHANGE by Rolando - 2009.07.03 

	/**
	 * Internal code factorization.
	 */
	void showImAccountManager();

	void updateWidgets();

	void updateSocialStatusMenu(QString newText);//VOXOX CHANGE Rolando 03-20-09

	Ui::IMProfileWidget * _ui;


	CUserProfile * _cUserProfile;

	QtIMAccountMonitor* _qtImAccountMonitor;

	QtWengoPhone * _qtWengoPhone;	

	//VOXOX - JRT - 2009.04.30 - TODO review this logic and memvars.  It seems redundant to what is available in model.
	QStringList _statusMessageList;//VOXOX CHANGE Rolando 03-20-09

	QMap<QObject*, QString> _socialStatusToAction;//VOXOX CHANGE Rolando 03-20-09

	QMap<QObject*, QString> _imContactKeyToAction;//VOXOX CHANGE Rolando 03-20-09

	typedef std::map<QString,IMAccount> IMAccountToId;//VOXOX CHANGE Rolando 03-20-09
	IMAccountToId _imAccountToId;//VOXOX CHANGE Rolando 03-20-09
	
	typedef std::map<QString,IMAccount> SipAccountToId;//VOXOX CHANGE Rolando 03-20-09
	SipAccountToId _sipAccountToId;//VOXOX CHANGE Rolando 03-20-09

	QString _nickName;//VOXOX CHANGE Rolando 03-20-09
	QString _voxoxPoints;//VOXOX CHANGE Rolando 03-20-09
	QString _sipNumber;//VOXOX CHANGE Rolando 03-20-09

	QString _currentIMAccountKey;
	
};

#endif	//OWQTIMPROFILEWIDGET_H
