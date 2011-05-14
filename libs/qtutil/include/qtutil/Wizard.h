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

#ifndef OWWIZARD_H
#define OWWIZARD_H

#include <qtutil/owqtutildll.h>

#include <util/NonCopyable.h>

#include <QtCore/QObject>

class QWidget;
class QDialog;
namespace Ui { class Wizard; }

/**
 * Wizard dialog for Qt.
 *
 * @see http://doc.trolltech.com/4.1/dialogs-simplewizard.html
 * @see http://doc.trolltech.com/4.1/dialogs-complexwizard.html
 * @see http://doc.trolltech.com/3.3/qwizard.html
 * @author Tanguy Krotoff
 */
class OWQTUTIL_API Wizard : public QObject, NonCopyable {
	Q_OBJECT
public:

	Wizard(QWidget * parent);

	~Wizard();

	void show();

	void addPage(QWidget * page);

Q_SIGNALS:

	void finished();

private Q_SLOTS:

	void backButtonClicked();

	void nextButtonClicked();

	void finishedButtonClicked();

private:

	void switchPage(QWidget * oldPage);

	Ui::Wizard * _ui;

	QDialog * _wizardDialog;

	QList<QWidget *> _history;
};

#endif	//OWWIZARD_H
