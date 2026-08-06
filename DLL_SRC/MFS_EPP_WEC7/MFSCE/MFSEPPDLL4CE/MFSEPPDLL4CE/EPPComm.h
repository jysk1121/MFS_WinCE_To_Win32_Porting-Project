#pragma once
#include "SerialComm.h"
#include "Queue.h"
#include "Util.h"
#include "EPPDEF.h"

typedef int (*EventCallBackFunction)(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength);

class CEPPComm :public CSerialComm
{
public:
	CEPPComm(void);
	~CEPPComm(void);

	HANDLE	m_hCommWatchThread;			// 포트 감시 스레드 핸들
	CQueue m_QueueRead;

	CUtil	m_pUtil;
	BOOL	m_bRecvCtrl;

public:
	//Open, Close
//	int OpenDevice(const TCHAR* szPortNum, DWORD dwBaud);
	int OpenDevice(int nPortNum, DWORD dwBaud);
	int CloseDevice();

	//COM port data 해석
	void ReceiveComData();

	void ClearQueueKeyCode()			{	m_QueueKeyCode.Clear();		};
	void ClearQqueuRead()				{	m_QueueRead.Clear();		};
	void SetRecvKeyFlag(BYTE bFlag)		{	m_byRecvKeyFlag = bFlag;	};

	BYTE GetRecvKeyFlag()				{	return m_byRecvKeyFlag;		};
	WORD GetRecvCommand()				{	return m_wCommand;			};
	
	//Kal KeyMap에 정의된 KeyCode를 리턴하며 특수Key 처리를 한다.
	BYTE GetKeyCode();
	//EPP로부터 들어오는 KeyCode를 direct로 리턴한다.
	BYTE GetEPPKeyCodeDirect();

	//원하는 key code만 keybuffer에 저장함
	BOOL SetKeyMask(LPBYTE pMaskBuff, BYTE byMaskCnt);
	void ClearKeyMask()					{	memset(m_KeyMaskBuff, NULL, sizeof(m_KeyMaskBuff)); 
											m_byKeyMaskCount = 0;		};
	
	void GetKeyCheckValueBuff(LPBYTE pDest, int nLength);
	void GetExtraBuff(LPBYTE pDest, int nLength);
	BYTE GetStatusInfo() { return m_byStatusInfo;	};
	unsigned long GetStatusInfo2() { return m_ulStatusInfo;	};
//	int SendCommand(int nCommand, LPBYTE pData, DWORD dwLen, LPBYTE pResp, DWORD &dwRet, BOOL bCheckStatus=TRUE, BOOL bWaitResponse=TRUE);
//	int SendCommand(int nCommand, LPBYTE pData, DWORD dwLen);
	int ExecuteCommand(int nCommand, BYTE *pData, DWORD dwDataLen,
						BYTE *pResult, DWORD &dwResultLength,
						BOOL bCheckStatus, BOOL bWaitResponse);
	int RegistEventCallBackFunction(EventCallBackFunction pFunc);
	int GetCommandResultData(LPBYTE lpData, int nBuffLeng, int* nLength);
	BOOL CheckBCC(LPBYTE pData, int nLength);

	EventCallBackFunction	m_EventFunc;

private:
	CQueue m_QueueKeyCode;				//Key code 값이 들어 있는 queue.

	char m_szErrCode[10];
	BYTE m_PINBlockBuff[8];
	BYTE m_KeyCheckValueBuff[10]; //8->10으로 늘림
	BYTE m_MACResultBuff[8];
	BYTE m_ExtraBuff[MAX_RECV_BUFFER_SIZE];
	BYTE m_KeyMaskBuff[MAX_RECV_BUFFER_SIZE];
	
	BOOL m_bCRCError;
	BYTE m_byStatusInfo;
	unsigned long m_ulStatusInfo;
	WORD m_wCommand;
	BYTE m_byRecvKeyFlag;
	BYTE m_byPINCount;
	BYTE m_byMINPINLen;
	BYTE m_byKeyMaskCount;
	int m_nRecvFrameLength;

	void AnalysisKeyResponse(BYTE *pData, int nFrameLength);
	BOOL AnalysisCmdResponse(BYTE *pData, int nFrameLength);

	void InitializeExecuteCommand();
	void SetStatusInfo(BYTE *pData);
	void SetDataBuffer(BYTE *pData, int nLength);

	BOOL CheckCRC(LPBYTE pData, int nLength, short RecvCRC);
	short Calculate_CRC (BYTE ch, short remainder );

	BOOL	m_bStartKey;
	BOOL	m_bLookStartKey;

//	int TestFuncPtr(int i, int k);

//	FuncPtr m_func;
};

DWORD ThreadWatchComm(CEPPComm* pCmd);
