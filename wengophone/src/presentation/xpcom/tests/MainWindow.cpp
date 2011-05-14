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

#include "MainWindow.h"

#include <Listener.h>
#include <Command.h>

#include <Object.h>
#include <WidgetFactory.h>

#include <thread/Thread.h>
#include <util/Logger.h>

#include <QtGui/QtGui>

#include <string>
using namespace std;

class QtListener : public Listener {
public:

	QtListener(MainWindow & mainWindow);

	void wengoLoginStateChangedEvent(LoginState state, const std::string & login, const std::string & password);

	void phoneLineStateChangedEvent(PhoneLineState state, int lineId, void * param);

	void phoneCallStateChangedEvent(PhoneCallState state, int lineId, int callId,
				const std::string & sipAddress, const std::string & userName, const std::string & displayName,
				void * param);
	//VOXOX CHANGE SUPPORT STATUS MESSAGE
	void presenceStateChangedEvent(PresenceState state, const std::string & note, const std::string & statusMessage, 
				const std::string & sipAddress, const std::string & userName, const std::string & displayName,
				void * param);

	void chatMessageReceivedEvent(const std::string & message,
				const std::string & sipAddress, const std::string & userName, const std::string & displayName,
				void * param);

	void loggerMessageAddedEvent(const std::string & message);

private:

	MainWindow & _mainWindow;
};


QtListener::QtListener(MainWindow & mainWindow)
	: _mainWindow(mainWindow) {
}

void QtListener::wengoLoginStateChangedEvent(LoginState state, const std::string & login, const std::string & password) {
	switch (state) {
	case LoginNetworkError:
		_mainWindow.addLogMessage("LoginNetworkError");
		break;

	case LoginPasswordError:
		_mainWindow.addLogMessage("LoginPasswordError");
		break;

	case LoginOk:
		_mainWindow.addLogMessage("LoginOk");
		break;

	case LoginNoAccount:
		_mainWindow.addLogMessage("LoginNoAccount");
		break;

	default:
		LOG_FATAL("unknown login state");
	}
}

void QtListener::phoneLineStateChangedEvent(PhoneLineState state, int lineId, void * param) {
	_mainWindow._currentLineId = lineId;

	switch (state) {
	case LineProxyError:
		_mainWindow.addLogMessage("LineProxyError");
		break;

	case LineServerError:
		_mainWindow.addLogMessage("LineServerError");
		break;

	case LineTimeout:
		_mainWindow.addLogMessage("LineTimeout");
		break;

	case LineOk:
		_mainWindow.addLogMessage("LineOk");

		//Presence
		_mainWindow._command->subscribeToPresenceOf("annefleur");
		_mainWindow._command->subscribeToPresenceOf("dbitton_office");
		_mainWindow._command->subscribeToPresenceOf("guirec_office");
		_mainWindow._command->subscribeToPresenceOf("dbitton_office");
		_mainWindow._command->subscribeToPresenceOf("antesoft");
		_mainWindow._command->subscribeToPresenceOf("tanguy-krotoff");
		break;

	case LineClosed:
		_mainWindow.addLogMessage("LineClosed");
		delete _mainWindow._command;
		_mainWindow._command = NULL;
		break;

	default:
		LOG_FATAL("unknown phone line state");
	}
}

void QtListener::phoneCallStateChangedEvent(PhoneCallState state, int lineId, int callId,
				const std::string & sipAddress, const std::string & userName, const std::string & displayName,
				void * param) {

	_mainWindow._currentLineId = lineId;
	_mainWindow._currentCallId = callId;

	switch (state) {
	case CallIncoming:
		_mainWindow.addLogMessage("CallIncoming: " + userName + " " + displayName);
		break;

	case CallOk:
		_mainWindow.addLogMessage("CallOk");
		break;

	case CallError:
		_mainWindow.addLogMessage("CallError");
		break;

	case CallResumeOk:
		_mainWindow.addLogMessage("CallResumeOk");
		break;

	case CallDialing:
		_mainWindow.addLogMessage("CallDialing");
		break;

	case CallRinging:
		_mainWindow.addLogMessage("CallRinging");
		break;

	case CallClosed:
		_mainWindow.addLogMessage("CallClosed");
		break;

	case CallHoldOk:
		_mainWindow.addLogMessage("CallHoldOk");
		break;

	default:
		LOG_FATAL("unknown phone call state");
	}
}
//VOXOX CHANGE CJC SUPPORT STATUS MESSAGE
void QtListener::presenceStateChangedEvent(PresenceState state, const std::string & note, const std::string & statusMessage,
			const std::string & sipAddress, const std::string & userName, const std::string & displayName,
			void * param) {

	switch(state) {
	case PresenceOnline:
		_mainWindow.addLogMessage("PresenceOnline: " + userName);
		break;

	case PresenceOffline:
		_mainWindow.addLogMessage("PresenceOffline: " + userName);
		break;

	case PresenceAway:
		_mainWindow.addLogMessage("PresenceAway: " + userName);
		break;

	case PresenceDoNotDisturb:
		_mainWindow.addLogMessage("PresenceDoNotDisturb: " + userName);
		break;

	case PresenceUserDefined:
		_mainWindow.addLogMessage("PresenceUserDefined: " + userName);
		break;

	case PresenceUnknown:
		_mainWindow.addLogMessage("PresenceUnknown: " + userName);
		break;

	default:
		LOG_FATAL("unknown presence state");
	}
}

