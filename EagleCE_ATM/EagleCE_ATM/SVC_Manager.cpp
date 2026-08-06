#include "stdafx.h"
#include "SVC_Manager.h"

#include "EagleSVCLib.h"
#include "EagleTritonMsg.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/IniFile.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"
#include "../../EagleCE_Device/EagleCE_Device/DevSimPinPadDlg.h"



#ifndef _WIN32_WCE
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#endif	// !_WIN32_WCE


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifndef _WIN32_WCE
namespace
{
	//
	// Win32 desktop network information.
	//
	// CUtil's network helpers read the WinCE "Comm\...\Parms\TcpIp" registry
	// layout, which does not exist here, and nothing in this application owns
	// the desktop NIC - Windows does. So the Win32 build only *reads* what
	// Windows already has, which keeps the addresses the terminal reports (and
	// the ones Check_System_Error() validates) equal to the real ones.
	//

	BOOL GetLocalAdapterInfo(CString& strIP, CString& strSubnet, CString& strGateway, BOOL& bDHCP)
	{
		ULONG ulSize = 0;

		if (::GetAdaptersInfo(NULL, &ulSize) != ERROR_BUFFER_OVERFLOW)
			return FALSE;

		BYTE*				pBuffer   = new BYTE[ulSize];
		IP_ADAPTER_INFO*	pAdapters = (IP_ADAPTER_INFO*)pBuffer;
		BOOL				bFound    = FALSE;

		if (::GetAdaptersInfo(pAdapters, &ulSize) == NO_ERROR)
		{
			for (IP_ADAPTER_INFO* pAdapter = pAdapters; pAdapter != NULL; pAdapter = pAdapter->Next)
			{
				// Skip loopback and anything that has not been given an address:
				// a disconnected adapter reports 0.0.0.0 and would look like a
				// perfectly valid terminal IP further up.
				if (pAdapter->Type == MIB_IF_TYPE_LOOPBACK)
					continue;

				CString strAddress = CString(pAdapter->IpAddressList.IpAddress.String);

				if (strAddress.IsEmpty() || strAddress == _T("0.0.0.0"))
					continue;

				strIP      = strAddress;
				strSubnet  = CString(pAdapter->IpAddressList.IpMask.String);
				strGateway = CString(pAdapter->GatewayList.IpAddress.String);
				bDHCP      = (pAdapter->DhcpEnabled != 0);
				bFound     = TRUE;
				break;
			}
		}

		delete [] pBuffer;

		return bFound;
	}


	BOOL GetLocalDNS(CString& strDNS)
	{
		ULONG ulSize = 0;

		if (::GetNetworkParams(NULL, &ulSize) != ERROR_BUFFER_OVERFLOW)
			return FALSE;

		BYTE*		pBuffer = new BYTE[ulSize];
		FIXED_INFO*	pInfo   = (FIXED_INFO*)pBuffer;
		BOOL		bFound  = FALSE;

		if (::GetNetworkParams(pInfo, &ulSize) == NO_ERROR)
		{
			CString strAddress = CString(pInfo->DnsServerList.IpAddress.String);

			if (!strAddress.IsEmpty() && strAddress != _T("0.0.0.0"))
			{
				strDNS = strAddress;
				bFound = TRUE;
			}
		}

		delete [] pBuffer;

		return bFound;
	}
}
#endif	// !_WIN32_WCE


BEGIN_MESSAGE_MAP(CSVC_Manager, CWnd)
	ON_MESSAGE(WM_PRT_NOTIFY, &CSVC_Manager::OnPrtNotify)
	ON_MESSAGE(WM_DIO_NOTIFY, &CSVC_Manager::OnDioNotify)
	//ON_MESSAGE(WM_EPP_NOTI_NOMALKEY, &CSVC_Manager::OnEppNotify)
	ON_MESSAGE(WM_USBMEM_NOTIFY, &CSVC_Manager::OnUsbMemNotify)
	ON_MESSAGE(WM_AMS_NOTIFY, &CSVC_Manager::OnAMSNotify)
END_MESSAGE_MAP()


// 인스턴스
CSVC_Manager* CSVC_Manager::m_pInstance = NULL;

extern HWND			g_hWndDisplay;			// V1.0.2.4 2018.07.19 - Clinet Wnd Handle

// for Cryptera
EventCallBackFunction	m_callback;
static int EventControlFunc(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength);
//

/** **********************************************************
*	@brief		인스턴스 생성
*	@retval		없음
************************************************************/
void CSVC_Manager::CreateInstance()
{
	m_pInstance = new CSVC_Manager();

}


/** **********************************************************
*	@brief		인스턴스 취득
*	@retval		없음
************************************************************/
CSVC_Manager* CSVC_Manager::GetInstance()
{
	return m_pInstance;
}


/** **********************************************************
*	@brief		인스턴스 해제
*	@retval		없음
************************************************************/
void CSVC_Manager::ReleaseInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
	}
}


/** **********************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CSVC_Manager::CSVC_Manager()
	: CWnd()
{
	// 데이터 관리자 생성
	CEagleDataManager::CreateInstance();

	// 장치 관리자 인스턴스 생성
	CDEV_Manager::CreateInstance();

	// 통지 윈도우
	m_pNotifyWnd = NULL;

	// 서비스 스레드 핸들
	m_hServiceThread = NULL;

	m_hSWMonitorThread = NULL;

	m_hATMStatus_MonitorThread = NULL;

	// 서비스 종료
	m_bStopService = FALSE;

	// 서비스 모드
	m_nServiceMode = 0;

	// 계원용 모드
	m_nOperatorMode = 0;


	// 날짜 변경 후 처음
	m_First_After_Day_Changed = TRUE;

	// 현재 날짜 시간
	GetLocalTime(&m_CurrentDatetime);


	// CDM - Connect
	m_CDM_Connect = FALSE;

	m_CDM_CommError = FALSE;

	// CDR - Connect
	m_CDR_Connect = FALSE;

	// DIO - Connect
	m_DIO_Connect = FALSE;

	// EJL - Connect
	m_EJL_Connect = FALSE;

	// EPP - Connect
	m_EPP_Connect = FALSE;

	// HOST - Connect
	m_HOST_Connect = FALSE;

	// PRT - Connect
	m_PRT_Connect = FALSE;

	// SHT - Connect
	//m_SHT_Connect = FALSE;


	// DIO - Front Door 열림
#if (EMULATION_SIU_DEVICE)
	m_DIO_Front_Door_Open = TRUE;
#else
	m_DIO_Front_Door_Open = FALSE;
#endif

	// DIO - Safe Door 열림
	m_DIO_Safe_Door_Open = FALSE;

	// DIO - Upper Frame 열림
	m_DIO_Upper_Frame_Open = FALSE;

	// DIO - Ear Jack 삽입
	m_DIO_Ear_Jack_Insert = FALSE;

	m_bDetectedSensorPoll_PowerOn = FALSE;


	// 명세서 출력 화면 사용 여부 설정
	m_bUseReceipt = FALSE;

	// EPP Key Timeout Flag
	m_bIsEPPTimeout = FALSE;

	m_bInitReset = FALSE;

	m_bOutOfService = FALSE;

	m_nActiveCmd = 0;

	m_nInjectKeySequence = 0;

	m_bAMSStatusSend = FALSE;

	m_bAMSUploadJNLSend = FALSE;

	m_bNotifyStatusToAMS_PowerOn = FALSE;

	m_bExecDownloadKeysToHost_PowerOn = FALSE;

	m_strSelectTouch = _T("SELECT_TOUCH");

	m_nEPP_Enable_Recovery = 0;
}


/** **********************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CSVC_Manager::~CSVC_Manager()
{
	// 인스턴스 해제
	CDEV_Manager::ReleaseInstance();
}


/** **********************************************************
*	@brief		초기화
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Initialize()
{
	LOG(Info, _T("Initialize Start"));


	// 메시지 윈도우 생성
	CreateEx(0, AfxRegisterWndClass(NULL), _T("CSVC_Manager"), 0, 0, 0, 0, 0, NULL, 0);

	// 통지 윈도우 설정
	CDEV_Manager::GetInstance()->SetNotifyWnd(this);

	// Software Monitoring Thread Start
	m_hSWMonitorThread = AfxBeginThread((AFX_THREADPROC)SWMonitoring, this);

	// ATM Status Monitoring Thread Start
	m_hATMStatus_MonitorThread = AfxBeginThread((AFX_THREADPROC)ATMAtstus_Monitoring, this);

	// 데이터 관리자 초기화
	if (FALSE == CEagleDataManager::GetInstance()->Initialize())
	{
		LOG(Error, _T("Initialize Error - Failed to initialize in data manager"));

		return FALSE;
	}

	LOG(Info, _T("EagleData Initialize - End"));

	// 장치 관리자 초기화
	if (FALSE == CDEV_Manager::GetInstance()->Initialize())
	{
		LOG(Error, _T("Initialize Error"));
	}

	LOG(Info, _T("EagleDevice Initialize - End"));

	CString strTemp;
	// 최초 Loading시의 CBX Info를 Log에 저장하도록 로직 추가
	LOG(Info, _T("CFG - CBX COUNT : %s"), CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Count);

	for(int i=0; i<4; i++)
	{
		LOG(Info, _T("CFG - CBX(%d) REMAIN COUNT : %s"), i+1, CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Item_Count[i]);
		LOG(Info, _T("CFG - CBX(%d) DENOMINATION : %s"), i+1, CEagleDataManager::GetInstance()->m_Config.m_CbxInfo.strCBX_Denomination[i]);
	}

	// EPP USB Driver Registry 확인 (USB Buffer Value Check)
	BOOL bRegUpdate = FALSE;
	int nValue = 0;
	nValue = CUtil::Int_GetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_BUF_IN_REG_NAME);

	strTemp.Format(_T("CRYPTERA EPP USB Drvier - Buffer In Registry Value - (0x%x)"), nValue);
	LOG(Info, strTemp);

	if (nValue == 0)
		bRegUpdate = TRUE;

	if (bRegUpdate == TRUE)
	{
		// Registry값이 적용되지 않았으므로 Registry 설정 후 해당 Registry값을 USB Driver가 적용하도록 System Reboot 수행
		CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_DRV_COMPORT_NAME, CRYPTERA_USB_DRV_COMPORT_VALUE);			// Comport Index Fix
		CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_BUF_IN_REG_NAME, CRYPTERA_USB_BUF_IN_REG_VALUE);				// Default is 8
		CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_BUF_OUT_REG_NAME, CRYPTERA_USB_BUF_OUT_REG_VALUE);			// Default is 8
		CUtil::Int_SetRegistry(CRYPTERA_USB_DRV_REG_PATH, CRYPTERA_USB_CTRL_IN_BUF_REG_NAME, CRYPTERA_USB_CTRL_IN_BUF_REG_VALUE);	// Default is 4

		RegFlushKey(HKEY_LOCAL_MACHINE);
		CUtil::Sleep_Wait(3000);

		strTemp.Format(_T("CRYPTERA EPP USB Driver - Set Registry Value"));
		LOG(Info, strTemp);

		// 현재 Cable 변경이 발생하지 않으므로 일단 재부팅 로직은 임시 주석 처리함 (OS발행시 Registry 적용 검토 의뢰 필요)
		//CDEV_Manager::GetInstance()->System_Reboot();
		//return FALSE;
	}

	// Config 파일 관련 설정 부분은 모두 이곳으로 통일하도록 함
	// Speaker Volume 설정
	CDEV_Manager::GetInstance()->SetVolume(CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strSound_Volume));

	// Set Network
	CDEV_Manager::GetInstance()->Set_Network_Config(FALSE);

#ifdef _WIN32_WCE
	// 실제 File에 있는 정보와 Registry 정보 동기화 처리 추가
	if (CUtil::GetDHCP() == TRUE)
	{
		LOG(Info, _T("Current Network DHCP Mode"));

		CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP = S_ENABLE;

		CUtil::GetDHCP_IP(strTemp);
		LOG(Info, _T("Current Network Information - DHCP IP  : %s"), strTemp);
		CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip = strTemp;	// DHCP IP정보를 File에 저장
	
		CUtil::GetDHCP_SubnetMask(strTemp);
		LOG(Info, _T("Current Network Information - DHCP Subnet  : %s"), strTemp);
		CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet = strTemp;

		CUtil::GetDHCP_GateWay(strTemp);
		CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway = strTemp;
		LOG(Info, _T("Current Network Information - DHCP GateWay  : %s"), strTemp);

		CUtil::GetDHCP_DNS(strTemp);
		CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS = strTemp;
		LOG(Info, _T("Current Network Information - DHCP DNS  : %s"), strTemp);
	}
	else
	{
		LOG(Info, _T("Current Network Static IP Mode"));

		CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP = S_DISABLE;

		CUtil::GetStaticIPAddress(strTemp);
		CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip = strTemp;
		LOG(Info, _T("Current Network Information - Static IP  : %s"), strTemp);

		CUtil::GetStaticSubnetMask(strTemp);
		CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet = strTemp;
		LOG(Info, _T("Current Network Information - Static SubNet  : %s"), strTemp);

		CUtil::GetStaticGateway(strTemp);
		CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway = strTemp;
		LOG(Info, _T("Current Network Information - Static GateWay  : %s"), strTemp);

		CUtil::GetStaticDNS(strTemp);
		CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS = strTemp;
		LOG(Info, _T("Current Network Information - Static DNS  : %s"), strTemp);
	}
#else
	// Win32 desktop: take the live addresses from the adapter Windows is using.
	// Without this the terminal would keep reporting whatever SYSTEM.CFG last
	// held - typically the factory default - even though the socket connects
	// from the machine's real address.
	{
		CString	strIP		= _T("");
		CString	strSubnet	= _T("");
		CString	strGateway	= _T("");
		CString	strDNS		= _T("");
		BOOL	bDHCP		= FALSE;

		if (GetLocalAdapterInfo(strIP, strSubnet, strGateway, bDHCP) == TRUE)
		{
			LOG(Info, bDHCP ? _T("Current Network DHCP Mode") : _T("Current Network Static IP Mode"));

			CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP = (bDHCP == TRUE) ? S_ENABLE : S_DISABLE;
			CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip = strIP;
			CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet = strSubnet;
			CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway = strGateway;

			LOG(Info, _T("Current Network Information - IP  : %s"), strIP);
			LOG(Info, _T("Current Network Information - Subnet  : %s"), strSubnet);
			LOG(Info, _T("Current Network Information - GateWay  : %s"), strGateway);

			if (GetLocalDNS(strDNS) == TRUE)
			{
				CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS = strDNS;
				LOG(Info, _T("Current Network Information - DNS  : %s"), strDNS);
			}
		}
		else
		{
			// Nothing is reachable without an address, so say so loudly rather
			// than letting the terminal come up and fail at connect time.
			LOG(Error, _T("Current Network Information - no connected adapter found"));
		}
	}
#endif

	CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("useDHCP"), CEagleDataManager::GetInstance()->m_Config.m_Host.strUseDHCP);
	CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("terminal_ip"), CEagleDataManager::GetInstance()->m_Config.m_Host.strTerminalip);
	CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("subnet"), CEagleDataManager::GetInstance()->m_Config.m_Host.strSubnet);
	CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("gateway"), CEagleDataManager::GetInstance()->m_Config.m_Host.strGateway);
	CEagleDataManager::GetInstance()->m_Config.SaveHostData(_T("DNS"), CEagleDataManager::GetInstance()->m_Config.m_Host.strDNS);

	// LOG FILE 관리 버전 Check
	LOG(Info, _T("Check Log File Managemet Version : %s"), CEagleDataManager::GetInstance()->m_Config.m_System.strLogMgr_Version);

	if (CEagleDataManager::GetInstance()->m_Config.m_System.strLogMgr_Version != LOG_MGR_VERSION)
	{
		CString strLogPath;

		strLogPath.Format(_T("%s%s"), CUtil::GetAppPath(), PATH_LOG);

		// Backup된 Log File을 모두 제거
		CUtil::DeleteFileInDirectory(strLogPath, _T("*.*"));

		CEagleDataManager::GetInstance()->m_Config.m_System.strLogMgr_Version = LOG_MGR_VERSION;
		CEagleDataManager::GetInstance()->m_Config.m_System.strBackUpLogIndex = _T("0");
		CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("log_mgr_version"), CEagleDataManager::GetInstance()->m_Config.m_System.strLogMgr_Version);
		CEagleDataManager::GetInstance()->m_Config.SaveSystemData(_T("backup_log_index"), CEagleDataManager::GetInstance()->m_Config.m_System.strBackUpLogIndex);

		LOG(Info, _T(" Write Log File Managemet Version : %s"), CEagleDataManager::GetInstance()->m_Config.m_System.strLogMgr_Version);
	}

	// V1.0.2.4 2018.07.04
	if(CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime.IsEmpty())
	{
		CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime = _T("0400");

		// Save File
		CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("reboot_newtime"), CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime);
	}

	// 서비스 시작
	m_hServiceThread = AfxBeginThread((AFX_THREADPROC)RunService, this);

	LOG(Info, _T("Initialize End"));


	return TRUE;
}


/** **********************************************************
*	@brief		종결화
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Finalize()
{
	LOG(Info, _T("Finalize Start"));


	// 데이터 관리자 초기화
	if (FALSE == CEagleDataManager::GetInstance()->Finalize())
	{
		LOG(Error, _T("Finalize Error - Failed to finalize in data manager"));

		return FALSE;
	}


	// 장치 관리자 종결화 - 오래 걸려서 안하는걸로 처리
	//if (FALSE == CDEV_Manager::GetInstance()->Finalize())
	//{
	//	LOG(Error, _T("Finalize Error - Failed to load config file"));
	//
	//	return FALSE;
	//}

	// 서비스 종료
	StopService();

	if (m_hATMStatus_MonitorThread)
	{
		// ATM Status Monitor 종료
		DWORD dwExitCode = STILL_ACTIVE;
		int nCount = 0;

		while(dwExitCode == STILL_ACTIVE)
		{
			GetExitCodeThread(m_hATMStatus_MonitorThread, &dwExitCode);

			if (nCount > 500)	// 5초 대기
				break;

			nCount++;

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
		}

		CloseHandle(m_hATMStatus_MonitorThread);
		m_hATMStatus_MonitorThread = NULL;
	}


	if (m_hSWMonitorThread)
	{
		// Software Monitor 종료
		DWORD dwExitCode = STILL_ACTIVE;
		int nCount = 0;

		while(dwExitCode == STILL_ACTIVE)
		{
			GetExitCodeThread(m_hSWMonitorThread, &dwExitCode);

			if (nCount > 500)	// 5초 대기
				break;

			nCount++;

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
		}

		CloseHandle(m_hSWMonitorThread);
		m_hSWMonitorThread = NULL;
	}

	// 서비스 종료 대기
	if (m_hServiceThread)
	{
		//WaitForSingleObject(m_hServiceThread, EAGLE_SLEEP_INTERVAL_5MIN);
		DWORD dwExitCode = STILL_ACTIVE;
		int nCount = 0;

		while(dwExitCode == STILL_ACTIVE)
		{
			GetExitCodeThread(m_hServiceThread, &dwExitCode);

			if (nCount > 500)	// 5초 대기
				break;

			nCount++;

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
		}

		CloseHandle(m_hServiceThread);
		m_hServiceThread = NULL;
	}


	LOG(Info, _T("Finalize End"));


	// 메시지 윈도우 해제
	return DestroyWindow();
}


/** **********************************************************
*	@brief		통지 윈도우 설정
*	@retval		없음
************************************************************/
void CSVC_Manager::SetNotifyWnd(CWnd* pNotifyWnd)
{
	m_pNotifyWnd = pNotifyWnd;
}


