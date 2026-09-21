// This Winsock Wrapper replaces all MFC socket classes (CSocket, CAsyncSocket
// CCeSocket). It offers async notifications, buffered read and easy data access
// functions.
//
// Coded by NautilusHyosung / @hyosung.com / 2007.08.22
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CESocket.h"
#include <stdlib.h>
#include <afxmt.h>
//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool CCESocket::m_bWSAStarted = FALSE;

CCESocket::CCESocket()
{
NHDEBUG(1, (_T("CCESocket() \n"), m_socketType, m_errorCode));
	WSADATA wsaData;

	s = INVALID_SOCKET;
	m_socketType = SOCK_STREAM;
	m_socketState = NONE;
	m_readThread = NULL;
	m_listenThread = NULL;
	m_checkConnectThread = NULL;
	m_readThreadState = CLOSED;
	m_listenThreadState = CLOSED;
	m_checkConnectThreadState = CLOSED;
	m_receiveAddrSz = sizeof(SOCKADDR_IN);
	m_errorCode = 0;
	m_recvBufSize = 0;
	m_bIsSSLConnected = FALSE;
	ssl = NULL;
	ctx = NULL;
	method = NULL;
	cert = NULL;

	if(!m_bWSAStarted)
	{
		m_errorCode = WSAStartup(MAKEWORD(2,2), &wsaData);	// winsock library start
		if(m_errorCode == 0)
			m_bWSAStarted = TRUE;
		else
			WSACleanup( );
	}

	m_recvbuf = new char[TCPBUFFERSIZE];
NHDEBUG(1, (_T("CCESocket() returned [socket type: %d] [errorcode: %d] \n"), m_socketType, m_errorCode));
}

CCESocket::~CCESocket()
{
NHDEBUG(1, (_T("~CCESocket() \n")));
	int watchDog;

	if(s != INVALID_SOCKET)
		Disconnect();

	watchDog = 0;
	while (m_readThread && watchDog < 3)
	{
		SetThreadPriority(m_readThread, THREAD_PRIORITY_HIGHEST);
		if((::WaitForSingleObject(m_readThread, 300) != WAIT_TIMEOUT) || watchDog >= THREAD_TERMINATION_MAXWAIT)
		{
			CloseHandle(m_readThread);
			m_readThread = NULL;
		}
		watchDog++;
	}

	watchDog = 0;
	while (m_listenThread && watchDog < 3)
	{
		SetThreadPriority(m_listenThread, THREAD_PRIORITY_HIGHEST);
		if((::WaitForSingleObject(m_listenThread, 300) != WAIT_TIMEOUT) || watchDog >= THREAD_TERMINATION_MAXWAIT)
		{
			CloseHandle(m_listenThread);
			m_listenThread = NULL;
		}
		watchDog++;
	}

	watchDog = 0;
	while (m_checkConnectThread && watchDog < 3)
	{
		SetThreadPriority(m_checkConnectThread, THREAD_PRIORITY_HIGHEST);
		if((::WaitForSingleObject(m_checkConnectThread, 300) != WAIT_TIMEOUT) || watchDog >= THREAD_TERMINATION_MAXWAIT)
		{
			CloseHandle(m_checkConnectThread);
			m_checkConnectThread = NULL;
		}
		watchDog++;
	}

	WSACleanup();
	m_bWSAStarted = FALSE;
	delete [] m_recvbuf;
NHDEBUG(1, (_T("~CCESocket() returned \n")));
}

void CCESocket::SetBufferSize(int bufSize)
{
	if(bufSize > 0)
		m_recvBufSize = __min(bufSize, TCPBUFFERSIZE);
}

