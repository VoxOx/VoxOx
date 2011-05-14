#include "wlm_password.h"

#include <owlibc.h>
#include <owsl.h>

#include <stdio.h>
#include <string.h>

/* authentication fails if password is not received in 5 seconds */
#define WLM_SIP_PASSWORD_TIMEOUT_MS 5000

typedef struct wlm_sip_password_asynchronous
{
	int is_connected ;
	char payload [1000] ;
	int payload_is_received ;
	int has_error ;
} wlm_sip_password_asynchronous ;

static int
wlm_sip_password_request_get
(
	const char * user_id,
	const char * domain_name,
	const char * password,
	char * request,
	size_t request_size
)
{
	const char * wlm_sip_password_request_header =
		"POST /RST.srf HTTP/1.1\r\n"
		"Accept: text/*\r\n"
		"Host: login.live.com\r\n"
		"Content-Length: %d\r\n"
		"Connection: Keep-Alive\r\n"
		"Cache-Control: no-cache\r\n"
		"Cookie: "
			"MSPPre=%s@%s; MSPCID=b90d02529edcee32; "
			"MUID=666E0769B90749C7AA6B1C17E4EDE754; "
			"ANON=A=BEC7A68380902D1735961C13FFFFFFFF&E=4c7&W=1; "
			"NAP=V=1.4&E=46d&C=TF3kuSL40geAQQJUK1pCxvnF5Uzj_B7kJh7ew-sXanaVb-3EEdVGMg&W=1\r\n\r\n" ;

	const char * wlm_sip_password_request_body =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<Envelope xmlns=\"http://schemas.xmlsoap.org/soap/envelope/\" "
			"xmlns:wsse=\"http://schemas.xmlsoap.org/ws/2003/06/secext\" "
			"xmlns:saml=\"urn:oasis:names:tc:SAML:1.0:assertion\" "
			"xmlns:wsp=\"http://schemas.xmlsoap.org/ws/2002/12/policy\" "
			"xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\" "
			"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/03/addressing\" "
			"xmlns:wssc=\"http://schemas.xmlsoap.org/ws/2004/04/sc\" "
			"xmlns:wst=\"http://schemas.xmlsoap.org/ws/2004/04/trust\">"
			"<Header>"
				"<ps:AuthInfo xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" Id=\"PPAuthInfo\">"
					"<ps:HostingApp>{7108E71A-9926-4FCB-BCC9-9A9D3F32E423}</ps:HostingApp>"
					"<ps:BinaryVersion>4</ps:BinaryVersion>"
					"<ps:UIVersion>1</ps:UIVersion>"
					"<ps:Cookies></ps:Cookies>"
					"<ps:RequestParams>AQAAAAIAAABsYwQAAAAxMDMz</ps:RequestParams>"
				"</ps:AuthInfo>"
				"<wsse:Security>"
					"<wsse:UsernameToken Id=\"user\">"
						"<wsse:Username>%s@%s</wsse:Username>"
						"<wsse:Password>%s</wsse:Password>"
					"</wsse:UsernameToken>"
				"</wsse:Security>"
			"</Header>"
			"<Body>"
				"<ps:RequestMultipleSecurityTokens xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" Id=\"RSTS\">"
					"<wst:RequestSecurityToken Id=\"RST0\">"
						"<wst:RequestType>"
							"http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue"
						"</wst:RequestType>"
						"<wsp:AppliesTo>"
							"<wsa:EndpointReference>"
								"<wsa:Address>"
									"http://Passport.NET/tb"
								"</wsa:Address>"
							"</wsa:EndpointReference>"
						"</wsp:AppliesTo>"
					"</wst:RequestSecurityToken>"
					"<wst:RequestSecurityToken Id=\"RST1\">"
						"<wst:RequestType>"
							"http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue"
						"</wst:RequestType>"
						"<wsp:AppliesTo>"
							"<wsa:EndpointReference>"
								"<wsa:Address>"
									"messengerclear.live.com"
								"</wsa:Address>"
							"</wsa:EndpointReference>"
						"</wsp:AppliesTo>"
						"<wsse:PolicyReference URI=\"MBI_KEY_OLD\">"
						"</wsse:PolicyReference>"
					"</wst:RequestSecurityToken>"
					"<wst:RequestSecurityToken Id=\"RST2\">"
						"<wst:RequestType>"
							"http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue"
						"</wst:RequestType>"
							"<wsp:AppliesTo>"
								"<wsa:EndpointReference>"
									"<wsa:Address>"
										"messengersecure.live.com"
									"</wsa:Address>"
								"</wsa:EndpointReference>"
							"</wsp:AppliesTo>"
						"<wsse:PolicyReference URI=\"MBI_SSL\">"
						"</wsse:PolicyReference>"
					"</wst:RequestSecurityToken>"
				"</ps:RequestMultipleSecurityTokens>"
			"</Body>"
		"</Envelope>" ;

	int header_length, body_length, length ;

	if (user_id == NULL || password == NULL || request == NULL)
	{
		return -1 ;
	}

	/* compute body length */
	body_length = snprintf
	(
		request,
		request_size,
		wlm_sip_password_request_body,
		user_id,
		domain_name,
		password
	) ;
	if (body_length < 0 || (size_t) body_length >= request_size)
	{
		return -1 ;
	}

	/* write header */
	header_length = snprintf
	(
		request,
		request_size,
		wlm_sip_password_request_header,
		body_length,
		user_id,
		domain_name
	) ;
	if (header_length < 0 || (size_t) header_length >= request_size)
	{
		return -1 ;
	}
	request += header_length ;
	request_size -= header_length ;

	/* write body */
	length = snprintf
	(
		request,
		request_size,
		wlm_sip_password_request_body,
		user_id,
		domain_name,
		password
	) ;
	if (length < 0 || (size_t) length >= request_size)
	{
		return -1 ;
	}

	return length ;
}

