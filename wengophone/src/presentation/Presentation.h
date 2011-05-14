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

#ifndef OWPRESENTATION_H
#define OWPRESENTATION_H

#include <util/Interface.h>

/**
 * @defgroup presentation Presentation Component
 *
 * The presentation includes all the classes implementing the graphical interface.
 * Different graphical interfaces can be implemented (Qt, GTK+...).
 *
 * The control component dialogs with the presentation via interfaces.
 * On the other hand the presentation dialogs directly with the control component.
 *
 * All classes inside the presentation component are named using a leading 'P' letter and
 * are inside the subdirectory presentation.
 *
 * Classes specific to the Qt graphical interface are inside the directory qt,
 * classes specific to the GTK+ graphical interface are inside the directory gtk ect...
 */

/**
 * Interface between the control component and the presentation component.
 *
 * The control component deals with the presentation component only via
 * this interface and its subclass. Thus control and presentation
 * are totally separated.
 *
 * @ingroup presentation
 * @author Tanguy Krotoff
 */
class Presentation : Interface {
public:

	virtual ~Presentation() {
	}

	/**
	 * Updates the presentation component.
	 *
	 * Cannot call this method update() since QWidget also
	 * contains a method update()
	 */
	virtual void updatePresentation() = 0;
};

#endif	//OWPRESENTATION_H