/** **********************************************************
*	@brief		서비스 모드 취득 (IN SERVICE / OUT OF SERVICE)
*	@retval		없음
************************************************************/
int CSVC_Manager::GetServiceMode(void)
{
	return m_nServiceMode;
}


/** **********************************************************
*	@brief		서비스 모드 설정 (IN SERVICE / OUT OF SERVICE)
*	@retval		없음
************************************************************/
void CSVC_Manager::SetServiceMode(int nServiceMode)
{
	// 이전 서비스 모드 상태를 저장한다.
	CDEV_Manager::GetInstance()->m_nPrevServiceMode = m_nServiceMode;

	m_nServiceMode = nServiceMode;

	CDEV_Manager::GetInstance()->CheckMemoryStatus();

	CString strCommand, strTextValue;
	// Logging Service Mode
	switch (nServiceMode)
	{
	case EAGLE_ATM_SVC_IN_SERVICE:
		LOG(Info, _T("Change Service Mode - In Service"));
		break;

	case EAGLE_ATM_SVC_OUT_OF_SERVICE:
		LOG(Info, _T("Change Service Mode - Out Of Service"));

		// Out of Service에서는 화면 Display 수행
		// Screen Data Make
		CSCR_Manager::GetInstance()->SetVariable(_T("PrevSetScreenNo"), CLIENT_SCREEN_OUTOFSERVICE);

		// Added Data
		strCommand.Format(_T("%s%d"), SCR_TEXTVALUE_CMD, 1);
		strTextValue.Format(_T("ERROR = %s"), CEagleSVCLib::GetInstance()->Get_ErrorCode());
		CSCR_Manager::GetInstance()->SetVariable(strCommand, strTextValue);

		// Display Screen
		CSCR_Manager::GetInstance()->SetVariable(_T("APCenterMovie"), CLIENT_SCREEN_OUTOFSERVICE);
		break;

	case EAGLE_ATM_SVC_SUPERVISOR_SERVICE:
		LOG(Info, _T("Change Service Mode - In Supervisor"));
		Operator_Initialize();
		break;

	case EAGLE_ATM_SVC_REMOTE_ACCESS_OF_SERVICE:
		LOG(Info, _T("Change Service Mode - Remote Access"));
		break;

	}
}


/** **********************************************************
*	@brief		계원 모드 취득 (ADMINISTRATOR / MASTER / REPLENISHER)
*	@retval		없음
************************************************************/
int CSVC_Manager::GetOperatorMode(void)
{
	return m_nOperatorMode;
}


/** **********************************************************
*	@brief		계원 모드 설정 (ADMINISTRATOR / MASTER / REPLENISHER)
*	@retval		없음
************************************************************/
void CSVC_Manager::SetOperatorMode(int nOperatorMode)
{
	m_nOperatorMode = nOperatorMode;

}


/** **********************************************************
*	@brief		서비스 시작
*	@retval		없음
************************************************************/
UINT CSVC_Manager::RunService(LPVOID pParam)
{
	CSVC_Manager* pSVC_Manager = (CSVC_Manager*)pParam;

	pSVC_Manager->RunService();

	return 0;
}


/** **********************************************************
*	@brief		Software Monitoring Polling 시작
*	@retval		없음
************************************************************/
UINT CSVC_Manager::SWMonitoring(LPVOID pParam)
{
	CSVC_Manager* pSVC_Manager = (CSVC_Manager*)pParam;

	// SW Monitor 기동
	// Execute SW_Monitor.exe
	{
		STARTUPINFO si;
		ZeroMemory (&si, sizeof(si));
		si.cb = sizeof (si);
		si.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi;
		ZeroMemory (&pi, sizeof(pi));

		CString strPath;
		strPath.Format(_T("%s%s"), CUtil::GetAppPath(), EXE_SW_MONITOR);

		CreateProcess(strPath, NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);
	}

	::Sleep(3000);

	// 3초 후 해당 HWND를 찾는다.
	HWND m_hSWMonitor = NULL;
	CString strTemp;

	char chPollValue = 0x01;

	COPYDATASTRUCT cds;

	cds.cbData = 1;
	cds.dwData = 1;
	cds.lpData = &chPollValue;

	m_hSWMonitor = ::FindWindow(_T("__MFS_SW_MONITOR_WND__"), NULL);

	if (m_hSWMonitor == NULL)
	{
		LOG(Error, _T("SW Monitor execute is failed"));
		return 0;
	}

	LOG(Info, _T("SW Monitor is executed"));

	while(TRUE)
	{
		if (pSVC_Manager->m_bStopService == TRUE)
			break;

		if ( m_hSWMonitor != NULL)
		{
			::SendMessage(m_hSWMonitor, WM_COPYDATA, 0, (LPARAM)&cds);
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_15SEC);	// 15초마다 한번씩 Polling 시도
	}

	return 0;
}



