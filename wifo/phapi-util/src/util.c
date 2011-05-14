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

#include <phapi-util/util.h>

#include <phapi-util/mystring.h>

#include <stdio.h>
#include <assert.h>

static int xmlLittleEndian = 1;

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
unsigned int msscanf(char ** text, char * format, ...){
	int i;
	va_list ap;
	char * buf_text;
	char * buf_format;
	int inter_format_length;
	int inter_text_length;
	char ** s_arg;

	if(text == NULL || *text == NULL || strlen(*text) <= 0 || format == NULL || strlen(format)<= 0){
		return FALSE;
	}

	va_start(ap, format);

	while(*format != '\0'){
		if ( *format == '%' ) {
			switch (*++format) {
				case '%' :
					break;
				case 'c' :
					break;
				case 'd' :
					break;
				case 'f' :
					break;
				case 's' :
					buf_text = *text;
					buf_format = ++format;
					inter_format_length = 0;
					// go to the next format token
					while(*buf_format != '\0' && (*buf_format != '%' && *(buf_format+1) != 's')){
						inter_format_length++;
						buf_format++;
					}
					// rewinds
					buf_format -= inter_format_length;
					// compare format and text until it matches, by shifting the text
					inter_text_length = 0;
					while(*buf_text != '\0'){
						if(strncmp(buf_format, buf_text, inter_format_length) != 0){
							for(i=1; i<=inter_format_length; i++){ // check that we can shift the text buffer enough
								if(*(buf_text+i) == '\0'){
									break;
								}
							}
							buf_text++;
							inter_text_length++;
						}else{
							break;
						}
					}
					// copies the found string into a given parameter
					if(inter_text_length > 0){
						s_arg = va_arg(ap, char **);
						*s_arg = (char *)malloc((inter_text_length+1) * sizeof(char));
						strncpy(*s_arg, *text, inter_text_length);
						(*s_arg)[inter_text_length] = '\0';
						*text += inter_text_length;
					}
					break;
			}
		}else{
			if(*format != **text){
				return FALSE; // TODO errorcode
			}
			format++;
			(*text)++;
		}
	}

	va_end(ap);

	return TRUE;
}

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
unsigned int sscanf2(const char * buffer, char * format, ...){
	int i;
	int res;
	va_list ap;
	char * buf_text;
	char * buf_format;
	int inter_format_length;
	int inter_text_length;
	char * s_arg;
	int s_arg_size;
	char ** text;

	if(buffer == NULL || strlen(buffer) <= 0 || format == NULL || strlen(format)<= 0){
		return FALSE;
	}

	text = (char **)&buffer;

	va_start(ap, format);

	while(*format != '\0'){
		if ( *format == '%' ) {
			switch (*++format) {
				case '%' :
					break;
				case 'c' :
					break;
				case 'd' :
					break;
				case 'f' :
					break;
				case 's' :
					buf_text = *text;
					buf_format = ++format;
					inter_format_length = 0;
					// go to the next format token
					while(*buf_format != '\0' && (*buf_format != '%' && *(buf_format+1) != 's')){
						inter_format_length++;
						buf_format++;
					}
					// rewinds
					buf_format -= inter_format_length;
					// compare format and text until it matches, by shifting the text
					inter_text_length = 0;
					if(strlen(buf_format) > 0){ // if not at the end of format
						while(*buf_text != '\0'){
							if((res = strncmp(buf_format, buf_text, inter_format_length)) != 0){
								for(i=1; i<=inter_format_length; i++){ // check that we can shift the text buffer enough
									if(*(buf_text+i) == '\0'){ // we arrived at the end of the text : put it all in the variable
										break;
									}
								}
								buf_text++;
								inter_text_length++;
							}else{
								break;
							}
						}
					}else{ // if at the end of format, the rest of text goes in the last variable
						inter_text_length = (int)strlen(buf_text);
					}
					// copies the found string into a given parameter
					if(inter_text_length >= 0){
						s_arg = va_arg(ap, char *);
						s_arg_size = va_arg(ap, int);
						//
						// CHECK THAT THE SIZE DOES NOT EXEEDS DESTINATION BUFFER SIZE
						//
						if(s_arg_size < inter_text_length+1){
							return FALSE; // TODO errorcode
						}
						strncpy(s_arg, *text, inter_text_length);
						(s_arg)[inter_text_length] = '\0';
						*text += inter_text_length;
					}
					break;
			}
		}else{
			if(*format != **text){
				return FALSE; // TODO errorcode
			}
			format++;
			(*text)++;
		}
	}

	va_end(ap);

	return TRUE;
}

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
unsigned int sscanf3(char ** text, char * format, ...){
	int i;
	int res;
	va_list ap;
	char * buf_text;
	char * buf_format;
	int inter_format_length;
	int inter_text_length;
	char * s_arg;
	int s_arg_size;

	if(text == NULL || *text == NULL || strlen(*text) <= 0 || format == NULL || strlen(format)<= 0){
		return FALSE;
	}

	va_start(ap, format);

	while(*format != '\0'){
		if ( *format == '%' ) {
			switch (*++format) {
				case '%' :
					break;
				case 'c' :
					break;
				case 'd' :
					break;
				case 'f' :
					break;
				case 's' :
					buf_text = *text;
					buf_format = ++format;
					inter_format_length = 0;
					// go to the next format token
					while(*buf_format != '\0' && (*buf_format != '%' && *(buf_format+1) != 's')){
						inter_format_length++;
						buf_format++;
					}
					// rewinds
					buf_format -= inter_format_length;
					// compare format and text until it matches, by shifting the text
					inter_text_length = 0;
					if(strlen(buf_format) > 0){ // if not at the end of format
						while(*buf_text != '\0'){
							if((res = strncmp(buf_format, buf_text, inter_format_length)) != 0){
								for(i=1; i<=inter_format_length; i++){ // check that we can shift the text buffer enough
									if(*(buf_text+i) == '\0'){
										break;
									}
								}
								buf_text++;
								inter_text_length++;
							}else{
								break;
							}
						}
					}else{ // if at the end of format, the rest of text goes in the last variable
						inter_text_length = (int)strlen(buf_text);
					}
					// copies the found string into a given parameter
					if(inter_text_length >= 0){
						s_arg = va_arg(ap, char *);
						s_arg_size = va_arg(ap, int);
						//
						// CHECK THAT THE SIZE DOES NOT EXEEDS DESTINATION BUFFER SIZE
						//
						if(s_arg_size < inter_text_length+1){
							return FALSE; // TODO errorcode
						}
						strncpy(s_arg, *text, inter_text_length);
						(s_arg)[inter_text_length] = '\0';
						*text += inter_text_length;
					}
					break;
			}
		}else{
			if(*format != **text){
				return FALSE; // TODO errorcode
			}
			format++;
			(*text)++;
		}
	}

	va_end(ap);

	return TRUE;
}

