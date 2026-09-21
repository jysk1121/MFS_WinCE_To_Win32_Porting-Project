#pragma once
#include "HTTPTypes.h"

class HTTP;

/**
 * Abstract interface for HTTP authentication request transformers
 */
class CHTTPAuthenticator
{
public:
	CHTTPAuthenticator(void)
	{

	};

	virtual ~CHTTPAuthenticator(void)
	{

	};

	/**
	 * Transforms the HTTP request to include data supporting the authentication method.
	 * @param http [in] The http service
	 * @param request [in,out] The request to modify
	 */
	virtual void Authenticate(HTTP* http, HttpRequest& request) = 0;
};
