#include "stdafx.h"

#include ".\Net\HTTP.h"

#include <WinSock2.h>
#include <openssl/opensslv.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <curl/curl.h>

#include ".\Common\NHDbgApi.h"

#ifdef UNDER_CE
#  define CA_ROOT_PEM_FILE "\\ca.pem"
#else
#  define CA_ROOT_PEM_FILE "..\\DAT\\Certs\\ca.pem"
#endif
#define PREFERRED_CIPHERS "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256:DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256"

#define STRING(s) #s
#define STR(s) STRING(s)
#define USER_AGENT_STRING_SIZE (128)
#define USER_AGENT_FORMAT "User-Agent: NHATM/%s+%s (%d; Client %s; %s)"
#define CLIENT_VERSION "0.3"
#define AP_VERSION_STRING STR(AP_DETAIL_VERSION)

// HTTP response message buffer
#define HTTP_RESPONSE_BUFF_SIZE		(512 * 1024)

// Timeouts
#define CONNECT_TIMEOUT_MS			(30000L)		// The time allowed for a connection to establish
#define CONNECT_RETRY_INTERVAL_MS	(250)			// The time to wait in between attempts to call BIO_do_connect
#define WRITE_TIMEOUT_MS			(15000)			// The maximum time a write will wait before failing
#define WRITE_RETRY_INTERVAL_MS		(25)			// The time to delay in between attempts to call BIO_write
#define READ_TIMEOUT_MS				(30000)			// The maximum time to wait before failing to read from the session
#define READ_RETRY_INTERVAL_MS		(25)			// The time to delay in between attempts to call BIO_read
#define READ_NETWORK_SPEED_MIN_Bps	(45L * 1024L) 	// The floor of the connection speed before beginning the LOW_SPEED timer
#define READ_NETWORK_SPEED_TO_S		(60L)			// The number of seconds the low speed limit should be endured before aborting


#define OPEN_ERROR_THROW(errormsg, val) DeInitConnection(); \
		NVDump('F', 'A', "11", L"", errormsg); \
		throw val

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	BIO* response = (BIO*) userdata;

	return BIO_write(response, ptr, nmemb);
}

size_t read_callback(char *buffer, size_t size, size_t nitems, void *userdata);
size_t read_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
	LPHttpRequest request = (LPHttpRequest) userdata;
	size_t requestLen = strlen((const char*) request->Data);
	memcpy_s((unsigned char*) buffer, size * nitems, request->Data, requestLen);

	return requestLen;
}

size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata);
size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
	LPHttpResponse response = (LPHttpResponse) userdata;

	CString header(buffer, nitems);

	if (header.Find(':') == -1)
	{
		// Not a header... Probably the HTTP resource line
		return nitems;
	}

	CString name, value;
	name = header.Left(header.Find(':')).Trim();
	value = header.Mid(header.Find(':') + 1).Trim();

	if (name.CompareNoCase(L"etag") == 0)
	{
		response->ETag = value;
	}

	return nitems;

}

//
// Private function declarations
//

/**
 * Logs the error message if one exists
 */
static void LogOpenSSLErrorMessage(const char *httpErr);
void LogOpenSSLErrorMessage(const char *httpErr)
{
	char buff[256] = {};
	unsigned long eCode = 0;
	while ((eCode = ERR_get_error()) != 0)
	{
		ERR_error_string(eCode, buff);
		NHERROR((_T("Fatal Error: %s %s\r\n"), CString(httpErr), CString(buff)));
	}
}

static void LogCURLErrorMessage(CURLcode);
void LogCURLErrorMessage(CURLcode err)
{
	NHERROR((L"CURL error (%d): %S\r\n", err, curl_easy_strerror(err)));
}

/**
 * HTTP provides an interface for interacting with HTTP services over TLS.
 */
