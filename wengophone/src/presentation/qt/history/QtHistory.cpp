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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtHistory.h"

#include "QtHistoryWidget.h"

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/chat/QtChatLogViewer.h>
#include <presentation/qt/webservices/sms/QtSms.h>

#include <control/history/CHistory.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/CWengoPhone.h>

#include <model/contactlist/ContactList.h>
#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/history/History.h>
#include <model/phonecall/SipAddress.h>
#include <model/profile/UserProfileHandler.h>
#include <model/profile/UserProfile.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>

QString QtHistory::formatName(const std::string& name, bool isWengo) const {
	// do not display realm whith wengo account
	SipAddress sipAddress(name);
	QString formattedName = QString::fromStdString(sipAddress.getSipAddress());
	
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString wengoSuffix = "@" + QString::fromStdString( config.getWengoRealm() );
	if (isWengo) {
		formattedName.remove(wengoSuffix);
	}
	formattedName.remove("sip:");

	Contact * contact = _cHistory.getCWengoPhone().getCUserProfileHandler().getUserProfileHandler().getCurrentUserProfile()->getContactList().getContactByNumber(formattedName.toStdString()); //VOXOX CHANGE by Rolando - 2009.10.06 
	QString displayName;//VOXOX CHANGE by Rolando - 2009.10.06 

	if(contact)//VOXOX CHANGE by Rolando - 2009.10.06 
	{
		QString newName = QString::fromStdString(contact->getDisplayName()) + " (" + formattedName + ")";//VOXOX CHANGE by Rolando - 2009.10.06 
		formattedName = newName;//VOXOX CHANGE by Rolando - 2009.10.06 
	}	

	return formattedName;
}

static QDateTime qDateTimeForMemento(HistoryMemento* memento) {
	Date dt = memento->getDate();
	QDate date(dt.getYear(), dt.getMonth(), dt.getDay());
	Time tm = memento->getTime();
	QTime time(tm.getHour(), tm.getMinute(), tm.getSecond());
	return QDateTime(date, time);
}

QString QtHistory::formatDate(const QDateTime& dateTime) {
	return dateTime.toString(Qt::SystemLocaleDate);
}

QString QtHistory::formatDuration(const QTime& time) {
	return time.toString(Qt::TextDate);
}

QString QtHistory::textForMementoState(const HistoryMemento::State& state) const {
	switch (state) {
	case HistoryMemento::IncomingCall:
		return tr("Incoming call");
	case HistoryMemento::OutgoingCall:
		return tr("Outgoing call");
	case HistoryMemento::MissedCall:
		return tr("Missed call");
	case HistoryMemento::RejectedCall:
		return tr("Rejected call");
	case HistoryMemento::OutgoingSmsOk:
		return tr("Outgoing SMS");
	case HistoryMemento::ChatSession:
		return tr("Chat session");
	case HistoryMemento::OutgoingSmsNok:
	case HistoryMemento::None:
	case HistoryMemento::Any:
	case HistoryMemento::FileTransferUploaded:
	case HistoryMemento::FileTransferInProgress:
	case HistoryMemento::FileTransferDownloaded:
	case HistoryMemento::FileTransferCancelled:
	case HistoryMemento::FileTransferPaused:
		break;
	}
	return QString();
}

static QIcon iconForMementoState(HistoryMemento::State state) {
	static QMap<HistoryMemento::State, QIcon> iconMap;
	if (iconMap.isEmpty()) {
		iconMap[HistoryMemento::IncomingCall]  = QIcon(":/pics/history/call_incoming.png");
		iconMap[HistoryMemento::OutgoingCall]  = QIcon(":/pics/history/call_outgoing.png");
		iconMap[HistoryMemento::MissedCall]    = QIcon(":/pics/history/call_missed.png");
		iconMap[HistoryMemento::RejectedCall]  = QIcon(":/pics/history/call_missed.png");
		iconMap[HistoryMemento::OutgoingSmsOk] = QIcon(":/pics/history/sms_sent.png");
		iconMap[HistoryMemento::ChatSession]   = QIcon(":/pics/history/chat.png");
	}
	QMap<HistoryMemento::State, QIcon>::ConstIterator it = iconMap.find(state);
	if (it != iconMap.end()) {
		return it.value();
	} else {
		return QIcon();
	}
}

