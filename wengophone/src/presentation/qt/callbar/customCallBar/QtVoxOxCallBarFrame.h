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

#ifndef OWQTVOXOXCALLBARFRAME_H
#define OWQTVOXOXCALLBARFRAME_H

#include "ui_VoxOxCallBarFrame.h"

#include <QtGui/QListWidget>
#include <QtGui/QPixmap>
#include <QtGui/QComboBox>
#include "callBarUtilities/QtFlagsManager.h"
#include "callBarUtilities/QtFlagsListWidget.h"

class VoxOxFrame;
class QComboBox;
class VoxOxToolTipLineEdit;
class CUserProfile;
class QtFlagsManager;
class QtFlag;

namespace Ui { class VoxOxCallBarFrame; }

class QtVoxOxCallBarFrame: public VoxOxFrame{
	Q_OBJECT
public:

	QtVoxOxCallBarFrame(QWidget * parent = 0);

	~QtVoxOxCallBarFrame();

	
	void setCUserProfile(CUserProfile* cUserProfile);
	QString getCurrentCountryName();
	QString getCurrentAreaCode();
	QPixmap getCurrentFlagPixmap();
	QString getCurrentComboBoxText();
	QString getFullPhoneNumberText();
	QComboBox * getComboBox();
	void clearComboBox();
	void clearComboBoxEditText();
	VoxOxToolTipLineEdit * getVoxOxToolTipLineEdit();
	void addComboBoxItem(QString text) ;

	bool textIsPhoneNumber();

	//VOXOX CHANGE by Rolando - 2009.06.16 
	void updateStyleSheet(QString backgroundFlagColor = "none", 
						QString borderFlagColor = "none",
						QString flagTextColor = "#000000",
						QString backgroundComboBoxColor = "none", 
						QString borderComboBoxColor = "none", 
						QString backgroundGeneralColor = "none", 
						QString backgroundComboBoxDropDown = "none",
						QString primaryComboBoxTextColor = "\"black\"",
						QString secondaryComboBoxTextColor = "\"gray\"",
						QString comboBoxDropDownPixmapPath = "",
						QString flagDropPixmapPath = "");//VOXOX CHANGE by Rolando - 2009.06.12 

	void setFullPhoneNumber(QString fullPhoneNumber);
	QString getPhoneNumberOnlyDigits(QString phoneNumber);

protected:
	void initFlagListWidget();
	void setFlagButtonPixmap(QString countryName);
	void setCountryCodeText(QString countryName);	
	const QtFlag getFlagByPhoneNumber(QtFlagsManager * qtFlagsManager, QString flagsNameLanguage, QString text,QString prefix, bool * error, QString * codeAreaFound );
	void fillComboBox();
	void clearOutgoingCalls();
	void fillCompletionList();

Q_SIGNALS:
	void currentFlagChanged(QString);
	void comboBoxTextChanged(QString);

public Q_SLOTS:
	void flagClickedSlot();
	void currentFlagChangedSlot(QString currentCountryName);
	void comboBoxTextChangedSlot(QString newText);
	void itemActivatedComboBoxSlot(int);
	void keyPressedSlot(int);

private:
	Ui::VoxOxCallBarFrame * _ui;
	QtFlagsListWidget * _qtFlagsListWidget;
	QtFlagsManager * _qtFlagsManager;
	VoxOxToolTipLineEdit * _voxOxToolTipLineEdit;
	CUserProfile * _cUserProfile;

};

#endif	//OWQTVOXOXCALLBARFRAME_H
