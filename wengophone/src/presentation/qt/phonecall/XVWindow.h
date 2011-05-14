/*
 * XVWindow.h - XVWindow
 * High-level class offering X-Video hardware acceleration
 *
 * Matthias Schneider <ma30002000@yahoo.de>
 * 31-August-2006: fixed some memory leaks
 * 22-June-2006: varios cosmetic changes
 * 07-May-2006: initial version
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, v2, as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 2006 Matthias Schneider
 */

#ifndef OWXVWINDOW_H
#define OWXVWINDOW_H

#include <stdint.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>

/**
 * String: wrapper/helper.
 *
 * This class provides XVideo support under Linux if it is supported by the graphics hardware and driver.
 * XVideo makes use of hardware capabilities in order to do
 * - colorspace transformation
 * - scaling
 * - anti-aliasing
 *
 * This class features a fullscreen mode, an always-on-top mode and allows to enable and disable the window
 * manager decorations. A picture-in-picture functionality is provided by registering a second XVWindow class
 * window as a slave window. This class should work with most if not all window managers. It has to initialized
 * with the display and window where it shall appear and the original image and intial window size
 * After having been initialized successfully a frame is passed via putFrame which takes care of the presentation.
 *
 * @author Matthias Schneider
 */
class XVWindow {
public:

	XVWindow();
	~XVWindow();
	int init(Display * dp, Window rootWindow, int x, int y, int windowWidth, int windowHeight, int imageWidth, int imageHeight);
	void putFrame(uint8_t * frame, uint16_t width, uint16_t height);
	void toggleFullscreen();
	void toggleOntop();

	void resize(int x, int y, int windowWidth, int windowHeight);
	void getWindowSize(unsigned int* windowWidth, unsigned int* windowHeight);

	bool isFullScreen() const {
		return (_state.fullscreen);
	};

	Window getWindow() const {
		return (_XVWindow);
	};

	int getYUVWidth() const {
		return (_XVImage->width);
	};

	int getYUVHeight() const {
		return (_XVImage->height);
	};

	void registerMaster(XVWindow * master) {
		_master = master;
	};

	void registerSlave(XVWindow * slave) {
		_slave = slave;
	};

private:

	Display *_display;
	Window _rootWindow;
	Window _XVWindow;
	unsigned int _XVPort;
	GC _gc;
	XvImage * _XVImage;
	XShmSegmentInfo _XShmInfo;
	int _wmType;

	typedef struct {
		bool fullscreen;
		bool ontop;
		bool decoration;
		int oldx;
		int oldy;
		int oldWidth;
		int oldHeight;
		int curX;
		int curY;
		int curWidth;
		int curHeight;
		int origLayer;
	} State;

	State _state;
	XVWindow * _master;
	XVWindow * _slave;

	Atom XA_NET_SUPPORTED;
	Atom XA_WIN_PROTOCOLS;
	Atom XA_WIN_LAYER;
	Atom XA_NET_WM_STATE;
	Atom XA_NET_WM_STATE_FULLSCREEN;
	Atom XA_NET_WM_STATE_ABOVE;
	Atom XA_NET_WM_STATE_STAYS_ON_TOP;
	Atom XA_NET_WM_STATE_BELOW;
	Atom WM_DELETE_WINDOW;

	/**
	 * Sets the layer for the window.
	 */
	void setLayer(int layer);

	/**
	 * Fullscreen for ewmh WMs.
	 */
	void setEWMHFullscreen(int action);

	void setDecoration(bool d);
	void setSizeHints(int x, int y, int imageWidth, int imageHeight, int windowWidth, int windowHeight);

	/**
	 * Detects window manager type.
	 */
	int getWMType();

	int getGnomeLayer();

	/**
	 * Tests an atom.
	 */
	int getSupportedState(Atom atom);

	/**
	 * Returns the root window's.
	 */
	int getWindowProperty(Atom type, Atom ** args, unsigned long * nitems);

	bool _isInitialized;
};

#endif //OWXVWINDOW_H
