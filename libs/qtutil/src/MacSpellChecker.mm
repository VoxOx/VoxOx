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
#include <qtutil/macspellchecker.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#import <AppKit/AppKit.h>
#include <QtCore/QList>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtCore/qdebug.h>

#if (MAC_OS_X_VERSION_MAX_ALLOWED == MAC_OS_X_VERSION_10_4)
@interface NSSpellChecker (NotYetPublicMethods)
- (void)learnWord:(NSString *)word;
@end
#endif

Range::Range(const NSRange &nsrange)
: index(nsrange.location), length(nsrange.length) { } 

Range::Range(int index, int length)
:index(index), length(length)
{ }

bool Range::operator==(const Range &other) const
{
    return (index == other.index && length == other.length);
}

QDebug operator<<(QDebug d, const Range &range)
{
    d << "index " << range.index << " lenght " << range.length;
    return d;
}

#ifdef __LP64__ 
# define TAG_TYPE NSInteger
#else
# define TAG_TYPE int
#endif
namespace SpellCheckerBridge
{
    // returns a liste of ranges that contains the locations
    QList<Range> spellingErrorIndexes(const QString& text, QTextDocument *document)
    {
        const QtMacCocoaAutoReleasePool pool;
        const QtCFString string(text);
        const int textLenght = text.length();
        
        int index = 0;
        QList<Range> ranges;
        while (index < textLenght) {
            const Range range = [[NSSpellChecker sharedSpellChecker] 
                                    checkSpellingOfString:(NSString *)(CFStringRef)string
                                    startingAt:index
                                    language: nil
                                    wrap : false
                                    inSpellDocumentWithTag : reinterpret_cast<TAG_TYPE>(document)
                                    wordCount : nil];
            const int rangeEnd = range.index + range.length;
            index = rangeEnd;
            if (range.index != INT_MAX)
                ranges.append(range);
        }
        return ranges;
    }

    // returns wether text contains no spelling errors.
    bool isSpeltCorrectly(const QString& text, QTextDocument *document)
    {
        return (spellingErrorIndexes(text, document).count() == 0);
    }

    // returns a list of suggestions for \a word.
    QStringList suggestions(const QString& word)
    {
        const QtMacCocoaAutoReleasePool pool;
        NSArray * const array  = [[NSSpellChecker sharedSpellChecker] guessesForWord : (NSString *)QtCFString::toCFStringRef(word)];

        QStringList suggestions;
        if (array == 0)
            return suggestions;

        for (unsigned int i = 0; i < [array count]; ++i)
            suggestions.append(QtCFString::toQString((CFStringRef)[array objectAtIndex: i]));
        
        return suggestions;
    }

    // Ignores a word when spell checking, for the given document only.
    void ignoreSpelling(const QString &word, QTextDocument *document)
    {
        const QtMacCocoaAutoReleasePool pool;
        [[NSSpellChecker sharedSpellChecker] 
            ignoreWord : (NSString *)QtCFString::toCFStringRef(word) 
            inSpellDocumentWithTag : reinterpret_cast<TAG_TYPE>(document)];
    }
    
    void learnSpelling(const QString &word)
    {
        [[NSSpellChecker sharedSpellChecker] learnWord: (NSString *)QtCFString::toCFStringRef(word)];
    }
}

void QtMacSpellCheckHighlighter::highlightBlock(const QString &text)
{
    if (!enabled)
        return;

	if (!text.endsWith(" ")) // VOXOX -ASV- 08-10-2009: we only trigger the spell check if the word was completed 
		return;
	
    QTextCharFormat spellingErrorFormat;
    spellingErrorFormat.setUnderlineColor(QColor(Qt::red));
    spellingErrorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
	
    Q_FOREACH(Range spellingErrorIndex, SpellCheckerBridge::spellingErrorIndexes(text, document())) {
        setFormat(spellingErrorIndex.index, spellingErrorIndex.length, spellingErrorFormat);
    }
}

QtMacSpellCheckingTextEdit::QtMacSpellCheckingTextEdit()
:ignoreSpellingAction(new QAction(tr("Ignore spelling"), this))
,learnSpellingAction(new QAction(tr("Learn spelling"), this))

,lookupSpotlightAction(new QAction(tr("Search in Spotlight"), this))
,lookupGoogleAction(new QAction(tr("Serach in Google"), this))

