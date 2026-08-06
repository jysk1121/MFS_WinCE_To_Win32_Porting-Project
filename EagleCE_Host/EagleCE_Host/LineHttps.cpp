/** ***************************************************************
*	@file LineHttps.cpp
*	@brief libcurl 기반 HTTP(S) 요청을 수행하는 소스파일입니다.
*	@details TangoPay(App/Dll/NetCtrl/HTTP.cpp)를 참고하여 이식했습니다.
*	캐싱(HTTPCache)과 인증 프레임워크(HTTPAuthenticator)는 제외했습니다.
********************************************************************/
#include "stdafx.h"
#include "LineHttps.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"

//------------------------------------------------------------------
//	Define Return Value
//------------------------------------------------------------------
#define RET_OK		0 ///< RET_OK 반환값은 정상임을 의미함
#define RET_NG		1 ///< RET_NG 반환값은 NG임을 의미함

//------------------------------------------------------------------
//	Timeouts (TangoPay HTTP.cpp 기준)
//------------------------------------------------------------------
#define CONNECT_TIMEOUT_MS			(30000L)		///< 연결 수립까지 허용 시간
#define READ_NETWORK_SPEED_MIN_Bps	(45L * 1024L)	///< 이 속도 미만이면 저속 타이머 시작
#define READ_NETWORK_SPEED_TO_S		(60L)			///< 저속 상태를 견딜 수 있는 최대 시간(초)

//------------------------------------------------------------------
//	Content-Type used for binary responses
//------------------------------------------------------------------
static const TCHAR CONTENT_TYPE_OCTET_STREAM[] = _T("application/octet-stream");

/** ********************************************************************
* @brief 응답 본문을 누적하는 libcurl write callback 입니다.
************************************************************************/
static size_t LineHttps_WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	CStringA *pBuf = (CStringA*)userdata;
	size_t nBytes = size * nmemb;

	pBuf->Append(ptr, (int)nBytes);

	return nBytes;
}

/** ********************************************************************
* @brief 응답 본문을 그대로 파일에 기록하는 libcurl write callback 입니다.
************************************************************************/
static size_t LineHttps_FileWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	FILE *fp = (FILE*)userdata;

	return fwrite(ptr, size, nmemb, fp);
}

/** ********************************************************************
* @brief 경로 상의 상위 폴더가 없으면 순차적으로 생성합니다.
************************************************************************/
static BOOL LineHttps_CreateDirectoryRecursive(CString strDirPath)
{
	if (strDirPath.IsEmpty() || CUtil::IsExistFile(strDirPath))
		return TRUE;

	int nPos = 0;
	CString strPart;
	CString strAccum;

	while ((nPos = strDirPath.Find('\\', nPos)) != -1)
	{
		strAccum = strDirPath.Left(nPos);

		if (strAccum.GetLength() > 0 && strAccum[strAccum.GetLength() - 1] != ':')
		{
			CreateDirectory(strAccum, NULL);	// already-exists error is ignored intentionally
		}

		nPos++;
	}

	CreateDirectory(strDirPath, NULL);

	return CUtil::IsExistFile(strDirPath);
}

CLineHttps::CLineHttps(CString strHostname, BOOL bUseHttps)
{
	m_strHostname = strHostname;
	m_strScheme = bUseHttps ? _T("https") : _T("http");
	m_strUserAgent = _T("EagleCE-ATM/1.0");
}

CLineHttps::~CLineHttps()
{
}

/** *****************************************************************************************************
*	@brief 요청 Path를 현재 Scheme/Hostname과 합쳐 전체 URL을 만듭니다.
********************************************************************************************************/
CString CLineHttps::BuildUrl(LPCTSTR lpszPath)
{
	CString strUrl;

	strUrl.Format(_T("%s://%s"), m_strScheme, m_strHostname);

	if (lpszPath != NULL)
		strUrl += lpszPath;

	return strUrl;
}

/** *****************************************************************************************************
*	@brief request.Headers("\r\n" 구분)와 기본 헤더(User-Agent)를 curl_slist로 구성합니다.
********************************************************************************************************/
curl_slist* CLineHttps::BuildHeaderList(HttpRequest &request)
{
	curl_slist *pList = NULL;
	CStringArray arrHeaders;

	int nCount = CUtil::ParsingStringToStringArray(request.Headers, _T("\r\n"), arrHeaders);

	for (int i = 0; i < nCount; i++)
	{
		CString strHeader = arrHeaders.GetAt(i);

		if (strHeader.IsEmpty())
			continue;

		char szHeader[1024] = { 0, };
		WideCharToMultiByte(CP_ACP, 0, strHeader, -1, szHeader, sizeof(szHeader), NULL, NULL);

		pList = curl_slist_append(pList, szHeader);
	}

	char szUserAgent[256] = { 0, };
	CString strUserAgentHeader;
	strUserAgentHeader.Format(_T("User-Agent: %s"), m_strUserAgent);
	WideCharToMultiByte(CP_ACP, 0, strUserAgentHeader, -1, szUserAgent, sizeof(szUserAgent), NULL, NULL);

	pList = curl_slist_append(pList, szUserAgent);

	return pList;
}