/**
* Checks wether a string contains something or not
*
* @param	[in]	string : the string to check
* @return	TRUE if the string is not empty; FALSE else
*/
unsigned int strfilled(const char * string){
	if(string != NULL && strlen(string) > 0){
		return TRUE;
	}
	return FALSE;
}

/**
* Indicates wether the strings are the same or not.
*
* @param	[in]	s1 : a string
* @param	[in]	s2 : another string
* @return	TRUE if the strings are the same; FALSE else.
*/
unsigned int strequals(const char * s1, const char * s2){
	if(!strcmp(s1, s2)){
		return TRUE;
	}
	return FALSE;
}

/**
* Indicates wether the strings is of one of the given values.
*
* @param	[in]	s1 : a string
* @param	[in]	nb_values : the number of given values to check
* @param	[in]	... : the values to check (must all be char *)
* @return	TRUE if the strings are the same; FALSE else.
*/
unsigned int strequalsamong(const char * s1, unsigned int nb_values, ...){
	va_list ap;

	if(nb_values == 0){
		return FALSE;
	}

	va_start(ap, nb_values);

	while(nb_values-- > 0){
		if(strcmp(s1, va_arg(ap, char *)) == 0){
			return TRUE;
		}
	}

	return FALSE;
}

/**
* Find the string that represent the max int value between two strings
* that represent integer values. Equivalent to a max between two int.
*
* @param	[in]	s1 : a string that represent an integer value
* @param	[in]	s2 : another string that represent an integer value
* @return	the string that represents the max integer value between s1 and s2;
*			NULL if an error occurs
*/
const char * str_int_max(const char * int1, const char * int2){
	int i1, i2;

	if(!strfilled(int1) || !strfilled(int2)){
		return NULL;
	}

	i1 = atoi(int1);
	i2 = atoi(int2);

	if(i1 == 0 && !strequals(int1, "0")){ // an error occured
		return NULL;
	}

	if(i2 == 0 && !strequals(int2, "0")){ // an error occured
		return NULL;
	}

	if(i1 > i2){
		return int1;
	}
	return int2;
}

/**
* Find the string that represent the min int value between two strings
* that represent integer values. Equivalent to a min between two int
*
* @param	[in]	s1 : a string that represent an integer value
* @param	[in]	s2 : another string that represent an integer value
* @return	the string that represents the min integer value between s1 and s2;
*			NULL if an error occurs
*/
const char * str_int_min(const char * int1, const char * int2){
	int i1, i2;

	if(!strfilled(int1) || !strfilled(int2)){
		return NULL;
	}

	i1 = atoi(int1);
	i2 = atoi(int2);

	if(i1 == 0 && !strequals(int1, "0")){ // an error occured
		return NULL;
	}

	if(i2 == 0 && !strequals(int2, "0")){ // an error occured
		return NULL;
	}

	if(i1 < i2){
		return int1;
	}
	return int2;
}

