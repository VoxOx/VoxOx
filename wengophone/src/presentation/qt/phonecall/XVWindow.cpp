/*
 * XVWindow.cpp - XVWindow
 * High-level class offering X-Video hardware acceleration
 *
 * Matthias Schneider <ma30002000@yahoo.de>
 * 22-June-2006: minor bugfix on aspect ratio calculating, varios cosmetic changes
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

#include "XVWindow.h"

#include <util/Logger.h>

#define GUID_I420_PLANAR 0x30323449

#define wm_LAYER		1
#define wm_FULLSCREEN		2
#define wm_STAYS_ON_TOP		4
#define wm_ABOVE		8
#define wm_BELOW		16
#define wm_NETWM 		(wm_FULLSCREEN | wm_STAYS_ON_TOP | wm_ABOVE | wm_BELOW)

#define WIN_LAYER_ONBOTTOM	2
#define WIN_LAYER_NORMAL	4
#define WIN_LAYER_ONTOP		6
#define WIN_LAYER_ABOVE_DOCK	10

#define _NET_WM_STATE_REMOVE	0	/* remove/unset property */
#define _NET_WM_STATE_ADD	1	/* add/set property */
#define _NET_WM_STATE_TOGGLE	2	/* toggle property */

#define MWM_HINTS_FUNCTIONS	(1L << 0)
#define MWM_HINTS_DECORATIONS	(1L << 1)
#define MWM_FUNC_RESIZE		(1L << 1)
#define MWM_FUNC_MOVE		(1L << 2)
#define MWM_FUNC_MINIMIZE	(1L << 3)
#define MWM_FUNC_MAXIMIZE	(1L << 4)
#define MWM_FUNC_CLOSE		(1L << 5)
#define MWM_DECOR_ALL		(1L << 0)
#define MWM_DECOR_MENU		(1L << 4)

#define DEFAULT_SLAVE_RATIO	5
#define DEFAULT_X		1
#define DEFAULT_Y		1

typedef struct {
	int flags;
	long functions;
	long decorations;
	long input_mode;
	long state;
} MotifWmHints;

extern XvImage * XvShmCreateImage(Display *, XvPortID, int, char *, int, int, XShmSegmentInfo *);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>

XVWindow::XVWindow() {
	// initialize class variables
	_master = NULL;
	_slave = NULL;
	_XVPort = 0;
	_state.fullscreen = false;
	_state.ontop = false;
	_state.decoration = true;
	_display = NULL;
	_XVWindow = 0;
	_XShmInfo.shmaddr = NULL;
	_gc = NULL;
	_isInitialized = false;
	_XVImage = NULL;
}

