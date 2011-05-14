#ifndef SPELLTEXTEDIT_H_
#define SPELLTEXTEDIT_H_

#include <QtGui/QTextEdit>
#include <QtGui/QAction>
#include <QtCore/QMimeData>
#include <QtGui/QContextMenuEvent>
#include "hunspell/hunspell.hxx"
#include "highlighter.h"

class SpellTextEdit : public QTextEdit
{
	Q_OBJECT

public:
    SpellTextEdit(QWidget *parent = 0,QString SpellDic="");
	~SpellTextEdit();
	QStringList getWordPropositions(const QString word);
	bool setDict(const QString SpellDic);

Q_SIGNALS:
	void addWord(QString word);

protected:
	void createActions();
    void contextMenuEvent(QContextMenuEvent *event);

private Q_SLOTS:
	void correctWord();
	void slot_addWord();
	void slot_ignoreWord();

	void insertFromMimeData( const QMimeData *source );

private:
    enum { MaxWords = 5 };
    QAction *misspelledWordsActs[MaxWords];

    QString spell_dic;
    Hunspell *pChecker;
	Highlighter * _highlighter;
    QPoint lastPos;

    QStringList addedWords;
};

#endif /*SPELLTEXTEDIT_H_*/
