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

#ifndef OWQTCONFERENCECALLDIALOG_H
#define OWQTCONFERENCECALLDIALOG_H

#include <QtGui/QDialog>

class IPhoneLine;
class CWengoPhone;
class QMenu;
class QPoint;

namespace Ui { class ConferenceCallDialog; }

/**
 *
 * @author Mathieu Stute
 */
class QtConferenceCallDialog : public QDialog {
	Q_OBJECT

public:

	/**
	 * Constructor.
	 */
	QtConferenceCallDialog(QWidget * parent, CWengoPhone & cWengoPhone, IPhoneLine * phoneLine , bool startConference = true);

	/**
	 * Destructor.
	 */
	virtual ~QtConferenceCallDialog();

	void setFirstPeer(std::string peer);

	QString getSecondPeer();

private Q_SLOTS:

	/**
	 * The user has clicked the start button.
	 */
	void startClicked();

	void peer1ToolButtonClicked();

	void peer2ToolButtonClicked();

	void updateLineEdit(QAction * action);

private:
	
	bool _startConference;

	void populateMenus();

	void showMenu(QPoint point);

	IPhoneLine * _phoneLine;

	QMenu * _menu;

	QMenu * _wengoMenu;
	
	QMenu * _sipMenu;

	QMenu * _landLineMenu;

	QMenu * _mobileMenu;

	CWengoPhone & _cWengoPhone;

	int _currentPeer;

	Ui::ConferenceCallDialog * _ui;
};

#endif	//OWQTCONFERENCECALLDIALOG_H