/** **********************************************************
*	@brief		ATM Status Monitoring 시작
*	@retval		없음
************************************************************/
UINT CSVC_Manager::ATMAtstus_Monitoring(LPVOID pParam)
{
	CSVC_Manager* pSVC_Manager = (CSVC_Manager*)pParam;

	// 3초 후 해당 HWND를 찾는다.
	CString strTemp;
	SYSTEMTIME	st;
	CIniFile iniFile(ATM_STATUS_MONITORING_FILE);
	//int nServiceMode = 0;

	// ATM 상태를 Check하여 In Service인 경우 별도 File로 상태를 저장 (중간에 Power를 Off하는 경우에 대해 In Service 가동률 Check를 위해)
	while(TRUE)
	{
		if (pSVC_Manager->m_bStopService == TRUE)
			break;

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1MIN);	// 1분마다 한번씩 Check

		// Format:YYYYMMDDHHMMSS
		GetLocalTime(&st);
		strTemp.Format(_T("%04d%02d%02d%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		iniFile.WriteString(_T("ATM_STATUS_INFO"), _T("TIME"), strTemp);
	}

	return 0;
}



int	nResult = 0;
/** **********************************************************
*	@brief		서비스 실행
*	@retval		없음
************************************************************/
void CSVC_Manager::RunService(void)
{
	// 연결
	Proc_Connect();

	// initialize devices
	Proc_Device_Initialize();

	CEagleSVCLib::GetInstance()->Set_Reboot_Time();		// 기기 Power On후 Next Reboot 시간을 Setting

	CEagleSVCLib::GetInstance()->Set_Heartbeat_Time();	// 최초 한번 등록

	// 서비스가 중지 될 때까지
	while (!m_bStopService)
	{
		// 서비스 체크
		nResult = CheckService();

		// Power On시 Configuration 수행
		if (m_bExecDownloadKeysToHost_PowerOn == FALSE)
		{
			// HOST IP 또는 Phone Number가 입력이 된 경우에만 수행하도록 로직 보완 -> 기기가 정상인 상태에서만 Configuration을 수행하도록 로직 수정
			// Out of Service인 경우 Host 통신 Delay를 없애기 위함
			//if (CEagleDataManager::GetInstance()->m_Config.m_Host.strNetworkType == _T("TCP/IP"))
			//	strHostAddressInfo = CEagleDataManager::GetInstance()->m_Config.m_Host.strHostip;
			//else
			//	strHostAddressInfo = CEagleDataManager::GetInstance()->m_Config.m_Host.strPrimary_phone_number;

			//strHostAddressInfo.TrimLeft();

			//if (strHostAddressInfo.IsEmpty() == FALSE)	// 값이 설정이 되었을 경우에만 수행
			if (GetServiceMode() == EAGLE_ATM_SVC_IN_SERVICE)
			{
				// AMS를 Close 처리한 후에 Host를 송신하도록 로직 보완 (Sync 처리)
				CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Sync_ConnectClose();

				// Option에 따라 처리하도록 Option 추가 필요
				m_bExecDownloadKeysToHost_PowerOn = TRUE;

				Client_Screen_Processing();

				CEagleTritonMsg::GetInstance()->Triton_MakeHostMsg(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG);

				if (CEagleSVCLib::GetInstance()->SendRecvHost(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG) == SUCCESS)
				{
					LOG(Info, _T("[Power On] Execute Download Keys - Success"));
				}
				else
				{
					LOG(Info, _T("[Power On] Execute Download Keys - Failed"));
					
					if (CEagleSVCLib::GetInstance()->Get_ErrorCode().Left(2) == _T("Z3"))
						CEagleSVCLib::GetInstance()->Clear_Error();
				}

				// Out of Service인 경우 화면 갱신을 위해 nResult값을 강제로 갱신
				nResult = 1;
			}
		}

		if ((nResult == 1) && (EAGLE_ATM_SVC_OUT_OF_SERVICE == GetServiceMode()))
		{
			// Display Out of Service Screen
			CrypteraEPP_EnterClearTextMode();
			SetServiceMode(EAGLE_ATM_SVC_OUT_OF_SERVICE);
		}

		// 커맨드 있음
		if (m_CmdQueue.ElemNum())
		{
			// 커맨드 취득
			BYTE byCmd = m_CmdQueue.Dequeue();

			if (EAGLE_ATM_SVC_IN_SERVICE == GetServiceMode())
			{
				// In Service
				RunService_Client(byCmd);
			}
			else if (EAGLE_ATM_SVC_SUPERVISOR_SERVICE == GetServiceMode())
			{
				// Operator
				RunService_Operator(byCmd);
			}
			else if (EAGLE_ATM_SVC_OUT_OF_SERVICE == GetServiceMode())
			{
				// Out Of Service
				// noting
			}
			else if (EAGLE_ATM_SVC_REMOTE_ACCESS_OF_SERVICE == GetServiceMode())
			{
				// Remote Access
				LOG(Info, _T("RunService - Change Remote Access Service (%d)"), byCmd);

				// AMS 연동 로직 추가
				RunService_RemoteAccess(byCmd);
			}
		}

		// 대기
		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}


	// 해제
	LOG(Info, _T("RunService - Disconnect - Start"));

	Proc_Disconnect();

	LOG(Info, _T("RunService - Disconnect - End"));
}



/** **********************************************************
*	@brief		서비스 실행 - Remote Access 모드
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::RunService_RemoteAccess(BYTE byCmd)
{
	LOG(Info, _T(" Start Remote Access progress"));

	// Display Screen
	AMS_Screen_RemoteProgress();

	// Connect Close 후, 수행하도록 로직 수정
	if (byCmd == EAGLE_CMD_AMS_STATUS_SEND)
	{
		LOG(Info, _T("AMS Close Start - Sync"));
		CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Sync_ConnectClose();
		LOG(Info, _T("AMS Close End - Sync"));
	}

	if (EAGLE_CMD_AMS_ACCEPT_OK == byCmd)
	{
		CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Sync_Communication(AMS_LISTEN);
	}
	else
	{
		CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Sync_Communication(AMS_CONNECT);
	}

	return TRUE;
}


/** **********************************************************
*	@brief		서비스 체크
*	@retval		없음
************************************************************/
int CSVC_Manager::CheckService(void)
{
	// Key가 계속 눌릴 경우 Timeout 없이 EPP가 동작되지 않는 현상 발생하여 Status가 Enable인 경우 명령 후 1분이 지난 경우 다시 명령을 하도록 로직 보완
	if (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEntryKeyStatus == ENTRY_KEY_STATUS_ENABLE)		// Enable이 발행한 상황에서만 Check
	{
		int nCurTickCnt = (int)GetTickCount();
		if (nCurTickCnt > m_nEPP_Enable_Recovery)
		{
			m_bIsEPPTimeout = TRUE;
			//LOG(Info, _T("60sec timeout detected - try enable cmd execute"));
		}
	}

	if (m_bIsEPPTimeout == TRUE)
	{
		m_bIsEPPTimeout = FALSE;

		if (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEntryKeyStatus == ENTRY_KEY_STATUS_ENABLE)		// Key가 Enable일 경우에만 Key Entry 재발행
		{
			// 재발행을 위해 상태 변경
			CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEntryKeyStatus = ENTRY_KEY_STATUS_NONE;

			// Retry 3회 수행
			int nRetryTime = 0;

			while(nRetryTime < 3)
			{
				if (CrypteraEPP_EnterClearTextMode() == FALSE)
				{
					LOG(Info, _T("CrypteraEPP_EnterClearTextMode Error - Retry Execute(%d)"), nRetryTime);
					nRetryTime++;
				}
				else
				{
					break;
				}

				CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
			}
		}
	}

	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsTransaction == TRUE)	// 거래 시작시에는 다른 장애 Check하지 않음
		return 0;

	// 계원 모드 사용 중
	if (m_nOperatorMode)
	{
		SetOperatorService();
		return 0;
	}

	// Reboot Time Check (거래 또는 Supervisor에서는 수행하지 않음)
	{
		COleDateTime	RebootTime, CurTime;
		int				nScheduleRebootYear, nScheduleRebootMonth, nScheduleRebootDay, nScheduleRebootHour, nScheduleRebootMin;

		SYSTEMTIME	st;
		GetLocalTime(&st);

		CurTime.SetDateTime((int)st.wYear, (int)st.wMonth, (int)st.wDay, (int)st.wHour, (int)st.wMinute, 0);

		nScheduleRebootYear = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootYear);
		nScheduleRebootMonth = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootMonth);
		nScheduleRebootDay = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootDay);
		nScheduleRebootHour = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime.Left(2));
		nScheduleRebootMin = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strRebootTime.Right(2));
		///////////////////////////////////

		RebootTime.SetDateTime(nScheduleRebootYear, nScheduleRebootMonth, nScheduleRebootDay, nScheduleRebootHour, nScheduleRebootMin, 0);

		if (CurTime >= RebootTime)
		{
			// Reboot 수행
			LOG(Info, _T("Reboot - Current Time : %04d-%02d-%02d %02d:%02d:%02d"), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
			LOG(Info, _T("Reboot - Reboot Time : %04d-%02d-%02d %02d:%02d:%02d"), RebootTime.GetYear(), RebootTime.GetMonth(), RebootTime.GetDay(), RebootTime.GetHour(), RebootTime.GetMinute(), RebootTime.GetSecond());
			LOG(Info, _T("Reboot - System Time : %04d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

			CString strTemp;
			strTemp = _T("SYSTEM REBOOT");
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(reboot, strTemp);

			// System Reboot 수행
			CDEV_Manager::GetInstance()->System_Reboot();

			return 1;
		}
	}	


#if !(NETWORK_OFFLINE_MODE)
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg == S_ENABLE)
	{
		// 최초 Power ON시에 Download Config 수행을 위해 수행 완료될 때까지 Listening 하지 않도록 보완 처리
		//if (m_bExecDownloadKeysToHost_PowerOn == TRUE)
		{
			if (CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Sync_ConnectOpen(_T("LISTEN"), _T(""), AMS_LISTEN_PORT) == AMS_OK)
			{
				// AMS Connect Check
				if (CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Async_ConnectCheck() == TRUE)
				{
					LOG(Info, _T("AMS_Async_ConnectCheck is TRUE"));

					SetRemoteAccessService();

					AMS_AcceptOK();

					LOG(Info, _T("Call AMS Accept OK"));

					return 0;
				}
			}
		}
	}

	// 2. Health Check
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strHeartbeat_message == S_ENABLE)
	{
		if (m_bExecDownloadKeysToHost_PowerOn == TRUE)	// Power On시 Configuration을 수행한 후에 진행을 하도록 로직 추가
		{
			if ((m_bOutOfService == TRUE) && (GetServiceMode() == EAGLE_ATM_SVC_IN_SERVICE))
			{
				// Health Check 송신 후 In-Service 처리
				m_bOutOfService = FALSE;

				CEagleSVCLib::GetInstance()->Send_HeartbeatOrScheduleDayCloseMsg(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG);

				return 1;
			}
			else if ((m_bOutOfService == FALSE) && (GetServiceMode() == EAGLE_ATM_SVC_OUT_OF_SERVICE))
			{
				m_bOutOfService = TRUE;

				CEagleSVCLib::GetInstance()->Send_HeartbeatOrScheduleDayCloseMsg(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG);

				return 1;
			}
			else if (CEagleSVCLib::GetInstance()->Get_Heartbeat_GapTime() >= CEagleSVCLib::GetInstance()->m_DelayTime)
			{
				CEagleSVCLib::GetInstance()->Send_HeartbeatOrScheduleDayCloseMsg(EAGLE_TRAN_CODE_DOWNLOAD_CONFIG);

				return 1;
			}
		}
	}

	// AMS Status Send
	if (CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg == S_ENABLE)
	{
		if(CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_Status_en_flg == S_ENABLE)
		{
			if (m_bNotifyStatusToAMS_PowerOn == FALSE)
			{
				// Service Mode가 Set이 된 후에 송신함
				if ((GetServiceMode() == EAGLE_ATM_SVC_OUT_OF_SERVICE) || (GetServiceMode() == EAGLE_ATM_SVC_IN_SERVICE))
				{
					m_bNotifyStatusToAMS_PowerOn = TRUE;		// 최초 한번만 Terminal 상태 송신

					if (GetServiceMode() == EAGLE_ATM_SVC_OUT_OF_SERVICE)	// 최초 Out of Service인 경우 2번 전송되는 현상 방지
					{
						m_bAMSStatusSend = TRUE;
					}

					RunService_RemoteAccess(EAGLE_CMD_AMS_STATUS_SEND);	// Power On후 최초 한번 ATM Status Send는 여기서 바로 처리하도록 로직 수정

					return 1;

				}
			}
			else
			{
				if (GetServiceMode() == EAGLE_ATM_SVC_IN_SERVICE)
				{
					if (m_bAMSStatusSend == TRUE)
					{
						m_bAMSStatusSend = FALSE;

						LOG(Info, _T("Call AMS Status Send - In Service"));

						RunService_RemoteAccess(EAGLE_CMD_AMS_STATUS_SEND);	// Status Send는 여기서 바로 처리하도록 로직 수정

						return 1;
					}

					if (m_bAMSUploadJNLSend == TRUE)
					{
						if (CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_flg == S_ENABLE)
						{
							// Schedule Upload Journal 기능 추가
							int nDiffCnt = CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo() - CDEV_Manager::GetInstance()->m_DEV_EJL.GetUploadedAMSJnlNo();
							int nOption_JournalCnt = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Host.strSchedule_journal_count);

							if (nDiffCnt >= nOption_JournalCnt)
							{
								m_bAMSUploadJNLSend = FALSE;	// Journal Send 실패시 무한 try 방지를 위해 1번만 수행 (실패시 다음 InsertCard인 경우 수행)

								// Connect Close 후, open 시도
								//CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Sync_ConnectClose();

								LOG(Info, _T("Schedule Upload is executed - Diff (%d) / JnlCnt Option (%d)"), nDiffCnt, nOption_JournalCnt);

								// AMS Status Send를 수행하면 Journal이 자동으로 upload됨
								RunService_RemoteAccess(EAGLE_CMD_AMS_STATUS_SEND);	// Status Send는 여기서 바로 처리하도록 로직 수정

								return 1;
							}
						}
					}
				}
				
				if ((m_bAMSStatusSend == FALSE) && (GetServiceMode() == EAGLE_ATM_SVC_OUT_OF_SERVICE))
				{
					m_bAMSStatusSend = TRUE;

					LOG(Info, _T("Call AMS Status Send - Out of Service"));

					RunService_RemoteAccess(EAGLE_CMD_AMS_STATUS_SEND);	// Status Send는 여기서 바로 처리하도록 로직 수정

					return 1;
				}

			}
		}
	}
