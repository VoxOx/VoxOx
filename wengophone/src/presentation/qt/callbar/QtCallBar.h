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

#ifndef OWQTCALLBAR_H
#define OWQTCALLBAR_H

#include "QtVoxOxCallBarWidget.h"

#include <string>

class WengoStyleLabel;
//class QtPhoneComboBox;
class QComboBox;

class QString;
class QIcon;

/**
 * Call bar inside the main window.
 *
 * The call bas is composed with:
 * - a call button
 * - a hang up button
 * - a phone number combo box
 *
 * @author Tanguy Krotoff
 */
class QtCallBar : public QtVoxOxCallBarWidget{
	Q_OBJECT
public:

	QtCallBar(QWidget * parent);

	~QtCallBar();

	//void setEnabledCallButton(bool enable);

	//void setEnabledHangUpButton(bool enable);

	std::string getPhoneComboBoxCurrentText();

	void setPhoneComboBoxEditText(const std::string & text);

	/**
	 * Prepends an item to the combo box.
	 *
	 * @param text item to prepend
	 */
	void addPhoneComboBoxItem(const std::string & text);
	//VOXOX CHANGE CJC PUBLIC METHOD TO CLEAR CALL BAR TEXT BOX FROM ELSE WHERE
	void clearPhoneComboBoxEditText();

	/**
	 *	accessor to QtPhoneComboBox
	 *
	 * @return a pointor on the QtPhoneComboBox
	 */
	//QtPhoneComboBox * getQtPhoneComboBox();

	QComboBox * getQComboBox();

	std::string getCurrentAreaCode();

Q_SIGNALS:

	void callButtonClicked();

	void hangUpButtonClicked();

	void phoneComboBoxReturnPressed();

	void phoneComboBoxEditTextChanged(const QString & text);

	void phoneComboBoxClicked();

private Q_SLOTS:

	void callButtonClickedSlot();

	//void hangUpButtonClickedSlot();

	void phoneComboBoxReturnPressedSlot();

	void phoneComboBoxEditTextChangedSlot(const QString & text);

	void phoneComboBoxClickedSlot();

	void languageChanged();

private:

	void init();	

};

#endif	//OWQTCALLBAR_H
