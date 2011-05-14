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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "LanguageList.h"

#include "Languages.h"

#include <util/Logger.h>

std::string LanguageList::getLanguageName(const std::string & iso639Code) {
	std::string languageName;
	for (int i = 0; i < LANGUAGES_SIZE; i++) {
		Language lang = LANGUAGES[i];
		if (lang.iso639Code == iso639Code) {
			languageName = lang.name;
			break;
		}
	}
	return languageName;
}

std::string LanguageList::getEnglishLanguageName(const std::string & iso639Code) {
	std::string englishLanguageName;
	for (int i = 0; i < LANGUAGES_SIZE; i++) {
		Language lang = LANGUAGES[i];
		if (lang.iso639Code == iso639Code) {
			englishLanguageName = lang.englishName;
			break;
		}
	}
	return englishLanguageName;
}

std::string LanguageList::getISO639Code(const std::string & languageName) {
	std::string iso639Code;
	for (int i = 0; i < LANGUAGES_SIZE; i++) {
		Language lang = LANGUAGES[i];
		if (lang.name == languageName) {
			iso639Code = lang.iso639Code;
			break;
		}
	}
	return iso639Code;
}
