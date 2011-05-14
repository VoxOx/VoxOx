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

#ifndef NULLHTTPREQUEST_H
#define NULLHTTPREQUEST_H

#include <http/IHttpRequest.h>

/**
 * Empty (null) HttpRequest implementation.
 *
 * @see HttpRequest
 * @author Tanguy Krotoff
 */
class NullHttpRequest : public IHttpRequest {
public:

	NullHttpRequest();

	int sendRequest(bool sslProtocol,
				const std::string & hostname,
				unsigned int hostPort,
				const std::string & path,
				const std::string & data,
				bool postMethod = false);

	void abort();

	void run();

private:
};

#endif	//NULLHTTPREQUEST_H