bool CCESocket::Create(int socketType, int bufferSize)
{
NHDEBUG(1, (_T("Create()  \n")));

	if(!m_bWSAStarted)
		return FALSE;

	//Exit if the socket has been already created
	if(s != INVALID_SOCKET)
	{
		m_errorCode = WSAEACCES;
NHDEBUG(1, (_T("Create() returned: ~INVALID_SOCKET [m_errorCode:%d] \n"), m_errorCode));
		return FALSE;
	}

	switch(socketType)
	{
		case SOCK_STREAM:
			s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			break;
		default:
			m_errorCode = WSAEINVAL;
NHDEBUG(1, (_T("Create() returned: WSAEINVAL [m_errorCode:%d] \n"), m_errorCode));
			return FALSE;
	}
	
	if(s == INVALID_SOCKET)
	{
		m_errorCode = WSAGetLastError();
NHDEBUG(1, (_T("Create() returned: INVALID_SOCKET [m_errorCode:%d] \n"), m_errorCode));
		return FALSE;
	}
	
//*/
	BOOL dontLinger = TRUE;
	if(setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (char*) &dontLinger, sizeof(BOOL)) != 0)
	{
		m_errorCode = WSAGetLastError();
		if (s != INVALID_SOCKET)		// [CS#63] NH AIREAT 2008.3.14
			closesocket(s);
NHDEBUG(1, (_T("Create() returned: setsockopt SO_DONTLINGER[m_errorCode:%d] \n"), m_errorCode));
		return FALSE;
	}
/*/
	LINGER linger;
	linger.l_linger = 0;
	linger.l_onoff = 0;
	if(setsockopt(s, SOL_SOCKET, SO_LINGER, (char*) &linger, sizeof(LINGER)) != 0)
	{
		m_errorCode = WSAGetLastError();
		if (s != INVALID_SOCKET)		// [CS#63] NH AIREAT 2008.3.14
			closesocket(s);
		NHDEBUG(1, (_T("Create() returned: setsockopt SO_DONTLINGER[m_errorCode:%d] \n"), m_errorCode));
		return FALSE;
	}
//*/
	BOOL optKeepAlive = TRUE;
	if(setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*) &optKeepAlive, sizeof(BOOL)) != 0)
	{
		m_errorCode = WSAGetLastError();
		if (s != INVALID_SOCKET)		// [CS#63] NH AIREAT 2008.3.14
			closesocket(s);
NHDEBUG(1, (_T("Create() returned: setsockopt SO_KEEPALIVE[m_errorCode:%d] \n"), m_errorCode));
		return FALSE;
	}
	
	if(bufferSize == 0 || bufferSize > TCPBUFFERSIZE)	// [CS#60] NH AIREAT 2008.3.14 - && -> ||
		bufferSize = TCPBUFFERSIZE;
	// end of [CS#59]
	
	SetBufferSize(bufferSize);

	//OK, all done
	m_socketType = socketType;
	m_socketState = CREATED;

NHDEBUG(1, (_T("Create() return [socketType: %d] [bufferSize %d] \n"), m_socketType, m_recvBufSize));
	return TRUE;
}

