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

#ifndef OWWIDGET_H
#define OWWIDGET_H

#include <qtutil/owqtutildll.h>

#include <util/NonCopyable.h>

class QWidget;
class QGridLayout;
class QDialog;

/**
 * Helper functions for QWidget.
 *
 * @author Tanguy Krotoff
 */
class Widget : NonCopyable {
public:

	/**
	 * Creates a QGridLayout inside a widget.
	 *
	 * @param parent QWidget where to create the layout
	 * @return QGridLayout
	 */
	OWQTUTIL_API static QGridLayout * createLayout(QWidget * parent);

	/**
	 * Transforms a QWidget into a QDialog.
	 *
	 * @param widget QWidget to transform into a QDialog
	 * @return the QDialog created
	 */
	OWQTUTIL_API static QDialog * transformToWindow(QWidget * widget);
};

#endif	//OWWIDGET_H
