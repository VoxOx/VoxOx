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
#include <qtutil/ScrollFrame.h>

#include <QtCore/QEvent>

#include <QtGui/QToolButton>
#include <QtGui/QHBoxLayout>

const int SCROLL_STEP = 32;

// FIXME: This should probably not be hardcoded
const int SCROLL_BUTTON_WIDTH = 16;


ScrollFrame::ScrollFrame(QWidget* parent)
: QFrame(parent)
, _child(0)
, _scrollStep(SCROLL_STEP)
{
	_viewport = new QWidget(this);
	_viewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	_leftButton = new QToolButton(this);
	_leftButton->setArrowType(Qt::LeftArrow);
	_leftButton->setAutoRepeat(true);
	_leftButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	_leftButton->setFixedWidth(SCROLL_BUTTON_WIDTH);
	_leftButton->setAutoRaise(true);
	connect(_leftButton, SIGNAL(clicked()), SLOT(scrollLeft()) );

	_rightButton = new QToolButton(this);
	_rightButton->setArrowType(Qt::RightArrow);
	_rightButton->setAutoRepeat(true);
	_rightButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	_rightButton->setFixedWidth(SCROLL_BUTTON_WIDTH);
	_rightButton->setAutoRaise(true);
	connect(_rightButton, SIGNAL(clicked()), SLOT(scrollRight()) );

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(_viewport);
	layout->addWidget(_leftButton);
	layout->addWidget(_rightButton);

	updateButtons();

}

void ScrollFrame::setChild(QWidget* child) {
	if (_child) {
		_child->removeEventFilter(this);
	}
	_child = child;
	_child->setParent(_viewport);
	_child->installEventFilter(this);
	int childWidth = _child->sizeHint().width();
	int childHeight = _child->sizeHint().height();
	_child->setGeometry(0, 0, childWidth, childHeight);
	updateGeometry();

}

void ScrollFrame::resizeEvent(QResizeEvent* event) {
	QFrame::resizeEvent(event);
	tryToFitChild();
}

QSize ScrollFrame::minimumSizeHint() const {
	QSize hint;
	hint.setWidth(_rightButton->sizeHint().width() * 3);

	if (_child) {
		hint.setHeight(_child->minimumSizeHint().height());
	} else {
		hint.setHeight(-1);
	}
	return hint;
}

QSize ScrollFrame::sizeHint() const {
	if (_child) {
	    return _child->sizeHint();
	} else {
	    return minimumSizeHint();
	}
}

void ScrollFrame::tryToFitChild() {
	if (_child->width() <= width()) {
	    _child->move(0, 0);
	} else {
	    int childVisibleWidth = _child->width() + _child->x();
	    if (childVisibleWidth < _viewport->width()) {
			int delta = _viewport->width() - childVisibleWidth;
			_child->move(_child->x() + delta, 0);
		}
	}
	updateButtons();
}
	
void ScrollFrame::updateButtons() {
	if (!_child) {
	    return;
	}
	
	if (_child->width() <= width()) {
	    _leftButton->setVisible(false);
	    _rightButton->setVisible(false);
	    return;
	}
	
	_leftButton->setVisible(true);
	_rightButton->setVisible(true);

	_leftButton->setEnabled(_child->x() < 0);

	int childVisibleWidth = _child->width() + _child->x();
	_rightButton->setEnabled(childVisibleWidth > _viewport->width());
}

void ScrollFrame::scrollLeft() {
	int posX = _child->x() + _scrollStep;
	if (posX > 0) {
	    posX = 0;
	}
	
	_child->move(posX, 0);
	updateButtons();
}

void ScrollFrame::scrollRight() {
	int posX = _child->x() - _scrollStep;
	if (posX + _child->width() < _viewport->width()) {
	    posX = _viewport->width() - _child->width();
	}
	
	_child->move(posX, 0);
	updateButtons();
}

bool ScrollFrame::eventFilter(QObject* object, QEvent* event) {
	Q_ASSERT(object == _child);
	if (event->type() == QEvent::Resize) {
		tryToFitChild();
	}
	return false;
}

int ScrollFrame::scrollStep() const {
	return _scrollStep;
}

void ScrollFrame::setScrollStep(int value) {
	_scrollStep = value;
}