#endif

	// Schedule 관련 Check

	// 3. Day Close
	if (CEagleDataManager::GetInstance()->m_Config.m_Option.strScheduleDayClose == S_ENABLE)
	{
		COleDateTime	AutoTotalTime, CurTime;
		int				nScheduleTotalYear, nScheduleTotalMonth, nScheduleTotalDay, nScheduleTotalHour, nScheduleTotalMin;

		SYSTEMTIME st;
		GetLocalTime(&st);

		//CurTime = COleDateTime::GetCurrentTime();
		CurTime.SetDateTime((int)st.wYear, (int)st.wMonth, (int)st.wDay, (int)st.wHour, (int)st.wMinute, 0);

		nScheduleTotalYear = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseYear);
		nScheduleTotalMonth = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseMonth);
		nScheduleTotalDay = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseDay);
		nScheduleTotalHour = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime.Left(2));
		nScheduleTotalMin = CUtil::StringToInt(CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseTime.Right(2));

		AutoTotalTime.SetDateTime(nScheduleTotalYear, nScheduleTotalMonth, nScheduleTotalDay, nScheduleTotalHour, nScheduleTotalMin, 0);

		if (CurTime >= AutoTotalTime)
		{
			int		nCurTime, nAutoTotalTime;

			// make string:2008-10-11 -> int:20081011 or string 12:30 -> int:1230
			nCurTime = ((CurTime.GetHour() * 100) + CurTime.GetMinute());
			nAutoTotalTime = ((AutoTotalTime.GetHour() * 100) + AutoTotalTime.GetMinute());

			LOG(Info, _T("Auto Day Close - Current : %04d-%02d-%02d %02d:%02d:%02d"), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());

			// Update Auto Day Total Time.
			COleDateTime		NextAutoTotalTime;
			NextAutoTotalTime.SetDateTime(CurTime.GetYear(), CurTime.GetMonth(),  CurTime.GetDay(), 0, 0, 0);

			// +1 day
			if (nCurTime >= nAutoTotalTime)
			{
				COleDateTimeSpan	spanOneDay(1, 0, 0, 0);		// 1 day.
				NextAutoTotalTime += spanOneDay;
			}

			LOG(Info, _T("Update Next Auto Day Close Time : %04d-%02d-%02d %02d:%02d:%02d"), NextAutoTotalTime.GetYear(), NextAutoTotalTime.GetMonth(), NextAutoTotalTime.GetDay(), NextAutoTotalTime.GetHour(), NextAutoTotalTime.GetMinute(), NextAutoTotalTime.GetSecond());

			CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseYear = CUtil::IntToString(NextAutoTotalTime.GetYear());
			CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseMonth = CUtil::IntToString(NextAutoTotalTime.GetMonth());
			CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseDay = CUtil::IntToString(NextAutoTotalTime.GetDay());

			// Save File
			CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("dayclose_year"), CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseYear);
			CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("dayclose_month"), CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseMonth);
			CEagleDataManager::GetInstance()->m_Config.SaveOptionData(_T("dayclose_day"), CEagleDataManager::GetInstance()->m_Config.m_Option.strDayCloseDay);

			if (CEagleSVCLib::GetInstance()->Send_HeartbeatOrScheduleDayCloseMsg(EAGLE_TRAN_CODE_HOST_TOTALS) == TRUE)
			{
				CEagleSVCLib::GetInstance()->SaveEJL_N_Print_DayClose(EAGLE_TRAN_CODE_HOST_TOTALS, TRUE, FALSE);
			}
			else
			{
				//CEagleSVCLib::GetInstance()->SaveEJL_N_Print_DayClose(EAGLE_TRAN_CODE_HOST_TOTALS, FALSE, FALSE);
				// Day Close 실패시에는 Close 저널이 아닌 Information만 저널에 저장
				CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, _T("[SCHEDULE] FAILED DAY CLOSE"));
			}

			return 1;
		}
	}

	// CDM - Connect
	if (FALSE == m_CDM_Connect)
	{
		// 서비스 체크 - OUT OF SERVICE
		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_CDM_PORTOPEN, ERROR_DEVICE_CDM);
		SetOutOfService();

		return 0;
	}

	// CDR - Connect
	if (FALSE == m_CDR_Connect)
	{
		// 서비스 체크 - OUT OF SERVICE
		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_IDC_PORTOPEN, ERROR_DEVICE_IDC);
		SetOutOfService();

		return 0;
	}

	// DIO - Connect
	if (FALSE == m_DIO_Connect)
	{
		// 서비스 체크 - OUT OF SERVICE
		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_SIU_PORTOPEN, ERROR_DEVICE_SIU);
		SetOutOfService();

		return 0;
	}

	// EPP - Connect
	if (FALSE == m_EPP_Connect)
	{
		// 서비스 체크 - OUT OF SERVICE
		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_EPP_PORTOPEN, ERROR_DEVICE_EPP);
		SetOutOfService();

		return 0;
	}

	// PRT - Connect
	if (FALSE == m_PRT_Connect)
	{
		// 서비스 체크 - OUT OF SERVICE
		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_PTR_PORTOPEN, ERROR_DEVICE_PTR);
		SetOutOfService();
	
		return 0;
	}

	// 우선순위는 Device 장애 -> Setting -> Door -> CST 순으로 Check한다.
	if (CEagleSVCLib::GetInstance()->IsTerminal_Error() == TRUE)
	{
		SetOutOfService();
		return 0;
	}

	if (CEagleDataManager::GetInstance()->m_Client_Info.m_bIsSetError == TRUE)
	{
		// 서비스 체크 - OUT OF SERVICE
		SetOutOfService();

		return 0;
	}

	// 서비스 체크 - IN SERVICE
	SetInService();
	
	return 0;
}


CString strSaveErrorCode = _T("");	// Error Code 변경시 화면 갱신
/** **********************************************************
*	@brief		서비스 체크 - OUT OF SERVICE
*	@retval		없음
************************************************************/
void CSVC_Manager::SetOutOfService()
{
	// Mode가 변경되었거나, Error Code가 변경되었을 경우
	if ((GetServiceMode() != EAGLE_ATM_SVC_OUT_OF_SERVICE) || (CEagleSVCLib::GetInstance()->Get_ErrorCode() != strSaveErrorCode))
	{
		// Out of Service인 경우에는 무조건 광고를 Disalbe 처리 하도록 로직 보완
		CSCR_Manager::GetInstance()->SetAdvertisement(FALSE);

		int nErrDevKind = 0;

		m_CmdQueue.Clear();

		// EPP(DIO_4)를 제외한 모든 Flicker Off 처리
		CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_1|DIO_2|DIO_3|DIO_5|DIO_6|DIO_7, DIO_CMD_OFF);
		CDEV_Manager::GetInstance()->m_DEV_CDR.SetLed(FALSE);

		strSaveErrorCode = CEagleSVCLib::GetInstance()->Get_ErrorCode();

		LOG(Error, _T("OUT OF SERVICE ERROR = %s"), CEagleSVCLib::GetInstance()->Get_ErrorCode());

		// Log Backup시에 시간 Delay 현상 방지를 위해 로직 개선
		CString strPath;
		strPath.Format(_T("%s%s"), CUtil::GetAppPath(), PATH_LOG);

		// Log Back up
		if (GetServiceMode() != EAGLE_ATM_SVC_REMOTE_ACCESS_OF_SERVICE)
		{

#if (EXEC_RECOVERY_PROC)
			// Error Recovery 처리
			nErrDevKind = CEagleDataManager::GetInstance()->m_Client_Info.m_nErrorDevice;

			// CDM은 CST가 사용하지 못할 상태인 경우에는 Recovery 수행하지 않도록 변경
			int nCBXStatus = CDEV_Manager::GetInstance()->m_DEV_CDM.Get_CBXAllStatus();

			if (nCBXStatus == WFS_CDM_STATCUMISSING || nCBXStatus == WFS_CDM_STATCUEMPTY)
			{
				nErrDevKind &= ~(ERROR_DEVICE_CDM);
			}
			
			if (nErrDevKind & ERROR_DEVICE_CDM)
			{
				CString strWDMError;
				strWDMError == CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError().Right(2);
				if ( strWDMError == _T("21") || strWDMError == _T("22") || strWDMError == _T("23") || strWDMError == _T("24") ||
					 strWDMError == _T("31") || strWDMError == _T("32") || strWDMError == _T("33") || strWDMError == _T("34") || 
					 strWDMError == _T("35") || strWDMError == _T("36") || strWDMError == _T("37") || strWDMError == _T("38") || 
					 strWDMError == _T("39") || strWDMError == _T("3A") || strWDMError == _T("3B") || strWDMError == _T("3C") || 
					 strWDMError == _T("50") || strWDMError == _T("51") || strWDMError == _T("52") || strWDMError == _T("53") || 
					 strWDMError == _T("54") || strWDMError == _T("60") || strWDMError == _T("61") || strWDMError == _T("6C") || 
					 strWDMError == _T("70") || strWDMError == _T("71") || strWDMError == _T("72"))
				{
					nErrDevKind &= ~(ERROR_DEVICE_CDM);					
				}
				else if (CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError().Right(2) == _T("0B"))	// CE부 인증 장애시에도 Recovery 수행 안함
				{
					nErrDevKind &= ~(ERROR_DEVICE_CDM);
				}
			}

			if (nErrDevKind != 0)
			{
				// Error Recovery 화면 표시
				Client_Screen_Initialize();

				if (CEagleSVCLib::GetInstance()->Recovery_ErrorDevice(nErrDevKind) == FALSE)
				{
					//strSaveErrorCode = CEagleSVCLib::GetInstance()->Get_ErrorCode();

					CString strTemp;
					strTemp.Format(_T("RECOVERY FAILED (%s)"), CEagleSVCLib::GetInstance()->Get_ErrorCode());
					CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(text_field, strTemp);

					// Recovery 실패
				}
				else
				{
					// Recovery 성공시에 Log Backup 하도록 로직 개선
					CEagleSVCLib::GetInstance()->Add_Backup_LogFile(strPath, strSaveErrorCode);	// 이전 에러코드를 Set한다.

					// 성공시 Inservice 진입
					Client_ChangeCustom();

					return;
				}
			}
#endif
		}

		CrypteraEPP_EnterClearTextMode();	// Key Enable 처리

		SetServiceMode(EAGLE_ATM_SVC_OUT_OF_SERVICE);
		m_strKeyValue.Empty();	// Key 초기화

		if (CDEV_Manager::GetInstance()->m_nPrevServiceMode != EAGLE_ATM_SVC_REMOTE_ACCESS_OF_SERVICE)
		{
			// Recovery 실패시에도 Log Backup을 한번 더 수행하도록 로직 개선
			strSaveErrorCode = CEagleSVCLib::GetInstance()->Get_ErrorCode();

			CEagleSVCLib::GetInstance()->Add_Backup_LogFile(strPath, strSaveErrorCode);

			// SET JOURNAL
			CString strTemp;
			strTemp.Format(_T("OUT OF SERVICE (%s)"), strSaveErrorCode);
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(out_of_service, strTemp);
		}
	}
}


/** **********************************************************
*	@brief		서비스 체크 - IN SERVICE
*	@retval		없음
************************************************************/
void CSVC_Manager::SetInService(void)
{
	// IN SERVICE
	if (GetServiceMode() != EAGLE_ATM_SVC_IN_SERVICE)
	{
		m_CmdQueue.Clear();

		SetServiceMode(EAGLE_ATM_SVC_IN_SERVICE);
		Client_ChangeCustom();

		// AMS Status Send가 Enable일 경우 Out of Service 후 In Service 전환시 In Service Journal이 저장되지 않는 Bug Fix
		// m_bAMSStatusSend가 TRUE일 경우 Out of Service Report가 되었으므로, In Service 저널 저장하도록 함.
		if ((CDEV_Manager::GetInstance()->m_nPrevServiceMode != EAGLE_ATM_SVC_REMOTE_ACCESS_OF_SERVICE) || m_bAMSStatusSend == TRUE)
		{
			// 이전 상태가 Remote Access Mode가 아닌 경우에만 Journal 저장하도록 보완
			CString strTemp;
			strTemp.Format(_T("IN SERVICE"));
			CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(in_service, strTemp);
		}
	}
}

/** **********************************************************
*	@brief		서비스 체크 - OPERATOR SERVICE
*	@retval		없음
************************************************************/
void CSVC_Manager::SetOperatorService(void)
{
	// OPERATOR SERVICE
	if (GetServiceMode() != EAGLE_ATM_SVC_SUPERVISOR_SERVICE)
	{
		m_CmdQueue.Clear();

		// DIO 제어
		CDEV_Manager::GetInstance()->m_DEV_DIO.SetLed(DIO_4|DIO_5|DIO_6|DIO_7, DIO_CMD_ON);

		// Supervisor 진입시 Language를 ENGLISH로 설정
		CSCR_Manager::GetInstance()->SetScreenLocale(EAGLE_LANGUAGE_ENGLISH);

		CString strTemp;

		switch(GetOperatorMode())
		{
		case EAGLE_ATM_SVC_MASTER:
			LOG(Info, _T("Operator Mode Changed - Master"));
			strTemp.Format(_T("OPERATOR MODE(MASTER)"));
			break;

		case EAGLE_ATM_SVC_ADMIN:
			LOG(Info, _T("Operator Mode Changed - Administrator"));
			strTemp.Format(_T("OPERATOR MODE(ADMINISTRATOR)"));
			break;

		case EAGLE_ATM_SVC_REPLENISH:
			LOG(Info, _T("Operator Mode Changed - Replenisher"));
			strTemp.Format(_T("OPERATOR MODE(REPLENISHMENT)"));
			break;

		default:
			strTemp.Format(_T("OPERATOR MODE"));
			break;
		}

		CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(in_supervisor, strTemp);

		SetServiceMode(EAGLE_ATM_SVC_SUPERVISOR_SERVICE);

		LOG(Info, _T("RunService - Operator Service"));
	}
}


/** **********************************************************
*	@brief		서비스 체크 - REMOTE ACCESS SERVICE
*	@retval		없음
************************************************************/
void CSVC_Manager::SetRemoteAccessService(void)
{
	// OPERATOR SERVICE
	if (GetServiceMode() != EAGLE_ATM_SVC_REMOTE_ACCESS_OF_SERVICE)
	{
		m_CmdQueue.Clear();

		SetServiceMode(EAGLE_ATM_SVC_REMOTE_ACCESS_OF_SERVICE);

		LOG(Info, _T("RunService - Remote Access Service"));
	}
}


/** **********************************************************
*	@brief		서비스 종료
*	@retval		없음
************************************************************/
void CSVC_Manager::StopService(void)
{
	m_bStopService = TRUE;
}


