/** ***************************************************************
*	@file LineCurl.cpp
*	@brief libcurl backed TCP/TLS line, client side only. See LineCurl.h.
********************************************************************/
#include "stdafx.h"
#include "LineCurl.h"

#include "Queue.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"

//------------------------------------------------------------------
//	Define extern function
//------------------------------------------------------------------
extern int		put_data(char *recv_data, int recv_size);
extern int		put_event(char recv_evt);

//------------------------------------------------------------------
//	Define Return Value
//------------------------------------------------------------------
#define RET_OK		0
#define RET_NG		1
#define RET_NG_SSL	22

// Matches the cipher policy the OpenSSL line applies (see LineTCP.cpp).
#define CURL_LINE_CIPHERS	"HIGH:!aNULL:!eNULL:!EXPORT:!RC4:!3DES:!MD5:!PSK"


CLineCurl::CLineCurl() : m_eStartCheckSocket(FALSE, FALSE)
{
	CLineCtrl::Register(LINE_CURL, this);

	// Refcounted inside libcurl, so pairing it with the cleanup in the
	// destructor is safe even though CLineHttps also uses curl.
	if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
		LOG(Error, _T("curl_global_init Error"));

	m_pCurl = NULL;
	m_Socket = INVALID_SOCKET;

	::InitializeCriticalSection(&m_csHandle);
	m_bHandleLockInit = TRUE;

	m_bConnectStatus = FALSE;
	m_bUseSSL = FALSE;
	m_nTCPConnectionTimeout = 30;

	m_bExitThread = FALSE;
	m_bCheckSocket = FALSE;
	m_bRequestConnect = FALSE;
}


CLineCurl::~CLineCurl()
{
	LOG(Info, _T("Line Curl Terminate"));

	CleanUp();

	m_bExitThread = TRUE;

	if (m_bHandleLockInit == TRUE)
	{
		::DeleteCriticalSection(&m_csHandle);
		m_bHandleLockInit = FALSE;
	}

	curl_global_cleanup();
}


/** ***************************************************
* @brief Drops the connection and releases the easy handle.
*******************************************************/
void CLineCurl::CleanUp()
{
	::EnterCriticalSection(&m_csHandle);

	if (m_pCurl != NULL)
	{
		curl_easy_cleanup(m_pCurl);
		m_pCurl = NULL;
	}

	m_Socket = INVALID_SOCKET;

	::LeaveCriticalSection(&m_csHandle);

	m_bCheckSocket = FALSE;
	m_bConnectStatus = FALSE;
	m_bRequestConnect = FALSE;
	m_bUseSSL = FALSE;
}


/** ***************************************************
* @brief Builds the URL curl connects to. The scheme only selects whether
*	TLS is negotiated - CURLOPT_CONNECT_ONLY means no request is ever sent.
*******************************************************/
CString CLineCurl::BuildUrl(LPCTSTR lpszHost, LPCTSTR lpszPort, BOOL bUseSSL)
{
	CString strUrl;

	int nPort = _ttoi(lpszPort);

	if (nPort > 0)
		strUrl.Format(_T("%s://%s:%d"), (bUseSSL == TRUE) ? _T("https") : _T("http"), lpszHost, nPort);
	else
		strUrl.Format(_T("%s://%s"), (bUseSSL == TRUE) ? _T("https") : _T("http"), lpszHost);

	return strUrl;
}


/** ***************************************************
* @brief select() on the socket curl is using. curl_easy_send/recv report
*	CURLE_AGAIN rather than blocking, so the wait happens here.
*******************************************************/
BOOL CLineCurl::WaitForSocket(BOOL bForRead, int nTimeoutMs)
{
	if (m_Socket == INVALID_SOCKET)
		return FALSE;

	struct fd_set	fdset;
	struct timeval	timeout;

	FD_ZERO(&fdset);
	FD_SET(m_Socket, &fdset);

	timeout.tv_sec  = nTimeoutMs / 1000;
	timeout.tv_usec = (nTimeoutMs % 1000) * 1000;

	int rc = 0;

	if (bForRead == TRUE)
		rc = select(0, &fdset, NULL, NULL, &timeout);
	else
		rc = select(0, NULL, &fdset, NULL, &timeout);

	return (rc > 0) ? TRUE : FALSE;
}


