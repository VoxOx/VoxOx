/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#ifndef OWCWENGOPHONE_H
#define OWCWENGOPHONE_H

#include <control/Control.h>

#include <pixertool/pixertool.h>
#include <util/Event.h>

class ContactInfo;
class PWengoPhone;
class WengoPhone;
class Presentation;
class CUserProfileHandler;
class DtmfThemeManager;
class CDtmfThemeManager;

class SoftUpdatePrefs;	//VOXOX - JRT - 2009.12.10 

/**
 * @ingroup control
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 * @author Mathieu Stute
 */
class CWengoPhone : public Control {
public:

	CWengoPhone(WengoPhone & wengoPhone, bool runInBackground);

	~CWengoPhone();

	Presentation * getPresentation() const;

	CWengoPhone & getCWengoPhone() const;

	CDtmfThemeManager & getCDtmfThemeManager() const;

	void handleUserCanceledMandatory() const;

	/** FIXME should be removed. */
	WengoPhone & getWengoPhone() const;

	/**
	 * Entry point of the application, equivalent to main().
	 *
	 * Starts the object WengoPhone thus almost everything.
	 * Starts the model component thread (the thread WengoPhone).
	 *
	 * @see WengoPhone::init()
	 * @see WengoPhone::run()
	 */
	void start();

	/**
	 * @see WengoPhone::terminate()
	 */
	void terminate();

	/**
	 * Gets the CUserProfileHandler
	 */
	CUserProfileHandler & getCUserProfileHandler() const;

	/**
	 * @see WengoPhone::enableSerialization.
	 */
	void enableSerialization(bool enable);

	SoftUpdatePrefs&	getUserSoftUpdatePrefs();	//VOXOX - JRT - 2009.12.10 
	void				saveUserSoftUpdatePrefs();	//VOXOX - JRT - 2009.12.10 


private:

	void initPresentationThreadSafe();

	void initFinishedEventHandler(WengoPhone & sender);

	void showAddContactEventHandler(ContactInfo contactInfo);

	void showAddContactEventHandlerThreadSafe(ContactInfo contactInfo);

	void bringMainWindowToFrontEventHandler();

	void bringMainWindowToFrontEventHandlerThreadSafe();

	void exitEventHandler();

	void exitEventHandlerThreadSafe();

	void dtmfThemeManagerCreatedEventHandler(WengoPhone & sender, DtmfThemeManager & dtmfThemeManager);

	/** Direct link to the model. */
	WengoPhone & _wengoPhone;

	/** Direct link to the presentation via an interface. */
	PWengoPhone * _pWengoPhone;

	CUserProfileHandler * _cUserProfileHandler;

	/** DtmfThemeManager */
	CDtmfThemeManager * _cDtmfThemeManager;
};

#endif	//OWCWENGOPHONE_H
