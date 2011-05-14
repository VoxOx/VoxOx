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
#include <qtutil/MacDesktopService.h>

#include <QtCore/QDir>

#include <QtGui/QImage>
#include <QtGui/QPixmap>

#include <CGGeometry.h>


const char* VOLUMES_DIR = "/Volumes";
const char* NETWORK_DIR = "/Network";


/**
 * This is a simple wrapper around Carbon references, which will make sure
 * CFRelease is called when getting out of scope
 */
template <class Ref>
class RefWrapper {
public:
	RefWrapper(const Ref& ref)
	: _ref(ref) {
	}

	~RefWrapper() {
		if (_ref) {
			CFRelease(_ref);
		}
	}

	operator Ref() {
		return _ref;
	}

	bool operator !() {
		return !_ref;
	}

private:
	Ref _ref;
};




static QPixmap qpixmapFromIconRef(IconRef iconRef, int size) {
	OSErr result;
	int iconSize;
	OSType elementType;

	// Determine elementType and iconSize
	if (size <= 16) {
		elementType = kSmall32BitData;
		iconSize = 16;
	} else if (size <= 32) {
		elementType = kLarge32BitData;
		iconSize = 32;
	} else {
		elementType = kThumbnail32BitData;
		iconSize = 128;
	}

	// Get icon into an IconFamily
	IconFamilyHandle hIconFamily = 0;
	IconRefToIconFamily(iconRef, kSelectorAllAvailableData, &hIconFamily);

	// Extract data
	Handle hRawBitmapData = NewHandle(iconSize * iconSize * 4);
	result = GetIconFamilyData( hIconFamily, elementType, hRawBitmapData );
	if (result != noErr) {
		DisposeHandle( hRawBitmapData );
		return QPixmap();
	}

	// Convert data to QImage
	QImage image(iconSize, iconSize, QImage::Format_ARGB32);
	HLock(hRawBitmapData);
	unsigned long* data = (unsigned long*) *hRawBitmapData;
	for (int posy=0; posy<iconSize; ++posy, data+=iconSize) {
	#ifdef __BIG_ENDIAN__
		uchar* line = image.scanLine(posy);
		memcpy(line, data, iconSize * 4);
	#else
		uchar* src = (uchar*) data;
		uchar* dst = image.scanLine(posy);
		for (int posx=0; posx<iconSize; src+=4, dst+=4, ++posx) {
			dst[0] = src[3];
			dst[1] = src[2];
			dst[2] = src[1];
			dst[3] = src[0];
		}
	#endif
	}
	HUnlock(hRawBitmapData);
	DisposeHandle( hRawBitmapData );

	// Scale to wanted size
	image = image.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	return QPixmap::fromImage(image);
}


QPixmap MacDesktopService::pixmapForPath(const QString& path, int size) {
	OSErr result;
	SInt16 label;
	FSRef fsRef;
	IconRef iconRef;

	result = FSPathMakeRef(
		(UInt8*)path.toUtf8().data(),
		&fsRef,
		0);
	if (result != noErr) {
		return QPixmap();
	}

	GetIconRefFromFileInfo(
		&fsRef,
		0,
		NULL,
		kFSCatInfoNone,
		NULL,
		kIconServicesNormalUsageFlag,
		&iconRef,
		&label);


	QPixmap pixmap = qpixmapFromIconRef(iconRef, size);
	ReleaseIconRef(iconRef);

	return pixmap;
}


QPixmap MacDesktopService::desktopIconPixmap(DesktopIconType icon, int size) {
	OSType macIcon;
	switch (icon) {
	case FolderIcon:
		macIcon = kGenericFolderIcon;
		break;
	case FileIcon:
		macIcon = kGenericDocumentIcon;
		break;
	}

	IconRef iconRef = 0;
	GetIconRef(kOnSystemDisk, kSystemIconsCreator, macIcon, &iconRef);
	QPixmap pixmap = qpixmapFromIconRef(iconRef, size);
	ReleaseIconRef(iconRef);

	return pixmap;
}


/**
 * Copied from:
 * http://developer.apple.com/documentation/CoreFoundation/Conceptual/CFPropertyLists/index.html
 */