HTTP::HTTP(CString hostname, CHTTPAuthenticator* authenticator)
{
	NHDBG((L"Initializing SSL Library\r\n"));

	curl = curl_easy_init();

	if (!GetHostnameComponents(hostname, &this->hostname, &this->port, &this->server))
	{
		NHERROR((L"Invalid hostname components provided\r\n"));
		throw HTTP_HOSTNAME_ERR;
	}

	this->cache = new CHTTPCache(CACHEMODE_DEFAULT, hostname);
	this->authenticator = authenticator;

#ifdef UNDER_CE
	// Set User Agent value: format: "NHATM/<APversion> (<os version>; Client <http client version>; <machine type>)"
	CNHConfig config;
	NH_OS_VERSION os = config.GetOSVersion();
	char machineType[64] = {};
	WideToMulti(machineType, config.GetMachineType(), min(sizeof(machineType), config.GetMachineType().GetLength()));

	userAgentString = new char[USER_AGENT_STRING_SIZE]();
	sprintf_s(userAgentString, USER_AGENT_STRING_SIZE, USER_AGENT_FORMAT, APCODE, AP_VERSION_STRING, os, CLIENT_VERSION, machineType);
#else
	userAgentString = new char[USER_AGENT_STRING_SIZE]();
	sprintf_s(userAgentString, USER_AGENT_STRING_SIZE, USER_AGENT_FORMAT, "T", "test", 1, "vdebug", "nhatm");
#endif
	NHDBG((L"Curl version: %S\n", curl_version()));
}

HTTP::~HTTP(void)
{
	if (curl != NULL)
	{
		curl_easy_cleanup(curl);
	}

	if (this->hostname != NULL)
	{
		memset(hostname, 0, strlen(hostname));
		delete [] hostname;
		hostname = NULL;
	}

	if (this->server != NULL)
	{
		memset(server, 0, strlen(server));
		delete [] server;
		server = NULL;
	}

	if (this->cache != NULL)
	{
		delete cache;
		cache = NULL;
	}

	if (this->userAgentString != NULL)
	{
		memset(userAgentString, 0, strlen(userAgentString));
		delete [] userAgentString;
		userAgentString = NULL;
	} 
}

HTTPERR HTTP::SendPostRequest(LPHttpResponse response, HttpRequest &request)
{
	NHDBG((L"SendPostRequest\n"));
	return this->SendRequest(HTTP_POST, response, request);
}

HTTPERR HTTP::SendGetRequest(LPHttpResponse response, HttpRequest &request)
{
	NHDBG((L"SendGetRequest\n"));
	return this->SendRequest(HTTP_GET, response, request);
}

HTTPERR HTTP::SendHeadRequest(LPHttpResponse response, HttpRequest &request)
{
	NHDBG((L"SendHeadRequest\n"));
	return this->SendRequest(HTTP_HEAD, response, request);
}

HTTPERR HTTP::SendRequest(HTTP_METHOD method, LPHttpResponse response, HttpRequest &request)
{
	NHDBG((L"SendHeadRequest\n"));
	request.Method = method;
	return this->MakeHTTPRequest(response, request);
}

HTTPERR HTTP::DownloadFile(HttpRequest &request, CString filename)
{
	NHDBG((L"DownloadFile\n"));
	CString dirPath = filename.Left(filename.ReverseFind('\\'));
	if (!IsExistFile(dirPath) && !CreateDirectoryWithIntermediate(dirPath))
	{
		NHDBG((L"DownloadFile - HTTP NO FILE\n"));
		LogOpenSSLErrorMessage("HTTP_NO_FILE");
		return HTTP_NO_FILE;
	}

	int filenameSize = filename.GetLength() + 1;
	char *filenameBuffer = new char[filenameSize]();
	WideToMulti(filenameBuffer, filename, filenameSize);

	BIO *file = BIO_new_file(filenameBuffer, "wb+");

	delete [] filenameBuffer;

	if (!file)
	{
		NHDBG((L"DownloadFile 2 - HTTP NO FILE\n"));
		LogOpenSSLErrorMessage("HTTP_NO_FILE");
		return HTTP_NO_FILE;
	}

	HttpResponse response;
	//request.Method = HTTP_GET;
	request.Method = HTTP_POST;		// [RWC6-612] SKKim 2024.02.07 Spec¡ío POST¡¤I ¨ùU¨öAC¨ª¨ú©¬CO.
	HTTPERR err = this->MakeHTTPRequest(file, &response, request);

	if (!response.StatusCodeIsSuccessful())
	{
		NHERROR((L"HTTP response(%s) was not status code 200\r\n", response.StatusCode));
		err = HTTP_NO_FILE;
	}

	BIO_flush(file);
	BIO_free_all(file);

	return err;
}