/** **********************************************************
*	@brief		DIO - Front Door 열림
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Is_DIO_Front_Door_Open()
{
	return m_DIO_Front_Door_Open;
}


/** **********************************************************
*	@brief		DIO - Safe Door 열림
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Is_DIO_Safe_Door_Open()
{
	return m_DIO_Safe_Door_Open;
}


/** **********************************************************
*	@brief		DIO - Upper Frame 열림
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Is_DIO_Upper_Frame_Open()
{
	return m_DIO_Upper_Frame_Open;
}


/** **********************************************************
*	@brief		DIO - Ear Jack 삽입
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Is_DIO_Ear_Jack_Insert()
{
#if (NETWORK_OFFLINE_MODE)
	// Offline시 거래 모드에서는 EarJack 미지원하도록 변경
	return FALSE;
#endif

	return CDEV_Manager::GetInstance()->m_DEV_DIO.m_bAudioJackInserted;
}


/** **********************************************************
*	@brief		연결
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Connect(void)
{
	BOOL bResult = TRUE;
	BOOL bMUB_OpenResult = FALSE;
	int nPortNumber = 0;
	BOOL bExistCEInfo = FALSE;
	CString strTemp;
	int nReturn = 0;

	LOG(Info, _T("Proc - Connect - Start"));

	m_bDetectedSensorPoll_PowerOn = FALSE;

	nPortNumber = _ttoi(CEagleDataManager::GetInstance()->m_Config.m_Device.strPortNumber_MUB);

//	bMUB_OpenResult = CDEV_Manager::GetInstance()->m_DEV_MUB.Open(GetSafeHwnd(), nPortNumber);
	bMUB_OpenResult = CDEV_Manager::GetInstance()->m_DEV_DIO.Open(GetSafeHwnd(), nPortNumber);

	if (FALSE == bMUB_OpenResult)
	{
		LOG(Error, _T("Connect - MUB - Failed - Port Number : %d"), nPortNumber);
		// 에러 코드 설정
		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_MUB_OPENFAIL, ERROR_DEVICE_MUB);		
	}
	else
	{
		// Port Open 성공시
		LOG(Info, _T("Connect - MUB - Succeeded - Port Number : %d"), nPortNumber);

		// Sensor값 갱신이 될때까지 대기 (Timeout은 5초)
		DWORD	max_timeout = GetTickCount() + (5 * 1000);

		while(GetTickCount() < max_timeout)
		{
			if (m_bDetectedSensorPoll_PowerOn == TRUE)
			{
				LOG(Info, _T(" Sensor Status is updated"));
				break;
			}

			Sleep(EAGLE_SLEEP_INTERVAL);
		}
	}

	// DIO(SIU) 연결
	if (bMUB_OpenResult == TRUE)
		m_DIO_Connect = TRUE;
	else
		m_DIO_Connect = FALSE;

	// AP가 Holding이 되거나 다른 Application을 실행해야 하는 경우를 위한 방안
	// Safe Door가 반드시 열려 있을 때만 동작하도록 보안 조치함
	if (m_DIO_Safe_Door_Open == TRUE)
	{
		// USB Memory Stick이 꼽혀 있을 경우
		if (CUtil::IsExistFile(PATH_RECOVERY_APPLICATION) == TRUE)
		{
			// Other Application을 실행해야함
			STARTUPINFO si;
			ZeroMemory (&si, sizeof(si));
			si.cb = sizeof (si);
			si.wShowWindow = SW_HIDE;
			PROCESS_INFORMATION pi;				

			CreateProcess(PATH_RECOVERY_APPLICATION,NULL,NULL,NULL,NULL,FALSE,0,NULL,&si, &pi);

			LOG(Info, _T("Execute - Recovery Software"));

			CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_5MIN);
		}
		else
		{
			LOG(Info, _T("USB is not detected"));
		}
	}
	else
	{
		LOG(Info, _T("Safe Door is closed"));
	}

	// Initialize Screen Engine
	if (!CSCR_Manager::GetInstance()->Initialize(::AfxGetInstanceHandle(), 0, 0, _T("EAGLECE_ATM"), CUtil::GetAppPath()+_T("\\Screen")))
	{
		LOG(Error, _T("Fail to load screen"));
	}

	CSCR_Manager::GetInstance()->LoadAdvertisement();	// V1.0.2.5 2018.08.06 - 최초 AP Load시에만 ADV 파일을 Decode 하도록 수정.


#if (SHOW_OFFLINE_TEXT)
	CSCR_Manager::GetInstance()->SetDemoMode(TRUE);
#endif

	LOG(Info, _T("EagleScreen Initialize - End"));

	// Initialize Screen Display
	Client_Screen_Initialize();

	// Get APP Version
	CDEV_Manager::GetInstance()->GetAPPVersion();

	// CDM 연결
	nPortNumber = _ttoi(CEagleDataManager::GetInstance()->m_Config.m_Device.strPortNumber_CDM);

	if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_OpenPort(GetSafeHwnd(), nPortNumber, COMM_DEBUG_LEVEL))
	{
		LOG(Error, _T("Connect - CDM - Failed - Port Number : %d"), nPortNumber);

		// 에러 코드 설정
		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_CDM_PORTOPEN, ERROR_DEVICE_CDM);
		
		bResult = FALSE;
	}
	else
	{
		m_CDM_Connect = TRUE;

		LOG(Info, _T("Connect - CDM - Succeeded - Port Number : %d"), nPortNumber);

		if(TRUE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_GetExistCEInfo(bExistCEInfo))
		{
			if(FALSE == bExistCEInfo)
			{
				LOG(Info, _T("Connect - CDM - System start"));

				if(FALSE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_CEInfoCertification())
				{
					LOG(Error, _T("Connect - CDM - System Error"));

					// 에러 코드 설정
					CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError(), ERROR_DEVICE_CDM);
					bResult = FALSE;
				}
				else
				{
					// Firmware Update시 시간이 오래 걸리므로 화면 전환 후 수행하도록 로직 보완 필요
					if(FALSE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_UpdateFirmware())
					{
						LOG(Error, _T("Connect - CDM - Update Firmware Error"));

						// 에러 코드 설정
						CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError(), ERROR_DEVICE_CDM);
						bResult = FALSE;
					}
				}
			}
			else
			{
				if(FALSE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_UpdateFirmware())
				{
					LOG(Error, _T("Connect - CDM - Update Firmware Error"));

					// 에러 코드 설정
					CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError(), ERROR_DEVICE_CDM);
					bResult = FALSE;
				}
			}
		}
		else
		{
			m_CDM_CommError = TRUE;

			LOG(Error, _T("Connect - CDM - System Info Error"));

			// 에러 코드 설정
			CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError(), ERROR_DEVICE_CDM);
			bResult = FALSE;
		}
	}

	// CDR 연결
	nPortNumber = _ttoi(CEagleDataManager::GetInstance()->m_Config.m_Device.strPortNumber_CDR);

	if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDR.Open(GetSafeHwnd(), nPortNumber, 30))
	{
		CString strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

		LOG(Error, _T("Connect - CDR - Failed - Port Number : %d"), nPortNumber);

		// 에러 코드 설정
		CEagleSVCLib::GetInstance()->Set_ErrorCode(strErrorCode, ERROR_DEVICE_IDC);

		bResult = FALSE;
	}
	else
	{
		m_CDR_Connect = TRUE;

		LOG(Info, _T("Connect - CDR - Succeeded - Port Number (%d)"), nPortNumber);
	}

	// PRT 연결
	int nRet = 0;
	CString strComPort;
	strComPort.Format(_T("COM%s:"), CEagleDataManager::GetInstance()->m_Config.m_Device.strPortNumber_PTR);
	nRet = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_Open(GetSafeHwnd(), strComPort);
	if (!nRet)
	{
		LOG(Error, _T("Connect - PRT - Failed (Port Number : %s"), CEagleDataManager::GetInstance()->m_Config.m_Device.strPortNumber_PTR);

		// 에러 코드 설정
		CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_PTR_PORTOPEN);

		bResult = FALSE;
	}
	else
	{
		m_PRT_Connect = TRUE;

		LOG(Info, _T("Connect - PRT - Succeeded - Port Number : %s"), CEagleDataManager::GetInstance()->m_Config.m_Device.strPortNumber_PTR);
	}


	nPortNumber = _ttoi(CEagleDataManager::GetInstance()->m_Config.m_Device.strPortNumber_EPP);
	char szPort[100] = { 0, };
	sprintf_s(szPort, 100, "COM%d:", nPortNumber);

	if (EPP_CRYPTERA == CEagleDataManager::GetInstance()->m_Config.m_Device.strType_EPP)
	{
		// Dll Version 1.0.0.2버전 사용해야함.
		m_callback = (EventCallBackFunction)EventControlFunc;
		CDEV_Manager::GetInstance()->m_DEV_CREPP.RegistEventCallBackFunction(m_callback);

		if (0 == CDEV_Manager::GetInstance()->m_DEV_CREPP.OpenDevice(nPortNumber, 115200))
		{
			LOG(Info, _T("Connect - CREPP - Succeeded - Port Number : %d"), nPortNumber);

			// Set Stuck EPP Timeout Setting.
			strTemp = CDEV_Manager::GetInstance()->m_DEV_CREPP.GetRegisterValue(STUCKKEY_PERIOD);

			BOOL bSetStuckTimeoutResult = TRUE;

			if (strTemp != STUCK_TIME_VALUE)
			{
				nReturn = CDEV_Manager::GetInstance()->m_DEV_CREPP.SetRegisterValue(STUCKKEY_PERIOD, STUCK_TIME_VALUE);

				if ( nReturn == 0)
				{
					LOG(Info, _T("Set Stuck EPP Timeout Value - Success... wait 30 sec for applying"));

					// Close Port를 한 후에 30초 대기 후 다시 Open Port 수행
					CDEV_Manager::GetInstance()->m_DEV_CREPP.CloseDevice();

					CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_30SEC);	// 30 sec 대기

					CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bEPPReady = FALSE;

					// 다시 Open Port 처리
					if (0 == CDEV_Manager::GetInstance()->m_DEV_CREPP.OpenDevice(nPortNumber, 115200))
					{
						LOG(Info, _T("Connect - CREPP - Succeeded - Retry Port Number : %d"), nPortNumber);
					}
					else
					{
						LOG(Error, _T("Connect - CREPP - Try Failed"));
						CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_EPP_PORTOPEN, ERROR_DEVICE_EPP);

						bSetStuckTimeoutResult = FALSE;
					}
				}
				else
				{
					LOG(Info, _T("Set Stuck EPP Timeout Value - Failed (%d)"), nReturn);
				}
			}

			if (bSetStuckTimeoutResult == TRUE)
			{
				// 최초 1회 EPP Status를 조회하여 EPP State 상태 갱신
				CDEV_Manager::GetInstance()->m_DEV_CREPP.GetDeviceStatus();

				LOG(Info, _T("CREPP - Current Active State : %d"), CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEPPState);

				m_strMasterKey_KCV = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(MASTER_KEYNAME);

				LOG(Info, _T("CREPP - Master Key KCV : %s"), m_strMasterKey_KCV);

				m_strPINKey_KCV = CDEV_Manager::GetInstance()->m_DEV_CREPP.Get_KeyCheckValue(WORKING_KEYNAME);

				LOG(Info, _T("CREPP - PIN Working Key KCV : %s"), m_strPINKey_KCV);

				m_EPP_Connect = TRUE;
			}
		}
		else
		{
			LOG(Error, _T("Connect - CREPP - Failed"));
			CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_EPP_PORTOPEN, ERROR_DEVICE_EPP);
		}
	}
	else // EPP_MFS
	{
		// DLL Load 체크
		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_EPP.IsDllLoaded())
		{
			LOG(Error, _T("Connect - EPP - Failed to load dll"));
		}

		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_EPP.EPP_OpenPort(GetSafeHwnd(), nPortNumber))
		{
			LOG(Error, _T("Connect - EPP - Failed - Port Number : %d"), nPortNumber);

			// 에러 코드 설정
			CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_EPP_PORTOPEN, ERROR_DEVICE_EPP);
			bResult = FALSE;
		}
		else
		{
			LOG(Info, _T("Connect - EPP - Succeeded - Port Number : %d"), nPortNumber);

			// EPP 연결 확인
			m_EPP_Connect = TRUE;
		}
	}
	//

	// EJL 연결
	if (FALSE == CDEV_Manager::GetInstance()->m_DEV_EJL.Open(GetSafeHwnd()))
	{
		LOG(Error, _T("Connect - EJL - Failed"));

		// 에러 코드 설정
		CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastError());	

		m_EJL_Connect = FALSE;
	}
	else
	{
		m_EJL_Connect = TRUE;

		LOG(Info, _T("Connect - EJL - Succeeded"));

		// POWER ON (AP / OS 버전 추가)
		CString strTemp, strKernelVer, strStackTime;

		// Power On 전에 이전 Service Mode를 Check 후에 Journal을 추가로 저장 (가동률 Data 왜곡 현상 방지)
		// 마지막 저널이 System Reboot인 경우에는 Powr Off 저널을 저장하지 않도록 추가 수정
		int nJnlNo = CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastJournalNo();

		FLSEJCOMMINFO CommInfo;
		memset(&CommInfo, 0x00, sizeof(CommInfo));

		FLSEJINFO	JnlInfo;
		memset(&JnlInfo, 0x00, sizeof(JnlInfo));

		LOG(Info, _T("Last Journal No (%d)"), nJnlNo);

		if (CDEV_Manager::GetInstance()->m_DEV_EJL.GetDetailJnlData(nJnlNo, &CommInfo, &JnlInfo) == TRUE)
		{
			if (CommInfo.jnl_cd != reboot)
			{
				CIniFile iniFile(ATM_STATUS_MONITORING_FILE);

				strStackTime = iniFile.ReadString(_T("ATM_STATUS_INFO"), _T("TIME"), _T(""));

				if (strStackTime.GetLength() == 14)
				{
					strTemp.Format(_T("OUT OF SERVICE (POWER OFF)"));
					CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(out_of_service, strTemp, FALSE, strStackTime);

					LOG(Info, _T("Save Journal Power Off Time in the EJ (%s - %s)"), strTemp, strStackTime);
				}
				else
				{
					LOG(Info, _T("Power Off file or time is invalid (%s)"), strStackTime);
				}
			}
			else
			{
				LOG(Info, _T("Last Journal is System reboot - Skip"));
			}
		}
		else
		{
			LOG(Info, _T("GetDetailJnlData error - [%s] - Skip"), CDEV_Manager::GetInstance()->m_DEV_EJL.GetLastError());
		}

		CString strProductVersion, strAppVersion;
		strProductVersion = CDEV_Manager::GetInstance()->m_strAP_Version;

#if (NETWORK_OFFLINE_MODE)
		strAppVersion.Format(_T("D%s"), strProductVersion);
#else
		strAppVersion.Format(_T("V%s"), strProductVersion);
#endif

		strKernelVer = CUtil::String_GetRegistry(REGISTRY_OS_INFO_KEY, REGISTRY_KERNEL_VALUE_NAME);
		strKernelVer.Replace(_T("v"), _T("V"));

		strTemp.Format(_T("POWER ON (%s, %s)"), strKernelVer, strAppVersion);
		CDEV_Manager::GetInstance()->SaveEJL_SetTextValue(power_on, strTemp);
	}

	// Camera쪽에 Hwnd Set
#if (USE_CAMERA_DEVICE)
	CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetHwnd(g_hWndDisplay);

	// Camera Driver 초기화
	CDEV_Manager::GetInstance()->m_DEV_CAMERA.InitCameraDriver();

#else
	CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetUseCamera(FALSE);

#endif

	// CAMERA 연결- Camera Open후 Run시 System 부하가 많이 발생하여 Capture하는 시점에 Open하는 걸로 로직 변경
	// 카메라 드라이버 유무에 따라 Camera Open 시도를 결정한다.
	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.GetCameraSetting() == TRUE)
 	{
 		if(TRUE == CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsExistCameraDriver())
		{
 			if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CAMERA.Open())
 			{
 				LOG(Error, _T("Connect - CAMERA - Failed"));
	 
 				// 에러 코드 설정
 				CEagleSVCLib::GetInstance()->Set_ErrorCode(ERROR_CAM_OPENFAIL, ERROR_DEVICE_CAM);	 
 			}
 			else
 			{
 				LOG(Info, _T("Connect - CAMERA - Succeeded"));
  			}
		}
		else
		{
			CDEV_Manager::GetInstance()->m_DEV_CAMERA.SetUseCamera(FALSE);
			LOG(Info, _T("Connect - CAMERA - Driver is not exist. - Camera Settin False Setting - Forced"));
		}
 	}
	else
	{
		LOG(Info, _T("Camera Setting is disable"));
	}

	LOG(Info, _T("Proc - Connect - End"));

	return bResult;
}


/** **********************************************************
*	@brief		연결
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Disconnect(void)
{
	BOOL bResult = TRUE;

	LOG(Info, _T("Proc - DisConnect - Start"));

	// CAMERA 해제
	if (CDEV_Manager::GetInstance()->m_DEV_CAMERA.IsOpened() == TRUE)
	{
		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CAMERA.Close())
		{
			LOG(Error, _T("Disconnect - CAMERA - Failed"));

			bResult = FALSE;
		}
		else
		{
			LOG(Info, _T("Disconnect - CAMERA - Succeeded"));
		}
	}


	// CDR 해제
	if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDR.Close())
	{
		CString strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

		LOG(Error, _T("Disconnect - CDR - Failed - Error Code : ") + strErrorCode);

		bResult = FALSE;
	}
	else
	{
		LOG(Info, _T("Disconnect - CDR - Succeeded"));
	}


	// EPP 해제
	if (EPP_CRYPTERA == CEagleDataManager::GetInstance()->m_Config.m_Device.strType_EPP)
	{

		bResult = CDEV_Manager::GetInstance()->m_DEV_CREPP.CloseDevice();
		if (bResult == FALSE)
		{
			LOG(Error, _T("Disconnect - CREPP - Failed"));
		}
		else
		{
			LOG(Info, _T("Disconnect - CREPP - Succeeded"));
		}
	}
	else // EPP_MFS
	{
		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_EPP.EPP_ClosePort())
		{
			LOG(Error, _T("Disconnect - EPP - Failed"));

			bResult = FALSE;
		}
		else
		{
			LOG(Info, _T("Disconnect - EPP - Succeeded"));
		}
	}

	// PRT 해제
	if (!CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_Close())
	{
		LOG(Error, _T("Disconnect - PRT - Failed"));

		bResult = FALSE;
	}
	else
	{
		LOG(Info, _T("Disconnect - PRT - Succeeded"));
	}


	// DIO 해제
	if (FALSE == CDEV_Manager::GetInstance()->m_DEV_DIO.Close())
	{
		LOG(Error, _T("Disconnect - DIO - Failed"));

		bResult = FALSE;
	}
	else
	{
		LOG(Info, _T("Disconnect - DIO - Succeeded"));
	}

	// CDM 해제
	if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_ClosePort())
	{

		LOG(Error, _T("Disconnect - CDM - Failed - Error Code : ") + CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError());

		bResult = FALSE;
	}
	else
	{
		LOG(Info, _T("Disconnect - CDM - Succeeded"));
	}

	// EJL Close
	if (FALSE == CDEV_Manager::GetInstance()->m_DEV_EJL.Close())
	{
		LOG(Error, _T("Disconnect - EJL - Failed"));
		bResult = FALSE;
	}
	else
	{
		LOG(Info, _T("Disconnect - EJL - Succeeded"));
	}

	LOG(Info, _T("Proc - DisConnect - End"));

	return bResult;
}


/** **********************************************************
*	@brief		Device - 초기화
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::Proc_Device_Initialize(void)
{
	LOG(Info, _T("RunService - Device - Initialize - Start"));

	BOOL bResult = TRUE;
	CString strTemp;

	int nProcessing_State = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nProtocol_processing_state;

	// Power Off 처리 추가 (출금 거래 중 Reversal Flag가 off인 경우 처리
	if ((CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_WITHDRAWAL) && 
		(CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal == 0))
	{
		if (CEagleDataManager::GetInstance()->m_Config.m_Host.strProtocolType == _T("VISA"))
		{
			if (nProcessing_State >= ENQIN_STS)
			{
				LOG(Info, _T("VISA PROTOCOL - Set Reversal Flag - Processing State (%d)"), nProcessing_State);

				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nProtocol_processing_state = INIT_STS;

				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
				// Reversal Reason Set
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
			}
		}
		else
		{
			if (nProcessing_State >= CONN_STS)
			{
				LOG(Info, _T("STANDARD PROTOCOL - Set Reversal Flag - Processing State (%d)"), nProcessing_State);

				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nProtocol_processing_state = INIT_STS;

				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = ENABLE_REVERSAL;
				// Reversal Reason Set
				CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strReasonforReversal = REVERSAL_REASON_PROTOCOL_ERROR;
			}
		}
	}

	if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal == 0)
	{
		// Power Off 후 Reversal이 아닌 경우 Transaction에 EJL이 Save 되지 않을 경우에 대한 처리
		if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_bSaveEJLforTransRecord == TRUE)
		{
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_bSaveEJLforTransRecord = FALSE;
			CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfoData(_T("IsSaveEJLTransRecord"), _T("0"));

			if (CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strTransType == S_WITHDRAWAL)
				CEagleSVCLib::GetInstance()->Set_ErrorCode(POWEROFF_DISPENSING_COMP, ERROR_DEVICE_CDM);

			CEagleSVCLib::GetInstance()->SaveEJL_TransactionInfo();
		}
	}
	else
	{
		// Reversal Flag가 On되어져 있는 상태에서 방출 중 Power Off인 경우 Requested Amount는 Dispensed Amount로 처리 (Reversal 미수행)
		if (CEagleDataManager::GetInstance()->m_Config.GetDispensingFlag() == TRUE)
		{
			CEagleDataManager::GetInstance()->m_Config.ClearDispenseFlag();

			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_nIsReversal = 0;
			CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount = CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strRequestAmount;
			CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfoData(_T("IsReversal"), _T("0"));
			CEagleDataManager::GetInstance()->m_Config.SaveLastTransInfoData(_T("DispenseAmount"), CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount);

			// SUM 로직 추가
			// Sum Withdrawal Transaction
			CEagleDataManager::GetInstance()->m_Config.m_TotalInfo.SumData(S_WITHDRAWAL, CEagleDataManager::GetInstance()->m_Config.m_LastTransInfo.m_strDispenseAmount);
			CEagleDataManager::GetInstance()->m_Config.SaveTotalInfo();

			// 방출 중 Power Off 장애 Setting
			CEagleSVCLib::GetInstance()->Set_ErrorCode(POWEROFF_DISPENSING, ERROR_DEVICE_CDM);
			CEagleSVCLib::GetInstance()->SaveEJL_TransactionInfo();
		}
	}

	// Execute Reversal Procedure
	Proc_Client_PowerOffReversalProcessing();

	// DIO Get Version 수행
	if (m_DIO_Connect == TRUE)
	{
		CDEV_Manager::GetInstance()->m_DEV_DIO.GetVersion();
	}

	// CDR 초기화
	if (m_CDR_Connect == TRUE)
	{
		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDR.Initialize())
		{
			CString strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

			LOG(Error, _T("Initialize - CDR - Error Code : ") + strErrorCode);

			// 에러 코드 설정
			CEagleSVCLib::GetInstance()->Set_ErrorCode(strErrorCode, ERROR_DEVICE_IDC);

			bResult = FALSE;
		}
		else
		{
			LOG(Info, _T("Initialize - CDR - Succeeded"));

			// 최초 한번 CDR FW Version Read하도록 로직 보안
			CDEV_Manager::GetInstance()->m_DEV_CDR.GetVersionInfo();
			
			if(TRUE == CDEV_Manager::GetInstance()->m_DEV_CDR.GetSupportEncryption())
			{
				// 암호화 인증 과정
				// 1. 암호화 연결 수행
				CDEV_Manager::GetInstance()->m_DEV_CDR.GenerateKeyInfoAndAuthData();

				BOOL bEncrypResult = CDEV_Manager::GetInstance()->m_DEV_CDR.GenerateKeyofDeviceAttestationDataAndKey();
				if(TRUE == bEncrypResult)
				{
					LOG(Info, _T("GenerateKeyofDeviceAttestationDataAndKey - CDR - Succeeded"));
				}
				else
				{
					CString strErrorCode = CDEV_Manager::GetInstance()->m_DEV_CDR.GetLastError();

					LOG(Error, _T("GenerateKeyofDeviceAttestationDataAndKey - CDR - Error Code : ") + strErrorCode);

					// 에러 코드 설정
					CEagleSVCLib::GetInstance()->Set_ErrorCode(strErrorCode, ERROR_DEVICE_IDC);
					
					bResult = FALSE;
				}
			}
		}
	}

	// CDM 초기화 - 초기에 Port Open 성공 후 통신 장애가 아닌 경우에만 하기 CDM_CfgStatus 수행하도록 로직 보완 (Delay 줄이기 위함)
	if ((m_CDM_Connect == TRUE) && (m_CDM_CommError == FALSE))
	{
		if (FALSE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_CfgStatus())
		{
			// 에러 코드 설정
			CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError(), ERROR_DEVICE_CDM);

			LOG(Error, _T("Initialize - CDM - Failed - Config (%s)"), CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError());

			bResult = FALSE;
		}
		else
		{
			LOG(Info, _T("Initialize - CDM - Succeeded - Config"));
		}

		if(FALSE == CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_Initialize(TRUE))
		{
			LOG(Error, _T("Initialize - CDM - Failed - Reset"));

			CEagleSVCLib::GetInstance()->Set_ErrorCode(CDEV_Manager::GetInstance()->m_DEV_CDM.GetLastError(), ERROR_DEVICE_CDM);
			LOG(Error, _T("CDM ERROR = %s"), CEagleSVCLib::GetInstance()->Get_ErrorCode());

			bResult = FALSE;
		}
		else
		{
			LOG(Info, _T("Initialize - CDM - Succeeded - Reset"));
		}

		CDEV_Manager::GetInstance()->m_DEV_CDM.CDM_QuiryStatusStart();	// Polling Start
	}

	// EPP 초기화
	if (EPP_CRYPTERA == CEagleDataManager::GetInstance()->m_Config.m_Device.strType_EPP)
	{
		// 초기화시에는 EPP 비활성화 처리
		if (m_EPP_Connect)
		{
			// Version Read 후 명령 수행하도록 로직 수정
			CDEV_Manager::GetInstance()->m_DEV_CREPP.GetVersionInfo();

			CrypteraEPP_StopClearTextMode();
		}
	}

	// PRT 초기화
	if (m_PRT_Connect == TRUE)
	{
		// PRT Open 성공시 Version Read부터 수행하도록 로직 보완
		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetVersionInfo();

		// Power On시에 Flash Rom에 Image Download하도록 로직 수정
//#if(ENABLE_RECEIPT_HEADER_IMAGE_PRINT)
		// Print Logo Image
		CString strTemp;
		
		strTemp.Format(_T("%s%s"), CUtil::GetAppPath(), RECEIPT_LOGO_IMAGE);
		CT2A	szPath(strTemp);

		int nPrintError = CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_S_SetBitmapFlash(EAGLE_PRT_TYPE_USB, &szPath.m_psz, 1);

		if (EAGLE_PRT_SUCCESS != nPrintError)
		{
			LOG(Info, _T("PRT - Download Bitmap To Flash is Failed - %d"), nPrintError);
		}
		else
		{
			LOG(Info, _T("PRT - Download Bitmap To Flash is success - (%s)"), strTemp);
		}
//#endif

		CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_QuiryStatusStart();

		CUtil::Sleep_Wait(2000);

		LOG(Info, _T("Initialize - PRT - Status Info : Status (%d) PaperStatus(%d)"), CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus(), CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetPaperStatus());
		
		if (CDEV_Manager::GetInstance()->m_DEV_PRT.PTR_GetStatus() == EAGLE_PRT_NORMAL)
		{
			LOG(Info, _T("Initialize - PRT - Status is Normal"));
		}
		else
		{
			LOG(Info, _T("Initialize - PRT - Status is Error"));
		}
	}

	LOG(Info, _T("RunService - Device - Initialize - End"));

	if (bResult == FALSE)
		CSVC_Manager::GetInstance()->SetOutOfService();

	return bResult;
}


/** **********************************************************
*	@brief		DIO 통지
*	@retval		없음
************************************************************/
LRESULT CSVC_Manager::OnDioNotify(WPARAM wParam, LPARAM lParam)
{

	LOG (Info, _T("OnDioNotify - Start"));

	// Sensor쪽 Interface 변경으로 인해 Emulation하는 Code 추가
	char chSensorInfo = 0;
	chSensorInfo = (char)wParam;

	// Front Door Open (Switch Sensor 변경으로 인해 값을 거꾸로 인지하도록 수정
#if (FRONT_DOOR_REVERSE)
	if (chSensorInfo & 0x01)
	{
		if (m_DIO_Front_Door_Open != FALSE)
		{
			m_DIO_Front_Door_Open = FALSE;
			LOG(Info, _T("Notify - DIO - Front Door Close"));
		}
	}
	else
	{
		if (m_DIO_Front_Door_Open != TRUE)
		{
			m_DIO_Front_Door_Open = TRUE;
			LOG(Info, _T("Notify - DIO - Front Door Open"));
		}
	}
#else
	if (chSensorInfo & 0x01)
	{
		if (m_DIO_Front_Door_Open != TRUE)
		{
			m_DIO_Front_Door_Open = TRUE;
			LOG(Info, _T("Notify - DIO - Front Door Open"));
		}
	}
	else
	{
		if (m_DIO_Front_Door_Open != FALSE)
		{
			m_DIO_Front_Door_Open = FALSE;
			LOG(Info, _T("Notify - DIO - Front Door Close"));
		}
	}
#endif

	// Safe Door
	if (chSensorInfo & 0x02)
	{
		if (m_DIO_Safe_Door_Open != TRUE)
		{
			m_DIO_Safe_Door_Open = TRUE;
			LOG(Info, _T("Notify - DIO - Safe Door Open"));
		}
	}
	else
	{
		if (m_DIO_Safe_Door_Open != FALSE)
		{
			m_DIO_Safe_Door_Open = FALSE;
			LOG(Info, _T("Notify - DIO - Safe Door Close"));
		}
	}

	// EarJack -> DIO쪽 변수를 참조하도록 로직 수정
	if (chSensorInfo & 0x10)	// 값이 Multi Board일 경우 반대로 올라옴 , Nexcom으로 변경시 다시 반대로 올라와 CS100과 동일한 사양으로 통일함
	{
		m_DIO_Ear_Jack_Insert = FALSE;

		if (m_DIO_Ear_Jack_Insert != TRUE)
		{
			m_DIO_Ear_Jack_Insert = TRUE;
			LOG(Info, _T("Notify - DIO - EarJack Inserted"));
		}
	}
	else
	{
		if (m_DIO_Ear_Jack_Insert != FALSE)
		{
			m_DIO_Ear_Jack_Insert = FALSE;
			LOG(Info, _T("Notify - DIO - EarJack Removed"));
		}
	}

	if (m_bDetectedSensorPoll_PowerOn == FALSE)
		m_bDetectedSensorPoll_PowerOn = TRUE;

	return 0;
}


