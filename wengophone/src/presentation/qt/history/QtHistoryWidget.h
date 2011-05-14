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

#ifndef OWQTHISTORYWIDGET_H
#define OWQTHISTORYWIDGET_H

#include <QtCore/QObject>

class QSortFilterProxyModel;
class QModelIndex;

class CHistory;
class QtCallBar;
class QtHistory;
class QtToolBar;
class QMenu;

namespace Ui { class HistoryWidget; }

/**
 * History Widget.
 *
 * Shows call/sms/chat history
 *
 * @author Mathieu Stute
 * @author Tanguy Krotoff
 */
class QtHistoryWidget : public QObject {
	Q_OBJECT
public:

	QtHistoryWidget(QWidget * parent, QtHistory * qtHistory);

	~QtHistoryWidget();

	QWidget * getWidget() const;

	void setQtToolBar(QtToolBar * qtToolBar) {
		_qtToolBar = qtToolBar;
	}

	void setQtCallBar(QtCallBar * qtCallBar) {
		_qtCallBar = qtCallBar;
	}
	
	void replayCurrentItem(); //VOXOX - SEMR - 2009.07.10 
	void removeCurrentItem(); //VOXOX - SEMR - 2009.07.10 
	void showLogCurrentItem(); //VOXOX - SEMR - 2009.07.10 

public Q_SLOTS:

	void retranslateUi();
	
Q_SIGNALS:

	void replayItemRequested(int id);

	void showChatLogRequested(int id);

	void removeItemRequested(int id);

	void missedCallsSeen();

	void showOnlyItemOfType(int state);
	

private Q_SLOTS:

	//VOXOX - SEMR - 2009.05.07 
	void showOutGoingCall(bool checked);

	void showIncomingCall(bool checked);

	void showChat(bool checked);

	void showMissedCall(bool checked);

	void showRejectedCall(bool checked);

	void showAll(bool checked);

	void headerClicked(int logicalIndex);

	void updateCallBarFromIndex(const QModelIndex&);

	void showPopupMenuForCurrentIndex();

	void emitReplayItemRequested(const QModelIndex&);

	void emitShowChatLogRequested(const QModelIndex&);

private:

	Ui::HistoryWidget * _ui;

	QWidget * _historyWidget;

	QMenu * _menu;

	QtToolBar * _qtToolBar;

	QtCallBar * _qtCallBar;

	CHistory & _cHistory;
	
	QtHistory * _qtHistory;

	QSortFilterProxyModel * _proxyModel;

	void computeTreeViewColumnSizes();

	int idFromIndex(const QModelIndex& index);

	
	//VOXOX - SEMR - 2009.05.07 
	void populateMenu();


};

#endif	//OWQTHISTORYWIDGET_H
