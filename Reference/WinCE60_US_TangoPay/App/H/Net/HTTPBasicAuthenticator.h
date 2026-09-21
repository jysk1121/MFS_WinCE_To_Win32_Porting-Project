#pragma once
#include "httpauthenticator.h"

class AFX_EXT_CLASS CHTTPBasicAuthenticator : public CHTTPAuthenticator
{
public:
	CHTTPBasicAuthenticator(CString username, CString password);
	~CHTTPBasicAuthenticator(void);

	/**
	 * Adds the appropriate headers to the request to send HTTP Basic auth.
	 * @param http [in] The http service
	 * @param request [in,out] The request to modify
	 */
	void Authenticate(HTTP* http, HttpRequest& request);

private:
	CString username;
	CString password;
};
