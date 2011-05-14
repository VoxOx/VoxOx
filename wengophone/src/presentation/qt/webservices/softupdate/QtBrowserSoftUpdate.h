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

#ifndef OWQTBROWSERSOFTUPDATE_H
#define OWQTBROWSERSOFTUPDATE_H

#include <QtCore/QObject>

#include <util/NonCopyable.h>

#include <string>

class QDialog;
namespace Ui { class BrowserSoftUpdateWindow; }

/**
 * Shows a window with the a clickable link to the WengoPhone download page.
 *
 * @author Tanguy Krotoff
 */
class QtBrowserSoftUpdate : public QObject, NonCopyable {
	Q_OBJECT
public:

	QtBrowserSoftUpdate(const std::string & downloadUrl,
				unsigned long long buildId,
				const std::string & version,
				unsigned fileSize, QWidget * parent);

	~QtBrowserSoftUpdate();

private Q_SLOTS:

	void showWengoDownloadWebPage();

private:

	Ui::BrowserSoftUpdateWindow * _ui;

	QDialog * _softUpdateWindow;
};

#endif	//OWQTBROWSERSOFTUPDATE_H
