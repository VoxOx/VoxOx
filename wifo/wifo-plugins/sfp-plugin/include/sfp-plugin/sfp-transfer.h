/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef __SFP_TRANSFER_H__
#define __SFP_TRANSFER_H__

#include <sfp-plugin/sfp-plugin.h>
#include <sfp-plugin/sfp-error.h>
#include <sfp-plugin/sfp-session.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	/**
	* Sends a file using the information provided in the session info
	* TODO mutualize initialization part
	*
	* @param	[in-out]	session : a session info
	* @return	TRUE if the sending succeeded; FALSE else
	*/
	unsigned int sfp_transfer_send_file(sfp_session_info_t * session);

	/**
	* Receives a file using the information provided in the session info
	* TODO mutualize initialization part
	*
	* @param	[in-out]	session
	* @return	TRUE if the receiving succeeded; FALSE else
	*/
	unsigned int sfp_transfer_receive_file(sfp_session_info_t * session);

	sfp_returncode_t sfp_transfer_get_free_port(sfp_session_info_t * session);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SFP_TRANSFER_H__ */

// </ncouturier>

