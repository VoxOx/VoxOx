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

#include <qtutil/WidgetBackgroundImage.h>

#include <QtCore/QString>
#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtGui/QPalette>
#include <QtGui/QBrush>

void WidgetBackgroundImage::setBackgroundImage(QWidget * widget, const char * imageFile, WidgetBackgroundImage::AdjustMode adjustMode) {
	widget->setAutoFillBackground(true);

	QPixmap pixmap(imageFile);

	switch (adjustMode) {
	case AdjustNone:
		break;
	case AdjustWidth:
		widget->setMinimumWidth(pixmap.width());
		break;
	case AdjustHeight:
		widget->setMinimumHeight(pixmap.height());
		break;
	case AdjustSize:
		widget->resize(pixmap.size());
		widget->setMinimumSize(pixmap.size());
		break;
	}

	QPalette palette = widget->palette();
	palette.setBrush(widget->backgroundRole(), QBrush(pixmap));
	widget->setPalette(palette);
}
