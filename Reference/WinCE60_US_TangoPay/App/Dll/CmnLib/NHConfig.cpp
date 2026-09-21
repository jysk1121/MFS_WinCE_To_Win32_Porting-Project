#include "stdafx.h"
#include ".\Common\NHConfig.h"
#include ".\Common\ConstDef.h"	// [#2388] US Justin include Constant

/*-------------------------------------------------
         NH OS Version String Define
---------------------------------------------------*/
#define NHDOVE_OSV			_T("V05.01.xx")
#define NH2700_OSV			_T("V06.01.xx")

#define GET_PANEL_TYPE		1
#define GET_TOUCH_EP_VER	2
#define GET_BOOTLOADER_VER	3

#define PANEL_TOUCH			1
#define PANEL_FUNCTION		2

// [#2518] US Kook 2017.12.21 Support MX-2800SE
#define USB_11_MODE			1
#define USB_20_MODE			2

#define GET_USB_MODE_TYPE	5
#define GET_USB_HUB_VIDPID	6
// end of [#2518]

#ifdef UNDER_CE
#  define UNKNOWN_OS_DEFAULT NH_OS_NH2700_B
#else
//#  define UNKNOWN_OS_DEFAULT NH_OS_NH1800SE_B		// for win32 debug test
#  define UNKNOWN_OS_DEFAULT NH_OS_NH2700_B			// for win32 debug test
#endif

/*---------------------------------------------------------------------------
 CLASS NAME   : CNHConfig
 FUNCTION NAME: CNHConfig()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자
----------------------------------------------------------------------------*/
CNHConfig::CNHConfig()
{
	// Os version
	m_eOSVersion = NH_OS_UNKNOWN;
	m_strTouchPanelVersion = L"N/A";
	m_strBootLoaderVersion = L"N/A";

	//////////////////////////////////
	// Check Touch/Function Driver
	{
		HANDLE hdSFO = CreateFile(L"SFO1:", GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);

//		if (hdSFO != NULL)
		if (hdSFO != INVALID_HANDLE_VALUE)		// [#2022] NH KSK 2011.02.22
		{
			BOOL	bResult = FALSE;
			DWORD	dwOut = 0, dwOutlen = 0;
			char	chVersion[20] = {};
			m_usbMode = 0;

			if (!DeviceIoControl(hdSFO, GET_USB_MODE_TYPE, NULL, 0, &m_usbMode, 1, &dwOutlen, NULL))
			{
				m_usbMode = USB_11_MODE;
				RETAILMSG(1, (L"-- USB 1.1 mode detected :( \n"));
			}

			RETAILMSG(1, (L"-- USB mode = %d \n", m_usbMode));

			if (m_usbMode == USB_20_MODE)
			{
				RETAILMSG(1, (L"-- USB 2.0 mode detected! \n"));
			}

			dwOut = 0;
			dwOutlen = 0;

			RETAILMSG(1, (L"-- TRY TOUCH TYPE\n"));
			bResult = DeviceIoControl(hdSFO, GET_PANEL_TYPE, NULL, 0, &dwOut, 1, &dwOutlen, NULL);
			if (bResult == TRUE)
			{
				RETAILMSG(1, (L"-- SFO1 DIRVER -- out(%X)\n", dwOut));

				if (dwOut == PANEL_TOUCH)
				{
					if (GetSystemMetrics(SM_CYSCREEN) == 1200)	// [#2064] NH KSK 2011.05.18
						m_eOSVersion = NH_OS_NH2700_T_T;			// end of [#2064]
					else
						m_eOSVersion = NH_OS_NH2700_L;
				}
				else if (dwOut == PANEL_FUNCTION)
				{
					RETAILMSG(1, (L"-- PANEL FUNCTION Resolution(%d)\n", GetSystemMetrics(SM_CXSCREEN)));
					// [#11] NH KSK 2010.09.14
					if (GetSystemMetrics(SM_CXSCREEN) == 800)
						m_eOSVersion = NH_OS_NH1800SE_B;
					else if (GetSystemMetrics(SM_CYSCREEN) == 1200)	// [#2064] NH KSK 2011.05.18
						m_eOSVersion = NH_OS_NH2700_F_T;			// end of [#2064]
					else if (GetSystemMetrics(SM_CXSCREEN) == 640)	// [#2267] US KSK 2014.05.08
						m_eOSVersion = NH_OS_NH1500SE;				// end of [#2267]
					else if (GetSystemMetrics(SM_CYSCREEN) == 768)
					{
#if (AU_VERSION)
						m_eOSVersion = NH_OS_MX2800SE;				// [#GLDV-2853] use 2800 as default for AU
#else
						m_eOSVersion = NH_OS_MX5200SE;				// [#2300] US KSK 2014.11.03		// [#2315] US Justin 2014.12.11 Change Machine Name. MX3000 => MX5200
#endif

						// [#2518] US Kook 2017.12.21 Support MX-2800SE
						// detecting MX-2800SE by resolution (1024x768) and USB 2.0 capability with hub.
						if (m_usbMode == USB_20_MODE)
						{
							BYTE outbuf[4];
							bResult = DeviceIoControl(hdSFO, GET_USB_HUB_VIDPID, NULL, 0, outbuf, 4, &dwOutlen, NULL);

							for (unsigned int i = 0; i < dwOutlen; i++)
								RETAILMSG(1, (L"outbuf[%d] = %d\r\n",i, outbuf[i]));

							if (bResult == TRUE && dwOutlen == 4)
							{
								// 당사 USB Hub ( Vendor ID 0x0409, Product ID 0x005a ) 
								// outbuf[0]  Product ID Low  Byte	// 90 (5a)
								// outbuf[1]  Product ID High Byte	// 0
								// outbuf[2]  Vendor  ID Low  Byte	// 9
								// outbuf[3]  Vendor  ID High Byte	// 4
								if (outbuf[0] == 0x5a && outbuf[1] == 0x00
									&& outbuf[2] == 0x09 && outbuf[3] == 0x04)
								{
									RETAILMSG(1, (L"-- USB hub found!\r\n"));
									m_eOSVersion = NH_OS_MX2800SE;
								}
							}
						}
					}
					else if (GetSystemMetrics(SM_CYSCREEN) == 1536)			// [#GLDV-2505] US KOOK 2019.05.13 Support MX2800T
						m_eOSVersion = NH_OS_MX2800_F_T;					// end of [#GLDV-2505]
					else
						m_eOSVersion = NH_OS_NH2700_B;
					// end of [#11]
				}
			}

			// 2018.04.27 remove unused item. ([#2321] NH KSK 2015.01.14)
			//// Get IR Touch EP Version
			//memset(chVersion, 0, sizeof(chVersion));
			//RETAILMSG(1, (L"-- TRY TOUCH EP VERSION\n"));
			//bResult = DeviceIoControl(hdSFO, GET_TOUCH_EP_VER, NULL, 0, chVersion, sizeof(chVersion), &dwOutlen, NULL);
			//if (bResult == TRUE)
			//{
			//	RETAILMSG(1, (L"TOUCH EP VERSION(%S)\n", chVersion));
			//	m_strTouchPanelVersion = chVersion;
			//}
			//else
			//{
			//	RETAILMSG(1, (L"-- TOUCH EP VERSION READ FAIL\n"));
			//}
			// end of 2018.04.27

			// Get BootLoader Version
			RETAILMSG(1, (L"-- TRY BOOTLOADER VERSION\n"));
			bResult = DeviceIoControl(hdSFO, GET_BOOTLOADER_VER, NULL, 0, chVersion, sizeof(chVersion), &dwOutlen, NULL);
			if (bResult == TRUE)
			{
				RETAILMSG(1, (L"BOOT LOADER VERSION(%S)\n", chVersion));
				m_strBootLoaderVersion = chVersion;
			}

			CloseHandle(hdSFO);
		}
		else
		{
			RETAILMSG(1, (L"-- Create File is Invalid\n"));
		}
	}


	//////////////////////////////////
	// Check OS Version
	if (m_eOSVersion == NH_OS_UNKNOWN)
	{
		m_eOSVersion = UNKNOWN_OS_DEFAULT;		// [#2064] NH KSK 2011.05.18 Os Version을 모를 경우 NH2700 BASIC으로 설정
		RETAILMSG(1, (_T("NH_OS_UNKNOWN, set m_eOSVersion to [%d]\n"), (int)m_eOSVersion));
	}

	RETAILMSG(1, (_T("m_eOSVersion [%d]\n"), (int)m_eOSVersion));

	//////////////////////////////////
	// CE Version
	OSVERSIONINFO info;
	memset(&info, 0, sizeof(OSVERSIONINFO));
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	m_eCEVersion = WINCE_6;
	if (GetVersionEx(&info))
	{
		RETAILMSG(1, (_T("OSVERSIONINFO.dwMajorVersion [%d]\n"), info.dwMajorVersion));

		if (info.dwMajorVersion == 7)				m_eCEVersion = WINCE_7;
		else /* if (info.dwMajorVersion == 6)*/		m_eCEVersion = WINCE_6;
	}

	//////////////////////////////////
	// Screen Setting
	SetScreenConfig();
}