int CCESocket::Connect(CString addr, UINT remotePort/*CString remotePort*/, bool isSSLConnect)
{
NHDEBUG(1, (_T("CCESocket::Connect [socket:%d] [addr:%s] [remotePort:%d] [isSSLConnect:%d] \n"), s, addr, remotePort, isSSLConnect));
	char hostStr[256];
	int wHostLen;
	LPTSTR wHost;
	ulong hostByIP;
	HOSTENT *hostByName;

	if(!m_bWSAStarted)
	{
NHDEBUG(1, (_T("CCESocket::Connect() return: [m_bWSAStarted: %d] \n"), -1));
		return -1;
	}

	//Make sure the socket was created
	if(s == INVALID_SOCKET)
	{
		m_errorCode = WSAENOTSOCK;
NHDEBUG(1, (_T("CCESocket::Connect() return: WSAENOTSOCK [m_errorCode:%d] \n"), m_errorCode));
		return -2;
	}

	//We cannot connect if the socket is already connected (TCP only)
	if(m_socketType == SOCK_STREAM && m_socketState > CREATED)
	{
		m_errorCode = WSAEISCONN;
NHDEBUG(1, (_T("CCESocket::Connect() return: WSAEISCONN [m_errorCode:%d] \n"), m_errorCode));
		return -3;
	}

	//Check port values
	if(remotePort > 65535)
	{
		m_errorCode = WSAEINVAL;
NHDEBUG(1, (_T("CCESocket::Connect() return: WSAEINVAL [m_errorCode:%d] \n"), m_errorCode));
		return -5;
	}

	//Gets address string and convert it from unicode to multibyte
	wHostLen = addr.GetLength();
	if(wHostLen > 256)
		wHostLen = 256;
	wHost = addr.GetBuffer(wHostLen);
	memset(hostStr, 0x00, sizeof(hostStr));

#ifdef _WIN32_WCE
	wcstombs(hostStr, wHost, 256);
#else
	wHost[wHostLen] = '\0';
	strcpy(hostStr, wHost);
#endif
	addr.ReleaseBuffer();

	//Builds destination address
	memset(&m_remoteAddress, 0, sizeof(SOCKADDR_IN));
	m_remoteAddress.sin_family = AF_INET;
	m_remoteAddress.sin_port = htons(remotePort);

	// 기존에 아래 주석과 같이 URL 인지 IP인지 체크하였으나, 체크가 제대로 되지 않는 경우가 발생하여
	// for loop를 사용하여 알파벳이 있는지 체크하고 알파벳이 있는경우 URL로 판단하도록 수정함.
	// [#385] [NH] PSC 2008.07.23
//	hostByName = gethostbyname(hostStr);
//	if(hostByName == NULL)

	bool fIsUrl = false;
	for(int i=0; i<wHostLen; i++)
	{
		if((hostStr[i] < '0' || hostStr[i] > '9') && hostStr[i] != '.')	// URL인지 체크.('.'을 제외한 '0'~'9' 범위가 아닌 값이 있으면 URL로 판단) 2008.08.19. psc
		{
			fIsUrl = true;
			break;
		}
	}

	if(fIsUrl)		// URL일 경우
	{
		hostByName = gethostbyname(hostStr);
		if(hostByName == NULL)		// Invalid URL
			return -6;
		m_remoteAddress.sin_addr = *((IN_ADDR*)hostByName->h_addr_list[0]);
	}
	else			// IP일 경우
	{
		CString strIP(L""), temp(L"");	

		// remove "0"pad string
		temp = strtok(hostStr, ".");
		if(temp.GetLength()<=3)
		{
			while(temp.GetLength() != NULL)		
			{		
				temp.Format(_T("%d"), _ttoi(temp));
				
				if(strIP.GetLength() > 0)
					strIP += L"." + temp;
				else
					strIP = temp;
				temp = L"";
				temp = strtok(NULL, ".");
			}

			wHostLen = strIP.GetLength();
			wHost = strIP.GetBuffer(wHostLen);
			memset(hostStr, 0x00, sizeof(hostStr));

//////////////////////////////////////////////////////////////////////////
#ifdef _WIN32_WCE
			wcstombs(hostStr, wHost, 256);
#else
			wHost[wHostLen] = '\0';
			strcpy(hostStr, wHost);
#endif
			strIP.ReleaseBuffer();
//////////////////////////////////////////////////////////////////////////
			
			hostByIP = inet_addr(hostStr);
			if(hostByIP == INADDR_NONE)		// Invalid IP address
			{
NHDEBUG(1, (_T("CCESocket::Connect() return: INADDR_NONE [m_errorCode:%d] \n"), m_errorCode));
				return -6;
			}
			m_remoteAddress.sin_addr.s_addr = hostByIP;
		}
		else
		{
NHDEBUG(1, (_T("CCESocket::Connect() return: hostByName != NULL [m_errorCode:%d] \n"), m_errorCode));
			return -7;
		}
	}
	// end of [#385]

	//Connects if TCP.
	if(m_socketType == SOCK_STREAM)
	{
		if(connect(s, (SOCKADDR*) &m_remoteAddress, sizeof(SOCKADDR_IN)) != 0)
		{
			m_errorCode = WSAGetLastError();
NHDEBUG(1, (_T("CCESocket::Connect() return: connect [m_errorCode:%d] \n"), m_errorCode));
			return -8;
		}
		m_socketState = CONNECTED;

		//TCP: Start reading thread
		if(isSSLConnect)
		{
			m_checkConnectThread = CreateThread(NULL, 0, CheckConnectThread, this, 0, NULL);
			int ret = ssl_Connect();
			if(ret < 1)
			{
NHDEBUG(1, (_T("CCESocket::Connect() return: ssl_Connect [ret:%d] \n"), ret));
				return -9;
			}
		}

		m_readThread = CreateThread(NULL, 0, StartThread, this, 0, NULL );
	}
	else
	{
NHDEBUG(1, (_T("CCESocket::Connect() return: m_socketType != SOCK_STREAM [m_socketType:%d] \n"), m_socketType));
		return -10;
	}

NHDEBUG(1, (_T("CCESocket::Connect() return: [m_readThread:%d] \n"), m_readThread));
	return TRUE;
}

