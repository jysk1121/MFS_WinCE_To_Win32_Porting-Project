/** ***************************************************************
*	@file LineCurl.h
*	@brief libcurl backed TCP/TLS line, client side only.
*
*	Same job as CLineTcp, but the socket and the TLS session are owned by
*	libcurl instead of being driven through OpenSSL by hand. The host
*	protocol is not HTTP, so curl is used in CURLOPT_CONNECT_ONLY mode:
*	curl performs the connect and the TLS handshake, then hands back a
*	plain send/recv pair (curl_easy_send / curl_easy_recv) that the
*	existing STX/LEN/ETX/BCC framing rides on unchanged.
*
*	Deliberately NOT a full replacement for CLineTcp: curl cannot listen or
*	accept, so the AMS inbound path stays on CLineTcp/OpenSSL. Only the
*	outbound host transaction flows can be pointed at this line - see
*	Set_FlowInform() in EagleCE_Host.cpp.
********************************************************************/

#ifndef __LINE_CURL_H__
#define __LINE_CURL_H__

#include "../../Template/MFSCtrlThread.h"
#include "LineCtrl.h"
#include <Ws2tcpip.h>

#pragma comment (lib, "libcurl.lib")

#include ".\curl/curl.h"

/** ***************************************************
*	@class CLineCurl
*	@brief Host TCP/TLS line implemented on libcurl.
*******************************************************/
class CLineCurl : public CLineCtrl, public CMFSCtrlThread
{
public:
	CLineCurl();
	~CLineCurl();

	/* LINECTRL INTERFACE */
public:
	virtual int	LineOpen(LPCTSTR Dest_Info,				/*ip:[xxx.xxx.xxx.xxx]*/
						 LPCTSTR dest_port=_T(""),		/*port:[xxxxx]*/
						 LPCTSTR sOption=_T(""),		/*SSL mode: "1" - enable, "0" - disable*/
						 LPCTSTR mode=_T("0"));			/*socket mode: "1" - ready for AMS (unsupported), "0" - host connect mode*/
	virtual int	LineClose();
	virtual int	LineSendData(BYTE *send_data, int len);
	virtual int LineAccept(BYTE opt);					/*not supported - curl cannot accept*/
	virtual int LineCallState(int nMode=0);

	virtual int TestPing(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply);

	/* THREAD */
protected:
	unsigned ThreadHandlerProc(void);

private:
	void	CleanUp();
	CString	BuildUrl(LPCTSTR lpszHost, LPCTSTR lpszPort, BOOL bUseSSL);
	BOOL	WaitForSocket(BOOL bForRead, int nTimeoutMs);

private:
	CURL*		m_pCurl;			///< easy handle, guarded by m_csHandle
	SOCKET		m_Socket;			///< CURLINFO_ACTIVESOCKET, used for select() only

	// libcurl forbids touching one easy handle from two threads at once, and
	// LineSendData() (AP thread) races the receive thread here - so unlike
	// CLineTcp, every curl_easy_send/recv/cleanup is serialised.
	CRITICAL_SECTION	m_csHandle;
	BOOL				m_bHandleLockInit;

	BOOL		m_bConnectStatus;
	BOOL		m_bUseSSL;
	int			m_nTCPConnectionTimeout;

	BOOL		m_bExitThread;
	BOOL		m_bCheckSocket;
	BOOL		m_bRequestConnect;

	CMFSEvent	m_eStartCheckSocket;
};

static CLineCurl theCLineCurl;

#endif //__LINE_CURL_H__
