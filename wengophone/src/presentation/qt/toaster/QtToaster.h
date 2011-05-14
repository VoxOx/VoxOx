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

#ifndef OWQTTOASTER_H
#define OWQTTOASTER_H

#include <QtCore/QObject>

class QWidget;
class QTimer;
class QFrame;

/**
 * This class codes the algorithm that show/hide the toaster.
 *
 * This class helps factorizing the code between QtCallToaster and QtChatToaster.
 *
 * @author Tanguy Krotoff
 */
class QtToaster : public QObject {
	Q_OBJECT
public:

	QtToaster(QWidget * toaster, QFrame * toasterWindowFrame);

	/**
	 * Sets the time with the toaster on top.
	 *
	 * @param time time toaster on top in milliseconds
	 */
	void setTimeOnTop(unsigned time);

	void show();

	void close();

private Q_SLOTS:

	void changeToasterPosition();

private:

	QWidget * _toaster;

	QTimer * _timer;

	bool _show;

	unsigned _timeOnTop;
};

#endif	//OWQTTOASTER_H