/*---------------------------------------------------------------------------
 CLASS NAME   : CNHConfig
 FUNCTION NAME: ~CNHConfig()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자
----------------------------------------------------------------------------*/
CNHConfig::~CNHConfig()
{
}


/*---------------------------------------------------------------------------
 CLASS NAME   : CNHConfig
 FUNCTION NAME: GetOSVersion()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 Install된 OS 정보를 조회한다.
----------------------------------------------------------------------------*/
NH_OS_VERSION CNHConfig::GetOSVersion()
{
	return m_eOSVersion;
}

/*---------------------------------------------------------------------------
 CLASS NAME   : CNHConfig
 FUNCTION NAME: GetBootLoaderVersion()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : BOOT Loader의 버전을 조회한다.
----------------------------------------------------------------------------*/
CString CNHConfig::GetBootLoaderVersion()
{
	return m_strBootLoaderVersion;
}

/*---------------------------------------------------------------------------
 CLASS NAME   : CNHConfig
 FUNCTION NAME: GetTouchPanelVersion()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : TOUCH PANEL의 버전을 조회한다.
----------------------------------------------------------------------------*/
CString CNHConfig::GetTouchPanelVersion()
{
	return m_strTouchPanelVersion;
}

/*---------------------------------------------------------------------------
 CLASS NAME   : CNHConfig
 FUNCTION NAME: GetMachineType()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : OS Version에 맞는 Machine을 조회한다.
----------------------------------------------------------------------------*/
CString CNHConfig::GetMachineType()
{
	CString strMachineType;

	switch (m_eOSVersion)
	{
	case NH_OS_NH2700_L:		strMachineType = _T("NH2700L");		break;	// KSK 2011.05.31 space 제거
	case NH_OS_NH2700_F_T:
	case NH_OS_NH2700_T_T:
								strMachineType = _T("NH2700T");		break;	// KSK 2011.05.31 space 제거
	// [#11] NH KSK 2010.09.14
	case NH_OS_NH2700_B:		strMachineType = _T("NH2700");		break;
	case NH_OS_NH1800SE_B:		strMachineType = _T("NH1800SE");	break;
	case NH_OS_NH1500SE:		strMachineType = _T("NH1500SE");	break;	// [#2267] NH KSK 2014.05.08
	case NH_OS_MX5200SE:		strMachineType = _T("MX5200SE");	break;	// [#2300] US KSK 2014.11.03		// [#2315] US Justin 2014.12.11 Change Machine Name MX3000 ==> MX5200SE
	case NH_OS_MX2800SE:		strMachineType = _T("MX2800SE");	break;	// [#2518] US Kook 2017.12.21 Support MS-2800SE
	case NH_OS_MX2800_F_T:		strMachineType = _T("MX2800T");		break;	// [#GLDV-2505] US Kook 2019.05.13 Support MX-2800T
	default:					strMachineType = _T("UNKNOWN");		break;
	// end of [#11]
	}

	return strMachineType;
}

