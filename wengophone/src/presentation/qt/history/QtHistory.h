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

#ifndef QTHISTORY_H
#define QTHISTORY_H

#include <presentation/PHistory.h>

#include <model/history/HistoryMemento.h>

#include <QtCore/QAbstractTableModel>
#include <QtCore/QObject>
#include <QtCore/QMutex>

class CHistory;
class QtChatLogViewer;
class QtHistoryWidget;

/**
 * Qt Presentation component for history.
 *
 * It presents the history as a Qt model.
 *
 * One can get the memento id of a row using data(index, Qt::UserRole).
 *
 * @author Mathieu Stute
 * @author Aurelien Gateau
 */
class QtHistory : public QAbstractTableModel, public PHistory {
	Q_OBJECT
public:

	QtHistory(CHistory & cHistory);

	~QtHistory();

	virtual void updatePresentation();

	void getMementosByState();

	void clearAllEntries();

	void historyLoadedEvent();

	void mementoAddedEvent(unsigned id);

	void mementoUpdatedEvent(unsigned id);

	void mementoRemovedEvent(unsigned id);

	void unseenMissedCallsChangedEvent(int count);

	void clearSmsEntries();

	void clearChatEntries();

	void clearIncomingCallEntries();

	void clearOutgoingCallEntries();

	void clearMissedCallEntries();

	void clearRejectedCallEntries();

	QWidget * getWidget() const;
	
	CHistory & getCHistory() const;

	/**
	 * @name QAbstractTableModel implementation
	 * @{
	 */
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	/** @} */

	/**
	 * Format a date in the date column
	 * Public so that QtHistoryWidget can compute reasonable column widths
	 */
	static QString formatDate(const QDateTime&);

	/**
	 * Format the duration column
	 */
	static QString formatDuration(const QTime&);

	/**
	 * Return the text for the memento state. Not static because it uses tr()
	 */
	QString textForMementoState(const HistoryMemento::State& state) const;

private Q_SLOTS:

	void replayItem(int id);

	void showChatLog(int id);

	void removeItem(int id);

	void resetUnseenMissedCalls();
	//VOXOX - SEMR - 2009.05.08 
	void showOnlyItemOfTypeSlot(int state);


private:

	QString formatName(const std::string& name, bool isWengo) const;//VOXOX CHANGE by Rolando - 2009.10.06 

	QtHistoryWidget * _historyWidget;
	
	QtChatLogViewer * _chatLogViewer;

	HistoryMemento::State _stateFilter;

	CHistory & _cHistory;

	mutable QMutex * _mutex;
	
	bool _isWengoAccountConnected;

	QList<int> _mementoIdList;
};

#endif	//QTHISTORY_H
