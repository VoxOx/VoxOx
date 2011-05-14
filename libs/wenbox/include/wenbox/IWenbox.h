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

#ifndef OWIWENBOX_H
#define OWIWENBOX_H

#include <wenbox/owwenboxdll.h>

#include <cutil/dllexport.h>

#include <util/Interface.h>

#include <string>
#include <list>

/**
 * Plugin interface for USB phone devices.
 *
 * Wenbox is a USB phone device loaded at runtime via a .dll
 * Historically named Wenbox since the first USB device handled by WengoPhone
 * was named Wenbox.
 *
 * Wenbox aims to handle different USB phones via runtime dll loading.
 * Each USB device should implement the interface IWenbox.
 *
 * For an example of IWenbox implementation, check:
 * http://dev.openwengo.com/trac/openwengo/trac.cgi/browser/wengophone-ng/trunk/libs/wenbox/src/yealink
 *
 * All methods are called from outside (i.e WengoPhone) except KeyPressedCallback.
 *
 * Your plugin must be a shared library (.dll under Windows, .dylib under MacOSX and .so under UNIX),
 * named 'wenboxplugin' and should be placed where qtwengophone.exe lies.
 *
 * Check log.txt (same directory as qtwenboxphone.exe) for error messages, examples:
 * <pre>
 * //wenboxplugin.dll not found
 * (error) Win32SharedLibLoader::load: couldn't load dll:126
 * (error) Win32SharedLibLoader::resolve: couldn't resolve symbol:127
 * (error) Wenbox::Wenbox: Wenbox dll not loaded
 *
 * //wenboxplugin.dll loaded + USB device disconnected (Yealink)
 * (debug) Wenbox::Wenbox: Wenbox dll loaded
 * (debug) Wenbox::open: open device
 * (error) YealinkWenbox::errorHandler: no device found
 *
 * //wenboxplugin.dll found + USB device connected (Yealink)
 * (debug) Wenbox::Wenbox: Wenbox dll loaded
 * (debug) Wenbox::open: open device
 * (debug) YealinkWenbox::gotoReady: ready state
 * (debug) YealinkWenbox::callback: Yealink USB Phone version: 0525 - B2K
 * </pre>
 *
 * If you have any problem implementing IWenbox,
 * - check the development website http://dev.openwengo.com/
 * - post on our mailing-list wengophone-devel@lists.openwengo.com
 * - talk to us on our IRC channel irc.freenode.net #openwengo
 * - send me a mail at tanguy.krotoff@wengo.fr
 *
 * @author Tanguy Krotoff
 */
class IWenbox : Interface {
public:

	enum Key {
		Key0,
		Key1,
		Key2,
		Key3,
		Key4,
		Key5,
		Key6,
		Key7,
		Key8,
		Key9,

		/** * button. */
		KeyStar,

		/** # button. */
		KeyPound,

		/** Pick up button. */
		KeyPickUp,

		/** Hang up button. */
		KeyHangUp
	};

	/**
	 * Callback, a key has been pressed from the USB phone device.
	 *
	 * @param key key pressed by the user
	 * @param param user parameter
	 */
	typedef void (*KeyPressedCallback)(Key key, void * param);

	/**
	 * Internal method, only usefull when implementing a Wenbox USB device.
	 *
	 * Sets the callback for the key pressed event.
	 *
	 * @param keyPressedCallback callback called each time a key is pressed
	 * @param param user parameter
	 */
	virtual void setKeyPressedCallback(KeyPressedCallback keyPressedCallback, void * param) = 0;

	virtual ~IWenbox() { }

	/**
	 * Opens the USB device.
	 *
	 * @return true if manage to open the device
	 */
	virtual bool open() = 0;

	/**
	 * Closes the USB device.
	 *
	 * @return true if manage to close the device
	 */
	virtual bool close() = 0;

	/**
	 * Gets the USB device name,
	 *
	 * Example: "Yealink-USB-P1K", "Yealink-USB-B2K"...
	 *
	 * @return USB device name
	 */
	virtual std::string getDeviceName() = 0;

	/**
	 * Gets the audio device name list related to the USB device.
	 *
	 * The list should be ordered from the most specific name to the
	 * most generic (e.g USB).
	 *
	 * @return list of the possible names for the USB audio device
	 */
	virtual std::list<std::string> getAudioDeviceNameList() const = 0;

	enum Mode {
		/** PSTN mode (standard phone line). */
		ModePSTN,

		/** USB mode. */
		ModeUSB
	};

	/**
	 * Sets the default mode: USB or PSTN.
	 *
	 * This method is only available for USB devices
	 * that support PSTN line.
	 *
	 * @param mode new mode
	 * @return true if manage to change the mode
	 */
	virtual bool setDefaultMode(Mode mode) = 0;

	/**
	 * Changes the current mode.
	 *
	 * This method is only available for USB devices
	 * that support PSTN line.
	 *
	 * @param mode new mode
	 * @return true if manage to change the mode
	 */
	virtual bool switchMode(Mode mode) = 0;

	/**
	 * Changes the LCD message on the USB device.
	 *
	 * This method is only available for USB devices
	 * that have a LCD screen.
	 *
	 * @param message message to show on the LCD screen
	 * @return true if manage to change the LCD screen message
	 */
	virtual bool setLCDMessage(const std::string & message) = 0;

	/**
	 * Changes the ringing tone.
	 *
	 * @param tone new ringing tone
	 * @return true if manage to change the ringing tone
	 */
	virtual bool setRingingTone(int tone) = 0;

	enum PhoneCallState {
		/** Incoming phone call. */
		CallIncoming,

		/** Ringing state. */
		CallRinging,

		/** Conversation state. */
		CallTalking,

		/** An error occured. */
		CallError,

		/** Outgoing call. */
		CallOutgoing,

		/** Phone call closed (call rejected or call hang up). */
		CallClosed
	};

	/**
	 * Changes the USB device state.
	 *
	 * @param state new state
	 * @param phoneNumber only used when in state CallIncoming
	 * @return true if manage to change the state
	 */
	virtual bool setState(PhoneCallState state, const std::string & phoneNumber) = 0;
};

/**
 * Gets the IWenbox instance.
 *
 * Used for loading the DLL at runtime.
 * It's easier to load at runtime a C function rather than a C++ method.
 */
extern "C" OWWENBOX_API IWenbox * getInstance();

#endif	//OWIWENBOX_H
