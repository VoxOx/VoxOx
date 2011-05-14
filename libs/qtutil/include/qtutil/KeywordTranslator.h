/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#ifndef KEYWORDTRANSLATOR_H
#define KEYWORDTRANSLATOR_H

#include <QtCore/QHash>
#include <QtCore/QTranslator>

/**
 * A translator which can replace keywords in a translated string with custom
 * values, useful for cobranding.
 *
 * QTranslator::translate() is supposed to return an empty string if it can't
 * translate a string, allowing Qt to fallback to another translator.
 * Since this would prevent us from replacing keywords in untranslated strings,
 * our implementation always returns something. If you need to fallback to
 * another translator (typically the one containing translations for Qt library
 * strings), use setFallbackTranslator()
 */
class KeywordTranslator : public QTranslator {
public:
	typedef QHash<QString, QString> KeywordHash;

	KeywordTranslator(QObject* parent);

	void setKeywordHash(const KeywordHash&);

	void setFallbackTranslator(const QTranslator* translator);

	virtual QString translate(const char * context, const char * sourceText, const char * comment = 0) const;

private:
	KeywordHash _keywordHash;
	const QTranslator* _fallbackTranslator;
};

#endif /* KEYWORDTRANSLATOR_H */