static void
wlm_sip_password_payload_parse
(
	char * string,
	wlm_sip_password_asynchronous * asynchronous
)
{
	const char * payload_start = "<wsse:BinarySecurityToken Id=\"Compact2\">" ;
	const size_t payload_start_length = strlen (payload_start) ;
	static int payload_start_parsing_state = 0 ;
	static int payload_start_is_parsed = 0 ;
	static size_t payload_length = 0 ;

	while (! payload_start_is_parsed && * string != 0)
	{
		if (* string == payload_start [payload_start_parsing_state])
		{
			payload_start_parsing_state ++ ;
			if (payload_start_parsing_state == (int) payload_start_length)
			{
				payload_start_is_parsed = 1 ;
			}
		}
		else
		{
			payload_start_parsing_state = 0 ;
		}
		string ++ ;
	}

	while (payload_start_is_parsed && ! asynchronous->payload_is_received && * string != 0)
	{
		asynchronous->payload [payload_length ++] = * string ;
		if (* string == '&')
		{
			asynchronous->payload [payload_length] = 0 ;
			asynchronous->payload_is_received = 1 ;
		}
		string ++ ;
	}
}

static void
wlm_sip_password_on_receive
(
	OWSLSocket socket,
	OWSLEvent event,
	void * user_data
)
{
	char response [10000] ;
	int response_length ;
	wlm_sip_password_asynchronous * asynchronous = user_data ;

	if (asynchronous->has_error)
	{
		return ;
	}

	switch (event)
	{
		case OWSL_EVENT_READ :
		{
			response_length = owsl_recv
			(
				socket,
				response,
				sizeof (response) - 1,
				0
			) ;
			if (response_length < 0)
			{
				return ;
			}
			response [response_length] = 0 ;
			wlm_sip_password_payload_parse
			(
				response,
				asynchronous
			) ;
			if (asynchronous->payload_is_received)
			{
				owsl_callback_set (socket, NULL, NULL) ;
			}
			break ;
		}
		case OWSL_EVENT_WRITE :
		{
			asynchronous->is_connected = 1 ;
			break ;
		}
		case OWSL_EVENT_ERROR :
		{
			asynchronous->has_error = 1 ;
			break ;
		}
	}
}

static char *
wlm_sip_password_build
(
	const char * payload,
	char * sip_password,
	size_t sip_password_size
)
{
	const char * wlm_sip_password_prefix = "msmsgs:RPS_" ;
	const char * wlm_sip_password_suffix = "p=" ;
	char * result ;

	/* password = prefix + payload + suffix */
	result = strncpy (sip_password, wlm_sip_password_prefix, sip_password_size) ;
	if (result != NULL)
	{
		sip_password_size -= strlen (result) ;
		result = strncat (sip_password, payload, sip_password_size) ;
		if (result != NULL)
		{
			sip_password_size -= strlen (result) ;
			result = strncat (sip_password, wlm_sip_password_suffix, sip_password_size) ;
		}
	}

	return result ;
}