/**
* Calculates the length of a 'printf-style' format whithout considering
* the special tokens (%c, %d, %f, %s)
*
* @param	[in]	format : the 'printf-style' format
* @return	the length of the string format whithout considering the special tokens
*/
unsigned int format_length_without_tokens(char * format){
	char * tmp = format;
	unsigned int length = 0;

	if(!strfilled(format)){
		return 0;
	}

	while(*tmp != '\0'){
		if(!(*tmp == '%' && *(tmp+1) != '\0' && (*(tmp+1) != 'c' || *(tmp+1) != 'd' || *(tmp+1) != 'f' || *(tmp+1) != 's'))){
			length++;
		}else{
			tmp++;
		}
		tmp++;
	}

	return length;
}

/**
* Simple log error. Simply print on stdout for the moment...
*
* @param	[in]	message, the error message to be displayed
* @param	[in]	function, the function name in which the error happened
* @param	[in]	filename, the filename from which originates the error (should be passed __FILE__)
* @param	[in]	line_number, the line of the file where the error happened (should be passed __LINE__)
*/
void phapi_log(const char * type, const char * message, const char * function, const char * filename, unsigned int line_number){
	if(strfilled(type) && strfilled(message) && strfilled(function) && strfilled(filename))
		printf("%s : %s (in %s(...) of file %s at line %d)\n", type, message, function, filename, line_number);
	//else ??
}

