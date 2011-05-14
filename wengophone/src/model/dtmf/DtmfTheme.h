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

#ifndef OWDTMFTHEME_H
#define OWDTMFTHEME_H

#include "Tone.h"

#include <util/StringList.h>

#include <string>
#include <map>

class WengoPhone;

/**
 * Dtmf theme.
 *
 * @ingroup model
 * @author Mathieu Stute
 */
class DtmfTheme {
	friend class DtmfThemeManager;
	typedef std::map<std::string, const Tone *> ToneList;
public:

	enum DialpadMode {
		plain,
		iconified,
		svg,
		unknown
	};

	~DtmfTheme();

	std::string getName() const;

	StringList getToneList() const;

	const Tone * getTone(const std::string & key) const;

	void playTone(const std::string & key) const;

	std::string getImageFile() const;

	std::string getRepertory() const;

	DtmfTheme::DialpadMode getDialpadMode() const;

private:

	DtmfTheme(WengoPhone & wengoPhone, const std::string & repertory, const std::string & xmlDescriptor);

	std::string _repertory;

	std::string _xmlDescriptor;

	std::string _name;

	std::string _imageFile;

	DtmfTheme::DialpadMode _dialpadMode;

	ToneList _toneList;

	WengoPhone & _wengoPhone;
};

#endif	//OWDTMFTHEME_H
