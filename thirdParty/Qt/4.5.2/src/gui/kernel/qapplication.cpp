/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qabstracteventdispatcher.h"
#include "qaccessible.h"
#include "qapplication.h"
#include "qclipboard.h"
#include "qcursor.h"
#include "qdesktopwidget.h"
#include "qdir.h"
#include "qevent.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qgraphicsscene.h"
#include "qhash.h"
#include "qset.h"
#include "qlayout.h"
#include "qsessionmanager.h"
#include "qstyle.h"
#include "qstylefactory.h"
#include "qtextcodec.h"
#include "qtranslator.h"
#include "qvariant.h"
#include "qwidget.h"
#include "qdnd_p.h"
#include "qcolormap.h"
#include "qdebug.h"
#include "private/qgraphicssystemfactory_p.h"
#include "private/qstylesheetstyle_p.h"
#include "private/qstyle_p.h"
#include "qmessagebox.h"
#include <QtGui/qgraphicsproxywidget.h>

#include "qinputcontext.h"
#include "qkeymapper_p.h"

#ifdef Q_WS_X11
#include <private/qt_x11_p.h>
#include "qinputcontextfactory.h"
#endif

#include <qthread.h>
#include <private/qthread_p.h>

#include <private/qfont_p.h>

#include <stdlib.h>

#include "qapplication_p.h"
#include "qwidget_p.h"

#include "qapplication.h"

#ifdef Q_OS_WINCE
#include "qdatetime.h"
#include "qguifunctions_wince.h"
extern bool qt_wince_is_smartphone(); //qguifunctions_wince.cpp
extern bool qt_wince_is_mobile();     //qguifunctions_wince.cpp
extern bool qt_wince_is_pocket_pc();  //qguifunctions_wince.cpp
#endif

//#define ALIEN_DEBUG

static void initResources()
{
#ifdef Q_OS_WINCE
    Q_INIT_RESOURCE(qstyle_wince);
#else
    Q_INIT_RESOURCE(qstyle);
#endif

#if !defined(QT_NO_DIRECT3D) && defined(Q_WS_WIN)
    Q_INIT_RESOURCE(qpaintengine_d3d);
#endif
    Q_INIT_RESOURCE(qmessagebox);
#if !defined(QT_NO_PRINTDIALOG)
    Q_INIT_RESOURCE(qprintdialog);
#endif

}

QT_BEGIN_NAMESPACE

extern void qt_call_post_routines();

int QApplicationPrivate::app_compile_version = 0x040000; //we don't know exactly, but it's at least 4.0.0

QApplication::Type qt_appType=QApplication::Tty;
QApplicationPrivate *QApplicationPrivate::self = 0;

QInputContext *QApplicationPrivate::inputContext;

bool QApplicationPrivate::quitOnLastWindowClosed = true;

#ifdef Q_OS_WINCE
int QApplicationPrivate::autoMaximizeThreshold = -1;
bool QApplicationPrivate::autoSipEnabled = false;
#endif

QApplicationPrivate::QApplicationPrivate(int &argc, char **argv, QApplication::Type type)
    : QCoreApplicationPrivate(argc, argv)
{
    application_type = type;
    qt_appType = type;

#ifndef QT_NO_SESSIONMANAGER
    is_session_restored = false;
#endif

    quitOnLastWindowClosed = true;

#ifdef QT3_SUPPORT
    qt_compat_used = 0;
    qt_compat_resolved = 0;
    qt_tryAccelEvent = 0;
    qt_tryComposeUnicode = 0;
    qt_dispatchAccelEvent = 0;
#endif
#if defined(Q_WS_QWS) && !defined(QT_NO_DIRECTPAINTER)
    directPainters = 0;
#endif

    if (!self)
        self = this;
}

QApplicationPrivate::~QApplicationPrivate()
{
    if (self == this)
        self = 0;
}

/*!
    \class QApplication
    \brief The QApplication class manages the GUI application's control
    flow and main settings.

    \ingroup application
    \mainclass

    QApplication contains the main event loop, where all events from the window
    system and other sources are processed and dispatched. It also handles the
    application's initialization and finalization, and provides session
    management. In addition, it handles most system-wide and application-wide
    settings.

    For any GUI application using Qt, there is precisely one QApplication
    object, no matter whether the application has 0, 1, 2 or more windows at
    any given time. For non-GUI Qt applications, use QCoreApplication instead,
    as it does not depend on the \l QtGui library.

    The QApplication object is accessible through the instance() function that
    returns a pointer equivalent to the global qApp pointer.

    QApplication's main areas of responsibility are:
        \list
            \o  It initializes the application with the user's desktop settings
                such as palette(), font() and doubleClickInterval(). It keeps
                track of these properties in case the user changes the desktop
                globally, for example through some kind of control panel.

            \o  It performs event handling, meaning that it receives events
                from the underlying window system and dispatches them to the
                relevant widgets. By using sendEvent() and postEvent() you can
                send your own events to widgets.

            \o  It parses common command line arguments and sets its internal
                state accordingly. See the \l{QApplication::QApplication()}
                {constructor documentation} below for more details.

            \o  It defines the application's look and feel, which is
                encapsulated in a QStyle object. This can be changed at runtime
                with setStyle().

            \o  It specifies how the application is to allocate colors. See
                setColorSpec() for details.

            \o  It provides localization of strings that are visible to the
                user via translate().

            \o  It provides some magical objects like the desktop() and the
                clipboard().

            \o  It knows about the application's windows. You can ask which
                widget is at a certain position using widgetAt(), get a list of
                topLevelWidgets() and closeAllWindows(), etc.

            \o  It manages the application's mouse cursor handling, see
                setOverrideCursor()

            \o  On the X window system, it provides functions to flush and sync
                the communication stream, see flushX() and syncX().

            \o  It provides support for sophisticated \l{Session Management}
                {session management}. This makes it possible for applications
                to terminate gracefully when the user logs out, to cancel a
                shutdown process if termination isn't possible and even to
                preserve the entire application's state for a future session.
                See isSessionRestored(), sessionId() and commitData() and
                saveState() for details.
        \endlist

    The QApplication object does so much initialization. Hence, it \e{must} be
    created before any other objects related to the user interface are created.
    Since QApplication also deals with common command line arguments, it is
    usually a good idea to create it \e before any interpretation or
    modification of \c argv is done in the application itself.

    \table
    \header
        \o{2,1} Groups of functions

        \row
        \o  System settings
        \o  desktopSettingsAware(),
            setDesktopSettingsAware(),
            cursorFlashTime(),
            setCursorFlashTime(),
            doubleClickInterval(),
            setDoubleClickInterval(),
            setKeyboardInputInterval(),
            wheelScrollLines(),
            setWheelScrollLines(),
            palette(),
            setPalette(),
            font(),
            setFont(),
            fontMetrics().

        \row
        \o  Event handling
        \o  exec(),
            processEvents(),
            exit(),
            quit().
            sendEvent(),
            postEvent(),
            sendPostedEvents(),
            removePostedEvents(),
            hasPendingEvents(),
            notify(),
            macEventFilter(),
            qwsEventFilter(),
            x11EventFilter(),
            x11ProcessEvent(),
            winEventFilter().

        \row
        \o  GUI Styles
        \o  style(),
            setStyle().

        \row
        \o  Color usage
        \o  colorSpec(),
            setColorSpec(),
            qwsSetCustomColors().

        \row
        \o  Text handling
        \o  installTranslator(),
            removeTranslator()
            translate().

        \row
        \o  Widgets
        \o  allWidgets(),
            topLevelWidgets(),
            desktop(),
            activePopupWidget(),
            activeModalWidget(),
            clipboard(),
            focusWidget(),
            activeWindow(),
            widgetAt().

        \row
        \o  Advanced cursor handling
        \o  overrideCursor(),
            setOverrideCursor(),
            restoreOverrideCursor().

        \row
        \o  X Window System synchronization
        \o  flushX(),
            syncX().

        \row
        \o  Session management
        \o  isSessionRestored(),
            sessionId(),
            commitData(),
            saveState().

        \row
        \o  Miscellaneous
        \o  closeAllWindows(),
            startingUp(),
            closingDown(),
            type().
    \endtable

    \sa QCoreApplication, QAbstractEventDispatcher, QEventLoop, QSettings
*/

/*!
    \enum QApplication::Type

    \value Tty a console application
    \value GuiClient a GUI client application
    \value GuiServer a GUI server application (for Qt for Embedded Linux)
*/

/*!
    \enum QApplication::ColorSpec

    \value NormalColor the default color allocation policy
    \value CustomColor the same as NormalColor for X11; allocates colors
    to a palette on demand under Windows
    \value ManyColor the right choice for applications that use thousands of
    colors

    See setColorSpec() for full details.
*/

/*!
    \fn QWidget *QApplication::topLevelAt(const QPoint &point)

    Returns the top-level widget at the given \a point; returns 0 if
    there is no such widget.
*/

/*!
    \fn QWidget *QApplication::topLevelAt(int x, int y)

    \overload

    Returns the top-level widget at the point (\a{x}, \a{y}); returns
    0 if there is no such widget.
*/


/*
    The qt_init() and qt_cleanup() functions are implemented in the
    qapplication_xyz.cpp file.
*/

void qt_init(QApplicationPrivate *priv, int type
#ifdef Q_WS_X11
              , Display *display = 0, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0
#endif
   );
void qt_cleanup();

Qt::MouseButtons QApplicationPrivate::mouse_buttons = Qt::NoButton;
Qt::KeyboardModifiers QApplicationPrivate::modifier_buttons = Qt::NoModifier;

QStyle *QApplicationPrivate::app_style = 0;        // default application style

#ifndef QT_NO_STYLE_STYLESHEET
QString QApplicationPrivate::styleSheet;           // default application stylesheet
#endif
QPointer<QWidget> QApplicationPrivate::leaveAfterRelease = 0;

int QApplicationPrivate::app_cspec = QApplication::NormalColor;
QPalette *QApplicationPrivate::app_pal = 0;        // default application palette
QPalette *QApplicationPrivate::sys_pal = 0;        // default system palette
QPalette *QApplicationPrivate::set_pal = 0;        // default palette set by programmer

QGraphicsSystem *QApplicationPrivate::graphics_system = 0; // default graphics system
QString QApplicationPrivate::graphics_system_name;         // graphics system id - for delayed initialization

Q_GLOBAL_STATIC(QMutex, applicationFontMutex);
QFont *QApplicationPrivate::app_font = 0;        // default application font
QFont *QApplicationPrivate::sys_font = 0;        // default system font
QFont *QApplicationPrivate::set_font = 0;        // default font set by programmer

QIcon *QApplicationPrivate::app_icon = 0;
QWidget *QApplicationPrivate::main_widget = 0;        // main application widget
QWidget *QApplicationPrivate::focus_widget = 0;        // has keyboard input focus
QWidget *QApplicationPrivate::hidden_focus_widget = 0; // will get keyboard input focus after show()
QWidget *QApplicationPrivate::active_window = 0;        // toplevel with keyboard focus
bool QApplicationPrivate::obey_desktop_settings = true;        // use winsys resources
int QApplicationPrivate::cursor_flash_time = 1000;        // text caret flash time
int QApplicationPrivate::mouse_double_click_time = 400;        // mouse dbl click limit
int QApplicationPrivate::keyboard_input_time = 400; // keyboard input interval
#ifndef QT_NO_WHEELEVENT
int QApplicationPrivate::wheel_scroll_lines;   // number of lines to scroll
#endif
bool qt_is_gui_used;
bool Q_GUI_EXPORT qt_tab_all_widgets = true;
bool qt_in_tab_key_event = false;
int qt_antialiasing_threshold = -1;
static int drag_time = 500;
static int drag_distance = 4;
static Qt::LayoutDirection layout_direction = Qt::LeftToRight;
QSize QApplicationPrivate::app_strut = QSize(0,0); // no default application strut

int QApplicationPrivate::voxToolTipDelayTime = 700;//VOXOX - CJC - 2009.11.05 

bool QApplicationPrivate::animate_ui = true;
bool QApplicationPrivate::animate_menu = false;
bool QApplicationPrivate::fade_menu = false;
bool QApplicationPrivate::animate_combo = false;
bool QApplicationPrivate::animate_tooltip = false;
bool QApplicationPrivate::fade_tooltip = false;
bool QApplicationPrivate::animate_toolbox = false;
bool QApplicationPrivate::widgetCount = false;
QString* QApplicationPrivate::styleOverride = 0;
#if defined(Q_WS_WIN) && !defined(Q_OS_WINCE)
bool QApplicationPrivate::inSizeMove = false;
#endif
#ifdef QT_KEYPAD_NAVIGATION
bool QApplicationPrivate::keypadNavigation = false;
QWidget *QApplicationPrivate::oldEditFocus = 0;
#endif

bool qt_tabletChokeMouse = false;
static bool force_reverse = false;

static inline bool isAlien(QWidget *widget)
{
    if (!widget)
        return false;
#ifdef Q_WS_MAC // Fake alien behavior on the Mac :)
    return !widget->isWindow() && widget->window()->testAttribute(Qt::WA_DontShowOnScreen);
#else
    return !widget->internalWinId();
#endif
}

// ######## move to QApplicationPrivate
// Default application palettes and fonts (per widget type)

typedef QHash<QByteArray, QPalette> PaletteHash;
Q_GLOBAL_STATIC(PaletteHash, app_palettes)
PaletteHash *qt_app_palettes_hash()
{
    return app_palettes();
}

typedef QHash<QByteArray, QFont> FontHash;
Q_GLOBAL_STATIC(FontHash, app_fonts)
FontHash *qt_app_fonts_hash()
{
    return app_fonts();
}

QWidgetList *QApplicationPrivate::popupWidgets = 0;        // has keyboard input focus

QDesktopWidget *qt_desktopWidget = 0;                // root window widgets
#ifndef QT_NO_CLIPBOARD
QClipboard              *qt_clipboard = 0;        // global clipboard object
#endif
QWidgetList * qt_modal_stack=0;                // stack of modal widgets

/*!
    \internal
*/
void QApplicationPrivate::process_cmdline()
{
    Q_Q(QApplication);
    Q_UNUSED(q);// only static members being used.
    // process platform-indep command line
    if (!qt_is_gui_used || !argc)
        return;

    int i, j;

    j = 1;
    for (i=1; i<argc; i++) { // if you add anything here, modify QCoreApplication::arguments()
        if (argv[i] && *argv[i] != '-') {
            argv[j++] = argv[i];
            continue;
        }
        QByteArray arg = argv[i];
        arg = arg;
        QString s;
        if (arg == "-qdevel" || arg == "-qdebug") {
            // obsolete argument
        } else if (arg.indexOf("-style=", 0) != -1) {
            s = QString::fromLocal8Bit(arg.right(arg.length() - 7).toLower());
        } else if (arg == "-style" && i < argc-1) {
            s = QString::fromLocal8Bit(argv[++i]).toLower();
#ifndef QT_NO_SESSIONMANAGER
        } else if (arg == "-session" && i < argc-1) {
            ++i;
            if (argv[i] && *argv[i]) {
                session_id = QString::fromLatin1(argv[i]);
                int p = session_id.indexOf(QLatin1Char('_'));
                if (p >= 0) {
                    session_key = session_id.mid(p +1);
                    session_id = session_id.left(p);
                }
                is_session_restored = true;
            }
#endif
#ifndef QT_NO_STYLE_STYLESHEET
        } else if (arg == "-stylesheet" && i < argc -1) {
            styleSheet = QLatin1String("file:///");
            styleSheet.append(QString::fromLocal8Bit(argv[++i]));
        } else if (arg.indexOf("-stylesheet=") != -1) {
            styleSheet = QLatin1String("file:///");
            styleSheet.append(QString::fromLocal8Bit(arg.right(arg.length() - 12)));
#endif
        } else if (qstrcmp(arg, "-reverse") == 0) {
            force_reverse = true;
            q->setLayoutDirection(Qt::RightToLeft);
        } else if (qstrcmp(arg, "-widgetcount") == 0) {
            widgetCount = true;
        } else if (arg == "-graphicssystem" && i < argc-1) {
            graphics_system_name = QString::fromLocal8Bit(argv[++i]);
        } else {
            argv[j++] = argv[i];
        }
        if (!s.isEmpty()) {
            if (app_style) {
                delete app_style;
                app_style = 0;
            }
            if (!styleOverride)
                styleOverride = new QString;
            *styleOverride = s;
        }
    }

    if(j < argc) {
        argv[j] = 0;
        argc = j;
    }
}

/*!
    Initializes the window system and constructs an application object with
    \a argc command line arguments in \a argv.

    \warning The data referred to by \a argc and \a argv must stay valid for
    the entire lifetime of the QApplication object. In addition, \a argc must
    be greater than zero and \a argv must contain at least one valid character
    string.

    The global \c qApp pointer refers to this application object. Only one
    application object should be created.

    This application object must be constructed before any \l{QPaintDevice}
    {paint devices} (including widgets, pixmaps, bitmaps etc.).

    \note \a argc and \a argv might be changed as Qt removes command line
    arguments that it recognizes.

    Qt debugging options (not available if Qt was compiled without the QT_DEBUG
    flag defined):
    \list
        \o  -nograb, tells Qt that it must never grab the mouse or the
            keyboard.
        \o  -dograb (only under X11), running under a debugger can cause an
            implicit -nograb, use -dograb to override.
        \o  -sync (only under X11), switches to synchronous mode for
            debugging.
    \endlist

    See \l{Debugging Techniques} for a more detailed explanation.

    All Qt programs automatically support the following command line options:
    \list
        \o  -style= \e style, sets the application GUI style. Possible values
            are \c motif, \c windows, and \c platinum. If you compiled Qt with
            additional styles or have additional styles as plugins these will
            be available to the \c -style command line option.
        \o  -style \e style, is the same as listed above.
        \o  -stylesheet= \e stylesheet, sets the application \l styleSheet. The
            value must be a path to a file that contains the Style Sheet.
            \note Relative URLs in the Style Sheet file are relative to the
            Style Sheet file's path.
        \o  -stylesheet \e stylesheet, is the same as listed above.
        \o  -session= \e session, restores the application from an earlier
            \l{Session Management}{session}.
        \o  -session \e session, is the same as listed above.
        \o  -widgetcount, prints debug message at the end about number of
            widgets left undestroyed and maximum number of widgets existed at
            the same time
        \o  -reverse, sets the application's layout direction to
            Qt::RightToLeft
        \o  -graphicssystem, sets the backend to be used for on-screen widgets
            and QPixmaps. Available options are \c{raster} and \c{opengl}.
    \endlist

    The Windows version of Qt supports an additional command line  option, if
    Direct3D support has been compiled into Qt:
    \list
        \o  -direct3d will make the Direct3D paint engine the default widget
            paint engine in Qt. \bold {This functionality is experimental.}
    \endlist

    The X11 version of Qt supports some traditional X11 command line options:
    \list
        \o  -display \e display, sets the X display (default is $DISPLAY).
        \o  -geometry \e geometry, sets the client geometry of the first window
            that is shown.
        \o  -fn or \c -font \e font, defines the application font. The font
            should be specified using an X logical font description.
        \o  -bg or \c -background \e color, sets the default background color
            and an application palette (light and dark shades are calculated).
        \o  -fg or \c -foreground \e color, sets the default foreground color.
        \o  -btn or \c -button \e color, sets the default button color.
        \o  -name \e name, sets the application name.
        \o  -title \e title, sets the application title.
        \o  -visual \c TrueColor, forces the application to use a TrueColor
            visual on an 8-bit display.
        \o  -ncols \e count, limits the number of colors allocated in the color
            cube on an 8-bit display, if the application is using the
            QApplication::ManyColor color specification. If \e count is 216
            then a 6x6x6 color cube is used (i.e. 6 levels of red, 6 of green,
            and 6 of blue); for other values, a cube approximately proportional
            to a 2x3x1 cube is used.
        \o  -cmap, causes the application to install a private color map on an
            8-bit display.
        \o  -im, sets the input method server (equivalent to setting the
            XMODIFIERS environment variable)
        \o  -inputstyle, defines how the input is inserted into the given
            widget, e.g., \c onTheSpot makes the input appear directly in the
            widget, while \c overTheSpot makes the input appear in a box
            floating over the widget and is not inserted until the editing is
            done.
    \endlist

    \sa arguments()
*/

