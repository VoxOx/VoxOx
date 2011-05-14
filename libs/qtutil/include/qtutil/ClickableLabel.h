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

#ifndef OWCLICKABLELABEL_H
#define OWCLICKABLELABEL_H

#include <qtutil/owqtutildll.h>

#include <QtGui/QLabel>

class QMouseEvent;

/**
 * Clickable QLabel.
 *
 * ClickableLabel is QLabel with QPushButton signals.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class OWQTUTIL_API ClickableLabel : public QLabel {
	Q_OBJECT
public:

	ClickableLabel(QWidget * parent);

Q_SIGNALS:

	/** Left click signal. */
	void clicked();

	/** Right click signal. */
	void rightClicked();

private:

	void mouseReleaseEvent(QMouseEvent * event);
};

#endif	//OWCLICKABLELABEL_H
