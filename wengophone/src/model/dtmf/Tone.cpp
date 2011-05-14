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
#include "Tone.h"

#include <util/Logger.h>

Tone::Tone(const std::string & key, const std::string & soundFile,
	const std::string & text, const std::string & imageFile,
	Tone::Action localAction, Tone::Action remoteAction, 
	Tone::AudioFormat audioFormat) :
	_localAction(localAction),
	_remoteAction(remoteAction),
	_audioFormat(audioFormat),
	_imageFile(imageFile),
	_soundFile(soundFile),
	_text(text),
	_key(key) {
}

Tone::~Tone() {
}

std::string Tone::getImageFile() const {
	return _imageFile;
}

std::string Tone::getSoundFile() const {
	return _soundFile;
}

std::string Tone::getText() const {
	return _text;
}

std::string Tone::getKey() const {
	return _key;
}

Tone::Action Tone::getLocalAction() const {
	return _localAction;
}

Tone::Action Tone::getRemoteAction() const {
	return _remoteAction;
}

Tone::AudioFormat Tone::getAudioFormat() const {
	return _audioFormat;
}