char *HTTP::GetHostname()
{
	return hostname;
}

HTTPERR HTTP::MakeHTTPRequest(LPHttpResponse response, HttpRequest &request)
{
	BIO *out = BIO_new(BIO_s_mem());
	char *buff = new char[HTTP_RESPONSE_BUFF_SIZE]();

	HTTPERR err = this->MakeHTTPRequest(out, response, request);
	if (err != HTTP_NO_ERR)
	{
		goto cleanup;
	}

	// Copy into the proper response buffer
	BUF_MEM *bptr;
	BIO_get_mem_ptr(out, &bptr);
	if (bptr->length >= HTTP_RESPONSE_BUFF_SIZE)
	{	
		err = HTTP_DATA_TOO_LONG;
		goto cleanup;
	}

	memcpy(buff, bptr->data, bptr->length);

	if (response->ContentType == L"application/octet-stream")
	{
		// This content type is binary, so we can't use CString
		response->SetBinaryData((unsigned char *)buff, bptr->length);
	}
	else if (!HTTP::ParseResponseMessage(response, buff))
	{
		NHERROR((L"HTTP::SendRequest error while parsing the response\r\n"));
		err = HTTP_RSP_PARSE_ERR;
	}

cleanup:

	if (buff != NULL) delete [] buff;

	BIO_free(out);

	return err;
}

HTTPERR HTTP::MakeHTTPRequest(BIO *out, LPHttpResponse response, HttpRequest &request)
{
	CURLcode res;
	CString cacheKey;
	HTTPERR err = HTTP_NO_ERR;
	CString url = L"https://";
	url.AppendFormat(L"%S", this->hostname);

	// Added minkook
	//if (strlen(this->port) > 0)
	if (atoi(this->port) > 0)		// [RWC6-612] SKKim 2024.03.08 fixed bug
		url.AppendFormat(L":%S", this->port);

	url.Append(request.Url);

	char murl[256] = {};
	WideToMulti(murl, url, min(sizeof(murl), url.GetLength()));

	NHDBG((L"Making request to %S\r\n", murl));

	if (curl == NULL)
	{
		return HTTP_CURL_INIT_FAIL;
	}

	// Do authenticator transforms
	if (this->authenticator != NULL)
	{
		this->authenticator->Authenticate(this, request);
	}

	curl_slist *headers = HTTP::AppendHeadersToSList(request);

	if (request.IsCacheable())
	{
		CString etag;
		if (cache->IsCached(request.Url, CString(request.Method), etag, cacheKey))
		{
			// Previous request was found!
			char etagBuff[256] = {};
			CString etagHeader;
			etagHeader.Format(L"If-None-Match: %s\r\n", etag);
			WideToMulti(etagBuff, etagHeader, min(sizeof(etagBuff) - 1, etagHeader.GetLength()));

			headers = curl_slist_append(headers, etagBuff);
		}
	}

	// Only set the upload data, if it is not NULL
	if (request.Data != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.Data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE , strlen((const char*) request.Data));
	}

	curl_easy_setopt(curl, CURLOPT_URL, murl);
	curl_easy_setopt(curl, CURLOPT_CAINFO, CA_ROOT_PEM_FILE);