QApplication::QApplication(int &argc, char **argv)
    : QCoreApplication(*new QApplicationPrivate(argc, argv, GuiClient))
{ Q_D(QApplication); d->construct(); }

QApplication::QApplication(int &argc, char **argv, int _internal)
    : QCoreApplication(*new QApplicationPrivate(argc, argv, GuiClient))
{ Q_D(QApplication); d->construct(); QApplicationPrivate::app_compile_version = _internal;}


/*!
    Constructs an application object with \a argc command line arguments in
    \a argv. If \a GUIenabled is true, a GUI application is constructed,
    otherwise a non-GUI (console) application is created.

    \warning The data referred to by \a argc and \a argv must stay valid for
    the entire lifetime of the QApplication object. In addition, \a argc must
    be greater than zero and \a argv must contain at least one valid character
    string.

    Set \a GUIenabled to false for programs without a graphical user interface
    that should be able to run without a window system.

    On X11, the window system is initialized if \a GUIenabled is true. If
    \a GUIenabled is false, the application does not connect to the X server.
    On Windows and Macintosh, currently the window system is always
    initialized, regardless of the value of GUIenabled. This may change in
    future versions of Qt.

    The following example shows how to create an application that uses a
    graphical interface when available.

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 0
*/

QApplication::QApplication(int &argc, char **argv, bool GUIenabled )
    : QCoreApplication(*new QApplicationPrivate(argc, argv, GUIenabled ? GuiClient : Tty))
{ Q_D(QApplication); d->construct(); }

QApplication::QApplication(int &argc, char **argv, bool GUIenabled , int _internal)
    : QCoreApplication(*new QApplicationPrivate(argc, argv, GUIenabled ? GuiClient : Tty))
{ Q_D(QApplication); d->construct();  QApplicationPrivate::app_compile_version = _internal;}



/*!
    Constructs an application object with \a argc command line arguments in
    \a argv.

    \warning The data referred to by \a argc and \a argv must stay valid for
    the entire lifetime of the QApplication object. In addition, \a argc must
    be greater than zero and \a argv must contain at least one valid character
    string.

    With Qt for Embedded Linux, passing QApplication::GuiServer for \a type
    makes this application the server (equivalent to running with the
    \c -qws option).
*/
QApplication::QApplication(int &argc, char **argv, Type type)
    : QCoreApplication(*new QApplicationPrivate(argc, argv, type))
{ Q_D(QApplication); d->construct(); }

QApplication::QApplication(int &argc, char **argv, Type type , int _internal)
    : QCoreApplication(*new QApplicationPrivate(argc, argv, type))
{ Q_D(QApplication); d->construct();  QApplicationPrivate::app_compile_version = _internal;}


/*!
    \internal
*/
void QApplicationPrivate::construct(
#ifdef Q_WS_X11
                                    Display *dpy, Qt::HANDLE visual, Qt::HANDLE cmap
#endif
                                    )
{
    initResources();

    qt_is_gui_used = (qt_appType != QApplication::Tty);
    process_cmdline();
    // Must be called before initialize()
    qt_init(this, qt_appType
#ifdef Q_WS_X11
            , dpy, visual, cmap
#endif
            );
    initialize();
    eventDispatcher->startingUp();

#ifdef QT_EVAL
    extern void qt_gui_eval_init(uint);
    qt_gui_eval_init(application_type);
#endif
}

#if defined(Q_WS_X11)
// ### a string literal is a cont char*
// ### using it as a char* is wrong and could lead to segfaults
// ### if aargv is modified someday
// ########## make it work with argc == argv == 0
static int aargc = 1;
static char *aargv[] = { (char*)"unknown", 0 };

/*!
    \fn QApplication::QApplication(Display* display, Qt::HANDLE visual, Qt::HANDLE colormap)

    Creates an application, given an already open display \a display. If
    \a visual and \a colormap are non-zero, the application will use those
    values as the default Visual and Colormap contexts.

    \warning Qt only supports TrueColor visuals at depths higher than 8
    bits-per-pixel.

    This function is only available on X11.
*/
QApplication::QApplication(Display* dpy, Qt::HANDLE visual, Qt::HANDLE colormap)
    : QCoreApplication(*new QApplicationPrivate(aargc, aargv, GuiClient))
{
    if (! dpy)
        qWarning("QApplication: Invalid Display* argument");
    Q_D(QApplication);
    d->construct(dpy, visual, colormap);
}

QApplication::QApplication(Display* dpy, Qt::HANDLE visual, Qt::HANDLE colormap, int _internal)
    : QCoreApplication(*new QApplicationPrivate(aargc, aargv, GuiClient))
{
    if (! dpy)
        qWarning("QApplication: Invalid Display* argument");
    Q_D(QApplication);
    d->construct(dpy, visual, colormap);
    QApplicationPrivate::app_compile_version = _internal;
}

/*!
    \fn QApplication::QApplication(Display *display, int &argc, char **argv,
        Qt::HANDLE visual, Qt::HANDLE colormap)

    Creates an application, given an already open \a display and using \a argc
    command line arguments in \a argv. If \a visual and \a colormap are
    non-zero, the application will use those values as the default Visual
    and Colormap contexts.

    \warning Qt only supports TrueColor visuals at depths higher than 8
    bits-per-pixel.

    This function is only available on X11.
*/
QApplication::QApplication(Display *dpy, int &argc, char **argv,
                           Qt::HANDLE visual, Qt::HANDLE colormap)
    : QCoreApplication(*new QApplicationPrivate(argc, argv, GuiClient))
{
    if (! dpy)
        qWarning("QApplication: Invalid Display* argument");
    Q_D(QApplication);
    d->construct(dpy, visual, colormap);
}

QApplication::QApplication(Display *dpy, int &argc, char **argv,
                           Qt::HANDLE visual, Qt::HANDLE colormap, int _internal)
    : QCoreApplication(*new QApplicationPrivate(argc, argv, GuiClient))
{
    if (! dpy)
        qWarning("QApplication: Invalid Display* argument");
    Q_D(QApplication);
    d->construct(dpy, visual, colormap);
    QApplicationPrivate::app_compile_version = _internal;
}

#endif // Q_WS_X11

extern void qInitDrawhelperAsm();

/*!
  \fn void QApplicationPrivate::initialize()

  Initializes the QApplication object, called from the constructors.
*/
void QApplicationPrivate::initialize()
{
    QWidgetPrivate::mapper = new QWidgetMapper;
    QWidgetPrivate::uncreatedWidgets = new QWidgetSet;
    if (qt_appType != QApplication::Tty)
        (void) QApplication::style();  // trigger creation of application style
    // trigger registering of QVariant's GUI types
    extern int qRegisterGuiVariant();
    qRegisterGuiVariant();

    is_app_running = true; // no longer starting up

    Q_Q(QApplication);
#ifndef QT_NO_SESSIONMANAGER
    // connect to the session manager
    session_manager = new QSessionManager(q, session_id, session_key);
#endif

    if (qgetenv("QT_USE_NATIVE_WINDOWS").toInt() > 0)
        q->setAttribute(Qt::AA_NativeWindows);

#if defined(Q_WS_WIN)
    // Alien is not currently working on Windows 98
    if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
        q->setAttribute(Qt::AA_NativeWindows);
#endif

#ifdef Q_OS_WINCE
#ifdef QT_AUTO_MAXIMIZE_THRESHOLD
    autoMaximizeThreshold = QT_AUTO_MAXIMIZE_THRESHOLD;
#else
    if (qt_wince_is_mobile())
        autoMaximizeThreshold = 50;
    else
        autoMaximizeThreshold = -1;
#endif //QT_AUTO_MAXIMIZE_THRESHOLD
#endif //Q_OS_WINCE

    // Set up which span functions should be used in raster engine...
    qInitDrawhelperAsm();

#if !defined(Q_WS_X11) && !defined(Q_WS_QWS)
    // initialize the graphics system - on X11 this is initialized inside
    // qt_init() in qapplication_x11.cpp because of several reasons.
    // On QWS, the graphics system is set by the QScreen plugin.
    graphics_system = QGraphicsSystemFactory::create(graphics_system_name);
#endif
#ifndef QT_NO_WHEELEVENT
#ifdef QT_MAC_USE_COCOA
    QApplicationPrivate::wheel_scroll_lines = 1;
#else
    QApplicationPrivate::wheel_scroll_lines = 3;
#endif
#endif
}

/*!
    Returns the type of application (\l Tty, GuiClient, or
    GuiServer). The type is set when constructing the QApplication
    object.
*/
QApplication::Type QApplication::type()
{
    return qt_appType;
}

/*****************************************************************************
  Functions returning the active popup and modal widgets.
 *****************************************************************************/

/*!
    Returns the active popup widget.

    A popup widget is a special top-level widget that sets the \c
    Qt::WType_Popup widget flag, e.g. the QMenu widget. When the application
    opens a popup widget, all events are sent to the popup. Normal widgets and
    modal widgets cannot be accessed before the popup widget is closed.

    Only other popup widgets may be opened when a popup widget is shown. The
    popup widgets are organized in a stack. This function returns the active
    popup widget at the top of the stack.

    \sa activeModalWidget(), topLevelWidgets()
*/

QWidget *QApplication::activePopupWidget()
{
    return QApplicationPrivate::popupWidgets && !QApplicationPrivate::popupWidgets->isEmpty() ?
        QApplicationPrivate::popupWidgets->last() : 0;
}


/*!
    Returns the active modal widget.

    A modal widget is a special top-level widget which is a subclass of QDialog
    that specifies the modal parameter of the constructor as true. A modal
    widget must be closed before the user can continue with other parts of the
    program.

    Modal widgets are organized in a stack. This function returns the active
    modal widget at the top of the stack.

    \sa activePopupWidget(), topLevelWidgets()
*/

QWidget *QApplication::activeModalWidget()
{
    return qt_modal_stack && !qt_modal_stack->isEmpty() ? qt_modal_stack->first() : 0;
}

/*!
    Cleans up any window system resources that were allocated by this
    application. Sets the global variable \c qApp to 0.
*/

QApplication::~QApplication()
{
    Q_D(QApplication);

#ifndef QT_NO_CLIPBOARD
    // flush clipboard contents
    if (qt_clipboard) {
        QEvent event(QEvent::Clipboard);
        QApplication::sendEvent(qt_clipboard, &event);
    }
#endif

    //### this should probable be done even later
    qt_call_post_routines();

    // kill timers before closing down the dispatcher
    d->toolTipWakeUp.stop();
    d->toolTipFallAsleep.stop();

    d->eventDispatcher->closingDown();
    d->eventDispatcher = 0;

    delete qt_desktopWidget;
    qt_desktopWidget = 0;
    QApplicationPrivate::is_app_closing = true;
    QApplicationPrivate::is_app_running = false;

#ifndef QT_NO_CLIPBOARD
    delete qt_clipboard;
    qt_clipboard = 0;
#endif

    // delete widget mapper
    if (QWidgetPrivate::mapper) {
        QWidgetMapper * myMapper = QWidgetPrivate::mapper;
        QWidgetPrivate::mapper = 0;
        for (QWidgetMapper::Iterator it = myMapper->begin(); it != myMapper->end(); ++it) {
            register QWidget *w = *it;
            if (!w->parent())                        // window
                w->destroy(true, true);
        }
        delete myMapper;
    }

    // delete uncreated widgets
    if (QWidgetPrivate::uncreatedWidgets) {
        QWidgetSet *mySet = QWidgetPrivate::uncreatedWidgets;
        QWidgetPrivate::uncreatedWidgets = 0;
        for (QWidgetSet::Iterator it = mySet->begin(); it != mySet->end(); ++it) {
            register QWidget *w = *it;
            if (!w->parent())                        // window
                w->destroy(true, true);
        }
        delete mySet;
    }

    delete QApplicationPrivate::app_pal;
    QApplicationPrivate::app_pal = 0;
    delete QApplicationPrivate::sys_pal;
    QApplicationPrivate::sys_pal = 0;
    delete QApplicationPrivate::set_pal;
    QApplicationPrivate::set_pal = 0;
    app_palettes()->clear();

    {
        QMutexLocker locker(applicationFontMutex());
        delete QApplicationPrivate::app_font;
        QApplicationPrivate::app_font = 0;
    }
    delete QApplicationPrivate::sys_font;
    QApplicationPrivate::sys_font = 0;
    delete QApplicationPrivate::set_font;
    QApplicationPrivate::set_font = 0;
    app_fonts()->clear();

    delete QApplicationPrivate::app_style;
    QApplicationPrivate::app_style = 0;
    delete QApplicationPrivate::app_icon;
    QApplicationPrivate::app_icon = 0;
#ifndef QT_NO_CURSOR
    d->cursor_list.clear();
#endif

#ifndef QT_NO_DRAGANDDROP
    if (qt_is_gui_used)
        delete QDragManager::self();
#endif

    qt_cleanup();

    if (QApplicationPrivate::widgetCount)
        qDebug("Widgets left: %i    Max widgets: %i \n", QWidgetPrivate::instanceCounter, QWidgetPrivate::maxInstances);
#ifndef QT_NO_SESSIONMANAGER
    delete d->session_manager;
    d->session_manager = 0;
#endif //QT_NO_SESSIONMANAGER

    QApplicationPrivate::obey_desktop_settings = true;
    QApplicationPrivate::cursor_flash_time = 1000;
    QApplicationPrivate::mouse_double_click_time = 400;
    QApplicationPrivate::keyboard_input_time = 400;

    drag_time = 500;
    drag_distance = 4;
    layout_direction = Qt::LeftToRight;
    QApplicationPrivate::app_strut = QSize(0, 0);
	QApplicationPrivate::voxToolTipDelayTime = 700;//VOXOX - CJC - 2009.11.05 
    QApplicationPrivate::animate_ui = true;
    QApplicationPrivate::animate_menu = false;
    QApplicationPrivate::fade_menu = false;
    QApplicationPrivate::animate_combo = false;
    QApplicationPrivate::animate_tooltip = false;
    QApplicationPrivate::fade_tooltip = false;
    QApplicationPrivate::widgetCount = false;

    // trigger unregistering of QVariant's GUI types
    extern int qUnregisterGuiVariant();
    qUnregisterGuiVariant();
}


/*!
    \fn QWidget *QApplication::widgetAt(const QPoint &point)

    Returns the widget at global screen position \a point, or 0 if there is no
    Qt widget there.

    This function can be slow.

    \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard()
*/
QWidget *QApplication::widgetAt(const QPoint &p)
{
    QWidget *window = QApplication::topLevelAt(p);
    if (!window)
        return 0;

    QWidget *child = 0;

    if (!window->testAttribute(Qt::WA_TransparentForMouseEvents))
        child = window->childAt(window->mapFromGlobal(p));

    if (child)
        return child;

    if (window->testAttribute(Qt::WA_TransparentForMouseEvents)) {
        //shoot a hole in the widget and try once again,
        //suboptimal on Qt for Embedded Linux where we do
        //know the stacking order of the toplevels.
        int x = p.x();
        int y = p.y();
        QRegion oldmask = window->mask();
        QPoint wpoint = window->mapFromGlobal(QPoint(x, y));
        QRegion newmask = (oldmask.isEmpty() ? QRegion(window->rect()) : oldmask)
                          - QRegion(wpoint.x(), wpoint.y(), 1, 1);
        window->setMask(newmask);
        QWidget *recurse = 0;
        if (QApplication::topLevelAt(p) != window) // verify recursion will terminate
            recurse = widgetAt(x, y);
        if (oldmask.isEmpty())
            window->clearMask();
        else
            window->setMask(oldmask);
        return recurse;
    }
    return window;
}

/*!
    \fn QWidget *QApplication::widgetAt(int x, int y)

    \overload

    Returns the widget at global screen position (\a x, \a y), or 0 if there is
    no Qt widget there.
*/

/*!
    \fn void QApplication::setArgs(int argc, char **argv)
    \internal
*/



/*!
    \internal
*/
bool QApplication::compressEvent(QEvent *event, QObject *receiver, QPostEventList *postedEvents)
{
    if ((event->type() == QEvent::UpdateRequest
#ifdef QT3_SUPPORT
          || event->type() == QEvent::LayoutHint
#endif
          || event->type() == QEvent::LayoutRequest
          || event->type() == QEvent::Resize
          || event->type() == QEvent::Move
          || event->type() == QEvent::LanguageChange
          || event->type() == QEvent::InputMethod)) {
        for (int i = 0; i < postedEvents->size(); ++i) {
            const QPostEvent &cur = postedEvents->at(i);
            if (cur.receiver != receiver || cur.event == 0 || cur.event->type() != event->type())
                continue;
            if (cur.event->type() == QEvent::LayoutRequest
#ifdef QT3_SUPPORT
                 || cur.event->type() == QEvent::LayoutHint
#endif
                 || cur.event->type() == QEvent::UpdateRequest) {
                ;
            } else if (cur.event->type() == QEvent::Resize) {
                ((QResizeEvent *)(cur.event))->s = ((QResizeEvent *)event)->s;
            } else if (cur.event->type() == QEvent::Move) {
                ((QMoveEvent *)(cur.event))->p = ((QMoveEvent *)event)->p;
            } else if (cur.event->type() == QEvent::LanguageChange) {
                ;
            } else if ( cur.event->type() == QEvent::InputMethod ) {
                *(QInputMethodEvent *)(cur.event) = *(QInputMethodEvent *)event;
            } else {
                continue;
            }
            delete event;
            return true;
        }
        return false;
    }
    return QCoreApplication::compressEvent(event, receiver, postedEvents);
}

/*!
    \property QApplication::styleSheet
    \brief the application style sheet
    \since 4.2

    By default, this property returns an empty string unless the user specifies
    the \c{-stylesheet} option on the command line when running the application.

    \sa QWidget::setStyle(), {Qt Style Sheets}
*/

/*!
    \property QApplication::autoMaximizeThreshold
    \since 4.4
    \brief defines a threshold for auto maximizing widgets

    The auto maximize threshold is only available as part of Qt for Windows CE.

    This property defines a threshold for the size of a window as a percentage
    of the screen size. If the minimum size hint of a window exceeds the
    threshold, calling show() will then cause the window to be maximized
    automatically.

    Setting the threshold to be 100 or greater means that it will cause it to
    always be maximized. Setting it to be 50 means that the widget is maximized
    if the vertical minimum size hint is at least 50% of the vertical screen
    size.

    If -1 is specified then this will disable the feature.

    On Windows CE the default is -1 (i.e. it is disabled).
    On Windows Mobile the default is 40.
*/

/*!
    \property QApplication::autoSipEnabled
    \since 4.5
    \brief toggles automatic SIP (software input panel) visibility

    The auto SIP property is only available as part of Qt for Windows CE.

    Set this property to true to automatically display the SIP when entering
    widgets that accept keyboard input. This property only affects widgets with
    the WA_InputMethodEnabled attribute set.
*/

#ifdef Q_OS_WINCE
void QApplication::setAutoMaximizeThreshold(const int threshold)
{
    QApplicationPrivate::autoMaximizeThreshold = threshold;
}

int QApplication::autoMaximizeThreshold() const
{
    return QApplicationPrivate::autoMaximizeThreshold;
}

void QApplication::setAutoSipEnabled(const bool enabled)
{
    QApplicationPrivate::autoSipEnabled = enabled;
}

bool QApplication::autoSipEnabled() const
{
    return QApplicationPrivate::autoSipEnabled;
}
#endif

#ifndef QT_NO_STYLE_STYLESHEET

QString QApplication::styleSheet() const
{
    return QApplicationPrivate::styleSheet;
}

void QApplication::setStyleSheet(const QString& styleSheet)
{
    QApplicationPrivate::styleSheet = styleSheet;
    QStyleSheetStyle *proxy = qobject_cast<QStyleSheetStyle*>(QApplicationPrivate::app_style);
    if (styleSheet.isEmpty()) { // application style sheet removed
        if (!proxy)
            return; // there was no stylesheet before
        setStyle(proxy->base);
    } else if (proxy) { // style sheet update, just repolish
        proxy->repolish(qApp);
    } else { // stylesheet set the first time
        QStyleSheetStyle *newProxy = new QStyleSheetStyle(QApplicationPrivate::app_style);
        QApplicationPrivate::app_style->setParent(newProxy);
        setStyle(newProxy);
    }
}