int XVWindow::init(Display* dp, Window rootWindow, int x, int y, int windowWidth, int windowHeight, int imageWidth, int imageHeight) {

	// local variables needed for creation of window and initialization of XV extension
	unsigned int i;
	unsigned int ver, rel, req, ev, err, adapt;
	XSetWindowAttributes xswattributes;
	XWindowAttributes xwattributes;
	XVisualInfo xvinfo;
	XvAdaptorInfo *xvainfo;
	unsigned int candidateXVPort=0;
	unsigned int busyPorts=0;

	_display=dp;
	_rootWindow=rootWindow;
	_state.origLayer=0;

	// initialize atoms
	WM_DELETE_WINDOW = XInternAtom(_display, "WM_DELETE_WINDOW", False);
	XA_WIN_PROTOCOLS = XInternAtom(_display, "_WIN_PROTOCOLS", False);
	XA_NET_SUPPORTED = XInternAtom(_display, "_NET_SUPPORTED", False);
	XA_NET_WM_STATE = XInternAtom(_display, "_NET_WM_STATE", False);
	XA_NET_WM_STATE_FULLSCREEN = XInternAtom(_display, "_NET_WM_STATE_FULLSCREEN", False);
	XA_NET_WM_STATE_ABOVE = XInternAtom(_display, "_NET_WM_STATE_ABOVE", False);
	XA_NET_WM_STATE_STAYS_ON_TOP = XInternAtom(_display, "_NET_WM_STATE_STAYS_ON_TOP", False);
	XA_NET_WM_STATE_BELOW = XInternAtom(_display, "_NET_WM_STATE_BELOW", False);

	XGetWindowAttributes(_display, _rootWindow, &xwattributes);
	XMatchVisualInfo(_display, DefaultScreen(_display), xwattributes.depth, TrueColor, &xvinfo);

	// define window properties and create the window
	xswattributes.colormap = XCreateColormap(_display, _rootWindow, xvinfo.visual, AllocNone);
	xswattributes.event_mask = StructureNotifyMask | ExposureMask;
	xswattributes.background_pixel = 0;
	xswattributes.border_pixel = 0;
	_XVWindow = XCreateWindow(_display, _rootWindow, x, y, windowWidth, windowHeight, 0, xvinfo.depth,
	 	InputOutput, xvinfo.visual, CWBackPixel|CWBorderPixel|CWColormap|CWEventMask, &xswattributes);

	// define inputs events
	XSelectInput(_display, _XVWindow, StructureNotifyMask | KeyPressMask | ButtonPressMask);

	setSizeHints(DEFAULT_X,DEFAULT_Y, imageWidth, imageHeight, windowWidth, windowHeight);

	// map the window
	XMapWindow(_display, _XVWindow);

	XSetWMProtocols(_display, _XVWindow, &WM_DELETE_WINDOW,1);

	// check if SHM XV window is possible
	if (Success != XvQueryExtension(_display, &ver, &rel, &req, &ev, &err)) {
		LOG_DEBUG("[x11] XQueryExtension failed");
		return 0;
	}
	if (!XShmQueryExtension(_display)) {
		LOG_DEBUG("[x11] XQueryShmExtension failed");
		return 0;
	}
	if (Success != XvQueryAdaptors(_display, _rootWindow, &adapt, &xvainfo)) {
		LOG_DEBUG("[x11] XQueryAdaptor failed"); XFree(xvainfo);
		return 0;
	}

	// look for a usable XV port
	for (i = 0; i < adapt && _XVPort == 0; i++) {
		if ((xvainfo[i].type & XvInputMask) && (xvainfo[i].type & XvImageMask)) {
			for (candidateXVPort = xvainfo[i].base_id; candidateXVPort < xvainfo[i].base_id + xvainfo[i].num_ports; ++candidateXVPort)
				if (!XvGrabPort(_display, candidateXVPort, CurrentTime)) {
					_XVPort = candidateXVPort;
					break;
				} else {
					LOG_DEBUG("[x11] Could not grab port: " + String::fromNumber(candidateXVPort));
					++busyPorts;
				}
		}
	}
	XvFreeAdaptorInfo(xvainfo);

	if (!_XVPort) {
		if (busyPorts) {
			LOG_WARN("[x11] Could not find free Xvideo port - maybe another process is already using it.");
		} else {
			LOG_WARN("[x11] It seems there is no Xvideo support for your video card available.");
		}
		return 0;
	} else {
		LOG_WARN("[x11] Use XVideo port: " + String::fromNumber(_XVPort));
	}

	_gc = XCreateGC(_display, _XVWindow, 0, 0);

	// create the shared memory portion
	_XVImage = XvShmCreateImage(_display, _XVPort, GUID_I420_PLANAR, 0, imageWidth, imageHeight, &_XShmInfo);

	_XShmInfo.shmid = shmget(IPC_PRIVATE, _XVImage->data_size, IPC_CREAT | 0777);
	_XShmInfo.shmaddr = (char *) shmat(_XShmInfo.shmid, 0, 0);
	_XVImage->data = _XShmInfo.shmaddr;
	_XShmInfo.readOnly = False;
	if (!XShmAttach(_display, &_XShmInfo)) {
		LOG_WARN("[x11] XShmAttach failed");
		return 0;
	} else {
		_isInitialized = true;
	}

	// detect the window manager type
	_wmType=getWMType();

	return 1;
}

XVWindow::~XVWindow() {
	if (_isInitialized && _XShmInfo.shmaddr) {
		XShmDetach(_display, &_XShmInfo);
		shmdt(_XShmInfo.shmaddr);
	}
	if (_XVImage) {
		XFree(_XVImage);
	}
	if (_gc) {
		XFree(_gc);
	}
	if (_XVWindow) {
		XUnmapWindow(_display, _XVWindow);
		XDestroyWindow(_display, _XVWindow);
	}
}

