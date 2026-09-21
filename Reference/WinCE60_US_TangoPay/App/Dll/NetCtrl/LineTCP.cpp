#include "stdafx.h"
#include "LineTcp.h"

//#define NH_DEBUG

#include ".\Common\NHDbgApi.h"
#include ".\Common\ConstDef.h"	// [#2388] US Justin include Constant
#include "Queue.h"

//------------------------------------------------------------------
//	Define extern function
//------------------------------------------------------------------
extern int		put_data(char *recv_data, int recv_size);
extern int		put_event(char recv_evt);

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------
#define DBG_ERR		1
#define DBG_CALL	1
#define DBG_INFO	1

//------------------------------------------------------------------
//	Define Return Value
//------------------------------------------------------------------
#define RET_OK	0
#define RET_NG	1
#define RET_NG_SSL	22 //[#2093] NH PCS 2011.10.25 "리턴값이 22가 되어야 에러코드가 D150100으로 나옴"


CLineTcp::CLineTcp() : m_eStartCheckSocket(FALSE, FALSE)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	CLineCtrl::Register(LINE_TCP, this);

	WSADATA wsd;

	// Load Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
		NHDEBUG(DBG_ERR, (L"[LineTcp] unable to load Winsock!\n"));
    }

	// Initial
	m_Socket = INVALID_SOCKET;
	m_ListenSocket = INVALID_SOCKET;
	m_pRemoteAddr = NULL;
	m_bRequestConnect = FALSE;

	m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27

	m_nTCPConnectionTimeout = 30;	// [#2234] NH KSK 2014.03.03

	Resume();
}


CLineTcp::~CLineTcp()
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	CleanUp();

	m_bExitThread = TRUE;

	// Unload Winsock
	if (WSACleanup() == SOCKET_ERROR)
	{
		NHDEBUG(DBG_ERR, (L"[LineTcp] unable to load Winsock!\n"));
	}
}

int CLineTcp::GetLineType()
{
	return LINE_TCP;
}

void CLineTcp::CleanUp()
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	m_bCheckConnect = FALSE;
	m_bCheckSocket = FALSE;

	m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27	

	NHDEBUG(DBG_ERR, (L"[LineTcp] m_bCheckSocket FALSE set\n"));
		
	m_bRequestConnect = FALSE;

	// close communication socket
	if (m_Socket != INVALID_SOCKET)
	{
		shutdown(m_Socket, SD_BOTH);
		NHDEBUG(DBG_ERR, (L"[LineTcp] shutdown (%x)\n", m_Socket));
		closesocket(m_Socket);
		NHDEBUG(DBG_ERR, (L"[LineTcp] closesocket (%x)\n", m_Socket));
		m_Socket = INVALID_SOCKET;
	}

	// close listen socket
	if (m_ListenSocket != INVALID_SOCKET)
	{
		shutdown(m_ListenSocket, SD_BOTH);
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}

	// delete addr info
	if (m_pRemoteAddr != NULL)
	{
		freeaddrinfo(m_pRemoteAddr);
		m_pRemoteAddr = NULL;
	}
}