#endif // QT_NO_STYLE_STYLESHEET

/*!
    Returns the application's style object.

    \sa setStyle(), QStyle
*/
QStyle *QApplication::style()
{
    if (QApplicationPrivate::app_style)
        return QApplicationPrivate::app_style;
    if (!qt_is_gui_used) {
        Q_ASSERT(!"No style available in non-gui applications!");
        return 0;
    }

#if defined(Q_WS_X11)
    if(!QApplicationPrivate::styleOverride)
        QApplicationPrivate::x11_initialize_style(); // run-time search for default style
#endif
    if (!QApplicationPrivate::app_style) {
        // Compile-time search for default style
        //
        QString style;
        if (QApplicationPrivate::styleOverride) {
            style = *QApplicationPrivate::styleOverride;
            delete QApplicationPrivate::styleOverride;
            QApplicationPrivate::styleOverride = 0;
        } else {
#if defined(Q_WS_WIN) && defined(Q_OS_WINCE)
    if (qt_wince_is_smartphone() || qt_wince_is_pocket_pc())
        style = QLatin1String("WindowsMobile");
     else
        style = QLatin1String("WindowsCE");

#elif defined(Q_WS_WIN)
            if ((QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA
                && QSysInfo::WindowsVersion < QSysInfo::WV_NT_based))
                style = QLatin1String("WindowsVista");
                else if ((QSysInfo::WindowsVersion >= QSysInfo::WV_XP
                && QSysInfo::WindowsVersion < QSysInfo::WV_NT_based))
                style = QLatin1String("WindowsXP");
            else
                style = QLatin1String("Windows");                // default styles for Windows
#elif defined(Q_WS_X11) && defined(Q_OS_SOLARIS)
            style = QLatin1String("CDE");                        // default style for X11 on Solaris
#elif defined(Q_WS_X11) && defined(Q_OS_IRIX)
            style = QLatin1String("SGI");                        // default style for X11 on IRIX
#elif defined(Q_WS_X11) || defined(Q_WS_QWS)
            style = QLatin1String("Plastique");                  // default style for X11 and small devices
#elif defined(Q_WS_MAC)
                style = QLatin1String("Macintosh");              // default style for all Mac's
#endif
        }

        QStyle *&app_style = QApplicationPrivate::app_style;
        app_style = QStyleFactory::create(style);
        if (!app_style) {
            QStringList styles = QStyleFactory::keys();
            for (int i = 0; i < styles.size(); ++i) {
                if ((app_style = QStyleFactory::create(styles.at(i))))
                    break;
            }
        }
        if (!app_style) {
            Q_ASSERT(!"No styles available!");
            return 0;
        }
    }
    // take ownership of the style
    QApplicationPrivate::app_style->setParent(qApp);

    if (!QApplicationPrivate::sys_pal)
        QApplicationPrivate::setSystemPalette(QApplicationPrivate::app_style->standardPalette());
    if (QApplicationPrivate::set_pal) // repolish set palette with the new style
        QApplication::setPalette(*QApplicationPrivate::set_pal);

#ifndef QT_NO_STYLE_STYLESHEET
    if (!QApplicationPrivate::styleSheet.isEmpty()) {
        qApp->setStyleSheet(QApplicationPrivate::styleSheet);
    } else
#endif
        QApplicationPrivate::app_style->polish(qApp);

    return QApplicationPrivate::app_style;
}

/*!
    Sets the application's GUI style to \a style. Ownership of the style object
    is transferred to QApplication, so QApplication will delete the style
    object on application exit or when a new style is set and the old style is
    still the parent of the application object.

    Example usage:
    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 1

    When switching application styles, the color palette is set back to the
    initial colors or the system defaults. This is necessary since certain
    styles have to adapt the color palette to be fully style-guide compliant.

    Setting the style before a palette has been se, i.e., before creating
    QApplication, will cause the application to use QStyle::standardPalette()
    for the palette.

    \warning Qt style sheets are currently not supported for custom QStyle
    subclasses. We plan to address this in some future release.

    \sa style(), QStyle, setPalette(), desktopSettingsAware()
*/
void QApplication::setStyle(QStyle *style)
{
    if (!style || style == QApplicationPrivate::app_style)
        return;

    QWidgetList all = allWidgets();

    // clean up the old style
    if (QApplicationPrivate::app_style) {
        if (QApplicationPrivate::is_app_running && !QApplicationPrivate::is_app_closing) {
            for (QWidgetList::ConstIterator it = all.constBegin(); it != all.constEnd(); ++it) {
                register QWidget *w = *it;
                if (!(w->windowType() == Qt::Desktop) &&        // except desktop
                     w->testAttribute(Qt::WA_WState_Polished)) { // has been polished
                    QApplicationPrivate::app_style->unpolish(w);
                }
            }
        }
        QApplicationPrivate::app_style->unpolish(qApp);
    }

    QStyle *old = QApplicationPrivate::app_style; // save

#ifndef QT_NO_STYLE_STYLESHEET
    if (!QApplicationPrivate::styleSheet.isEmpty() && !qobject_cast<QStyleSheetStyle *>(style)) {
        // we have a stylesheet already and a new style is being set
        QStyleSheetStyle *newProxy = new QStyleSheetStyle(style);
        style->setParent(newProxy);
        QApplicationPrivate::app_style = newProxy;
    } else
#endif // QT_NO_STYLE_STYLESHEET
        QApplicationPrivate::app_style = style;

    QApplicationPrivate::app_style->setParent(qApp); // take ownership

    // take care of possible palette requirements of certain gui
    // styles. Do it before polishing the application since the style
    // might call QApplication::setPalette() itself
    if (QApplicationPrivate::set_pal) {
        QApplication::setPalette(*QApplicationPrivate::set_pal);
    } else if (QApplicationPrivate::sys_pal) {
        QApplicationPrivate::initializeWidgetPaletteHash();
        QApplicationPrivate::setPalette_helper(*QApplicationPrivate::sys_pal, /*className=*/0, /*clearWidgetPaletteHash=*/false);
    } else if (!QApplicationPrivate::sys_pal) {
        // Initialize the sys_pal if it hasn't happened yet...
        QApplicationPrivate::setSystemPalette(QApplicationPrivate::app_style->standardPalette());
    }

    // initialize the application with the new style
    QApplicationPrivate::app_style->polish(qApp);

    // re-polish existing widgets if necessary
    if (QApplicationPrivate::is_app_running && !QApplicationPrivate::is_app_closing) {
        for (QWidgetList::ConstIterator it1 = all.constBegin(); it1 != all.constEnd(); ++it1) {
            register QWidget *w = *it1;
            if (w->windowType() != Qt::Desktop && w->testAttribute(Qt::WA_WState_Polished)) {
                if (w->style() == QApplicationPrivate::app_style)
                    QApplicationPrivate::app_style->polish(w);                // repolish
#ifndef QT_NO_STYLE_STYLESHEET
                else
                    w->setStyleSheet(w->styleSheet()); // touch
#endif
            }
        }

        for (QWidgetList::ConstIterator it2 = all.constBegin(); it2 != all.constEnd(); ++it2) {
            register QWidget *w = *it2;
            if (w->windowType() != Qt::Desktop && !w->testAttribute(Qt::WA_SetStyle)) {
                    QEvent e(QEvent::StyleChange);
                    QApplication::sendEvent(w, &e);
#ifdef QT3_SUPPORT
                    if (old)
                        w->styleChange(*old);
#endif
                    w->update();
            }
        }
    }

#ifndef QT_NO_STYLE_STYLESHEET
    if (QStyleSheetStyle *oldProxy = qobject_cast<QStyleSheetStyle *>(old)) {
        oldProxy->deref();
    } else
#endif
    if (old && old->parent() == qApp) {
        delete old;
    }

    if (QApplicationPrivate::focus_widget) {
        QFocusEvent in(QEvent::FocusIn, Qt::OtherFocusReason);
        QApplication::sendEvent(QApplicationPrivate::focus_widget->style(), &in);
        QApplicationPrivate::focus_widget->update();
    }
}

/*!
    \overload

    Requests a QStyle object for \a style from the QStyleFactory.

    The string must be one of the QStyleFactory::keys(), typically one of
    "windows", "motif", "cde", "plastique", "windowsxp", or "macintosh". Style
    names are case insensitive.

    Returns 0 if an unknown \a style is passed, otherwise the QStyle object
    returned is set as the application's GUI style.

    \warning To ensure that the application's style is set correctly, it is
    best to call this function before the QApplication constructor, if
    possible.
*/
QStyle* QApplication::setStyle(const QString& style)
{
    QStyle *s = QStyleFactory::create(style);
    if (!s)
        return 0;

    setStyle(s);
    return s;
}

/*!
    \since 4.5

    Sets the default graphics backend to \a system, which will be used for
    on-screen widgets and QPixmaps. The available systems are \c{"native"},
    \c{"raster"} and \c{"opengl"}.

    This function call overrides both the application commandline
    \c{-graphicssystem} switch and the configure \c{-graphicssystem} switch.

    \warning This function must be called before the QApplication constructor
    is called.

    \note The \c{"opengl"} option is currently experimental.
*/

void QApplication::setGraphicsSystem(const QString &system)
{
    QApplicationPrivate::graphics_system_name = system;
}

/*!
    Returns the color specification.

    \sa QApplication::setColorSpec()
*/

int QApplication::colorSpec()
{
    return QApplicationPrivate::app_cspec;
}

/*!
    Sets the color specification for the application to \a spec.

    The color specification controls how the application allocates colors when
    run on a display with a limited amount of colors, e.g. 8 bit / 256 color
    displays.

    The color specification must be set before you create the QApplication
    object.

    The options are:
    \list
        \o  QApplication::NormalColor. This is the default color allocation
            strategy. Use this option if your application uses buttons, menus,
            texts and pixmaps with few colors. With this option, the
            application uses system global colors. This works fine for most
            applications under X11, but on Windows machines it may cause
            dithering of non-standard colors.
        \o  QApplication::CustomColor. Use this option if your application
            needs a small number of custom colors. On X11, this option is the
            same as NormalColor. On Windows, Qt creates a Windows palette, and
            allocates colors to it on demand.
        \o  QApplication::ManyColor. Use this option if your application is
            very color hungry, e.g., it requires thousands of colors. \br
            Under X11 the effect is:
            \list
                \o  For 256-color displays which have at best a 256 color true
                    color visual, the default visual is used, and colors are
                    allocated from a color cube. The color cube is the 6x6x6
                    (216 color) "Web palette" (the red, green, and blue
                    components always have one of the following values: 0x00,
                    0x33, 0x66, 0x99, 0xCC, or 0xFF), but the number of colors
                    can be changed by the \e -ncols option. The user can force
                    the application to use the true color visual with the
                    \l{QApplication::QApplication()}{-visual} option.
                \o  For 256-color displays which have a true color visual with
                    more than 256 colors, use that visual. Silicon Graphics X
                    servers this feature, for example. They provide an 8 bit
                    visual by default but can deliver true color when asked.
            \endlist
            On Windows, Qt creates a Windows palette, and fills it with a color
            cube.
    \endlist

    Be aware that the CustomColor and ManyColor choices may lead to colormap
    flashing: The foreground application gets (most) of the available colors,
    while the background windows will look less attractive.

    Example:

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 2

    \sa colorSpec()
*/

void QApplication::setColorSpec(int spec)
{
    if (qApp)
        qWarning("QApplication::setColorSpec: This function must be "
                 "called before the QApplication object is created");
    QApplicationPrivate::app_cspec = spec;
}

/*!
    \property QApplication::globalStrut
    \brief the minimum size that any GUI element that the user can interact
           with should have

    For example, no button should be resized to be smaller than the global
    strut size. The strut size should be considered when reimplementing GUI
    controls that may be used on touch-screens or similar I/O devices.

    Example:

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 3

    By default, this property contains a QSize object with zero width and height.
*/
QSize QApplication::globalStrut()
{
    return QApplicationPrivate::app_strut;
}

void QApplication::setGlobalStrut(const QSize& strut)
{
    QApplicationPrivate::app_strut = strut;
}

int QApplication::voxToolTipDelay()//VOXOX - CJC - 2009.11.05 
{
    return QApplicationPrivate::voxToolTipDelayTime;
}

void QApplication::setVoxToolTipDelay(int timeMilisecods)//VOXOX - CJC - 2009.11.05 
{
    QApplicationPrivate::voxToolTipDelayTime = timeMilisecods;
}

/*!
    Returns the application palette.

    \sa setPalette(), QWidget::palette()
*/
QPalette QApplication::palette()
{
    if (!QApplicationPrivate::app_pal)
        QApplicationPrivate::app_pal = new QPalette(Qt::black);
    return *QApplicationPrivate::app_pal;
}

/*!
    \fn QPalette QApplication::palette(const QWidget* widget)
    \overload

    If a \a widget is passed, the default palette for the widget's class is
    returned. This may or may not be the application palette. In most cases
    there is no special palette for certain types of widgets, but one notable
    exception is the popup menu under Windows, if the user has defined a
    special background color for menus in the display settings.

    \sa setPalette(), QWidget::palette()
*/
QPalette QApplication::palette(const QWidget* w)
{
    PaletteHash *hash = app_palettes();
    if (w && hash && hash->size()) {
        QHash<QByteArray, QPalette>::ConstIterator it = hash->constFind(w->metaObject()->className());
        if (it != hash->constEnd())
            return *it;
        for (it = hash->constBegin(); it != hash->constEnd(); ++it) {
            if (w->inherits(it.key()))
                return it.value();
        }
    }
    return palette();
}

/*!
    \overload

    Returns the palette for widgets of the given \a className.

    \sa setPalette(), QWidget::palette()
*/
QPalette QApplication::palette(const char *className)
{
    if (!QApplicationPrivate::app_pal)
        palette();
    PaletteHash *hash = app_palettes();
    if (className && hash && hash->size()) {
        QHash<QByteArray, QPalette>::ConstIterator it = hash->constFind(className);
        if (it != hash->constEnd())
            return *it;
    }
    return *QApplicationPrivate::app_pal;
}

void QApplicationPrivate::setPalette_helper(const QPalette &palette, const char* className, bool clearWidgetPaletteHash)
{
    QPalette pal = palette;

    if (QApplicationPrivate::app_style)
        QApplicationPrivate::app_style->polish(pal); // NB: non-const reference

    bool all = false;
    PaletteHash *hash = app_palettes();
    if (!className) {
        if (QApplicationPrivate::app_pal && pal.isCopyOf(*QApplicationPrivate::app_pal))
            return;
        if (!QApplicationPrivate::app_pal)
            QApplicationPrivate::app_pal = new QPalette(pal);
        else
            *QApplicationPrivate::app_pal = pal;
        if (hash && hash->size()) {
            all = true;
            if (clearWidgetPaletteHash)
                hash->clear();
        }
    } else if (hash) {
        hash->insert(className, pal);
    }

    if (QApplicationPrivate::is_app_running && !QApplicationPrivate::is_app_closing) {
        // Send ApplicationPaletteChange to qApp itself, and to the widgets.
        QEvent e(QEvent::ApplicationPaletteChange);
        QApplication::sendEvent(QApplication::instance(), &e);

        QWidgetList wids = QApplication::allWidgets();
        for (QWidgetList::ConstIterator it = wids.constBegin(); it != wids.constEnd(); ++it) {
            register QWidget *w = *it;
            if (all || (!className && w->isWindow()) || w->inherits(className)) // matching class
                QApplication::sendEvent(w, &e);
        }

        // Send to all scenes as well.
#ifndef QT_NO_GRAPHICSVIEW
        QList<QGraphicsScene *> &scenes = qApp->d_func()->scene_list;
        for (QList<QGraphicsScene *>::ConstIterator it = scenes.constBegin();
             it != scenes.constEnd(); ++it) {
            QApplication::sendEvent(*it, &e);
        }
#endif //QT_NO_GRAPHICSVIEW
    }
    if (!className && (!QApplicationPrivate::sys_pal || !palette.isCopyOf(*QApplicationPrivate::sys_pal))) {
        if (!QApplicationPrivate::set_pal)
            QApplicationPrivate::set_pal = new QPalette(palette);
        else
            *QApplicationPrivate::set_pal = palette;
    }
}

/*!
    Changes the default application palette to \a palette.

    If \a className is passed, the change applies only to widgets that inherit
    \a className (as reported by QObject::inherits()). If \a className is left
    0, the change affects all widgets, thus overriding any previously set class
    specific palettes.

    The palette may be changed according to the current GUI style in
    QStyle::polish().

    \warning Do not use this function in conjunction with \l{Qt Style Sheets}.
    When using style sheets, the palette of a widget can be customized using
    the "color", "background-color", "selection-color",
    "selection-background-color" and "alternate-background-color".

    \note Some styles do not use the palette for all drawing, for instance, if
    they make use of native theme engines. This is the case for the Windows XP,
    Windows Vista, and Mac OS X styles.

    \sa QWidget::setPalette(), palette(), QStyle::polish()
*/

void QApplication::setPalette(const QPalette &palette, const char* className)
{
    QApplicationPrivate::setPalette_helper(palette, className, /*clearWidgetPaletteHash=*/ true);
}



void QApplicationPrivate::setSystemPalette(const QPalette &pal)
{
    QPalette adjusted;

#if 0
    // adjust the system palette to avoid dithering
    QColormap cmap = QColormap::instance();
    if (cmap.depths() > 4 && cmap.depths() < 24) {
        for (int g = 0; g < QPalette::NColorGroups; g++)
            for (int i = 0; i < QPalette::NColorRoles; i++) {
                QColor color = pal.color((QPalette::ColorGroup)g, (QPalette::ColorRole)i);
                color = cmap.colorAt(cmap.pixel(color));
                adjusted.setColor((QPalette::ColorGroup)g, (QPalette::ColorRole) i, color);
            }
    }
#else
    adjusted = pal;
#endif

    if (!sys_pal)
        sys_pal = new QPalette(adjusted);
    else
        *sys_pal = adjusted;


    if (!QApplicationPrivate::set_pal)
        QApplication::setPalette(*sys_pal);
}

/*!
    Returns the default application font.

    \sa fontMetrics(), QWidget::font()
*/
QFont QApplication::font()
{
    QMutexLocker locker(applicationFontMutex());
    if (!QApplicationPrivate::app_font)
        QApplicationPrivate::app_font = new QFont(QLatin1String("Helvetica"));
    return *QApplicationPrivate::app_font;
}

/*!
    \overload

    Returns the default font for the \a widget.

    \sa fontMetrics(), QWidget::setFont()
*/

QFont QApplication::font(const QWidget *widget)
{
    FontHash *hash = app_fonts();

#ifdef Q_WS_MAC
    // short circuit for small and mini controls
    if (widget->testAttribute(Qt::WA_MacSmallSize)) {
        return hash->value("QSmallFont");
    } else if (widget->testAttribute(Qt::WA_MacMiniSize)) {
        return hash->value("QMiniFont");
    }
#endif
    if (widget && hash  && hash->size()) {
        QHash<QByteArray, QFont>::ConstIterator it =
                hash->constFind(widget->metaObject()->className());
        if (it != hash->constEnd())
            return it.value();
        for (it = hash->constBegin(); it != hash->constEnd(); ++it) {
            if (widget->inherits(it.key()))
                return it.value();
        }
    }
    return font();
}

/*!
    \overload

    Returns the font for widgets of the given \a className.

    \sa setFont(), QWidget::font()
*/
QFont QApplication::font(const char *className)
{
    FontHash *hash = app_fonts();
    if (className && hash && hash->size()) {
        QHash<QByteArray, QFont>::ConstIterator it = hash->constFind(className);
        if (it != hash->constEnd())
            return *it;
    }
    return font();
}


/*!
    Changes the default application font to \a font. If \a className is passed,
    the change applies only to classes that inherit \a className (as reported
    by QObject::inherits()).

    On application start-up, the default font depends on the window system. It
    can vary depending on both the window system version and the locale. This
    function lets you override the default font; but overriding may be a bad
    idea because, for example, some locales need extra large fonts to support
    their special characters.

    \warning Do not use this function in conjunction with \l{Qt Style Sheets}.
    The font of an application can be customized using the "font" style sheet
    property. To set a bold font for all QPushButtons, set the application
    styleSheet() as "QPushButton { font: bold }"

    \sa font(), fontMetrics(), QWidget::setFont()
*/