#ifdef UNDER_CE
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
#else
	 //uncomment below configurations to skip ssl certificate validation (simulator test purpose)
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
#endif

	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, CONNECT_TIMEOUT_MS);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, READ_NETWORK_SPEED_MIN_Bps);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, READ_NETWORK_SPEED_TO_S);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.Method);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, response);
#ifdef _DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		err = HTTP_CURL_CODE_START + res;
		LogCURLErrorMessage(res);
		goto cleanup;
	}

	err = HTTP::SetResponseData(curl, response);

	// Check for an unmodified cached response
	if (response->StatusCode == L"304") 
	{
		// The cache is still valid!
		int size = 0;
		err = GetResponseFromCache(cacheKey, out, size);
		if (err != HTTP_NO_ERR)
		{
			// Error while pulling from cached data
			goto cleanup;
		}

		response->StatusCode = L"200"; // Set to static value for downstream protocol analysis.
		response->ContentLength = size;
	}
	else
	{
		// Check for more protocol data
		if (response->ContentLength == 0)
		{
			// No data to receive! We're done!
			goto cleanup;
		}

		// Store response in Cache
		if (request.UseCache && !response->ETag.IsEmpty())
		{
			BUF_MEM *bptr;
			BIO_get_mem_ptr(out, &bptr);
			unsigned char *cacheData = new unsigned char[bptr->length]();
			memcpy(cacheData, bptr->data, bptr->length);

			cache->CacheResponse(request.Url, CString(request.Method), response->ETag, (byte *)cacheData, response->ContentLength);

			memset(cacheData, 0, (size_t)response->ContentLength);
			delete [] cacheData;
			cacheData = NULL;
		}
	}

cleanup:

	// Free up the headers
	if (headers != NULL)
	{
		curl_slist_free_all(headers);
	}

	curl_easy_reset(curl);

	return err;
}

BOOL HTTP::GetHostnameComponents(CString hostname, char **host, char **port, char **server)
{
	CStringArray hostComponents;
	int n = SplitString(hostname, L":", hostComponents);

	*port = "443";
	CString domain = hostComponents[0];

	int hostnameLen = domain.GetLength();
	*host = new char[hostnameLen + 1]();
	WideToMulti(*host, domain, hostnameLen);

	if (n == 2)
	{
		int portLen = hostComponents[1].GetLength();
		*port = new char[portLen + 1]();
		WideToMulti(*port, hostComponents[1], portLen);

		int serverLen = hostname.GetLength();
		*server = new char[serverLen + 1]();
		WideToMulti(*server, hostname, serverLen);
	}
	else
	{
		int serverLen = hostname.GetLength() + strlen(*port) + 2;
		*server = new char[serverLen]();
		sprintf_s(*server, serverLen, "%s:%s", *host, *port);
	}

	return true;
}

BOOL HTTP::ParseResponseMessage(LPHttpResponse response, char* responseData) 
{
	// Get body substring
	CString body = (CString)responseData;

	response->ResponseData = (CString) body;

	return true;
}

/**
 * Gets the response from the cache, if possible
 * @param[in] cacheKey The key for the cache lookup
 * @param[in,out] responseData A pointer to the BIO which will contain the response data
 * @param[out] size the size of the response data
 * @returns HTTP_NO_ERR when the response has been retrieved from the cache
 */
HTTPERR	HTTP::GetResponseFromCache(CString cacheKey, BIO *responseData, int &size)
{
	HTTPERR result = HTTP_NO_ERR;
	byte *cacheBuffer = NULL;

	size = cache->GetCacheItemSize(cacheKey);
	if (size == 0)
	{
		// No cache item available
		result = HTTP_CACHE_MISS;
		goto cleanup;
	}

	cacheBuffer = new byte[size]();
	if (cache->ReadCacheItem(cacheKey, cacheBuffer, size) == 0)
	{
		// No bytes read from cache
		result = HTTP_CACHE_MISS;
		goto cleanup;
	}

	BIO_write(responseData, cacheBuffer, size);

cleanup:
	// Cleanup

	if (cacheBuffer) 
	{
		memset(cacheBuffer, 0, size);
		delete [] cacheBuffer;
		cacheBuffer = NULL;
	}

	return result;
}