void CCESocket::Disconnect()
{
NHDEBUG(1, (_T("CCESocket::Disconnect() [socketID:%d] \n"), s));
	if(s == INVALID_SOCKET)
	{
NHDEBUG(1, (_T("CCESocket::Disconnect() returned: INVALID_SOCKET [s: %d] \n"), s));
		return;
	}
	
	m_readThreadState = CLOSING;	// thread 종료조건 설정 2008.02.12
	m_listenThreadState = CLOSING;	// thread 종료조건 설정 2008.02.12
	m_checkConnectThreadState = CLOSING;
	
	if(m_bIsSSLConnected && ssl)
		ssl_Disconnect();

	//After a shutodown reading thread will self terminate
	m_socketState = DISCONNECTING;
	CString strLog;									// [#270] NH AIREAT 2008.06.09 - NV LOG
	int nRet;										// [#270] NH AIREAT 2008.06.09 - NV LOG
	NVDump('O', 'K', "01", L"", L"SHUTDOWN..." );	// [#270] NH AIREAT 2008.06.09 - NV LOG
	nRet = shutdown(s, SD_BOTH);
	strLog.Format(L"SHUTDOWN[0x%04X]", nRet);		// [#270] NH AIREAT 2008.06.09 - NV LOG
	NVDump('O', 'K', "01", L"", strLog );			// [#270] NH AIREAT 2008.06.09 - NV LOG

	nRet = closesocket(s);
	strLog.Format(L"CLOSE[0x%04X]", nRet);			// [#270] NH AIREAT 2008.06.09 - NV LOG
	NVDump('O', 'K', "01", L"", strLog );			// [#270] NH AIREAT 2008.06.09 - NV LOG

	s = INVALID_SOCKET;
	m_socketState = NONE;
	m_errorCode = 0;
NHDEBUG(1, (_T("CCESocket::Disconnect() returned [s: %d] \n"), s));
}

DWORD WINAPI CCESocket::StartThread(LPVOID pParam)
{
	CCESocket *parent = (CCESocket*) pParam;
	
	if(parent->m_socketState == ACCEPTING && parent->m_socketType == SOCK_STREAM)
		parent->ListenThread();
	else
		parent->ReadThread();
	
	return 0;
}

