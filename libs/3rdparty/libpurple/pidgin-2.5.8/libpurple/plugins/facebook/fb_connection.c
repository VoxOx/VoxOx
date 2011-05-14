/*
 * libfacebook
 *
 * libfacebook is the property of its developers.  See the COPYRIGHT file
 * for more details.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fb_connection.h"

static void fb_attempt_connection(FacebookConnection *);

#ifdef HAVE_ZLIB
#include <zlib.h>

static gchar *fb_gunzip(const guchar *gzip_data, ssize_t *len_ptr)
{
	gsize gzip_data_len	= *len_ptr;
	z_stream zstr;
	int gzip_err = 0;
	gchar *data_buffer;
	gulong gzip_len = G_MAXUINT16;
	GString *output_string = NULL;

	data_buffer = g_new0(gchar, gzip_len);

	zstr.next_in = NULL;
	zstr.avail_in = 0;
	zstr.zalloc = Z_NULL;
	zstr.zfree = Z_NULL;
	zstr.opaque = 0;
	gzip_err = inflateInit2(&zstr, MAX_WBITS+32);
	if (gzip_err != Z_OK)
	{
		g_free(data_buffer);
		purple_debug_error("facebook", "no built-in gzip support in zlib\n");
		return NULL;
	}
	
	zstr.next_in = (Bytef *)gzip_data;
	zstr.avail_in = gzip_data_len;
	
	zstr.next_out = (Bytef *)data_buffer;
	zstr.avail_out = gzip_len;
	
	gzip_err = inflate(&zstr, Z_SYNC_FLUSH);

	if (gzip_err == Z_DATA_ERROR)
	{
		inflateEnd(&zstr);
		inflateInit2(&zstr, -MAX_WBITS);
		if (gzip_err != Z_OK)
		{
			g_free(data_buffer);
			purple_debug_error("facebook", "Cannot decode gzip header\n");
			return NULL;
		}
		zstr.next_in = (Bytef *)gzip_data;
		zstr.avail_in = gzip_data_len;
		zstr.next_out = (Bytef *)data_buffer;
		zstr.avail_out = gzip_len;
		gzip_err = inflate(&zstr, Z_SYNC_FLUSH);
	}
	output_string = g_string_new("");
	while (gzip_err == Z_OK)
	{
		//append data to buffer
		output_string = g_string_append_len(output_string, data_buffer, gzip_len - zstr.avail_out);
		//reset buffer pointer
		zstr.next_out = (Bytef *)data_buffer;
		zstr.avail_out = gzip_len;
		gzip_err = inflate(&zstr, Z_SYNC_FLUSH);
	}
	if (gzip_err == Z_STREAM_END)
	{
		output_string = g_string_append_len(output_string, data_buffer, gzip_len - zstr.avail_out);
	} else {
		purple_debug_error("facebook", "gzip inflate error\n");
	}
	inflateEnd(&zstr);

	g_free(data_buffer);	

	gchar *output_data = g_strdup(output_string->str);
	*len_ptr = output_string->len;

	g_string_free(output_string, TRUE);

	return output_data;
}
#endif

void fb_connection_destroy(FacebookConnection *fbconn)
{
	fbconn->fba->conns = g_slist_remove(fbconn->fba->conns, fbconn);

	if (fbconn->request != NULL)
		g_string_free(fbconn->request, TRUE);

	g_free(fbconn->rx_buf);

	if (fbconn->connect_data != NULL)
		purple_proxy_connect_cancel(fbconn->connect_data);

	if (fbconn->ssl_conn != NULL)
		purple_ssl_close(fbconn->ssl_conn);

	if (fbconn->fd >= 0) {
		close(fbconn->fd);
	}

	if (fbconn->input_watcher > 0)
		purple_input_remove(fbconn->input_watcher);

	g_free(fbconn->url);
	g_free(fbconn->hostname);
	g_free(fbconn);
}

static void fb_update_cookies(FacebookAccount *fba, const gchar *headers)
{
	const gchar *cookie_start;
	const gchar *cookie_end;
	gchar *cookie_name;
	gchar *cookie_value;
	int header_len;

	g_return_if_fail(headers != NULL);

	header_len = strlen(headers);

	/* look for the next "Set-Cookie: " */
	/* grab the data up until ';' */
	cookie_start = headers;
	while ((cookie_start = strstr(cookie_start, "\r\nSet-Cookie: ")) &&
			(cookie_start - headers) < header_len)
	{
		cookie_start += 14;
		cookie_end = strchr(cookie_start, '=');
		cookie_name = g_strndup(cookie_start, cookie_end-cookie_start);
		cookie_start = cookie_end + 1;
		cookie_end = strchr(cookie_start, ';');
		cookie_value= g_strndup(cookie_start, cookie_end-cookie_start);
		cookie_start = cookie_end;

		g_hash_table_replace(fba->cookie_table, cookie_name,
				cookie_value);
	}
}

