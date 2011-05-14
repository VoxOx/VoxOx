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

#ifndef OWQTDIALPAD_H
#define OWQTDIALPAD_H

#include <util/NonCopyable.h>

#include <QtGui/QWidget>

#include <pixertool/pixertool.h>

#include <string>

class QtWengoPhone;
class CDtmfThemeManager;
class QtSVGDialpad;
class QStringList;
class UpQComboBox;
class QtPhoneCall;
class QtConferenceCallWidget;


namespace Ui { class DialpadWidget; }

/**
 * Dialpad widget: where the user clicks for DTMF.
 *
 * @author Tanguy Krotoff
 */
class QtDialpad : public QWidget, NonCopyable {
	Q_OBJECT
public:
	enum ThemeMode { ThemeModeAll, ThemeModeDefaultOnly, ThemeModeAudioSmileysOnly };

	QtDialpad(CDtmfThemeManager & cDtmfThemeManager, QtWengoPhone * qtWengoPhone);

	~QtDialpad();
	//VOXOX CHANGE by Rolando 04-02-09
	void setThemeMode(ThemeMode);

	void showInitiateConferenceCallWindow();

	void showClassicalDialpadWindow();

	void setEnabledTransferButton(bool enabled);

	bool existsFirstQtPhoneCall();

	void setDefaultPage();

	CDtmfThemeManager & getCDtmfThemeManager(){ return _cDtmfThemeManager;}

	void keyPressEvent ( QKeyEvent * event );//VOXOX CHANGE by Rolando - 2009.05.28 - added to handle keys pressed from 0-9 and "#", "*" keys

	bool isConferenceWindowShown();

	bool isKeypadWindowShown();

	QtConferenceCallWidget * getQtConferenceCallWidget(){ return _qtConferenceCallWidget;}

	void setPhoneNumber(QString phoneNumber);//VOXOX CHANGE by Rolando - 2009.06.23  - necessary to implement the sync between text in callbar and keypad window 

	QString getPhoneNumberOnlyDigits(QString phoneNumber);//VOXOX CHANGE by Rolando - 2009.06.23 

	void setKeypadStyleSheet(QString keypadBackgroundPath = "");//VOXOX - SEMR - 2009.07.31 change keypad background

private Q_SLOTS:

	//VOXOX CHANGE Rolando 04-21-09
	void hideConferenceWidget();

	/*void removeCallTab();*/

	void oneButtonClicked();

	void twoButtonClicked();

	void threeButtonClicked();

	void fourButtonClicked();

	void fiveButtonClicked();

	void sixButtonClicked();

	void sevenButtonClicked();

	void eightButtonClicked();

	void nineButtonClicked();

	void zeroButtonClicked();

	void starButtonClicked();

	void poundButtonClicked();
public Q_SLOTS:
	//VOXOX CHANGE by Rolando 04-07-09
	void voiceMailButtonClicked();

	//VOXOX CHANGE by Rolando 04-07-09
	void redial();

	//VOXOX CHANGE by Rolando 04-02-09
	void backButtonClicked();

	//VOXOX CHANGE by Rolando 04-02-09
	void callButtonClicked();

	//VOXOX CHANGE by Rolando - 2009.06.17 
	void smsButtonClicked();

	//VOXOX CHANGE by Rolando - 2009.06.17 
	void faxButtonClicked();

	//VOXOX CHANGE by Rolando 04-02-09
	void ratesButtonClicked();

	//VOXOX CHANGE by Rolando 04-02-09
	void conferenceButtonClicked();	

	//VOXOX CHANGE by Rolando 04-02-09
	//void setCurrentTheme(const QString & newThemeName);
	//VOXOX CHANGE by Rolando 04-02-09
	//void refreshComboBox();

private:
	void playTone(const std::string & tone);

	Ui::DialpadWidget * _ui;

	QtWengoPhone * _qtWengoPhone;

	CDtmfThemeManager & _cDtmfThemeManager;

	QtConferenceCallWidget * _qtConferenceCallWidget;

	ThemeMode _themeMode;

	QWidget * _defaultPage;
};

#endif	//OWQTDIALPADWIDGET_H
