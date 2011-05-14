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

#ifndef OWQTPHONELINE_H
#define OWQTPHONELINE_H

#include <presentation/PPhoneLine.h>

#include <util/Trackable.h>

#include <qtutil/QObjectThreadSafe.h>

class CPhoneLine;
class CWengoPhone;
class CPhoneCall;
class QtPhoneCall;
class QtWengoPhone;

class QtPhoneLine : public QObjectThreadSafe, public PPhoneLine, public Trackable {
	Q_OBJECT
public:

	QtPhoneLine(CPhoneLine & cPhoneLine);

	virtual ~QtPhoneLine();

	virtual void updatePresentation() {}

private Q_SLOTS:

	void callButtonClicked();

	/** @see QtMacApplication::openURLRequestEvent */
	void openURLRequest(QString url);

private:

	virtual void initThreadSafe();

	/**
	 * Handles state of the associated PhoneLine and dispatch it to
	 * the systray and the status bar.
	 * @param state the new state of the PhoneLine.
	 */
	void stateChangedEventHandler(EnumPhoneLineState::PhoneLineState state);

	/**
	 * @see QtPhoneLine::stateChangedEventHandler
	 */
	void stateChangedEventHandlerThreadSafe(EnumPhoneLineState::PhoneLineState state);

	void phoneCallCreatedEventHandler(CPhoneCall & cPhoneCall);

	void phoneCallCreatedEventHandlerThreadSafe(CPhoneCall & cPhoneCall);

	void phoneCallClosedEventHandler(CPhoneCall & cPhoneCall);

	void phoneCallClosedEventHandlerThreadSafe(CPhoneCall & cPhoneCall);

	CPhoneLine & _cPhoneLine;

	QtWengoPhone * _qtWengoPhone;

	CPhoneCall * _activeCPhoneCall;
};

#endif	//OWQTPHONELINE_H
