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

#ifndef QTISETTINGS_H
#define QTISETTINGS_H

#include <util/Interface.h>

class QWidget;
class QString;

/**
 * Interface for settings panel from the configuration window.
 *
 * @author Tanguy Krotoff
 */
class QtISettings : Interface {
public:

	virtual ~QtISettings() { }

	virtual QWidget * getWidget() const = 0;

	virtual QString getName() const = 0;

	virtual QString getTitle() const = 0;

	virtual QString getDescription() const = 0;

	virtual QString getIconName() const = 0;

	virtual void saveConfig() = 0;

	virtual void postInitialize() { }

protected:

	virtual void readConfig() = 0;
};

#endif	//QTISETTINGS_H