void QApplication::setFont(const QFont &font, const char *className)
{
    bool all = false;
    FontHash *hash = app_fonts();
    if (!className) {
        QMutexLocker locker(applicationFontMutex());
        if (!QApplicationPrivate::app_font)
            QApplicationPrivate::app_font = new QFont(font);
        else
            *QApplicationPrivate::app_font = font;
        if (hash && hash->size()) {
            all = true;
            hash->clear();
        }
    } else if (hash) {
        hash->insert(className, font);
    }
    if (QApplicationPrivate::is_app_running && !QApplicationPrivate::is_app_closing) {
        // Send ApplicationFontChange to qApp itself, and to the widgets.
        QEvent e(QEvent::ApplicationFontChange);
        QApplication::sendEvent(QApplication::instance(), &e);

        QWidgetList wids = QApplication::allWidgets();
        for (QWidgetList::ConstIterator it = wids.constBegin(); it != wids.constEnd(); ++it) {
            register QWidget *w = *it;
            if (all || (!className && w->isWindow()) || w->inherits(className)) // matching class
                sendEvent(w, &e);
        }

#ifndef QT_NO_GRAPHICSVIEW
        // Send to all scenes as well.
        QList<QGraphicsScene *> &scenes = qApp->d_func()->scene_list;
        for (QList<QGraphicsScene *>::ConstIterator it = scenes.constBegin();
             it != scenes.constEnd(); ++it) {
            QApplication::sendEvent(*it, &e);
        }
#endif //QT_NO_GRAPHICSVIEW
    }
    if (!className && (!QApplicationPrivate::sys_font || !font.isCopyOf(*QApplicationPrivate::sys_font))) {
        if (!QApplicationPrivate::set_font)
            QApplicationPrivate::set_font = new QFont(font);
        else
            *QApplicationPrivate::set_font = font;
    }
}

/*! \internal
*/
void QApplicationPrivate::setSystemFont(const QFont &font)
{
     if (!sys_font)
        sys_font = new QFont(font);
    else
        *sys_font = font;

    if (!QApplicationPrivate::set_font)
        QApplication::setFont(*sys_font);
}

/*!
    \property QApplication::windowIcon
    \brief the default window icon

    \sa QWidget::setWindowIcon(), {Setting the Application Icon}
*/
QIcon QApplication::windowIcon()
{
    return QApplicationPrivate::app_icon ? *QApplicationPrivate::app_icon : QIcon();
}

void QApplication::setWindowIcon(const QIcon &icon)
{
    if (!QApplicationPrivate::app_icon)
        QApplicationPrivate::app_icon = new QIcon();
    *QApplicationPrivate::app_icon = icon;
    if (QApplicationPrivate::is_app_running && !QApplicationPrivate::is_app_closing) {
#ifdef Q_WS_MAC
        void qt_mac_set_app_icon(const QPixmap &); //qapplication_mac.cpp
        QSize size = QApplicationPrivate::app_icon->actualSize(QSize(128, 128));
        qt_mac_set_app_icon(QApplicationPrivate::app_icon->pixmap(size));
#endif
        QEvent e(QEvent::ApplicationWindowIconChange);
        QWidgetList all = QApplication::allWidgets();
        for (QWidgetList::ConstIterator it = all.constBegin(); it != all.constEnd(); ++it) {
            register QWidget *w = *it;
            if (w->isWindow())
                sendEvent(w, &e);
        }
    }
}

/*!
    Returns a list of the top-level widgets (windows) in the application.

    \note Some of the top-level widgets may be hidden, for example a tooltip if
    no tooltip is currently shown.

    Example:

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 4

    \sa allWidgets(), QWidget::isWindow(), QWidget::isHidden()
*/
QWidgetList QApplication::topLevelWidgets()
{
    QWidgetList list;
    QWidgetList all = allWidgets();

    for (QWidgetList::ConstIterator it = all.constBegin(); it != all.constEnd(); ++it) {
        QWidget *w = *it;
        if (w->isWindow() && w->windowType() != Qt::Desktop)
            list.append(w);
    }
    return list;
}

/*!
    Returns a list of all the widgets in the application.

    The list is empty (QList::isEmpty()) if there are no widgets.

    \note Some of the widgets may be hidden.

    Example:
    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 5

    \sa topLevelWidgets(), QWidget::isVisible()
*/

QWidgetList QApplication::allWidgets()
{
    QWidgetList list;
    if (QWidgetPrivate::mapper)
        list += QWidgetPrivate::mapper->values();
    if (QWidgetPrivate::uncreatedWidgets)
        list += QWidgetPrivate::uncreatedWidgets->toList();
    return list;
}

/*!
    Returns the application widget that has the keyboard input focus, or 0 if
    no widget in this application has the focus.

    \sa QWidget::setFocus(), QWidget::hasFocus(), activeWindow(), focusChanged()
*/

QWidget *QApplication::focusWidget()
{
    return QApplicationPrivate::focus_widget;
}

void QApplicationPrivate::setFocusWidget(QWidget *focus, Qt::FocusReason reason)
{
#ifndef QT_NO_GRAPHICSVIEW
    if (focus && focus->window()->graphicsProxyWidget())
        return;
#endif

    hidden_focus_widget = 0;

    if (focus != focus_widget) {
        if (focus && focus->isHidden()) {
            hidden_focus_widget = focus;
            return;
        }

        if (focus && (reason == Qt::BacktabFocusReason || reason == Qt::TabFocusReason)
            && qt_in_tab_key_event)
            focus->window()->setAttribute(Qt::WA_KeyboardFocusChange);
        else if (focus && reason == Qt::ShortcutFocusReason) {
            focus->window()->setAttribute(Qt::WA_KeyboardFocusChange);
        }
        QWidget *prev = focus_widget;
        focus_widget = focus;

        if (prev && ((reason != Qt::PopupFocusReason && reason != Qt::MenuBarFocusReason
            && prev->testAttribute(Qt::WA_InputMethodEnabled))
            // Do reset the input context, in case the new focus widget won't accept keyboard input
            // or it is not created fully yet.
            || (focus_widget && (!focus_widget->testAttribute(Qt::WA_InputMethodEnabled)
            || !focus_widget->testAttribute(Qt::WA_WState_Created))))) {
             QInputContext *qic = prev->inputContext();
            if(qic) {
                qic->reset();
                qic->setFocusWidget(0);
            }
        }

        if(focus_widget)
            focus_widget->d_func()->setFocus_sys();

        if (reason != Qt::NoFocusReason) {

            //send events
            if (prev) {
#ifdef QT_KEYPAD_NAVIGATION
                if (QApplication::keypadNavigationEnabled()) {
                    if (prev->hasEditFocus() && reason != Qt::PopupFocusReason)
                        prev->setEditFocus(false);
                }
#endif
                QFocusEvent out(QEvent::FocusOut, reason);
                QPointer<QWidget> that = prev;
                QApplication::sendEvent(prev, &out);
                if (that)
                    QApplication::sendEvent(that->style(), &out);
            }
            if(focus && QApplicationPrivate::focus_widget == focus) {
                if (focus->testAttribute(Qt::WA_InputMethodEnabled)) {
                    QInputContext *qic = focus->inputContext();
                    if (qic && focus_widget->testAttribute(Qt::WA_WState_Created))
                        qic->setFocusWidget( focus_widget );
                }
                QFocusEvent in(QEvent::FocusIn, reason);
                QPointer<QWidget> that = focus;
                QApplication::sendEvent(focus, &in);
                if (that)
                    QApplication::sendEvent(that->style(), &in);
            }
            emit qApp->focusChanged(prev, focus_widget);
        }
    }
}


/*!
    Returns the application top-level window that has the keyboard input focus,
    or 0 if no application window has the focus. There might be an
    activeWindow() even if there is no focusWidget(), for example if no widget
    in that window accepts key events.

    \sa QWidget::setFocus(), QWidget::hasFocus(), focusWidget()
*/

QWidget *QApplication::activeWindow()
{
    return QApplicationPrivate::active_window;
}

/*!
    Returns display (screen) font metrics for the application font.

    \sa font(), setFont(), QWidget::fontMetrics(), QPainter::fontMetrics()
*/

QFontMetrics QApplication::fontMetrics()
{
    return desktop()->fontMetrics();
}


/*!
    Closes all top-level windows.

    This function is particularly useful for applications with many top-level
    windows. It could, for example, be connected to a \gui{Exit} entry in the
    \gui{File} menu:

    \snippet examples/mainwindows/mdi/mainwindow.cpp 0

    The windows are closed in random order, until one window does not accept
    the close event. The application quits when the last window was
    successfully closed; this can be turned off by setting
    \l quitOnLastWindowClosed to false.

    \sa quitOnLastWindowClosed, lastWindowClosed(), QWidget::close(),
    QWidget::closeEvent(), lastWindowClosed(), quit(), topLevelWidgets(),
    QWidget::isWindow()
*/
void QApplication::closeAllWindows()
{
    bool did_close = true;
    QWidget *w;
    while((w = activeModalWidget()) && did_close) {
        if(!w->isVisible())
            break;
        did_close = w->close();
    }
    QWidgetList list = QApplication::topLevelWidgets();
    for (int i = 0; did_close && i < list.size(); ++i) {
        w = list.at(i);
        if (w->isVisible() && w->windowType() != Qt::Desktop) {
            did_close = w->close();
            list = QApplication::topLevelWidgets();
            i = -1;
        }
    }
}

/*!
    Displays a simple message box about Qt. The message includes the version
    number of Qt being used by the application.

    This is useful for inclusion in the \gui Help menu of an application, as
    shown in the \l{mainwindows/menus}{Menus} example.

    This function is a convenience slot for QMessageBox::aboutQt().
*/
void QApplication::aboutQt()
{
#ifndef QT_NO_MESSAGEBOX
    QMessageBox::aboutQt(
#ifdef Q_WS_MAC
            0
#else
            activeWindow()
#endif // Q_WS_MAC
            );
#endif // QT_NO_MESSAGEBOX
}


/*!
    \fn void QApplication::lastWindowClosed()

    This signal is emitted from QApplication::exec() when the last visible
    primary window (i.e. window with no parent) with the Qt::WA_QuitOnClose
    attribute set is closed.

    By default,

    \list
        \o  this attribute is set for all widgets except transient windows such
            as splash screens, tool windows, and popup menus

        \o  QApplication implicitly quits when this signal is emitted.
    \endlist

    This feature can be turned off by setting \l quitOnLastWindowClosed to
    false.

    \sa QWidget::close()
*/

/*!
    \since 4.1
    \fn void QApplication::focusChanged(QWidget *old, QWidget *now)

    This signal is emitted when the widget that has keyboard focus changed from
    \a old to \a now, i.e., because the user pressed the tab-key, clicked into
    a widget or changed the active window. Both \a old and \a now can be the
    null-pointer.

    The signal is emitted after both widget have been notified about the change
    through QFocusEvent.

    \sa QWidget::setFocus(), QWidget::clearFocus(), Qt::FocusReason
*/

/*!
    \since 4.5
    \fn void QApplication::fontDatabaseChanged()

    This signal is emitted when application fonts are loaded or removed.

    \sa QFontDatabase::addApplicationFont(),
    QFontDatabase::addApplicationFontFromData(),
    QFontDatabase::removeAllApplicationFonts(),
    QFontDatabase::removeApplicationFont()
*/

#ifndef QT_NO_TRANSLATION
static bool qt_detectRTLLanguage()
{
    return force_reverse ^
        (QApplication::tr("QT_LAYOUT_DIRECTION",
                         "Translate this string to the string 'LTR' in left-to-right"
                         " languages or to 'RTL' in right-to-left languages (such as Hebrew"
                         " and Arabic) to get proper widget layout.") == QLatin1String("RTL"));
}
#endif

/*!\reimp

*/
bool QApplication::event(QEvent *e)
{
    Q_D(QApplication);
    if(e->type() == QEvent::Close) {
        QCloseEvent *ce = static_cast<QCloseEvent*>(e);
        ce->accept();
        closeAllWindows();

        QWidgetList list = topLevelWidgets();
        for (int i = 0; i < list.size(); ++i) {
            QWidget *w = list.at(i);
            if (w->isVisible() && !(w->windowType() == Qt::Desktop) && !(w->windowType() == Qt::Popup) &&
                 (!(w->windowType() == Qt::Dialog) || !w->parentWidget())) {
                ce->ignore();
                break;
            }
        }
        if(ce->isAccepted())
            return true;
    } else if(e->type() == QEvent::LanguageChange) {
#ifndef QT_NO_TRANSLATION
        setLayoutDirection(qt_detectRTLLanguage()?Qt::RightToLeft:Qt::LeftToRight);
#endif
        QWidgetList list = topLevelWidgets();
        for (int i = 0; i < list.size(); ++i) {
            QWidget *w = list.at(i);
            if (!(w->windowType() == Qt::Desktop))
                postEvent(w, new QEvent(QEvent::LanguageChange));
        }
    } else if (e->type() == QEvent::Timer) {
        QTimerEvent *te = static_cast<QTimerEvent*>(e);
        Q_ASSERT(te != 0);
        if (te->timerId() == d->toolTipWakeUp.timerId()) {
            d->toolTipWakeUp.stop();
            if (d->toolTipWidget) {
                QWidget *w = d->toolTipWidget->window();
                // show tooltip if WA_AlwaysShowToolTips is set, or if
                // any ancestor of d->toolTipWidget is the active
                // window
                bool showToolTip = w->testAttribute(Qt::WA_AlwaysShowToolTips);
                while (w && !showToolTip) {
                    showToolTip = w->isActiveWindow();
                    w = w->parentWidget();
                    w = w ? w->window() : 0;
                }
                if (showToolTip) {
                    QHelpEvent e(QEvent::ToolTip, d->toolTipPos, d->toolTipGlobalPos);
                    QApplication::sendEvent(d->toolTipWidget, &e);
                    if (e.isAccepted())
                        d->toolTipFallAsleep.start(2000, this);
                }
            }
        } else if (te->timerId() == d->toolTipFallAsleep.timerId()) {
            d->toolTipFallAsleep.stop();
        }
#ifdef QT_MAC_USE_COCOA
    } else if (e->type() == QEvent::CocoaRequestModal) {
        d->_q_runAppModalWindow();
#endif
    }
    return QCoreApplication::event(e);
}
#if !defined(Q_WS_X11)

// The doc and X implementation of this function is in qapplication_x11.cpp

void QApplication::syncX()        {}                // do nothing

#endif

/*!
    \fn Qt::WindowsVersion QApplication::winVersion()

    Use \l QSysInfo::WindowsVersion instead.
*/

/*!
    \fn void QApplication::setActiveWindow(QWidget* active)

    Sets the active window to the \a active widget in response to a system
    event. The function is called from the platform specific event handlers.

    \warning This function does \e not set the keyboard focus to the active
    widget. Call QWidget::activateWindow() instead.

    It sets the activeWindow() and focusWidget() attributes and sends proper
    \l{QEvent::WindowActivate}{WindowActivate}/\l{QEvent::WindowDeactivate}
    {WindowDeactivate} and \l{QEvent::FocusIn}{FocusIn}/\l{QEvent::FocusOut}
    {FocusOut} events to all appropriate widgets. The window will then be
    painted in active state (e.g. cursors in line edits will blink), and it
    will have tool tips enabled.

    \sa activeWindow(), QWidget::activateWindow()
*/
void QApplication::setActiveWindow(QWidget* act)
{
    QWidget* window = act?act->window():0;

    if (QApplicationPrivate::active_window == window)
        return;

#ifndef QT_NO_GRAPHICSVIEW
    if (window && window->graphicsProxyWidget()) {
        // Activate the proxy's view->viewport() ?
        return;
    }
#endif

    QWidgetList toBeActivated;
    QWidgetList toBeDeactivated;

    if (QApplicationPrivate::active_window) {
        if (style()->styleHint(QStyle::SH_Widget_ShareActivation, 0, QApplicationPrivate::active_window)) {
            QWidgetList list = topLevelWidgets();
            for (int i = 0; i < list.size(); ++i) {
                QWidget *w = list.at(i);
                if (w->isVisible() && w->isActiveWindow())
                    toBeDeactivated.append(w);
            }
        } else {
            toBeDeactivated.append(QApplicationPrivate::active_window);
        }
    }

#if !defined(Q_WS_MAC)
    QWidget *previousActiveWindow =  QApplicationPrivate::active_window;
#endif
    QApplicationPrivate::active_window = window;

    if (QApplicationPrivate::active_window) {
        if (style()->styleHint(QStyle::SH_Widget_ShareActivation, 0, QApplicationPrivate::active_window)) {
            QWidgetList list = topLevelWidgets();
            for (int i = 0; i < list.size(); ++i) {
                QWidget *w = list.at(i);
                if (w->isVisible() && w->isActiveWindow())
                    toBeActivated.append(w);
            }
        } else {
            toBeActivated.append(QApplicationPrivate::active_window);
        }

    }

    // first the activation/deactivation events
    QEvent activationChange(QEvent::ActivationChange);
    QEvent windowActivate(QEvent::WindowActivate);
    QEvent windowDeactivate(QEvent::WindowDeactivate);

#if !defined(Q_WS_MAC)
    if (!previousActiveWindow) {
        QEvent appActivate(QEvent::ApplicationActivate);
        sendSpontaneousEvent(qApp, &appActivate);
    }
#endif

    for (int i = 0; i < toBeActivated.size(); ++i) {
        QWidget *w = toBeActivated.at(i);
        sendSpontaneousEvent(w, &windowActivate);
        sendSpontaneousEvent(w, &activationChange);
    }

    for(int i = 0; i < toBeDeactivated.size(); ++i) {
        QWidget *w = toBeDeactivated.at(i);
        sendSpontaneousEvent(w, &windowDeactivate);
        sendSpontaneousEvent(w, &activationChange);
    }

#if !defined(Q_WS_MAC)
    if (!QApplicationPrivate::active_window) {
        QEvent appDeactivate(QEvent::ApplicationDeactivate);
        sendSpontaneousEvent(qApp, &appDeactivate);
    }
#endif

    if (QApplicationPrivate::popupWidgets == 0) { // !inPopupMode()
        // then focus events
        if (!QApplicationPrivate::active_window && QApplicationPrivate::focus_widget) {
            QApplicationPrivate::setFocusWidget(0, Qt::ActiveWindowFocusReason);
        } else if (QApplicationPrivate::active_window) {
            QWidget *w = QApplicationPrivate::active_window->focusWidget();
            if (w && w->isVisible() /*&& w->focusPolicy() != QWidget::NoFocus*/)
                w->setFocus(Qt::ActiveWindowFocusReason);
            else {
                w = QApplicationPrivate::focusNextPrevChild_helper(QApplicationPrivate::active_window, true);
                if (w) {
                    w->setFocus(Qt::ActiveWindowFocusReason);
                } else {
                    // If the focus widget is not in the activate_window, clear the focus
                    w = QApplicationPrivate::focus_widget;
                    if (!w && QApplicationPrivate::active_window->focusPolicy() != Qt::NoFocus)
                        QApplicationPrivate::setFocusWidget(QApplicationPrivate::active_window, Qt::ActiveWindowFocusReason);
                    else if (!QApplicationPrivate::active_window->isAncestorOf(w))
                        QApplicationPrivate::setFocusWidget(0, Qt::ActiveWindowFocusReason);
                }
            }
        }
    }
}

/*!internal
 * Helper function that returns the new focus widget, but does not set the focus reason.
 * Returns 0 if a new focus widget could not be found.
*/
QWidget *QApplicationPrivate::focusNextPrevChild_helper(QWidget *toplevel, bool next)
{
    uint focus_flag = qt_tab_all_widgets ? Qt::TabFocus : Qt::StrongFocus;

    QWidget *f = toplevel->focusWidget();
    if (!f)
        f = toplevel;

    QWidget *w = f;
    QWidget *test = f->d_func()->focus_next;
    while (test && test != f) {
        if ((test->focusPolicy() & focus_flag) == focus_flag
            && !(test->d_func()->extra && test->d_func()->extra->focus_proxy)
            && test->isVisibleTo(toplevel) && test->isEnabled()
            && !(w->windowType() == Qt::SubWindow && !w->isAncestorOf(test))
            && (toplevel->windowType() != Qt::SubWindow || toplevel->isAncestorOf(test))) {
            w = test;
            if (next)
                break;
        }
        test = test->d_func()->focus_next;
    }
    if (w == f) {
        if (qt_in_tab_key_event) {
            w->window()->setAttribute(Qt::WA_KeyboardFocusChange);
            w->update();
        }
        return 0;
    }
    return w;
}

