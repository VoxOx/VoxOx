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
#ifndef MACSPELLCHECKER_H
#define MACSPELLCHECKER_H

#include "qtutil/mac_private.h"
#include <QtGui/QSyntaxHighlighter>
#include <QtGui/QTextEdit>
#include <QtGui/QTextCursor>
#include <QtCore/QMimeData>
struct _NSRange;
typedef struct _NSRange NSRange;
struct Range
{
    Range(const NSRange &nsrange);
    Range(int index, int length);
    bool operator==(const Range &other) const;
    int index;
    int length;
};

QDebug operator<<(QDebug d, const Range &range);

class QtMacSpellCheckHighlighter : public QSyntaxHighlighter
{
public:
    QtMacSpellCheckHighlighter(QTextDocument *textDocument)
    : QSyntaxHighlighter(textDocument), enabled(true) { }
    QtMacSpellCheckHighlighter(QTextEdit *textEdit)
    : QSyntaxHighlighter(textEdit), enabled(true) { }

    void highlightBlock(const QString &text);
    bool enabled;
};

class QAction;
class QtMacSpellCheckingTextEdit: public QTextEdit
{
Q_OBJECT
public:    
    QtMacSpellCheckingTextEdit();
protected:
    void contextMenuEvent(QContextMenuEvent * e);
    void toggleSyntaxHighlighting();
    void spotlight(const QString &word);
    void openPanel(const QString &word);
private Q_SLOTS:
    void suggestedWordSelected(QAction *action);
	void insertFromMimeData( const QMimeData *source );
private:
    QTextCursor checkedTextCursor;
    QString currentWord;
    QtMacSpellCheckHighlighter *syntaxhighlighter;

    QAction *ignoreSpellingAction;
    QAction *learnSpellingAction;

    QAction *lookupSpotlightAction;
    QAction *lookupGoogleAction;

    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;

    QAction *spellingPanelAction;
    QAction *asYouTypeAction;
};

#endif
