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

#ifndef OWQTCHATUTILS_H
#define OWQTCHATUTILS_H

#include <imwrapper/EnumIMProtocol.h>

class QColor;
class QFont;
class QString;

/**
 * Helper class that format message
 *
 * @ingroup presentation
 * @author Mr K.
 * @author Mathieu Stute
 */
class QtChatUtils {
public:
	/**
	 * Gets a string representing the given protocol.
	 *
	 * @param protocol the protocol.
	 * @return a string representing the protocol.
	 */
//	static const QString getProtocol1(EnumIMProtocol::IMProtocol protocol);	//VOXOX - JRT - 2009.04.21 - move to QtEnumIMProtocolMap

	/**
	 * Prepares a message to be sent to the network.
	 *
	 * @param font the font to use.
	 * @param protocol used protocol.
	 * @param message the message to prepare.
	 * @return the prepared message.
	 */
//	static const QString encodeMessage( EnumIMProtocol::IMProtocol protocol, const QString & message);
	static const QString encodeMessage( const QString & message );		//VOXOX - JRT - OK - 2010.01.06 - protocol param is not used in method.

	//VOXOX - CJC - 2009.05.08 Encode message for chat to email
	static const QString encodeMessageChatToEmail(const QString & toEmail,const QString & from, const QString & subject, const QString & message);
	//VOXOX - CJC - 2009.06.15 
	static const QString encodeMessageChatToSMS(const QString & toNumber,const QString & from, const QString & message);

	/**
	 * Decodes a message from the network.
	 *
	 * @param protocol used protocol.
	 * @param message the message to decode.
	 * @return the prepared message.
	 */
	static const QString decodeMessage(const QString & message);

	static const QString fixHTMLforJavascript(const QString & html);

	//VOXOX CHANGE by Rolando - 2009.12.11 
	/**
	 * @see QtEmoticonsManager::emoticon2Text.
	 */
	static const QString emoticons2Text(const QString & htmlstr);//VOXOX CHANGE by Rolando - 2009.12.10 

private:

	/**
	 * Replaces urls by href html code.
	 *
	 * @param str the message in plain text.
	 * @param htmlstr the message in html.
	 * @return the changed message.
	 */
	static const QString replaceUrls(const QString & str, const QString & htmlstr);

	static const QString replaceImgUrls(const QString & htmlstr);

	static const QString checkIfHTMLIsInHREF(const QString & fullHtml,const QString & concatHtml);

	static const QString replaceYoutubeAddress(const QString & htmlstr);

	static const QString removeFontTags(const QString & htmlstr);

	/**
	 * @see QtEmoticonsManager::text2Emoticon.
	 */
	static const QString text2Emoticon(const QString & htmlstr);

	/**
	// * @see QtEmoticonsManager::emoticon2Text.
	// */
	//static const QString emoticons2Text(const QString & htmlstr);

	static const QString removeBreaklines(const QString & html);//VOXOX - CJC - 2009.09.17 
};

#endif	//OWQTCHATUTILS_H