void XVWindow::putFrame(uint8_t* frame, uint16_t width, uint16_t height) {
	XEvent event;

	if ((width!= _XVImage->width) || (height!=_XVImage->height)) {
	    printf ("[x11] dynamic switching of resolution not supported\n");
	    return;
	}

	// event handling
	while (XPending(_display)) {
		XNextEvent(_display, &event);
		if (event.type == ClientMessage) {
//			if "closeWindow" is clicked do nothing right now (window is closed from the Qt Gui
//			if (event.xclient.format == 32 && event.xclient.data.l[0] == (signed) WM_DELETE_WINDOW) exit(0);
		}
		// the window size has changed
		if (event.type == ConfigureNotify) {
			XConfigureEvent* xce = (XConfigureEvent*) &event;
			// if a slave window exists it has to be resized as well
			if (_slave) { _slave->resize(
				xce->width - (int)(xce->width / DEFAULT_SLAVE_RATIO),
				xce->height - (int)(_slave->getYUVHeight() * xce->width / DEFAULT_SLAVE_RATIO / _slave->getYUVWidth()),
				(int)(xce->width / DEFAULT_SLAVE_RATIO),
				(int)(_slave->getYUVHeight() * xce->width / DEFAULT_SLAVE_RATIO / _slave->getYUVWidth()));
			}

			// if we are a slave window the master window takes care of aspect ration
			if (_master) {
				_state.curX=0;
				_state.curY=0;
				_state.curWidth=xce->width;
				_state.curHeight=xce->height;
			// if we are a master window we have to recalculate the window size (important for non 4:3 screens)
			} else {
				if ((xce->width*_XVImage->height/_XVImage->width)>xce->height) {
					_state.curX=(int)((xce->width-(xce->height*_XVImage->width/_XVImage->height))/2);
					_state.curY=0;
					_state.curWidth=(int)(xce->height*_XVImage->width/_XVImage->height);
					_state.curHeight=xce->height;
				} else if ((xce->height*_XVImage->width/_XVImage->height)>xce->width) {
					_state.curX=0;
					_state.curY=(int)((xce->height-(xce->width*_XVImage->height/_XVImage->width))/2);
					_state.curWidth=xce->width;
					_state.curHeight=(int)(xce->width*_XVImage->height/_XVImage->width);
				} else {
					_state.curX=0;
					_state.curY=0;
					_state.curWidth=xce->width;
					_state.curHeight=xce->height;
				}
			}
		}

		// a key is pressed
		if (event.type == KeyPress) {
			XKeyEvent* xke = (XKeyEvent*) &event;
			switch (xke->keycode) {
				case 41:	toggleFullscreen(); // "f"
				break;
				case 40:	setDecoration(!_state.decoration); // "d"
				break;
				case 32:	toggleOntop(); // "o"
				break;
				case 9:		if (_state.fullscreen) { toggleFullscreen(); } // esc
				break;
			}
		}

		// a mouse button is clicked
		if (event.type == ButtonPress) {
			if (_master) {
				_master->toggleFullscreen();
			} else {
				toggleFullscreen();
			}
		}
	}

	// copy image to shared memory
	memcpy(_XVImage->data, frame, (int) (_XVImage->width * _XVImage->height * 3 /2));

	XvShmPutImage(_display, _XVPort, _XVWindow, _gc, _XVImage, 0, 0, _XVImage->width, _XVImage->height,
		_state.curX, _state.curY, _state.curWidth, _state.curHeight, True);

	XFlush(_display);
}

void XVWindow::toggleFullscreen() {
	int newX, newY, newWidth, newHeight;
	Window childWindow;
	XWindowAttributes xwattributes;

	if (_state.fullscreen) {
		// not needed with EWMH fs
		if ( ! (_wmType & wm_FULLSCREEN) ) {
			newX = _state.oldx;
			newY = _state.oldy;
			newWidth = _state.oldWidth;
			newHeight = _state.oldHeight;
			setDecoration(true);
		}

		// removes fullscreen state if wm supports EWMH
		setEWMHFullscreen(_NET_WM_STATE_REMOVE);
	} else {
		// sets fullscreen state if wm supports EWMH
		setEWMHFullscreen(_NET_WM_STATE_ADD);

		// not needed with EWMH fs - save window coordinates/size and discover fullscreen window size
		if ( ! (_wmType & wm_FULLSCREEN) ) {
			newX = 0;
			newY = 0;
			newWidth = DisplayWidth(_display, DefaultScreen(_display));
			newHeight = DisplayHeight(_display, DefaultScreen(_display));

			setDecoration(false);
			XFlush(_display);
			XTranslateCoordinates(_display, _XVWindow, RootWindow(_display, DefaultScreen(_display)),
						0,0,&_state.oldx,&_state.oldy, &childWindow);
			XGetWindowAttributes(_display, _XVWindow, &xwattributes);
			_state.oldWidth = xwattributes.width;
			_state.oldHeight = xwattributes.height;
		}
	}
	 // not needed with EWMH fs - create a screen-filling window on top and turn of decorations
	if (!(_wmType & wm_FULLSCREEN) ) {
		setSizeHints(newX, newY, _XVImage->width, _XVImage->height, newWidth, newHeight);
		setLayer((!_state.fullscreen) ? 0 : 1);
		XMoveResizeWindow(_display, _XVWindow, newX, newY, newWidth, newHeight);
	}

	/* some WMs lose ontop after fullscreeen */
	if ((_state.fullscreen) & _state.ontop) {
		setLayer(1);
	}

	XMapRaised(_display, _XVWindow);
	XRaiseWindow(_display, _XVWindow);
	XFlush(_display);
	_state.fullscreen=!_state.fullscreen;
}

