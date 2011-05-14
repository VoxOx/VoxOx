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
#include "testdropdownwindow.h"

#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>

#include <qtutil/DropDownWindow.h>

TestWindow::TestWindow()
: QWidget() {
	_button = new QToolButton(this);
	_button->setText("&Click Me");
	_button->setCheckable(true);

	_window = new DropDownWindow(_button);
	_window->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

	QLabel* label = new QLabel(_window);
	label->setText("Some random content");
	QCheckBox* checkBox = new QCheckBox(_window);
	checkBox->setText("A check box");

	QVBoxLayout* layout = new QVBoxLayout(_window);
	layout->addWidget(label);
	layout->addWidget(checkBox);

	layout = new QVBoxLayout(this);
	layout->addWidget(_button);
}


int main(int argc, char ** argv) {
	QApplication app(argc, argv);

	QWidget* widget = new TestWindow();
	widget->show();

	return app.exec();
}
