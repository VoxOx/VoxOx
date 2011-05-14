/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* CLASS QtPopUpChatDialog
* @author Rolando 
* @date 2009.06.10
*/

#ifndef OWQTPOPUPCHATWINDOW_H
#define OWQTPOPUPCHATWINDOW_H


#include <QtGui/QDialog>

#include <util/Trackable.h>

#include <thread/ThreadEvent.h>
#include <presentation/PFactory.h>

#include "QtEnumUMMode.h"
#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMContact.h>

class QtUMBase;
class CWengoPhone;//VOXOX CHANGE by Rolando - 2009.06.16 

class IMChatSession;//VOXOX CHANGE by Rolando - 2009.06.11 
class IMContact;//VOXOX CHANGE by Rolando - 2009.06.11 

namespace Ui { class PopUpChatDialog; }

/**
 *
 * @ingroup presentation
 * @author Mathieu Stute.
 */
class QtPopUpChatDialog : public QDialog, public Trackable {
	Q_OBJECT
public:

	QtPopUpChatDialog(QWidget * parent, QtUMBase * QtUMBase, CWengoPhone & cWengoPhone);

	~QtPopUpChatDialog();

	void show();

	QtUMBase * getQtUMBase();

	QtEnumUMMode::Mode getWidgetMode();

	//Utilitary methods
	void setBtnChatEnabled(bool enable);
	void setBtnCallButtonEnabled(bool enable);
	void setSMSButtonEnabled(bool enable);
	void setSendFileButtonEnabled(bool enable);
	void setProfileButtonEnabled(bool enable);
	void setEmailButtonEnabled(bool enable);
	void setFaxButtonEnabled(bool enable);
	void setPhoneSettingsButtonEnabled(bool enable);
	void setButtonFrameEnabled(bool enable);

	void setBtnChatChecked(bool checked);
	void setBtnCallButtonChecked(bool checked);
	void setSMSButtonChecked(bool checked);
	void setSendFileButtonChecked(bool checked);
	void setProfileButtonChecked(bool checked);
	void setEmailButtonChecked(bool checked);
	void setFaxButtonChecked(bool checked);
	void setPhoneSettingsButtonChecked(bool checked);

	void flashWindow();

Q_SIGNALS:

	void closeWindowSignal(QString contactId);
	void implodeButtonClicked(QString contactId);
	void typingStateChangedSignal(const IMChatSession * sender, const IMContact * imContact, const IMChat::TypingState * state);//VOXOX CHANGE by Rolando - 2009.06.11 

public Q_SLOTS:

	void setChatWindowTitle(const QString & text);

	void typingStateChangedThreadSafe(const IMChatSession * sender, const IMContact * imContact, const IMChat::TypingState * state);//VOXOX CHANGE by Rolando - 2009.06.11 

	void implodeButtonClickedSlot();

	void closeWindow();

	// Added from QtChatWindow
	void startChat();
	void startCall();
	void startSMS();
	void startSendFile();
	void startProfile();
	void startEmailChat();
	void startFax();
	void phoneSettings();

private:

	void setupToolBarActions();

	void typingStateChangedEventHandler(IMChatSession & sender, const IMContact & imContact, IMChat::TypingState state);//VOXOX CHANGE by Rolando - 2009.06.11 
	
	QtUMBase * _QtUMBase;
	
	Ui::PopUpChatDialog * _ui;

	CWengoPhone & _cWengoPhone;

};


#endif	//OWQTPOPUPCHATWINDOW_H