void QtListener::chatMessageReceivedEvent(const std::string & message,
			const std::string & sipAddress, const std::string & userName, const std::string & displayName,
			void * param) {

	_mainWindow.addLogMessage(message + " " + sipAddress);
}

void QtListener::loggerMessageAddedEvent(const std::string & message) {
}


MainWindow::MainWindow() {
	_mainWindow = WidgetFactory::create("MainWindow.ui", NULL);

	_makeCallButton = Object::findChild<QPushButton *>(_mainWindow, "makeCallButton");
	connect(_makeCallButton, SIGNAL(clicked()), SLOT(makeCallButtonClicked()));

	_hangUpButton = Object::findChild<QPushButton *>(_mainWindow, "hangUpButton");
	connect(_hangUpButton, SIGNAL(clicked()), SLOT(hangUpButtonClicked()));

	_acceptCallButton = Object::findChild<QPushButton *>(_mainWindow, "acceptCallButton");
	connect(_acceptCallButton, SIGNAL(clicked()), SLOT(acceptCallButtonClicked()));

	_phoneNumber = Object::findChild<QLineEdit *>(_mainWindow, "phoneNumber");
	connect(_phoneNumber, SIGNAL(returnPressed()), SLOT(makeCallButtonClicked()));

	_sendChatMessageButton = Object::findChild<QPushButton *>(_mainWindow, "sendChatMessageButton");
	connect(_sendChatMessageButton, SIGNAL(clicked()), SLOT(sendChatMessageButtonClicked()));

	_chatMessage = Object::findChild<QLineEdit *>(_mainWindow, "chatMessage");
	connect(_chatMessage, SIGNAL(returnPressed()), SLOT(sendChatMessageButtonClicked()));

	_commandStartButton = Object::findChild<QPushButton *>(_mainWindow, "commandStartButton");
	connect(_commandStartButton, SIGNAL(clicked()), SLOT(commandStartButtonClicked()));

	_commandTerminateButton = Object::findChild<QPushButton *>(_mainWindow, "commandTerminateButton");
	connect(_commandTerminateButton, SIGNAL(clicked()), SLOT(commandTerminateButtonClicked()));

	_command = NULL;
	commandStartButtonClicked();
}

void MainWindow::addLogMessage(const std::string & message) {
	typedef PostEvent1<void (const std::string &), std::string> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&MainWindow::addLogMessageThreadSafe, this, _1), message);
	postEvent(event);
}

void MainWindow::addLogMessageThreadSafe(std::string message) {
	static QListWidget * listWidget = Object::findChild<QListWidget *>(_mainWindow, "listWidget");

	listWidget->addItem(message.c_str());
	listWidget->scrollToItem(listWidget->item(listWidget->count() - 1));
}

void MainWindow::makeCallButtonClicked() {
	string number = _phoneNumber->text().toStdString();
	if (!number.empty()) {
		_command->makeCall(number, _currentLineId);
	}
}

void MainWindow::hangUpButtonClicked() {
	_command->hangUp(_currentCallId);
}

void MainWindow::acceptCallButtonClicked() {
	_command->pickUp(_currentCallId);
}

void MainWindow::sendChatMessageButtonClicked() {
	string message = _chatMessage->text().toStdString();
	string number = _phoneNumber->text().toStdString();
	if (!number.empty()) {
		_command->sendChatMessage(number, message);
	}
}

void MainWindow::commandStartButtonClicked() {
	static QtListener * listener = new QtListener(*this);

	//QString configDir = QDir::homePath() + QDir::separator() + QString("Jérôme") + QDir::separator();
	QString configDir;

	_command = new Command(configDir.toStdString());
	_command->start();
	_command->addListener(listener);
	addLogMessage("Command::start()");
}

void MainWindow::commandTerminateButtonClicked() {
	if (_command) {
		_command->terminate();
	}

	addLogMessage("Command::terminate()");
}
