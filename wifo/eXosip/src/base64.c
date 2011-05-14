/*
 * Open Wengo SIP stack
 * Copyright (C) 2007  Wengo
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

/*
 * adapted from Bob Trower work: http://base64.sourceforge.net/b64.c
 */

#include "eXosip2.h"

#include <stdlib.h>

/*
** Translation Table as described in RFC1113
*/
static const char cb64 [] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;

/*
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
static void base64_encode_block
(
	unsigned char in [3],
	int in_length,
	unsigned char out [4]
)
{
	out [0] = cb64 [in [0] >> 2] ;
	out [1] = cb64 [((in [0] & 0x03) << 4) | ((in [1] & 0xf0) >> 4)] ;
	out [2] = (unsigned char) (in_length > 1 ?
		cb64 [((in [1] & 0x0f) << 2) | ((in [2] & 0xc0) >> 6)] :
		'=') ;
	out [3] = (unsigned char) (in_length > 2 ?
		cb64 [in [2] & 0x3f] :
		'=') ;
}

/*
** base64 encode a stream adding padding and line breaks as per spec.
*/
int base64_encode
(
	const void * in,
	int in_length,
	char * out,
	int out_size,
	int line_size
)
{
	int in_index = 0, out_index = 0 ;
	int block_count, block_index ;
	unsigned char in_block [3], out_block [4] ;
	int in_block_index, in_block_length ;
	int out_block_index ;

	if (line_size > 0 && line_size < 4)
	{
		return -1 ;
	}

	block_count = (in_length + 2) / 3 ;
	if (out_size < block_count * 4 + 1)
	{
		return -1 ;
	}

	for (block_index = 0 ; block_index < block_count ; block_index ++)
	{
		if
		(
			line_size > 0 &&
			block_index > 0 &&
			block_index % (line_size / 4) == 0
		)
		{
			out [out_index ++] = '\r' ;
			out [out_index ++] = '\n' ;
		}

		in_block_length = 0 ;
		for (in_block_index = 0 ; in_block_index < 3 ; in_block_index ++)
		{
			if (in_index < in_length)
			{
				in_block [in_block_index] = ((unsigned char *) in) [in_index] ;
				in_block_length ++ ;
			}
			else
			{
				in_block [in_block_index] = 0 ;
			}
			in_index ++ ;
		}

		base64_encode_block (in_block, in_block_length, out_block) ;

		for (out_block_index = 0 ; out_block_index < 4 ; out_block_index ++)
		{
			out [out_index ++] = out_block [out_block_index] ;
		}
	}
	out [out_index ++] = 0 ;

	return out_index ;
}