/*---------------------------------------------------------------------------
 CLASS NAME   : CNHConfig
 FUNCTION NAME: GetScreenConfig()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Screen 설정 정보를 조회한다.
----------------------------------------------------------------------------*/
NH_SCR_CONFIG CNHConfig::GetScreenConfig(NH_SCR_TYPE eScreenType)
{
	if (eScreenType < 0 || eScreenType > SCR_LOCATION_MAX)
		return m_eScreenConfig[SCR_FRONT];

	return m_eScreenConfig[eScreenType];
}

/**
 * Returns the version of the WinCE platform
 */
NH_CE_VERSION CNHConfig::GetCEVersion()
{
	return m_eCEVersion;
}

/**
* Returns the directory where files to be copied are located
*/
CString CNHConfig::GetUpdatePath(BASEDIR_TYPE eBaseDir)
{
	CString strUpdateDir;

	// e.g.) \USB
	switch (eBaseDir)
	{
	case BASEDIR_USB:			strUpdateDir = USB_PATH;				break;
	case BASEDIR_SDCARD:		strUpdateDir = SDCARD_PATH;				break;
	case BASEDIR_ROOT:
	default:					strUpdateDir = _T("");					break;	// root path is included in UPDATE_BASEDIR_60/70
	}

	// e.g.) \USB\UPDATE\NH2700CE	or	\USB\UPDATE7
	switch (m_eCEVersion)
	{
	case WINCE_7:				strUpdateDir += UPDATE_BASEDIR_70;		break;
	case WINCE_6:				
	default:					strUpdateDir += UPDATE_BASEDIR_60;		break;
	}

	return strUpdateDir;
}

/**
 * Returns true when the Camera upkit is supported
 */
bool CNHConfig::SupportsCamera()
{
	return m_usbMode == USB_20_MODE;
}

