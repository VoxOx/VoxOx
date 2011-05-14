/****************************************************************************
**
** Copyright (C) 2007 Trolltech ASA. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qtutil/mac_private.h"
#import <AppKit/AppKit.h>
#include <QtCore/QVarLengthArray>

QString QtCFString::toQString(CFStringRef str)
{
    if(!str)
        return QString();
    CFIndex length = CFStringGetLength(str);
    const UniChar *chars = CFStringGetCharactersPtr(str);
    if (chars)
        return QString(reinterpret_cast<const QChar *>(chars), length);

    QVarLengthArray<UniChar> buffer(length);
    CFStringGetCharacters(str, CFRangeMake(0, length), buffer.data());
    return QString(reinterpret_cast<const QChar *>(buffer.constData()), length);
}

QtCFString::operator QString() const
{
    if (string.isEmpty() && type)
        const_cast<QtCFString*>(this)->string = toQString(type);
    return string;
}

CFStringRef QtCFString::toCFStringRef(const QString &string)
{
    return CFStringCreateWithCharacters(0, reinterpret_cast<const UniChar *>(string.unicode()),
                                        string.length());
}

QtCFString::operator CFStringRef() const
{
    if (!type)
        const_cast<QtCFString*>(this)->type = toCFStringRef(string);
    return type;
}

QtMacCocoaAutoReleasePool::QtMacCocoaAutoReleasePool()
{
    NSApplicationLoad();
    pool = (void*)[[NSAutoreleasePool alloc] init];
}

QtMacCocoaAutoReleasePool::~QtMacCocoaAutoReleasePool()
{
    [(NSAutoreleasePool*)pool release];
}