/** *****************************************************************************************************
*	@brief 응답의 StatusCode/ContentType/ContentLength 를 채웁니다.
********************************************************************************************************/
void CLineHttps::SetResponseMeta(CURL *curl, LPHttpResponse pResponse)
{
	long nStatusCode = 0;
	char *pContentType = NULL;
	curl_off_t nContentLength = 0;

	if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &nStatusCode) == CURLE_OK)
	{
		pResponse->StatusCode.Format(_T("%d"), nStatusCode);
	}

	if (curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &pContentType) == CURLE_OK && pContentType != NULL)
	{
		pResponse->ContentType.Format(_T("%S"), pContentType);
	}

	if (curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &nContentLength) == CURLE_OK)
	{
		pResponse->ContentLength = nContentLength;
	}
}

/** *****************************************************************************************************
*	@brief HTTP GET 요청을 수행합니다.
********************************************************************************************************/
int CLineHttps::SendGetRequest(LPHttpResponse pResponse, HttpRequest &request)
{
	request.Method = HTTP_GET;
	return MakeHTTPRequest(pResponse, request);
}

/** *****************************************************************************************************
*	@brief HTTP POST 요청을 수행합니다.
********************************************************************************************************/
int CLineHttps::SendPostRequest(LPHttpResponse pResponse, HttpRequest &request)
{
	request.Method = HTTP_POST;
	return MakeHTTPRequest(pResponse, request);
}

/** *****************************************************************************************************
*	@brief HTTP HEAD 요청을 수행합니다.
********************************************************************************************************/
int CLineHttps::SendHeadRequest(LPHttpResponse pResponse, HttpRequest &request)
{
	request.Method = HTTP_HEAD;
	return MakeHTTPRequest(pResponse, request);
}

/** *****************************************************************************************************
*	@brief 임의의 HTTP Method로 요청을 수행합니다.
********************************************************************************************************/
int CLineHttps::SendRequest(HTTP_METHOD method, LPHttpResponse pResponse, HttpRequest &request)
{
	request.Method = method;
	return MakeHTTPRequest(pResponse, request);
}

/** *****************************************************************************************************
*	@brief 실제 libcurl easy handle을 구성하고 요청을 수행하는 내부 함수입니다.
*	@details CA 검증은 HOST_ROOTCA_CERTIFICATE_FILE(rootCA.pem)로 수행합니다. (Scheme이 http일 때는
*	TLS 관련 옵션은 curl에서 자연히 무시됩니다.)
********************************************************************************************************/
int CLineHttps::MakeHTTPRequest(LPHttpResponse pResponse, HttpRequest &request)
{
	m_strLastError.Empty();

	if (pResponse == NULL)
		return RET_NG;

	CURL *curl = curl_easy_init();
	if (curl == NULL)
	{
		m_strLastError = _T("curl_easy_init failed");
		LOG(Error, _T("CLineHttps::MakeHTTPRequest - curl_easy_init failed"));
		return RET_NG;
	}

	CString strUrl = BuildUrl(request.Url);
	char szUrl[1024] = { 0, };
	WideCharToMultiByte(CP_ACP, 0, strUrl, -1, szUrl, sizeof(szUrl), NULL, NULL);

	CString strCaFile;
	char szCaFile[MAX_PATH] = { 0, };
	strCaFile.Format(_T("%s%s"), CUtil::GetAppPath(), HOST_ROOTCA_CERTIFICATE_FILE);
	WideCharToMultiByte(CP_ACP, 0, strCaFile, -1, szCaFile, sizeof(szCaFile), NULL, NULL);

	CStringA strResponseBufA;
	curl_slist *pHeaders = BuildHeaderList(request);

	curl_easy_setopt(curl, CURLOPT_URL, szUrl);
	curl_easy_setopt(curl, CURLOPT_CAINFO, szCaFile);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, (long)CURL_SSLVERSION_TLSv1_2);

	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, CONNECT_TIMEOUT_MS);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, READ_NETWORK_SPEED_MIN_Bps);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, READ_NETWORK_SPEED_TO_S);

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pHeaders);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, LineHttps_WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponseBufA);

	if (strcmp(request.Method, HTTP_POST) == 0)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.Data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request.DataLen);
	}
	else if (strcmp(request.Method, HTTP_HEAD) == 0)
	{
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
	}
	else if (strcmp(request.Method, HTTP_GET) != 0)
	{
		// PUT / PATCH / DELETE / OPTIONS 등
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.Method);

		if (request.Data != NULL && request.DataLen > 0)
		{
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.Data);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request.DataLen);
		}
	}

	LOG(Info, _T("CLineHttps::MakeHTTPRequest Call (%S %s)"), request.Method, strUrl);

	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		m_strLastError.Format(_T("%S"), curl_easy_strerror(res));
		LOG(Error, _T("CLineHttps::MakeHTTPRequest curl_easy_perform failed (%s)"), m_strLastError);

		curl_slist_free_all(pHeaders);
		curl_easy_cleanup(curl);
		return RET_NG;
	}

	SetResponseMeta(curl, pResponse);

	if (pResponse->ContentType.CompareNoCase(CONTENT_TYPE_OCTET_STREAM) == 0)
	{
		pResponse->SetBinaryData((const unsigned char*)(LPCSTR)strResponseBufA, strResponseBufA.GetLength());
	}
	else
	{
		pResponse->ResponseData = CString(strResponseBufA);
	}

	curl_slist_free_all(pHeaders);
	curl_easy_cleanup(curl);

	LOG(Info, _T("CLineHttps::MakeHTTPRequest end (HTTP %s, %d bytes)"), pResponse->StatusCode, strResponseBufA.GetLength());

	return RET_OK;
}