static QTime qTimeForDuration(int duration) {
	if (duration == -1) {
		duration = 0;
	}
	QTime time;
	return time.addSecs(duration);
}

/**
 * This helper class is used to sort history by date
 */
class HistoryMementoSorter {
public:
	HistoryMementoSorter(HistoryMementoCollection* collection)
	: _collection(collection) {}

	bool operator()(int id1, int id2) {
		HistoryMemento* m1 = _collection->getMemento(id1);
		Q_ASSERT(m1);
		HistoryMemento* m2 = _collection->getMemento(id2);
		Q_ASSERT(m2);
		QDateTime dt1 = qDateTimeForMemento(m1);
		QDateTime dt2 = qDateTimeForMemento(m2);
		return dt1 > dt2;
	}

private:
	HistoryMementoCollection* _collection;
};

QtHistory::QtHistory(CHistory & cHistory)
	: QAbstractTableModel(NULL),
	_cHistory(cHistory) {

	_chatLogViewer = NULL;
	_mutex = new QMutex(QMutex::Recursive);

	_stateFilter = HistoryMemento::Any;

	QtWengoPhone * qtWengoPhone = (QtWengoPhone *) _cHistory.getCWengoPhone().getPresentation();
	_historyWidget = new QtHistoryWidget(qtWengoPhone->getWidget(), this);
	SAFE_CONNECT(_historyWidget,
		SIGNAL(replayItemRequested(int)),
		SLOT(replayItem(int))
	);
	SAFE_CONNECT(_historyWidget,
		SIGNAL(showChatLogRequested(int)),
		SLOT(showChatLog(int))
	);
	SAFE_CONNECT(_historyWidget,
		SIGNAL(removeItemRequested(int)),
		SLOT(removeItem(int))
	);
	SAFE_CONNECT(_historyWidget,
		SIGNAL(missedCallsSeen()),
		SLOT(resetUnseenMissedCalls())
	);

	SAFE_CONNECT(_historyWidget,
		SIGNAL(showOnlyItemOfType(int)),
		SLOT(showOnlyItemOfTypeSlot(int))
	);
	qtWengoPhone->setQtHistoryWidget(_historyWidget);
	
	// define user use a Wengo or a SIP account
	UserProfile * userProfile = _cHistory.getCWengoPhone().getCUserProfileHandler().getUserProfileHandler().getCurrentUserProfile();
	if (userProfile) {
		_isWengoAccountConnected = userProfile->hasWengoAccount();
	} else {
		_isWengoAccountConnected = false;
	}
	////

	updatePresentation();
}

QtHistory::~QtHistory() {
	delete _mutex;
	
	OWSAFE_DELETE(_chatLogViewer);
	OWSAFE_DELETE(_historyWidget);
}

QWidget * QtHistory::getWidget() const {
	return _historyWidget->getWidget();
}

void QtHistory::historyLoadedEvent() {
}

void QtHistory::updatePresentation() {

	reset();
	_mementoIdList.clear();

	HistoryMementoCollection * collection = _cHistory.getHistory().getHistoryMementoCollection();
	for (HistoryMap::iterator it = collection->begin(); it != collection->end(); it++) {
		HistoryMemento * memento = (*it).second;

		if ((memento->getState() == _stateFilter) || (_stateFilter == HistoryMemento::Any)) {
			_mementoIdList << it->first;
		}
	}
	HistoryMementoSorter sorter(collection);
	qSort(_mementoIdList.begin(), _mementoIdList.end(), sorter);
}
//VOXOX - SEMR - 2009.05.08 
void QtHistory::showOnlyItemOfTypeSlot(int state) {
	_stateFilter = (HistoryMemento::State)state;
	updatePresentation();
}
void QtHistory::getMementosByState(){
	reset();
	_mementoIdList.clear();
	
	HistoryMementoCollection * collection = _cHistory.getHistory().getMementos(HistoryMemento::ChatSession);
	HistoryMap::const_iterator
		it = collection->begin(),
		end = collection->end();
	for (; it != end; ++it) {
		_mementoIdList << it->first;
	}

	HistoryMementoSorter sorter(collection);
	qSort(_mementoIdList.begin(), _mementoIdList.end(), sorter);
}