void XVWindow::setEWMHFullscreen(int action) {
	if (_wmType & wm_FULLSCREEN) {

		// create an event event to toggle fullscreen mode
		XEvent xev;
		xev.xclient.type = ClientMessage;
		xev.xclient.serial = 0;
		xev.xclient.send_event = True;
		xev.xclient.message_type = XInternAtom(_display, "_NET_WM_STATE", False);
		xev.xclient.window = _XVWindow;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = action;
		xev.xclient.data.l[1] = XInternAtom(_display, "_NET_WM_STATE_FULLSCREEN", False);
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 0;
		xev.xclient.data.l[4] = 0;

		// send the event to the window
		if (!XSendEvent(_display, _rootWindow, False, SubstructureRedirectMask | SubstructureNotifyMask, &xev)) {
			LOG_DEBUG("[x11] setEWMHFullscreen failed");
		}
	}
}

void XVWindow::toggleOntop() {
	setLayer((_state.ontop) ? 0 : 1);
	_state.ontop=!_state.ontop;
}

void XVWindow::setLayer(int layer) {
	Window mRootWin = RootWindow(_display, DefaultScreen(_display));
	XClientMessageEvent xev;
	memset(&xev, 0, sizeof(xev));
	char *state;

	if (_wmType & wm_LAYER) {

		if (!_state.origLayer) {
			_state.origLayer = getGnomeLayer();
		}
		xev.type = ClientMessage;
		xev.display = _display;
		xev.window = _XVWindow;
		xev.message_type = XA_WIN_LAYER;
		xev.format = 32;
		xev.data.l[0] = layer ? WIN_LAYER_ABOVE_DOCK : _state.origLayer;
		xev.data.l[1] = CurrentTime;
		LOG_DEBUG("[x11] Layered style stay on top (layer " + String::fromNumber(xev.data.l[0]) + ").");
		XSendEvent(_display, mRootWin, False, SubstructureNotifyMask, (XEvent *) & xev);

	} else if (_wmType & wm_NETWM) {

		xev.type = ClientMessage;
		xev.message_type = XA_NET_WM_STATE;
		xev.display = _display;
		xev.window = _XVWindow;
		xev.format = 32;
		xev.data.l[0] = layer;

		if (_wmType & wm_STAYS_ON_TOP) xev.data.l[1] = XA_NET_WM_STATE_STAYS_ON_TOP;
		else if (_wmType & wm_ABOVE) xev.data.l[1] = XA_NET_WM_STATE_ABOVE;
		else if (_wmType & wm_FULLSCREEN) xev.data.l[1] = XA_NET_WM_STATE_FULLSCREEN;
		else if (_wmType & wm_BELOW) xev.data.l[1] = XA_NET_WM_STATE_BELOW;

		XSendEvent(_display, mRootWin, False, SubstructureRedirectMask, (XEvent *) & xev);
		state = XGetAtomName(_display, xev.data.l[1]);
		LOG_DEBUG("[x11] NET style stay on top (layer" + String::fromNumber(layer) + "). Using state" + state);
		XFree(state);
	}
}

int XVWindow::getWMType() {
	unsigned int i;
	int wmType = 0;
	int metacityHack=0;
	unsigned long nitems;
	Atom *args = NULL;

	// check if WM supports layers
	if (getWindowProperty(XA_WIN_PROTOCOLS, &args, &nitems)) {
		LOG_DEBUG("[x11] Detected wm supports layers.");
		for (i = 0; i < nitems; i++) {
			if (args[i] == XA_WIN_LAYER) {
				wmType |= wm_LAYER;
				metacityHack |= 1;
			} else {
				metacityHack |= 2;
			}
		}
		XFree(args);

		// metacity WM reports that it supports layers, but it is not really truth :-)
		if (wmType && (metacityHack == 1)) {
			wmType ^= wm_LAYER;
			LOG_DEBUG("[x11] Using workaround for Metacity bugs.");
		}
	}

	// NETWM
	if (getWindowProperty(XA_NET_SUPPORTED, &args, &nitems)) {
		LOG_DEBUG("[x11] Detected wm supports NetWM.");
		for (i = 0; i < nitems; i++) {
			wmType |= getSupportedState(args[i]);
		}
		XFree(args);
	}

	// unknown WM
	if (wmType == 0) {
		LOG_DEBUG("[x11] Unknown wm type...");
	}
	return wmType;
}

