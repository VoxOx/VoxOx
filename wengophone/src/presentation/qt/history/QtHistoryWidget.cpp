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
#include "QtHistoryWidget.h"

#include "ui_HistoryWidget.h"

#include <model/history/HistoryMemento.h>
#include <model/phonecall/SipAddress.h>
#include <control/history/CHistory.h>
#include <presentation/qt/QtToolBar.h>
#include <presentation/qt/callbar/QtCallBar.h>
#include <presentation/qt/history/QtHistory.h>
#include <presentation/qt/chat/QtChatLogViewer.h>
#include <presentation/qt/contactlist/QtContactActionManager.h>



#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

QtHistoryWidget::QtHistoryWidget(QWidget * parent, QtHistory * qtHistory)
	: QObject(parent), 
	_cHistory(qtHistory->getCHistory()),
	_qtHistory(qtHistory) {

	_qtToolBar = NULL;
	_qtCallBar = NULL;
	_historyWidget = new QWidget(parent);

	_ui = new Ui::HistoryWidget();
	_ui->setupUi(_historyWidget);

	QIcon filterIcon(":/pics/history/filter.png");
	_ui->lineEdit->createLeftButton(filterIcon);

	_proxyModel = new QSortFilterProxyModel(this);
	_proxyModel->setSourceModel(qtHistory);
	_proxyModel->setFilterKeyColumn(1);
	_ui->treeView->setModel(_proxyModel);
	SAFE_CONNECT_RECEIVER(_ui->lineEdit, SIGNAL(textChanged(const QString&)),
		_proxyModel, SLOT(setFilterFixedString(const QString&)) );

//VOXOX CHANGE ASV 04-06-09: we need a margin on top for Mac so the tab bar doesn't get on top of the widget content.
#if defined(OS_MACOSX)
	_historyWidget->layout()->setContentsMargins(0, 11, 0, 0);
#endif

	computeTreeViewColumnSizes();

	//treeView
	SAFE_CONNECT(_ui->treeView,
		SIGNAL(doubleClicked(const QModelIndex&)),
		SLOT(emitShowChatLogRequested(const QModelIndex&))
		);
	SAFE_CONNECT(_ui->treeView,
		SIGNAL(clicked(const QModelIndex&)),
		SIGNAL(missedCallsSeen()) );

	SAFE_CONNECT(_ui->treeView->selectionModel(),
		SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
		SLOT(updateCallBarFromIndex(const QModelIndex&))
		);

	SAFE_CONNECT(_ui->treeView,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		SLOT(showPopupMenuForCurrentIndex())
		);

	SAFE_CONNECT(_ui->treeView->header(),
		SIGNAL(sectionClicked(int)), 
		SLOT(headerClicked(int))
		);

	_ui->treeView->header()->setClickable(true);
	//_ui->treeView->setSortingEnabled(true);

	_menu = new QMenu();
	populateMenu();

}

QtHistoryWidget::~QtHistoryWidget() {
	OWSAFE_DELETE(_ui);
	OWSAFE_DELETE(_historyWidget);
}

QWidget * QtHistoryWidget::getWidget() const {
	return _historyWidget;
}

void QtHistoryWidget::headerClicked(int logicalIndex) {
	if (!logicalIndex == 0) {
		return;
	}
	QCursor cursor;
	_menu->popup(cursor.pos());
}

void QtHistoryWidget::populateMenu() {
	
	_menu->clear();
	
	QAction * action = _menu->addAction(tr("Chat"));
	SAFE_CONNECT(action, SIGNAL(triggered(bool)), SLOT(showChat(bool)));

	action = _menu->addAction(tr("Outgoing call"));
	SAFE_CONNECT(action, SIGNAL(triggered(bool)), SLOT(showOutGoingCall(bool)));

	action = _menu->addAction(tr("Incoming call"));
	SAFE_CONNECT(action, SIGNAL(triggered(bool)), SLOT(showIncomingCall(bool)));

	action = _menu->addAction(tr("Rejected call"));
	SAFE_CONNECT(action, SIGNAL(triggered(bool)), SLOT(showRejectedCall(bool)));

	action = _menu->addAction(tr("Missed call"));
	SAFE_CONNECT(action, SIGNAL(triggered(bool)), SLOT(showMissedCall(bool)));

	action = _menu->addAction(tr("All"));
	SAFE_CONNECT(action, SIGNAL(triggered(bool)), SLOT(showAll(bool)));

}

