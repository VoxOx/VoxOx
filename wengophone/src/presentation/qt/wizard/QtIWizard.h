/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* CLASS DESCRIPTION 
* @author Chris Jimenez C 
* @date 2009.06.01
*/



#ifndef QTIWIZARD_H
#define QTIWIZARD_H

#include <util/Interface.h>

class QWidget;
class QString;

/**
 * Interface for settings panel from the wizard window.
 *
 * @author CJC
 */
class QtIWizard : Interface {
public:

	virtual ~QtIWizard() { }

	virtual QWidget * getWidget() const = 0;

	virtual QString getName() const = 0;

	virtual QString getTitle() const = 0;

	virtual QString getDescription() const = 0;

	virtual int getStepNumber() const = 0;

	virtual QString getPixmapPath() const = 0;

	virtual void saveConfig() = 0;

	virtual void readConfig() = 0;

	virtual void postInitialize() { }

protected:

	
};

#endif	//QTISETTINGS_H