static void fb_connection_process_data(FacebookConnection *fbconn)
{
	ssize_t len;
	gchar *tmp;

	len = fbconn->rx_len;
	tmp = g_strstr_len(fbconn->rx_buf, len, "\r\n\r\n");
	if (tmp == NULL) {
		/* This is a corner case that occurs when the connection is
		 * prematurely closed either on the client or the server.
		 * This can either be no data at all or a partial set of
		 * headers.  We pass along the data to be good, but don't
		 * do any fancy massaging.  In all likelihood the result will
		 * be tossed by the connection callback func anyways
		 */
		tmp = g_strndup(fbconn->rx_buf, len);
	} else {
		tmp += 4;
		len -= g_strstr_len(fbconn->rx_buf, len, "\r\n\r\n") -
				fbconn->rx_buf + 4;
		tmp = g_memdup(tmp, len + 1);
		tmp[len] = '\0';
		fbconn->rx_buf[fbconn->rx_len - len] = '\0';
		fb_update_cookies(fbconn->fba, fbconn->rx_buf);

#ifdef HAVE_ZLIB
		if (strstr(fbconn->rx_buf, "Content-Encoding: gzip"))
		{
			/* we've received compressed gzip data, decompress */
			gchar *gunzipped;
			gunzipped = fb_gunzip((const guchar *)tmp, &len);
			g_free(tmp);
			tmp = gunzipped;
		}
#endif
	}

	g_free(fbconn->rx_buf);
	fbconn->rx_buf = NULL;

	if (fbconn->callback != NULL) {
		purple_debug_info("facebook", "executing callback for %s\n", fbconn->url);
		fbconn->callback(fbconn->fba, tmp, len, fbconn->user_data);
	}

	g_free(tmp);
}

static void fb_fatal_connection_cb(FacebookConnection *fbconn)
{
	PurpleConnection *pc = fbconn->fba->pc;

	purple_debug_error("facebook", "fatal connection error\n");

	fb_connection_destroy(fbconn);

	/* We died.  Do not pass Go.  Do not collect $200 */
	/* In all seriousness, don't attempt to call the normal callback here.
	 * That may lead to the wrong error message being displayed */
	purple_connection_error_reason(pc,
				PURPLE_CONNECTION_ERROR_NETWORK_ERROR,
				_("Server closed the connection."));

}

static void fb_post_or_get_readdata_cb(gpointer data, gint source,
		PurpleInputCondition cond)
{
	FacebookConnection *fbconn;
	gchar buf[4096];
	ssize_t len;

	fbconn = data;

	if (fbconn->method & FB_METHOD_SSL) {
		len = purple_ssl_read(fbconn->ssl_conn,
				buf, sizeof(buf) - 1);
	} else {
		len = recv(fbconn->fd, buf, sizeof(buf) - 1, 0);
	}

	if (len < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			/* Try again later */
			return;
		}

		if (fbconn->method & FB_METHOD_SSL && fbconn->rx_len > 0) {
			/*
			 * This is a slightly hacky workaround for a bug in either
			 * GNU TLS or in the SSL implementation on Facebook's web
			 * servers.  The sequence of events is:
			 * 1. We attempt to read the first time and successfully read
			 *    the server's response.
			 * 2. We attempt to read a second time and libpurple's call
			 *    to gnutls_record_recv() returns the error
			 *    GNUTLS_E_UNEXPECTED_PACKET_LENGTH, or
			 *    "A TLS packet with unexpected length was received."
			 *
			 * Normally the server would have closed the connection
			 * cleanly and this second read() request would have returned
			 * 0.  Or maybe it's normal for SSL connections to be severed
			 * in this manner?  In any case, this differs from the behavior
			 * of the standard recv() system call.
			 */
			purple_debug_warning("facebook",
				"ssl error, but data received.  attempting to continue\n");
		} else {
			/* TODO: Is this a regular occurrence?  If so then maybe resend the request? */
			fb_fatal_connection_cb(fbconn);
			return;
		}
	}

	if (len > 0)
	{
		buf[len] = '\0';

		fbconn->rx_buf = g_realloc(fbconn->rx_buf,
				fbconn->rx_len + len + 1);
		memcpy(fbconn->rx_buf + fbconn->rx_len, buf, len + 1);
		fbconn->rx_len += len;

		/* Wait for more data before processing */
		return;
	}

	/* The server closed the connection, let's parse the data */
	fb_connection_process_data(fbconn);

	fb_connection_destroy(fbconn);
}

