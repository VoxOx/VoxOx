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
#ifndef QTIMACCOUNTMONITOR_H
#define QTIMACCOUNTMONITOR_H

#include <imwrapper/EnumPresenceState.h>

#include <util/Trackable.h>
#include <util/NonCopyable.h>

#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QStringList>

class CUserProfile;
class IMAccount;


/**
 * This class monitors the changes in the IM accounts of the user.
 * It knows about the connection state and details as well as the IM presence.
 * It stores transient info not stored in the model, like the fact that the IM
 * account is connecting (neither disconnected nor connected yet) or the info
 * message of an account which is connecting.
 *
 * It will emits Qt signals when IM acccounts are added/removed/updated (@see
 * imAccountAdded, @see imAccountRemoved, @see imAccountUpdated).
 *
 * These signals are emitted from the model thread.
 *
 * @author Aurelien Gateau
 */
class QtIMAccountMonitor : public QObject, public Trackable, private NonCopyable {
Q_OBJECT
public:
	enum ConnectionState {
		StateDisconnected, /** IM account is disconnected */
		StateConnecting, /** IM account is trying to connect. Use getIMAccountInfo().message() to know more. */
		StateConnected, /** IM account is connected */
		StateFailure /** IM account failed to connect. Use getIMAccountInfo().message() to know more. */
	};

	/**
	 * This class gathers the IMAccount information.
	 */
	class IMAccountInfo {
	public:
		ConnectionState connectionState() const {
			return _state;
		}

		QString message() const {
			return _message;
		}

	private:
		ConnectionState _state;
		QString _message;

		friend class QtIMAccountMonitor;
	};

	typedef std::auto_ptr<IMAccountInfo> IMAccountInfoAutoPtr;

	QtIMAccountMonitor(QObject* parent, CUserProfile*);

	/**
	 * Returns an auto_ptr to the IMAccountInfo for account imAccountid. If
	 * there is no such account, the auto_ptr will point to 0.
	 */
	IMAccountInfoAutoPtr getIMAccountInfo(const QString& imAccountId) const;

Q_SIGNALS:
	/**
	 * Emitted when an IM account has been added.
	 * This signal is emitted in the model thread.
	 */
	void imAccountAdded(QString imAccountId);

	/**
	 * Emitted when an IM account has been added.
	 * This signal is emitted in the model thread.
	 */
	void imAccountRemoved(QString imAccountId);

	/**
	 * Emitted when an IM account has been added.
	 * This signal is emitted in the model thread.
	 */
	void imAccountUpdated(QString imAccountId);

private:
	typedef QMap<QString, IMAccountInfo> IMAccountInfoMap;

	void imAccountAddedEventHandler(std::string imAccountId);
	void imAccountRemovedEventHandler(std::string imAccountId);

	void connectedEventHandler(std::string imAccountId);
	void disconnectedEventHandler(std::string imAccountId, bool connectionError, std::string reason);
	void connectionProgressEventHandler(std::string imAccountId, int currentStep, int totalSteps, std::string infoMessage);

	void myPresenceStatusEventHandler(std::string imAccountId, EnumPresenceState::MyPresenceStatus status);

	QString connectedMessageForIMAccountId(std::string imAccountId);
	QString connectedMessageForIMAccount(const IMAccount& imAccount);

	CUserProfile* _cUserProfile;

	IMAccountInfoMap _imAccountInfoMap;
	mutable QMutex _imAccountInfoMapMutex;
};

#endif /* QTIMACCOUNTMONITOR_H */