void QtHistoryWidget::showOutGoingCall(bool) {
	showOnlyItemOfType((int)HistoryMemento::OutgoingCall);
}

void QtHistoryWidget::showIncomingCall(bool) {
	showOnlyItemOfType((int)HistoryMemento::IncomingCall);
}

void QtHistoryWidget::showChat(bool) {
	showOnlyItemOfType((int)HistoryMemento::ChatSession);
}

void QtHistoryWidget::showMissedCall(bool) {
	showOnlyItemOfType((int)HistoryMemento::MissedCall);
}

void QtHistoryWidget::showRejectedCall(bool checked) {
	showOnlyItemOfType((int)HistoryMemento::RejectedCall);
}

void QtHistoryWidget::showAll(bool) {
	showOnlyItemOfType((int)HistoryMemento::Any);
}

int QtHistoryWidget::idFromIndex(const QModelIndex& proxyIndex) {
	QModelIndex sourceIndex = _proxyModel->mapToSource(proxyIndex);
	return _qtHistory->data(sourceIndex, Qt::UserRole).toInt();
}

void QtHistoryWidget::emitReplayItemRequested(const QModelIndex& index) {
	int id = idFromIndex(index);
	replayItemRequested(id);
}

void QtHistoryWidget::emitShowChatLogRequested(const QModelIndex& index) {
	if (!index.isValid()) {
		return;
	}
	
	int id = idFromIndex(index);
	HistoryMementoCollection * collection = _cHistory.getHistory().getHistoryMementoCollection();
	HistoryMemento* memento = collection->getMemento(id);

	if(memento){//VOXOX CHANGE by Rolando - 2009.10.06 
	//Q_ASSERT(memento);//VOXOX CHANGE by Rolando - 2009.10.06 

		QMenu menu(_ui->treeView);

		if (id){
			if (memento->getState() == HistoryMemento::ChatSession){
				showChatLogRequested(id);
			}
			else{
				replayItemRequested(id);
			}
		}
	}
}

void QtHistoryWidget::updateCallBarFromIndex(const QModelIndex& index) {
	if (!index.isValid()) {
		return;
	}

	int id = idFromIndex(index);
	HistoryMementoCollection * collection = _cHistory.getHistory().getHistoryMementoCollection();
	HistoryMemento* memento = collection->getMemento(id);
	if (!memento || !_qtCallBar) {
		return;
	}

	if (memento->getState() != HistoryMemento::ChatSession) {
		SipAddress sipAddress = memento->getPeer();
		QString phoneNumber = QString::fromStdString(sipAddress.toString());//VOXOX CHANGE by Rolando - 2009.10.07
		if(phoneNumber.startsWith("011")){//VOXOX CHANGE by Rolando - 2009.10.07 
			phoneNumber.replace(0,3,"+");//VOXOX CHANGE by Rolando - 2009.10.07 
		}
		_qtCallBar->setPhoneComboBoxEditText(phoneNumber.toStdString());//VOXOX CHANGE by Rolando - 2009.10.07 
	}
}
void QtHistoryWidget::replayCurrentItem(){
	QModelIndex index = _ui->treeView->selectionModel()->currentIndex();
	if (!index.isValid()) {
		return;
	}
	
	int id = idFromIndex(index);
	if (id){
		replayItemRequested(id);
	}
}
void QtHistoryWidget::removeCurrentItem(){
	QModelIndex index = _ui->treeView->selectionModel()->currentIndex();
	if (!index.isValid()) {
		return;
	}
	
	int id = idFromIndex(index);
	if (id){
		removeItemRequested(id);
	}
}
void QtHistoryWidget::showLogCurrentItem(){
	QModelIndex index = _ui->treeView->selectionModel()->currentIndex();
	if (!index.isValid()) {
		return;
	}
	
	int id = idFromIndex(index);
	HistoryMementoCollection * collection = _cHistory.getHistory().getHistoryMementoCollection();
	HistoryMemento* memento = collection->getMemento(id);
	//Q_ASSERT(memento);//VOXOX CHANGE by Rolando - 2009.10.06 
	if(memento){//VOXOX CHANGE by Rolando - 2009.10.06 
		QMenu menu(_ui->treeView);

		if (id){
			if (memento->getState() == HistoryMemento::ChatSession){
				showChatLogRequested(id);
			}
		}
	}
}