int	CLineTcp::LineOpen(LPCTSTR Dest_Info,		/*ip:[xxx.xxx.xxx.xxx]*/
					   LPCTSTR dest_port,		/*port:[xxxxx]*/ 
					   LPCTSTR sOption,			/*SSL mode: "1" - v2 enable, "2" - v3 enable, "0" - disable, "7" - TCP/IP Test*/	 
					   LPCTSTR mode)			/*socket mode: "1" - ready for Rms, "0" - host connect mode	"2" - RMS connect mode*/
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	CString option = L"";

	int		nServerMode = _ttoi(mode);
	CString	strTemp;		// [#2257] NH KSK 2014.03.03 Log 강화를 위한 변수

	// [#2234] NH KSK 2014.03.03
	if (nServerMode == 2)
	{
		m_nTCPConnectionTimeout = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RMS_TIMEOUT_CONNECT_TCPIP);
		nServerMode = 0;	// Client Mode로 변환
	}
	else
	{
		m_nTCPConnectionTimeout = 30;	// Default로 Setting
	}
	strTemp.Format(L"R_C_TO:%d", m_nTCPConnectionTimeout);
	NVDump('O', 'A', "11", L"", strTemp); // [#2171] NH KSK 2012.12.26
	// end of [#2234]

	if ((m_Socket != INVALID_SOCKET) || (m_ListenSocket != INVALID_SOCKET))
	{
		NHDEBUG(DBG_ERR, (L"[LineTcp] Socket already use Socket[%d], ListenSocket[%d]\n", m_Socket, m_ListenSocket));
		CleanUp();

		NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG1"); // [#2024] NH KSK 2011.02.24

		return RET_NG;
	}
	
	// Init value
	m_bRequestConnect = FALSE;
	m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27	

	// Client Mode
	if (nServerMode == 0)
	{
		NHDEBUG(DBG_INFO, (L"CLIENT MODE\n"));

		// 1. resolve address
		m_pRemoteAddr = ResolveAddress(Dest_Info, dest_port);
		if (m_pRemoteAddr == NULL)
		{

			NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG2"); // [#2024] NH KSK 2011.02.24

			return RET_NG;
		}

		// 2. Create Socket
		m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_Socket == INVALID_SOCKET)
		{
			NHDEBUG(DBG_ERR, (L"[LineTcp] Socket Create Failed err[%d]\n", WSAGetLastError()));
			CleanUp();

			NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG3"); // [#2024] NH KSK 2011.02.24

			return RET_NG;
		}

		// 3. Set socket option
		{
			BOOL	bDontLinger = TRUE;
			if (setsockopt(m_Socket, SOL_SOCKET, SO_DONTLINGER, (char*)&bDontLinger, sizeof(BOOL)) != 0)
			{
				NHDEBUG(DBG_ERR, (L"[LineTcp] fail to set Socket SO_DONTLINGER option err[%d]\n", WSAGetLastError()));
				CleanUp();

				NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG4"); // [#2024] NH KSK 2011.02.24

				return RET_NG;
			}

			BOOL	bKeepAlive = TRUE;
			if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(BOOL)) != 0)
			{
				NHDEBUG(DBG_ERR, (L"[LineTcp] fail to set Socket SO_KEEPALIVE option err[%d]\n", WSAGetLastError()));
				CleanUp();

				NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG5"); // [#2024] NH KSK 2011.02.24

				return RET_NG;
			}
		}

		// 4. Connect
		m_bCheckConnect = TRUE;	// [#2096] NH KSK 2011.11.01 Connect시 무한 대기 방지를 위해 Timer 기동하도록 추가함
		m_eStartCheckSocket.Set();	// [#2096] NH KSK 2011.11.01 Connect시 무한 대기 방지를 위해 Timer 기동하도록 추가함

		NHDEBUG(DBG_INFO, (L"[LINE_TCP] SOCKET CONNECT START\n"));
		if (connect(m_Socket, m_pRemoteAddr->ai_addr, m_pRemoteAddr->ai_addrlen) == SOCKET_ERROR)
		{
			NHDEBUG(DBG_ERR, (L"[LineTcp] connect failed: err[%d]\n", WSAGetLastError()));
			CleanUp();

			NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG6"); // [#2024] NH KSK 2011.02.24

			return RET_NG;
		}
		NHDEBUG(DBG_INFO, (L"[LINE_TCP] SOCKET CONNEC OK\n"));

		// 6. Start Check Soket
		m_bCheckConnect = FALSE;
		m_bCheckSocket = TRUE;
		//m_eStartCheckSocket.Set();

		m_bConnectStatus = TRUE;	// [#2075] NH KSK 2011.06.27
		put_event(CONNECT_EVT);
	}
	// Server Mode
	else
	{
		NHDEBUG(DBG_INFO, (L"SERVER MODE\n"));

		// 1. Create Socket
		m_ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_ListenSocket == INVALID_SOCKET)
		{
			NHDEBUG(DBG_ERR, (L"[LineTcp] Listen Socket Create Failed err[%d]\n", WSAGetLastError()));
			CleanUp();

			NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG12"); // [#2024] NH KSK 2011.02.24

			return RET_NG;
		}

		// 2. Set socket option
		{
			BOOL	bDontLinger = TRUE;
			if (setsockopt(m_ListenSocket, SOL_SOCKET, SO_DONTLINGER, (char*)&bDontLinger, sizeof(BOOL)) != 0)
			{
				NHDEBUG(DBG_ERR, (L"[LineTcp] fail to set Listen Socket SO_DONTLINGER option err[%d]\n", WSAGetLastError()));
				CleanUp();

				NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG13"); // [#2024] NH KSK 2011.02.24

				return RET_NG;
			}

			BOOL	bKeepAlive = TRUE;
			if (setsockopt(m_ListenSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(BOOL)) != 0)
			{
				NHDEBUG(DBG_ERR, (L"[LineTcp] fail to set Listen Socket SO_KEEPALIVE option err[%d]\n", WSAGetLastError()));
				CleanUp();

				NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG14"); // [#2024] NH KSK 2011.02.24

				return RET_NG;
			}
		}

		// 3. Bind
		{
			SOCKADDR_IN		ServerAddr;

			memset(&ServerAddr, 0, sizeof(SOCKADDR_IN));
			ServerAddr.sin_family = AF_INET;
			ServerAddr.sin_port = htons((unsigned short)_ttoi(dest_port));
			ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

			if (bind(m_ListenSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
			{
				NHDEBUG(DBG_ERR, (L"[LineTcp] fail to bind Listen Socket err[%d]\n", WSAGetLastError()));
				CleanUp();

				NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG15"); // [#2024] NH KSK 2011.02.24

				return RET_NG;
			}
		}

		// 4. Listen
		if (listen(m_ListenSocket, 5) == SOCKET_ERROR)
		{
			NHDEBUG(DBG_ERR, (L"[LineTcp] fail to listen err[%d]\n", WSAGetLastError()));
			CleanUp();

			NVDump('F', 'A', "11", L"", L"TCP_L_OPEN_NG16"); // [#2024] NH KSK 2011.02.24

			return RET_NG;
		}

		// 5. Start Check Soket
		m_bCheckSocket = TRUE;
		m_eStartCheckSocket.Set();
	}

	return RET_OK;
}

