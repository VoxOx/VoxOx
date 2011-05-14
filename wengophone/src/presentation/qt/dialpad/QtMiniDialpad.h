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

#ifndef OWQTMINIDIALPAD_H
#define OWQTMINIDIALPAD_H

#include <util/NonCopyable.h>

#include <QtGui/QWidget>
#include <QtCore/QTimer>

#include <string>

class QtWengoPhone;
class CDtmfThemeManager;
class QtSVGDialpad;
class QStringList;
class UpQComboBox;

namespace Ui { class MiniDialpadWidget; }

/**
 * Dialpad widget: where the user clicks for DTMF.
 *
 * @author Tanguy Krotoff
 */
class QtMiniDialpad : public QObject, NonCopyable {
	Q_OBJECT
public:
	enum ThemeMode { ThemeModeAll, ThemeModeDefaultOnly, ThemeModeAudioSmileysOnly };

	QtMiniDialpad(CDtmfThemeManager & cDtmfThemeManager, QtWengoPhone * qtWengoPhone, QString firstParticipantPhoneNumber = "", QString currentDuration = "", QString secondParticipantPhoneNumber = "");

	~QtMiniDialpad();
	//VOXOX CHANGE by Rolando 04-02-09
	void setThemeMode(ThemeMode);

	void setFirstParticipantPhoneNumber(QString firstParticipantPhoneNumber);

	void setSecondParticipantPhoneNumber(QString secondParticipantPhoneNumber);

	void setDurationTime(int seconds);

	void keyPressEvent ( QKeyEvent * event );//VOXOX CHANGE by Rolando - 2009.05.29

	QWidget * getWidget(){ return _miniDialpadWidget;}

Q_SIGNALS:
	void backButtonFunClicked();

	void backMiniDialpadClicked();

	void hangUpMiniDialpadClicked();

private Q_SLOTS:

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

	//VOXOX CHANGE by Rolando 04-02-09
	void backButtonClicked();

	void backButtonFunClickedSlot();

	void updateCallDuration();

	void hangUpButtonClicked();

private:

	//void fillComboBox();

	void playTone(const std::string & tone);

	Ui::MiniDialpadWidget * _ui;

	QtWengoPhone * _qtWengoPhone;

	QWidget * _miniDialpadWidget;

	CDtmfThemeManager & _cDtmfThemeManager;

	ThemeMode _themeMode;

	QString _firstParticipantPhoneNumber;
	QString _currentDuration;
	QString _secondParticipantPhoneNumber;

	QTimer * _callTimer;
	int _duration;
};

#endif	//OWQTMINIDIALPAD_H