DWORD WINAPI CCESocket::CheckConnectThread(LPVOID pParam)
{
	CCESocket *parent = (CCESocket*) pParam;
NHDEBUG(1, (_T("CCESocket::CheckConnectThread() [pParam: %x] \n"), pParam));
	parent->m_checkConnectThreadState = RUNNING;
	for(int i=0; (i<300 && parent->m_checkConnectThreadState == RUNNING); i++)
	{
		if(parent->m_bIsSSLConnected)
		{
NHDEBUG(1, (_T("CCESocket::CheckConnectThread() return: 0")));
			parent->m_checkConnectThreadState = CLOSED;
			CloseHandle(parent->m_checkConnectThread);
			parent->m_checkConnectThread = NULL;
			return 0;
		}
		Delay_Msg(100);
	}

NHDEBUG(1, (_T("CCESocket::CheckConnectThread() CLOSING \n")));
	parent->Disconnect();
	parent->m_checkConnectThreadState = CLOSED;
	CloseHandle(parent->m_checkConnectThread);
	parent->m_checkConnectThread = NULL;
NHDEBUG(1, (_T("CCESocket::CheckConnectThread() return: ExitThread \n")));
//	ExitThread(0);

	return 0;
}

// Read any incoming data as well as detect the connected lost/reset
void CCESocket::ReadThread()
{
NHDEBUG(1, (_T("CCESocket::ReadThread() \n")));
	int bytesRead=0, bufSize=0;				// [CS#84] NH AIREAT 2008.3.14 cast (WORD)	
	
	m_readThreadState = RUNNING;
	bufSize = m_recvBufSize;
	
	fd_set  read_fds;
	int maxfdp = s+1;
	timeval time_interval;

	// [#448] [NH] KSK 2008.11.17
	// select 설정 누락되어 response data를 받지 못하는 현상 수정
//	FD_ZERO(&read_fds);
//	FD_SET(s, &read_fds);
//	timeval time_interval;
//	time_interval.tv_sec = 1;

//	timeval time_interval;
//	time_interval.tv_sec = 1;
//	time_interval.tv_usec = 0;
	// end of [#448]

	do 
	{
		//Blocking receive
		if(m_socketType == SOCK_STREAM)
		{
			{
				if(m_bIsSSLConnected)
				{
					while(m_readThreadState == RUNNING)
					{
						// [#448] [NH] KSK 2008.11.17
						FD_ZERO(&read_fds);
						FD_SET(s, &read_fds);
						time_interval.tv_sec = 1;	// PSC 2009.07.22 Timer 설정이 Clear되는 Bug Fix
						time_interval.tv_usec = 0;
						// end of [#448]

						//if(SSL_want_read(ssl))
						int ret = select(maxfdp, &read_fds, (fd_set *)0, (fd_set *)0,(struct timeval *)&time_interval);
						if(ret > 0 && m_readThreadState == RUNNING && FD_ISSET(s, &read_fds))
						{
							bytesRead = SSL_read(ssl, m_recvbuf, bufSize);
							break;
						}
						else
							Delay_Msg(100);
					}
				}
				else
					bytesRead = recv(s, m_recvbuf, bufSize, 0);
			}
NHDEBUG(1, (_T("CCESocket::ReadThread() recv() [len:%d] \n"), bytesRead));
		}
		
		//If there was an error exit thread and notify owner with OnClose
		if(bytesRead == SOCKET_ERROR || bytesRead <= 0)
		{
			m_errorCode = WSAGetLastError();
			NHDEBUG(1, (_T("WSAGetLastError(): %d\n"), m_errorCode));
NHDEBUG(1, (_T("CCESocket::ReadThread() returned: bytesRead [m_errorCode:%d] [bytesRead:%d] \n"), m_errorCode, bytesRead));
			break;
		}
		
		//If the client doesn't accepts the raw packet store it in the buffer
		if(!OnReceive(m_recvbuf, bytesRead))
		{
			//Notify that there are available data
NHDEBUG(1, (_T("CCESocket::ReadThread() onReceive() [len:%d] \n"), bytesRead));
			continue;
		}
	} while(m_readThreadState == RUNNING);
	
	m_readThreadState = CLOSING;
	if(m_socketState > DISCONNECTING)
	{
		OnClose(EVN_CONNCLOSED);
	}
	// Self terminate the thread
	m_readThreadState = CLOSED;
	CloseHandle(m_readThread);	// Handle이 증가하는 문제가 발생하여 추가 2008.02.12
	m_readThread = NULL;
NHDEBUG(1, (_T("CCESocket::ReadThread() returned \n")));
	//ExitThread(0);
}

