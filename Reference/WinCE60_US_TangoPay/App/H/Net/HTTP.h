#pragma once

#include <openssl/bio.h>
#include <curl/curl.h>

#include ".\Common\CmnLib.h"
#include "HTTPCache.h"
#include "HTTPTypes.h"
#include "HTTPAuthenticator.h"
#include "HTTPBasicAuthenticator.h"

// HTTP Errors
#define HTTP_ERR_BASE 8000

typedef int HTTPERR;

const HTTPERR  HTTP_NO_ERR				= 0;
const HTTPERR  HTTP_TLS_TYPE_FAILURE	(HTTP_ERR_BASE + 1);
const HTTPERR  HTTP_OPENSSL_INIT_FAIL	(HTTP_ERR_BASE + 2);
const HTTPERR  HTTP_TLS_NO_CA_FILE		(HTTP_ERR_BASE + 3);
const HTTPERR  HTTP_TLS_WEB_INIT		(HTTP_ERR_BASE + 4);
const HTTPERR  HTTP_TCP_INIT_CONN_FAIL	(HTTP_ERR_BASE + 5);
const HTTPERR  HTTP_TLS_INIT_CONN_FAIL	(HTTP_ERR_BASE + 6);
const HTTPERR  HTTP_TLS_CIPHER_FAIL		(HTTP_ERR_BASE + 7);
const HTTPERR  HTTP_HOSTNAME_ERR		(HTTP_ERR_BASE + 8);
const HTTPERR  HTTP_NO_FILE				(HTTP_ERR_BASE + 9);
const HTTPERR  HTTP_TLS_UNKNOWN_ERR		(HTTP_ERR_BASE + 10);
const HTTPERR  HTTP_RSP_PARSE_ERR		(HTTP_ERR_BASE + 11);
const HTTPERR  HTTP_SEND_ERR			(HTTP_ERR_BASE + 12);
const HTTPERR  HTTP_NO_DATA_RCV			(HTTP_ERR_BASE + 13);
const HTTPERR  HTTP_DATA_TOO_LONG		(HTTP_ERR_BASE + 14);
const HTTPERR  HTTP_WRITE_ERR			(HTTP_ERR_BASE + 15);
const HTTPERR  HTTP_X509_SETUP_ERR		(HTTP_ERR_BASE + 16);
const HTTPERR  HTTP_X509_SETUP_ERR2		(HTTP_ERR_BASE + 17);
const HTTPERR  HTTP_CACHE_MISS			(HTTP_ERR_BASE + 18);
const HTTPERR  HTTP_TLS_CONN_TIMEOUT	(HTTP_ERR_BASE + 19);
const HTTPERR  HTTP_WRITE_TIMEOUT		(HTTP_ERR_BASE + 20);
const HTTPERR  HTTP_READ_TIMEOUT		(HTTP_ERR_BASE + 21);
const HTTPERR  HTTP_TLS_READ_ERR		(HTTP_ERR_BASE + 22);
const HTTPERR  HTTP_CURL_INIT_FAIL		(HTTP_ERR_BASE + 23);

// Error range for curl error codes
const HTTPERR  HTTP_CURL_CODE_START		(HTTP_ERR_BASE + 100);
const HTTPERR  HTTP_CURL_CODE_END		(HTTP_ERR_BASE + 150);


class AFX_EXT_CLASS HTTP
{
public:
	HTTP(CString hostname, CHTTPAuthenticator* authenticator=NULL);
	~HTTP(void);

	/**
	 * Sends a POST request to the host and resource.
	 * @param[out] response the HTTP response returned by the destination server
	 * @param[in] request the request information
	 * @returns HTTP_NO_ERR for no error
	 */
	HTTPERR SendPostRequest(LPHttpResponse response, HttpRequest &request);

	/**
	 * Sends a GET request to the host and resource.
	 * @param[out] response Information about the HTTP response
	 * @param[in] request the request information
	 * @returns HTTP_NO_ERR for no error
	 */
	HTTPERR SendGetRequest(LPHttpResponse response, HttpRequest &request);

	/**
	 * Sends a HEAD request to the host and resource.
	 * @param[out] response Information about the HTTP response
	 * @param[in] request the request information
	 * @returns HTTP_NO_ERR for no error
	 */
	HTTPERR SendHeadRequest(LPHttpResponse response, HttpRequest &request);

	/**
	 * Sends a HEAD request to the host and resource.
	 * @param[in] the HTTP method for the request
	 * @param[out] response Information about the HTTP response
	 * @param[in] request the request information
	 * @returns HTTP_NO_ERR for no error
	 */
	HTTPERR SendRequest(HTTP_METHOD method, LPHttpResponse response, HttpRequest &request);

	/**
	 * Downloads the file from the URL.
	 * @param[in] request the request information
	 * @param[in] filename the path of the file to which to save the downloaded data
	 */
	HTTPERR DownloadFile(HttpRequest &request, CString filename);

	/**
	 * Gets the current hostname from the HTTP service.
	 */
	char *GetHostname();

private:
	CHTTPCache	*cache;
	CURL		*curl;

	// Default connection parameters
	char *hostname;
	char *server;
	char *port;

	// The value of the user agent header
	char *userAgentString;

	friend class CHTTPBasicAuthenticator;
	CHTTPAuthenticator *authenticator;

	/* Parsing */

	static BOOL		GetHostnameComponents(CString hostname, char **host, char **port, char **server);
	static BOOL		ParseResponseMessage(LPHttpResponse response, char* responseData);

	/**
	 * Sends an HTTP request with the specified parameters
	 * @returns HTTP_NO_ERR for no error
	 */
	HTTPERR MakeHTTPRequest(LPHttpResponse response, HttpRequest &request);
	HTTPERR MakeHTTPRequest(BIO *out, LPHttpResponse response, HttpRequest &request);

	/* Caching */

	/**
	 * Gets the response from the cache, if possible
	 * @param[in] cacheKey The key for the cache lookup
	 * @param[in,out] responseData A pointer to the BIO which will contain the response data.
	 * @param[out] size the size of the response data
	 * @returns HTTP_NO_ERR when the response has been retrieved from the cache
	 */
	HTTPERR	GetResponseFromCache(CString cacheKey, BIO *responseData, int &size);

	/* cURL-specific functions */

	curl_slist* AppendHeadersToSList(HttpRequest request);
	HTTPERR		SetResponseData(CURL *curl, LPHttpResponse response);
};
