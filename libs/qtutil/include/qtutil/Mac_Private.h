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
#ifndef MAC_PRIVATE_H
#define MAC_PRIVATE_H

#include <CoreFoundation/CoreFoundation.h>
#include <QtCore/QString>

template <typename T>
class QtCFType
{
public:
    inline QtCFType(const T &t = 0) : type(t) {}
    inline QtCFType(const QtCFType &helper) : type(helper.type) { if (type) CFRetain(type); }
    inline ~QtCFType() { if (type) CFRelease(type); }
    inline operator T() { return type; }
    inline QtCFType operator =(const QtCFType &helper)
    {
	if (helper.type)
	    CFRetain(helper.type);
	CFTypeRef type2 = type;
	type = helper.type;
	if (type2)
	    CFRelease(type2);
	return *this;
    }
    inline T *operator&() { return &type; }
    static QtCFType constructFromGet(const T &t)
    {
        CFRetain(t);
        return QtCFType<T>(t);
    }
protected:
    T type;
};

class QtCFString : public QtCFType<CFStringRef>
{
public:
    inline QtCFString(const QString &str) : QtCFType<CFStringRef>(0), string(str) {}
    inline QtCFString(const CFStringRef cfstr = 0) : QtCFType<CFStringRef>(cfstr) {}
    inline QtCFString(const QtCFType<CFStringRef> &other) : QtCFType<CFStringRef>(other) {}
    operator QString() const;
    operator CFStringRef() const;
    static QString toQString(CFStringRef cfstr);
    static CFStringRef toCFStringRef(const QString &str);
private:
    QString string;
};

class QtMacCocoaAutoReleasePool
{
private:
    void *pool;
public:
    QtMacCocoaAutoReleasePool();
    ~QtMacCocoaAutoReleasePool();

    inline void *handle() const { return pool; }
};


#endif