int	CLineTcp::LineClose()
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27	

	if (m_Socket == INVALID_SOCKET && 
		m_ListenSocket == INVALID_SOCKET)
		return RET_OK;   //이미 Close된 상태이므로 RET_OK 리턴함 //SOOK 2010.01.08 

	m_bCheckSocket = FALSE;
	m_bCheckConnect = FALSE;
	CleanUp();

	if (m_eMethodUse.Wait(1100) == WAIT_OBJECT_0)
		m_eMethodUse.Set();

	return RET_OK;
}

/*accept option: '0x01' - call accept, '0x00' - call deny*/
int CLineTcp::LineAccept(BYTE opt)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	if ((m_ListenSocket == INVALID_SOCKET) ||		// don't create.
		(m_Socket != INVALID_SOCKET))				// already aceepted. we support only one connection.
	{
		NHDEBUG(DBG_ERR, (L"[LineTcp] Listen Socket[%d] didn't create or Socket[%d] Already Aceepted.\n", m_ListenSocket, m_Socket));
		return RET_OK;	// 리턴값이 반대임. ㅠㅠ;;
	}

	// Accept
	{
		CNHAutoLock	Sync(m_eMethodUse);

		SOCKADDR_IN		ClientAddr;
		int				nClientAddrLen;

		memset(&ClientAddr, 0, sizeof(ClientAddr));
		nClientAddrLen = sizeof(ClientAddr);
		m_Socket = accept(m_ListenSocket, (SOCKADDR*)&ClientAddr, &nClientAddrLen);
		if (m_Socket == INVALID_SOCKET)
		{
			NHDEBUG(DBG_ERR, (L"[LineTcp] fail to accept from remote err[%d]\n", WSAGetLastError()));

			NVDump('F', 'A', "11", L"", L"TCP_L_ACEP_NG1"); // [#2024] NH KSK 2011.02.24

			return RET_OK;	// 리턴값이 반대임. ㅠㅠ;;
		}

		NHDEBUG(DBG_INFO, (L"[LineTcp] accept successful.\n"));

		m_bRequestConnect = FALSE;

		m_bConnectStatus = TRUE;	// [#2075] NH KSK 2011.06.27
		put_event(CONNECT_EVT);
	}

	return RET_NG;	// 리턴값이 반대임. ㅠㅠ;;
}

