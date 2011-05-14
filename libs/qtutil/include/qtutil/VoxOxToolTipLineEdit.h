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

#ifndef OWVOXOXTOOLTIPLINEEDIT_H
#define OWVOXOXTOOLTIPLINEEDIT_H

#include <qtutil/owqtutildll.h>

#include <QtGui/QLineEdit>
#include <QtGui/QPalette>
#include <QtCore/QString>

class QWidget;
class QMouseEvent;
class QKeyEvent;
//VOXOX CHANGE for VoxOx by Rolando 01-09-09 we use QEvent
class QFocusEvent;

/**
 * QLineEdit with a tooltip that disappears when the user clicks on it.
 *
 * @see QLineEdit
 * @author Tanguy Krotoff
 */
class OWQTUTIL_API VoxOxToolTipLineEdit : public QLineEdit {
	Q_OBJECT
public:

	VoxOxToolTipLineEdit(QWidget * parent);

	/**
	 * @see QLineEdit::text()
	 */
	QString text() const;

	QString realText() const;
	
	void displayToolTipMessage();

	void setToolTipDefaultText(const QString & text);
	
	void setPrimaryColorStyleSheet(QString stylesheet);

	void setSecondaryColorStyleSheet(QString stylesheet);
	
	void changeFocusToParent();//VOXOX CHANGE by Rolando - 2009.05.22 - method to send focus to another widget

Q_SIGNALS:

	void currentTextChanged(QString);

	void keyPressedSignal(int);

public Q_SLOTS:

	/**
	 * @see QLineEdit::setText()
	 */
	void setText(const QString & text);

	void textChangedSlot(QString) ;

	void languageChanged();

private:

	/**
	 * Initializes the ToolTopLineEdit.
	 */
	void init();	

	void clearLineEdit();

	void mousePressEvent(QMouseEvent * event);

	void enterEvent( QEvent * event );

	void keyPressEvent(QKeyEvent * event);

	void leaveEvent ( QEvent * event );

	void updateMessageText(QString text);

	void resizeEvent ( QResizeEvent * );//VOXOX CHANGE by Rolando - 2009.10.13 

	QString verifyText(const QRect & painterRect, QFont font, QString text);//VOXOX CHANGE by Rolando - 2009.10.13 

	int getMaximumCharsAllowed();//VOXOX CHANGE by Rolando - 2009.10.13 

	void updateCurrentText();//VOXOX CHANGE by Rolando - 2009.10.13 

	bool _cleared;

	/** Tool tip text already set? */
	bool _toolTipTextDone;

	QString _toolTip;

	QString _shortMessage;

	QString _message;


	QString _primaryStyleSheet;

	QString _secondaryStyleSheet;

	void updateStyleSheet(QString newStyleSheet);

	void repaintPrimaryColor();

	void repaintSecondaryColor();
	QWidget * _parentWidget;

	int _maximumWidthPixels;//VOXOX CHANGE by Rolando - 2009.10.13 
};

#endif	//OWVOXOXTOOLTIPLINEEDIT_H