void QtHistory::showChatLog(int id) {
	if (id != 0){
		HistoryMementoCollection * collection = _cHistory.getHistory().getHistoryMementoCollection();
		HistoryMemento* memento = collection->getMemento(id);

		QtWengoPhone * qtWengoPhone = (QtWengoPhone *) _cHistory.getCWengoPhone().getPresentation();

		if(memento->getState() == HistoryMemento::ChatSession){
			_chatLogViewer = new QtChatLogViewer(NULL, *qtWengoPhone, QString::fromStdString(memento->getData()));
			_chatLogViewer->show();
		}
	}
}

void QtHistory::replayItem(int id) {
	if (id != 0){
	HistoryMementoCollection * collection = _cHistory.getHistory().getHistoryMementoCollection();
	HistoryMemento* memento = collection->getMemento(id);
	QString data = QString::fromStdString(memento->getData());

	QtWengoPhone * qtWengoPhone = (QtWengoPhone *) _cHistory.getCWengoPhone().getPresentation();
	//VOXOX - CJC - 2009.06.10 
	QtVoxMessageBox box(_historyWidget->getWidget());
	box.setWindowTitle("VoxOx - Call History");
	box.setText(tr("Do you want to call %1?").arg(formatName(memento->getPeer(), _isWengoAccountConnected)));
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No );


	/*QMessageBox mb(tr("@product@ - Call History"),
		tr("Do you want to call %1?").arg(formatName(memento->getPeer(), _isWengoAccountConnected)),
		QMessageBox::Question,
		QMessageBox::Yes | QMessageBox::Default,
		QMessageBox::No | QMessageBox::Escape,
		QMessageBox::NoButton, _historyWidget->getWidget());*/

	switch (memento->getState()) {
	case HistoryMemento::OutgoingSmsOk:
	case HistoryMemento::OutgoingSmsNok: {
		//Retrieve info & configure the Sms widget
		std::string data = _cHistory.getMementoData(id);
		QString text = QString::fromUtf8(data.c_str(), data.size());
		QString phoneNumber = QString::fromStdString(_cHistory.getMementoPeer(id));

		//Test existance of Sms (available only if a WengoAccount has been created)
		QtSms * sms = qtWengoPhone->getQtSms();
		if (sms) {
			sms->setText(text);
			sms->setPhoneNumber(phoneNumber);
			sms->getWidget()->show();
		}
		break;
	}

	case HistoryMemento::OutgoingCall:
	case HistoryMemento::IncomingCall:
	case HistoryMemento::MissedCall:
	case HistoryMemento::RejectedCall:
		if (box.exec() == QMessageBox::Yes) {
			_cHistory.replay(id);
		}
		break;

	case HistoryMemento::ChatSession:
		_chatLogViewer = new QtChatLogViewer(NULL, *qtWengoPhone, data );
		_chatLogViewer->restartChat();
		break;

	default:
		break;
		}
	}
}

void QtHistory::removeItem(int id) {
	int firstRow = _mementoIdList.indexOf(id);
	if (firstRow == -1) {
		return;
	}
	beginRemoveRows(QModelIndex(), firstRow, firstRow);
	_cHistory.removeHistoryMemento(id);
	_mementoIdList.removeAll(id);	
	endRemoveRows();
}

void QtHistory::mementoAddedEvent(unsigned id) {
	beginInsertRows(QModelIndex(), 0, 0);
	_mementoIdList.insert(0, id);
	endInsertRows();
}