/** **********************************************************
*	@brief		EPP 통지
*	@retval		없음
************************************************************/
//LRESULT CSVC_Manager::OnEppNotify(WPARAM wParam, LPARAM lParam)
//{
//	// 버튼 입력 값 취득
//	if ((WPARAM)wParam == _NORMAL_KEY_PRESS)
//	{
//		BYTE* pPinValue = (BYTE*)lParam;
//
//		CString strPin = _T("");
//		
//		strPin.Format(_T("%c%c"), pPinValue[0], pPinValue[1]);
//
//		m_EppMsgBuff.Enqueue((BYTE)_tcstoul(strPin, NULL, 16));
//
//		LOG(Info, _T("Notify - EPP - Pin Pressed - %s"), strPin);
//
//		CHAR	chKeyValue = 0x00;
//
//		PlaySound(CUtil::GetKeyPressWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
//
//		chKeyValue = m_EppMsgBuff.Dequeue();
//
//		// Supervisor Mode Check
//		if (GetServiceMode() == EAGLE_ATM_SVC_OUT_OF_SERVICE)
//		{
//			if (Is_DIO_Front_Door_Open() == TRUE)
//			{
//				if ((KEYMAP_0 <= chKeyValue) && (chKeyValue <= KEYMAP_9))
//				{
//					// 숫자 입력
//					m_strKeyValue.Append(CString(chKeyValue));
//				}
//				else if (KEYMAP_ENTER == chKeyValue)
//				{
//					// ENTER 입력
//					if (CEagleDataManager::GetInstance()->m_Config.m_System.strAdministrator_password == m_strKeyValue.Right(6))		// 마지막 6자리 Check
//					{
//						CSVC_Manager::GetInstance()->SetOperatorMode(EAGLE_ATM_SVC_ADMIN);
//					}
//					else if (CEagleDataManager::GetInstance()->m_Config.m_System.strMaster_password == m_strKeyValue.Right(6))			// 마지막 6자리 Check
//					{
//						CSVC_Manager::GetInstance()->SetOperatorMode(EAGLE_ATM_SVC_MASTER);
//					}
//					else if (CEagleDataManager::GetInstance()->m_Config.m_System.strReplenishment_password == m_strKeyValue.Right(6))	// 마지막 6자리 Check
//					{
//						CSVC_Manager::GetInstance()->SetOperatorMode(EAGLE_ATM_SVC_REPLENISH);
//					}
//
//					m_strKeyValue.Empty();
//				}
//			}
//		}
//		else
//		{
//			CSCR_Manager::GetInstance()->SetKeyEvent(CString(chKeyValue));
//		}
//	}
//
//	return 0;
//}


