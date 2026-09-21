#pragma once

// HTTP Verbs
typedef const char*	HTTP_METHOD;
const HTTP_METHOD HTTP_GET		= "GET";
const HTTP_METHOD HTTP_POST		= "POST";
const HTTP_METHOD HTTP_HEAD		= "HEAD";
const HTTP_METHOD HTTP_PUT		= "PUT";
const HTTP_METHOD HTTP_PATCH	= "PATCH";
const HTTP_METHOD HTTP_DELETE	= "DELETE";
const HTTP_METHOD HTTP_OPTIONS	= "OPTIONS";


// A type containing data from an HTTP Response
typedef struct _http_response  {
	CString	StatusCode;
	CString	StatusMessage;
	CString	ResponseData;
	unsigned char * BinaryData;
	CString TransferEncoding;
	CString ContentType;
	unsigned __int64 ContentLength;
	CString ETag;

	int _binDataSize;

	_http_response::_http_response() : StatusCode(), StatusMessage(), ResponseData(), TransferEncoding(), ContentType(), ETag()
	{
		BinaryData = NULL;
		_binDataSize = 0;
		ContentLength = 0;
	}

	_http_response::~_http_response() 
	{
		Release();
	}

	/**
	 * Whether or not the status code returned by the webserver indicates success
	 */
	bool StatusCodeIsSuccessful() 
	{
		if (this->StatusCode.IsEmpty())
		{
			return false;
		}

		int len = this->StatusCode.GetLength();
		char *buffer = new char[len]();
		WideToMulti(buffer, this->StatusCode, len);

		int code = atoi(buffer);
		delete [] buffer;

		if (code >= 200 && code < 400)
		{
			return true;
		}

		return false;
	};

	void SetBinaryData(unsigned char * data, unsigned int size)
	{
		BinaryData = new unsigned char[size]();
		memcpy(BinaryData, data, size);
		_binDataSize = size;
	}

private:
	void Release()
	{
		if (BinaryData != NULL)
		{
			memset(BinaryData, 0, _binDataSize);
			delete [] BinaryData;
			BinaryData = NULL;
		}
	}
} HttpResponse, *LPHttpResponse;

// A type containing information about an HTTP request
typedef struct _http_request {
	CString		Url;
	CString		Headers;
	HTTP_METHOD Method;
	unsigned char* Data;

	bool UseCache;

	_http_request() : Url(), Headers()
	{
		Method = NULL;
		Data = NULL;
	};

	// Determines if the request is cacheable
	bool IsCacheable() const
	{
		return UseCache && strcmp(Method, HTTP_GET) == 0;
	};
} HttpRequest, *LPHttpRequest;