void QtHistory::mementoUpdatedEvent(unsigned id) {
}

void QtHistory::mementoRemovedEvent(unsigned id) {
	int firstRow = _mementoIdList.indexOf(id);
	if (firstRow == -1) {
		updatePresentation();//VOXOX CHANGE by Rolando - 2009.10.07 
		return;
	}
	LOG_WARN("We should not reach this code");
	// We should not reach this code because the memento has probably been
	// removed by removeItem, which should have removed the id from our list
	beginRemoveRows(QModelIndex(), firstRow, firstRow);
	_mementoIdList.removeAll(id);
	endRemoveRows();

}

void QtHistory::unseenMissedCallsChangedEvent(int count) {
}

void QtHistory::resetUnseenMissedCalls() {
	_cHistory.resetUnseenMissedCalls();
}

void QtHistory::clearAllEntries() {
	_cHistory.clear(HistoryMemento::Any);
}

void QtHistory::clearSmsEntries() {
	_cHistory.clear(HistoryMemento::OutgoingSmsOk);
}

void QtHistory::clearChatEntries() {
	_cHistory.clear(HistoryMemento::ChatSession);
}

void QtHistory::clearIncomingCallEntries() {
	_cHistory.clear(HistoryMemento::IncomingCall);
}

void QtHistory::clearOutgoingCallEntries() {
	_cHistory.clear(HistoryMemento::OutgoingCall);
}

void QtHistory::clearMissedCallEntries() {
	_cHistory.clear(HistoryMemento::MissedCall);
}

void QtHistory::clearRejectedCallEntries() {
	_cHistory.clear(HistoryMemento::RejectedCall);
}

CHistory & QtHistory::getCHistory() const {
	return _cHistory;
}

int QtHistory::rowCount(const QModelIndex& parent) const {
	if (parent.isValid()) {
		return 0;
	}
	HistoryMementoCollection * collection = _cHistory.getHistory().getHistoryMementoCollection();
	return collection->size();
}

int QtHistory::columnCount(const QModelIndex&) const {
	return 4;
}

QVariant QtHistory::data(const QModelIndex& index, int role) const {
	if (index.row() < 0 || index.row() >= _mementoIdList.size()) {
		return QVariant();
	}
	int id = _mementoIdList[index.row()];
	HistoryMementoCollection * collection = _cHistory.getHistory().getHistoryMementoCollection();
	HistoryMemento* memento = collection->getMemento(id);
	if (!memento) {
		LOG_ERROR("Couldn't get memento for id " + String::fromNumber(id));
		return QVariant();
	}

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case 0:
			return textForMementoState(memento->getState());

		case 1:
			return QVariant(formatName(memento->getPeer(), _isWengoAccountConnected));

		case 2:
			return QVariant(formatDate(qDateTimeForMemento(memento)));
		case 3:
			//VOXOX - CJC - 2009.05.31 Only show duration for calls
			if(memento->getState() == HistoryMemento::OutgoingCall || memento->getState()== HistoryMemento::IncomingCall
				||memento->getState() == HistoryMemento::MissedCall || memento->getState() == HistoryMemento::RejectedCall){
					return QVariant(formatDuration(qTimeForDuration(memento->getDuration())));
				}else{
					return QVariant("");
			}

		default:
			return QVariant();
		}
	} else if (role == Qt::DecorationRole) {
		if (index.column() == 0) {
			return QVariant(iconForMementoState(memento->getState()));
		} else {
			return QVariant();
		}
	} else if (role == Qt::UserRole) {
		return QVariant(id);
	}
	return QVariant();
}

QVariant QtHistory::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role != Qt::DisplayRole) {
		return QVariant();
	}

	if (orientation == Qt::Vertical) {
		return QVariant();
	}

	switch (section) {
	case 0:
		return tr("Type");
	
	case 1:
		return tr("Name/Number");
	
	case 2:
		return tr("Date/Time");

	case 3:
		return tr("Duration");
	
	default:
		return QVariant();
	}
}