void itostr(int value, char * buffer, const size_t sizeof_buffer, int radix){
	char HEX[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	unsigned int cpt=0;
	unsigned int cpt2;
	int y;
	int reste;
	char * temp = malloc(sizeof_buffer * sizeof(char));
	int k;
	int l;

	while(value > 0){
		y = value;
		value = (int) y / radix;
		reste = y - value * radix;
		if(cpt >= sizeof_buffer){
			break;
		}
		temp[cpt] = HEX[reste];
		cpt++;
	}
	buffer[cpt]='\0';

	cpt2 = cpt;
	//inverse le sens de chaine
	while((k = cpt+(cpt2-cpt-1)) >= 0){
		l = cpt-cpt2;
		buffer[l] = temp[k];
		cpt2--;
	}

	free(temp);
}

int UTF8ToUTF16LE(unsigned char* outb, int *outlen, const unsigned char* in, int *inlen) {
	unsigned short* out = (unsigned short*) outb;
	const unsigned char* processed = in;
	unsigned short* outstart= out;
	unsigned short* outend;
	const unsigned char* inend= in+*inlen;
	unsigned int c, d;
	int trailing;
	unsigned char *tmp;
	unsigned short tmp1, tmp2;

	if (in == NULL) {
		/*
		* initialization, add the Byte Order Mark
		*/
		if (*outlen >= 2) {
			outb[0] = 0xFF;
			outb[1] = 0xFE;
			*outlen = 2;
			*inlen = 0;
#ifdef DEBUG_ENCODING
			xmlGenericError(xmlGenericErrorContext,
				"Added FFFE Byte Order Mark\n");
#endif
			return(2);
		}
		*outlen = 0;
		*inlen = 0;
		return(0);
	}
	outend = out + (*outlen / 2);
	while (in < inend) {
		d= *in++;
		if      (d < 0x80)  { c= d; trailing= 0; }
		else if (d < 0xC0) {
			/* trailing byte in leading position */
			*outlen = (out - outstart) * 2;
			*inlen = processed - in;
			return(-2);
		} else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
		else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
		else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
		else {
			/* no chance for this in UTF-16 */
			*outlen = (out - outstart) * 2;
			*inlen = processed - in;
			return(-2);
		}

		if (inend - in < trailing) {
			break;
		}

		for ( ; trailing; trailing--) {
			if ((in >= inend) || (((d= *in++) & 0xC0) != 0x80))
				break;
			c <<= 6;
			c |= d & 0x3F;
		}

		/* assertion: c is a single UTF-4 value */
		if (c < 0x10000) {
			if (out >= outend)
				break;
			if (xmlLittleEndian) {
				*out++ = c;
			} else {
				tmp = (unsigned char *) out;
				*tmp = c ;
				*(tmp + 1) = c >> 8 ;
				out++;
			}
		}
		else if (c < 0x110000) {
			if (out+1 >= outend)
				break;
			c -= 0x10000;
			if (xmlLittleEndian) {
				*out++ = 0xD800 | (c >> 10);
				*out++ = 0xDC00 | (c & 0x03FF);
			} else {
				tmp1 = 0xD800 | (c >> 10);
				tmp = (unsigned char *) out;
				*tmp = (unsigned char) tmp1;
				*(tmp + 1) = tmp1 >> 8;
				out++;

				tmp2 = 0xDC00 | (c & 0x03FF);
				tmp = (unsigned char *) out;
				*tmp  = (unsigned char) tmp2;
				*(tmp + 1) = tmp2 >> 8;
				out++;
			}
		}
		else
			break;
		processed = in;
	}
	*outlen = (out - outstart) * 2;
	*inlen = processed - in;
	return(0);
}

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
int asciiToUTF8(unsigned char* out, int *outlen, const unsigned char* in, int *inlen) {
	unsigned char* outstart = out;
	const unsigned char* base = in;
	const unsigned char* processed = in;
	unsigned char* outend = out + *outlen;
	const unsigned char* inend;
	unsigned int c;
	int bits;

	inend = in + (*inlen);
	while ((in < inend) && (out - outstart + 5 < *outlen)) {
		c= *in++;

		/* assertion: c is a single UTF-4 value */
		if (out >= outend)
			break;
		if      (c <    0x80) {  *out++=  c;                bits= -6; }
		else {
			*outlen = out - outstart;
			*inlen = processed - base;
			return(-1);
		}

		for ( ; bits >= 0; bits-= 6) {
			if (out >= outend)
				break;
			*out++= ((c >> bits) & 0x3F) | 0x80;
		}
		processed = (const unsigned char*) in;
	}
	*outlen = out - outstart;
	*inlen = processed - base;
	return(0);
}

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
int UTF8Toascii(unsigned char* out, int *outlen, const unsigned char* in, int *inlen) {
	const unsigned char* processed = in;
	const unsigned char* outend;
	const unsigned char* outstart = out;
	const unsigned char* instart = in;
	const unsigned char* inend;
	unsigned int c, d;
	int trailing;

	if (in == NULL) {
		/*
		* initialization nothing to do
		*/
		*outlen = 0;
		*inlen = 0;
		return(0);
	}
	inend = in + (*inlen);
	outend = out + (*outlen);
	while (in < inend) {
		d = *in++;
		if      (d < 0x80)  { c= d; trailing= 0; }
		else if (d < 0xC0) {
			/* trailing byte in leading position */
			*outlen = out - outstart;
			*inlen = processed - instart;
			return(-2);
		} else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
		else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
		else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
		else {
			/* no chance for this in Ascii */
			*outlen = out - outstart;
			*inlen = processed - instart;
			return(-2);
		}

		if (inend - in < trailing) {
			break;
		}

		for ( ; trailing; trailing--) {
			if ((in >= inend) || (((d= *in++) & 0xC0) != 0x80))
				break;
			c <<= 6;
			c |= d & 0x3F;
		}

		/* assertion: c is a single UTF-4 value */
		if (c < 0x80) {
			if (out >= outend)
				break;
			*out++ = c;
		} else {
			/* no chance for this in Ascii */
			*outlen = out - outstart;
			*inlen = processed - instart;
			return(-2);
		}
		processed = in;
	}
	*outlen = out - outstart;
	*inlen = processed - instart;
	return(0);
}


/*
 * Get the host name and the port from a string of the form host:port
 */
char *
ph_split_host_port(char *buf, int bufsize, const char *host, int *port)
{
	char *s;

	assert(buf != 0);
	assert(port != 0);

	*port = 0;

	if (!host) {
		return 0;
	}

	if (!strchr(host, ':')) {
		strncpy(buf, host, bufsize);
	}
	else {
		strncpy(buf, host, bufsize);
		s = strchr(buf, ':');
		if (s)
		{
			*( char *) s = 0;
			*port = atoi(s+1);
		}
	}

	return buf;
}

void wclean_filename_for_windows(wchar_t * filename, size_t sizeof_filename) {
	int i = 0;
	int j = 0;
	wchar_t retirer[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'};

	for(i = 0; i < (int)sizeof_filename; i++) {
		for(j=0; j < (int)sizeof(retirer)/2; j++) {
			if(filename[i] == retirer[j]) {
				filename[i] = '_';
			}
		}
	}
}

void clean_filename_for_windows(char * filename, size_t sizeof_filename) {
	int i = 0;
	int j = 0;
	char retirer[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'};

	for(i = 0; i < (int)sizeof_filename; i++) {
		for(j=0; j < (int)sizeof(retirer); j++) {
			if(filename[i] == retirer[j]) {
				filename[i] = '_';
			}
		}
	}
}
