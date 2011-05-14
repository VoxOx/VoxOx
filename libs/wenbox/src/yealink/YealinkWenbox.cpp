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

#include "YealinkWenbox.h"

#define LOGGER_COMPONENT "Wenbox"
#include <util/Logger.h>
#include <util/StringList.h>

#include "YLTELBOX.h"

IWenbox * getInstance() {
	YealinkWenbox * wenbox = new YealinkWenbox();
	return wenbox;
}

YealinkWenbox::YealinkWenbox() {
}

YealinkWenbox::~YealinkWenbox() {
}

void YealinkWenbox::setKeyPressedCallback(KeyPressedCallback keyPressedCallback, void * param) {
	_keyPressedCallback = keyPressedCallback;
	_userCallbackParam = param;
}

void /*YealinkWenbox::*/callback(void * wParam, void * lParam, unsigned long instance) {
	YealinkWenbox * wenbox = (YealinkWenbox *) instance;
	wenbox->callback((unsigned int) wParam, (long) lParam);
}

bool YealinkWenbox::open() {
	unsigned long status = YL_DeviceIoControl(YL_IOCTL_OPEN_DEVICE, (void *) ::callback, (unsigned long) this, 0, 0);
	if (errorHandler(status)) {
		return gotoReady();
	}
	return false;
}

bool YealinkWenbox::close() {
	gotoUnReady();
	unsigned long status = YL_DeviceIoControl(YL_IOCTL_CLOSE_DEVICE, 0, 0, 0, 0);
	return errorHandler(status);
}

std::string YealinkWenbox::getDeviceName() {
	return "Yealink-Wenbox";
}

std::list<std::string> YealinkWenbox::getAudioDeviceNameList() const {
	StringList deviceList;

	//winXP us/fr
	//win2k us
	deviceList += "USB Audio Device";

	//win2k fr
	deviceList += "Périphérique audio USB";

	//generic
	deviceList += "USB";

	return deviceList;
}

bool YealinkWenbox::setDefaultMode(Mode mode) {
	unsigned long status;

	switch(mode) {
	case ModeUSB:
		LOG_DEBUG("default to USB mode");
		status = YL_DeviceIoControl(YL_IOCTL_GEN_DEFAULTUSB, 0, 0, 0, 0);
		return errorHandler(status);

	case ModePSTN:
		LOG_DEBUG("default to PSTN mode");
		status = YL_DeviceIoControl(YL_IOCTL_GEN_DEFAULTPSTN, 0, 0, 0, 0);
		return errorHandler(status);

	default:
		LOG_FATAL("unknown mode");
		return false;
	}
}

bool YealinkWenbox::switchMode(Mode mode) {
	unsigned long status;

	switch(mode) {
	case ModeUSB:
		LOG_DEBUG("switch to USB mode");
		status = YL_DeviceIoControl(YL_IOCTL_GEN_GOTOUSB, 0, 0, 0, 0);
		return errorHandler(status);

	case ModePSTN:
		LOG_DEBUG("switch to PSTN mode");
		status = YL_DeviceIoControl(YL_IOCTL_GEN_GOTOPSTN, 0, 0, 0, 0);
		return errorHandler(status);

	default:
		LOG_FATAL("unknown mode");
		return false;
	}
}

bool YealinkWenbox::setLCDMessage(const std::string & message) {
	/*
	status = YL_DeviceIoControl(YL_IOCTL_SHOW_LCD, (void *)(char *) message.c_str(), message.length(), 0, 0);
	return errorHandler(status);
	*/
	return false;
}

bool YealinkWenbox::setRingingTone(int tone) {
	/*
	status = YL_DeviceIoControl(YL_IOCTL_GEN_RINGSELECT, &tone, sizeof(int), 0, 0);
	return errorHandler(status);
	*/
	return false;
}

bool YealinkWenbox::setState(PhoneCallState state, const std::string & phoneNumber) {
	unsigned long status;

	switch(state) {
	case CallIncoming:
		LOG_DEBUG("incoming call=" + phoneNumber);
		//YL_DeviceIoControl(YL_IOCTL_SET_LCD, (void *)(char *) phoneNumber.c_str(), phoneNumber.length(), 0, 0);
		status = YL_DeviceIoControl(YL_IOCTL_GEN_CALLIN, (void *)(char *) phoneNumber.c_str(), phoneNumber.length(), 0, 0);
		return errorHandler(status);

	case CallRinging:
		//Same as talking state
		LOG_DEBUG("ringing state");
		status = YL_DeviceIoControl(YL_IOCTL_GEN_TALKING, 0, 0, 0, 0);
		return errorHandler(status);

	case CallTalking:
		LOG_DEBUG("talking state");
		status = YL_DeviceIoControl(YL_IOCTL_GEN_TALKING, 0, 0, 0, 0);
		return errorHandler(status);

	case CallError:
		LOG_DEBUG("call error");
		return true;

	case CallOutgoing:
		LOG_DEBUG("call outgoing");
		status = YL_DeviceIoControl(YL_IOCTL_GEN_CALLOUT, 0, 0, 0, 0);
		return errorHandler(status);

	case CallClosed:
		LOG_DEBUG("call closed");
		return gotoReady();

	default:
		LOG_FATAL("unknown state=" + String::fromNumber(state));
		return false;
	}
}

