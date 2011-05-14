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
#include <qtutil/KeywordTranslator.h>

static const char KEYWORD_MARKER = '@';

KeywordTranslator::KeywordTranslator(QObject* parent)
: QTranslator(parent)
, _fallbackTranslator(0)
{}

void KeywordTranslator::setKeywordHash(const KeywordTranslator::KeywordHash& hash) {
	_keywordHash.clear();

	// Instead of copying hash directly to _keywordHash, adds the keyword
	// markers to the hash keys instead of doing it for every translated
	// strings.
	KeywordHash::const_iterator
		it = hash.begin(),
		end = hash.end();
	for (;it!=end; ++it) {
		QString key = KEYWORD_MARKER + it.key() + KEYWORD_MARKER;
		_keywordHash[key] = it.value();
	}
}

void KeywordTranslator::setFallbackTranslator(const QTranslator* translator) {
	_fallbackTranslator = translator;
}

QString KeywordTranslator::translate(const char* context, const char* sourceText, const char* comment) const {
	QString text = QTranslator::translate(context, sourceText, comment);
	if (text.isEmpty() && _fallbackTranslator) {
		text = _fallbackTranslator->translate(context, sourceText, comment);
	}
	if (text.isEmpty()) {
		text = QString::fromUtf8(sourceText);
	}

	KeywordHash::const_iterator
		it = _keywordHash.begin(),
		end = _keywordHash.end();
	for (;it!=end; ++it) {
		text.replace(it.key(), it.value());
	}
	return text;
}
