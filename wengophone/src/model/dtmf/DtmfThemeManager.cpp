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
#include "DtmfThemeManager.h"

#include <model/WengoPhone.h>

#include <util/File.h>
#include <util/Logger.h>

using namespace std;

DtmfThemeManager::DtmfThemeManager(WengoPhone & wengoPhone, const string & dtmfDirPath) 
	: _dtmfDirPath(dtmfDirPath),
	_wengoPhone(wengoPhone) {

	constructDTMF();
}

DtmfThemeManager::~DtmfThemeManager() {
	deleteDTMF();
}

void DtmfThemeManager::constructDTMF() {
	File dir(_dtmfDirPath);
	StringList dirList = dir.getDirectoryList();

	//iterate over dtmf dir
	for (unsigned int i = 0; i != dirList.size(); i++) {

		File themeDir(
			_dtmfDirPath +
			File::getPathSeparator() +
			dirList[i] +
			File::getPathSeparator()
		);
		StringList fileList = themeDir.getFileList();

		//iterate over files
		for (unsigned int j = 0; j != fileList.size(); j++) {

			//find the .xml file
			string filename = fileList[j];
			string ext = filename.substr(filename.size() - 4, fileList[j].size() - 1);
			if (ext == string(".xml")) {

				string themeRepertory = 
					_dtmfDirPath + dirList[i] + File::getPathSeparator();

				//build DtmfTheme
				DtmfTheme * theme = new DtmfTheme(
					_wengoPhone, themeRepertory, fileList[j]
				);

#if defined(OS_MACOSX)
				/*
				 * some audio smileys are deactivated on MacOS X because Raw files cannot
				 * currently be played on this platform.
				 */

				bool isRaw = false;
				if (theme->getTone("0")->getAudioFormat() == Tone::Raw) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("1")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("2")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("3")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("4")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("5")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("6")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("7")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("8")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("9")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("#")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}
				if (!isRaw && (theme->getTone("*")->getAudioFormat() == Tone::Raw)) {
					isRaw = true;
				}

				if (isRaw) {
					LOG_DEBUG("\n\nfind a xml file: " + fileList[j] + "\n -> CONTAIN RAW !! \n");
					OWSAFE_DELETE(theme);
					continue;
				}
#endif

				//add it in the list
				_dtmfThemeList[theme->getName()] = theme;
				LOG_DEBUG("find a theme : " + fileList[j]);
			}
		}
	}
}

void DtmfThemeManager::deleteDTMF() {

	//delete allocated DtmfTheme
	DtmfThemeManager::DtmfThemeList::const_iterator it;
	const DtmfTheme * ref = NULL;
	for (it = _dtmfThemeList.begin(); it != _dtmfThemeList.end(); it++) {
		ref = (*it).second;
		if (ref) {
			OWSAFE_DELETE(ref);
		}
	}
	OWSAFE_DELETE(ref);
	_dtmfThemeList.clear();
}

bool DtmfThemeManager::refreshDtmfThemes() {

	//delete old themes
	deleteDTMF();

	//construct new themes
	constructDTMF();

	return true;
}

StringList DtmfThemeManager::getThemeList() const {

	StringList toReturn;

	DtmfThemeManager::DtmfThemeList::const_iterator it;
	for (it = _dtmfThemeList.begin(); it != _dtmfThemeList.end(); it++) {
		toReturn.push_back((*it).first);
	}

	return toReturn;
}

const DtmfTheme * DtmfThemeManager::getDtmfTheme(const string & themeName) const {

	DtmfThemeManager::DtmfThemeList::const_iterator it;
	for (it = _dtmfThemeList.begin(); it != _dtmfThemeList.end(); it++) {
		if (((*it).first) == themeName) {
			return (*it).second;
		}
	}

	return NULL;
}