/** **********************************************************
*	@brief		PRT 통지
*	@retval		없음
************************************************************/
LRESULT CSVC_Manager::OnPrtNotify(WPARAM wParam, LPARAM lParam)
{
	return 0;
}


/** **********************************************************
*	@brief		USBMEM 통지
*	@retval		없음
************************************************************/
LRESULT CSVC_Manager::OnUsbMemNotify(WPARAM wParam, LPARAM lParam)
{
	return 0;
}


/** **********************************************************
*	@brief		DIO 통지
*	@retval		없음
************************************************************/
LRESULT CSVC_Manager::OnAMSNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == EAGLE_ATM_DEV_AMS_LISTEN_START)
	{
		// AMS Listening Start
		//if (CEagleDataManager::GetInstance()->m_Config.m_Host.strRms_en_flg == S_ENABLE)
		//{
		//	// Close 후 Open하도록 수정
		//	CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Sync_ConnectClose();

		//	LOG(Info, _T("OnAMSNotify - AMS Listening Start"));
		//	CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Sync_ConnectOpen(_T("LISTEN"));
		//	LOG(Info, _T("OnAMSNotify - AMS Listening Finish"));
		//}
	}
	else
	{
		// AMS Connection Close
		LOG(Info, _T("OnAMSNotify - AMS Close Start"));
		CDEV_Manager::GetInstance()->m_DEV_HOST.AMS_Sync_ConnectClose();
		LOG(Info, _T("OnAMSNotify - AMS Close Finish"));
	}

	return 0;
}


/** **********************************************************
*	@brief		디바이스 변경
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	return TRUE;
}

/** **********************************************************
*	@brief		Cryptera EPP Normal Key entry
*	@retval		없음
************************************************************/
BOOL CSVC_Manager::CrypteraEPP_EnterClearTextMode()
{
#if (EMULATION_EPP_DEVICE)
	// Win32 simulator: the modeless pad stands in for the key pad that was
	// just enabled. Clear-text mode, so key codes are reported as pressed.
	CDevSimPinPadDlg::Show(FALSE);
	CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEntryKeyStatus = ENTRY_KEY_STATUS_ENABLE;
	return TRUE;
#endif

	// 이미 Enable 상태라면 재시도 하지 않도록 처리
	if (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEntryKeyStatus == ENTRY_KEY_STATUS_ENABLE)
	{
		//LOG(Info, _T("Already Key is enabled"));
		return TRUE;
	}

	if (m_EPP_Connect == FALSE)
		return TRUE;

	m_nEPP_Enable_Recovery = GetTickCount() + 60000;	// EPP Enable 명령 60초 Count Save;
	//LOG(Info, _T("Execute CrypteraEPP_EnterClearTextMode (%d)"), m_nEPP_Enable_Recovery);

	int bRet = CDEV_Manager::GetInstance()->m_DEV_CREPP.StartEntryClear();

	if (bRet)
		return FALSE;

	return TRUE;
}

/** **********************************************************
*	@brief		Cryptera EPP Normal Key entry Stop
*	@retval		없음
************************************************************/
void CSVC_Manager::CrypteraEPP_StopClearTextMode()
{

#if (EMULATION_EPP_DEVICE)
	CDevSimPinPadDlg::Hide();
	CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEntryKeyStatus = ENTRY_KEY_STATUS_DISABLE;
	return;
#endif

	// 이미 Disable 상태라면 재시도 하지 않도록 처리
	if (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEntryKeyStatus == ENTRY_KEY_STATUS_DISABLE)
	{
		LOG(Info, _T("Already Key is disabled"));
		return;
	}

	if (m_EPP_Connect == FALSE)
		return;

	WORD wRet = 0;
	BOOL bRet = FALSE;

	bRet = CDEV_Manager::GetInstance()->m_DEV_CREPP.StopEntry();

}

/** **********************************************************
*	@brief		Cryptera EPP event 전달
*	@retval		없음
************************************************************/
int EventControlFunc(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength)
{
	switch(usEventID)
	{
	case EVENT_ENTRY:
		CSVC_Manager::GetInstance()->PressKeyHandler(usParam, lpByte, usDataLength);
		break;

	case EVENT_FLOAT_FRAME:
		break;

	case EVENT_STATUS:
		CSVC_Manager::GetInstance()->StatusEventHandler(usParam, lpByte, usDataLength);
		break;

	default:
		LOG(Info, _T("EventControlFunc - Event ID : 0x%x, Param : 0x%x"), usEventID, usParam);
		break;
	}
	return 0;
}

