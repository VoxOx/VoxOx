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
#ifndef SCROLLFRAME_H
#define SCROLLFRAME_H

#include <QtGui/QFrame>

class QToolButton;

/**
 * A frame which will show buttons to scroll its content on the left or on the
 * right if it does not fit.
 *
 * @author Aurelien Gateau
 */
class ScrollFrame : public QFrame
{
	Q_OBJECT
public:
	ScrollFrame(QWidget* parent = 0);

	void setScrollStep(int);
	int scrollStep() const;

	/**
	 * Sets the child widget to show in the frame
	 */
	void setChild(QWidget* child);
	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;

protected:
	virtual void resizeEvent(QResizeEvent*);

	/**
	 * Reimplemented to get notified of changes in the child size
	 */
	virtual bool eventFilter(QObject*, QEvent*);

private Q_SLOTS:
	void scrollLeft();
	void scrollRight();

private:
	QWidget* _child;
	QWidget* _viewport;
	QToolButton* _leftButton;
	QToolButton* _rightButton;
	int _scrollStep;

	void updateButtons();
	void tryToFitChild();
};

#endif // SCROLLFRAME_H