/*---------------------------------------------------------------------------
 CLASS NAME   : CNHConfig
 FUNCTION NAME: GetRegValue()
 RETURN TYPE  : 주어진 Value에 할당된 Value
 PARAMETER    : hKey : HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_CLASSES_ROOT 중에 한 값.
				lpszKeyName : 조회 하고자 하는 Value가 속한 Key 이름
				lpszValueName : 조회 하고자 하는 Value 이름
				lpType : 조회 하고자 하는 Value의 Type (REG_SZ, REG_DWORD, REG_BINARY ..)
				lpData : 주어진 버퍼
				lpcbData : 주어진 버퍼에 채워진 데이터 크기
 DESCRIPTION  : 주어진 Key의 String 값을 조회한다.
----------------------------------------------------------------------------*/
BOOL CNHConfig::GetRegValue(HKEY hKey, LPCTSTR lpszKeyName, LPCTSTR lpszValueName, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	if (RegOpenKeyEx(hKey, lpszKeyName, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		// [#2022] NH KSK 2011.02.22 Code Sonar 지적사항 대책
		if (RegQueryValueEx(hKey, lpszValueName, 0, lpType, lpData, lpcbData) == ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return TRUE;
		}
		else
		{
			RegCloseKey(hKey);
		}
		// end of [#2022]
	}
	
	return FALSE;
}

/*---------------------------------------------------------------------------
 CLASS NAME   : CNHConfig
 FUNCTION NAME: SetScreenConfig()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Model에 따른 Screen Setting을 수행
----------------------------------------------------------------------------*/
void CNHConfig::SetScreenConfig()
{
	switch(m_eOSVersion)
	{
	case NH_OS_NH2700_L:	// iTM Luxury Model
		{
			RETAILMSG(1, (L"\nNH_OS_NH2700_L\n"));
			////////////////////////////
			// SCR_FRONT Setting
			m_eScreenConfig[SCR_FRONT].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_FRONT].bCreateEngine = TRUE;					// Default
			m_eScreenConfig[SCR_FRONT].strLoadPath = SCREEN_PATH;				// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_FRONT].strLoadSection = _T("FRONT_TOUCH");
			m_eScreenConfig[SCR_FRONT].nX = 0;
			m_eScreenConfig[SCR_FRONT].nY = 0;
			m_eScreenConfig[SCR_FRONT].nWidth = 1024;
			m_eScreenConfig[SCR_FRONT].nHeight = 600;
			m_eScreenConfig[SCR_FRONT].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_FRONT].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_REAR Setting
			m_eScreenConfig[SCR_REAR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_REAR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_REAR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_REAR].strLoadSection = _T("REAR_TOUCH");
			m_eScreenConfig[SCR_REAR].nX = 0;
			m_eScreenConfig[SCR_REAR].nY = 0;
			m_eScreenConfig[SCR_REAR].nWidth = 1024;
			m_eScreenConfig[SCR_REAR].nHeight = 600;
			m_eScreenConfig[SCR_REAR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_REAR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_KEYMGR Setting
			m_eScreenConfig[SCR_KEYMGR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_KEYMGR].bCreateEngine = FALSE;
			m_eScreenConfig[SCR_KEYMGR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_KEYMGR].strLoadSection = _T("KEYMGR_TOUCH");
			m_eScreenConfig[SCR_KEYMGR].nX = 0;
			m_eScreenConfig[SCR_KEYMGR].nY = 0;
			m_eScreenConfig[SCR_KEYMGR].nWidth = 1024;
			m_eScreenConfig[SCR_KEYMGR].nHeight = 600;
			m_eScreenConfig[SCR_KEYMGR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_KEYMGR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_UPDATE Setting
			m_eScreenConfig[SCR_UPDATE].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_UPDATE].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_UPDATE].strLoadPath = _T("");
			m_eScreenConfig[SCR_UPDATE].strLoadSection = _T("UPDATE");
			m_eScreenConfig[SCR_UPDATE].nX = 0;
			m_eScreenConfig[SCR_UPDATE].nY = 0;
			m_eScreenConfig[SCR_UPDATE].nWidth = 1024;
			m_eScreenConfig[SCR_UPDATE].nHeight = 600;
			m_eScreenConfig[SCR_UPDATE].strSDUpdateSymbol = GetUpdatePath(BASEDIR_SDCARD);
			m_eScreenConfig[SCR_UPDATE].strUSBUpdateSymbol = GetUpdatePath(BASEDIR_USB);
		}
		break;

	case NH_OS_NH2700_F_T:	// iTM F/K TTW Model [#2064] NH KSK 2011.05.18
		{
			RETAILMSG(1, (L"\nNH_OS_NH2700_F_T\n"));
			////////////////////////////
			// SCR_FRONT Setting
			m_eScreenConfig[SCR_FRONT].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_FRONT].bCreateEngine = TRUE;					// Default
			m_eScreenConfig[SCR_FRONT].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_FRONT].strLoadSection = _T("FRONT_FUNCTION");	// [#2064] NH KSK 2011.05.18
			m_eScreenConfig[SCR_FRONT].nX = 0;
			m_eScreenConfig[SCR_FRONT].nY = 0;
			m_eScreenConfig[SCR_FRONT].nWidth = 1024;
			m_eScreenConfig[SCR_FRONT].nHeight = 600;
			m_eScreenConfig[SCR_FRONT].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_FRONT].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_REAR Setting
			m_eScreenConfig[SCR_REAR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_REAR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_REAR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_REAR].strLoadSection = _T("REAR_TOUCH");
			m_eScreenConfig[SCR_REAR].nX = 0;
			m_eScreenConfig[SCR_REAR].nY = 720;
			m_eScreenConfig[SCR_REAR].nWidth = 720;
			m_eScreenConfig[SCR_REAR].nHeight = 480;
			m_eScreenConfig[SCR_REAR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_REAR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_KEYMGR Setting
			m_eScreenConfig[SCR_KEYMGR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_KEYMGR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_KEYMGR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_KEYMGR].strLoadSection = _T("KEYMGR_FUNCTION");	// [#2064] NH KSK 2011.05.18
			m_eScreenConfig[SCR_KEYMGR].nX = 0;
			m_eScreenConfig[SCR_KEYMGR].nY = 0;
			m_eScreenConfig[SCR_KEYMGR].nWidth = 1024;
			m_eScreenConfig[SCR_KEYMGR].nHeight = 600;
			m_eScreenConfig[SCR_KEYMGR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_KEYMGR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_UPDATE Setting
			m_eScreenConfig[SCR_UPDATE].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_UPDATE].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_UPDATE].strLoadPath = _T("");
			m_eScreenConfig[SCR_UPDATE].strLoadSection = _T("UPDATE");
			m_eScreenConfig[SCR_UPDATE].nX = 0;
			m_eScreenConfig[SCR_UPDATE].nY = 720;
			m_eScreenConfig[SCR_UPDATE].nWidth = 720;
			m_eScreenConfig[SCR_UPDATE].nHeight = 480;
			m_eScreenConfig[SCR_UPDATE].strSDUpdateSymbol = GetUpdatePath(BASEDIR_SDCARD);
			m_eScreenConfig[SCR_UPDATE].strUSBUpdateSymbol = GetUpdatePath(BASEDIR_USB);
		}
		break;

	case NH_OS_NH2700_T_T:	// iTM F/KK TTW Model [#2064] NH KSK 2011.05.18
		{
			RETAILMSG(1, (L"\nNH_OS_NH2700_T_T\n"));
			////////////////////////////
			// SCR_FRONT Setting
			m_eScreenConfig[SCR_FRONT].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_FRONT].bCreateEngine = TRUE;					// Default
			m_eScreenConfig[SCR_FRONT].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_FRONT].strLoadSection = _T("FRONT_TOUCH");
			m_eScreenConfig[SCR_FRONT].nX = 0;
			m_eScreenConfig[SCR_FRONT].nY = 0;
			m_eScreenConfig[SCR_FRONT].nWidth = 1024;
			m_eScreenConfig[SCR_FRONT].nHeight = 600;
			m_eScreenConfig[SCR_FRONT].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_FRONT].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_REAR Setting
			m_eScreenConfig[SCR_REAR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_REAR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_REAR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_REAR].strLoadSection = _T("REAR_TOUCH");
			m_eScreenConfig[SCR_REAR].nX = 0;
			m_eScreenConfig[SCR_REAR].nY = 720;
			m_eScreenConfig[SCR_REAR].nWidth = 720;
			m_eScreenConfig[SCR_REAR].nHeight = 480;
			m_eScreenConfig[SCR_REAR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_REAR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_KEYMGR Setting
			m_eScreenConfig[SCR_KEYMGR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_KEYMGR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_KEYMGR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_KEYMGR].strLoadSection = _T("KEYMGR_TOUCH");
			m_eScreenConfig[SCR_KEYMGR].nX = 0;
			m_eScreenConfig[SCR_KEYMGR].nY = 0;
			m_eScreenConfig[SCR_KEYMGR].nWidth = 1024;
			m_eScreenConfig[SCR_KEYMGR].nHeight = 600;
			m_eScreenConfig[SCR_KEYMGR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_KEYMGR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_UPDATE Setting
			m_eScreenConfig[SCR_UPDATE].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_UPDATE].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_UPDATE].strLoadPath = _T("");
			m_eScreenConfig[SCR_UPDATE].strLoadSection = _T("UPDATE");
			m_eScreenConfig[SCR_UPDATE].nX = 0;
			m_eScreenConfig[SCR_UPDATE].nY = 720;
			m_eScreenConfig[SCR_UPDATE].nWidth = 720;
			m_eScreenConfig[SCR_UPDATE].nHeight = 480;
			m_eScreenConfig[SCR_UPDATE].strSDUpdateSymbol = GetUpdatePath(BASEDIR_SDCARD);
			m_eScreenConfig[SCR_UPDATE].strUSBUpdateSymbol = GetUpdatePath(BASEDIR_USB);
		}
		break;

	// [#11] NH KSK 2010.09.14
	case NH_OS_NH1800SE_B:
		RETAILMSG(1, (L"\nNH_OS_NH1800SE\n"));
		////////////////////////////
		// SCR_FRONT Setting
		m_eScreenConfig[SCR_FRONT].eEngine = SCR_ENG_PICASSO;
		m_eScreenConfig[SCR_FRONT].bCreateEngine = TRUE;					// Default
		m_eScreenConfig[SCR_FRONT].strLoadPath = SCREEN_PATH;
		m_eScreenConfig[SCR_FRONT].strLoadSection = _T("1800SE_FRONT_FUNCTION");
		m_eScreenConfig[SCR_FRONT].nX = 0;
		m_eScreenConfig[SCR_FRONT].nY = 0;
		m_eScreenConfig[SCR_FRONT].nWidth = 800;
		m_eScreenConfig[SCR_FRONT].nHeight = 600;
		m_eScreenConfig[SCR_FRONT].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
		m_eScreenConfig[SCR_FRONT].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

		////////////////////////////
		// SCR_REAR Setting
		m_eScreenConfig[SCR_REAR].eEngine = SCR_ENG_PICASSO;
		m_eScreenConfig[SCR_REAR].bCreateEngine = FALSE;
		m_eScreenConfig[SCR_REAR].strLoadPath = SCREEN_PATH;
		m_eScreenConfig[SCR_REAR].strLoadSection = _T("1800SE_REAR_FUNCTION");
		m_eScreenConfig[SCR_REAR].nX = 0;
		m_eScreenConfig[SCR_REAR].nY = 0;
		m_eScreenConfig[SCR_REAR].nWidth = 800;
		m_eScreenConfig[SCR_REAR].nHeight = 600;
		m_eScreenConfig[SCR_REAR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
		m_eScreenConfig[SCR_REAR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

		////////////////////////////
		// SCR_KEYMGR Setting
		m_eScreenConfig[SCR_KEYMGR].eEngine = SCR_ENG_PICASSO;
		m_eScreenConfig[SCR_KEYMGR].bCreateEngine = FALSE;
		m_eScreenConfig[SCR_KEYMGR].strLoadPath = SCREEN_PATH;
		m_eScreenConfig[SCR_KEYMGR].strLoadSection = _T("1800SE_KEYMGR_FUNCTION");
		m_eScreenConfig[SCR_KEYMGR].nX = 0;
		m_eScreenConfig[SCR_KEYMGR].nY = 0;
		m_eScreenConfig[SCR_KEYMGR].nWidth = 800;
		m_eScreenConfig[SCR_KEYMGR].nHeight = 600;
		m_eScreenConfig[SCR_KEYMGR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
		m_eScreenConfig[SCR_KEYMGR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

		////////////////////////////
		// SCR_UPDATE Setting
		m_eScreenConfig[SCR_UPDATE].eEngine = SCR_ENG_PICASSO;
		m_eScreenConfig[SCR_UPDATE].bCreateEngine = FALSE;
		m_eScreenConfig[SCR_UPDATE].strLoadPath = _T("");
		m_eScreenConfig[SCR_UPDATE].strLoadSection = _T("UPDATE");
		m_eScreenConfig[SCR_UPDATE].nX = 0;
		m_eScreenConfig[SCR_UPDATE].nY = 0;
		m_eScreenConfig[SCR_UPDATE].nWidth = 800;
		m_eScreenConfig[SCR_UPDATE].nHeight = 600;
		m_eScreenConfig[SCR_UPDATE].strSDUpdateSymbol = GetUpdatePath(BASEDIR_SDCARD);
		m_eScreenConfig[SCR_UPDATE].strUSBUpdateSymbol = GetUpdatePath(BASEDIR_USB);
		break;
	// end of [#11]

	case NH_OS_NH1500SE:		// [#2267] US KSK 2014.05.08
		{
			RETAILMSG(1, (L"\nNH_OS_NH1500SE\n"));
			////////////////////////////
			// SCR_FRONT Setting
			m_eScreenConfig[SCR_FRONT].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_FRONT].bCreateEngine = TRUE;					// Default
			m_eScreenConfig[SCR_FRONT].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_FRONT].strLoadSection = _T("1500_FRONT_FUNCTION");
			m_eScreenConfig[SCR_FRONT].nX = 0;
			m_eScreenConfig[SCR_FRONT].nY = 0;
			m_eScreenConfig[SCR_FRONT].nWidth = 640;
			m_eScreenConfig[SCR_FRONT].nHeight = 480;
			m_eScreenConfig[SCR_FRONT].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_FRONT].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_REAR Setting
			m_eScreenConfig[SCR_REAR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_REAR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_REAR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_REAR].strLoadSection = _T("1500_REAR_FUNCTION");
			m_eScreenConfig[SCR_REAR].nX = 0;
			m_eScreenConfig[SCR_REAR].nY = 0;
			m_eScreenConfig[SCR_REAR].nWidth = 640;
			m_eScreenConfig[SCR_REAR].nHeight = 480;
			m_eScreenConfig[SCR_REAR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_REAR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_KEYMGR Setting
			m_eScreenConfig[SCR_KEYMGR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_KEYMGR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_KEYMGR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_KEYMGR].strLoadSection = _T("1500_KEYMGR_FUNCTION");
			m_eScreenConfig[SCR_KEYMGR].nX = 0;
			m_eScreenConfig[SCR_KEYMGR].nY = 0;
			m_eScreenConfig[SCR_KEYMGR].nWidth = 640;
			m_eScreenConfig[SCR_KEYMGR].nHeight = 480;
			m_eScreenConfig[SCR_KEYMGR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_KEYMGR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_UPDATE Setting
			m_eScreenConfig[SCR_UPDATE].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_UPDATE].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_UPDATE].strLoadPath = _T("");
			m_eScreenConfig[SCR_UPDATE].strLoadSection = _T("UPDATE");
			m_eScreenConfig[SCR_UPDATE].nX = 0;
			m_eScreenConfig[SCR_UPDATE].nY = 0;
			m_eScreenConfig[SCR_UPDATE].nWidth = 640;
			m_eScreenConfig[SCR_UPDATE].nHeight = 480;
			m_eScreenConfig[SCR_UPDATE].strSDUpdateSymbol = GetUpdatePath(BASEDIR_SDCARD);
			m_eScreenConfig[SCR_UPDATE].strUSBUpdateSymbol = GetUpdatePath(BASEDIR_USB);
		}
		break;			// end of [#2267] -> 1800 및 2100T는 추후 지원 예정

	case NH_OS_MX5200SE:		// [#2300] US KSK 2014.11.03		// [#2315] US Justin 2014.12.11 Name change MX3000 => MX5200
		{
			RETAILMSG(1, (L"\nNH_OS_MX5200SE\n"));
			////////////////////////////
			// SCR_FRONT Setting
			m_eScreenConfig[SCR_FRONT].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_FRONT].bCreateEngine = TRUE;					// Default
			m_eScreenConfig[SCR_FRONT].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_FRONT].strLoadSection = _T("5200_FRONT_FUNCTION");			// [#2315] US Justin 2014.12.11 Change Machine Name. MX3000 => MX5200
			m_eScreenConfig[SCR_FRONT].nX = 0;
			m_eScreenConfig[SCR_FRONT].nY = 0;
			m_eScreenConfig[SCR_FRONT].nWidth = 1024;
			m_eScreenConfig[SCR_FRONT].nHeight = 768;
			m_eScreenConfig[SCR_FRONT].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_FRONT].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_REAR Setting
			m_eScreenConfig[SCR_REAR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_REAR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_REAR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_REAR].strLoadSection = _T("5200_REAR_FUNCTION");			// [#2315] US Justin 2014.12.11 Change Machine Name. MX3000 => MX5200
			m_eScreenConfig[SCR_REAR].nX = 0;
			m_eScreenConfig[SCR_REAR].nY = 0;
			m_eScreenConfig[SCR_REAR].nWidth = 1024;
			m_eScreenConfig[SCR_REAR].nHeight = 768;
			m_eScreenConfig[SCR_REAR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_REAR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_KEYMGR Setting
			m_eScreenConfig[SCR_KEYMGR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_KEYMGR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_KEYMGR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_KEYMGR].strLoadSection = _T("5200_KEYMGR_FUNCTION");		// [#2315] US Justin 2014.12.11 Change Machine Name. MX3000 => MX5200
			m_eScreenConfig[SCR_KEYMGR].nX = 0;
			m_eScreenConfig[SCR_KEYMGR].nY = 0;
			m_eScreenConfig[SCR_KEYMGR].nWidth = 1024;
			m_eScreenConfig[SCR_KEYMGR].nHeight = 768;
			m_eScreenConfig[SCR_KEYMGR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_KEYMGR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_UPDATE Setting
			m_eScreenConfig[SCR_UPDATE].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_UPDATE].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_UPDATE].strLoadPath = _T("");
			m_eScreenConfig[SCR_UPDATE].strLoadSection = _T("UPDATE");
			m_eScreenConfig[SCR_UPDATE].nX = 0;
			m_eScreenConfig[SCR_UPDATE].nY = 0;
			m_eScreenConfig[SCR_UPDATE].nWidth = 1024;
			m_eScreenConfig[SCR_UPDATE].nHeight = 768;
			m_eScreenConfig[SCR_UPDATE].strSDUpdateSymbol = GetUpdatePath(BASEDIR_SDCARD);
			m_eScreenConfig[SCR_UPDATE].strUSBUpdateSymbol = GetUpdatePath(BASEDIR_USB);
		}
		break;			// end of [#2300]

		// [#2518] US Kook 2018.01.18 Support MX-2800SE
	case NH_OS_MX2800SE:		// [#2518] US Kook 2017.12.21 Support MX-2800SE
		{
			RETAILMSG(1, (L"\nNH_OS_MX2800SE\n"));
			////////////////////////////
			// SCR_FRONT Setting
			m_eScreenConfig[SCR_FRONT].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_FRONT].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_FRONT].strLoadPath = SCREEN_PATH;
			m_eScreenConfig[SCR_FRONT].strLoadSection = _T("2800_FRONT_FUNCTION");
			m_eScreenConfig[SCR_FRONT].nX = 0;
			m_eScreenConfig[SCR_FRONT].nY = 0;
			m_eScreenConfig[SCR_FRONT].nWidth = 1024;
			m_eScreenConfig[SCR_FRONT].nHeight = 768;
			m_eScreenConfig[SCR_FRONT].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_FRONT].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_REAR Setting
			m_eScreenConfig[SCR_REAR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_REAR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_REAR].strLoadPath = SCREEN_PATH;
			m_eScreenConfig[SCR_REAR].strLoadSection = _T("2800_REAR_FUNCTION");
			m_eScreenConfig[SCR_REAR].nX = 0;
			m_eScreenConfig[SCR_REAR].nY = 0;
			m_eScreenConfig[SCR_REAR].nWidth = 1024;
			m_eScreenConfig[SCR_REAR].nHeight = 768;
			m_eScreenConfig[SCR_REAR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_REAR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_KEYMGR Setting
			m_eScreenConfig[SCR_KEYMGR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_KEYMGR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_KEYMGR].strLoadPath = SCREEN_PATH;
			m_eScreenConfig[SCR_KEYMGR].strLoadSection = _T("2800_KEYMGR_FUNCTION");
			m_eScreenConfig[SCR_KEYMGR].nX = 0;
			m_eScreenConfig[SCR_KEYMGR].nY = 0;
			m_eScreenConfig[SCR_KEYMGR].nWidth = 1024;
			m_eScreenConfig[SCR_KEYMGR].nHeight = 768;
			m_eScreenConfig[SCR_KEYMGR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_KEYMGR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_UPDATE Setting
			m_eScreenConfig[SCR_UPDATE].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_UPDATE].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_UPDATE].strLoadPath = _T("");
			m_eScreenConfig[SCR_UPDATE].strLoadSection = _T("UPDATE");
			m_eScreenConfig[SCR_UPDATE].nX = 0;
			m_eScreenConfig[SCR_UPDATE].nY = 0;
			m_eScreenConfig[SCR_UPDATE].nWidth = 1024;
			m_eScreenConfig[SCR_UPDATE].nHeight = 768;
			m_eScreenConfig[SCR_UPDATE].strSDUpdateSymbol = GetUpdatePath(BASEDIR_SDCARD);
			m_eScreenConfig[SCR_UPDATE].strUSBUpdateSymbol = GetUpdatePath(BASEDIR_USB);
		}
		break;
		// end of [#2518]

		// [#GLDV-2505] US Kook 2019.05.13 Support MX2800T
	case NH_OS_MX2800_F_T:
		{
			RETAILMSG(1, (L"\nNH_OS_MX2800_F_T\n"));
			////////////////////////////
			// SCR_FRONT Setting
			m_eScreenConfig[SCR_FRONT].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_FRONT].bCreateEngine = TRUE;					// Default
			m_eScreenConfig[SCR_FRONT].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_FRONT].strLoadSection = _T("2800_FRONT_FUNCTION");
			m_eScreenConfig[SCR_FRONT].nX = 0;
			m_eScreenConfig[SCR_FRONT].nY = 0;
			m_eScreenConfig[SCR_FRONT].nWidth = 1024;
			m_eScreenConfig[SCR_FRONT].nHeight = 768;
			m_eScreenConfig[SCR_FRONT].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_FRONT].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_REAR Setting
			m_eScreenConfig[SCR_REAR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_REAR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_REAR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_REAR].strLoadSection = _T("REAR_TOUCH");
			m_eScreenConfig[SCR_REAR].nX = 0;
			m_eScreenConfig[SCR_REAR].nY = 1056;
			m_eScreenConfig[SCR_REAR].nWidth = 720;
			m_eScreenConfig[SCR_REAR].nHeight = 480;
			m_eScreenConfig[SCR_REAR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_REAR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_KEYMGR Setting
			m_eScreenConfig[SCR_KEYMGR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_KEYMGR].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_KEYMGR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_KEYMGR].strLoadSection = _T("2800_KEYMGR_FUNCTION");
			m_eScreenConfig[SCR_KEYMGR].nX = 0;
			m_eScreenConfig[SCR_KEYMGR].nY = 0;
			m_eScreenConfig[SCR_KEYMGR].nWidth = 1024;
			m_eScreenConfig[SCR_KEYMGR].nHeight = 768;
			m_eScreenConfig[SCR_KEYMGR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_KEYMGR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_UPDATE Setting
			m_eScreenConfig[SCR_UPDATE].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_UPDATE].bCreateEngine = TRUE;
			m_eScreenConfig[SCR_UPDATE].strLoadPath = _T("");
			m_eScreenConfig[SCR_UPDATE].strLoadSection = _T("UPDATE");
			m_eScreenConfig[SCR_UPDATE].nX = 0;
			m_eScreenConfig[SCR_UPDATE].nY = 1056;
			m_eScreenConfig[SCR_UPDATE].nWidth = 720;
			m_eScreenConfig[SCR_UPDATE].nHeight = 480;
			m_eScreenConfig[SCR_UPDATE].strSDUpdateSymbol = GetUpdatePath(BASEDIR_SDCARD);
			m_eScreenConfig[SCR_UPDATE].strUSBUpdateSymbol = GetUpdatePath(BASEDIR_USB);
		}
		break;
		// end of [#GLDV-2505]

	case NH_OS_NH2700_B:	// iTM Basic Model
	default:				// Default Model
		{
			RETAILMSG(1, (L"\nNH_OS_NH2700_B\n"));
			////////////////////////////
			// SCR_FRONT Setting
			m_eScreenConfig[SCR_FRONT].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_FRONT].bCreateEngine = TRUE;					// Default
			m_eScreenConfig[SCR_FRONT].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_FRONT].strLoadSection = _T("FRONT_FUNCTION");
			m_eScreenConfig[SCR_FRONT].nX = 0;
			m_eScreenConfig[SCR_FRONT].nY = 0;
			m_eScreenConfig[SCR_FRONT].nWidth = 1024;
			m_eScreenConfig[SCR_FRONT].nHeight = 600;
			m_eScreenConfig[SCR_FRONT].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_FRONT].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_REAR Setting
			m_eScreenConfig[SCR_REAR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_REAR].bCreateEngine = FALSE;
			m_eScreenConfig[SCR_REAR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_REAR].strLoadSection = _T("REAR_FUNCTION");
			m_eScreenConfig[SCR_REAR].nX = 0;
			m_eScreenConfig[SCR_REAR].nY = 0;
			m_eScreenConfig[SCR_REAR].nWidth = 1024;
			m_eScreenConfig[SCR_REAR].nHeight = 600;
			m_eScreenConfig[SCR_REAR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_REAR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_KEYMGR Setting
			m_eScreenConfig[SCR_KEYMGR].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_KEYMGR].bCreateEngine = FALSE;
			m_eScreenConfig[SCR_KEYMGR].strLoadPath = SCREEN_PATH;	// [#11] NH KSK 2010.09.14
			m_eScreenConfig[SCR_KEYMGR].strLoadSection = _T("KEYMGR_FUNCTION");
			m_eScreenConfig[SCR_KEYMGR].nX = 0;
			m_eScreenConfig[SCR_KEYMGR].nY = 0;
			m_eScreenConfig[SCR_KEYMGR].nWidth = 1024;
			m_eScreenConfig[SCR_KEYMGR].nHeight = 600;
			m_eScreenConfig[SCR_KEYMGR].strSDUpdateSymbol = L"__**XXXX--";		// Not Support
			m_eScreenConfig[SCR_KEYMGR].strUSBUpdateSymbol = L"__**XXXX--";		// Not Support

			////////////////////////////
			// SCR_UPDATE Setting
			m_eScreenConfig[SCR_UPDATE].eEngine = SCR_ENG_PICASSO;
			m_eScreenConfig[SCR_UPDATE].bCreateEngine = FALSE;
			m_eScreenConfig[SCR_UPDATE].strLoadPath = _T("");
			m_eScreenConfig[SCR_UPDATE].strLoadSection = _T("UPDATE");
			m_eScreenConfig[SCR_UPDATE].nX = 0;
			m_eScreenConfig[SCR_UPDATE].nY = 0;
			m_eScreenConfig[SCR_UPDATE].nWidth = 1024;
			m_eScreenConfig[SCR_UPDATE].nHeight = 600;
			m_eScreenConfig[SCR_UPDATE].strSDUpdateSymbol = GetUpdatePath(BASEDIR_SDCARD);
			m_eScreenConfig[SCR_UPDATE].strUSBUpdateSymbol = GetUpdatePath(BASEDIR_USB);
		}
		break;
	}
}