static void fb_post_or_get_ssl_readdata_cb (gpointer data,
		PurpleSslConnection *ssl, PurpleInputCondition cond)
{
	fb_post_or_get_readdata_cb(data, -1, cond);
}

static void fb_post_or_get_connect_cb(gpointer data, gint source,
		const gchar *error_message)
{
	FacebookConnection *fbconn;
	ssize_t len;

	fbconn = data;
	fbconn->connect_data = NULL;

	if (error_message)
	{
		purple_debug_error("facebook", "post_or_get_connect failure to %s\n", fbconn->url);
		purple_debug_error("facebook", "post_or_get_connect_cb %s\n",
				error_message);
		fb_fatal_connection_cb(fbconn);
		return;
	}

	fbconn->fd = source;

	/* TODO: Check the return value of write() */
	len = write(fbconn->fd, fbconn->request->str,
			fbconn->request->len);
	fbconn->input_watcher = purple_input_add(fbconn->fd,
			PURPLE_INPUT_READ,
			fb_post_or_get_readdata_cb, fbconn);
}

static void fb_post_or_get_ssl_connect_cb(gpointer data,
		PurpleSslConnection *ssl, PurpleInputCondition cond)
{
	FacebookConnection *fbconn;
	ssize_t len;

	fbconn = data;

	purple_debug_info("facebook", "post_or_get_ssl_connect_cb\n");

	/* TODO: Check the return value of write() */
	len = purple_ssl_write(fbconn->ssl_conn,
			fbconn->request->str, fbconn->request->len);
	purple_ssl_input_add(fbconn->ssl_conn,
			fb_post_or_get_ssl_readdata_cb, fbconn);
}

static void fb_host_lookup_cb(GSList *hosts, gpointer data,
		const char *error_message)
{
	GSList *host_lookup_list;
	struct sockaddr_in *addr;
	gchar *hostname;
	gchar *ip_address;
	FacebookAccount *fba;
	PurpleDnsQueryData *query;

	/* Extract variables */
	host_lookup_list = data;

	fba = host_lookup_list->data;
	host_lookup_list =
			g_slist_delete_link(host_lookup_list, host_lookup_list);
	hostname = host_lookup_list->data;
	host_lookup_list =
			g_slist_delete_link(host_lookup_list, host_lookup_list);
	query = host_lookup_list->data;
	host_lookup_list =
			g_slist_delete_link(host_lookup_list, host_lookup_list);

	/* The callback has executed, so we no longer need to keep track of
	 * the original query.  This always needs to run when the cb is 
	 * executed. */
	fba->dns_queries = g_slist_remove(fba->dns_queries, query);

	/* Any problems, capt'n? */
	if (error_message != NULL)
	{
		purple_debug_warning("facebook",
				"Error doing host lookup: %s\n", error_message);
		return;
	}

	if (hosts == NULL)
	{
		purple_debug_warning("facebook",
				"Could not resolve host name\n");
		return;
	}

	/* Discard the length... */
	hosts = g_slist_delete_link(hosts, hosts);
	/* Copy the address then free it... */
	addr = hosts->data;
	ip_address = g_strdup(inet_ntoa(addr->sin_addr));
	g_free(addr);
	hosts = g_slist_delete_link(hosts, hosts);

	/*
	 * DNS lookups can return a list of IP addresses, but we only cache
	 * the first one.  So free the rest.
	 */
	while (hosts != NULL)
	{
		/* Discard the length... */
		hosts = g_slist_delete_link(hosts, hosts);
		/* Free the address... */
		g_free(hosts->data);
		hosts = g_slist_delete_link(hosts, hosts);
	}

	g_hash_table_insert(fba->hostname_ip_cache, hostname, ip_address);
}