int CLineTcp::LineCallState(int nMode)
{
	// [#2075] NH KSK 2011.06.27
//	return (int)m_bRequestConnect;
	if (nMode == 0)
		return (int)m_bRequestConnect;

	return m_bConnectStatus;
	// end of [#2075]
}


int	CLineTcp::LineSendData(BYTE *send_data, int len)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]-[%d]\n", TEXT(__FUNCTION__), len));

	int	nSent = 0;
	int	ret;

	if (m_Socket == INVALID_SOCKET)
		return RET_NG;

	while(nSent < len)
	{
		ret = send(m_Socket, (char*)&send_data[nSent], (len-nSent), 0);

		if (ret == SOCKET_ERROR)
		{
			CleanUp();

			NVDump('F', 'A', "11", L"", L"TCP_L_SND_NG1"); // [#2024] NH KSK 2011.02.24

			NHDEBUG(DBG_INFO, (L"Send SOCKET ERROR\n"));

			return RET_NG;
		}
		NHDEBUG(DBG_INFO, (L"Send Data(%d)\n", ret));
		nSent += ret;
	}

	return RET_OK;
}

addrinfo* CLineTcp::ResolveAddress(LPCTSTR lpszAddr, LPCTSTR lpszPort)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s], ADDR[%s], PORT[%s]\n", TEXT(__FUNCTION__), lpszAddr, lpszPort));

    struct addrinfo hints,  *pAddr = NULL;
    int             rc;
	char	szAddr[256] ={0,};
	char	szPort[256] ={0,};
	CString strIpAddr;

	// check IP ADDRESS 
	strIpAddr = lpszAddr;
	if (strIpAddr.GetLength() <= 15)
	{
		int		nDot = 0;
		BOOL	bNumeric = TRUE;
		
		for (int i = 0; i < strIpAddr.GetLength(); i++)
		{
			if (strIpAddr[i] == '.')
				nDot++;
			else if (strIpAddr[i] < '0' || strIpAddr[i] > '9')
				bNumeric = FALSE;
		}

		// replace '000' -> '0'
		if (nDot == 3 && bNumeric == TRUE)
		{
			int		nStartIndex, nLastIndex;
			CString strTemp, strNewIpAddr;

			nStartIndex = 0;
			nLastIndex = strIpAddr.Find('.');
			while (nLastIndex != -1)
			{
				strTemp.Format(L"%d", _ttoi((LPCTSTR)strIpAddr.Mid(nStartIndex, nLastIndex)));

				if (strNewIpAddr.GetLength() > 0)
					strNewIpAddr += _T(".");
				strNewIpAddr += strTemp;

				nStartIndex = nLastIndex+1;
				nLastIndex = strIpAddr.Find('.', nStartIndex);
				if (nLastIndex == -1 && nStartIndex < strIpAddr.GetLength())
					nLastIndex = strIpAddr.GetLength() - 1;
			}
			strIpAddr = strNewIpAddr;
		}

		NHDEBUG(DBG_INFO, (L"CONVERT IPADDR : [%s]\n", strIpAddr));
	}

	// check port range
	int	nPort = _ttoi(lpszPort);
	if (nPort < 0 || nPort > 65535)
	{
		NHDEBUG(DBG_ERR, (L"[LineTcp] Invalid Port range port[%d]\n", nPort));
		return NULL;
	}

	// convert string unicode -> multibyte
	WideToMulti(szAddr, strIpAddr, 256);
	WideToMulti(szPort, lpszPort, 256);

	// make hint info
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags  = ((szAddr) ? 0 : AI_PASSIVE);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

	// get address
    rc = getaddrinfo(szAddr, szPort, &hints, &pAddr);
    if (rc != 0)
    {
		NHDEBUG(DBG_ERR, (L"[LineTcp] Invalid address [%s][%s], getaddrinfo failed: %d\n", lpszAddr, lpszPort, rc));
        return NULL;
    }

    return pAddr;
}