/** *****************************************************************************************************
*	@brief 응답 본문을 그대로 파일로 저장합니다. (대용량 다운로드 용도)
********************************************************************************************************/
int CLineHttps::DownloadFile(HttpRequest &request, CString strSaveFilename)
{
	m_strLastError.Empty();

	CString strDirPath = strSaveFilename.Left(strSaveFilename.ReverseFind('\\'));

	if (!CUtil::IsExistFile(strDirPath) && !LineHttps_CreateDirectoryRecursive(strDirPath))
	{
		m_strLastError = _T("Cannot create destination directory");
		LOG(Error, _T("CLineHttps::DownloadFile - Cannot create directory (%s)"), strDirPath);
		return RET_NG;
	}

	FILE *fp = _tfopen(strSaveFilename, _T("wb"));
	if (fp == NULL)
	{
		m_strLastError = _T("Cannot open destination file");
		LOG(Error, _T("CLineHttps::DownloadFile - Cannot open file (%s)"), strSaveFilename);
		return RET_NG;
	}

	CURL *curl = curl_easy_init();
	if (curl == NULL)
	{
		m_strLastError = _T("curl_easy_init failed");
		fclose(fp);
		return RET_NG;
	}

	CString strUrl = BuildUrl(request.Url);
	char szUrl[1024] = { 0, };
	WideCharToMultiByte(CP_ACP, 0, strUrl, -1, szUrl, sizeof(szUrl), NULL, NULL);

	CString strCaFile;
	char szCaFile[MAX_PATH] = { 0, };
	strCaFile.Format(_T("%s%s"), CUtil::GetAppPath(), HOST_ROOTCA_CERTIFICATE_FILE);
	WideCharToMultiByte(CP_ACP, 0, strCaFile, -1, szCaFile, sizeof(szCaFile), NULL, NULL);

	curl_slist *pHeaders = BuildHeaderList(request);

	curl_easy_setopt(curl, CURLOPT_URL, szUrl);
	curl_easy_setopt(curl, CURLOPT_CAINFO, szCaFile);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, (long)CURL_SSLVERSION_TLSv1_2);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, CONNECT_TIMEOUT_MS);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, READ_NETWORK_SPEED_MIN_Bps);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, READ_NETWORK_SPEED_TO_S);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pHeaders);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, LineHttps_FileWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)fp);

	if (request.Data != NULL && request.DataLen > 0)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.Data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request.DataLen);
	}

	LOG(Info, _T("CLineHttps::DownloadFile Call (%s -> %s)"), strUrl, strSaveFilename);

	CURLcode res = curl_easy_perform(curl);

	long nStatusCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &nStatusCode);

	curl_slist_free_all(pHeaders);
	curl_easy_cleanup(curl);
	fclose(fp);

	if (res != CURLE_OK)
	{
		m_strLastError.Format(_T("%S"), curl_easy_strerror(res));
		LOG(Error, _T("CLineHttps::DownloadFile curl_easy_perform failed (%s)"), m_strLastError);
		return RET_NG;
	}

	if (nStatusCode < 200 || nStatusCode >= 400)
	{
		m_strLastError.Format(_T("HTTP status %d"), nStatusCode);
		LOG(Error, _T("CLineHttps::DownloadFile failed (%s)"), m_strLastError);
		return RET_NG;
	}

	LOG(Info, _T("CLineHttps::DownloadFile end (HTTP %d)"), nStatusCode);

	return RET_OK;
}
