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

#ifndef LANGUAGELIST_H
#define LANGUAGELIST_H

#include <util/NonCopyable.h>
#include <util/List.h>
#include <util/StringList.h>

/**
 * Parses Languages.h
 *
 * @see Languages.h
 * @author Tanguy Krotoff
 */
class LanguageList : NonCopyable {
public:

	/**
	 * Gets the language name given its ISO 639 Alpha-2 code.
	 *
	 * Warning!
	 * returns a UTF-8 encoded string.
	 * For Qt4, check QString::fromUtf8()
	 * <pre>
	 * std::string tmp;
	 * QString tmpUtf8 = QString::fromUtf8(tmp.c_str());
	 * </pre>
	 *
	 * @param iso639Code ISO 639 Alpha-2 code (e.g en, fr ect...)
	 * @return language name (e.g français, english...) or empty string
	 */
	static std::string getLanguageName(const std::string & iso639Code);

	/**
	 * Gets the ISO 639 Alpha-2 code from the language name.
	 *
	 * Warning!
	 * Takes an UTF-8 encoded string.
	 * For Qt4, check QString::toUtf8()
	 * <pre>
	 * Qstring tmp;
	 * std::string tmpUtf8 = tmp.toUtf8().constData();
	 * </pre>
	 *
	 * @param languageName language name (e.g français, english...)
	 * @return ISO 639 Alpha-2 code (e.g en, fr ect...) or empty string
	 */
	static std::string getISO639Code(const std::string & languageName);

	/**
	 * Gets the english language name given its ISO 639 Alpha-2 code.
	 *
	 * @param iso639Code ISO 639 Alpha-2 code (e.g en, fr ect...)
	 * @return english language name (e.g french, english...) or empty string
	 */
	static std::string getEnglishLanguageName(const std::string & iso639Code);

private:
};

#endif	//LANGUAGELIST_H
