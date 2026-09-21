#ifndef __NH_LINE_TCP_H__
#define __NH_LINE_TCP_H__

#include ".\Common\NHCtrlThread.h"
#include ".\Net\LineCtrl.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

#ifdef UNDER_CE
#  pragma comment (lib, "ws2.lib")
#else
#  pragma comment (lib, "ws2_32.lib")
#endif

class CLineTcp : public CLineCtrl, public CNHCtrlThread
{
public:
	CLineTcp();
	~CLineTcp();

	/* LINECTRL INTERFACE */
public:
	virtual int	LineOpen(LPCTSTR Dest_Info,			/*ip:[xxx.xxx.xxx.xxx]*/
						 LPCTSTR dest_port=L"",		/*port:[xxxxx]*/ 
						 LPCTSTR sOption=L"",		/*SSL mode: "1" - enable, "0" - disable*/	 
						 LPCTSTR mode=L"");			/*socket mode: "1" - ready for Rms, "0" - host connect mode*/
	virtual int	LineClose();
	virtual int	LineSendData(BYTE *send_data, int len);
	virtual int LineAccept(BYTE opt);				/*accept option: '0x01' - call accept, '0x00' - call deny*/
	virtual int LineCallState(int nMode=0);			// [#2075] NH KSK 2011.06.27
	virtual int GetLineType();						// [#RWC6-67] US William 2019.10.22 MoniView TLS

private:
	BOOL	m_bConnectStatus;						// [#2075] NH KSK 2011.06.27

	int		m_nTCPConnectionTimeout;				// [#2234] NH KSK 2014.03.03

	/* THREAD */
protected:
	unsigned ThreadHandlerProc(void);	// 스레드의 Handler.

private:
	void		CleanUp();
	addrinfo*	ResolveAddress(LPCTSTR lpszAddr, LPCTSTR lpszPort);

private:
	SOCKET		m_Socket;				// Communication Socket
	SOCKET		m_ListenSocket;			// Linstening Socket
	addrinfo	*m_pRemoteAddr;

	BOOL		m_bExitThread;
	BOOL		m_bCheckSocket;			// check flag : recv data
	BOOL		m_bCheckConnect;		// check flag : connect
	CNHEvent	m_eStartCheckSocket;

	BOOL		m_bRequestConnect;
};

static CLineTcp theCLineTCP;

#endif //__NH_LINE_TCP_H__