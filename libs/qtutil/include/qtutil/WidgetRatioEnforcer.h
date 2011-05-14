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
#ifndef WIDGETRATIOENFORCER_H
#define WIDGETRATIOENFORCER_H

#include <QtGui/QFrame>


/**
 * Adjust the geometry of a child widget so that it's centered and its size has
 * a specific ratio.
 *
 * Inherits from QFrame instead of QWidget for easy integration in Designer
 * 4.1.
 */
class WidgetRatioEnforcer : public QFrame {
public:
	WidgetRatioEnforcer(QWidget* parent=0);

	/**
	 * Defines the ratio to enforce. A ratio of 2.0 means the child width
	 * should be equal to twice the child height.
	 */
	void setRatio(double ratio);

	/**
	 * Defines the widget whose geometry must be enforced. child should be a
	 * direct child of the WidgetRatioEnforcer.
	 */
	void setChild(QWidget* child);

	virtual int heightForWidth(int) const;
	
	virtual QSize sizeHint() const;

	virtual QSize minimumSizeHint() const;

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	void updateChild(void);

	QWidget* _child;
	double _ratio;
};

#endif /* WIDGETRATIOENFORCER_H */
