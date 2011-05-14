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

#ifndef OWMAINWINDOW_H
#define OWMAINWINDOW_H

#include <qtutil/QObjectThreadSafe.h>

#include <string>

class QWidget;
class QPushButton;
class QLineEdit;
class Command;
class QtListener;

class MainWindow : public QObjectThreadSafe {
	Q_OBJECT

	friend class QtListener;
public:

	MainWindow();

	QWidget * getWidget() const {
		return _mainWindow;
	}

	void addLogMessage(const std::string & message);

private Q_SLOTS:

	void makeCallButtonClicked();

	void hangUpButtonClicked();

	void acceptCallButtonClicked();

	void sendChatMessageButtonClicked();

	void commandStartButtonClicked();

	void commandTerminateButtonClicked();

private:

	void initThreadSafe() { }

	void addLogMessageThreadSafe(std::string message);

	QWidget * _mainWindow;

	QPushButton * _makeCallButton;

	QPushButton * _hangUpButton;

	QPushButton * _acceptCallButton;

	QPushButton * _rejectCallButton;

	QLineEdit * _phoneNumber;

	QPushButton * _sendChatMessageButton;

	QLineEdit * _chatMessage;

	QPushButton * _commandStartButton;

	QPushButton * _commandTerminateButton;

	Command * _command;

	int _currentCallId;

	int _currentLineId;
};

#endif	//OWMAINWINDOW_H