/** ********************************************************************
* @brief Connects (and, when asked, negotiates TLS) through libcurl.
* @param Dest_Info	host address
* @param dest_port	port
* @param sOption	SSL mode: "1" - TLS, "0" - plain
* @param mode		socket mode: "0"/"2" - client, "1" - listen (unsupported)
* @retval RET_OK 0, RET_NG 1, RET_NG_SSL 22
************************************************************************/
int	CLineCurl::LineOpen(LPCTSTR Dest_Info, LPCTSTR dest_port, LPCTSTR sOption, LPCTSTR mode)
{
	int nServerMode = _ttoi(mode);
	int nUseSSL     = _ttoi(sOption);

	if (nServerMode == 1)
	{
		// AMS listens for an inbound connection; libcurl has no server side.
		// That flow is kept on CLineTcp - see Set_FlowInform().
		LOG(Error, _T("LineOpen(curl) server mode is not supported - use LINE_TCP for AMS"));
		return RET_NG;
	}

	LOG(Info, _T("LineOpen(curl) Call (Host : %s, Port : %s, SSL Option : %s, Mode : %s)"),
		Dest_Info, dest_port, sOption, mode);

	if (m_pCurl != NULL)
	{
		LOG(Error, _T("LineOpen(curl) is already open"));
		CleanUp();
	}

	m_bUseSSL = (nUseSSL > 0) ? TRUE : FALSE;
	m_bConnectStatus = FALSE;
	m_bRequestConnect = FALSE;

	::EnterCriticalSection(&m_csHandle);

	m_pCurl = curl_easy_init();

	if (m_pCurl == NULL)
	{
		::LeaveCriticalSection(&m_csHandle);
		LOG(Error, _T("curl_easy_init is failed"));
		return RET_NG;
	}

	CString strUrl = BuildUrl(Dest_Info, dest_port, m_bUseSSL);
	char szUrl[1024] = { 0, };
	WideCharToMultiByte(CP_ACP, 0, strUrl, -1, szUrl, sizeof(szUrl), NULL, NULL);

	curl_easy_setopt(m_pCurl, CURLOPT_URL, szUrl);

	// Connect and negotiate only - no HTTP request is issued, the caller
	// owns the bytes on the wire from here on.
	curl_easy_setopt(m_pCurl, CURLOPT_CONNECT_ONLY, 1L);
	curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, (long)m_nTCPConnectionTimeout);
	curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(m_pCurl, CURLOPT_TCP_KEEPALIVE, 1L);

	if (m_bUseSSL == TRUE)
	{
		CString strCertFile;
		char szCertFile[MAX_PATH] = { 0, };
		strCertFile.Format(_T("%s%s"), CUtil::GetAppPath(), HOST_ROOTCA_CERTIFICATE_FILE);
		WideCharToMultiByte(CP_ACP, 0, strCertFile, -1, szCertFile, sizeof(szCertFile), NULL, NULL);

		// Trust is pinned to the CA file shipped with the terminal, exactly
		// as the OpenSSL line does - curl is not asked to fall back to any
		// system trust store.
		curl_easy_setopt(m_pCurl, CURLOPT_CAINFO, szCertFile);
		curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 1L);

		// The host is reached by IP, so the certificate's CN/SAN cannot be
		// matched against a name (the same reasoning the OpenSSL line
		// documents). Chain verification above is what is relied on.
		curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L);

		curl_easy_setopt(m_pCurl, CURLOPT_SSLVERSION, (long)CURL_SSLVERSION_TLSv1_2);
		curl_easy_setopt(m_pCurl, CURLOPT_SSL_CIPHER_LIST, CURL_LINE_CIPHERS);

		LOG(Info, _T("LineOpen(curl) CA file (%s)"), strCertFile);
	}

	CURLcode res = curl_easy_perform(m_pCurl);

	if (res != CURLE_OK)
	{
		LOG(Error, _T("curl connect is failed (%d) (%S)"), res, curl_easy_strerror(res));

		curl_easy_cleanup(m_pCurl);
		m_pCurl = NULL;

		::LeaveCriticalSection(&m_csHandle);

		// Tell the caller a TLS failure apart from a plain connect failure,
		// the way the OpenSSL line does.
		if ((m_bUseSSL == TRUE) &&
			((res == CURLE_SSL_CONNECT_ERROR) || (res == CURLE_PEER_FAILED_VERIFICATION) ||
			 (res == CURLE_SSL_CACERT_BADFILE) || (res == CURLE_SSL_CIPHER)))
		{
			return RET_NG_SSL;
		}

		return RET_NG;
	}

	curl_socket_t curlSocket = CURL_SOCKET_BAD;

	if ((curl_easy_getinfo(m_pCurl, CURLINFO_ACTIVESOCKET, &curlSocket) != CURLE_OK) ||
		(curlSocket == CURL_SOCKET_BAD))
	{
		LOG(Error, _T("curl active socket is invalid"));

		curl_easy_cleanup(m_pCurl);
		m_pCurl = NULL;

		::LeaveCriticalSection(&m_csHandle);
		return RET_NG;
	}

	m_Socket = (SOCKET)curlSocket;

	::LeaveCriticalSection(&m_csHandle);

	LOG(Info, (m_bUseSSL == TRUE) ? _T("TCP/IP + TLS Connect OK (curl)") : _T("TCP/IP Connect OK (curl)"));

	m_bConnectStatus = TRUE;
	m_bCheckSocket = TRUE;

	put_event(CONNECT_EVT);

	// Release the receive thread now that there is a socket to watch.
	m_eStartCheckSocket.Set();

	return RET_OK;
}