bool YealinkWenbox::errorHandler(unsigned long status) {
	switch(status) {
	case YL_RETURN_SUCCESS:
		return true;

	case YL_RETURN_NO_FOUND_HID:
		LOG_DEBUG("no device found");
		break;

	case YL_RETURN_HID_ISOPENED:
		LOG_WARN("re-open invalid");
		break;

	case YL_RETURN_HID_NO_OPEN:
		LOG_WARN("couldn't open device");
		break;

	case YL_RETURN_MAP_ERROR:
		LOG_WARN("memory map error");
		break;

	case YL_RETURN_DEV_VERSION_ERROR:
		LOG_WARN("incorrect device version");
		break;

	case YL_RETURN_HID_COMM_ERROR:
		LOG_WARN("HID communication error");
		break;

	case YL_RETURN_COMMAND_INVALID:
		LOG_WARN("invalid command");
		break;

	default:
		LOG_FATAL("unknow status=" + String::fromNumber(status));
	}

	return false;
}

bool YealinkWenbox::gotoReady() {
	LOG_DEBUG("ready state");
	unsigned long status = YL_DeviceIoControl(YL_IOCTL_GEN_READY, 0, 0, 0, 0);
	return errorHandler(status);
}

bool YealinkWenbox::gotoUnReady() {
	LOG_DEBUG("unready state");
	unsigned long status = YL_DeviceIoControl(YL_IOCTL_GEN_UNREADY, 0, 0, 0, 0);
	return errorHandler(status);
}

void YealinkWenbox::callback(unsigned int wParam, long lParam) {
	switch(wParam) {

	case YL_CALLBACK_MSG_USBPHONE_VERSION: {
		//FIXME commented since it crashes under Visual C++ 8.0
		//do not know why...
		/*char str[10];
		unsigned short ver = (unsigned short) lParam;
		sprintf(str, (ver>=0x0520)?"%04x - B2K":"%04x - Err", ver);
		LOG_DEBUG("Yealink USB Phone version: " + std::string(str));*/
		LOG_DEBUG("Yealink USB Phone");
		break;
	}

	case YL_CALLBACK_MSG_USBPHONE_SERIALNO: {
		LOG_DEBUG(String::fromNumber(lParam));
		break;
	}

	case YL_CALLBACK_GEN_OFFHOOK:
		LOG_DEBUG("wenbox offhook");
		_keyPressedCallback(KeyPickUp, _userCallbackParam);
		break;

	case YL_CALLBACK_GEN_HANGUP:
		LOG_DEBUG("wenbox hangup");
		_keyPressedCallback(KeyHangUp, _userCallbackParam);
		break;

	case YL_CALLBACK_GEN_KEYBUF_CHANGED: {
		std::string keyStr((char *) lParam);
		LOG_DEBUG("GEN_KEYBUF_CHANGED key pressed=" + keyStr);
		break;
	}

	case YL_CALLBACK_GEN_KEYDOWN: {
		int key = (int) lParam;
		LOG_DEBUG("GEN_KEYDOWN key pressed=" + String::fromNumber(key));

		switch(key) {
		case KEY_0:
			_keyPressedCallback(Key0, _userCallbackParam);
			break;

		case KEY_1:
			_keyPressedCallback(Key1, _userCallbackParam);
			break;

		case KEY_2:
			_keyPressedCallback(Key2, _userCallbackParam);
			break;

		case KEY_3:
			_keyPressedCallback(Key3, _userCallbackParam);
			break;

		case KEY_4:
			_keyPressedCallback(Key4, _userCallbackParam);
			break;

		case KEY_5:
			_keyPressedCallback(Key5, _userCallbackParam);
			break;

		case KEY_6:
			_keyPressedCallback(Key6, _userCallbackParam);
			break;

		case KEY_7:
			_keyPressedCallback(Key7, _userCallbackParam);
			break;

		case KEY_8:
			_keyPressedCallback(Key8, _userCallbackParam);
			break;

		case KEY_9:
			_keyPressedCallback(Key9, _userCallbackParam);
			break;

		case KEY_STAR:
			_keyPressedCallback(KeyStar, _userCallbackParam);
			break;

		case KEY_POUND:
			_keyPressedCallback(KeyPound, _userCallbackParam);
			break;

		case KEY_SEND:
			break;
		}

		break;
	}

	case YL_CALLBACK_GEN_PSTNRING_START:
		break;

	case YL_CALLBACK_GEN_PSTNRING_STOP:
		break;

	case YL_CALLBACK_GEN_INUSB:
		LOG_DEBUG("USB mode");
		break;

	case YL_CALLBACK_GEN_INPSTN:
		LOG_DEBUG("PSTN mode");
		break;

	case YL_CALLBACK_MSG_WARNING:
		errorHandler((unsigned long) lParam);
		break;

	case YL_CALLBACK_MSG_ERROR:
		errorHandler((unsigned long) lParam);
		break;

	default:
		LOG_FATAL("unknown param=" + String::fromNumber(wParam));
	}
}
