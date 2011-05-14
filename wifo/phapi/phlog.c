/*
 * phlog - logger for phapi
 *
 * Copyright (C) 2006 Wengo SAS
 * Author: Mathieu Stute <mathieu.stute@wengo.fr>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with dpkg; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "phlog.h"

#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
# include <stdarg.h>
# include <stdio.h>

/* something 'safe' */
#define STRING_LENGTH 240

FILE *log_file;

void init_log(char *file, char *mode)
{
  char error[STRING_LENGTH];
  static int count = 0;
  if(count)
  {
    return;
  }
  count++;

  /* open file for mode */
  if (!(log_file = fopen(file, mode)))
  {
    sprintf(error, "init_log() failed to open %s.\n", file);
    perror(error);
    /* message and exit on fail */
    exit(1);
  }
}

int logToFile(char *fmt, ...)
{
  va_list ap;                                /* special type for variable    */
  char format[STRING_LENGTH];                /* argument lists               */
  int count = 0;
  int i, j;                                  /* Need all these to store      */
  char c;                                    /* values below in switch       */
  double d;
  unsigned u;
  char *s;
  void *v;

  init_log(LOG_FILE, "w");

  va_start(ap, fmt);                         /* must be called before work   */
  while (*fmt)
  {
    for (j = 0; fmt[j] && fmt[j] != '%'; j++)
    {
      format[j] = fmt[j];                    /* not a format string          */
    }
    if (j)
    {
      format[j] = '\0';
      count += fprintf(log_file, format);    /* log it verbatim              */
      fmt += j;
    }
    else
    {
      for (j = 0; !isalpha(fmt[j]); j++)
      {   /* find end of format specifier */
        format[j] = fmt[j];
        /* special case printing '%' */
        if (j && fmt[j] == '%')
        {
          break;
        }
      }
      format[j] = fmt[j];                    /* finish writing specifier     */
      format[j + 1] = '\0';                  /* don't forget NULL terminator */
      fmt += j + 1;
      /* cases for all specifiers */
      switch (format[j])
      {
        case 'd':
        case 'i':                              /* many use identical actions   */
          i = va_arg(ap, int);                 /* process the argument         */
          count += fprintf(log_file, format, i); /* and log it                 */
          break;
        case 'o':
        case 'x':
        case 'X':
        case 'u':
          u = va_arg(ap, unsigned);
          count += fprintf(log_file, format, u);
          break;
        case 'c':
          c = (char) va_arg(ap, int);          /* must cast!                   */
          count += fprintf(log_file, format, c);
          break;
        case 's':
          s = va_arg(ap, char *);
          count += fprintf(log_file, format, s);
          break;
        case 'f':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
          d = va_arg(ap, double);
          count += fprintf(log_file, format, d);
          break;
        case 'p':
          v = va_arg(ap, void *);
          count += fprintf(log_file, format, v);
          break;
        case 'n':
          count += fprintf(log_file, "%d", count);
          break;
        case '%':
          count += fprintf(log_file, "%%");
          break;
        default:
          fprintf(stderr, "Invalid format specifier in log().\n");
      }
    }
  }

  va_end(ap);

  return count;
}
