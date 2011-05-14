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

#ifndef OWQTPHONECOMBOBOX_H
#define OWQTPHONECOMBOBOX_H

#include <qtutil/UpQComboBox.h>

#include <QtGui/QComboBox>
#include <QtGui/QPalette>
#include <QtCore/QString>

class QtPhoneComboBoxLineEdit;
class QtWengoPhone;

class QWidget;
class QMouseEvent;
class QKeyEvent;
class QFocusEvent;
class QString;
class QStringList;
class QMenu;

/**
 * QComboBox based on ToolTipLineEdit.
 *
 * Permits to add a message like:
 * "Please enter a phone number"
 *
 * FIXME factorize code with ToolTipLineEdit
 *
 * @author Tanguy Krotoff
 */
class QtPhoneComboBox : public UpQComboBox {
	Q_OBJECT
	friend class QtPhoneComboBoxLineEdit;
public:

	QtPhoneComboBox(QWidget * parent);

	~QtPhoneComboBox();

	QString currentText() const;

	void setQtWengoPhone(QtWengoPhone* newQtWengoPhone);

public Q_SLOTS:

	void setEditText(const QString & text);

	void languageChanged();
	
public :

	void setTextWithCompletion(const QString & newText);

private Q_SLOTS:

	void comboBoxActivated();

	void popupDisplayedSlot();

	void popupHiddenSlot();

	void itemhighlighted(const QString & text);

private:

	void init();

	void setToolTipText();

	void clearLineEdit();

	void clearList();

	void insertPhoneNumber(QStringList * phoneList, std::string toBeAdded);

	void mousePressEvent(QMouseEvent * event);

	void keyPressEvent(QKeyEvent * event);

	void focusOutEvent(QFocusEvent * event);

	bool _cleared;

	/** Tool tip text already set? */
	bool _toolTipTextDone;

	QString _toolTip;

	QString _lastHighlighted;

	QPalette _originalPalette;

	QPalette _greyPalette;

	QtWengoPhone * _qtWengoPhone;
	
	QStringList _totalCompletionList;
	
	QStringList _restrainedCompletionList;
};

#include <QtGui/QLineEdit>

class QtPhoneComboBoxLineEdit : public QLineEdit {
	Q_OBJECT
public:

	QtPhoneComboBoxLineEdit(QtPhoneComboBox * qtPhoneComboBox);
	
private Q_SLOTS:

	void textEditedSlot(const QString & newText);

private:

	void mousePressEvent(QMouseEvent * event);

	void keyPressEvent(QKeyEvent * event);

	QtPhoneComboBox * _qtPhoneComboBox;
	
	QString _enteredText;
};

#endif	//OWQTPHONECOMBOBOX_H
