// MFS_MBU_WEC7.cpp : 해당 DLL의 초기화 루틴을 정의합니다.
//

#include "stdafx.h"
#include "MFS_MBU_WEC7.h"
#include "CommThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: 이 DLL이 MFC DLL에 대해 동적으로 링크되어 있는 경우
//		MFC로 호출되는 이 DLL에서 내보내지는 모든 함수의
//		시작 부분에 AFX_MANAGE_STATE 매크로가
//		들어 있어야 합니다.
//
//		예:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 일반적인 함수 본문은 여기에 옵니다.
//		}
//
//		이 매크로는 MFC로 호출하기 전에
//		각 함수에 반드시 들어 있어야 합니다.
//		즉, 매크로는 함수의 첫 번째 문이어야 하며 
//		개체 변수의 생성자가 MFC DLL로
//		호출할 수 있으므로 개체 변수가 선언되기 전에
//		나와야 합니다.
//
//		자세한 내용은
//		MFC Technical Note 33 및 58을 참조하십시오.
//


// CMFS_MBU_WEC7App

BEGIN_MESSAGE_MAP(CMFS_MBU_WEC7App, CWinApp)
END_MESSAGE_MAP()


// CMFS_MBU_WEC7App 생성

CMFS_MBU_WEC7App::CMFS_MBU_WEC7App()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CMFS_MBU_WEC7App 개체입니다.

CMFS_MBU_WEC7App theApp;

// 전역변수 선언
extern RX_PACKET *RecvData ;
extern CCommThread *g_pCPortCom;
extern BOOL m_bConnected;				// 
//extern OVERLAPPED m_osRead, m_osWrite;	// 포트 파일 Overlapped structure
//extern HANDLE m_hComm;					// 통신 포트 파일 핸들
//extern HANDLE m_hThreadWatchComm;		// Watch함수 Thread 핸들.
extern HANDLE WaitEventHandle;
//extern BYTE g_byRcvBuf[512];
//extern BYTE g_bySendBuf[MAX_SEND];
//extern BYTE bSensor[2];					// Save Sensor Status
//extern BYTE ReceivedSTX ;
//extern BYTE ReceivedETX ;
//extern BYTE ReceivedACK ;
//extern BYTE ReceivedNAK ; 
//extern BYTE ReceivedENQ ; 
//extern BYTE ReceivedDLE ; 
//extern BYTE ReceivedPacket ;


// CMFS_MBU_WEC7App 초기화

BOOL CMFS_MBU_WEC7App::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

extern "C" __declspec(dllexport) BOOL __stdcall MBU_Open(BYTE comport)
{
	//TCHAR * Comport;
	CString strComPort;

	if(m_bConnected)
		return FALSE;

	g_pCPortCom = new CCommThread;   

	switch(comport)
	{
	case 1: strComPort = _T("COM1:");
		break;
	case 2: strComPort = _T("COM2:");
		break;
	case 3: strComPort = _T("COM3:");
		break;
	case 4: strComPort = _T("COM4:");
		break;
	case 5: strComPort = _T("COM5:");
		break;
	case 6: strComPort = _T("COM6:");
		break;
	case 7: strComPort = _T("COM7:");
		break;
	case 8: strComPort = _T("COM8:");
		break;
	case 9: strComPort = _T("COM9:");
		break;

	default: strComPort = _T("COM3:");
		break;
	}

	if(!g_pCPortCom->bInitComPort(strComPort)) // 정상이 아니면
	{
		return FALSE;
	}

	//WaitEventHandle = CreateEvent(NULL,FALSE,FALSE,TEXT("SINGLE"));

	// Open 성공시 FW Version으로 분기처리하기 위해 내부적으로 Version Read를 수행함
	BOOL bRet = g_pCPortCom->Send_Packet(MBU_CMD_VERSION, ID_MBU,  NULL, 1);

	return TRUE;
}

extern "C" __declspec(dllexport)  BOOL __stdcall MBU_Close(void)
{
	if(!m_bConnected)
		return FALSE;

	g_pCPortCom->ClosePort();

	delete g_pCPortCom;

	CloseHandle(WaitEventHandle);

	return TRUE;
}

extern "C" __declspec(dllexport)  BOOL __stdcall MBU_IsOpen(void)
{
	return m_bConnected;
}

extern "C" __declspec(dllexport)  BOOL __stdcall MBU_GetVersion(LPSTR szVersion)
{
	if(!m_bConnected)
	{
		return FALSE;
	}

	BOOL bRet = g_pCPortCom->Send_Packet(MBU_CMD_VERSION, ID_MBU,  NULL, 1);

	if (bRet)
	{
		if(RecvData->byCommand == MBU_CMD_VERSION)
		{
			memcpy(szVersion, &RecvData->byDevID, 4);
			//ReceivedPacket = 0;
			return TRUE;
		}
	}

	return FALSE;
}

extern "C" __declspec(dllexport)  BOOL __stdcall MBU_Reset(void)
{
	if(!m_bConnected)
	{
		return FALSE;
	}

	BOOL bRet = g_pCPortCom->Send_Packet(MBU_CMD_RESET, ID_MBU,  NULL, 1);

	if (bRet)
	{
		if(RecvData->byCommand == MBU_CMD_RESET)
		{
			return TRUE;
		}
	}

	return FALSE;
}

extern "C" __declspec(dllexport)  BOOL __stdcall MBU_SendData(BYTE byID, BYTE* pData, int nLen)
{
	if(!m_bConnected)
	{
		return FALSE;
	}

	BOOL bRet = g_pCPortCom->Send_Packet(MBU_CMD_SEND, byID, pData, nLen + 2); // nLen + MUB cmd + Dev id

	if (bRet)
	{
		if(RecvData->byCommand == MBU_CMD_SEND)
		{
			return TRUE;
		}
	}

	return FALSE;
}

extern "C" __declspec(dllexport)  int __stdcall MBU_ReadData(BYTE byID, BYTE* byData)
{
	if(!m_bConnected)
	{
		return FALSE;
	}

	int nRetLen = g_pCPortCom->Recv_Packet(byID, byData);

	if (nRetLen > 0)
	{
		if (byID == RecvData->byDevID)
		{
			memcpy(byData, &RecvData->Data[0], nRetLen);
		}
		else
		{
			return FALSE;
		}
	}

	return nRetLen;
}

// 2020.11.13 interface 추가
extern "C" __declspec(dllexport)  BOOL __stdcall MBU_SetComportConfig(BYTE byID, BYTE* pbyCinfogData, int nLen)
{
	if(!m_bConnected)
	{
		return FALSE;
	}

	BOOL bRet = g_pCPortCom->Send_Packet(MBU_CMD_MULTI_COMPORT_SET, byID, pbyCinfogData, nLen + 2); // nLen + MUB cmd + Dev id

	if (bRet)
	{
		if(RecvData->byCommand == MBU_CMD_MULTI_COMPORT_SET)
		{
			return TRUE;
		}
	}

	return FALSE;
}

extern "C" __declspec(dllexport)  BOOL __stdcall MBU_ReadBufferReset(BYTE byID)
{
	if(!m_bConnected)
	{
		return FALSE;
	}

	BOOL bRet = g_pCPortCom->Recv_Buffer_Reset(byID);

	if (bRet)
	{
		if(byID == RecvData->byDevID)
		{
			return TRUE;
		}
	}

	return FALSE;
}

extern "C" __declspec(dllexport)  void __stdcall MBU_WriteLogLevel(BYTE byID, int nLogLevel)
{
	g_pCPortCom->WriteLogLevel(byID, nLogLevel);
}