void XVWindow::getWindowSize(unsigned int* windowWidth, unsigned int* windowHeight) {
	unsigned int ud; Window _dw; int temp;
	XGetGeometry(_display, _XVWindow, &_dw, &temp, &temp, windowWidth, windowHeight, &ud, &ud);
}

void XVWindow::resize (int x, int y, int windowWidth, int windowHeight) {
	XMoveResizeWindow(_display, _XVWindow, x, y, windowWidth, windowHeight);
}

int XVWindow::getWindowProperty(Atom type, Atom ** args, unsigned long *nitems) {
	int format;
	unsigned long bytesafter;
	return (Success == XGetWindowProperty(_display, _rootWindow, type, 0, 16384, False,
			AnyPropertyType, &type, &format, nitems, &bytesafter, (unsigned char **) args) && *nitems > 0);
}

int XVWindow::getSupportedState(Atom atom) {
	if (atom==XA_NET_WM_STATE_FULLSCREEN)	{ return wm_FULLSCREEN;		};
	if (atom==XA_NET_WM_STATE_ABOVE)	{ return wm_ABOVE;		};
	if (atom==XA_NET_WM_STATE_STAYS_ON_TOP)	{ return wm_STAYS_ON_TOP;	};
	if (atom==XA_NET_WM_STATE_BELOW)	{ return wm_BELOW;		};
	return 0;
}

void XVWindow::setSizeHints(int x, int y, int imageWidth, int imageHeight, int windowWidth, int windowHeight) {
	XSizeHints xshints;

	xshints.flags = PPosition | PSize | PAspect | PMinSize;

	xshints.min_aspect.x = imageWidth;
	xshints.min_aspect.y = imageHeight;
	xshints.max_aspect.x = imageWidth;
	xshints.max_aspect.y = imageHeight;

	xshints.x = x;
	xshints.y = y;
	xshints.width = windowWidth;
	xshints.height = windowHeight;
	xshints.min_width = 25;
	xshints.min_height = 25;

	XSetStandardProperties(_display, _XVWindow, "Video", "Video", None, NULL, 0, &xshints);
}

void XVWindow::setDecoration(bool d) {

	static unsigned int oldDecor = MWM_DECOR_ALL;
	static unsigned int oldFuncs = MWM_FUNC_MOVE | MWM_FUNC_CLOSE | MWM_FUNC_MINIMIZE |
			MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE;
	Atom motifHints;
	MotifWmHints setHints;
	MotifWmHints* getHints = NULL;

	Atom mType;
	int mFormat;
	unsigned long mn, mb;

	motifHints = XInternAtom(_display, "_MOTIF_WM_HINTS", 0);
	if (motifHints != None) {
		memset(&setHints, 0, sizeof(setHints));

		if (!d) {
			XGetWindowProperty(_display, _XVWindow, motifHints, 0, 20, False, motifHints, &mType,
					    &mFormat, &mn, &mb, (unsigned char **) &getHints);
			if (getHints) {
				if (getHints->flags & MWM_HINTS_DECORATIONS) oldDecor = getHints->decorations;
				if (getHints->flags & MWM_HINTS_FUNCTIONS) oldFuncs = getHints->functions;
				XFree(getHints);
			}

			setHints.decorations = 0;
		} else {
			setHints.functions = oldFuncs;
			setHints.decorations = oldDecor;
		}

		setHints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;

		XChangeProperty(_display, _XVWindow, motifHints, motifHints, 32,
						PropModeReplace, (unsigned char *) &setHints, 5);
		_state.decoration=!_state.decoration;
	}
}

int XVWindow::getGnomeLayer() {
	Atom type;
	int format;
	unsigned long nitems;
	unsigned long bytesafter;
	unsigned short *args = NULL;

	if (XGetWindowProperty(_display, _XVWindow, XA_WIN_LAYER, 0, 16384, False, AnyPropertyType, &type, &format, &nitems,
		&bytesafter, (unsigned char **) &args) == Success && nitems > 0 && args) {
			LOG_DEBUG("[x11] original window layer is" + String::fromNumber(*args));
			return *args;
		}
		return WIN_LAYER_NORMAL;
}
