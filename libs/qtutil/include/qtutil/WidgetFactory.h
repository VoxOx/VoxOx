/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef OWWIDGETFACTORY_H
#define OWWIDGETFACTORY_H

#include <qtutil/owqtutildll.h>

#include <util/NonCopyable.h>

#include <QtCore/QString>
#include <QtGui/QWidget>

/**
 * Replacement for QFormBuilder::load().
 *
 * Creates widgets dynamically (from a XML file .ui).
 *
 * @author Tanguy Krotoff
 */
class WidgetFactory : NonCopyable {
public:

	/**
	 * Creates a widget given its XML file descriptor.
	 *
	 * @see QFormBuilder::load()
	 */
	OWQTUTIL_API static QWidget * create(const QString & uiFile, QWidget * parent);
};

#endif	//OWWIDGETFACTORY_H
