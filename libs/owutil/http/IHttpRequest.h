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

#ifndef OWIHTTPREQUEST_H
#define OWIHTTPREQUEST_H

#include <util/Event.h>

#include <thread/Thread.h>

#include <string>

//=============================================================================

//VOXOX - JRT - 2009.12.23 - Let's use a class so we can enhance parameters on sendRequest() more easily.
class CurlHttpRequestParms
{
public:
	CurlHttpRequestParms()
	{
		initVars();
	}

	//Gets -----------------------------------------------------------------------
	bool			getSslProtocol()	const	{ return _sslProtocol;	}
	std::string		getHostName()		const	{ return _hostname;		}
	unsigned int	getHostPort()		const	{ return _hostPort;		}
	std::string		getPath()			const	{ return _path;			}
	std::string		getData()			const	{ return _data;			}
	std::string		getSslRandomFile()	const	{ return _sslRandomFile;}
	bool			getPostMethod()		const	{ return _postMethod;	}
	void*			getCurlLogCbk()		const	{ return _curlLogCbk;	}

	//Sets -----------------------------------------------------------------------
	void setSslProtocol	 ( bool				 val )			{ _sslProtocol	 = val;	}
	void setHostName	 ( const std::string val )			{ _hostname		 = val;	}
	void setHostPort	 ( unsigned int		 val )			{ _hostPort		 = val;	}
	void setPath		 ( const std::string val )			{ _path			 = val;	}
	void setData		 ( const std::string val )			{ _data			 = val;	}
	void setSslRandomFile( const std::string val )			{ _sslRandomFile = val; }
	void setPostMethod	 ( bool				 val )			{ _postMethod	 = val;	}
	void setCurlLogCbk	 ( void*			 val )			{ _curlLogCbk	 = val;	}

	CurlHttpRequestParms operator=( const CurlHttpRequestParms& src )
	{
		if ( this != &src )
		{
			setSslProtocol  ( src.getSslProtocol()   );
			setHostName	    ( src.getHostName()	     );
			setHostPort	    ( src.getHostPort()	     );
			setPath		    ( src.getPath()		     );
			setData		    ( src.getData()		     );
			setSslRandomFile( src.getSslRandomFile() );
			setPostMethod	( src.getPostMethod()    );
			setCurlLogCbk   ( src.getCurlLogCbk()	 );
		}

		return *this;
	}

	void initVars()
	{
		setSslProtocol  ( false );
		setHostName	    ( "" );
		setHostPort	    ( 0 );
		setPath		    ( "" );
		setData		    ( "" );
		setSslRandomFile( "" );
		setPostMethod	( false );
		setCurlLogCbk	( NULL );
	}

private:
	bool			_sslProtocol;
	std::string		_hostname;
	unsigned int	_hostPort;
	std::string		_path;
	std::string		_data;
	std::string		_sslRandomFile;
	bool			_postMethod;
	void*			_curlLogCbk;
};

//=============================================================================

/**
 * Sends a HTTP request on a HTTP server then gets and reads the answer from the server.
 *
 * A proxy can be set and an error code is returned.
 *
 * Example **OBSOLETE**:
 * <pre>
 * class MyHttpRequest : public HttpRequest {
 * public:
 *    //Callback called when the server answer to the HTTP request.
 *    virtual void answerReceived(const std::string & answer, Error error) {
 *        if (error == NoError && !answer.empty()) {
 *            std::cout << "HTTP server anwser: " << answer << std::endl;
 *        }
 *    }
 * };
 *
 * HttpRequest::setProxy("proxy.wengo.fr", 8080, "myProxyUsername", "myProxyPassword");
 * MyHttpRequest * http = new MyHttpRequest();
 * http->sendRequest("https://wengo.fr:8080/login.php", "login=mylogin&password=mypassword");
 * </pre>
 *
 * @author Tanguy Krotoff
 */
class IHttpRequest : public Thread 
{
public:

	enum Error 
	{
		NoError,					//No error.
		HostNotFound,				//The hostname lookup failed.
		ConnectionRefused,			//The server refused the connection.
		UnexpectedClose,			//The server closed the connection unexpectedly.
		InvalidResponseHeader,		//The server sent an invalid response header.
		WrongContentLength,			//The client could not read the content correctly because
									//	an error with respect to the content length occurred.
		Aborted,					//The request was aborted.
		ProxyAuthenticationError,	//The proxy authentication failed.
		ProxyConnectionError,		//The connection with the proxy failed.
		TimeOut,					//operation timeouted.
		UnknownError				//Unknown error.
	};

	/**
	 * The HTTP answer to the request has been received.
	 *
	 * @param sender this class
	 * @param requestId HTTP request ID
	 * @param answer HTTP answer (std::string is used as a byte array)
	 * @param error Error code
	 */
	Event<void (IHttpRequest * sender, int requestId, const std::string & answer, Error error)> answerReceivedEvent;

	/**
	 * Indicates the current progress of the download.
	 *
	 * @see QHttp::dataReadProgress()
	 * @param requestId HTTP request ID
	 * @param bytesDone specifies how many bytes have been transfered
	 * @param bytesTotal total size of the HTTP transfer, if 0 then the total number of bytes is not known
	 */
	Event<void (int requestId, double bytesDone, double bytesTotal)> dataReadProgressEvent;

	/**
	 * Indicates the current progress of the upload.
	 *
	 * @see QHttp::dataSendProgress()
	 * @param requestId HTTP request ID
	 * @param bytesDone specifies how many bytes have been transfered
	 * @param bytesTotal total size of the HTTP transfer, if 0 then the total number of bytes is not known
	 */
	Event<void (int requestId, double bytesDone, double bytesTotal)> dataSendProgressEvent;

	virtual ~IHttpRequest() { }

	/**
	 * Sends a HTTP request given a HTTP URL.
	 *
	 * A complete HTTP URL is something like:
	 * https://wengo.fr:8080/login.php?login=mylogin&password=mypassword
	 *
	 * @param sslProtocol true if HTTPS protocol (https://), false if HTTP protocol (http://)
	 * @param hostname HTTP server hostname (e.g wengo.fr, yahoo.fr)
	 * @param hostPort HTTP server port number (e.g 80, 8080)
	 * @param path path on the server (e.g login.php)
	 * @param data HTTP data (e.g login=mylogin&password=mypassword)
	 * @param postMethod HTTP POST method if true, HTTP GET method if false
	 * @return HTTP request ID
	 */
	//virtual int sendRequest(bool sslProtocol,
	//		const std::string & hostname,
	//		unsigned int hostPort,
	//		const std::string & path,
	//		const std::string & data,
	//		bool postMethod = false,
	//		const char* sslRandomFile = NULL ) = 0;		//VOXOX - JRT - 2009.06.04 - OpenSSL init improvement.
	virtual int sendRequest( const CurlHttpRequestParms& httpRequestParms ) = 0;

	/**
	 * Aborts the current request and deletes all scheduled requests.
	 *
	 * Throws answerReceivedEvent with error=Aborted
	 */
	virtual void abort() = 0;

	/**
	 * Thread run method.
	 *
	 * Internal method, this should not be used from outside.
	 */
	virtual void run() = 0;
};

#endif	//OWIHTTPREQUEST_H
