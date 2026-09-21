#include "stdafx.h"
#include ".\Net\HTTP.h"
#include ".\Net\HTTPBasicAuthenticator.h"

CHTTPBasicAuthenticator::CHTTPBasicAuthenticator(CString username, CString password)
{
	this->username = username;
	this->password = password;
}

CHTTPBasicAuthenticator::~CHTTPBasicAuthenticator(void)
{
}

void CHTTPBasicAuthenticator::Authenticate(HTTP* http, HttpRequest& request)
{
	char* username = new char[this->username.GetLength() + 1]();
	char* password = new char[this->password.GetLength() + 1]();

	WideToMulti(username, this->username, this->username.GetLength());
	WideToMulti(password, this->password, this->password.GetLength());

	curl_easy_setopt(http->curl, CURLOPT_USERNAME, username);
	curl_easy_setopt(http->curl, CURLOPT_PASSWORD, password);

	memset(username, 0, this->username.GetLength());
	memset(password, 0, this->password.GetLength());
	
	delete [] username;
	delete [] password;
}