curl_slist* HTTP::AppendHeadersToSList(HttpRequest request)
{
//#define MAX_HEADER_BUFFER_LEN 2 * 1024 + 1
#define MAX_HEADER_BUFFER_LEN 4 * 1024 + 1	// [RWC6-676] SKKim 2024.05.22

	CString header;
	CStringArray arr_headers;
	curl_slist* slist = NULL;
	char* buffer = NULL;

	// [RWC6-676] start SKKim 2024.05.22
	//int index = -1;
	//int last = 0;
	int nheadercnt = 0;
	int len = 0;

	arr_headers.RemoveAll();

	nheadercnt = SplitString(request.Headers, L"\r\n", arr_headers);

	for(int i=0; i<nheadercnt; i++)
	{
		header = arr_headers.GetAt(i);

		len = WideCharToMultiByte(CP_ACP, 0, header, -1, NULL, NULL, NULL, NULL);

		if (len > 0)
		{
			NHDBG((L"Making request - AppendHeadersToSList : %s\r\n", header));

			buffer = new char[len + 1]();
			memset(buffer, 0, len + 1);
			WideToMulti(buffer, header, len);

			slist = curl_slist_append(slist, buffer);

			memset(buffer, 0, len + 1);

			delete [] buffer;
			buffer = NULL;
		}
	}

	//while ((index = request.Headers.Find(L"\r\n", index+1)) != -1) 
	//{
	//	header = request.Headers.Mid(last, index - last).Trim();
	//	// Max header length will be evaluated to 1 less than MAX_HEADER_BUFFER_LEN to account for the
	//	// null byte
	//	if (header.GetLength() >= MAX_HEADER_BUFFER_LEN)
	//	{
	//		// Skip header if it is too long
	//		continue;
	//	}
	//	buffer = new char[header.GetLength() + 1]();
	//	WideToMulti(buffer, header, header.GetLength());
	//	slist = curl_slist_append(slist, buffer);
	//	last = index;
	//	//memset(buffer, 0, header.GetLength() + 1);
	//	memset(buffer, 0, len + 1);
	//	delete [] buffer;
	//	buffer = NULL;
	//}
	// [RWC6-676] end SKKim 2024.05.22


	// Default Headers
	header = (CString)this->userAgentString;
	NHDBG((L"Making request - default headers : %s\r\n", header));	// [RWC6-676] SKKim 2024.05.22
	slist = curl_slist_append(slist, this->userAgentString);

	return slist;
}

HTTPERR	HTTP::SetResponseData(CURL *curl, LPHttpResponse response)
{
	CURLcode res;
	long tmpLong = 0L;
	char *tmpChar = NULL;

	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &tmpLong);
	if (res == CURLE_OK)
	{
		response->StatusCode.Format(L"%d", tmpLong);
		NHDBG((L"Response code: %s\r\n", response->StatusCode));
	}
	else 
	{
		NHERROR((L"Could not find response code\r\n"));
		LogCURLErrorMessage(res);
	}

	
	res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &tmpChar);
	if (res == CURLE_OK)
	{
		response->ContentType.Format(L"%S", tmpChar);
		NHDBG((L"Content-Type: %S\r\n", tmpChar));
	}
	else 
	{
		NHERROR((L"Could not find content-type\r\n"));
		LogCURLErrorMessage(res);
	}


	res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &response->ContentLength);
	if (res != CURLE_OK)
	{
		NHERROR((L"Could not find content-length\r\n"));
		LogCURLErrorMessage(res);
	}

	return HTTP_NO_ERR;
}