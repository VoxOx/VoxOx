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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <phapi-util/phapiutildll.h>

#include <stdarg.h>
#include <stdlib.h>

/*
#ifdef WIN32
#if defined(BUILD_UTILITIES_DLL)
#define PHAPIUTIL_API __declspec(dllexport)
#elif defined(USE_UTILITIES_DLL)
#define PHAPIUTIL_API __declspec(dllimport)
#endif
#endif

#ifndef PHAPIUTIL_API
#define PHAPIUTIL_API
#endif
*/

#ifndef TRUE
	#define TRUE	1
#endif /* TRUE */
#ifndef FALSE
	#define FALSE	0
#endif /*FALSE*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


	/**
	*
	* WARNING : needs improvements
	*
	* Makes the same as sscanf but allocates memory for the variables.
	* There need to be as much variables as specified parsing tokens.
	* For the moment, the only token supported is '%s' for strings.
	*
	* @param	[in]	text : the text to parse
	* @param	[in]	format : the format for parsing (same syntax as printf, scanf, ...)
	* @param	[out]	... : variables being allocated and receiving the extracted values
	* @return	TRUE if succeeds; FALSE else
	*/
	PHAPIUTIL_API unsigned int msscanf(char ** text, char * format, ...);

	/**
	*
	* WARNING : needs improvements
	*
	* Does exactly the same as sscanf but if we give it "%s/%s" as the format and "a/b" as buffer
	* and finally two char s[2] as variables, we will get the first s="a" and the second="b" which
	* does not work with sscanf!
	* For the moment, the only token supported is '%s' for strings.
	* NOTE: after a %s, the size of the string variable must be given in argument
	* for example
	* <pre>
	*	int i;
	*	char[SIZE] string;
	*	sscanf2("25 bazaaar", "%d %s", i, string, sizeof(string));
	* </pre>
	*
	* @param	[in]	buffer : the text to parse
	* @param	[in]	format : the format for parsing (same syntax as printf, scanf, ...)
	* @param	[out]	... : variables already allocated and receiving the extracted values
	* @return	TRUE if succeeds; FALSE else
	*/
	PHAPIUTIL_API unsigned int sscanf2(const char * buffer, char * format, ...);

	/**
	*
	* WARNING : needs improvements
	*
	* Does exactly the same as sscanf but if we give it "%s/%s" as the format and "a/b" as buffer
	* and finally two char s[2] as variables, we will get the first s="a" and the second="b" which
	* does not work with sscanf!
	* For the moment, the only token supported is '%s' for strings.
	* NOTE: after a %s, the size of the string variable must be given in argument
	* for example
	* <pre>
	*	int i;
	*	char[SIZE] string;
	*	sscanf2("25 bazaaar", "%d %s", i, string, sizeof(string));
	* </pre>
	*
	* @param	[in-out]	text : the text to parse
	* @param	[in]	format : the format for parsing (same syntax as printf, scanf, ...)
	* @param	[out]	... : variables already allocated and receiving the extracted values
	* @return	TRUE if succeeds; FALSE else
	*/
	PHAPIUTIL_API unsigned int sscanf3(char ** text, char * format, ...);

	/**
	* Checks wether a string contains something or not
	*
	* @param	[in]	string : the string to check
	* @return	TRUE if the string is not empty; FALSE else
	*/
	PHAPIUTIL_API unsigned int strfilled(const char * string);

	/**
	* Indicates wether the strings are the same or not.
	*
	* @param	[in]	s1 : a string
	* @param	[in]	s2 : another string
	* @return	TRUE if the strings are the same; FALSE else.
	*/
	PHAPIUTIL_API unsigned int strequals(const char * s1, const char * s2);

	/**
	* Indicates wether the strings is of one of the given values.
	*
	* @param	[in]	s1 : a string
	* @param	[in]	nb_values : the number of given values to check
	* @param	[in]	... : the values to check
	* @return	TRUE if the strings are the same; FALSE else.
	*/
	PHAPIUTIL_API unsigned int strequalsamong(const char * s1, unsigned int nb_values, ...);

	/**
	* Find the string that represent the max int value between two strings
	* that represent integer values. Equivalent to a max between two int.
	*
	* @param	[in]	s1 : a string that represent an integer value
	* @param	[in]	s2 : another string that represent an integer value
	* @return	the string that represents the max integer value between s1 and s2;
	*			NULL if an error occurs
	*/
	PHAPIUTIL_API const char * str_int_max(const char * int1, const char * int2);

	/**
	* Find the string that represent the min int value between two strings
	* that represent integer values. Equivalent to a min between two int
	*
	* @param	[in]	s1 : a string that represent an integer value
	* @param	[in]	s2 : another string that represent an integer value
	* @return	the string that represents the min integer value between s1 and s2;
	*			NULL if an error occurs
	*/
	PHAPIUTIL_API const char * str_int_min(const char * int1, const char *int2);

	/**
	* Calculates the length of a 'printf-style' format whithout considering
	* the special tokens (%c, %d, %f, %s)
	*
	* @param	[in]	format : the 'printf-style' format
	* @return	the length of the string format whithout considering the special tokens
	*/
	PHAPIUTIL_API unsigned int format_length_without_tokens(char * format);

	/**
	* Simple log error. Simply print on stdout for the moment...
	*
	* @param	[in]	message, the error message to be displayed
	* @param	[in]	function, the function name in which the error happened
	* @param	[in]	filename, the filename from which originates the error (should be passed __FILE__)
	* @param	[in]	line_number, the line of the file where the error happened (should be passed __LINE__)
	*/
	PHAPIUTIL_API void phapi_log(const char * type, const char * message, const char * function, const char * filename, unsigned int line_number);
	#define m_log(message, function_name) phapi_log("INFO", message, function_name, __FILE__, __LINE__)
	#define m_log_error(message, function_name) phapi_log("ERROR", message, function_name, __FILE__, __LINE__)

	PHAPIUTIL_API void itostr(int value, char * buffer, const size_t sizeof_buffer, int radix);

	/**
	 * UTF8ToUTF16LE:
	 * @outb:  a pointer to an array of bytes to store the result
	 * @outlen:  the length of @outb
	 * @in:  a pointer to an array of UTF-8 chars
	 * @inlen:  the length of @in
	 *
	 * Take a block of UTF-8 chars in and try to convert it to an UTF-16LE
	 * block of chars out.
	 *
	 * Returns the number of byte written, or -1 by lack of space, or -2
	 *     if the transcoding failed.
	 */
	PHAPIUTIL_API int UTF8ToUTF16LE(unsigned char* outb, int *outlen, const unsigned char* in, int *inlen);

	/**
	* asciiToUTF8:
	* @out:  a pointer to an array of bytes to store the result
	* @outlen:  the length of @out
	* @in:  a pointer to an array of ASCII chars
	* @inlen:  the length of @in
	*
	* Take a block of ASCII chars in and try to convert it to an UTF-8
	* block of chars out.
	* Returns 0 if success, or -1 otherwise
	* The value of @inlen after return is the number of octets consumed
	*     as the return value is positive, else unpredictable.
	* The value of @outlen after return is the number of ocetes consumed.
	*/
	PHAPIUTIL_API int asciiToUTF8(unsigned char* out, int *outlen, const unsigned char* in, int *inlen);

	/**
	* UTF8Toascii:
	* @out:  a pointer to an array of bytes to store the result
	* @outlen:  the length of @out
	* @in:  a pointer to an array of UTF-8 chars
	* @inlen:  the length of @in
	*
	* Take a block of UTF-8 chars in and try to convert it to an ASCII
	* block of chars out.
	*
	* Returns 0 if success, -2 if the transcoding fails, or -1 otherwise
	* The value of @inlen after return is the number of octets consumed
	*     as the return value is positive, else unpredictable.
	* The value of @outlen after return is the number of ocetes consumed.
	*/
	PHAPIUTIL_API int UTF8Toascii(unsigned char* out, int *outlen, const unsigned char* in, int *inlen);

	PHAPIUTIL_API char *ph_split_host_port(char *buf, int bufsize, const char *host, int *port);

	/**
	 * Replace characters : / \ ? * " < > by a _. Useful for filenames under windows
	 *
	 * @param	filename	the filename to clean
	 * @param	sizeof_filename	the size of the string filename
	 */
	PHAPIUTIL_API void clean_filename_for_windows(char * filename, size_t sizeof_filename);

	/**
	 * Replace characters : / \ ? * " < > by a _. Useful for filenames under windows
	 *
	 * @param	filename	the filename to clean
	 * @param	sizeof_filename	the size of the string filename
	 */
	PHAPIUTIL_API void wclean_filename_for_windows(wchar_t * filename, size_t sizeof_filename);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UTIL_H__ */
