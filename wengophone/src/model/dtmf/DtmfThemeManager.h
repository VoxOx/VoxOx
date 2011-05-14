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

#ifndef OWDTMFTHEMEMANAGER_H
#define OWDTMFTHEMEMANAGER_H

#include "DtmfTheme.h"

#include <util/StringList.h>

#include <string>
#include <map>

class WengoPhone;

/**
 * Dtmf theme.
 *
 * @ingroup model
 * @author Mathieu Stute
 */
class DtmfThemeManager {
public:

	DtmfThemeManager(WengoPhone & wengoPhone, const std::string & dtmfDirPath);

	~DtmfThemeManager();

	StringList getThemeList() const;

	const DtmfTheme * getDtmfTheme(const std::string & themeName) const;

	bool refreshDtmfThemes();

private:

	void constructDTMF();

	void deleteDTMF();

	typedef std::map<std::string, const DtmfTheme *> DtmfThemeList;

	DtmfThemeList _dtmfThemeList;

	std::string _dtmfDirPath;

	WengoPhone & _wengoPhone;
};

#endif	//OWDTMFTHEMEMANAGER_H
