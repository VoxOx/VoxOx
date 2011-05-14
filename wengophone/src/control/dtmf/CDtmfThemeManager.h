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

#ifndef OWCDTMFTHEMEMANAGER_H
#define OWCDTMFTHEMEMANAGER_H

#include <control/Control.h>

#include <util/StringList.h>

class Presentation;
class CWengoPhone;
class DtmfThemeManager;
class DtmfTheme;

/**
 * Control for DtmfThemeManager.
 *
 * @ingroup control
 * @author Xavier Desjardins
 */
class CDtmfThemeManager : public Control {
public:

	/**
	 * Default constructor.
	 */
	CDtmfThemeManager(DtmfThemeManager & dtmfThemeManager, CWengoPhone & cWengoPhone);

	~CDtmfThemeManager();

	Presentation * getPresentation() const;

	CWengoPhone & getCWengoPhone() const;

	//play a tone
	void playTone(const std::string & themeName, const std::string & key) const;

	//get theme list
	StringList getThemeList() const;

	//get a theme
	const DtmfTheme * getDtmfTheme(const std::string & themeName) const;

	//refresh DTMFThemes
	bool refreshDtmfThemes();

private:

	void initPresentationThreadSafe();

	/** Link to the model. */
	DtmfThemeManager & _dtmfThemeManager;

	/** Link to the CWengoPhone. */
	CWengoPhone & _cWengoPhone;

	/** Link to the presentation via an interface. */
};

#endif	//OWCDTMFTHEMEMANAGER_H
