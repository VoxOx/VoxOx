/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
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

#include "FtpUpload.h"

#include <cutil/global.h>
#include <curl/curl.h>
#include <sys/stat.h>

#ifdef OS_WINDOWS
	#define snprintf _snprintf
	#define stat _stat
#else
	#include <cstring>
#endif

int curlFTPProgress(void * instance, double dltotal, double dlnow, double ultotal, double ulnow) {

	if (instance) {
		FtpUpload * ftpUpload = (FtpUpload*) instance;
		ftpUpload->setProgress(ultotal, ulnow);
	}
	return 0;
}

char * get_filename(const char * full) {
	char * filename;
	char * begin = (char *) full;

	if (!full) {
		return NULL;
	}

	filename = begin + strlen(begin) - 1;

	while (filename != begin && *filename != '\\' && *filename != '/') {
		filename--;
	}

	if (filename == begin) {
		return begin;
	} else {
		return ++filename;
	}
}

int ftp_upload(const char * host, const char * login, const char * password,
	const char * path, const char * fullfilename, void * ftpUploadInstance) {

	CURL * handle;
	char url_buff[1024];
	char auth_passwd[1024];
	char tmp[1024];
	const char * filename;
	FILE * lfile;
	struct stat buf;
	int res;

	memset(url_buff, 0, sizeof(url_buff));
	memset(auth_passwd, 0, sizeof(auth_passwd));
	memset(tmp, 0, sizeof(tmp));

	if (!fullfilename) {
		return -1;
	}

	stat(fullfilename, &buf);
	filename = get_filename(fullfilename);

	snprintf(url_buff, sizeof(url_buff), "ftp://%s", host);

	if (path && *path) {
		snprintf(tmp, sizeof(tmp), "%s/%s", url_buff, path);
	}

	snprintf(url_buff, sizeof(url_buff), "%s/%s", tmp, filename);

	snprintf(auth_passwd, sizeof(auth_passwd), "%s:%s", login, password);

	curl_global_init(CURL_GLOBAL_ALL);

	lfile = fopen(fullfilename, "rb");

	handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, url_buff);
	curl_easy_setopt(handle, CURLOPT_USERPWD, auth_passwd);
	curl_easy_setopt(handle, CURLOPT_READDATA, lfile);
	curl_easy_setopt(handle, CURLOPT_INFILESIZE, buf.st_size);
	curl_easy_setopt(handle, CURLOPT_UPLOAD, 1);
	curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(handle, CURLOPT_FTP_USE_EPSV, 0);
	curl_easy_setopt(handle, CURLOPT_FTP_SKIP_PASV_IP, 1);

	curl_easy_setopt(handle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);	//VOXOX - JRT - 2009.07.25 
	

	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, curlFTPProgress);
	curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, ftpUploadInstance);

	res = curl_easy_perform(handle);

	curl_easy_cleanup(handle);

	fclose(lfile);

	curl_global_cleanup();

	return res;
}
