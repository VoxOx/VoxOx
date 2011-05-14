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

#ifndef OWCONTROL_H
#define OWCONTROL_H

#include <util/Interface.h>
#include <util/Trackable.h>

class CWengoPhone;
class Presentation;
class Control;

/**
 * @defgroup control Control Component
 *
 * Like the model component, the control is part of the heart of WengoPhone.
 * It is developped in pure C++ using libraries like LibUtil, Boost, TinyXML and Curl.
 * This component has a very limited 'intelligence', everything is in fact done
 * inside the model component.
 *
 * The control component goal is to make the jonction between the model component
 * and the presentation component.
 *
 * The control component receives informations from the model component via the design
 * pattern observer. On the other hand it sends informations to the model component directly.
 * The control component dialogs with the presentation component via interfaces thus allowing
 * different graphical interfaces to be implemented (Qt, GTK+...).
 *
 * Control component deal with the switch between the thread from the model and the thread
 * from the presentation component.
 *
 * All classes inside the control component are named using a leading 'C' letter and
 * are inside the subdirectory control.
 */

/**
 * Interface for control component classes.
 *
 * @ingroup control
 * @author Tanguy Krotoff
 */
class Control : Interface, public Trackable {
public:

	virtual ~Control() {
	}

	/**
	 * Gets the Presentation associated with this Control.
	 *
	 * @return Presentation layer associated with this Control
	 */
	virtual Presentation * getPresentation() const = 0;

	/**
	 * Gets the root Control object.
	 *
	 * The root Control object is the one that creates all the Control objects.
	 *
	 * @return root Control object
	 */
	virtual CWengoPhone & getCWengoPhone() const = 0;

protected:

	/**
	 * Creates the presentation component in a thread safe way (via a postEvent()).
	 */
	virtual void initPresentationThreadSafe() = 0;
};

#endif	//OWCONTROL_H
