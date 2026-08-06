#pragma once


typedef int (*EventCallBackFunction)(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength);

class CCommLib
{
public:
	CCommLib(void);
	~CCommLib(void);

	int OpenDevice(int nPortNum, DWORD dwBaudRate);
	int CloseDevice();
	int ExecuteCommand(int nCommand, LPBYTE pData, DWORD dwLen, LPBYTE pResp, DWORD &dwRet, BOOL bCheckStatus=TRUE, BOOL bWaitResponse=TRUE);
	int RegistEventCallBackFunction(EventCallBackFunction pFunc);
//	int GetCommandResultData(LPBYTE lpData, int nBuffLeng, int* nLength);

private:
	HINSTANCE	m_hDLL;
};
