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

#ifndef QTADVANCEDCONFIG_H
#define QTADVANCEDCONFIG_H

#include <model/config/Config.h>

#include <util/NonCopyable.h>

#include <QtGui/QDialog>

class QWidget;
class QDialog;
namespace Ui { class AdvancedConfigWindow; }

/**
 * Firefox' about:config like window.
 *
 * Shows the key/value settings.
 *
 * @author Tanguy Krotoff
 */
class QtAdvancedConfig : public QDialog, NonCopyable {
	Q_OBJECT
public:

	QtAdvancedConfig(QWidget * parent);

	~QtAdvancedConfig();

	void populate();

private Q_SLOTS:

	void saveConfig();

private:

	void setItem(boost::any value, int row, int column);

	Ui::AdvancedConfigWindow * _ui;
};

#endif	//QTADVANCEDCONFIG_H