/*!
    \fn void QApplicationPrivate::dispatchEnterLeave(QWidget* enter, QWidget* leave)
    \internal

    Creates the proper Enter/Leave event when widget \a enter is entered and
    widget \a leave is left.
 */
#if defined(Q_WS_WIN)
  extern void qt_win_set_cursor(QWidget *, bool);
#elif defined(Q_WS_X11)
  extern void qt_x11_enforce_cursor(QWidget *, bool);
#endif

void QApplicationPrivate::dispatchEnterLeave(QWidget* enter, QWidget* leave) {
#if 0
    if (leave) {
        QEvent e(QEvent::Leave);
        QApplication::sendEvent(leave, & e);
    }
    if (enter) {
        QEvent e(QEvent::Enter);
        QApplication::sendEvent(enter, & e);
    }
    return;
#endif

    QWidget* w ;
    if ((!enter && !leave) || (enter == leave))
        return;
#ifdef ALIEN_DEBUG
    qDebug() << "QApplicationPrivate::dispatchEnterLeave, ENTER:" << enter << "LEAVE:" << leave;
#endif
    QWidgetList leaveList;
    QWidgetList enterList;

    bool sameWindow = leave && enter && leave->window() == enter->window();
    if (leave && !sameWindow) {
        w = leave;
        do {
            leaveList.append(w);
        } while (!w->isWindow() && (w = w->parentWidget()));
    }
    if (enter && !sameWindow) {
        w = enter;
        do {
            enterList.prepend(w);
        } while (!w->isWindow() && (w = w->parentWidget()));
    }
    if (sameWindow) {
        int enterDepth = 0;
        int leaveDepth = 0;
        w = enter;
        while (!w->isWindow() && (w = w->parentWidget()))
            enterDepth++;
        w = leave;
        while (!w->isWindow() && (w = w->parentWidget()))
            leaveDepth++;
        QWidget* wenter = enter;
        QWidget* wleave = leave;
        while (enterDepth > leaveDepth) {
            wenter = wenter->parentWidget();
            enterDepth--;
        }
        while (leaveDepth > enterDepth) {
            wleave = wleave->parentWidget();
            leaveDepth--;
        }
        while (!wenter->isWindow() && wenter != wleave) {
            wenter = wenter->parentWidget();
            wleave = wleave->parentWidget();
        }

        w = leave;
        while (w != wleave) {
            leaveList.append(w);
            w = w->parentWidget();
        }
        w = enter;
        while (w != wenter) {
            enterList.prepend(w);
            w = w->parentWidget();
        }
    }

    QEvent leaveEvent(QEvent::Leave);
    for (int i = 0; i < leaveList.size(); ++i) {
        w = leaveList.at(i);
        if (!qApp->activeModalWidget() || QApplicationPrivate::tryModalHelper(w, 0)) {
#if defined(Q_WS_WIN) || defined(Q_WS_X11)
            if (leaveAfterRelease == w)
                leaveAfterRelease = 0;
#endif
            QApplication::sendEvent(w, &leaveEvent);
            if (w->testAttribute(Qt::WA_Hover) &&
                (!qApp->activePopupWidget() || qApp->activePopupWidget() == w->window())) {
                Q_ASSERT(instance());
                QHoverEvent he(QEvent::HoverLeave, QPoint(-1, -1), w->mapFromGlobal(QApplicationPrivate::instance()->hoverGlobalPos));
                qApp->d_func()->notify_helper(w, &he);
            }
        }
    }
    QPoint posEnter = QCursor::pos();
    QEvent enterEvent(QEvent::Enter);
    for (int i = 0; i < enterList.size(); ++i) {
        w = enterList.at(i);
        if (!qApp->activeModalWidget() || QApplicationPrivate::tryModalHelper(w, 0)) {
            QApplication::sendEvent(w, &enterEvent);
            if (w->testAttribute(Qt::WA_Hover) &&
                (!qApp->activePopupWidget() || qApp->activePopupWidget() == w->window())) {
                QHoverEvent he(QEvent::HoverEnter, w->mapFromGlobal(posEnter), QPoint(-1, -1));
                qApp->d_func()->notify_helper(w, &he);
            }
        }
    }

#ifndef QT_NO_CURSOR
    // Update cursor for alien/graphics widgets.

    const bool enterOnAlien = (enter && (isAlien(enter) || enter->testAttribute(Qt::WA_DontShowOnScreen)));
#if defined(Q_WS_X11)
    //Whenever we leave an alien widget on X11, we need to reset its nativeParentWidget()'s cursor.
    // This is not required on Windows as the cursor is reset on every single mouse move.
    QWidget *parentOfLeavingCursor = 0;
    for (int i = 0; i < leaveList.size(); ++i) {
        w = leaveList.at(i);
        if (!isAlien(w))
            break;
        if (w->testAttribute(Qt::WA_SetCursor)) {
            parentOfLeavingCursor = w->parentWidget();
            //continue looping, we need to find the downest alien widget with a cursor.
            // (downest on the screen)
        }
    }
    //check that we will not call qt_x11_enforce_cursor twice with the same native widget
    if (parentOfLeavingCursor && (!enterOnAlien
        || parentOfLeavingCursor->effectiveWinId() != enter->effectiveWinId())) {
#ifndef QT_NO_GRAPHICSVIEW
        if (!parentOfLeavingCursor->window()->graphicsProxyWidget())
#endif
        {
            qt_x11_enforce_cursor(parentOfLeavingCursor,true);
        }
    }
#endif
    if (enterOnAlien) {
        QWidget *cursorWidget = enter;
        while (!cursorWidget->isWindow() && !cursorWidget->isEnabled())
            cursorWidget = cursorWidget->parentWidget();

        if (!cursorWidget)
            return;

#ifndef QT_NO_GRAPHICSVIEW
        if (cursorWidget->window()->graphicsProxyWidget()) {
            QWidgetPrivate::nearestGraphicsProxyWidget(cursorWidget)->setCursor(cursorWidget->cursor());
        } else
#endif
        {
#if defined(Q_WS_WIN)
            qt_win_set_cursor(cursorWidget, true);
#elif defined(Q_WS_X11)
            qt_x11_enforce_cursor(cursorWidget, true);
#endif
        }
    }
#endif
}

/* exported for the benefit of testing tools */
Q_GUI_EXPORT bool qt_tryModalHelper(QWidget *widget, QWidget **rettop)
{
    return QApplicationPrivate::tryModalHelper(widget, rettop);
}

/*! \internal
    Returns true if \a widget is blocked by a modal window.
 */
bool QApplicationPrivate::isBlockedByModal(QWidget *widget)
{
    widget = widget->window();
    if (!modalState())
        return false;
    if (qApp->activePopupWidget() == widget)
        return false;

    for (int i = 0; i < qt_modal_stack->size(); ++i) {
        QWidget *modalWidget = qt_modal_stack->at(i);

        {
            // check if the active modal widget is our widget or a parent of our widget
            QWidget *w = widget;
            while (w) {
                if (w == modalWidget)
                    return false;
                w = w->parentWidget();
            }
#ifdef Q_WS_WIN
            if ((widget->testAttribute(Qt::WA_WState_Created) || widget->data->winid)
                && (modalWidget->testAttribute(Qt::WA_WState_Created) || modalWidget->data->winid)
                && IsChild(modalWidget->data->winid, widget->data->winid))
                return false;
#endif
        }

        Qt::WindowModality windowModality = modalWidget->windowModality();
        if (windowModality == Qt::NonModal) {
            // determine the modality type if it hasn't been set on the
            // modalWidget, this normally happens when waiting for a
            // native dialog. use WindowModal if we are the child of a
            // group leader; otherwise use ApplicationModal.
            QWidget *m = modalWidget;
            while (m && !m->testAttribute(Qt::WA_GroupLeader)) {
                m = m->parentWidget();
                if (m)
                    m = m->window();
            }
            windowModality = (m && m->testAttribute(Qt::WA_GroupLeader))
                             ? Qt::WindowModal
                             : Qt::ApplicationModal;
        }

        switch (windowModality) {
        case Qt::ApplicationModal:
            {
                QWidget *groupLeaderForWidget = widget;
                while (groupLeaderForWidget && !groupLeaderForWidget->testAttribute(Qt::WA_GroupLeader))
                    groupLeaderForWidget = groupLeaderForWidget->parentWidget();

                if (groupLeaderForWidget) {
                    // if \a widget has WA_GroupLeader, it can only be blocked by ApplicationModal children
                    QWidget *m = modalWidget;
                    while (m && m != groupLeaderForWidget && !m->testAttribute(Qt::WA_GroupLeader))
                        m = m->parentWidget();
                    if (m == groupLeaderForWidget)
                        return true;
                } else if (modalWidget != widget) {
                    return true;
                }
                break;
            }
        case Qt::WindowModal:
            {
                QWidget *w = widget;
                do {
                    QWidget *m = modalWidget;
                    do {
                        if (m == w)
                            return true;
                        m = m->parentWidget();
                        if (m)
                            m = m->window();
                    } while (m);
                    w = w->parentWidget();
                    if (w)
                        w = w->window();
                } while (w);
                break;
            }
        default:
            Q_ASSERT_X(false, "QApplication", "internal error, a modal widget cannot be modeless");
            break;
        }
    }
    return false;
}

/*!\internal
 */
void QApplicationPrivate::enterModal(QWidget *widget)
{
    QSet<QWidget*> blocked;
    QList<QWidget*> windows = qApp->topLevelWidgets();
    for (int i = 0; i < windows.count(); ++i) {
        QWidget *window = windows.at(i);
        if (window->windowType() != Qt::Tool && isBlockedByModal(window))
            blocked.insert(window);
    }

    enterModal_sys(widget);

    windows = qApp->topLevelWidgets();
    QEvent e(QEvent::WindowBlocked);
    for (int i = 0; i < windows.count(); ++i) {
        QWidget *window = windows.at(i);
        if (!blocked.contains(window) && window->windowType() != Qt::Tool && isBlockedByModal(window))
            QApplication::sendEvent(window, &e);
    }
}

/*!\internal
 */
void QApplicationPrivate::leaveModal(QWidget *widget)
{
    QSet<QWidget*> blocked;
    QList<QWidget*> windows = qApp->topLevelWidgets();
    for (int i = 0; i < windows.count(); ++i) {
        QWidget *window = windows.at(i);
        if (window->windowType() != Qt::Tool && isBlockedByModal(window))
            blocked.insert(window);
    }

    leaveModal_sys(widget);

    windows = qApp->topLevelWidgets();
    QEvent e(QEvent::WindowUnblocked);
    for (int i = 0; i < windows.count(); ++i) {
        QWidget *window = windows.at(i);
        if(blocked.contains(window) && window->windowType() != Qt::Tool && !isBlockedByModal(window))
            QApplication::sendEvent(window, &e);
    }
}



/*!\internal

  Called from qapplication_\e{platform}.cpp, returns true
  if the widget should accept the event.
 */
bool QApplicationPrivate::tryModalHelper(QWidget *widget, QWidget **rettop)
{
    QWidget *top = QApplication::activeModalWidget();
    if (rettop)
        *rettop = top;

    // the active popup widget always gets the input event
    if (qApp->activePopupWidget())
        return true;

#if defined(Q_WS_MAC) && defined(QT_MAC_USE_COCOA)
    top = QApplicationPrivate::tryModalHelper_sys(top);
    if (rettop)
        *rettop = top;
#endif

    return !isBlockedByModal(widget->window());
}

/*
   \internal
*/
QWidget *QApplicationPrivate::pickMouseReceiver(QWidget *candidate, const QPoint &globalPos,
                                                QPoint &pos, QEvent::Type type,
                                                Qt::MouseButtons buttons, QWidget *buttonDown,
                                                QWidget *alienWidget)
{
    Q_ASSERT(candidate);

    QWidget *mouseGrabber = QWidget::mouseGrabber();
    if (((type == QEvent::MouseMove && buttons) || (type == QEvent::MouseButtonRelease))
            && !buttonDown && !mouseGrabber) {
        return 0;
    }

    if (alienWidget && alienWidget->internalWinId())
        alienWidget = 0;

    QWidget *receiver = candidate;

    if (!mouseGrabber)
        mouseGrabber = buttonDown ? buttonDown : alienWidget;

    if (mouseGrabber && mouseGrabber != candidate) {
        receiver = mouseGrabber;
        pos = receiver->mapFromGlobal(globalPos);
#ifdef ALIEN_DEBUG
        qDebug() << "  ** receiver adjusted to:" << receiver << "pos:" << pos;
#endif
    }

    return receiver;

}

/*
   \internal
*/
bool QApplicationPrivate::sendMouseEvent(QWidget *receiver, QMouseEvent *event,
                                         QWidget *alienWidget, QWidget *nativeWidget,
                                         QWidget **buttonDown, QPointer<QWidget> &lastMouseReceiver)
{
    Q_ASSERT(receiver);
    Q_ASSERT(event);
    Q_ASSERT(nativeWidget);
    Q_ASSERT(buttonDown);

    if (alienWidget && !isAlien(alienWidget))
        alienWidget = 0;

    QPointer<QWidget> receiverGuard = receiver;
    QPointer<QWidget> nativeGuard = nativeWidget;
    QPointer<QWidget> alienGuard = alienWidget;
    QPointer<QWidget> activePopupWidget = qApp->activePopupWidget();

    const bool graphicsWidget = nativeWidget->testAttribute(Qt::WA_DontShowOnScreen);

    if (*buttonDown) {
        if (!graphicsWidget) {
            // Register the widget that shall receive a leave event
            // after the last button is released.
            if ((alienWidget || !receiver->internalWinId()) && !leaveAfterRelease && !QWidget::mouseGrabber())
                leaveAfterRelease = *buttonDown;
            if (event->type() == QEvent::MouseButtonRelease && !event->buttons())
                *buttonDown = 0;
        }
    } else if (lastMouseReceiver) {
        // Dispatch enter/leave if we move:
        // 1) from an alien widget to another alien widget or
        //    from a native widget to an alien widget (first OR case)
        // 2) from an alien widget to a native widget (second OR case)
        if ((alienWidget && alienWidget != lastMouseReceiver)
            || (isAlien(lastMouseReceiver) && !alienWidget)) {
            if (activePopupWidget) {
                if (!QWidget::mouseGrabber())
                    dispatchEnterLeave(alienWidget ? alienWidget : nativeWidget, lastMouseReceiver);
            } else {
                dispatchEnterLeave(receiver, lastMouseReceiver);
            }

        }
    }

#ifdef ALIEN_DEBUG
    qDebug() << "QApplicationPrivate::sendMouseEvent: receiver:" << receiver
             << "pos:" << event->pos() << "alien" << alienWidget << "button down"
             << *buttonDown << "last" << lastMouseReceiver << "leave after release"
             << leaveAfterRelease;
#endif

    // We need this quard in case someone opens a modal dialog / popup. If that's the case
    // leaveAfterRelease is set to null, but we shall not update lastMouseReceiver.
    const bool wasLeaveAfterRelease = leaveAfterRelease != 0;
    bool result = QApplication::sendSpontaneousEvent(receiver, event);

    if (!graphicsWidget && leaveAfterRelease && event->type() == QEvent::MouseButtonRelease
        && !event->buttons() && QWidget::mouseGrabber() != leaveAfterRelease) {
        // Dispatch enter/leave if:
        // 1) the mouse grabber is an alien widget
        // 2) the button is released on an alien widget

        QWidget *enter = 0;
        if (nativeGuard)
            enter = alienGuard ? alienWidget : nativeWidget;
        else // The receiver is typically deleted on mouse release with drag'n'drop.
            enter = QApplication::widgetAt(event->globalPos());

        dispatchEnterLeave(enter, leaveAfterRelease);
        leaveAfterRelease = 0;
        lastMouseReceiver = enter;
    } else if (!wasLeaveAfterRelease) {
        if (activePopupWidget) {
            if (!QWidget::mouseGrabber())
                lastMouseReceiver = alienGuard ? alienWidget : (nativeGuard ? nativeWidget : 0);
        } else {
            lastMouseReceiver = receiverGuard ? receiver : QApplication::widgetAt(event->globalPos());
        }
    }

    return result;
}

#if defined(Q_WS_WIN) || defined(Q_WS_X11)
/*
    This function should only be called when the widget changes visibility, i.e.
    when the \a widget is shown, hidden or deleted. This function does nothing
    if the widget is a top-level or native, i.e. not an alien widget. In that
    case enter/leave events are genereated by the underlying windowing system.
*/
extern QPointer<QWidget> qt_last_mouse_receiver;
extern QWidget *qt_button_down;
void QApplicationPrivate::sendSyntheticEnterLeave(QWidget *widget)
{
#ifndef QT_NO_CURSOR
    if (!widget || widget->internalWinId() || widget->isWindow())
        return;

    const bool widgetInShow = widget->isVisible() && !widget->data->in_destructor;
    if (!widgetInShow && widget != qt_last_mouse_receiver)
        return; // Widget was not under the cursor when it was hidden/deleted.

    if (widgetInShow && widget->parentWidget()->data->in_show)
        return; // Ingore recursive show.

    QWidget *mouseGrabber = QWidget::mouseGrabber();
    if (mouseGrabber && mouseGrabber != widget)
        return; // Someone else has the grab; enter/leave should not occur.

    QWidget *tlw = widget->window();
    if (tlw->data->in_destructor || tlw->data->is_closing)
        return; // Closing down the business.

    if (widgetInShow && (!qt_last_mouse_receiver || qt_last_mouse_receiver->window() != tlw))
        return; // Mouse cursor not inside the widget's top-level.

    const QPoint globalPos(QCursor::pos());
    QPoint pos = tlw->mapFromGlobal(globalPos);

    // Find the current widget under the mouse. If this function was called from
    // the widget's destructor, we have to make sure childAt() doesn't take into
    // account widgets that are about to be destructed.
    QWidget *widgetUnderCursor = tlw->d_func()->childAt_helper(pos, widget->data->in_destructor);
    if (!widgetUnderCursor)
        widgetUnderCursor = tlw;
    else
        pos = widgetUnderCursor->mapFrom(tlw, pos);

    if (widgetInShow && widgetUnderCursor != widget && !widget->isAncestorOf(widgetUnderCursor))
        return; // Mouse cursor not inside the widget or any of its children.

    if (widget->data->in_destructor && qt_button_down == widget)
        qt_button_down = 0;

    // Send enter/leave events followed by a mouse move on the entered widget.
    QMouseEvent e(QEvent::MouseMove, pos, globalPos, Qt::NoButton, mouse_buttons, modifier_buttons);
    sendMouseEvent(widgetUnderCursor, &e, widgetUnderCursor, tlw, &qt_button_down, qt_last_mouse_receiver);
#endif // QT_NO_CURSOR
}
#endif // Q_WS_WIN || Q_WS_X11

/*!
    Returns the desktop widget (also called the root window).

    The desktop may be composed of multiple screens, so it would be incorrect,
    for example, to attempt to \e center some widget in the desktop's geometry.
    QDesktopWidget has various functions for obtaining useful geometries upon
    the desktop, such as QDesktopWidget::screenGeometry() and
    QDesktopWidget::availableGeometry().

    On X11, it is also possible to draw on the desktop.
*/
QDesktopWidget *QApplication::desktop()
{
    if (!qt_desktopWidget || // not created yet
         !(qt_desktopWidget->windowType() == Qt::Desktop)) { // reparented away
        qt_desktopWidget = new QDesktopWidget();
    }
    return qt_desktopWidget;
}

