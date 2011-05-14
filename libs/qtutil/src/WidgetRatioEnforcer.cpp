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
#include <qtutil/WidgetRatioEnforcer.h>

#include <QtCore/QEvent>
#include <QtGui/QWidget>

WidgetRatioEnforcer::WidgetRatioEnforcer(QWidget* parentWidget)
: QFrame(parentWidget)
, _child(0)
, _ratio(1.)
{
	QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	policy.setHeightForWidth(true);
	setSizePolicy(policy);
}

void WidgetRatioEnforcer::setRatio(double ratio) {
	Q_ASSERT(ratio!=0.);
	_ratio = ratio;
	updateChild();
}

void WidgetRatioEnforcer::setChild(QWidget* child) {
	_child = child;
	updateChild();
}

QSize WidgetRatioEnforcer::sizeHint() const {
	if (_child) {
		return _child->sizeHint();
	} else {
		return QFrame::sizeHint();
	}
}

QSize WidgetRatioEnforcer::minimumSizeHint() const {
	if (_child) {
		return _child->minimumSizeHint();
	} else {
		return QFrame::minimumSizeHint();
	}
}

int WidgetRatioEnforcer::heightForWidth(int width) const {
	if (!_child) {
		return -1;
	}

	if (_ratio == 0.) {
		return -1;
	}

	return qMin(int(width / _ratio), _child->maximumHeight());
}

void WidgetRatioEnforcer::resizeEvent(QResizeEvent*) {
	updateChild();
}

void WidgetRatioEnforcer::updateChild() {
	if (!_child) {
		return;
	}

	if (_ratio == 0.) {
		return;
	}

	int parentWidth = width();
	int parentHeight = height();

	int childWidth;
	int childHeight = int(parentWidth / _ratio);
	if (childHeight > _child->maximumHeight()) {
		childHeight = _child->maximumHeight();
	}

	if (childHeight <= parentHeight) {
	    childWidth = int(childHeight * _ratio);
	} else {
	    childWidth = int(parentHeight * _ratio);
		if (childWidth > _child->maximumWidth()) {
			childWidth = _child->maximumWidth();
		}
	    childHeight = int(childWidth / _ratio);
	}

	int posX = (parentWidth - childWidth) / 2;
	int posY = (parentHeight - childHeight) / 2;
	_child->setGeometry(posX, posY, childWidth, childHeight);
}