,cutAction(new QAction(tr("Cut"), this))
,copyAction(new QAction(tr("Copy"), this))
,pasteAction(new QAction(tr("Paste"), this))
,spellingPanelAction(new QAction(tr("Spelling..."), this))
,asYouTypeAction(new QAction(tr("Check Spelling as You Type"), this))
{
    syntaxhighlighter = new QtMacSpellCheckHighlighter(this);
    asYouTypeAction->setCheckable(true);
    asYouTypeAction->setChecked(true);
}

//VOXOX - CJC - 2009.05.06 Fix bug when inserting test with html format
void QtMacSpellCheckingTextEdit::insertFromMimeData( const QMimeData *source ){
   
	insertPlainText(source->text());
   
}

void QtMacSpellCheckingTextEdit::contextMenuEvent(QContextMenuEvent * e)
{
    QTextCursor cursor = cursorForPosition(e->pos());
    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);

    currentWord = cursor.selectedText();
    const bool gotWord = (currentWord != QString());
    QMenu * const menu = new QMenu();

    // If the word is misspelt, add the suggestions to the popup menu.
    // If there are no suggestions we add a disabled "Not found action".
    if (SpellCheckerBridge::isSpeltCorrectly(currentWord, document()) == false) {
        const QStringList suggestions = SpellCheckerBridge::suggestions(currentWord);
        if (suggestions.isEmpty()) {
            QAction * const noSuggestionsAction = menu->addAction(tr("No Guesses Found"));
            noSuggestionsAction->setEnabled(false);
        } else {
            Q_FOREACH (const QString &suggestion, suggestions)
                menu->addAction(suggestion);  
        }

        menu->addSeparator();
        menu->addAction(ignoreSpellingAction);
        menu->addAction(learnSpellingAction);
    }
        
    if (gotWord) {
        menu->addSeparator();
        menu->addAction(lookupSpotlightAction);
        menu->addAction(lookupGoogleAction);
    }

    menu->addSeparator();
    menu->addAction(cutAction);
    menu->addAction(copyAction);
    menu->addAction(pasteAction);
    
    cutAction->setDisabled(!gotWord);
    copyAction->setDisabled(!gotWord);
    const bool gotClipboardContents = (QApplication::clipboard()->text() == QString());
    pasteAction->setDisabled(!gotClipboardContents);
    
    menu->addSeparator();
    QMenu *spelling = new QMenu(tr("Spelling"), menu);
    menu->addMenu(spelling);
    spelling->addAction(spellingPanelAction);
    spelling->addAction(asYouTypeAction);
    
    checkedTextCursor = cursor;
    connect(menu, SIGNAL(triggered(QAction *)), SLOT(suggestedWordSelected(QAction *)));

    menu->exec(e->globalPos());
    delete menu;
 }

void QtMacSpellCheckingTextEdit::toggleSyntaxHighlighting()
{
    syntaxhighlighter->enabled = !syntaxhighlighter->enabled;
    syntaxhighlighter->rehighlight();
}

void QtMacSpellCheckingTextEdit::spotlight(const QString &word)
{
    const OSStatus resultCode = HISearchWindowShow(QtCFString::toCFStringRef(word),kNilOptions);
    if (resultCode != noErr)
        qWarning("Failed to open spotlight window");
}

void QtMacSpellCheckingTextEdit::openPanel(const QString &word)
{
    [[NSSpellChecker sharedSpellChecker] updateSpellingPanelWithMisspelledWord : (NSString *)QtCFString::toCFStringRef(word)];
    [[[NSSpellChecker sharedSpellChecker] spellingPanel] orderFront:nil];
}

void QtMacSpellCheckingTextEdit::suggestedWordSelected(QAction *action)
{
    if (action == ignoreSpellingAction) {
        SpellCheckerBridge::ignoreSpelling(currentWord, document());
    } else if (action == learnSpellingAction) {
        SpellCheckerBridge::learnSpelling(currentWord);    
    } else if (action == lookupSpotlightAction) {
        spotlight(currentWord);
    } else if (action == lookupGoogleAction) {
        const QString searchString = "http://www.google.com/search?q=" + currentWord;
        QDesktopServices::openUrl(QUrl(searchString));
    } else if (action == cutAction) {
        QApplication::clipboard()->setText(currentWord);
    } else if (action == copyAction) {
        QApplication::clipboard()->setText(currentWord);
    } else if (action == pasteAction) {
        checkedTextCursor.insertText(action->text());
    } else if (action == spellingPanelAction) {
        openPanel(currentWord);
    } else if (action == asYouTypeAction) {
        toggleSyntaxHighlighting();
    } else {
        checkedTextCursor.insertText(action->text());
    }
}

