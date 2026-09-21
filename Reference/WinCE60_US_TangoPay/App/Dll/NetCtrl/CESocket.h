// This Winsock Wrapper replaces all MFC socket classes (CSocket, CAsyncSocket
// CCeSocket). It offers async notifications, buffered read and easy data access
// functions.
//
// Coded by NautilusHyosung / @hyosung.com / 2007.08.22
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CESOCKET_H__DEAC64AC_C034_4162_9C57_5C75CBEDE2BB__INCLUDED_)
#define AFX_CESOCKET_H__DEAC64AC_C034_4162_9C57_5C75CBEDE2BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _WIN32_WCE
	#define _WCE_SECTION
#endif

#ifdef _WCE_SECTION
	#pragma comment(lib,"ws2.lib")
	//#pragma comment(lib,"winsock.lib")
#else
	#pragma comment (lib, "ws2_32.lib")
	#include <afxmt.h>
#endif

#ifdef _WCE_SECTION
	#include <winsock2.h>
	//#include <winsock.h>
#else
	#include <winsock2.h>
#endif

#pragma comment (lib, "libeay32.lib")
#pragma comment (lib, "ssleay32.lib")

// [2ND] AIREAT 2009.04.15
//#include "containers.h"
//#include <openssl/ssl.h>
//#include <openssl/err.h>
#include ".\openssl/ssl.h"
#include ".\openssl/err.h"
// [2ND]


#define TCPBUFFERSIZE 2048/*NETBUFFSIZE==2048*/
#define THREAD_TERMINATION_MAXWAIT	2	//seconds


class CCESocket  
{
public:
	//! Possible socket states
	enum socketState {NONE=0, DISCONNECTING, CREATED, CONNECTED, ACCEPTING};
	//! Asyncronous events sent with the OnClose notification
	enum asyncEvents {EVN_CONNCLOSED=0, EVN_CONNLOST, EVN_SERVERDOWN};

	///////////////////////////////////////////////////////
	// CREATE FUNCTIONS
	///////////////////////////////////////////////////////
	CCESocket();
	virtual ~CCESocket();
	bool Create(int socketType, int bufferSize = 0);

	///////////////////////////////////////////////////////
	// CONNECT FUNCTIONS
	///////////////////////////////////////////////////////
	int Connect(CString addr, UINT remotePort/*CString remotePort*/, bool isSSLConnect = FALSE);
	bool Listen(UINT localPort, int maxConn = 5);
	void Disconnect();

	///////////////////////////////////////////////////////
	// SENDING FUNCTIONS
	///////////////////////////////////////////////////////
	int Send(const char* buf, int len);

	///////////////////////////////////////////////////////
	// ASYNCHRONOUS NOTIFICATIONS
	///////////////////////////////////////////////////////
	virtual bool OnReceive(char* buf, int len){return FALSE;}
	virtual void OnReceive(){}
	virtual bool OnAccept(SOCKET serviceSocket){return FALSE;}
	virtual void OnClose(/*asyncEvents*/ int closeEvent){}
	virtual int OnEvent(int evt){return FALSE;}
	bool AcceptServiceSocket(SOCKET serviceSocket);

	///////////////////////////////////////////////////////
	// OTHER FUNCTIONS
	///////////////////////////////////////////////////////
	void SetBufferSize(int bufSize);
	int GetLastError() {return m_errorCode;}
	int GetSocketType() {return m_socketType;}

	//! @return 0=NONE, 1=DISCONNECTED, 2=CREATED, 3=CONNECTED, 4=ACCEPTING.
	int GetSocketState() {return m_socketState;}
	int ssl_Connect();
	void ssl_Disconnect();


protected:
	enum threadState {CLOSED=0, CLOSING, RUNNING};

	static DWORD WINAPI StartThread(LPVOID pParam);
	static DWORD WINAPI CheckConnectThread(LPVOID pParma);
	void ReadThread();
	void ListenThread();

	static bool m_bWSAStarted;
	SOCKET s;
	int m_socketType;
	socketState m_socketState;
	SOCKADDR_IN m_remoteAddress;
	SOCKADDR_IN m_localAddress;
	int m_receiveAddrSz;
	HANDLE m_readThread;
	HANDLE m_listenThread;
	HANDLE m_checkConnectThread;
	threadState m_readThreadState;
	threadState m_listenThreadState;
	threadState m_checkConnectThreadState;
	int m_errorCode;
	char *m_recvbuf;
	int m_recvBufSize;

	bool m_bIsSSLConnected;
	SSL_METHOD* method;
	SSL_CTX * ctx;
	SSL *ssl;
	X509 *cert;
};

#endif // !defined(AFX_CESOCKET_H__DEAC64AC_C034_4162_9C57_5C75CBEDE2BB__INCLUDED_)