/** **********************************************************
*	@brief		Cryptera EPP Key 전달
*	@retval		없음
************************************************************/
int CSVC_Manager::PressKeyHandler(unsigned short usParam, byte *lpByte, unsigned short usDataLength)
{
	char byPinEmulatedValue = 0x00;
	CString	strTemp;

	if(usDataLength < 17)
	{
		LOG(Error, _T("PressKeyHandler - Data Size Error (%d)"), usDataLength);
		return -1;
	}

	BYTE byKeyData[20];

	memset(byKeyData, 0, sizeof(byKeyData));
	memcpy(byKeyData, lpByte, 17);

	BYTE byKeyCode = 0;
	unsigned long ulXFSKeyCode = 0;
	unsigned short usKeyCount = 0;
	unsigned short usReasonCode = 0;

	BYTE byPos = 0;
	//0-1 : Size 1
	byPos++;
	byPos++;
	//2 : Key Code
	byKeyCode = byKeyData[byPos++];

	//3-4 : Size 2
	byPos++;
	byPos++;
	ulXFSKeyCode = byKeyData[byPos++] * 0x1000000;
	ulXFSKeyCode += byKeyData[byPos++] * 0x10000;
	ulXFSKeyCode += byKeyData[byPos++] * 0x100;
	ulXFSKeyCode += byKeyData[byPos++];

	//9-10 : Size 3
	byPos++;
	byPos++;
	usKeyCount = byKeyData[byPos++] * 0x100;
	usKeyCount += byKeyData[byPos++];

	//13-14 : Size 4
	byPos++;
	byPos++;
	usReasonCode = byKeyData[byPos++] * 0x100;
	usReasonCode += byKeyData[byPos++];

	if(ulXFSKeyCode)	//Key Pressed
	{
		//LOG(Info, _T("Key pressed (0x%X) - COUNT (%d)"), ulXFSKeyCode, usKeyCount);

		if(usReasonCode == 0x200)		// FDK
		{
			// MFS Function Key 사양과 동일하게 변경
			if (Is_DIO_Ear_Jack_Insert() == FALSE)
			{
				byPinEmulatedValue =
					(WFS_PIN_FK_FDK01 == ulXFSKeyCode) ? KEYMAP_FDK_R1 :
					(WFS_PIN_FK_FDK02 == ulXFSKeyCode) ? KEYMAP_FDK_R2 :
					(WFS_PIN_FK_FDK03 == ulXFSKeyCode) ? KEYMAP_FDK_R3 :
					(WFS_PIN_FK_FDK04 == ulXFSKeyCode) ? KEYMAP_FDK_R4 :
					(WFS_PIN_FK_FDK05 == ulXFSKeyCode) ? KEYMAP_FDK_L1 :
					(WFS_PIN_FK_FDK06 == ulXFSKeyCode) ? KEYMAP_FDK_L2 :
					(WFS_PIN_FK_FDK07 == ulXFSKeyCode) ? KEYMAP_FDK_L3 :
					(WFS_PIN_FK_FDK08 == ulXFSKeyCode) ? KEYMAP_FDK_L4 :
					0x00;
			}
			else
			{
				// ADA Mode인 경우 FDK 무효화 처리
				LOG(Info, _T("PressKeyHandler - ADA MODE FDK Key Skip"), usKeyCount, usReasonCode);
				byPinEmulatedValue = 0x00;
			}
		}
		else							// EPP Key
		{
			byPinEmulatedValue =	
				(WFS_PIN_FK_0 == ulXFSKeyCode)		? KEYMAP_0 : 
				(WFS_PIN_FK_1 == ulXFSKeyCode)		? KEYMAP_1 :
				(WFS_PIN_FK_2 == ulXFSKeyCode)		? KEYMAP_2 :
				(WFS_PIN_FK_3 == ulXFSKeyCode)		? KEYMAP_3 :
				(WFS_PIN_FK_4 == ulXFSKeyCode)		? KEYMAP_4 :
				(WFS_PIN_FK_5 == ulXFSKeyCode)		? KEYMAP_5 :
				(WFS_PIN_FK_6 == ulXFSKeyCode)		? KEYMAP_6 :
				(WFS_PIN_FK_7 == ulXFSKeyCode)		? KEYMAP_7 :
				(WFS_PIN_FK_8 == ulXFSKeyCode)		? KEYMAP_8 :
				(WFS_PIN_FK_9 == ulXFSKeyCode)		? KEYMAP_9 :
				(WFS_PIN_FK_RES1 == ulXFSKeyCode)	? KEYMAP_LEFT :
				(WFS_PIN_FK_RES2 == ulXFSKeyCode)	? KEYMAP_RIGHT :
				(WFS_PIN_FK_RES3 == ulXFSKeyCode)	? KEYMAP_BLANK :
				(WFS_PIN_FK_ENTER == ulXFSKeyCode)	? KEYMAP_ENTER :
				(WFS_PIN_FK_CLEAR == ulXFSKeyCode)	? KEYMAP_CLEAR :
				(WFS_PIN_FK_CANCEL == ulXFSKeyCode)	? KEYMAP_CANCEL :
				0x00;
		}

		if (byPinEmulatedValue != 0x00)
		{
			if (CSCR_Manager::GetInstance()->GetCurrentScreenNumber() == OP_SCREEN_DEVICE_SOUND)
			{
				// Sound 화면에서 Function Key를 누를때만 Sound 처리 하지 않도록 수정
				if (!(byPinEmulatedValue == KEYMAP_FDK_R1 || byPinEmulatedValue == KEYMAP_FDK_R2 || byPinEmulatedValue == KEYMAP_FDK_R3 || byPinEmulatedValue == KEYMAP_FDK_R4 ||
					byPinEmulatedValue == KEYMAP_FDK_L1 || byPinEmulatedValue == KEYMAP_FDK_L2 || byPinEmulatedValue == KEYMAP_FDK_L3 || byPinEmulatedValue == KEYMAP_FDK_L4))
				{
					PlaySound(CUtil::GetKeyPressWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
				}				
			}
			else
			{
				if (Is_DIO_Ear_Jack_Insert() == FALSE)
				{
					PlaySound(CUtil::GetKeyPressWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
				}
			}

			// Supervisor Mode Check
			if (GetServiceMode() == EAGLE_ATM_SVC_OUT_OF_SERVICE)
			{
				if (Is_DIO_Front_Door_Open() == TRUE)
				{
					if ((KEYMAP_0 <= byPinEmulatedValue) && (byPinEmulatedValue <= KEYMAP_9))
					{
						// 숫자 입력
						m_strKeyValue.Append(CString(byPinEmulatedValue));
					}
					else if (KEYMAP_ENTER == byPinEmulatedValue)
					{
						// ENTER 입력
						if (CEagleDataManager::GetInstance()->m_Config.m_System.strAdministrator_password == m_strKeyValue.Right(6))		// 마지막 6자리 Check
						{
							CSVC_Manager::GetInstance()->SetOperatorMode(EAGLE_ATM_SVC_ADMIN);
						}
						else if (CEagleDataManager::GetInstance()->m_Config.m_System.strMaster_password == m_strKeyValue.Right(6))			// 마지막 6자리 Check
						{
							CSVC_Manager::GetInstance()->SetOperatorMode(EAGLE_ATM_SVC_MASTER);
						}
						else if (CEagleDataManager::GetInstance()->m_Config.m_System.strReplenishment_password == m_strKeyValue.Right(6))	// 마지막 6자리 Check
						{
							CSVC_Manager::GetInstance()->SetOperatorMode(EAGLE_ATM_SVC_REPLENISH);
						}

						m_strKeyValue.Empty();
					}
				}
			}
			else
			{
				// ADA Mode인 경우에는 Key emulation하도록 수정
				CSCR_Manager::GetInstance()->SetKeyEvent(CString(byPinEmulatedValue), Is_DIO_Ear_Jack_Insert());
			}
		}
	}
	else				//No Key Pressed
	{
		//LOG(Info, _T("NO KEY : Count = %d, CODE = %X"), usKeyCount, usReasonCode);

		// Master Key 주입 화면에서만 Screen으로 전달하도록 로직 수정
		if ((m_nInjectKeySequence == START_INJECT_PART_A) || (m_nInjectKeySequence == START_INJECT_PART_B) || (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEntryKeyStatus == ENTRY_KEY_STATUS_ENCRYPT))
		{
			// Secure 및 Pin Mode에서 Key 유효성 Check
			if (usReasonCode == 0x00)
			{
				if (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEntryKeyStatus == ENTRY_KEY_STATUS_ENCRYPT)
				{
					if (Is_DIO_Ear_Jack_Insert() == TRUE)	// ADA Mode에서 Key 입력시 stop wave file 수행
						CDEV_Manager::GetInstance()->StopPlayWaveFile();
				}

				PlaySound(CUtil::GetKeyPressWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
				CSCR_Manager::GetInstance()->SetKeyEvent(_T("0"));	// Normal Key값을 전달
			}
		}
	}

	strTemp.Empty();

	if((usReasonCode & 0x04) == 0x4)
	{
		// Timeout 발생시 다시 한번 Key Entry 수행
		m_bIsEPPTimeout = TRUE;

		if (CSCR_Manager::GetInstance()->GetCurrentScreenNumber() == OP_SCREEN_INJECT_MASTER_KEY)
		{
			strTemp = _T("TIMEOUT RECEIVED IN INJECT MASTER KEY SCREEN - ERROR");

			if (m_nInjectKeySequence == START_INJECT_PART_A || m_nInjectKeySequence == START_INJECT_PART_B)
				CSCR_Manager::GetInstance()->SetKeyEvent(_T("ERROR"));
		}
	}

	if((usReasonCode & 0x08) == 0x08)
	{
		if (CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEntryKeyStatus == ENTRY_KEY_STATUS_ENCRYPT)
		{
			strTemp = _T("AUTO END RECEIVED ON ENTRY_KEY_STATUS_ENCRYPT");

			// Enter Key 발행
			PlaySound(CUtil::GetKeyPressWaveFilePath(), AfxGetInstanceHandle(), SND_ASYNC);
			CSCR_Manager::GetInstance()->SetKeyEvent(CString(KEYMAP_ENTER), Is_DIO_Ear_Jack_Insert());
		}
	}

	if((usReasonCode & 0x01) == 0x01)
	{
		strTemp = _T("MAX DIGITS");

		if (CSCR_Manager::GetInstance()->GetCurrentScreenNumber() == OP_SCREEN_INJECT_MASTER_KEY)
		{
			LOG(Info, _T("MAX DIGITS - INJECT MASTER INPUT MODE"));

			// Auto End Response 수신 시 "Enter" 가상으로 발생 (Key MAnagement 처리를 위해)
			CSCR_Manager::GetInstance()->SetKeyEvent(_T("ENTER"), Is_DIO_Ear_Jack_Insert());
		}
	}

	if((usReasonCode & 0x02) == 0x02)
		strTemp = _T("TERMINATING KEY");

	if (strTemp.IsEmpty() == FALSE)
		LOG(Info, _T("PressKeyHandler - Event Reason (%s)"), strTemp);

	return 0;
}


/** **********************************************************
*	@brief		Cryptera EPP Status 전달
*	@retval		없음
************************************************************/
int CSVC_Manager::StatusEventHandler(unsigned short usParam, BYTE *lpByte, unsigned short usDataLength)
{
	if(usDataLength < 38)
		return -1;

	unsigned short usPos = 0;
	unsigned long ulEventCode = 0;
	unsigned short usLeng = 0;
	unsigned char ActivateState = 0;
	unsigned long ulRKLCertStatus = 0;
	unsigned long ulDeviceStatus = 0;
	unsigned long ulKeyLoadStatus = 0;
	unsigned char EntryModeStatus = 0;
	unsigned long ulCFSStatus = 0;

	char szOptionInfo[128];
	BYTE *pData = new BYTE[usDataLength+1];
	memset(pData, 0, usDataLength+1);
	memcpy(pData, lpByte, usDataLength);

	//0
	if((usPos+6) >= usDataLength)
	{
		delete [] pData;
		return -1;
	}

	usPos++;
	usPos++;
	ulEventCode += pData[usPos++] * 0x1000000;
	ulEventCode += pData[usPos++] * 0x10000;
	ulEventCode += pData[usPos++] * 0x100;
	ulEventCode += pData[usPos++];

	//1
	if((usPos+2) >= usDataLength)
	{
		delete [] pData;
		return -1;
	}
	usLeng += pData[usPos++] * 0x100;
	usLeng += pData[usPos++];
	memset(szOptionInfo, 0, sizeof(szOptionInfo));

	if(usLeng < sizeof(szOptionInfo))
		memcpy(szOptionInfo, &pData[usPos], usLeng);
	else
		memcpy(szOptionInfo, &pData[usPos], sizeof(szOptionInfo)-1);
	usPos += usLeng;

	//2
	if(usPos+3 >= usDataLength)
	{
		delete [] pData;
		return -1;
	}
	usPos++;
	usPos++;
	ActivateState = pData[usPos++];

	//3
	if((usPos+6) >= usDataLength)
	{
		delete [] pData;
		return -1;
	}
	usPos++;
	usPos++;
	ulRKLCertStatus += pData[usPos++] * 0x1000000;
	ulRKLCertStatus += pData[usPos++] * 0x10000;
	ulRKLCertStatus += pData[usPos++] * 0x100;
	ulRKLCertStatus += pData[usPos++];

	//4
	if((usPos+6) >= usDataLength)
	{
		delete [] pData;
		return -1;
	}
	usPos++;
	usPos++;
	ulDeviceStatus += pData[usPos++] * 0x1000000;
	ulDeviceStatus += pData[usPos++] * 0x10000;
	ulDeviceStatus += pData[usPos++] * 0x100;
	ulDeviceStatus += pData[usPos++];

	//5
	if((usPos+6) >= usDataLength)
	{
		delete [] pData;
		return -1;
	}
	usPos++;
	usPos++;
	ulKeyLoadStatus += pData[usPos++] * 0x1000000;
	ulKeyLoadStatus += pData[usPos++] * 0x10000;
	ulKeyLoadStatus += pData[usPos++] * 0x100;
	ulKeyLoadStatus += pData[usPos++];

	//6
	if(usPos+3 >= usDataLength)
	{
		delete [] pData;
		return -1;
	}
	usPos++;
	usPos++;
	EntryModeStatus = pData[usPos++];

	//7
	if((usPos+6) >= usDataLength)
	{
		delete [] pData;
		return -1;
	}
	usPos++;
	usPos++;
	ulCFSStatus += pData[usPos++] * 0x1000000;
	ulCFSStatus += pData[usPos++] * 0x10000;
	ulCFSStatus += pData[usPos++] * 0x100;
	ulCFSStatus += pData[usPos++];

	switch(ulEventCode)
	{
	case 0x81:						//Battery low warning
		LOG(Info, _T("BATTERY LOW WARNING"));
		break;

	case 0x82:						//Stuck key state change; Read response pareameter 5 for info of state
		//Device Status
		if(ulDeviceStatus & 0x01)
		{
			LOG(Info, _T("STUCK ERROR EVENT - DETECT"));
			CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bEPPStuckState = TRUE;
		}
		else
		{
			LOG(Info, _T("STUCK ERROR EVENT - DISAPPEAR"));
			CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bEPPStuckState = FALSE;
		}
		break;

	case 0x83:						//Severe application event
		//Activation State & Device Status
		LOG(Info, _T("ACTIVATE = %X, DEVICE = %08X"), ActivateState & 0xFF, ulDeviceStatus);
		break;

	case 0x84:						//Tamper events
		LOG(Info, _T("TAMPER EVENT DETECTED"));
		break;

	case 0x85:						//Device Activation state change
		//Activation State
		if(ActivateState == EPP_STATE_REMOVED)
		{
			CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEPPState = EPP_STATE_REMOVED;
			LOG(Info, _T("ACIVATION STATUS CHANGED TO REMOVED"));
		}
		else if(ActivateState == EPP_STATE_PREACTIVATED)
		{
			CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEPPState = EPP_STATE_PREACTIVATED;
			LOG(Info, _T("ACIVATION STATUS CHANGED TO PREACTIVATED"));
		}
		else if(ActivateState == EPP_STATE_ACTIVATED)
		{
			CDEV_Manager::GetInstance()->m_DEV_CREPP.m_nEPPState = EPP_STATE_ACTIVATED;
			LOG(Info, _T("ACIVATION STATUS CHANGED TO ACTIVATED"));
		}
		else
		{
			LOG(Info, _T("ACIVATION STATUS CHANGED TO UNKNOWN"));
		}
		break;
	case 0x86:						//Key History Log full
		LOG(Info, _T("KEY HISTORY LOG FULL"));
		break;

	case 0x88:						//Key History Log almost full
		LOG(Info, _T("KEY HISTORY LOG ALMOST FULL"));
		break;

	case 0x89:						//Removal switch state changed
		//Device Status
		if((ulDeviceStatus & 0x08) == 0x08)
		{
			CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bRemovalSensorStatus = TRUE;
			LOG(Error, _T("REMOVAL SWITCH DETECTED"));
		}
		else
		{
			CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bRemovalSensorStatus = FALSE;
			LOG(Error, _T("REMOVAL SWITCH UNDETECTED"));
		}
		break;

	case 0x01:						//Certificate Change
		LOG(Info, _T("CERTIFICATE CHANGED"));
		break;

	case 0x02:						//Initial Key (TMK) Loaded + Name of key register name
		//Optional Info
		LOG(Info, _T("TMK(%s) LOADED"), szOptionInfo);
		break;

	case 0x03:						//PIN Code/Block erased (after PIN use timeout)
		LOG(Info, _T("PIN CODE ERASED"));
		break;

	case 0x04:						//NEW_PIN erased (after use timeout in PIN-Change)
		LOG(Info, _T("NEW PIN CODE ERASED"));
		break;

	case 0x05:						//EPP ready (has just been powered on or rebooted)
		LOG(Info, _T("EPP READY"));
		CDEV_Manager::GetInstance()->m_DEV_CREPP.m_bEPPReady = TRUE;
		break;

	case 0x06:						//Sent out during ETS calibration. Enough touch samples collected.
		break;

	case 0x07:						//Sent out during ETS calibration. Finger has been removed from touch panel.
		break;

	case 0x08:						//Sent out during ETS calibration. Timeout.
		break;

	default:
		LOG(Info, _T("UNKNOWN STATUS EVENT %08X"), ulEventCode);
		break;
	}

	delete [] pData;

	return 0;
}
