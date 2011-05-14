/*
** Copyright (C) 2002-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/

/* 
** MSVC++ assumes that all floating point constants without a trailing 
** letter 'f' are double precision. 
**
** If this assumption is incorrect and one of these floating point constants
** is assigned to a float variable MSVC++ generates a warning.
**
** Since there are currently about 25000 of these warnings generated in
** src/src_sinc.c this slows down compile times considerably. The 
** following #pragma disables the warning.
*/

#pragma warning(disable: 4305)

/*----------------------------------------------------------------------------
** Normal #defines follow.
*/

/* Target processor clips on negative float to int conversion. */
#define CPU_CLIPS_NEGATIVE 1

/* Target processor clips on positive float to int conversion. */
#define CPU_CLIPS_POSITIVE 0

/* Name of package */
#define PACKAGE "libsamplerate"

/* Version number of package */
#define VERSION "0.1.2"

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG 4

