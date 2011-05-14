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

#ifndef OWSTRINGLISTCONVERT_H
#define OWSTRINGLISTCONVERT_H

#include <qtutil/owqtutildll.h>

#include <util/NonCopyable.h>

class QStringList;
class StringList;

/**
 * Converts a StringList object to a QStringList.
 *
 * @author Tanguy Krotoff
 */
class StringListConvert : NonCopyable {
public:

	/**
	 * Converts a StringList object to a QStringList.
	 *
	 * @param strList StringList to convert
	 * @return StringList converted into a QStringList
	 */
	OWQTUTIL_API static QStringList toQStringList(const StringList & strList);
};

#endif	//OWSTRINGLISTCONVERT_H
