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

#ifndef PHAPICODECLIST_H
#define PHAPICODECLIST_H

#include <util/NonCopyable.h>

#include <string>

/**
 * Audio and video codec list for PhApi.
 *
 * @author Tanguy Krotoff
 */
class PhApiCodecList : NonCopyable {
public:

	/** Audio codecs. */
	static const std::string AUDIO_CODEC_PCMU;
	static const std::string AUDIO_CODEC_PCMA;
	static const std::string AUDIO_CODEC_G722;
	static const std::string AUDIO_CODEC_G726;
	static const std::string AUDIO_CODEC_ILBC;
	static const std::string AUDIO_CODEC_GSM;
	static const std::string AUDIO_CODEC_AMRNB;
	static const std::string AUDIO_CODEC_AMRWB;
	static const std::string AUDIO_CODEC_SPEEXWB;

	/** Video codecs. */
	static const std::string VIDEO_CODEC_H263;
	static const std::string VIDEO_CODEC_H264;
	static const std::string VIDEO_CODEC_MPEG4;
};

#endif	//PHAPICODECLIST_H
