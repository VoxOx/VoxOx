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

#ifndef OWQTSTATUSBAR_H
#define OWQTSTATUSBAR_H


#include <util/Trackable.h>

#include <QtGui/QWidget>



class QStatusBar;

class QWidget;
class QAction;
class QMenu;

namespace Ui { class StatusBarWidget; }

/**
 * Main window status bar.
 *
 * Contains status icons for Internet connection, sound status, SIP (network) status...
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class QtStatusBar : public QWidget, public Trackable {
	Q_OBJECT
public:
	QtStatusBar(QStatusBar * statusBar);
	~QtStatusBar();

	void setAddMenu     (QMenu* addMenu,	  QString tooltip);
	void setSettingsMenu(QMenu* settingsMenu, QString tooltip);

Q_SIGNALS:

private Q_SLOTS:
	void languageChanged();

private:
	/**
	 * Initializes widgets content.
	 */
	void init();

	Ui::StatusBarWidget* _ui;

	QMenu*		_addMenu;
	QMenu*		_settingsMenu;
	QStatusBar* _statusBar;
};

#endif	//OWQTSTATUSBAR_H