int CCESocket::Send(const char* buf, int len)
{
NHDEBUG(1, (_T("CCESocket::Send() [len:%d] \n"), len));
	int dataPtr = 0;
	int sentBytes = 0;

	if(!m_bWSAStarted)
	{
NHDEBUG(1, (_T("CCESocket::Send() returned: m_bwSAStarted [m_bWSAStarted:%d] \n"), m_bWSAStarted));
		return SOCKET_ERROR;
	}

	//We cannot send data if the socket's not connected
	if(m_socketState < CONNECTED)
	{
		m_errorCode = WSAENOTCONN;
NHDEBUG(1, (_T("CCESocket::Send() returned: m_socketState [m_errorCode:%d] \n"), m_errorCode));
		return SOCKET_ERROR;
	}

	//TCP listening socket cannot send data
	if(m_socketState == ACCEPTING && m_socketType == SOCK_STREAM)
	{
		m_errorCode = WSAENOTCONN;
NHDEBUG(1, (_T("CCESocket::Send() returned: m_socketState [m_errorCode:%d] \n"), m_errorCode));
		return SOCKET_ERROR;
	}

	//Have we a valid buffer?
	if(!buf || len <= 0)
	{
		m_errorCode = WSAEFAULT;
NHDEBUG(1, (_T("CCESocket::Send() returned: m_socketState [m_errorCode:%d] \n"), m_errorCode));
		return SOCKET_ERROR;
	}

	if(m_socketType == SOCK_STREAM)
	{
		while(len > 0)
		{
			if(m_bIsSSLConnected)
				sentBytes = SSL_write(ssl, &buf[dataPtr], len);
			else
				sentBytes = send(s, &buf[dataPtr], len, 0);
NHDEBUG(0, (_T("CCESocket::Send() send() [len:%d] [buf:%S] \n"), sentBytes, &buf[dataPtr]));
			if(sentBytes == SOCKET_ERROR)
			{
				m_errorCode = WSAGetLastError();
NHDEBUG(0, (_T("CCESocket::Send() returned: sentBytes [m_errorCode:%d] \n"), m_errorCode));
				return SOCKET_ERROR;
			}
			dataPtr += sentBytes;
			len -= sentBytes;
		}
	}

NHDEBUG(1, (_T("CCESocket::Send() returned [send Len:%d] \n"), dataPtr));
	return dataPtr;
}


int CCESocket::ssl_Connect()
{
NHDEBUG(1, (_T("CCESocket::ssl_Connect() [s:%d] [ssl:%d] [ctx:%d] \n"), s, ssl, ctx));
	int ret = 0;
 	SSL_library_init();						// always return 1
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	method = SSLv23_client_method();
	if(method == NULL)
	{
NHDEBUG(1, (_T("CCESocket::ssl_Connect() returne: mehtod [metod:%d] \n"), method));
		return FALSE;
	}

	ctx = SSL_CTX_new(method);
	if(ctx == NULL)
	{
NHDEBUG(1, (_T("CCESocket::ssl_Connect() returne: ctx [ctx:%d] \n"), ctx));
		return FALSE;
	}

//	SSL_CTX_set_options(ctx, SSL_OP_ALL);	// optional
//	SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);
//	SSL_CTX_set_timeout(ctx, 30);
	ssl = SSL_new(ctx);
	if(ssl == NULL)
	{
NHDEBUG(1, (_T("CCESocket::ssl_Connect() returne: ssl [ssl:%d] \n"), ssl));
		return FALSE;
	}

	ret = SSL_set_fd(ssl, s);
	if(ret < 1)
	{
NHDEBUG(1, (_T("CCESocket::ssl_Connect() returne: ret [ret:%d] \n"), ret));
		return ret;
	}
	SSL_set_connect_state(ssl);
	SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
//	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

	ret = SSL_connect(ssl);
	if(ret > 0)
		m_bIsSSLConnected = TRUE;
	else
	{
		// [CS#64, 65] NH AIREAT 2008.3.14
		// if 문 삭제.
		SSL_clear(ssl);
		SSL_free(ssl);
		//delete ssl;
		ssl = NULL;
		
		SSL_CTX_free(ctx);
		//delete ctx;
		ctx = NULL;
		// end of [CS#64, 65]
		
		method = NULL;
		m_bIsSSLConnected = FALSE;
		
		ERR_clear_error();
		ERR_remove_state(0);

NHDEBUG(1, (_T("CCESocket::ssl_Connect() returne: SSL_connect [ret:%d] \n"), ret));
		return ret;
	}

NHDEBUG(1, (_T("CCESocket::ssl_Connect() returne [ret:%d] \n"), ret));
	return ret;
}