unsigned CLineTcp::ThreadHandlerProc()
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	struct fd_set		fdread,
						fdexcept;
    struct timeval		timeout;
	int					rc;
	SOCKET				CheckSocket;
	int					nWaitCount;

	m_bExitThread = FALSE;

	while(m_bExitThread != TRUE)
	{
		if (m_eStartCheckSocket.Wait(1000) != WAIT_OBJECT_0)
			continue;
		
		NHDEBUG(DBG_INFO, (L"GO GO CHECK - CONNECT TIMEOUT\n"));
		nWaitCount = 0;

		/* CHECK -- SSL CONNECTION TIMEOUT */
		while(m_bCheckConnect)
		{
			// Max 30sec
//			if (nWaitCount < 30)
			if (nWaitCount < m_nTCPConnectionTimeout)	// [#2234] NH KSK 2014.03.03
			{
				NHDEBUG(DBG_INFO, (L"TIMEOUT COUNT(%d)\n", nWaitCount));
				m_eStartCheckSocket.Wait(1000);
				nWaitCount++;
			}
			else
			{
				NVDump('F', 'A', "11", L"", L"TCP_CON_TO");			// [#2096] NH KSK 2011.11.01

				CleanUp();	// [#2096] NH KSK 2011.11.01 TCP/IP connect시 timeout시 발생하지 않는 Bug Fix
				NHDEBUG(DBG_ERR, (L"[LineTcp] timeout to wait connect\n"));
				break;
			}
		} // end of while(m_bCheckConnect)

		NHDEBUG(DBG_INFO, (L"GO GO CHECK - CONNECTED\n"));

		/* CHECK -- DATA RECV FROM REMOTE */
		while(m_bCheckSocket)
		{
			CNHAutoLock	Sync(m_eMethodUse);

			// select to check socket
			if (m_Socket == INVALID_SOCKET)
				CheckSocket = m_ListenSocket;		// when wait for connect to remote
			else
				CheckSocket = m_Socket;				// when connected to remote

			// clear set
			FD_ZERO(&fdread);
			FD_ZERO(&fdexcept);
		
			// add socket to set
			FD_SET(CheckSocket, &fdread);
			FD_SET(CheckSocket, &fdexcept);

			// set timeout value
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			rc = select(0, &fdread, NULL, &fdexcept, &timeout);
			if (rc == SOCKET_ERROR)
			{
				NHDEBUG(DBG_ERR, (L"[LineTcp] select failed : err[%d]\n", WSAGetLastError()));
				break;
			}
			// rc '0' is timeout.
			else if (rc != 0)
			{
				if (FD_ISSET(CheckSocket, &fdread))
				{
					// Recv Data from remote
					if (CheckSocket == m_Socket)
					{
						//while (1)
						{
							// [#2049] NH KSK 2011.04.18 select 1초 delay time 문제로 인해 동기화 안맞는 현상 수정을 위해 flag를 한번 더 Check하도록 수정
							if (m_bCheckSocket == FALSE)	// AP에서 Line Close를 했으므로, Thread 대기하도록 함
							{
								NHDEBUG(DBG_ERR, (L"[ThreadHandlerProc] m_bCheckSocket FALSE Detected\n"));
								break;
							}
							// end of [#2049]

							while(1)
							{
								rc = recv(m_Socket, m_RecvBuf, sizeof(m_RecvBuf), 0);
								NHDEBUG(DBG_INFO, (L"[ThreadHandlerProc] recv call (%d)\n", rc));

								// [#RWC6-4] US William 2019.09.11 D1706 and WinATM.exe crash
								if (rc == SOCKET_ERROR)
								{
									NHDEBUG(DBG_INFO, (L"[LineTcp] TCP SOCKET_ERROR: %d\r\n", rc));
									break;
								}

								// Read data again if ATM receives Maximum data size (CLineCtrl::m_RecvBuf[2048])
								if(rc >= sizeof(m_RecvBuf))		
								{
									put_data(m_RecvBuf, rc);
									NHDEBUG(DBG_INFO, (L"[LineTcp] Read Maximum data [%d], Read Again\n", rc));
								}
								else
								{
									break;
								}
							}
							// End of [#2449]

							// successfully full read
							if (rc > 0)
							{
								put_data(m_RecvBuf, rc);
								NHDEBUG(DBG_INFO, (L"[LineTcp] receive data [%d]\n", rc));
							}
							else if (rc == 0 || rc == SOCKET_ERROR)
							{
								NHDEBUG(DBG_INFO, (L"[LineTcp] receive data error [%d]\n", WSAGetLastError()));
								// break;		// [#2411] US Justin 2016.04.01 Timeout Bug Fix.. TCP already has an Error.. 
							}
						}

						// an error occur
						if (rc == SOCKET_ERROR)
						{
							int err = WSAGetLastError();	// [#RWC6-4] US William 2019.09.11 D1706 and WinATM.exe crash
							NHDEBUG(DBG_ERR, (L"[LineTcp] receive return error: %d\r\n", err));
							m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27
							put_event(DISCONNECT_EVT);
							break;
						}
						// gracefully closed
						else if (rc == 0)
						{
							NHDEBUG(DBG_ERR, (L"[LineTcp] connection gracefully closed\n"));
							m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27
							put_event(DISCONNECT_EVT);
							break;
						}
					}
					// Request connection from remote
					else
					{
						m_bRequestConnect = TRUE;
						NHDEBUG(DBG_INFO, (L"REQUEST CONNECTION.. WAIT CONNECT COMMAND\n"));
						m_eStartCheckSocket.Wait(1000);
					}
				}

				if (FD_ISSET(CheckSocket, &fdexcept))
				{
					// [#2497] AU Kook 2017.10.18	fix an unexpected disconnection from ALPHI.
					int error = 0;
					socklen_t len = sizeof( error );
					getsockopt(CheckSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);

					NHDEBUG(DBG_ERR, (L"[LineTcp] errno: [%d]\n", error));
					NHDEBUG(DBG_ERR, (L"[LineTcp] Socket err[%d]\n", WSAGetLastError()));
					NHDEBUG(DBG_ERR, (L"[LineTcp] Receive Data(%d) - (0x%0X)\n", rc, m_RecvBuf[0]));

#if (ALPHI_DEBUG)
					WriteAlphiLogToFile(L"Socket Exception (OOB?)");
					CString strLog;
					for( int nIdx =0; nIdx < rc; nIdx++ )   
					{   
						int n = m_RecvBuf[nIdx];   
						CString csTemp;   
						csTemp.Format( _T("%x"), n );   
						strLog += csTemp;   
					} 
					WriteAlphiLogToFile(strLog);
#endif
					if (rc == 1 && m_RecvBuf[0] == NULL)
					{
						// NULL character received. This disconnects current connection so ignore it.
						NVDump('O', 'A', "11", L"", L"OOB_NULL");
#if (ALPHI_DEBUG)
						WriteAlphiLogToFile(L"NULL received.");
#endif
					}
					else
					// end of [#2497]
					{
						NHDEBUG(DBG_ERR, (L"[LineTcp] receive OOB data so just close the connection\n"));
						m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27
						put_event(DISCONNECT_EVT);
						break;
					}
				}
			}

		} // end of while(m_bCheckSocket)

		NHDEBUG(DBG_INFO, (L"WAIT NEXT CONNECTION\n"));

		m_bCheckSocket = FALSE;
		m_bCheckConnect = FALSE;

	} // end of while(m_bExitThread != TRUE)

	return 0;
}