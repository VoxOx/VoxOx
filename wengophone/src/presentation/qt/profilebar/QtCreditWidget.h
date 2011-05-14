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

#ifndef OWQTCREDITWIDGET_H
#define OWQTCREDITWIDGET_H

#include <QtCore/QObject>
#include <QtCore/QString>

class CWengoPhone;
class UserProfile;

class QWidget;
namespace Ui { class CreditWidget; }

/**
 * Credit widget of the profile bar
 *
 * Shows the Wengo's credit available for the user + call forward status.
 *
 * @author Mathieu Stute
 */
class QtCreditWidget : public QObject {
	Q_OBJECT
public:

	QtCreditWidget(QWidget * parent);

	void init(CWengoPhone*, UserProfile *);

	virtual ~QtCreditWidget();

	void setCallForwardMode(const QString & callForwardMode);

	QWidget * getWidget() const;

	void updatePresentation();

private Q_SLOTS:

	void buyCreditsClicked();

	void callForwardModeClicked();

	void voiceMailClicked();

	void languageChanged();

private:

	bool isWengoConnexion();

	CWengoPhone * _cWengoPhone;

	UserProfile * _userProfile;

	QWidget * _creditWidget;

	QString _callForwardMode;

	Ui::CreditWidget * _ui;
};

#endif	//OWQTCREDITWIDGET_H