static void fb_cookie_foreach_cb(gchar *cookie_name,
		gchar *cookie_value, GString *str)
{
	/* TODO: Need to escape name and value? */
	g_string_append_printf(str, "%s=%s;", cookie_name, cookie_value);
}

/**
 * Serialize the fba->cookie_table hash table to a string.
 */
static gchar *fb_cookies_to_string(FacebookAccount *fba)
{
	GString *str;

	str = g_string_new(NULL);

	g_hash_table_foreach(fba->cookie_table,
			(GHFunc)fb_cookie_foreach_cb, str);

	return g_string_free(str, FALSE);
}

static void fb_ssl_connection_error(PurpleSslConnection *ssl,
		PurpleSslErrorType errortype, gpointer data)
{
	FacebookConnection *fbconn = data;
	PurpleConnection *pc = fbconn->fba->pc;

	fbconn->ssl_conn = NULL;
	fb_connection_destroy(fbconn);
	purple_connection_ssl_error(pc, errortype);
}

void fb_post_or_get(FacebookAccount *fba, FacebookMethod method,
		const gchar *host, const gchar *url, const gchar *postdata,
		FacebookProxyCallbackFunc callback_func, gpointer user_data,
		gboolean keepalive)
{
	GString *request;
	gchar *cookies;
	FacebookConnection *fbconn;
	gchar *real_url;
	gboolean is_proxy = FALSE;
	const gchar *user_agent;
	const gchar* const *languages;
	gchar *language_names;
	PurpleProxyInfo *proxy_info = NULL;
	gchar *proxy_auth;
	gchar *proxy_auth_base64;

	/* TODO: Fix keepalive and use it as much as possible */
	keepalive = FALSE;

	if (host == NULL)
		host = "www.facebook.com";

	if (fba && fba->account && !(method & FB_METHOD_SSL))
	{
		proxy_info = purple_proxy_get_setup(fba->account);
		if (purple_proxy_info_get_type(proxy_info) == PURPLE_PROXY_USE_GLOBAL)
			proxy_info = purple_global_proxy_get_info();
		if (purple_proxy_info_get_type(proxy_info) == PURPLE_PROXY_HTTP)
		{
			is_proxy = TRUE;
		}	
	}
	if (is_proxy == TRUE)
	{
		real_url = g_strdup_printf("http://%s%s", host, url);
	} else {
		real_url = g_strdup(url);
	}

	cookies = fb_cookies_to_string(fba);
	user_agent = purple_account_get_string(fba->account, "user-agent", "Opera/9.50 (Windows NT 5.1; U; en-GB)");
	
	if (method & FB_METHOD_POST && !postdata)
		postdata = "";

	/* Build the request */
	request = g_string_new(NULL);
	g_string_append_printf(request, "%s %s HTTP/1.0\r\n",
			(method & FB_METHOD_POST) ? "POST" : "GET",
			real_url);
	if (is_proxy == FALSE)
		g_string_append_printf(request, "Host: %s\r\n", host);
	g_string_append_printf(request, "Connection: %s\r\n",
			(keepalive ? "Keep-Alive" : "close"));
	g_string_append_printf(request, "User-Agent: %s\r\n", user_agent);
	if (method & FB_METHOD_POST) {
		g_string_append_printf(request,
				"Content-Type: application/x-www-form-urlencoded\r\n");
		g_string_append_printf(request,
				"Content-length: %zu\r\n", strlen(postdata));
	}
	g_string_append_printf(request, "Accept: */*\r\n");
	g_string_append_printf(request, "Cookie: isfbe=false;%s\r\n", cookies);
#ifdef HAVE_ZLIB
	g_string_append_printf(request, "Accept-Encoding: gzip\r\n");
#endif
	if (is_proxy == TRUE)
	{
		if (purple_proxy_info_get_username(proxy_info) &&
			purple_proxy_info_get_password(proxy_info))
		{
			proxy_auth = g_strdup_printf("%s:%s", purple_proxy_info_get_username(proxy_info), purple_proxy_info_get_password(proxy_info));
			proxy_auth_base64 = purple_base64_encode((guchar *)proxy_auth, strlen(proxy_auth));
			g_string_append_printf(request, "Proxy-Authorization: Basic %s\r\n", proxy_auth_base64);
			g_free(proxy_auth_base64);
			g_free(proxy_auth);
		}
	}

	/* Tell the server what language we accept, so that we get error messages in our language (rather than our IP's) */
	languages = g_get_language_names();
	language_names = g_strjoinv(", ", (gchar **)languages);
	purple_util_chrreplace(language_names, '_', '-');
	g_string_append_printf(request, "Accept-Language: %s\r\n", language_names);
	g_free(language_names);

	purple_debug_info("facebook", "getting url %s\n", url);

	g_string_append_printf(request, "\r\n");
	if (method & FB_METHOD_POST)
		g_string_append_printf(request, "%s", postdata);

	/* If it needs to go over a SSL connection, we probably shouldn't print
	 * it in the debug log.  Without this condition a user's password is
	 * printed in the debug log */
	if (method == FB_METHOD_POST)
		purple_debug_info("facebook", "sending request data:\n%s\n",
			postdata);

	g_free(cookies);

	/*
	 * Do a separate DNS lookup for the given host name and cache it
	 * for next time.
	 *
	 * TODO: It would be better if we did this before we call
	 *       purple_proxy_connect(), so we could re-use the result.
	 *       Or even better: Use persistent HTTP connections for servers
	 *       that we access continually.
	 *
	 * TODO: This cache of the hostname<-->IP address does not respect
	 *       the TTL returned by the DNS server.  We should expire things
	 *       from the cache after some amount of time.
	 */
	if (!is_proxy && !(method & FB_METHOD_SSL))
	{
		/* Don't do this for proxy connections, since proxies do the DNS lookup */
		gchar *host_ip;

		host_ip = g_hash_table_lookup(fba->hostname_ip_cache, host);
		if (host_ip != NULL) {
			host = host_ip;
		} else if (fba->account && !fba->account->disconnecting) {
			GSList *host_lookup_list = NULL;
			PurpleDnsQueryData *query;

			host_lookup_list = g_slist_prepend(
					host_lookup_list, g_strdup(host));
			host_lookup_list = g_slist_prepend(
					host_lookup_list, fba);

			query = purple_dnsquery_a(host, 80,
					fb_host_lookup_cb, host_lookup_list);
			fba->dns_queries = g_slist_prepend(fba->dns_queries, query);
			host_lookup_list = g_slist_append(host_lookup_list, query);
		}
	}

	fbconn = g_new0(FacebookConnection, 1);
	fbconn->fba = fba;
	fbconn->url = real_url;
	fbconn->method = method;
	fbconn->hostname = g_strdup(host);
	fbconn->request = request;
	fbconn->callback = callback_func;
	fbconn->user_data = user_data;
	fbconn->fd = -1;
	fbconn->connection_keepalive = keepalive;
	fbconn->request_time = time(NULL);
	fba->conns = g_slist_prepend(fba->conns, fbconn);

	fb_attempt_connection(fbconn);
}

