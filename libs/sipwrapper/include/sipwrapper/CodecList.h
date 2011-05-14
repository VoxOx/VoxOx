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

#ifndef CODECLIST_H
#define CODECLIST_H

#include <util/NonCopyable.h>

#include <string>

/**
 * Audio and video codec list for the SIP stack.
 *
 * FIXME NOT FINISHED YET
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class CodecList : NonCopyable {
public:

	enum AudioCodec {
		AudioCodecError,

		/** G711 audio codec. */
		AudioCodecPCMU,

		/** G711 audio codec. */
		AudioCodecPCMA,

		/** G722 audio codec. */
		AudioCodecG722,

        /** G726 audio codec. */
		AudioCodecG726,
        
		/**
		 * ILBC audio codec (8 kHz).
		 *
		 * @see http://www.ilbcfreeware.org/
		 */
		AudioCodecILBC,

		/** GSM audio codec (8 kHz). */
		AudioCodecGSM,

		/**
		 * AMR narrowband audio codec (8 kHz).
		 */
		AudioCodecAMRNB,

		/**
		 * AMR wideband audio codec (16 kHz).
		 */
		AudioCodecAMRWB,

		/**
		 * Speex narrowband audio codec (8 kHz).
		 *
		 * @see http://www.speex.org/
		 */
		AudioCodecSPEEXNB,

		/** Speex wideband audio codec (16 kHz). */
		AudioCodecSPEEXWB
	};

	enum VideoCodec {
		VideoCodecError,

		/** H263 video codec. */
		VideoCodecH263,

		/** H264 video codec. */
		VideoCodecH264,

		/** MPEG4 video codec. */
		VideoCodecMPEG4
	};

	/**
	 * Converts an AudioCodec into a string.
	 *
	 * @return the string
	 */
	static std::string toString(AudioCodec audioCodec);

	/**
	 * Converts a VideoCodec into a string.
	 *
	 * @return the string
	 */
	static std::string toString(VideoCodec videoCodec);
};

#endif	//CODECLIST_H
