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

#ifndef OWFTPUPLOAD_H
#define OWFTPUPLOAD_H

#include <thread/Thread.h>
#include <util/Event.h>

#include <string>

int ftp_upload(const char * host, const char * login, const char *pwd,
	const char * path, const char * fullfilename, void * instance);

/**
 * FtpUpload upload a file to a ftp server.
 *
 * @author Julien Bossart
 * @author Mathieu Stute
 */
class FtpUpload : public Thread {
public:

	enum Status {
		Ok,
		Error,
		None,
	};

	/**
	 * Progression event.
	 *
	 * @param sender this class
	 * @param requestId HTTP request ID
	 * @param answer HTTP answer (std::string is used as a byte array)
	 * @param error Error code
	 */
	Event<void (FtpUpload * sender, Status status)> statusEvent;

	/**
	 * Progression event.
	 *
	 * @param sender this class
	 * @param ultotal total
	 * @param ulnow current
	 */
	Event<void (FtpUpload * sender, double ultotal, double ulnow)> progressionEvent;

	/**
	 * Constructor.
	 *
	 * @param host hostname of the ftp server.
	 * @param login login used for connexion.
	 * @param password password used for connexion.
	 * @param path path on the ftp server where to upload the file.
	 * @param filename file to upload.
	 */
	FtpUpload(const std::string & host, const std::string & login, const std::string & password,
		const std::string & path, const std::string & filename)
		: _host(host), _login(login), _password(password), _path(path), _filename(filename) {
	}

	const std::string getHost() const { return _host; }

	const std::string getLogin() const { return _login; }

	const std::string getPassword() const { return _password; }

	const std::string getPath() const { return _path; }

	const std::string getFilename() const { return _filename; }

	void setProgress(double ultotal, double ulnow) {
		progressionEvent(this, ultotal, ulnow);
	}

	void run() {
		int res = ftp_upload(_host.c_str(), _login.c_str(), _password.c_str(), 
			_path.c_str(), _filename.c_str(), this);

		if (res == 0) {
			statusEvent(this, Ok);
		} else {
			statusEvent(this, Error);
		}
	}

private:

	std::string _host;

	std::string _login;

	std::string _password;

	std::string _path;

	std::string _filename;
};

#endif	//OWFTPUPLOAD_H