#ifndef QT_NO_CLIPBOARD
/*!
    Returns a pointer to the application global clipboard.

    \note The QApplication object should already be constructed before
    accessing the clipboard.
*/
QClipboard *QApplication::clipboard()
{
    if (qt_clipboard == 0) {
        if (!qApp) {
            qWarning("QApplication: Must construct a QApplication before accessing a QClipboard");
            return 0;
        }
        qt_clipboard = new QClipboard(0);
    }
    return qt_clipboard;
}
#endif // QT_NO_CLIPBOARD

/*!
    Sets whether Qt should use the system's standard colors, fonts, etc., to
    \a on. By default, this is true.

    This function must be called before creating the QApplication object, like
    this:

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 6

    \sa desktopSettingsAware()
*/
void QApplication::setDesktopSettingsAware(bool on)
{
    QApplicationPrivate::obey_desktop_settings = on;
}

/*!
    Returns true if Qt is set to use the system's standard colors, fonts, etc.;
    otherwise returns false. The default is true.

    \sa setDesktopSettingsAware()
*/
bool QApplication::desktopSettingsAware()
{
    return QApplicationPrivate::obey_desktop_settings;
}

/*!
    Returns the current state of the modifier keys on the keyboard. The current
    state is updated sychronously as the event queue is emptied of events that
    will spontaneously change the keyboard state (QEvent::KeyPress and
    QEvent::KeyRelease events).

    It should be noted this may not reflect the actual keys held on the input
    device at the time of calling but rather the modifiers as last reported in
    one of the above events. If no keys are being held Qt::NoModifier is
    returned.

    \sa mouseButtons()
*/

Qt::KeyboardModifiers QApplication::keyboardModifiers()
{
    return QApplicationPrivate::modifier_buttons;
}

/*!
    Returns the current state of the buttons on the mouse. The current state is
    updated syncronously as the event queue is emptied of events that will
    spontaneously change the mouse state (QEvent::MouseButtonPress and
    QEvent::MouseButtonRelease events).

    It should be noted this may not reflect the actual buttons held on the
    input device at the time of calling but rather the mouse buttons as last
    reported in one of the above events. If no mouse buttons are being held
    Qt::NoButton is returned.

    \sa keyboardModifiers()
*/

Qt::MouseButtons QApplication::mouseButtons()
{
    return QApplicationPrivate::mouse_buttons;
}

/*!
    \fn bool QApplication::isSessionRestored() const

    Returns true if the application has been restored from an earlier
    \l{Session Management}{session}; otherwise returns false.

    \sa sessionId(), commitData(), saveState()
*/


/*!
    \fn QString QApplication::sessionId() const

    Returns the current \l{Session Management}{session's} identifier.

    If the application has been restored from an earlier session, this
    identifier is the same as it was in that previous session. The session
    identifier is guaranteed to be unique both for different applications
    and for different instances of the same application.

    \sa isSessionRestored(), sessionKey(), commitData(), saveState()
*/

/*!
    \fn QString QApplication::sessionKey() const

    Returns the session key in the current \l{Session Management}{session}.

    If the application has been restored from an earlier session, this key is
    the same as it was when the previous session ended.

    The session key changes with every call of commitData() or saveState().

    \sa isSessionRestored(), sessionId(), commitData(), saveState()
*/
#ifndef QT_NO_SESSIONMANAGER
bool QApplication::isSessionRestored() const
{
    Q_D(const QApplication);
    return d->is_session_restored;
}

QString QApplication::sessionId() const
{
    Q_D(const QApplication);
    return d->session_id;
}

QString QApplication::sessionKey() const
{
    Q_D(const QApplication);
    return d->session_key;
}
#endif



/*!
    \since 4.2
    \fn void QApplication::commitDataRequest(QSessionManager &manager)

    This signal deals with \l{Session Management}{session management}. It is
    emitted when the QSessionManager wants the application to commit all its
    data.

    Usually this means saving all open files, after getting permission from
    the user. Furthermore you may want to provide a means by which the user
    can cancel the shutdown.

    You should not exit the application within this signal. Instead,
    the session manager may or may not do this afterwards, depending on the
    context.

    \warning Within this signal, no user interaction is possible, \e
    unless you ask the \a manager for explicit permission. See
    QSessionManager::allowsInteraction() and
    QSessionManager::allowsErrorInteraction() for details and example
    usage.

    \note You should use Qt::DirectConnection when connecting to this signal.

    \sa isSessionRestored(), sessionId(), saveState(), {Session Management}
*/

/*!
    This function deals with \l{Session Management}{session management}. It is
    invoked when the QSessionManager wants the application to commit all its
    data.

    Usually this means saving all open files, after getting permission from the
    user. Furthermore you may want to provide a means by which the user can
    cancel the shutdown.

    You should not exit the application within this function. Instead, the
    session manager may or may not do this afterwards, depending on the
    context.

    \warning Within this function, no user interaction is possible, \e
    unless you ask the \a manager for explicit permission. See
    QSessionManager::allowsInteraction() and
    QSessionManager::allowsErrorInteraction() for details and example
    usage.

    The default implementation requests interaction and sends a close event to
    all visible top-level widgets. If any event was rejected, the shutdown is
    canceled.

    \sa isSessionRestored(), sessionId(), saveState(), {Session Management}
*/
#ifndef QT_NO_SESSIONMANAGER
void QApplication::commitData(QSessionManager& manager )
{
    emit commitDataRequest(manager);
    if (manager.allowsInteraction()) {
        QWidgetList done;
        QWidgetList list = QApplication::topLevelWidgets();
        bool cancelled = false;
        for (int i = 0; !cancelled && i < list.size(); ++i) {
            QWidget* w = list.at(i);
            if (w->isVisible() && !done.contains(w)) {
                cancelled = !w->close();
                if (!cancelled)
                    done.append(w);
                list = QApplication::topLevelWidgets();
                i = -1;
            }
        }
        if (cancelled)
            manager.cancel();
    }
}

/*!
    \since 4.2
    \fn void QApplication::saveStateRequest(QSessionManager &manager)

    This signal deals with \l{Session Management}{session management}. It is
    invoked when the \l{QSessionManager}{session manager} wants the application
    to preserve its state for a future session.

    For example, a text editor would create a temporary file that includes the
    current contents of its edit buffers, the location of the cursor and other
    aspects of the current editing session.

    You should never exit the application within this signal. Instead, the
    session manager may or may not do this afterwards, depending on the
    context. Futhermore, most session managers will very likely request a saved
    state immediately after the application has been started. This permits the
    session manager to learn about the application's restart policy.

    \warning Within this function, no user interaction is possible, \e
    unless you ask the \a manager for explicit permission. See
    QSessionManager::allowsInteraction() and
    QSessionManager::allowsErrorInteraction() for details.

    \note You should use Qt::DirectConnection when connecting to this signal.

    \sa isSessionRestored(), sessionId(), commitData(), {Session Management}
*/

/*!
    This function deals with \l{Session Management}{session management}. It is
    invoked when the \l{QSessionManager}{session manager} wants the application
    to preserve its state for a future session.

    For example, a text editor would create a temporary file that includes the
    current contents of its edit buffers, the location of the cursor and other
    aspects of the current editing session.

    You should never exit the application within this function. Instead, the
    session manager may or may not do this afterwards, depending on the
    context. Futhermore, most session managers will very likely request a saved
    state immediately after the application has been started. This permits the
    session manager to learn about the application's restart policy.

    \warning Within this function, no user interaction is possible, \e
    unless you ask the \a manager for explicit permission. See
    QSessionManager::allowsInteraction() and
    QSessionManager::allowsErrorInteraction() for details.

    \sa isSessionRestored(), sessionId(), commitData(), {Session Management}
*/

void QApplication::saveState(QSessionManager &manager)
{
    emit saveStateRequest(manager);
}
#endif //QT_NO_SESSIONMANAGER
/*
  Sets the time after which a drag should start to \a ms ms.

  \sa startDragTime()
*/

void QApplication::setStartDragTime(int ms)
{
    drag_time = ms;
}

/*!
    \property QApplication::startDragTime
    \brief the time in milliseconds that a mouse button must be held down
    before a drag and drop operation will begin

    If you support drag and drop in your application, and want to start a drag
    and drop operation after the user has held down a mouse button for a
    certain amount of time, you should use this property's value as the delay.

    Qt also uses this delay internally, e.g. in QTextEdit and QLineEdit, for
    starting a drag.

    The default value is 500 ms.

    \sa startDragDistance(), {Drag and Drop}
*/

int QApplication::startDragTime()
{
    return drag_time;
}

/*
    Sets the distance after which a drag should start to \a l pixels.

    \sa startDragDistance()
*/

void QApplication::setStartDragDistance(int l)
{
    drag_distance = l;
}

/*!
    \property QApplication::startDragDistance

    If you support drag and drop in your application, and want to start a drag
    and drop operation after the user has moved the cursor a certain distance
    with a button held down, you should use this property's value as the
    minimum distance required.

    For example, if the mouse position of the click is stored in \c startPos
    and the current position (e.g. in the mouse move event) is \c currentPos,
    you can find out if a drag should be started with code like this:

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 7

    Qt uses this value internally, e.g. in QFileDialog.

    The default value is 4 pixels.

    \sa startDragTime() QPoint::manhattanLength() {Drag and Drop}
*/

int QApplication::startDragDistance()
{
    return drag_distance;
}

/*!
    \fn void QApplication::setReverseLayout(bool reverse)

    Use setLayoutDirection() instead.
*/

/*!
    \fn void QApplication::reverseLayout()

    Use layoutDirection() instead.
*/

/*!
    \fn bool QApplication::isRightToLeft()

    Returns true if the application's layout direction is
    Qt::RightToLeft; otherwise returns false.

    \sa layoutDirection(), isLeftToRight()
*/

/*!
    \fn bool QApplication::isLeftToRight()

    Returns true if the application's layout direction is
    Qt::LeftToRight; otherwise returns false.

    \sa layoutDirection(), isRightToLeft()
*/

/*!
    \property QApplication::layoutDirection
    \brief the default layout direction for this application

    On system start-up, the default layout direction depends on the
    application's language.

    \sa QWidget::layoutDirection, isLeftToRight(), isRightToLeft()
 */

void QApplication::setLayoutDirection(Qt::LayoutDirection direction)
{
    if (layout_direction == direction)
        return;

    layout_direction = direction;

    QWidgetList list = topLevelWidgets();
    for (int i = 0; i < list.size(); ++i) {
        QWidget *w = list.at(i);
        QEvent ev(QEvent::ApplicationLayoutDirectionChange);
        sendEvent(w, &ev);
    }
}

Qt::LayoutDirection QApplication::layoutDirection()
{
    return layout_direction;
}


/*!
    \obsolete

    Strips out vertical alignment flags and transforms an alignment \a align
    of Qt::AlignLeft into Qt::AlignLeft or Qt::AlignRight according to the
    language used.
*/

#ifdef QT3_SUPPORT
Qt::Alignment QApplication::horizontalAlignment(Qt::Alignment align)
{
    return QStyle::visualAlignment(layoutDirection(), align);
}
#endif


/*!
    \fn QCursor *QApplication::overrideCursor()

    Returns the active application override cursor.

    This function returns 0 if no application cursor has been defined (i.e. the
    internal cursor stack is empty).

    \sa setOverrideCursor(), restoreOverrideCursor()
*/
#ifndef QT_NO_CURSOR
QCursor *QApplication::overrideCursor()
{
    return qApp->d_func()->cursor_list.isEmpty() ? 0 : &qApp->d_func()->cursor_list.first();
}

/*!
    Changes the currently active application override cursor to \a cursor.

    This function has no effect if setOverrideCursor() was not called.

    \sa setOverrideCursor(), overrideCursor(), restoreOverrideCursor(),
    QWidget::setCursor()
 */
void QApplication::changeOverrideCursor(const QCursor &cursor)
{
    if (qApp->d_func()->cursor_list.isEmpty())
        return;
    qApp->d_func()->cursor_list.removeFirst();
    setOverrideCursor(cursor);
}
#endif

/*!
    \fn void QApplication::setOverrideCursor(const QCursor &cursor, bool replace)

    Use changeOverrideCursor(\a cursor) (if \a replace is true) or
    setOverrideCursor(\a cursor) (if \a replace is false).
*/

/*!
    Enters the main event loop and waits until exit() is called, then returns
    the value that was set to exit() (which is 0 if exit() is called via
    quit()).

    It is necessary to call this function to start event handling. The main
    event loop receives events from the window system and dispatches these to
    the application widgets.
 
    Generally, no user interaction can take place before calling exec(). As a
    special case, modal widgets like QMessageBox can be used before calling
    exec(), because modal widgets call exec() to start a local event loop.

    To make your application perform idle processing, i.e., executing a special
    function whenever there are no pending events, use a QTimer with 0 timeout.
    More advanced idle processing schemes can be achieved using processEvents().

    We recommend that you connect clean-up code to the
    \l{QCoreApplication::}{aboutToQuit()} signal, instead of putting it in your
    application's \c{main()} function because on some platforms the
    QApplication::exec() call may not return. For example, on Windows when the
    user logs off, the system terminates the process after Qt closes all
    top-level windows. Hence, there is no guarantee that the application will
    have time to exit its event loop and execute code at the end of the
    \c{main()} function after the QApplication::exec() call.

    \sa quitOnLastWindowClosed, quit(), exit(), processEvents(),
        QCoreApplication::exec()
*/
int QApplication::exec()
{
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::setRootObject(qApp);
#endif
    return QCoreApplication::exec();
}

/*! \reimp
 */
