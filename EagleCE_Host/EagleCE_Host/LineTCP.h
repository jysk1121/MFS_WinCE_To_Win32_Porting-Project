/** ***************************************************************
*	@file LineTCP.h 
*	@date 2017/10/20
*	@author	MFS	
*	@brief 실질적인 통신을 수행하는 TCP라인을 구현한 헤더파일입니다. 
********************************************************************/

#ifndef __LINE_TCP_H__
#define __LINE_TCP_H__

#include "../../Template/MFSCtrlThread.h"
#include "LineCtrl.h"
#include <Ws2tcpip.h>

#pragma comment (lib, "libcrypto.lib")
#pragma comment (lib, "libssl.lib")

#include ".\openssl/ssl.h"
#include ".\openssl/err.h"

/** ***************************************************
*	@class CLineTcp 
*	@date 2017/10/19 
*	@author MFS 
*	@brief 서버 또는 호스트와의 TCP 통신 구현 클래스입니다.
*	@details CLineCtrl 과 CMFSCtrlThread의 다중상속을 받았습니다.
*******************************************************/
class CLineTcp : public CLineCtrl, public CMFSCtrlThread
{
public:
	CLineTcp();
	~CLineTcp();

	/* LINECTRL INTERFACE */
public:
	virtual int	LineOpen(LPCTSTR Dest_Info,				/*ip:[xxx.xxx.xxx.xxx]*/
						 LPCTSTR dest_port=_T(""),		/*port:[xxxxx]*/ 
						 LPCTSTR sOption=_T(""),		/*SSL mode: "1" - enable, "0" - disable*/	 
						 LPCTSTR mode=_T("0"));			/*socket mode: "1" - ready for AMS, "0" - host connect mode*/
	virtual int	LineClose();
	virtual int	LineSendData(BYTE *send_data, int len);
	virtual int LineAccept(BYTE opt);					/*accept option: '0x01' - call accept, '0x00' - call deny*/
	virtual int LineCallState(int nMode=0);

	virtual int TestPing(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply);

private:
	BOOL	m_bConnectStatus;
	int		m_nTCPConnectionTimeout;

	/* THREAD */
protected:
	unsigned ThreadHandlerProc(void);	// 스레드의 Handler.

private:
	void		CleanUp();
	addrinfo*	ResolveAddress(LPCTSTR lpszAddr, LPCTSTR lpszPort);

private:
	SOCKET		m_Socket; ///< 커뮤니케이션용 소켓입니다. 
	SOCKET		m_ListenSocket; ///< 리스닝용 소켓입니다. 
	addrinfo	*m_pRemoteAddr; ///< 아이피 주소를 저장하는 변수입니다.   

	BOOL		m_bExitThread; ///< 쓰레드 탈출 상태 플래그입니다.     
	BOOL		m_bCheckSocket; ///< 소켓이 데이터를 받았는지 확인하는 플래그입니다. 
	BOOL		m_bCheckConnect; ///< 연결상태를 확인하는 플래그입니다. 

	CMFSEvent	m_eStartCheckSocket;

	BOOL		m_bUseListenSSL;	///< SSL Listen 여부를 확인하는 flag
	BOOL		m_bCheckSSLAccept;
	BOOL		m_bSocketClose_Forced;

	BOOL		m_bUseSSL;
	const SSL_METHOD* m_pSSLMethod;
	SSL_CTX*	m_pSSLCtx;
	SSL*		m_pSSL;

	BOOL		m_bRequestConnect;
};

static CLineTcp theCLineTCP;

#endif //__LINE_TCP_H__