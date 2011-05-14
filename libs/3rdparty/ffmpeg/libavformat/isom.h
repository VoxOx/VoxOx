/*
 * ISO Media common code
 * copyright (c) 2001 Fabrice Bellard.
 * copyright (c) 2002 Francois Revol <revol@free.fr>
 * copyright (c) 2006 Baptiste Coudurier <baptiste.coudurier@free.fr>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef FFMPEG_ISOM_H
#define FFMPEG_ISOM_H

/* isom.c */
extern const CodecTag ff_mov_obj_type[];

int ff_mov_iso639_to_lang(const char *lang, int mp4);
int ff_mov_lang_to_iso639(int code, char *to);

typedef struct Time2Sample{
    int count;
    int duration;
}Time2Sample;

#endif /* FFMPEG_ISOM_H */
