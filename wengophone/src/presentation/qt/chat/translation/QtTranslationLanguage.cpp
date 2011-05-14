/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* Translation 
* @author Chris Jimenez C 
* @date 2010.01.16
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtTranslationLanguage.h"

#include <util/Logger.h>

#include <QtGui/QtGui>

QtTranslationLanguage::QtTranslationLanguage() {

	_translationName = "";
	_translationAbbreviation = "";
	_supportSMS = true;
}

QtTranslationLanguage::QtTranslationLanguage(const QtTranslationLanguage & source) {
	_translationName = source._translationName;
	_translationAbbreviation = source._translationAbbreviation;
	_supportSMS = source._supportSMS;
}

QtTranslationLanguage::~QtTranslationLanguage() {
}


bool QtTranslationLanguage::isNull() const {
	return _translationName.isEmpty();
}


QtTranslationLanguage & QtTranslationLanguage::operator=(const QtTranslationLanguage & source) {

	_translationName = source._translationName;
	_translationAbbreviation = source._translationAbbreviation;
	_supportSMS = source._supportSMS;
	return *this;

}

bool QtTranslationLanguage::getSupportSMS() const{

	return _supportSMS;
}