static void fb_attempt_connection(FacebookConnection *fbconn)
{
	FacebookAccount *fba = fbconn->fba;

#if 0
	/* Connection to attempt retries.  This code doesn't work perfectly, but
	 * remains here for future reference if needed */
	if (time(NULL) - fbconn->request_time > 5) {
		/* We've continuously tried to remake this connection for a 
		 * bit now.  It isn't happening, sadly.  Time to die. */
		purple_debug_error("facebook", "could not connect after retries\n");
		fb_fatal_connection_cb(fbconn);
		return;
	}

	purple_debug_info("facebook", "making connection attempt\n");

	/* TODO: If we're retrying the connection, consider clearing the cached
	 * DNS value.  This will require some juggling with the hostname param */
	/* TODO/FIXME: This retries almost instantenously, which in some cases
	 * runs at blinding speed.  Slow it down. */
	/* TODO/FIXME: this doesn't retry properly on non-ssl connections */
#endif

	if (fbconn->method & FB_METHOD_SSL) {
		fbconn->ssl_conn = purple_ssl_connect(fba->account, fbconn->hostname,
				443, fb_post_or_get_ssl_connect_cb,
				fb_ssl_connection_error, fbconn);
	} else {
		fbconn->connect_data = purple_proxy_connect(NULL, fba->account,
				fbconn->hostname, 80, fb_post_or_get_connect_cb, fbconn);
	}

	return;
}