static CFPropertyListRef createPropertyListFromFile(CFURLRef fileURL) {
	CFPropertyListRef propertyList;
	CFStringRef       errorString;
	CFDataRef         resourceData;
	Boolean           status;
	SInt32            errorCode;

	// Read the XML file.
	status = CFURLCreateDataAndPropertiesFromResource(
			kCFAllocatorDefault,
			fileURL,
			&resourceData,            // place to put file data
			NULL,      
			NULL,
			&errorCode);

	// Reconstitute the dictionary using the XML data.
	propertyList = CFPropertyListCreateFromXMLData( kCFAllocatorDefault,
			resourceData,
			kCFPropertyListImmutable,
			&errorString);

	CFRelease( resourceData );
	return propertyList;
}


static CFURLRef createCFURLForPath(const QString& path) {
	RefWrapper<CFStringRef> pathString = CFStringCreateWithCString(
		kCFAllocatorDefault,
		path.toUtf8().data(),
		kCFStringEncodingUTF8);

	return CFURLCreateWithFileSystemPath(
		kCFAllocatorDefault, 
		pathString,
		kCFURLPOSIXPathStyle, false /* directory */);
}


static CFArrayRef getCustomListItemsArray(CFPropertyListRef propertyList) {
	CFDictionaryRef dictionary = (CFDictionaryRef)propertyList;
	CFDictionaryRef useritems = (CFDictionaryRef)CFDictionaryGetValue(dictionary, CFSTR("useritems"));

	if (!useritems || CFGetTypeID(useritems) != CFDictionaryGetTypeID()) {
		return 0;
	}

	CFArrayRef customListItems = (CFArrayRef)CFDictionaryGetValue(useritems, CFSTR("CustomListItems"));
	if (!customListItems || CFGetTypeID(customListItems) != CFArrayGetTypeID()) {
		return 0;
	}
	return customListItems;
}


static void fillStartFolderList(const void* value, void* context) {
	CFDictionaryRef itemDict = static_cast<CFDictionaryRef>(value);
	CFDataRef aliasData = static_cast<CFDataRef>(
		CFDictionaryGetValue(itemDict, CFSTR("Alias"))
		);
	if (!aliasData) {
		return;
	}

	int length = CFDataGetLength(aliasData);
	AliasHandle aliasHandle = (AliasHandle)NewHandle(length);
	CFRange range;
	range.location = 0;
	range.length = length;
	CFDataGetBytes(aliasData, range, (UInt8*)*aliasHandle);

	FSRef fsRef;
	Boolean wasChanged;
	
	OSErr err = FSResolveAliasWithMountFlags(
			0,
			aliasHandle,
			&fsRef,
			&wasChanged,
			kResolveAliasFileNoUI);

	DisposeHandle((Handle)aliasHandle);

	if (err != noErr) {
		return;
	}

	char path[256];
	FSRefMakePath(&fsRef, (UInt8*)path, sizeof(path));

	QStringList* list = static_cast<QStringList*>(context);
	list->append(QString::fromUtf8(path));
}


static QStringList userStartFolderList() {
	QString plistFilePath = QDir::homePath() + "/Library/Preferences/com.apple.sidebarlists.plist";
	RefWrapper<CFURLRef> fileURL = createCFURLForPath(plistFilePath);
	RefWrapper<CFPropertyListRef> propertyList = createPropertyListFromFile(fileURL);

	if (CFGetTypeID(propertyList) != CFDictionaryGetTypeID()) {
		return QStringList();
	}

	CFArrayRef customListItems = getCustomListItemsArray(propertyList);
	if (!customListItems) {
		return QStringList();
	}

	QStringList list;

	CFRange range;
	range.location = 0;
	range.length = CFArrayGetCount(customListItems);
	CFArrayApplyFunction(customListItems, range, fillStartFolderList, &list);

	return list;
}


QStringList MacDesktopService::startFolderList() {
	QStringList list;
	QDir volumeDir(VOLUMES_DIR);
	QStringList volumeList = volumeDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
	Q_FOREACH(QString volume, volumeList) {
		list << QString(VOLUMES_DIR) + "/" + volume;
	}

	list << NETWORK_DIR;

	list << userStartFolderList();

	return list;
}


QString MacDesktopService::userFriendlyNameForPath(const QString & path) {
	FSRef fsRef;
	int err = FSPathMakeRef((UInt8*)path.toUtf8().data(), &fsRef, 0 /* isDirectory */);
	if (err != noErr) {
		return DesktopService::userFriendlyNameForPath(path);
	}
	
	CFStringRef nameString;
	LSCopyDisplayNameForRef(&fsRef, &nameString);

	char name[256];
	CFStringGetCString(nameString, name, sizeof(name), kCFStringEncodingUTF8);
	CFRelease(nameString);

	return QString::fromUtf8(name);
}
