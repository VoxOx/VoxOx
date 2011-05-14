/****************************************************************************
** Filename: URLLabel.cpp
** Last updated [dd/mm/yyyy]: 14/02/2005
**
** QLabel subclass with URL handling and more.
**
** Copyright(C) 2005 Angius Fabrizio. All rights reserved.
**
** Based on the LGPL v.2 licensed KURLLabel from the KDE libraries by
** Kurt Granroth <granroth@kde.org> and Peter Putzer <putzer@kde.org>
**
** Changes made to the KURLLabel code:
**  - link color is no longer taken from KGlobalSettings but from qApp->palette().active().link()
**  - removed virtual_hook() member function
**  - replaced KCursor::handCursor() with QCursor(Qt::PointingHandCursor)
**  - added context menu (see mouseReleaseEvent method)
**
** This file is part of the OSDaB project(http://osdab.sourceforge.net/).
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See the file LICENSE.GPL that came with this software distribution or
** visit http://www.gnu.org/copyleft/gpl.html for GPL licensing information.
**
**********************************************************************/

#include <qtutil/VoxOxLinkLabel.h>

#include <QtGui/QApplication>
#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>


VoxOxLinkLabel::VoxOxLinkLabel(QWidget* parent)
	: QLabel(parent)
{

	setCursor(QCursor(Qt::PointingHandCursor));
}

void VoxOxLinkLabel::mouseReleaseEvent(QMouseEvent* e)
{
	QLabel::mouseReleaseEvent(e);

	switch(e->button()) {
	case Qt::LeftButton:
		linkActivated(_link);
		break;

	default:
		; // nothing
	}
}


void VoxOxLinkLabel::setLink(const QString& link)
{
	_link = link;
}

const QString& VoxOxLinkLabel::link() const
{
	return _link;
}

void VoxOxLinkLabel::enterEvent(QEvent* e)
{
	QLabel::enterEvent(e);
	QFont newFont = font();
	newFont.setUnderline(true);
	setFont(newFont);
	linkHovered(_link);
}

void VoxOxLinkLabel::leaveEvent(QEvent* e)
{
	QLabel::leaveEvent(e);
	QFont newFont = font();
	newFont.setUnderline(false);
	setFont(newFont);
}