int	CLineCurl::LineClose()
{
	LOG(Info, _T("LineClose(curl) Call"));

	CleanUp();

	return RET_OK;
}


int CLineCurl::LineAccept(BYTE opt)
{
	// libcurl has no server side. AMS inbound stays on CLineTcp.
	LOG(Error, _T("LineAccept(curl) is not supported (%d)"), opt);

	return RET_NG;
}


int CLineCurl::LineCallState(int nMode)
{
	if (nMode == 0)
		return (int)m_bRequestConnect;

	return m_bConnectStatus;
}


/** ****************************************
* @brief Sends the framed message over the connection curl established.
* @retval RET_NG 1, RET_OK 0
********************************************/
int	CLineCurl::LineSendData(BYTE *send_data, int len)
{
	if ((send_data == NULL) || (len <= 0))
		return RET_NG;

	if (m_pCurl == NULL)
	{
		LOG(Error, _T("LineSendData(curl) is not connected"));
		return RET_NG;
	}

	int		nSent = 0;
	int		nRet = RET_OK;

	while (nSent < len)
	{
		size_t		nWritten = 0;
		CURLcode	res = CURLE_OK;

		::EnterCriticalSection(&m_csHandle);

		if (m_pCurl == NULL)
		{
			::LeaveCriticalSection(&m_csHandle);
			LOG(Error, _T("LineSendData(curl) connection was closed while sending"));
			return RET_NG;
		}

		res = curl_easy_send(m_pCurl, &send_data[nSent], (size_t)(len - nSent), &nWritten);

		::LeaveCriticalSection(&m_csHandle);

		if (res == CURLE_OK)
		{
			nSent += (int)nWritten;
			continue;
		}

		if (res == CURLE_AGAIN)
		{
			// Socket is not writable yet - wait rather than spin.
			if (WaitForSocket(FALSE, 1000) == FALSE)
			{
				LOG(Error, _T("LineSendData(curl) send timeout"));
				nRet = RET_NG;
				break;
			}

			continue;
		}

		LOG(Error, _T("curl_easy_send is failed (%d) (%S)"), res, curl_easy_strerror(res));
		nRet = RET_NG;
		break;
	}

	return nRet;
}


/** ***************************************************
* @brief Receive thread. Mirrors CLineTcp::ThreadHandlerProc(): wait on the
*	socket, drain it, and hand whatever arrives to the queue the flow
*	control layer reads from.
*******************************************************/
unsigned CLineCurl::ThreadHandlerProc()
{
	m_bExitThread = FALSE;

	while (m_bExitThread != TRUE)
	{
		if (m_eStartCheckSocket.Wait(1000) != WAIT_OBJECT_0)
			continue;

		while (m_bCheckSocket)
		{
			if (WaitForSocket(TRUE, 1000) == FALSE)
			{
				// Nothing to read this second - re-check the flags and wait
				// again. AP may have closed the line in the meantime.
				continue;
			}

			if (m_bCheckSocket == FALSE)
				break;

			size_t		nRead = 0;
			CURLcode	res = CURLE_OK;

			::EnterCriticalSection(&m_csHandle);

			if (m_pCurl == NULL)
			{
				::LeaveCriticalSection(&m_csHandle);
				break;
			}

			res = curl_easy_recv(m_pCurl, m_RecvBuf, sizeof(m_RecvBuf), &nRead);

			::LeaveCriticalSection(&m_csHandle);

			if (res == CURLE_AGAIN)
				continue;

			if (res != CURLE_OK)
			{
				LOG(Info, _T("curl_easy_recv return (%d) (%S)"), res, curl_easy_strerror(res));

				m_bConnectStatus = FALSE;
				put_event(DISCONNECT_EVT);
				break;
			}

			if (nRead == 0)
			{
				// Peer closed the connection gracefully.
				LOG(Info, _T("curl connection closed by remote"));

				m_bConnectStatus = FALSE;
				put_event(DISCONNECT_EVT);
				break;
			}

			put_data(m_RecvBuf, (int)nRead);
		}

		m_bCheckSocket = FALSE;
		m_bConnectStatus = FALSE;
		m_bRequestConnect = FALSE;
	}

	return 0;
}


/** *******************************************************************
* @brief TestPing - ICMP, independent of the transport, so it simply
*	defers to the OpenSSL line's implementation.
*********************************************************************/
int CLineCurl::TestPing(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply)
{
	CLineCtrl* pTcpLine = CLineCtrl::Instance(LINE_TCP);

	if (pTcpLine == NULL)
	{
		LOG(Error, _T("TestPing(curl) LINE_TCP instance is null"));
		return RET_NG;
	}

	return pTcpLine->TestPing(sHostInfo, icmpEchoReply);
}
