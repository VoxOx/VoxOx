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

#ifndef OWQTEVENTWIDGET_H
#define OWQTEVENTWIDGET_H

#include <qtutil/QObjectThreadSafe.h>

class CUserProfile;
class CWengoPhone;

class QWidget;
namespace Ui { class EventWidget; }

/**
 * Event widget of the profile bar.
 *
 * Shows missed calls and voice mail messages.
 *
 * @author Mathieu Stute
 */
class QtEventWidget : public QObjectThreadSafe {
	Q_OBJECT
public:

	QtEventWidget(CWengoPhone & cWengoPhone, CUserProfile & cUserProfile, QWidget * parent);

	~QtEventWidget();

	void setVoiceMail(int count);

	void setMissedCall(int count);

	void updatePresentation();

	QWidget * getWidget() const;

private Q_SLOTS:

	void voiceMailClicked();

	void missedCallClicked();

	void languageChanged();

private:

	void initThreadSafe();

	void updatePresentationThreadSafe();

	CUserProfile & _cUserProfile;

	CWengoPhone & _cWengoPhone;

	QWidget * _eventWidget;

	Ui::EventWidget * _ui;

	int _voiceMailCount;

	int _missedCallCount;
};

#endif	//OWQTEVENTWIDGET_H
