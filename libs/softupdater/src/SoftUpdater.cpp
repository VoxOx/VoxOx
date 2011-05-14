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

#include <softupdater/SoftUpdater.h>

#include <util/Logger.h>
#include <util/File.h>
#include <util/String.h>
#include <util/SafeDelete.h>

#include <ctime>

//JRT-XXX //VOXOX - JRT - 2009.12.28 

#include <curl/curl.h>

int voxoxCurlLogCbk1(CURL *mycurl, curl_infotype type, char *buffer, size_t size, void * arg)
{
	bool logIt = false;
	std::string sType = "unknown";

	char* temp = new char[size+1];
	strncpy( temp, buffer, size );
	temp[size] = NULL;

	switch( type )
	{
	case CURLINFO_TEXT:				//0
		sType = "CURLINFO_TEXT";
		logIt = true;
		break;

	case CURLINFO_HEADER_IN:		//1
		sType = "CURLINFO_HEADER_IN";
		logIt = true;
		break;

	case CURLINFO_HEADER_OUT:		//2
		sType = "CURLINFO_HEADER_OUT";
		logIt = true;
		break;

	case CURLINFO_DATA_IN:			//3
		sType = "CURLINFO_DATA_IN";
		logIt = false;
		break;

	case CURLINFO_DATA_OUT:			//4
		sType = "CURLINFO_DATA_OUT";
		logIt = false;
		break;

	case CURLINFO_SSL_DATA_IN:		//5
		sType = "CURLINFO_SSL_DATA_IN";
		logIt = false;
		break;

	case CURLINFO_SSL_DATA_OUT:		//6
		sType = "CURLINFO_SSL_DATA_OUT";
		logIt = false;
		break;

	default:
		sType = "unknown";
		logIt = true;
	}

	if ( logIt )
	{
		LOG_INFO( sType + std::string( " - " ) + std::string( temp ) );
	}

	delete[] temp;

	return 0;
}

//---------------------------------------------------------------------

SoftUpdater::SoftUpdater(const std::string & url, const std::string & fileName) 
{
	_url		 = url;
	_fileName	 = fileName;
	_httpRequest = NULL;
}

SoftUpdater::~SoftUpdater() 
{
	OWSAFE_DELETE(_httpRequest);
}

void SoftUpdater::start() 
{
	if (_httpRequest) 
	{
		LOG_FATAL("file transfer already started");
		return;
	}

	_httpRequest = new HttpRequest();
	_httpRequest->dataReadProgressEvent += boost::bind(&SoftUpdater::dataReadProgressEventHandler, this, _1, _2, _3);
	_httpRequest->answerReceivedEvent   += boost::bind(&SoftUpdater::answerReceivedEventHandler,   this, _1, _2, _3, _4);

	_httpRequest->sendRequest( _url, String::null, false, (void*)voxoxCurlLogCbk1 );	//VOXOX - JRT - 2009.12.28 
}

void SoftUpdater::abort() 
{
	if (_httpRequest) 
	{
		_httpRequest->abort();
	}
}

void SoftUpdater::dataReadProgressEventHandler(int requestId, double bytesDone, double bytesTotal) 
{
	//-1 so that there is no divide by 0
	static const int startTime = time(NULL) - 1;

	int currentTime = time(NULL);

	unsigned int downloadSpeed = (unsigned int) bytesDone / (currentTime - startTime) / 1000;
	dataReadProgressEvent(bytesDone, bytesTotal, downloadSpeed);
}

void SoftUpdater::answerReceivedEventHandler(IHttpRequest * sender, int requestId, const std::string & answer, HttpRequest::Error error) 
{
	LOG_DEBUG("requestId=" + String::fromNumber(requestId) + " error=" + String::fromNumber(error));

	if (error == HttpRequest::NoError && !answer.empty()) 
	{
		#ifdef OS_WINDOWS
		FileWriter file(_fileName);
		#endif

		//VOXOX - ASV - 2009.12.02
		#ifdef OS_MACOSX
		std::string tempLocation = getenv("TMPDIR");
		tempLocation += _fileName;
		FileWriter file(tempLocation);
		#endif

		file.write(answer);
	}

	downloadFinishedEvent(error);
	//sender == _httpRequest
	/*delete sender;
	sender = NULL;*/
	//OWSAFE_DELETE(_httpRequest);
}