void CCESocket::ssl_Disconnect()
{
NHDEBUG(1, (_T("CCESocket::ssl_Disconnect() [s:%d] [ssl:%d] [ctx:%d] \n"), s, ssl, ctx));
	int ret = 0;
	if(ssl == NULL)
	{
NHDEBUG(1, (_T("CCESocket::ssl_Disconnect() returned: ssl == NULL [s:%d] [ssl:%d] [ctx:%d] \n"), s, ssl, ctx));
		return;
	}

	ret = SSL_shutdown(ssl);
	if(ret < 1)
	{
		shutdown(s, SD_SEND);
NHDEBUG(1, (_T("CCESocket::ssl_Disconnect() : shutdown() [s:%d] [ssl:%d] [ret:%d] \n"), s, ssl, ret));
	}

	if(ret > 0)		// [CS#66] NH AIREAT 2008.3.14
	{
		SSL_free(ssl);
		//delete ssl;
NHDEBUG(1, (_T("CCESocket::ssl_Disconnect() SSL_free: ret [ret:%d] [s:%d] [ssl:%d] [ctx:%d] \n"), ret, s, ssl, ctx));
	}
	else
	{
		// [CS#66] NH AIREAT 2008.3.14
		SSL_clear(ssl);
		SSL_free(ssl);
		//delete ssl;
NHDEBUG(1, (_T("CCESocket::ssl_Disconnect() SSL_clear: ret [ret:%d] [s:%d] [ssl:%d] [ctx:%d] \n"), ret, s, ssl, ctx));
		// end of [CS#66]
	}
	
	if(ctx)
	{
		SSL_CTX_free(ctx);
		//delete ctx;
	}
	ssl = NULL;
	ctx = NULL;
	
	method = NULL;
	m_bIsSSLConnected = FALSE;
	
	ERR_clear_error();
	ERR_remove_state(0);

NHDEBUG(1, (_T("CCESocket::ssl_Disconnect() returned [s:%d] [ssl:%d] [ctx:%d] \n"), s, ssl, ctx));
}



