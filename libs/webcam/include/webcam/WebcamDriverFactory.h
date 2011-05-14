/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef WEBCAMDRIVERFACTORY_H
#define WEBCAMDRIVERFACTORY_H

class IWebcamDriver;
class WebcamDriver;

/**
 * Factory for webcam drivers.
 *
 * @author Philippe Bernery
 */
class WebcamDriverFactory {
public:

	WebcamDriverFactory() {}
	virtual ~WebcamDriverFactory() {}

	/**
	 * Creates a webcam driver instance.
	 *
	 * If a webcam driver factory has not been set, the default one
	 * is returned.
	 *
	 * @param driver common driver
	 * @param flags set the webcam driver
	 * @return new webcam object
	 */
	virtual IWebcamDriver * create(WebcamDriver * driver, int flags) const = 0;
};

#endif	//WEBCAMDRIVERFACTORY_H
