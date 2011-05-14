//YLUSBTEL.h Non-MFC DLL header file

//sure it will be included only once
#ifndef _ATM_YLUSBTEL_H_
#define _ATM_YLUSBTEL_H_
#pragma once

#define		VERSION_B2K		0x0520

#define		YL_IOCTL_OPEN_DEVICE				0x0000
#define		YL_IOCTL_CLOSE_DEVICE				0xFFFF

enum {
	YL_CALLBACK_MSG_WARNING = -2,
	YL_CALLBACK_MSG_ERROR = -1,
	YL_CALLBACK_MSG_USBPHONE_VERSION,
	YL_CALLBACK_MSG_USBPHONE_SERIALNO,

	YL_CALLBACK_GEN_KEYBUF_CHANGED,
	YL_CALLBACK_GEN_KEYDOWN,
	YL_CALLBACK_GEN_OFFHOOK,
	YL_CALLBACK_GEN_HANGUP,
	YL_CALLBACK_GEN_INUSB,
	YL_CALLBACK_GEN_INPSTN,

	YL_CALLBACK_GEN_PSTNRING_START,
	YL_CALLBACK_GEN_PSTNRING_STOP,

	YL_CALLBACK_GEN_CALLERID,
};

#define		YL_IOCTL_GEN_READY				0x1001
#define		YL_IOCTL_GEN_UNREADY				0x1002
#define		YL_IOCTL_GEN_CALLIN				0x1003
#define		YL_IOCTL_GEN_CALLOUT				0x1004
#define		YL_IOCTL_GEN_TALKING				0x1006

#define		YL_IOCTL_GEN_GOTOUSB				0x1010
#define		YL_IOCTL_GEN_GOTOPSTN				0x1011
#define		YL_IOCTL_GEN_DEFAULTPSTN			0x1012
#define		YL_IOCTL_GEN_DEFAULTUSB				0x1013
#define		YL_IOCTL_GEN_ONLYUSB				0x1014

#define		YL_IOCTL_OPEN_SIGNAL				0x0107
#define		YL_IOCTL_CLOSE_SIGNAL				0x0207

#define		KEY_0			 0x80
#define		KEY_1			 0x81
#define		KEY_2			 0x82
#define		KEY_3			 0x83
#define		KEY_4			 0x84
#define		KEY_5			 0x85
#define		KEY_6			 0x86
#define		KEY_7			 0x87
#define		KEY_8			 0x88
#define		KEY_9			 0x89
#define		KEY_STAR		 0x8B
#define		KEY_POUND		 0x8C

#define		KEY_SEND		 0x91

#define DLL_EXPORT __declspec(dllexport) 
#define DLL_IMPORT __declspec(dllimport)

#if defined(WIN32)
/* Sheesh */
DLL_EXPORT unsigned long YL_DeviceIoControl(unsigned long dwIoControlCode, 
		void*	lpInBuffer = 0,		unsigned long nInBufferSize = 0, 
		void*	lpOutBuffer = 0,	unsigned long nOutBufferSize = 0);
#else
unsigned long YL_DeviceIoControl(unsigned long dwIoControlCode, 
		void*	lpInBuffer = 0,		unsigned long nInBufferSize = 0, 
		void*	lpOutBuffer = 0,	unsigned long nOutBufferSize = 0);
#endif

// RETURN VALUE & ERROR CODE
enum {
	YL_RETURN_SUCCESS = 0,
	YL_RETURN_NO_FOUND_HID,
	YL_RETURN_HID_ISOPENED,
	YL_RETURN_HID_NO_OPEN,
	YL_RETURN_MAP_ERROR,

	YL_RETURN_DEV_VERSION_ERROR,
	YL_RETURN_HID_COMM_ERROR,
	YL_RETURN_COMMAND_INVALID,
};

#endif