bool QApplication::notify(QObject *receiver, QEvent *e)
{
    Q_D(QApplication);
    // no events are delivered after ~QCoreApplication() has started
    if (QApplicationPrivate::is_app_closing)
        return true;

    if (receiver == 0) {                        // serious error
        qWarning("QApplication::notify: Unexpected null receiver");
        return true;
    }

#ifndef QT_NO_DEBUG
    d->checkReceiverThread(receiver);
#endif

#ifdef QT3_SUPPORT
    if (e->type() == QEvent::ChildRemoved && !receiver->d_func()->pendingChildInsertedEvents.isEmpty())
        receiver->d_func()->removePendingChildInsertedEvents(static_cast<QChildEvent *>(e)->child());
#endif // QT3_SUPPORT

    // capture the current mouse/keyboard state
    if(e->spontaneous()) {
        if (e->type() == QEvent::KeyPress
            || e->type() == QEvent::KeyRelease) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(e);
            QApplicationPrivate::modifier_buttons = ke->modifiers();
        } else if(e->type() == QEvent::MouseButtonPress
            || e->type() == QEvent::MouseButtonRelease) {
                QMouseEvent *me = static_cast<QMouseEvent*>(e);
                QApplicationPrivate::modifier_buttons = me->modifiers();
                if(me->type() == QEvent::MouseButtonPress)
                    QApplicationPrivate::mouse_buttons |= me->button();
                else
                    QApplicationPrivate::mouse_buttons &= ~me->button();
        }
#if !defined(QT_NO_WHEELEVENT) || !defined(QT_NO_TABLETEVENT)
        else if (
#  ifndef QT_NO_WHEELEVENT
                 e->type() == QEvent::Wheel ||
#  endif
#  ifndef QT_NO_TABLETEVENT
                 e->type() == QEvent::TabletMove
                 || e->type() == QEvent::TabletPress
                 || e->type() == QEvent::TabletRelease
#  endif
        ) {
            QInputEvent *ie = static_cast<QInputEvent*>(e);
            QApplicationPrivate::modifier_buttons = ie->modifiers();
        }
#endif // !QT_NO_WHEELEVENT || !QT_NO_TABLETEVENT
    }

    // User input and window activation makes tooltips sleep
    switch (e->type()) {
    case QEvent::Wheel:
    case QEvent::ActivationChange:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::FocusOut:
    case QEvent::FocusIn:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
        d->toolTipFallAsleep.stop();
    case QEvent::Leave:
        d->toolTipWakeUp.stop();
    default:
        break;
    }

    bool res = false;
    if (!receiver->isWidgetType()) {
        res = d->notify_helper(receiver, e);
    } else switch (e->type()) {
#if defined QT3_SUPPORT && !defined(QT_NO_SHORTCUT)
    case QEvent::Accel:
        {
            if (d->use_compat()) {
                QKeyEvent* key = static_cast<QKeyEvent*>(e);
                res = d->notify_helper(receiver, e);

                if (!res && !key->isAccepted())
                    res = d->qt_dispatchAccelEvent(static_cast<QWidget *>(receiver), key);

                // next lines are for compatibility with Qt <= 3.0.x: old
                // QAccel was listening on toplevel widgets
                if (!res && !key->isAccepted() && !static_cast<QWidget *>(receiver)->isWindow())
                    res = d->notify_helper(static_cast<QWidget *>(receiver)->window(), e);
            }
            break;
        }
#endif //QT3_SUPPORT && !QT_NO_SHORTCUT
    case QEvent::ShortcutOverride:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        {
            bool isWidget = receiver->isWidgetType();
            bool isGraphicsWidget = false;
#ifndef QT_NO_GRAPHICSVIEW
            isGraphicsWidget = !isWidget && qobject_cast<QGraphicsWidget *>(receiver);
#endif
            if (!isWidget && !isGraphicsWidget) {
                res = d->notify_helper(receiver, e);
                break;
            }

            QKeyEvent* key = static_cast<QKeyEvent*>(e);
#if defined QT3_SUPPORT && !defined(QT_NO_SHORTCUT)
            if (d->use_compat() && d->qt_tryComposeUnicode(static_cast<QWidget*>(receiver), key))
                break;
#endif
            if (key->type()==QEvent::KeyPress) {
#ifndef QT_NO_SHORTCUT
                // Try looking for a Shortcut before sending key events
                if ((res = qApp->d_func()->shortcutMap.tryShortcutEvent(receiver, key)))
                    return res;
#endif
                qt_in_tab_key_event = (key->key() == Qt::Key_Backtab
                                       || key->key() == Qt::Key_Tab
                                       || key->key() == Qt::Key_Left
                                       || key->key() == Qt::Key_Up
                                       || key->key() == Qt::Key_Right
                                       || key->key() == Qt::Key_Down);
            }
            bool def = key->isAccepted();
            QPointer<QObject> pr = receiver;
            while (receiver) {
                if (def)
                    key->accept();
                else
                    key->ignore();
                res = d->notify_helper(receiver, e);
                QWidget *w = isWidget ? static_cast<QWidget *>(receiver) : 0;
#ifndef QT_NO_GRAPHICSVIEW
                QGraphicsWidget *gw = isGraphicsWidget ? static_cast<QGraphicsWidget *>(receiver) : 0;
#endif

                if ((res && key->isAccepted())
                    /*
                       QLineEdit will emit a signal on Key_Return, but
                       ignore the event, and sometimes the connected
                       slot deletes the QLineEdit (common in itemview
                       delegates), so we have to check if the widget
                       was destroyed even if the event was ignored (to
                       prevent a crash)

                       note that we don't have to reset pw while
                       propagating (because the original receiver will
                       be destroyed if one of its ancestors is)
                    */
                    || !pr
                    || (isWidget && (w->isWindow() || !w->parentWidget()))
#ifndef QT_NO_GRAPHICSVIEW
                    || (isGraphicsWidget && (gw->isWindow() || !gw->parentWidget()))
#endif
                    ) {
                    break;
                }

#ifndef QT_NO_GRAPHICSVIEW
                receiver = w ? (QObject *)w->parentWidget() : (QObject *)gw->parentWidget();
#else
                receiver = w->parentWidget();
#endif
            }
            qt_in_tab_key_event = false;
        }
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
        {
            QWidget* w = static_cast<QWidget *>(receiver);

            QMouseEvent* mouse = static_cast<QMouseEvent*>(e);
            QPoint relpos = mouse->pos();

            if (e->spontaneous()) {

                if (e->type() == QEvent::MouseButtonPress) {
                    QWidget *fw = w;
                    while (fw) {
                        if (fw->isEnabled()
                            && QApplicationPrivate::shouldSetFocus(fw, Qt::ClickFocus)) {
                            fw->setFocus(Qt::MouseFocusReason);
                            break;
                        }
                        if (fw->isWindow())
                            break;
                        fw = fw->parentWidget();
                    }
                }

                if (e->type() == QEvent::MouseMove && mouse->buttons() == 0) {
                    d->toolTipWidget = w;
                    d->toolTipPos = relpos;
                    d->toolTipGlobalPos = mouse->globalPos();
                    d->toolTipWakeUp.start(d->toolTipFallAsleep.isActive()?20:QApplicationPrivate::voxToolTipDelayTime, this);
                }
            }

            bool eventAccepted = mouse->isAccepted();

            QPointer<QWidget> pw = w;
            while (w) {
                QMouseEvent me(mouse->type(), relpos, mouse->globalPos(), mouse->button(), mouse->buttons(),
                               mouse->modifiers());
                me.spont = mouse->spontaneous();
                // throw away any mouse-tracking-only mouse events
                if (!w->hasMouseTracking()
                    && mouse->type() == QEvent::MouseMove && mouse->buttons() == 0) {
                    // but still send them through all application event filters (normally done by notify_helper)
                    for (int i = 0; i < d->eventFilters.size(); ++i) {
                        register QObject *obj = d->eventFilters.at(i);
                        if (!obj)
                            continue;
                        if (obj->d_func()->threadData != w->d_func()->threadData) {
                            qWarning("QApplication: Object event filter cannot be in a different thread.");
                            continue;
                        }
                        if (obj->eventFilter(w, w == receiver ? mouse : &me))
                            break;
                    }
                    res = true;
                } else {
                    w->setAttribute(Qt::WA_NoMouseReplay, false);
                    res = d->notify_helper(w, w == receiver ? mouse : &me);
                    e->spont = false;
                }
                eventAccepted = (w == receiver ? mouse : &me)->isAccepted();
                if (res && eventAccepted)
                    break;
                if (w->isWindow() || w->testAttribute(Qt::WA_NoMousePropagation))
                    break;
                relpos += w->pos();
                w = w->parentWidget();
            }

            mouse->setAccepted(eventAccepted);

            if (e->type() == QEvent::MouseMove) {
                if (!pw)
                    break;

                w = static_cast<QWidget *>(receiver);
                relpos = mouse->pos();
                QPoint diff = relpos - w->mapFromGlobal(d->hoverGlobalPos);
                while (w) {
                    if (w->testAttribute(Qt::WA_Hover) &&
                        (!qApp->activePopupWidget() || qApp->activePopupWidget() == w->window())) {
                        QHoverEvent he(QEvent::HoverMove, relpos, relpos - diff);
                        d->notify_helper(w, &he);
                    }
                    if (w->isWindow() || w->testAttribute(Qt::WA_NoMousePropagation))
                        break;
                    relpos += w->pos();
                    w = w->parentWidget();
                }
            }

            d->hoverGlobalPos = mouse->globalPos();
        }
        break;
#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel:
        {
            QWidget* w = static_cast<QWidget *>(receiver);
            QWheelEvent* wheel = static_cast<QWheelEvent*>(e);
            QPoint relpos = wheel->pos();
            bool eventAccepted = wheel->isAccepted();

            if (e->spontaneous()) {
                QWidget *fw = w;
                while (fw) {
                    if (fw->isEnabled()
                        && QApplicationPrivate::shouldSetFocus(fw, Qt::WheelFocus)) {
                        fw->setFocus(Qt::MouseFocusReason);
                        break;
                    }
                    if (fw->isWindow())
                        break;
                    fw = fw->parentWidget();
                }
            }

            while (w) {
                QWheelEvent we(relpos, wheel->globalPos(), wheel->delta(), wheel->buttons(),
                               wheel->modifiers(), wheel->orientation());
                we.spont = wheel->spontaneous();
                res = d->notify_helper(w, w == receiver ? wheel : &we);
                eventAccepted = ((w == receiver) ? wheel : &we)->isAccepted();
                e->spont = false;
                if ((res && eventAccepted)
                    || w->isWindow() || w->testAttribute(Qt::WA_NoMousePropagation))
                    break;

                relpos += w->pos();
                w = w->parentWidget();
            }
            wheel->setAccepted(eventAccepted);
        }
        break;
#endif
#ifndef QT_NO_CONTEXTMENU
    case QEvent::ContextMenu:
        {
            QWidget* w = static_cast<QWidget *>(receiver);
            QContextMenuEvent *context = static_cast<QContextMenuEvent*>(e);
            QPoint relpos = context->pos();
            bool eventAccepted = context->isAccepted();
            while (w) {
                QContextMenuEvent ce(context->reason(), relpos, context->globalPos(), context->modifiers());
                ce.spont = e->spontaneous();
                res = d->notify_helper(w, w == receiver ? context : &ce);
                eventAccepted = ((w == receiver) ? context : &ce)->isAccepted();
                e->spont = false;

                if ((res && eventAccepted)
                    || w->isWindow() || w->testAttribute(Qt::WA_NoMousePropagation))
                    break;

                relpos += w->pos();
                w = w->parentWidget();
            }
            context->setAccepted(eventAccepted);
        }
        break;
#endif // QT_NO_CONTEXTMENU
#ifndef QT_NO_TABLETEVENT
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
        {
            QWidget *w = static_cast<QWidget *>(receiver);
            QTabletEvent *tablet = static_cast<QTabletEvent*>(e);
            QPoint relpos = tablet->pos();
            bool eventAccepted = tablet->isAccepted();
            while (w) {
                QTabletEvent te(tablet->type(), relpos, tablet->globalPos(),
                                tablet->hiResGlobalPos(), tablet->device(), tablet->pointerType(),
                                tablet->pressure(), tablet->xTilt(), tablet->yTilt(),
                                tablet->tangentialPressure(), tablet->rotation(), tablet->z(),
                                tablet->modifiers(), tablet->uniqueId());
                te.spont = e->spontaneous();
                res = d->notify_helper(w, w == receiver ? tablet : &te);
                eventAccepted = ((w == receiver) ? tablet : &te)->isAccepted();
                e->spont = false;
                if ((res && eventAccepted)
                     || w->isWindow()
                     || w->testAttribute(Qt::WA_NoMousePropagation))
                    break;

                relpos += w->pos();
                w = w->parentWidget();
            }
            tablet->setAccepted(eventAccepted);
            qt_tabletChokeMouse = tablet->isAccepted();
        }
        break;
#endif // QT_NO_TABLETEVENT

#if !defined(QT_NO_TOOLTIP) || !defined(QT_NO_WHATSTHIS)
    case QEvent::ToolTip:
    case QEvent::WhatsThis:
    case QEvent::QueryWhatsThis:
        {
            QWidget* w = static_cast<QWidget *>(receiver);
            QHelpEvent *help = static_cast<QHelpEvent*>(e);
            QPoint relpos = help->pos();
            bool eventAccepted = help->isAccepted();
            while (w) {
                QHelpEvent he(help->type(), relpos, help->globalPos());
                he.spont = e->spontaneous();
                res = d->notify_helper(w, w == receiver ? help : &he);
                e->spont = false;
                eventAccepted = (w == receiver ? help : &he)->isAccepted();
                if ((res && eventAccepted) || w->isWindow())
                    break;

                relpos += w->pos();
                w = w->parentWidget();
            }
            help->setAccepted(eventAccepted);
        }
        break;
#endif
#if !defined(QT_NO_STATUSTIP) || !defined(QT_NO_WHATSTHIS)
    case QEvent::StatusTip:
    case QEvent::WhatsThisClicked:
        {
            QWidget *w = static_cast<QWidget *>(receiver);
            while (w) {
                res = d->notify_helper(w, e);
                if ((res && e->isAccepted()) || w->isWindow())
                    break;
                w = w->parentWidget();
            }
        }
        break;
#endif

#ifndef QT_NO_DRAGANDDROP
    case QEvent::DragEnter: {
            QWidget* w = static_cast<QWidget *>(receiver);
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent *>(e);
#ifdef Q_WS_MAC
            // HIView has a slight difference in how it delivers events to children and parents
            // It will not give a leave to a child's parent when it enters a child.
            QWidget *currentTarget = QDragManager::self()->currentTarget();
            if (currentTarget) {
                // Assume currentTarget did not get a leave
                QDragLeaveEvent event;
                QApplication::sendEvent(currentTarget, &event);
            }
#endif
#ifndef QT_NO_GRAPHICSVIEW
            // QGraphicsProxyWidget handles its own propagation,
            // and we must not change QDragManagers currentTarget.
            QWExtra *extra = w->window()->d_func()->extra;
            if (extra && extra->proxyWidget) {
                res = d->notify_helper(w, dragEvent);
                break;
            }
#endif
            while (w) {
                if (w->isEnabled() && w->acceptDrops()) {
                    res = d->notify_helper(w, dragEvent);
                    if (res && dragEvent->isAccepted()) {
                        QDragManager::self()->setCurrentTarget(w);
                        break;
                    }
                }
                if (w->isWindow())
                    break;
                dragEvent->p = w->mapToParent(dragEvent->p);
                w = w->parentWidget();
            }
        }
        break;
    case QEvent::DragMove:
    case QEvent::Drop:
    case QEvent::DragLeave: {
            QWidget* w = static_cast<QWidget *>(receiver);
#ifndef QT_NO_GRAPHICSVIEW
            // QGraphicsProxyWidget handles its own propagation,
            // and we must not change QDragManagers currentTarget.
            QWExtra *extra = w->window()->d_func()->extra;
            bool isProxyWidget = extra && extra->proxyWidget;
            if (!isProxyWidget)
#endif
                w = QDragManager::self()->currentTarget();

            if (!w) {
#ifdef Q_WS_MAC
                // HIView has a slight difference in how it delivers events to children and parents
                // It will not give an enter to a child's parent when it leaves the child.
                if (e->type() == QEvent::DragLeave)
                    break;
                // Assume that w did not get an enter.
                QDropEvent *dropEvent = static_cast<QDropEvent *>(e);
                QDragEnterEvent dragEnterEvent(dropEvent->pos(), dropEvent->possibleActions(),
                                               dropEvent->mimeData(), dropEvent->mouseButtons(),
                                               dropEvent->keyboardModifiers());
                QApplication::sendEvent(receiver, &dragEnterEvent);
                w = QDragManager::self()->currentTarget();
                if (!w)
#endif
                    break;
            }
            if (e->type() == QEvent::DragMove || e->type() == QEvent::Drop) {
                QDropEvent *dragEvent = static_cast<QDropEvent *>(e);
                QWidget *origReciver = static_cast<QWidget *>(receiver);
                while (origReciver && w != origReciver) {
                    dragEvent->p = origReciver->mapToParent(dragEvent->p);
                    origReciver = origReciver->parentWidget();
                }
            }
            res = d->notify_helper(w, e);
            if (e->type() != QEvent::DragMove
#ifndef QT_NO_GRAPHICSVIEW
                && !isProxyWidget
#endif
                )
                QDragManager::self()->setCurrentTarget(0, e->type() == QEvent::Drop);
        }
        break;
#endif

    default:
        res = d->notify_helper(receiver, e);
        break;
    }

    return res;
}

bool QApplicationPrivate::notify_helper(QObject *receiver, QEvent * e)
{
    // send to all application event filters
    if (sendThroughApplicationEventFilters(receiver, e))
        return true;

    if (receiver->isWidgetType()) {
        QWidget *widget = static_cast<QWidget *>(receiver);

#if !defined(Q_OS_WINCE) || (defined(GWES_ICONCURS) && !defined(QT_NO_CURSOR))
        // toggle HasMouse widget state on enter and leave
        if ((e->type() == QEvent::Enter || e->type() == QEvent::DragEnter) &&
            (!qApp->activePopupWidget() || qApp->activePopupWidget() == widget->window()))
            widget->setAttribute(Qt::WA_UnderMouse, true);
        else if (e->type() == QEvent::Leave || e->type() == QEvent::DragLeave)
            widget->setAttribute(Qt::WA_UnderMouse, false);
#endif

        if (QLayout *layout=widget->d_func()->layout) {
            layout->widgetEvent(e);
        }
    }

    // send to all receiver event filters
    if (sendThroughObjectEventFilters(receiver, e))
        return true;

    // deliver the event
    bool consumed = receiver->event(e);
    e->spont = false;
    return consumed;
}


/*!
    \class QSessionManager
    \brief The QSessionManager class provides access to the session manager.

    \ingroup application
    \ingroup environment

    A session manager in a desktop environment (in which Qt GUI applications
    live) keeps track of a session, which is a group of running applications,
    each of which has a particular state. The state of an application contains
    (most notably) the documents the application has open and the position and
    size of its windows.

    The session manager is used to save the session, e.g., when the machine is
    shut down, and to restore a session, e.g., when the machine is started up.
    We recommend that you use QSettings to save an application's settings,
    for example, window positions, recently used files, etc. When the
    application is restarted by the session manager, you can restore the
    settings.

    QSessionManager provides an interface between the application and the
    session manager so that the program can work well with the session manager.
    In Qt, session management requests for action are handled by the two
    virtual functions QApplication::commitData() and QApplication::saveState().
    Both provide a reference to a session manager object as argument, to allow
    the application to communicate with the session manager. The session
    manager can only be accessed through these functions.

    No user interaction is possible \e unless the application gets explicit
    permission from the session manager. You ask for permission by calling
    allowsInteraction() or, if it is really urgent, allowsErrorInteraction().
    Qt does not enforce this, but the session manager may.

    You can try to abort the shutdown process by calling cancel(). The default
    commitData() function does this if some top-level window rejected its
    closeEvent().

    For sophisticated session managers provided on Unix/X11, QSessionManager
    offers further possibilities to fine-tune an application's session
    management behavior: setRestartCommand(), setDiscardCommand(),
    setRestartHint(), setProperty(), requestPhase2(). See the respective
    function descriptions for further details.

    \sa QApplication, {Session Management}
*/

/*! \enum QSessionManager::RestartHint

    This enum type defines the circumstances under which this application wants
    to be restarted by the session manager. The current values are:

    \value  RestartIfRunning    If the application is still running when the
                                session is shut down, it wants to be restarted
                                at the start of the next session.

    \value  RestartAnyway       The application wants to be started at the
                                start of the next session, no matter what.
                                (This is useful for utilities that run just
                                after startup and then quit.)

    \value  RestartImmediately  The application wants to be started immediately
                                whenever it is not running.

    \value  RestartNever        The application does not want to be restarted
                                automatically.

    The default hint is \c RestartIfRunning.
*/


/*!
    \fn QString QSessionManager::sessionId() const

    Returns the identifier of the current session.

    If the application has been restored from an earlier session, this
    identifier is the same as it was in the earlier session.

    \sa sessionKey(), QApplication::sessionId()
*/

/*!
    \fn QString QSessionManager::sessionKey() const

    Returns the session key in the current session.

    If the application has been restored from an earlier session, this key is
    the same as it was when the previous session ended.

    The session key changes with every call of commitData() or saveState().

    \sa sessionId(), QApplication::sessionKey()
*/

/*!
    \fn void* QSessionManager::handle() const

    \internal
*/

/*!
    \fn bool QSessionManager::allowsInteraction()

    Asks the session manager for permission to interact with the user. Returns
    true if interaction is permitted; otherwise returns false.

    The rationale behind this mechanism is to make it possible to synchronize
    user interaction during a shutdown. Advanced session managers may ask all
    applications simultaneously to commit their data, resulting in a much
    faster shutdown.

    When the interaction is completed we strongly recommend releasing the user
    interaction semaphore with a call to release(). This way, other
    applications may get the chance to interact with the user while your
    application is still busy saving data. (The semaphore is implicitly
    released when the application exits.)

    If the user decides to cancel the shutdown process during the interaction
    phase, you must tell the session manager that this has happened by calling
    cancel().

    Here's an example of how an application's QApplication::commitData() might
    be implemented:

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 8

    If an error occurred within the application while saving its data, you may
    want to try allowsErrorInteraction() instead.

    \sa QApplication::commitData(), release(), cancel()
*/


/*!
    \fn bool QSessionManager::allowsErrorInteraction()

    Returns true if error interaction is permitted; otherwise returns false.

    This is similar to allowsInteraction(), but also enables the application to
    tell the user about any errors that occur. Session managers may give error
    interaction requests higher priority, which means that it is more likely
    that an error interaction is permitted. However, you are still not
    guaranteed that the session manager will allow interaction.

    \sa allowsInteraction(), release(), cancel()
*/

/*!
    \fn void QSessionManager::release()

    Releases the session manager's interaction semaphore after an interaction
    phase.

    \sa allowsInteraction(), allowsErrorInteraction()
*/

/*!
    \fn void QSessionManager::cancel()

    Tells the session manager to cancel the shutdown process.  Applications
    should not call this function without asking the user first.

    \sa allowsInteraction(), allowsErrorInteraction()
*/

/*!
    \fn void QSessionManager::setRestartHint(RestartHint hint)

    Sets the application's restart hint to \a hint. On application startup, the
    hint is set to \c RestartIfRunning.

    \note These flags are only hints, a session manager may or may not respect
    them.

    We recommend setting the restart hint in QApplication::saveState() because
    most session managers perform a checkpoint shortly after an application's
    startup.

    \sa restartHint()
*/

/*!
    \fn QSessionManager::RestartHint QSessionManager::restartHint() const

    Returns the application's current restart hint. The default is
    \c RestartIfRunning.

    \sa setRestartHint()
*/

/*!
    \fn void QSessionManager::setRestartCommand(const QStringList& command)

    If the session manager is capable of restoring sessions it will execute
    \a command in order to restore the application. The command defaults to

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 9

    The \c -session option is mandatory; otherwise QApplication cannot tell
    whether it has been restored or what the current session identifier is.
    See QApplication::isSessionRestored() and QApplication::sessionId() for
    details.

    If your application is very simple, it may be possible to store the entire
    application state in additional command line options. This is usually a
    very bad idea because command lines are often limited to a few hundred
    bytes. Instead, use QSettings, temporary files, or a database for this
    purpose. By marking the data with the unique sessionId(), you will be able
    to restore the application in a future  session.

    \sa restartCommand(), setDiscardCommand(), setRestartHint()
*/

/*!
    \fn QStringList QSessionManager::restartCommand() const

    Returns the currently set restart command.

    To iterate over the list, you can use the \l foreach pseudo-keyword:

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 10

    \sa setRestartCommand(), restartHint()
*/

/*!
    \fn void QSessionManager::setDiscardCommand(const QStringList& list)

    Sets the discard command to the given \a list.

    \sa discardCommand(), setRestartCommand()
*/


/*!
    \fn QStringList QSessionManager::discardCommand() const

    Returns the currently set discard command.

    To iterate over the list, you can use the \l foreach pseudo-keyword:

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 11

    \sa setDiscardCommand(), restartCommand(), setRestartCommand()
*/

/*!
    \fn void QSessionManager::setManagerProperty(const QString &name, const QString &value)
    \overload

    Low-level write access to the application's identification and state
    records are kept in the session manager.

    The property called \a name has its value set to the string \a value.
*/

/*!
    \fn void QSessionManager::setManagerProperty(const QString& name,
                                                 const QStringList& value)

    Low-level write access to the application's identification and state record
    are kept in the session manager.

    The property called \a name has its value set to the string list \a value.
*/

/*!
    \fn bool QSessionManager::isPhase2() const

    Returns true if the session manager is currently performing a second
    session management phase; otherwise returns false.

    \sa requestPhase2()
*/

/*!
    \fn void QSessionManager::requestPhase2()

    Requests a second session management phase for the application. The
    application may then return immediately from the QApplication::commitData()
    or QApplication::saveState() function, and they will be called again once
    most or all other applications have finished their session management.

    The two phases are useful for applications such as the X11 window manager
    that need to store information about another application's windows and
    therefore have to wait until these applications have completed their
    respective session management tasks.

    \note If another application has requested a second phase it may get called
    before, simultaneously with, or after your application's second phase.

    \sa isPhase2()
*/

/*****************************************************************************
  Stubbed session management support
 *****************************************************************************/
#ifndef QT_NO_SESSIONMANAGER
#if defined(Q_WS_WIN) || defined(Q_WS_MAC) || defined(Q_WS_QWS)