static int
wlm_sip_password_wait
(
	int * condition,
	int * error
)
{
	const time_t sleep_time_ms = 100 ;
	const int timeout = WLM_SIP_PASSWORD_TIMEOUT_MS / (int) sleep_time_ms ;
	int sleep_count ;

	if (condition == NULL)
	{
		return -1 ;
	}

	/* wait for the condition or a timeout */
	for (sleep_count = 0 ; sleep_count < timeout ; sleep_count ++)
	{
		if (* condition || (error != NULL && * error))
		{
			break ;
		}
		msleep (sleep_time_ms) ;
	}
	if (! * condition)
	{
		return -1 ;
	}

	return 0 ;
}

static OWSLSocket
wlm_sip_password_connection_new
(
	wlm_sip_password_asynchronous * asynchronous
)
{
	const char * wlm_request_address = "login.live.com:443" ;
	int return_code ;
	struct sockaddr * remote_address ;
	OWSLSocket socket ;

	/* create address structure of the server */
	remote_address = owsl_address_new (OWSL_AF_UNDEFINED, wlm_request_address) ;
	if (remote_address == NULL)
	{
		return -1 ;
	}

	/* create socket */
	socket = owsl_socket
	(
		remote_address->sa_family,
		OWSL_MODE_UNDEFINED,
		OWSL_CIPHERING_ENABLED
	) ;
	if (socket < 0)
	{
		owsl_address_free (remote_address) ;
		return -1 ;
	}

	/* set non blocking mode */
	return_code = owsl_blocking_mode_set (socket, OWSL_NON_BLOCKING) ;
	if (return_code)
	{
		owsl_close (socket) ;
		return -1 ;
	}

	/* initialize asynchronous data */
	asynchronous->is_connected = 0 ;
	asynchronous->payload [0] = 0 ;
	asynchronous->payload_is_received = 0 ;
	asynchronous->has_error = 0 ;

	/* set callback */
	return_code = owsl_callback_set (socket, wlm_sip_password_on_receive, asynchronous) ;
	if (return_code)
	{
		owsl_close (socket) ;
		return -1 ;
	}

	/* connect to the server */
	return_code = owsl_connect
	(
		socket,
		remote_address,
		OWSL_ADDRESS_SIZE
	) ;
	if (return_code && OW_GET_ERROR != EINPROGRESS)
	{
		owsl_close (socket) ;
		owsl_address_free (remote_address) ;
		return -1 ;
	}
	return_code = wlm_sip_password_wait
	(
		& asynchronous->is_connected,
		& asynchronous->has_error
	) ;
	if (return_code)
	{
		return -1 ;
	}

	/* free address */
	owsl_address_free (remote_address) ;

	return socket ;
}

static void
wlm_sip_password_connection_free
(
	OWSLSocket socket
)
{
	owsl_close (socket) ;
}

static int
wlm_sip_password_response_receive
(
	wlm_sip_password_asynchronous * asynchronous
)
{
	int return_code ;

	return_code = wlm_sip_password_wait
	(
		& asynchronous->payload_is_received,
		& asynchronous->has_error
	) ;
	if (return_code)
	{
		return -1 ;
	}

	return (int) strlen (asynchronous->payload) ;
}

char *
wlm_sip_password_get
(
	const char * user_id,
	const char * domain_name,
	const char * password,
	char * sip_password,
	size_t sip_password_size
)
{
	int return_code ;
	OWSLSocket connection ;
	char request [8000] ;
	size_t request_length ;
	wlm_sip_password_asynchronous asynchronous ;

	if (user_id == NULL || password == NULL || sip_password == NULL)
	{
		return NULL ;
	}

	return_code = wlm_sip_password_request_get
	(
		user_id,
		domain_name,
		password,
		request,
		sizeof (request)
	) ;
	if (return_code < 0)
	{
		return NULL ;
	}

	connection = wlm_sip_password_connection_new
	(
		& asynchronous
	) ;
	if (connection < 0)
	{
		return NULL ;
	}

	request_length = strlen (request) ;
	return_code = owsl_send
	(
		connection,
		request,
		(int) request_length,
		0
	) ;
	if (return_code != (int) request_length)
	{
		wlm_sip_password_connection_free (connection) ;
		return NULL ;
	}

	return_code = wlm_sip_password_response_receive
	(
		& asynchronous
	) ;
	if (return_code <= 0)
	{
		wlm_sip_password_connection_free (connection) ;
		return NULL ;
	}

	wlm_sip_password_connection_free (connection) ;

	sip_password = wlm_sip_password_build
	(
		asynchronous.payload,
		sip_password,
		sip_password_size
	) ;
	if (sip_password == NULL)
	{
		return NULL ;
	}

	return sip_password ;
}
