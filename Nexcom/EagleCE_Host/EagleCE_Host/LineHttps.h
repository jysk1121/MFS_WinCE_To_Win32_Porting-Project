/** ***************************************************************
*	@file LineHttps.h
*	@brief libcurl 기반 HTTP(S) 요청을 수행하는 클래스입니다.
*	@details TangoPay(App/Dll/NetCtrl/HTTP.cpp)를 참고하여 이식했습니다.
*	캐싱(HTTPCache)과 인증 프레임워크(HTTPAuthenticator)는 제외했습니다.
********************************************************************/

#ifndef __LINE_HTTPS_H__
#define __LINE_HTTPS_H__

#pragma comment (lib, "libcurl.lib")

#include ".\curl/curl.h"

//------------------------------------------------------------------
//	HTTP Verbs
//------------------------------------------------------------------
typedef const char* HTTP_METHOD;
const HTTP_METHOD HTTP_GET		= "GET";
const HTTP_METHOD HTTP_POST	= "POST";
const HTTP_METHOD HTTP_HEAD	= "HEAD";
const HTTP_METHOD HTTP_PUT		= "PUT";
const HTTP_METHOD HTTP_PATCH	= "PATCH";
const HTTP_METHOD HTTP_DELETE	= "DELETE";
const HTTP_METHOD HTTP_OPTIONS	= "OPTIONS";

//------------------------------------------------------------------
//	HTTP 응답 정보
//------------------------------------------------------------------
typedef struct _http_response
{
	CString				StatusCode;			///< 응답 상태 코드 (예: "200")
	CString				ContentType;
	unsigned __int64	ContentLength;
	CString				ResponseData;		///< 텍스트 응답 본문 (ContentType이 application/octet-stream이 아닌 경우)
	unsigned char		*BinaryData;		///< 바이너리 응답 본문 (ContentType이 application/octet-stream인 경우)
	int					BinaryDataSize;

	_http_response() : StatusCode(), ContentType(), ResponseData()
	{
		ContentLength = 0;
		BinaryData = NULL;
		BinaryDataSize = 0;
	}

	~_http_response()
	{
		Release();
	}

	/// 상태 코드가 2xx/3xx (성공)인지 여부
	bool StatusCodeIsSuccessful() const
	{
		if (StatusCode.IsEmpty())
			return false;

		int nCode = _ttoi(StatusCode);

		return (nCode >= 200 && nCode < 400);
	}

	void SetBinaryData(const unsigned char *pData, int nSize)
	{
		Release();

		if (nSize <= 0)
			return;

		BinaryData = new unsigned char[nSize];
		memcpy(BinaryData, pData, nSize);
		BinaryDataSize = nSize;
	}

private:
	void Release()
	{
		if (BinaryData != NULL)
		{
			delete [] BinaryData;
			BinaryData = NULL;
			BinaryDataSize = 0;
		}
	}
} HttpResponse, *LPHttpResponse;

//------------------------------------------------------------------
//	HTTP 요청 정보
//------------------------------------------------------------------
typedef struct _http_request
{
	CString					Url;		///< Path (+Query), 예: "/api/v1/ping"
	CString					Headers;	///< "\r\n" 로 구분된 추가 요청 헤더, 예: "X-Api-Key: abc\r\nX-Trace-Id: 123"
	HTTP_METHOD				Method;
	const unsigned char		*Data;		///< 요청 본문 (POST/PUT/PATCH 용, GET/HEAD 이면 사용 안 함)
	int						DataLen;

	_http_request() : Url(), Headers()
	{
		Method = HTTP_GET;
		Data = NULL;
		DataLen = 0;
	}
} HttpRequest, *LPHttpRequest;

/** ***************************************************
*	@class CLineHttps
*	@brief HTTP(S) 요청을 수행하는 1회성 요청/응답 클래스입니다.
*	@details CLineCtrl(상시 접속 라인 관리)과 달리, 호출 시점에 커넥션을
*	새로 맺고 응답을 받으면 바로 종료하는 요청 단위 통신에 사용합니다.
*	기본은 HTTPS이며, bUseHttps=FALSE 로 평문 HTTP 접속도 가능합니다.
*******************************************************/
class CLineHttps
{
public:
	/// @param strHostname "host" 또는 "host:port" 형태 (스킴은 붙이지 않음)
	/// @param bUseHttps TRUE(기본) - https://, FALSE - http://
	CLineHttps(CString strHostname, BOOL bUseHttps = TRUE);
	~CLineHttps();

	int SendGetRequest(LPHttpResponse pResponse, HttpRequest &request);
	int SendPostRequest(LPHttpResponse pResponse, HttpRequest &request);
	int SendHeadRequest(LPHttpResponse pResponse, HttpRequest &request);
	int SendRequest(HTTP_METHOD method, LPHttpResponse pResponse, HttpRequest &request);

	/// 응답 본문을 그대로 파일로 저장합니다.
	int DownloadFile(HttpRequest &request, CString strSaveFilename);

	CString GetLastError() const { return m_strLastError; }

private:
	int MakeHTTPRequest(LPHttpResponse pResponse, HttpRequest &request);
	curl_slist* BuildHeaderList(HttpRequest &request);
	void SetResponseMeta(CURL *curl, LPHttpResponse pResponse);
	CString BuildUrl(LPCTSTR lpszPath);

private:
	CString		m_strHostname;
	CString		m_strScheme;		///< "https" 또는 "http"
	CString		m_strUserAgent;
	CString		m_strLastError;
};

#endif //__LINE_HTTPS_H__