#if defined(Q_OS_WINCE)
HRESULT qt_CoCreateGuid(GUID* guid)
{
    // We will use the following information to create the GUID
    // 1. absolute path to application
    wchar_t tempFilename[512];
    if (!GetModuleFileNameW(0, tempFilename, 512))
        return S_FALSE;
    unsigned int hash = qHash(QString::fromUtf16((const unsigned short *) tempFilename));
    guid->Data1 = hash;
    // 2. creation time of file
    QFileInfo info(QString::fromUtf16((const unsigned short *) tempFilename));
    guid->Data2 = qHash(info.created().toTime_t());
    // 3. current system time
    guid->Data3 = qHash(QDateTime::currentDateTime().toTime_t());
    return S_OK;
}
#if !defined(OLE32_MCOMGUID) || defined(QT_WINCE_FORCE_CREATE_GUID)
#define CoCreateGuid qt_CoCreateGuid
#endif

#endif

class QSessionManagerPrivate : public QObjectPrivate
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QString sessionKey;
    QSessionManager::RestartHint restartHint;
};

QSessionManager* qt_session_manager_self = 0;
QSessionManager::QSessionManager(QApplication * app, QString &id, QString &key)
    : QObject(*new QSessionManagerPrivate, app)
{
    Q_D(QSessionManager);
    setObjectName(QLatin1String("qt_sessionmanager"));
    qt_session_manager_self = this;
#if defined(Q_WS_WIN)
    wchar_t guidstr[40];
    GUID guid;
    CoCreateGuid(&guid);
    StringFromGUID2(guid, guidstr, 40);
    id = QString::fromUtf16((ushort*)guidstr);
    CoCreateGuid(&guid);
    StringFromGUID2(guid, guidstr, 40);
    key = QString::fromUtf16((ushort*)guidstr);
#endif
    d->sessionId = id;
    d->sessionKey = key;
    d->restartHint = RestartIfRunning;
}

QSessionManager::~QSessionManager()
{
    qt_session_manager_self = 0;
}

QString QSessionManager::sessionId() const
{
    Q_D(const QSessionManager);
    return d->sessionId;
}

QString QSessionManager::sessionKey() const
{
    Q_D(const QSessionManager);
    return d->sessionKey;
}


#if defined(Q_WS_X11) || defined(Q_WS_MAC)
void* QSessionManager::handle() const
{
    return 0;
}
#endif

#if !defined(Q_WS_WIN)
bool QSessionManager::allowsInteraction()
{
    return true;
}

bool QSessionManager::allowsErrorInteraction()
{
    return true;
}
void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
}
#endif


void QSessionManager::setRestartHint(QSessionManager::RestartHint hint)
{
    Q_D(QSessionManager);
    d->restartHint = hint;
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
    Q_D(const QSessionManager);
    return d->restartHint;
}

void QSessionManager::setRestartCommand(const QStringList& command)
{
    Q_D(QSessionManager);
    d->restartCommand = command;
}

QStringList QSessionManager::restartCommand() const
{
    Q_D(const QSessionManager);
    return d->restartCommand;
}

void QSessionManager::setDiscardCommand(const QStringList& command)
{
    Q_D(QSessionManager);
    d->discardCommand = command;
}

QStringList QSessionManager::discardCommand() const
{
    Q_D(const QSessionManager);
    return d->discardCommand;
}

void QSessionManager::setManagerProperty(const QString&, const QString&)
{
}

void QSessionManager::setManagerProperty(const QString&, const QStringList&)
{
}

bool QSessionManager::isPhase2() const
{
    return false;
}

void QSessionManager::requestPhase2()
{
}

#endif
#endif // QT_NO_SESSIONMANAGER

/*!
    \typedef QApplication::ColorMode
    \compat

    Use ColorSpec instead.
*/

/*!
    \fn Qt::MacintoshVersion QApplication::macVersion()

    Use QSysInfo::MacintoshVersion instead.
*/

/*!
    \fn QApplication::ColorMode QApplication::colorMode()

    Use colorSpec() instead, and use ColorSpec as the enum type.
*/

/*!
    \fn void QApplication::setColorMode(ColorMode mode)

    Use setColorSpec() instead, and pass a ColorSpec value instead.
*/

/*!
    \fn bool QApplication::hasGlobalMouseTracking()

    This feature does not exist anymore. This function always returns true
    in Qt 4.
*/

/*!
    \fn void QApplication::setGlobalMouseTracking(bool dummy)

    This function does nothing in Qt 4. The \a dummy parameter is ignored.
*/

/*!
    \fn void QApplication::flushX()

    Use flush() instead.
*/

/*!
    \fn void QApplication::setWinStyleHighlightColor(const QColor &c)

    Use the palette instead.

    \oldcode
    app.setWinStyleHighlightColor(color);
    \newcode
    QPalette palette(qApp->palette());
    palette.setColor(QPalette::Highlight, color);
    qApp->setPalette(palette);
    \endcode
*/

/*!
    \fn void QApplication::setPalette(const QPalette &pal, bool b, const char* className = 0)

    Use the two-argument overload instead.
*/

/*!
    \fn void QApplication::setFont(const QFont &font, bool b, const char* className = 0)

    Use the two-argument overload instead.
*/

/*!
    \fn const QColor &QApplication::winStyleHighlightColor()

    Use qApp->palette().color(QPalette::Active, QPalette::Highlight) instead.
*/

/*!
    \fn QWidget *QApplication::widgetAt(int x, int y, bool child)

    Use the two-argument widgetAt() overload to get the child widget. To get
    the top-level widget do this:

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 12
*/

/*!
    \fn QWidget *QApplication::widgetAt(const QPoint &point, bool child)

    Use the single-argument widgetAt() overload to get the child widget. To get
    the top-level widget do this:

    \snippet doc/src/snippets/code/src_gui_kernel_qapplication.cpp 13
*/

#ifdef QT3_SUPPORT
QWidget *QApplication::mainWidget()
{
    return QApplicationPrivate::main_widget;
}
#endif
bool QApplicationPrivate::inPopupMode() const
{
    return QApplicationPrivate::popupWidgets != 0;
}

/*!
    \property QApplication::quitOnLastWindowClosed

    \brief whether the application implicitly quits when the last window is
    closed.

    The default is true.

    If this property is true, the applications quits when the last visible
    primary window (i.e. window with no parent) with the Qt::WA_QuitOnClose
    attribute set is closed. By default this attribute is set for all widgets
    except for sub-windows. Refer to \l{Qt::WindowType} for a detailed list of
    Qt::Window objects.

    \sa quit(), QWidget::close()
 */

void QApplication::setQuitOnLastWindowClosed(bool quit)
{
    QApplicationPrivate::quitOnLastWindowClosed = quit;
}

bool QApplication::quitOnLastWindowClosed()
{
    return QApplicationPrivate::quitOnLastWindowClosed;
}

void QApplicationPrivate::emitLastWindowClosed()
{
    if (qApp && qApp->d_func()->in_exec) {
        if (QApplicationPrivate::quitOnLastWindowClosed) {
            // get ready to quit, this event might be removed if the
            // event loop is re-entered, however
            QApplication::postEvent(qApp, new QEvent(QEvent::Quit));
        }
        emit qApp->lastWindowClosed();
    }
}

/*! \variable QApplication::NormalColors
    \compat

    Use \l NormalColor instead.
*/

/*! \variable QApplication::CustomColors
    \compat

    Use \l CustomColor instead.
*/

#ifdef QT_KEYPAD_NAVIGATION
/*!
    Sets whether Qt should use focus navigation suitable for use with a
    minimal keypad.

    If \a enable is true, Qt::Key_Up and Qt::Key_Down are used to change focus.

    This feature is available in Qt for Embedded Linux only.

    \sa keypadNavigationEnabled()
*/
void QApplication::setKeypadNavigationEnabled(bool enable)
{
    QApplicationPrivate::keypadNavigation = enable;
}

/*!
    Returns true if Qt is set to use keypad navigation; otherwise returns
    false. The default is false.

    This feature is available in Qt for Embedded Linux only.

    \sa setKeypadNavigationEnabled()
*/
bool QApplication::keypadNavigationEnabled()
{
    return QApplicationPrivate::keypadNavigation;
}
#endif

/*!
    \fn void QApplication::alert(QWidget *widget, int msec)
    \since 4.3

    Causes an alert to be shown for \a widget if the window is not the active
    window. The alert is shown for \a msec miliseconds. If \a msec is zero (the
    default), then the alert is shown indefinitely until the window becomes
    active again.

    Currently this function does nothing on Qt for Embedded Linux.

    On Mac OS X, this works more at the application level and will cause the
    application icon to bounce in the dock.

    On Windows this causes the window's taskbar entry to flash for a time. If
    \a msec is zero, the flashing will stop and the taskbar entry will turn a
    different color (currently orange).

    On X11, this will cause the window to be marked as "demands attention", the
    window must not be hidden (i.e. not have hide() called on it, but be
    visible in some sort of way) in order for this to work.
*/

/*!
    \property QApplication::cursorFlashTime
    \brief the text cursor's flash (blink) time in milliseconds

    The flash time is the time required to display, invert and restore the
    caret display. Usually the text cursor is displayed for half the cursor
    flash time, then hidden for the same amount of time, but this may vary.

    The default value on X11 is 1000 milliseconds. On Windows, the control
    panel value is used. Widgets should not cache this value since it may be
    changed at any time by the user changing the global desktop settings.

    \note On Microsoft Windows, setting this property sets the cursor flash
    time for all applications.
*/

/*!
    \property QApplication::doubleClickInterval
    \brief the time limit in milliseconds that distinguishes a double click from two
    consecutive mouse clicks

    The default value on X11 is 400 milliseconds. On Windows and Mac OS X, the
    operating system's value is used.

    On Microsoft Windows, calling this function sets the double click interval
    for all applications.
*/

/*!
    \property QApplication::keyboardInputInterval
    \brief the time limit in milliseconds that distinguishes a key press
    from two consecutive key presses
    \since 4.2

    The default value on X11 is 400 milliseconds. On Windows and Mac OS X, the
    operating system's value is used.
*/

/*!
    \property QApplication::wheelScrollLines
    \brief the number of lines to scroll a widget, when the
    mouse wheel is rotated.

    If the value exceeds the widget's number of visible lines, the widget
    should interpret the scroll operation as a single \e{page up} or
    \e{page down}. If the widget is an \l{QAbstractItemView}{item view class},
    then the result of scrolling one \e line depends on the setting of the
    widget's \l{QAbstractItemView::verticalScrollMode()}{scroll mode}. Scroll
    one \e line can mean \l{QAbstractItemView::ScrollPerItem}{scroll one item}
    or \l{QAbstractItemView::ScrollPerPixel}{scroll one pixel}.

    By default, this property has a value of 3.
*/

/*!
    \fn void QApplication::setEffectEnabled(Qt::UIEffect effect, bool enable)

    Enables the UI effect \a effect if \a enable is true, otherwise the effect
    will not be used.

    \note All effects are disabled on screens running at less than 16-bit color
    depth.

    \sa isEffectEnabled(), Qt::UIEffect, setDesktopSettingsAware()
*/

/*!
    \fn bool QApplication::isEffectEnabled(Qt::UIEffect effect)

    Returns true if \a effect is enabled; otherwise returns false.

    By default, Qt will try to use the desktop settings. To prevent this, call
    setDesktopSettingsAware(false).

    \note All effects are disabled on screens running at less than 16-bit color
    depth.

    \sa setEffectEnabled(), Qt::UIEffect
*/

/*!
    \fn QWidget *QApplication::mainWidget()

    Returns the main application widget, or 0 if there is no main widget.
*/

/*!
    \fn void QApplication::setMainWidget(QWidget *mainWidget)

    Sets the application's main widget to \a mainWidget.

    In most respects the main widget is like any other widget, except that if
    it is closed, the application exits. QApplication does \e not take
    ownership of the \a mainWidget, so if you create your main widget on the
    heap you must delete it yourself.

    You need not have a main widget; connecting lastWindowClosed() to quit()
    is an alternative.

    For X11, this function also resizes and moves the main widget according
    to the \e -geometry command-line option, so you should set the default
    geometry (using \l QWidget::setGeometry()) before calling setMainWidget().

    \sa mainWidget(), exec(), quit()
*/

/*!
    \fn void QApplication::beep()

    Sounds the bell, using the default volume and sound. The function is \e not
    available in Qt for Embedded Linux.
*/

/*!
    \fn void QApplication::setOverrideCursor(const QCursor &cursor)

    Sets the application override cursor to \a cursor.

    Application override cursors are intended for showing the user that the
    application is in a special state, for example during an operation that
    might take some time.

    This cursor will be displayed in all the application's widgets until
    restoreOverrideCursor() or another setOverrideCursor() is called.

    Application cursors are stored on an internal stack. setOverrideCursor()
    pushes the cursor onto the stack, and restoreOverrideCursor() pops the
    active cursor off the stack. changeOverrideCursor() changes the curently
    active application override cursor. Every setOverrideCursor() must
    eventually be followed by a corresponding restoreOverrideCursor(),
    otherwise the stack will never be emptied.

    Example:
    \snippet doc/src/snippets/code/src_gui_kernel_qapplication_x11.cpp 0

    \sa overrideCursor(), restoreOverrideCursor(), changeOverrideCursor(),
    QWidget::setCursor()
*/

/*!
    \fn void QApplication::restoreOverrideCursor()

    Undoes the last setOverrideCursor().

    If setOverrideCursor() has been called twice, calling
    restoreOverrideCursor() will activate the first cursor set. Calling this
    function a second time restores the original widgets' cursors.

    \sa setOverrideCursor(), overrideCursor()
*/

/*!
    \macro qApp
    \relates QApplication

    A global pointer referring to the unique application object. It is
    equivalent to the pointer returned by the QCoreApplication::instance()
    function except that, in GUI applications, it is a pointer to a
    QApplication instance.

    Only one application object can be created.

    \sa QCoreApplication::instance()
*/

// ************************************************************************
// Input Method support
// ************************************************************************

/*!
    This function replaces the QInputContext instance used by the application
    with \a inputContext.

    \sa inputContext()
*/
void QApplication::setInputContext(QInputContext *inputContext)
{
    Q_D(QApplication);
    Q_UNUSED(d);// only static members being used.
    if (!inputContext) {
        qWarning("QApplication::setInputContext: called with 0 input context");
        return;
    }
    if (d->inputContext)
        delete d->inputContext;
    d->inputContext = inputContext;
}

/*!
    Returns the QInputContext instance used by the application.

    \sa setInputContext()
*/
QInputContext *QApplication::inputContext() const
{
    Q_D(const QApplication);
    Q_UNUSED(d);// only static members being used.
#ifdef Q_WS_X11
    if (!X11)
        return 0;
    if (!d->inputContext) {
        QApplication *that = const_cast<QApplication *>(this);
        QInputContext *qic = QInputContextFactory::create(X11->default_im, that);
        // fallback to default X Input Method.
        if (!qic)
            qic = QInputContextFactory::create(QLatin1String("xim"), that);
        that->d_func()->inputContext = qic;
    }
#endif
    return d->inputContext;
}

//Returns the current platform used by keyBindings
uint QApplicationPrivate::currentPlatform(){
    uint platform = KB_Win;
#ifdef Q_WS_MAC
    platform = KB_Mac;
#elif defined Q_WS_X11
    platform = KB_X11;
    if (X11->desktopEnvironment == DE_KDE)
        platform |= KB_KDE;
    if (X11->desktopEnvironment == DE_GNOME)
        platform |= KB_Gnome;
    if (X11->desktopEnvironment == DE_CDE)
        platform |= KB_CDE;
#endif
    return platform;
}

bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event)
{
    return QCoreApplication::sendSpontaneousEvent(receiver, event);
}


/*!
    \since 4.2

    Returns the current keyboard input locale.
*/
QLocale QApplication::keyboardInputLocale()
{
    if (!QApplicationPrivate::checkInstance("keyboardInputLocale"))
        return QLocale::c();
    return qt_keymapper_private()->keyboardInputLocale;
}

/*!
    \since 4.2

    Returns the current keyboard input direction.
*/
Qt::LayoutDirection QApplication::keyboardInputDirection()
{
    if (!QApplicationPrivate::checkInstance("keyboardInputDirection"))
        return Qt::LeftToRight;
    return qt_keymapper_private()->keyboardInputDirection;
}

bool QApplicationPrivate::shouldSetFocus(QWidget *w, Qt::FocusPolicy policy)
{
    QWidget *f = w;
    while (f->d_func()->extra && f->d_func()->extra->focus_proxy)
        f = f->d_func()->extra->focus_proxy;

    if ((w->focusPolicy() & policy) != policy)
        return false;
    if (w != f && (f->focusPolicy() & policy) != policy)
        return false;
    return true;
}

/*! \fn QDecoration &QApplication::qwsDecoration()
    Return the QWSDecoration used for decorating windows.

    \warning This method is non-portable. It is only available in
    Qt for Embedded Linux.

    \sa QDecoration
*/

/*!
    \fn void QApplication::qwsSetDecoration(QDecoration *decoration)

    Sets the QDecoration derived class to use for decorating the
    windows used by Qt for Embedded Linux to the \a decoration
    specified.

    This method is non-portable. It is only available in Qt for Embedded Linux.

    \sa QDecoration
*/

/*! \fn QDecoration* QApplication::qwsSetDecoration(const QString &decoration)
  \overload

  Requests a QDecoration object for \a decoration from the QDecorationFactory.

  The string must be one of the QDecorationFactory::keys(). Keys are
  case insensitive.

  A later call to the QApplication constructor will override the
  requested style when a "-style" option is passed in as a commandline
  parameter.

  Returns 0 if an unknown \a decoration is passed, otherwise the QStyle object
  returned is set as the application's GUI style.
*/

/*!
    \fn bool QApplication::qwsEventFilter(QWSEvent *event)

    This virtual function is only implemented under Qt for Embedded Linux.

    If you create an application that inherits QApplication and
    reimplement this function, you get direct access to all QWS (Q
    Window System) events that the are received from the QWS master
    process. The events are passed in the \a event parameter.

    Return true if you want to stop the event from being processed.
    Return false for normal event dispatching. The default
    implementation returns false.
*/

/*! \fn void QApplication::qwsSetCustomColors(QRgb *colorTable, int start, int numColors)
    Set Qt for Embedded Linux custom color table.

    Qt for Embedded Linux on 8-bpp displays allocates a standard 216 color cube.
    The remaining 40 colors may be used by setting a custom color
    table in the QWS master process before any clients connect.

    \a colorTable is an array of up to 40 custom colors. \a start is
    the starting index (0-39) and \a numColors is the number of colors
    to be set (1-40).

    This method is non-portable. It is available \e only in
    Qt for Embedded Linux.

    \note The custom colors will not be used by the default screen
    driver. To make use of the new colors, implement a custom screen
    driver, or use QDirectPainter.
*/

/*! \fn int QApplication::qwsProcessEvent(QWSEvent* event)
    \internal
*/

/*! \fn int QApplication::x11ClientMessage(QWidget* w, XEvent* event, bool passive_only)
    \internal
*/

/*! \fn int QApplication::x11ProcessEvent(XEvent* event)
    This function does the core processing of individual X
    \a{event}s, normally by dispatching Qt events to the right
    destination.

    It returns 1 if the event was consumed by special handling, 0 if
    the \a event was consumed by normal handling, and -1 if the \a
    event was for an unrecognized widget.

    \sa x11EventFilter()
*/

/*!
    \fn bool QApplication::x11EventFilter(XEvent *event)

    \warning This virtual function is only implemented under X11.

    If you create an application that inherits QApplication and
    reimplement this function, you get direct access to all X events
    that the are received from the X server. The events are passed in
    the \a event parameter.

    Return true if you want to stop the event from being processed.
    Return false for normal event dispatching. The default
    implementation returns false.

    It is only the directly addressed messages that are filtered.
    You must install an event filter directly on the event
    dispatcher, which is returned by
    QAbstractEventDispatcher::instance(), to handle system wide
    messages.

    \sa x11ProcessEvent()
*/

/*! \fn void QApplication::winFocus(QWidget *widget, bool gotFocus)
    \internal
    \since 4.1

    If \a gotFocus is true, \a widget will become the active window.
    Otherwise the active window is reset to 0.
*/

/*! \fn void QApplication::winMouseButtonUp()
  \internal
 */

/*! \fn void QApplication::syncX()
  Synchronizes with the X server in the X11 implementation.
  This normally takes some time. Does nothing on other platforms.
*/

QT_END_NAMESPACE

#include "moc_qapplication.cpp"
