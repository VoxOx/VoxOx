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

#ifndef ENUMTONE_H
#define ENUMTONE_H

#include <util/NonCopyable.h>

/**
 * DTMF tones.
 *
 * @author Tanguy Krotoff
 */
class EnumTone : NonCopyable {
public:

	enum Tone {
		/** DMTF 0. */
		Tone0,

		/** DMTF 1. */
		Tone1,

		/** DMTF 2. */
		Tone2,

		/** DMTF 3. */
		Tone3,

		/** DMTF 4. */
		Tone4,

		/** DMTF 5. */
		Tone5,

		/** DMTF 6. */
		Tone6,

		/** DMTF 7. */
		Tone7,

		/** DMTF 8. */
		Tone8,

		/** DMTF 9. */
		Tone9,

		/** DMTF *. */
		ToneStar,

		/** DMTF #. */
		TonePound,

		/** Dialtone. */
		ToneDialtone,

		/** Call-busy tone. */
		ToneBusy,

		/** Remote party is ringing feedback tone. */
		ToneRingback,

		/** Default ring/alert tone. */
		ToneRingtone,

		/** Fasy busy/call failed tone. */
		ToneCallFailed,

		/** Silence. */
		ToneSilence,

		/** Backspace tone. */
		ToneBackspace,

		/** Call waiting alert tone. */
		ToneCallWaiting,

		/** Call held feedback tone. */
		ToneCallHeld,

		/** Off hook/fast busy tone. */
		ToneLoudFastBusy
	};
};

#endif	//ENUMTONE_H