void QtHistoryWidget::showPopupMenuForCurrentIndex() {
	QModelIndex index = _ui->treeView->selectionModel()->currentIndex();
	if (!index.isValid()) {
		return;
	}
	
	int id = idFromIndex(index);
	HistoryMementoCollection * collection = _cHistory.getHistory().getHistoryMementoCollection();
	HistoryMemento* memento = collection->getMemento(id);
	//Q_ASSERT(memento);//VOXOX CHANGE by Rolando - 2009.10.06 
	if(memento){//VOXOX CHANGE by Rolando - 2009.10.06 

		QMenu menu(_ui->treeView);
		
		QAction * replayAction = NULL;//VOXOX CHANGE by Rolando - 2009.10.07 - bug fixed when variable is not initialized
		QAction * showLogAction = NULL;//VOXOX CHANGE by Rolando - 2009.10.07 - bug fixed when variable is not initialized

		switch (memento->getState()) {
		case HistoryMemento::OutgoingSmsOk:
		case HistoryMemento::OutgoingSmsNok:
			replayAction = menu.addAction(tr("Open in SMS window"));
			break;
		case HistoryMemento::OutgoingCall:
		case HistoryMemento::IncomingCall:
		case HistoryMemento::MissedCall:
		case HistoryMemento::RejectedCall:
			replayAction = menu.addAction(tr("Call"));
			break;
		case HistoryMemento::ChatSession:
			showLogAction = menu.addAction(tr("Show Log..."));
			replayAction = menu.addAction(tr("Restart"));
			
			break;
		default:
			LOG_FATAL("Unhandled memento state");
			break;
		}

		QAction* deleteAction = menu.addAction(tr("Erase"));

		QAction* action = menu.exec(QCursor::pos());

		if(action){//VOXOX CHANGE by Rolando - 2009.10.07 
			
			if(replayAction){//VOXOX CHANGE by Rolando - 2009.10.07
				if (action == replayAction) {
					replayItemRequested(id);
				}
			}
				
			if(deleteAction){//VOXOX CHANGE by Rolando - 2009.10.07 
				if (action == deleteAction) {
					removeItemRequested(id);
				}
			}
			
			if(showLogAction){//VOXOX CHANGE by Rolando - 2009.10.07 
				if (action == showLogAction) {
					showChatLogRequested(id);
				}
			}
		}
	}
}
//void QtHistoryWidget::startChat() {
//	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
//	contactActionManager->chatContact(QString::fromStdString(getCurrentItemContactKey()));
//}

void QtHistoryWidget::retranslateUi() {
	_ui->retranslateUi(_historyWidget);
	_qtHistory->updatePresentation();
}

void QtHistoryWidget::computeTreeViewColumnSizes() {
	// Do *not* use QHeaderView::ResizeToContents because this cause the
	// widget to ask for the data of *all* the rows.
	QHeaderView* header = _ui->treeView->header();

	int margin = _ui->treeView->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

	// State
	// lst is the list of state with a text, see QtHistory::textForMementoState
	QList<HistoryMemento::State> lst;
	lst
		<< HistoryMemento::IncomingCall
		<< HistoryMemento::OutgoingCall
		<< HistoryMemento::MissedCall
		<< HistoryMemento::RejectedCall
		<< HistoryMemento::OutgoingSmsOk
		<< HistoryMemento::ChatSession;
	int width = 0;
	QFontMetrics fm = _ui->treeView->fontMetrics();
	/*Q_FOREACH(HistoryMemento::State state, lst) {
		QString text = _qtHistory->textForMementoState(state);
		width = qMax(width, fm.width(text));
	}*/
	width += _ui->treeView->style()->pixelMetric(QStyle::PM_ListViewIconSize);
	header->resizeSection(0, width + 4 * margin);

	// Name
	header->resizeSection(1, fm.width("a long long long long username"));

	// Date
	QString text = QtHistory::formatDate(QDateTime::currentDateTime());
	width = fm.width(text);
	header->resizeSection(2, width + 2 * margin);

	// Duration
	text = QtHistory::formatDuration(QTime(20, 0));
	width = fm.width(text);
	header->setResizeMode(3, QHeaderView::Interactive);
	header->resizeSection(3, width + 2 * margin);
}