bool CCESocket::Listen(UINT localPort, int maxConn)
{
	if(!m_bWSAStarted)
		return FALSE;

	//Make sure the socket was created
	if(s == INVALID_SOCKET)
	{
		m_errorCode = WSAENOTSOCK;
NHDEBUG(1, (_T("CCESocket::Listen() [m_errorCode:%d] \n"), m_errorCode));
		return FALSE;
	}

	//We cannot connect if the socket is already connected or accepting
	if(m_socketState > CREATED)
	{
NHDEBUG(1, (_T("CCESocket::Listen() [m_errorCode:%d] \n"), m_errorCode));
		m_errorCode = WSAEISCONN;
		return FALSE;
	}

	//Check port values
	if(localPort > 65535)
	{
NHDEBUG(1, (_T("CCESocket::Listen() [m_errorCode:%d] \n"), m_errorCode));
		m_errorCode = WSAEINVAL;
		return FALSE;
	}

	memset(&m_localAddress, 0, sizeof(SOCKADDR_IN));
	m_localAddress.sin_family = AF_INET;
	m_localAddress.sin_port = htons(localPort);
	m_localAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(s, (SOCKADDR*) &m_localAddress, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		m_errorCode = WSAGetLastError();
NHDEBUG(1, (_T("CCESocket::Listen() [m_errorCode:%d] \n"), m_errorCode));
		return FALSE;
	}
	
	//The following is TCP listen + accept code
	if(listen(s, maxConn) == SOCKET_ERROR)
	{
		m_errorCode = WSAGetLastError();
NHDEBUG(1, (_T("CCESocket::Listen() [m_errorCode:%d] \n"), m_errorCode));
		return FALSE;
	}

	m_socketState = ACCEPTING;
	m_listenThread = CreateThread(NULL, 0, StartThread, this, 0, NULL );

	return TRUE;
}

bool CCESocket::AcceptServiceSocket(SOCKET serviceSocket)
{
	SOCKADDR_IN peerName;
	int nameSize = sizeof(SOCKADDR_IN);

	if(s == INVALID_SOCKET)
		return FALSE;

	//Is it connected?
	if(getpeername(serviceSocket, (SOCKADDR*) &peerName, &nameSize) == INVALID_SOCKET)
	{
		shutdown(serviceSocket, SD_BOTH);
		closesocket(serviceSocket);
		return FALSE;
	}

	Disconnect();
	s = serviceSocket;

	if(m_recvBufSize == 0)
		m_recvBufSize = TCPBUFFERSIZE;

	m_socketState = CONNECTED;
	m_readThread = CreateThread(NULL, 0, StartThread, this, 0, NULL );
	if(!m_readThread)
		return FALSE;

	return TRUE;
}

void CCESocket::ListenThread()
{
	int ret = -1;
	fd_set  read_fds;
	int maxfdp = s+1;
	SOCKET serviceSocket;

	m_listenThreadState = RUNNING;
	// [#448] [NH] KSK 2008.11.17
 //   FD_ZERO(&read_fds);
 //   FD_SET(s, &read_fds);

	timeval time_interval;
	time_interval.tv_sec = 1;
	time_interval.tv_usec = 0;
	// end of [#448]

NHDEBUG(1, (_T("CCESocket::ListenThread() start\n")));
	do 
	{
		// [#448] [NH] KSK 2008.11.17
		FD_ZERO(&read_fds);
		FD_SET(s, &read_fds);

		int ret = select(maxfdp, &read_fds, (fd_set *)0, (fd_set *)0,(struct timeval *)&time_interval);

NHDEBUG(1, (_T("CCESocket::ListenThread() select() out \n")));

		if(ret > 0 && m_listenThreadState == RUNNING && FD_ISSET(s, &read_fds))
		{
			memset(&m_remoteAddress, 0, sizeof(SOCKADDR_IN));
			serviceSocket = accept(s, (SOCKADDR*) &m_remoteAddress, &m_receiveAddrSz);

			if(serviceSocket == INVALID_SOCKET) 
			{
				m_errorCode = WSAGetLastError();
				break;
			}

			//request a new connection
			if(!OnAccept(serviceSocket))
			{
				shutdown(s, SD_BOTH);
				closesocket(s);
			}
		}
		else
			Delay_Msg(100);
		// end of [#448]

	} while(m_listenThreadState == RUNNING);

//	m_listenThreadState = CLOSING;
	
NHDEBUG(1, (_T("CCESocket::ListenThread() return \n")));
	// Self terminate the thread
	m_listenThreadState = CLOSED;
	CloseHandle(m_listenThread);	// Handle이 증가하는 문제가 발생하여 추가 2008.02.12
	m_listenThread = NULL;
//	ExitThread(0);
}

