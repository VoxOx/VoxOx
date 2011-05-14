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

#ifndef OWTONE_H
#define OWTONE_H

#include <string>

/**
 * Dtmf theme.
 *
 * @ingroup model
 * @author Mathieu Stute
 */
class Tone {
	friend class DtmfTheme;
public:

	enum Action {
		Play,
		None,
	};

	enum AudioFormat {
		Dtmf,
		Raw,
		Wav,
		Unknown,
	};

	~Tone();

	std::string getKey() const;

	std::string getSoundFile() const;

	std::string getText() const;

	std::string getImageFile() const;

	Tone::Action getLocalAction() const;

	Tone::Action getRemoteAction() const;

	Tone::AudioFormat getAudioFormat() const;

private:

	Tone(const std::string & key, 
		const std::string & soundFile,
		const std::string & text = "",
		const std::string & imageFile = "",
		Tone::Action localAction = Tone::Play,
		Tone::Action remoteAction = Tone::Play,
		Tone::AudioFormat audioFormat = Tone::Raw
	);

	/* action for local */
	Tone::Action _localAction;

	/* action for remote */
	Tone::Action _remoteAction;

	/* the format of the sound file associated with this Tone */
	Tone::AudioFormat _audioFormat;

	/* the path to the image file associated with this Tone */
	std::string _imageFile;

	/* the path to the sound file associated with this Tone */
	std::string _soundFile;

	/* the text string associated with this Tone. */
	std::string _text;

	/* the code of the key associated with this Tone. */
	std::string _key;
};

#endif	//OWTONE_H
