#ifndef		__MYCOMMTHREAD_H__
#define		__MYCOMMTHREAD_H__

#define QUEUE_MAX 1024


typedef enum {
	RX_NONE = 0,
	RX_ACK,
	RX_NAK,
	RX_RESPONSE,
	RX_RESPONSE_WITH_TEXT,
	LEN_ERROR,
	RX_ERROR
} RXTYPE;


class	CCommThread
{
public:
	CCommThread();
	~CCommThread();

	HWND m_hWnd;
	char *	m_sPortName;

	void ProcessRcvData( BYTE *byBuf, int nSize );
	BOOL Send_Packet(BYTE byCommand, BYTE byID, BYTE* pData, int nLen);
	int  Recv_Packet(BYTE byID, BYTE* pData);
	BOOL Recv_Buffer_Reset(BYTE byID);

	//BOOL bInitComPort(TCHAR * strPortName);
	BOOL bInitComPort(CString strPortName);
	void ClosePort();

	DWORD WriteComm(BYTE *pBuff, DWORD nToWrite);
	DWORD ReadComm(BYTE *pBuff, DWORD nToRead);
	
	BYTE Calculate_BCC(BYTE *buffer, WORD wleng);
	WORD Calc_CRC16(BYTE *buffer, int wleng);

//	BOOL WritePort(LPSTR lpBuf, DWORD nByteToWrite);
//	BOOL ReadPort(LPSTR lpBuf, DWORD nByteToRead);
//	RXTYPE GetAction(LPSTR lpBuf);
	
	BOOL WritePort(LPBYTE lpBuf, DWORD nByteToWrite);
	BOOL ReadPort(LPBYTE lpBuf, DWORD nByteToRead);
	RXTYPE GetAction(LPBYTE lpBuf);
	
	void WriteLogLevel(BYTE byID, int nLogLevel);

	BYTE CheckBCC ;
	WORD DataLen;

	int m_nLogLevel;
	BOOL m_bPollingCommand4CIM;
};

// For Thread Function
DWORD	ThreadWatchComm(CCommThread* pComm);

#endif

