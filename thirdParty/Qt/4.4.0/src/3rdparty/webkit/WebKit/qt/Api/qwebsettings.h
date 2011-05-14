/*
    Copyright (C) 2007-2008 Trolltech ASA

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QWEBSETTINGS_H
#define QWEBSETTINGS_H

#include "qwebkitglobal.h"

#include <QtCore/qstring.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qicon.h>
#include <QtCore/qshareddata.h>

namespace WebCore
{
    class Settings;
}

class QWebPage;
class QWebSettingsPrivate;
QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class QWEBKIT_EXPORT QWebSettings
{
public:
    enum FontFamily {
        StandardFont,
        FixedFont,
        SerifFont,
        SansSerifFont,
        CursiveFont,
        FantasyFont
    };
    enum WebAttribute {
        AutoLoadImages,
        JavascriptEnabled,
        JavaEnabled,
        PluginsEnabled,
        PrivateBrowsingEnabled,
        JavascriptCanOpenWindows,
        JavascriptCanAccessClipboard,
        DeveloperExtrasEnabled,
        LinksIncludedInFocusChain
    };
    enum WebGraphic {
        MissingImageGraphic,
        MissingPluginGraphic,
        DefaultFrameIconGraphic,
        TextAreaSizeGripCornerGraphic
    };
    enum FontSize {
        MinimumFontSize,
        MinimumLogicalFontSize,
        DefaultFontSize,
        DefaultFixedFontSize
    };

    static QWebSettings *globalSettings();

    void setFontFamily(FontFamily which, const QString &family);
    QString fontFamily(FontFamily which) const;
    void resetFontFamily(FontFamily which);

    void setFontSize(FontSize type, int size);
    int fontSize(FontSize type) const;
    void resetFontSize(FontSize type);

    void setAttribute(WebAttribute attr, bool on);
    bool testAttribute(WebAttribute attr) const;
    void resetAttribute(WebAttribute attr);

    void setUserStyleSheetUrl(const QUrl &location);
    QUrl userStyleSheetUrl() const;

    static void setIconDatabasePath(const QString &location);
    static QString iconDatabasePath();
    static void clearIconDatabase();
    static QIcon iconForUrl(const QUrl &url);

    static void setWebGraphic(WebGraphic type, const QPixmap &graphic);
    static QPixmap webGraphic(WebGraphic type);

    static void setMaximumPagesInCache(int pages);
    static int maximumPagesInCache();
    static void setObjectCacheCapacities(int cacheMinDeadCapacity, int cacheMaxDead, int totalCapacity);

private:
    friend class QWebPagePrivate;
    friend class QWebSettingsPrivate;

    Q_DISABLE_COPY(QWebSettings)

    QWebSettings();
    QWebSettings(WebCore::Settings *settings);
    ~QWebSettings();

    QWebSettingsPrivate *d;
};

#endif
