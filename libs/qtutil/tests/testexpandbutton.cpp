/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#include "testexpandbutton.h"

#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <qtutil/ExpandButton.h>

TestWindow::TestWindow()
: QWidget() {
	_button = new ExpandButton(this);
	_button->setText("&An expand button");
	_label = new QLabel(this);
	_label->setText("Some random content");
	_label->hide();

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(_button);
	layout->addWidget(_label);

	connect(_button, SIGNAL(toggled(bool)), SLOT(showAdvancedSettings(bool)));
}

void TestWindow::showAdvancedSettings(bool show) {
	_label->setVisible(show);

	layout()->invalidate();
	int height = layout()->sizeHint().height();

	setMinimumHeight(height);
	resize(width(), height);
}

int main(int argc, char ** argv) {
	Q_INIT_RESOURCE(qtutil);
	QApplication app(argc, argv);

	QWidget* widget = new TestWindow();
	widget->show();

	return app.exec();
}
