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

#include <sipwrapper/EnumVideoQuality.h>

#include <util/String.h>
#include <util/Logger.h>

#include <map>

typedef std::map<EnumVideoQuality::VideoQuality, std::string> VideoQualityMap;
static VideoQualityMap _videoQualityMap;

static void init() {
	_videoQualityMap[EnumVideoQuality::VideoQualityNormal] = "VideoQualityNormal";
	_videoQualityMap[EnumVideoQuality::VideoQualityGood] = "VideoQualityGood";
	_videoQualityMap[EnumVideoQuality::VideoQualityVeryGood] = "VideoQualityVeryGood";
	_videoQualityMap[EnumVideoQuality::VideoQualityExcellent] = "VideoQualityExcellent";
}

std::string EnumVideoQuality::toString(VideoQuality videoQuality) {
	init();
	std::string tmp = _videoQualityMap[videoQuality];
	if (tmp.empty()) {
		LOG_FATAL("unknown VideoQuality=" + String::fromNumber(videoQuality));
	}
	return tmp;
}

EnumVideoQuality::VideoQuality EnumVideoQuality::toVideoQuality(const std::string & videoQuality) {
	init();
	for (VideoQualityMap::const_iterator it = _videoQualityMap.begin();
		it != _videoQualityMap.end(); ++it) {

		if ((*it).second == videoQuality) {
			return (*it).first;
		}
	}

	LOG_FATAL("unknown VideoQuality=" + videoQuality);
	return VideoQualityNormal;
}
