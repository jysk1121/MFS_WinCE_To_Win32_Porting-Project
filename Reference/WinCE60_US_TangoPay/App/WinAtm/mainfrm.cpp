// MainFrm.cpp : implementation of the CMainFrame class

#include "stdafx.h"
#include "WinAtm.h"

#include ".\Dev\DiskManager.h"
#include "UpdateSchedulerThread.hpp"
#if (APP_LIBERTYX)
#  include "LibertyXHeartbeatThread.hpp"
#endif
#include ".\Dev\LoginManager.h"

//#define DEV_SIMULATION		// 2018.07.25 Device 시뮬레이션 Registry 설정
//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

#include "MainFrm.h"
#include "..\H\Dll\AssetArchive.h"

const DWORD dwAdornmentFlags = 0; // exit button

////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

/////////////////////////////////////////////////////////////////////////////
// AXEvent map : AXINAX

BEGIN_EVENTSINK_MAP(CMainFrame, CFrameWnd)
    //{{AFX_EVENTSINK_MAP(CMainFrame)
	ON_EVENT(CMainFrame, IDC_NHMWI, 3 /* OnDisplayData */, OnDisplayData, VTS_BSTR VTS_BSTR)
	ON_EVENT(CMainFrame, IDC_NHMWI, 4 /* OnDisplayPinpadData */, OnDisplayPinpadData, VTS_BSTR)
	ON_EVENT(CMainFrame, IDC_NHMWI, 5 /* OnSafeDoorChanged */, OnSafeDoorChanged, VTS_BSTR)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
//	m_pOwner	= NULL;

	m_pDevCmn	= NULL;
	m_pTranCmn	= NULL;
//	m_pNetWork	= NULL;
	m_pRmsCtrl  = NULL;

// [#540] HWANG 2008.07.01
	m_pDiagMcu	= NULL;
	m_pDiagCdu	= NULL;
	m_pDiagSpr	= NULL;
	m_pDiagRfid	= NULL;			// [#2325] NH KSK 2015.01.25
	m_pDiagBcr	= NULL;			// [#GLDV-3005] US Kook 2021.10.22
	m_pDiagBna	= NULL;			// [#GLDV-3005] US Kook 2021.10.22

	m_bDiagMCUOpen = FALSE;
	m_bDiagSPROpen = FALSE;
	m_bDiagCDUOpen = FALSE;
	m_bDiagRFIDOpen = FALSE;	// [#2325] NH KSK 2015.01.25
	m_bDiagBCROpen = FALSE;		// [#GLDV-3005] US Kook 2021.10.22
	m_bDiagBNAOpen = FALSE;		// [#GLDV-3005] US Kook 2021.10.22
// end of [#540]
	
	for (int i = 0; i < SCR_LOCATION_MAX; i++)
		m_arScrCtrl[i] = NULL;

	nLastUploadedJNLCount = 0;				// [#613] AU_C KSK 2010.01.18

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	m_nPINInputCount = 0;								// [#250] NZ AIREAT 2008.06.02
#endif

	bNeedRebootOnInit = FALSE;				// [#2186] US KMK 2013.05.06 Reboot Flag

#ifndef NO_REMOTE_UPDATE
	m_updateConfiguration = NULL;
	m_updateRepo = NULL;
	m_updateCache = NULL;
	m_hScheduledUpdateThread = INVALID_HANDLE_VALUE;
#endif

	m_hLibertyXThread = INVALID_HANDLE_VALUE;

#if (APP_DIGITALMINT)
	m_bAcceptorStatusChanged = false;
#endif
}

CMainFrame::~CMainFrame()
{
	
}

void CMainFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	if(m_pDevCmn)
	{
		m_pDevCmn->fnAPL_UnLoadDevice();
	}

	//CMN lib에 있는 함수 호출, NVRAM memory를 free해준다.
	FreeMemoryAddress();

	if(m_pAdaCtrl)
	{
		delete m_pAdaCtrl;
		m_pAdaCtrl = NULL;
	}

	if(m_pTranCmn)
	{
		delete m_pTranCmn;
		m_pTranCmn = NULL;
	}

	if(m_pDevCmn)
	{
		delete m_pDevCmn;
		m_pDevCmn = NULL;
	}

//	if(m_pNetWork)
//	{
//		delete m_pNetWork;
//		m_pNetWork = NULL;
//	}

	if (m_pRmsCtrl)
	{
		delete m_pRmsCtrl;
		m_pRmsCtrl = NULL;
	}

	if (m_LXConfig)
	{
		delete m_LXConfig;
		m_LXConfig = NULL;
	}

#ifndef NO_REMOTE_UPDATE
	delete m_updateConfiguration;
	delete m_updateRepo;
	delete m_updateCache;

	if (m_hScheduledUpdateThread != INVALID_HANDLE_VALUE)
	{
		TerminateThread(m_hScheduledUpdateThread, 0);
		CloseHandle(m_hScheduledUpdateThread);
	}
#endif

#if (APP_LIBERTYX)
	if (m_hLibertyXThread != INVALID_HANDLE_VALUE)
	{
		TerminateThread(m_hLibertyXThread, 0);
		CloseHandle(m_hLibertyXThread);
	}
#endif

	CFrameWnd::OnClose();
}

/////////////////////////////////////////////////////////////////////////////
// Set Owner
void CMainFrame::SetOwner(CMainFrame *pOwner)
{
	return;
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// 2008.05.29 JSW //////////////////////////////////////////////////////////////////////////////////////
	// NextWare Log trace기능 On/Off 옵션 조절
	// TraceType		BS Trace		NVRAM Log
	//     1			OFF				ON
	//     2			ON				OFF
	//    이외			ON				ON

	// [#2570] AU HJAHN 2018.08.17	NEXTWARE Log ON/OFF 방식 변경, 특정 폴더 있을 경우 ON / 없을 경우 OFF
	//								필요 시 Patch 로 생성/삭제할 수 있도록 해당 폴더 위치를 ATM 파티션 내부로 둠
	//#if NEXTWARE_BSTRACE_ON

// [RWC6-676] Start SKKim 2024.05.21 중복 코드 제거 (InstallNextwareRegistries() 함수에서 수행됨)
//	if (IsExistFile( _T("\\ATM\\BSTrace.nwlog") ))
//	{
//		int remaining = 120 * 1000;
//		const int delay = 1000;
//
//		while (remaining > 0)
//		{
//			if (IsExistFile(L"\\USB"))
//			{
//				break;
//			}
//
//			Delay_Msg(delay);
//			remaining -= delay;
//		}
//
//		CopyFile(_T("\\ATM\\BSTrace.nwlog"), L"\\USB\\BSTrace.nwlog", FALSE);
//	}
//
//	if (IsExistFile( _T("\\ATM\\BSTRACE_ON") ))
//	// end of [#2570]
//	{
//		//BSTRACE ON
//		RegSetInt(L"SOFTWARE\\Nextware", L"TraceType", 2);					// 2:BS Trace 남김
//		RegSetInt(L"SOFTWARE\\Nextware", L"Tracelevel", 5);					// Trace Level 조정
//	// 	RegSetInt(L"SOFTWARE\\Nextware", L"TraceFileSize", 0xA00000);			// Trace Level 조정
//		RegSetInt(L"SOFTWARE\\Nextware", L"TraceFileSize", 0x1400000);			// Trace Level 조정
// 		RegSetStr(L"SOFTWARE\\Nextware", L"TraceDirectory", L"\\ATM");			// Trace Level 조정
//	}
//	else
//	{
//		//BSTRACE OFF
//		RegSetInt(L"SOFTWARE\\Nextware", L"TraceType", 1);			// 1:BS Trace 남기지 않음
//	}
//
//#ifdef UNDER_CE		// RFID OCX registration should be skipped on Win32
//	// [#2325] NH KSK 2015.01.21 RFID 지원을 위한 Registry 설정
//	// Registering the CLSID
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}", L"", L"NXCardReaderX Control", 2);
//
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\ProgID", L"", L"NXCardReaderX.NXRFIDReaderXCtrl.1", 2);
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\InprocServer32", L"", L"\\ATM\\NXRFIDReader30.ocx", 2);
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\InprocServer32", L"ThreadingModel", L"Apartment", 2);
//
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\Control", L"", L"", 2);
//
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\Implemented Categories", L"", L"", 2);
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\Implemented Categories\\{7DD95801-9882-11CF-9FA9-00AA006C42C4}", L"@", L"", 2);
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\MiscStatus", L"", L"0", 2);
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\MiscStatus\\1", L"", L"131217", 2);
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\ToolboxBitmap32", L"", L"\\ATM\\NXRFIDReader30.ocx, 1", 2);
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\TypeLib", L"", L"{E686457D-5350-4AE1-9B4A-3B80BDAB7EED}", 2);
//	RegSetStr(L"CLSID\\{DDF7D417-FB61-44A0-8023-06FC57ABA9CF}\\Version", L"", L"1.0", 2);
//	// end of [#2325]
//#endif
//
//	Delay_Msg(100);
// [RWC6-676] End SKKim 2024.05.21

	// 2008.03.05 JSW //////////////////////////////////////////////////////////////////////////////////////

	// [#6] NH PSC 2008.03.24 AtmStart() 실행위치 이동 CMainFrm::OnTimer() -> CWinAtmApp::InitInstance()
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

    //타이틀바를 없앤다.
	cs.style = WS_VISIBLE|WS_POPUP;

    return CFrameWnd::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}


#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

// [#RWC6-14] US William 2019.09.18 Add Safe Door Status To Journal
void CMainFrame::OnSafeDoorChanged(LPCTSTR status)
{
	NHERROR((L"Safe door status changed: %s\r\n", status));
	
	if (wcscmp(status, L"OPEN") == 0)
	{
		m_pDevCmn->fnAPL_StackError(_T("2000400"), m_pDevCmn->fstrSCR_GetStringFromTextID(T_APMSG_003061), DEV_DOR);
	}

	CString errMsg;
	errMsg.Format(L"Safe door: %s", status);
	m_pDevCmn->m_JNLMgr.Save(OPERATOR_ACTION, errMsg);
}

/////////////////////////////////////////////////////////////////////////////
// PROTOTYPE : void OnDisplayData(LPCTSTR szDataName, LPCTSTR szDataValue)
// DESCRIPT  : Request event for display(To Flash)
// PARAMETER : Data Name, Data
// RETURN    : 
// REMARK    : This function is event handler
//             Receive display screen event from MWI and do some process
/////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnDisplayData(LPCTSTR szDataName, LPCTSTR szDataValue)
{
NHDEBUG(1, (_T("***MainFrm***CMainFrame::OnDisplayData(%s, %s) \n"), szDataName, szDataValue));

	if (m_pDevCmn)
		m_pDevCmn->fnSCR_SetVariable(szDataName, szDataValue);
}

/////////////////////////////////////////////////////////////////////////////
// PROTOTYPE : void OnDisplayPinpadData(LPCTSTR szDataValue)
// DESCRIPT  : Request event for display(To Flash)
// PARAMETER : Data
// RETURN    : 
// REMARK    : This function is event handler
//             Receive display screen event from MWI and do some process
//			   2004.04.07
/////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnDisplayPinpadData(LPCTSTR szDataValue)
{
	DWORD curTickCount = GetTickCount();

	CString	strDataValue(szDataValue);
	int		nTempScrNo = 0;

	// [#2056] NH KSK 2011.05.04
	if (m_pDevCmn->AtmStatus == ATM_INIT)
	{
		if (m_pDevCmn->m_strHotKey.GetLength() > 50)
		{
			m_pDevCmn->m_strHotKey.Empty();
		}
		else
		{
			m_pDevCmn->m_strHotKey += strDataValue;
			m_pDevCmn->m_strHotKey += L",";
		}

		m_pDevCmn->fnSCR_SetKeyEvent(strDataValue);
	}
	// end of [#2056]
															
	//1. 초기화면 & Error 화면 Hot key를 check한다.	
	if( m_pDevCmn->AtmStatus == ATM_READY  || 
		m_pDevCmn->AtmStatus == ATM_CUSTOM || 
		m_pDevCmn->AtmStatus == ATM_ERROR)
	{
		//ADA Jack In인 경우
		if ((m_pDevCmn->fnSNS_GetEnhancedAudio() == FALSE) && (m_pTranCmn->IsStartTransaction() == FALSE))
		{
			// [#RWC6-94] NH Kook 2020.01.31 Bring hotkey checking here to cover both of TTW/non-TTW models.
			m_pDevCmn->ReArrangeEnteredHotKey();	
			m_pDevCmn->m_strHotKey += strDataValue + ",";
			// end of [#RWC6-94]

			//if (!(m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T || m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T))	// [#2064] NH 2011.05.18
			if (m_pDevCmn->GetDeviceType(L"SPL") != SPL_SET)																					// [#GLDV-2505] US Kook 2019.05.13
			{
				//[#2283] NH Justin Support Customized Hot Key
				/*
				if ((strDataValue == "ENTER") || (m_pDevCmn->m_strHotKey.GetLength() > 30))			//무한정 입력되어 m_strKey string이 커지는 것 방지
				{
					m_pDevCmn->m_strHotKey = "";
				}
				m_pDevCmn->m_strHotKey = m_pDevCmn->m_strHotKey + strDataValue;
				m_pDevCmn->m_strHotKey += ",";
				
				//Hot Key가 입력된 경우
				if(m_pDevCmn->m_strHotKey.GetLength() >= (int)strlen("ENTER,CLEAR,CANCEL,1,2,3,"))
				{
					if(m_pDevCmn->m_strHotKey.Find(L"ENTER,CLEAR,CANCEL,1,2,3") != -1)	//Supervisor 진입
					{
						m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
						m_pDevCmn->m_nHotKeyStatus = OPER_SUPERVISOR;
					}
					else
						m_pDevCmn->m_nHotKeyStatus = OPER_RUN;
				}
				*/

				// [#RWC6-94] NH Kook 2020.01.31 Move hotkey checking outside of here to cover both of TTW/non-TTW models.
				//m_pDevCmn->ReArrangeEnteredHotKey();	
				//m_pDevCmn->m_strHotKey += strDataValue + ",";
				// end of [#RWC6-94]

				if(m_pDevCmn->m_strHotKey.GetLength() >= (m_pDevCmn->m_strStoredHotKey.GetLength()+1))
				{
					if(m_pDevCmn->m_strHotKey.Find(m_pDevCmn->m_strStoredHotKey) != -1)	//Supervisor 진입
					{
						m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
						m_pDevCmn->m_nHotKeyStatus = OPER_SUPERVISOR;
					}
					else
						m_pDevCmn->m_nHotKeyStatus = OPER_RUN;
				}
				// End of [#2283]
			}

			// [#RWC6-94] NH Kook 2020.01.31 allowing 'RESET_MP_SPECIAL_KEY' on ATM ERROR mode w/ opened safe door.
			if (m_pDevCmn->AtmStatus == ATM_ERROR && m_pDevCmn->fnDOR_GetDoorStatus() == DOOR_OPENED)
			{
				if (m_pDevCmn->m_strHotKey.Find(RESET_MP_SPECIAL_KEY) != -1)
				{
					m_pDevCmn->m_bExecuteResetMP = true;
					Main_SetAtmStatus(ATM_INIT);
				}
			}
			// end of [#RWC6-94]
		}
	}

	//////////////////////////////////////////////////////////////////////////

	//거래 화면에서의 Key 처리
	if (m_pDevCmn != NULL)
	{
		// ADA MODE
		if (m_pTranCmn->IsAdaTransaction())
		{
			// PASSWORD에서 숫자 KEY 처리
			if (m_pTranCmn->IsPasswordMode())
			{
				// [#2375] US Justin Disable Echo for US S/W TTS
				#if !(US_VERSION)		 
				m_pAdaCtrl->fnExp_StopPlay();
				Delay_Msg(20);

				if(strDataValue == L"CANCEL" || strDataValue == L"CLEAR" || strDataValue == L"ENTER")
				{
					Delay_Msg(20);
					m_pAdaCtrl->fnExp_SndPlay(strDataValue + L".wav",1);
				}
				#endif
			}
			else 
			{
				if (strDataValue == L"CANCEL" || strDataValue == L"CLEAR" || strDataValue == L"ENTER"||
				    strDataValue == L"0" || strDataValue == L"1" || strDataValue == L"2" ||
					strDataValue == L"3" || strDataValue == L"4" || strDataValue == L"5" ||
					strDataValue == L"6" || strDataValue == L"7" || strDataValue == L"8" ||
					strDataValue == L"9")
				{
				//////////////////////////////////////////////////////////////////////////////////////////////////
				//[#2117] US PCS 2012.01.12 "연속 입력을 제외한 부분에 대해서는 음성 안내 수정."
				// 유효하지 않은 값에 대해서는 Invalid하다는 안내 문구 재생.
				#if (US_VERSION)
					// [#2375] US Justin 2015.10.28 US Justin Software TTS... All are handled by each screen
					/*
					if (m_pTranCmn ->IsAdaSuccessiveInput() == TRUE)
					{
						m_pAdaCtrl->fnExp_StopPlay();				//숫자 Or Enter/Cancel 음성 출력
						Delay_Msg(20);
						m_pAdaCtrl->fnExp_SndPlay(strDataValue + L".wav",1);
					}
					*/
					// End of [#2375]
				#else
					m_pAdaCtrl->fnExp_StopPlay();				//숫자 Or Enter/Cancel 음성 출력
					Delay_Msg(20);
					m_pAdaCtrl->fnExp_SndPlay(strDataValue + L".wav",1);
				#endif
				//end of [#2117]
				}
			}
			m_pDevCmn->fnSCR_SetByPassData(strDataValue);
		}
		// NORMAL MODE
		else if ((m_pDevCmn->AtmStatus == ATM_CUSTOM) ||
				 //((m_pDevCmn->AtmStatus == ATM_CLERK) && (m_pDevCmn->m_pConfig->GetOSVersion() != NH_OS_NH2700_T)))
				 //((m_pDevCmn->AtmStatus == ATM_CLERK) && (!(m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T || m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T))))	// [#2064] NH KSK 2011.05.18
				 ((m_pDevCmn->AtmStatus == ATM_CLERK) && (m_pDevCmn->GetDeviceType(L"SPL") != SPL_SET)))		// [#GLDV-2505] US Kook 2019.05.13
		{
			//[#2000] SOOK 2010.10.22 호주 사양 적용 
			#if ( AU_VERSION)	// [#2069] NH KSK 2011.06.13
				if (m_pDevCmn->fnSCR_GetCurrentScreenNo() == 103) // Password Screen 이면 
				{
					if (strDataValue == L"CLEAR")
					{
						m_pDevCmn->fnSCR_SetKeyEvent(strDataValue);
						m_pDevCmn->fnSCR_SetKeyEvent(L"PINRETRY");
						sndPlaySound(L"\\ATM\\dingdingding.wav", SND_ASYNC);
						m_nPINInputCount = 0;
					}
					else if ((strDataValue != L"CANCEL") && (strDataValue != L"ENTER" ))
					{
						m_nPINInputCount++;

						if (m_nPINInputCount == 1)
						{
							m_pDevCmn->fnSCR_SetKeyEvent(strDataValue);
							m_pDevCmn->fnSCR_SetKeyEvent(L"FIRSTUSERINPUT");
						}
						else
						{
							m_pDevCmn->fnSCR_SetKeyEvent(strDataValue);
						}
					}
					else
					{
						m_pDevCmn->fnSCR_SetKeyEvent(strDataValue);
					}

				}
				else
				{
					m_pDevCmn->fnSCR_SetKeyEvent(strDataValue);
					m_nPINInputCount = 0;

				}
			#else  //end of [#2000]
				m_pDevCmn->fnSCR_SetKeyEvent(strDataValue);
			#endif
		}
	}

	if (strDataValue == "000") 
	{
		NHDBG((L"Taking screenshot"));
		m_pDevCmn->fnSCR_SaveScreenshotToSD();
	}

	NHDEBUG(1, (_T("DONE [%s] (%lu ms)\n"), szDataValue, GetTickCount() - curTickCount));
}

bool CMainFrame::DoRemoteSoftwareUpdate()
{
	BYTE* data = NULL;
	CString swVersion;
	CFile file;

	if (!IsExistFile(REMOTE_UPDATE_INIT_FILE))
	{
		return false;
	}

	if (!file.Open(REMOTE_UPDATE_INIT_FILE, CFile::modeRead))
	{
		return false;
	}
	
	data = (BYTE*) new BYTE[(unsigned int)file.GetLength() + 1]();
	file.Read(data, (unsigned int)file.GetLength());

	swVersion = CString(data);
	delete [] data;
	file.Close();

	DeleteFile(REMOTE_UPDATE_INIT_FILE);

	// Set up the registry values for the update.exe process
	RegSetStr(_T("SOFTWARE\\ATM\\APP"), _T("TerminalID"), MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));
	RegSetStr(_T("SOFTWARE\\ATM\\APP"), _T("RemoteDownloadVersion"), swVersion);
	RegSetInt(_T("SOFTWARE\\ATM\\APP"), _T("RMSFileUpdate"), 4);

	return ExecuteSWUpdate() == TRUE;
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	KillTimer(nIDEvent);

	switch(nIDEvent)
	{
		case TIMER_THREAD:
			m_pDevCmn->HostOpenRetryTime = 0;
			break;

		// [#6] NH PSC 2008.03.24 AtmStart() 실행위치 이동 CMainFrm::OnTimer() -> CWinAtmApp::InitInstance()	
		default:
			break;
	}

	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::AtmStart()
{
	NHDEBUG(1, (_T("***MainFrm***CMainFrame::AtmStart() \n")));
	NVDump('O', 'A', "02", L"", L"1:AtmStart()" );

	// Load MWI
	NHDEBUG(1, (L"MWI CREATE\n"));
	BOOL mwiLoadResult = Mwi.Create(NULL, 0, CRect(0,0,0,0), this, IDC_NHMWI);
	if (!mwiLoadResult)
	{
		NHWARN((L"MWI failed. Attempting reload.\r\n"));

		mwiLoadResult = Mwi.Create(NULL, 0, CRect(0,0,0,0), this, IDC_NHMWI);
		if (!mwiLoadResult)
		{
			NHERROR((L"MWI Load failed.\r\n"));
		}
	}

	CNHConfig *m_pConfig = GetConfigFuncPointer();

	// [#2370] NH KSK 2015.09.08 간헐적으로 AP Loading Fail을 현상 수정
	NHDEBUG(DBG_INFO, (L"LOAD NV-RAM\n"));
	GetMemoryAddress();
	NHDEBUG(DBG_INFO, (L"CHECK NV-RAM\n"));
	Check_BrokenNVRAM();		// Check NVRAM

	UpgradePasswords();		// [#RWC6-56] US William 2019.12.26 Hash passwords

	NHDEBUG(DBG_INFO, (L"CREATE CONTROL (Dev, Tran, Ada, Rms)\n"));

	// If the update file is present, go ahead and start the update process
	if (DoRemoteSoftwareUpdate())
	{
		NHERROR((L"Performing software update. Stop loading AP.\r\n"));
		return;
	}

	m_pDevCmn  = new CDevCmn();									// Create Device
	m_pTranCmn = new CTranCmn();								// Create Transaction
	m_pAdaCtrl = new CAdaCtrl();
	m_pRmsCtrl = new CRmsCtrl();
	m_pAdaCtrl->fnExp_SetDevPointer(m_pDevCmn);

#if (SUPPORT_SOFTWARE_TTS)	// [#2368] US KSK 2015.10.25
	m_pAdaCtrl->fnExp_TTS_Initialize(this->m_hWnd);
#endif				// end of [#2368]


	// Share MainFrame's TAR Asset to its children
	if (CAssetArchive::GetInstance())
		m_pDevCmn->SetTarAsset(CAssetArchive::GetInstance());

	//m_pRmsCtrl->SetDevCmn(m_pDevCmn);
	m_pRmsCtrl->SetDevCmn(m_pDevCmn, m_pTranCmn);				// [#2467] US Justin 2017.01.25
	m_pDevCmn->SetOwner(&Mwi);									// Set Owner(Device)
	m_pTranCmn->SetOwner(m_pDevCmn, m_pAdaCtrl);				// Set Owner(Transaction)
	// end of [#2370]

	// Screen Setting
	{
		CScrCtrl		*pScrCtrl = NULL;

		// Create Screen
		for (int i = SCR_FRONT; i <= SCR_KEYMGR; i++)
		{
			NHDEBUG(DBG_INFO, (L"CREATE TYE(%d)\n", i));

			// Get Screen Config
			NH_SCR_CONFIG eScreenConfig = m_pConfig->GetScreenConfig((NH_SCR_TYPE)i);
			
			pScrCtrl = CScrCtrl::CreateScreenEngine((SCREEN_TYPE)eScreenConfig.eEngine,
													  0,
													  0,
													  eScreenConfig.nWidth,
													  eScreenConfig.nHeight);
			m_arScrCtrl[i] = pScrCtrl;
			if (pScrCtrl != NULL)
			{
				pScrCtrl->Initialize();

				// Move Screen
				pScrCtrl->MoveScreen(eScreenConfig.nX, eScreenConfig.nY, 
									 eScreenConfig.nWidth, eScreenConfig.nHeight);

				pScrCtrl->GetUserInputCheck(INFINITE);
				pScrCtrl->ClearUserInputData();
							
				if (i == SCR_FRONT)
					pScrCtrl->SetActiveScreen();

				pScrCtrl->LoadScreens(eScreenConfig.strLoadPath,
									eScreenConfig.strLoadSection,
									eScreenConfig.nWidth,
									eScreenConfig.nHeight);

				if (eScreenConfig.eEngine == SCR_TYPE_PICASSO)
				{
					pScrCtrl->GetUserInputCheck(INFINITE);
					pScrCtrl->ClearUserInputData();

					if (i == SCR_FRONT)
					{
						pScrCtrl->SetVariable(L"PrevSetScreenNo", L"INIT");
						pScrCtrl->SetVariable(L"kindLanguage", L"ENGLISH");
						pScrCtrl->SetVariable(L"APCenterMovie", L"INIT");
					}
				}
			}
		}
	}

	NHDEBUG(DBG_INFO, (L"LOAD DEVICE\n"));
	m_pDevCmn->fnAPL_LoadDevice();								// Load Device
	NHDEBUG(DBG_INFO, (L"LOAD DEVICE OK\n"));

	// Set AP Screen
//	{
//		NHDEBUG(DBG_INFO, (L"SHWO INITIALIZE SCREEN(001)\n"));
//		CScrCtrl *pScrCtrl = m_arScrCtrl[SCR_FRONT];
//		if (pScrCtrl != NULL)
//		{
//			pScrCtrl->SetActiveScreen();
//			pScrCtrl->SetVariable(L"PrevSetScreenNo", L"001");
//			pScrCtrl->SetVariable(L"kindLanguage", L"ENGLISH");
//			pScrCtrl->SetVariable(L"APCenterMovie", L"001");
//		}
//	}

	// [#2370] NH KSK 2015.09.08
//	NHDEBUG(DBG_INFO, (L"LOAD NV-RAM\n"));
//	GetMemoryAddress();
////	AntiSkimmingInit();					-- Not support
////	SetAntiSkimmingDetectTime(5*1000);	-- Not support
//
//	NHDEBUG(DBG_INFO, (L"CHECK NV-RAM\n"));
//	// Check NVRAM
//	Check_BrokenNVRAM();
	// end of [#2370]

// [#247] NH KSK 2008.5.28 Added DEMO Information for STD1 / STD3
#ifdef APP_LOCAL_MODE
	
//	ClearAPNVRam();		// KSK 2010.06.09 Local Mode시 NVRAM을 Clear하는 이유는?

	// 2008.02.28 //////////////////////////////////////////////////////////////////////////////////
	if (MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE) == MSG_TRITON_TYPE)
	{
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_HEALTH,		L"DEMO1234       .60.~d8ebbd6ca011bafe.!00000150.{0d66b909beebca58./0.md8ebbd6ca011bafe.}0d66b909beebca58.");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_CONFIG,		L"DEMO1234       .60.~d8ebbd6ca011bafe.!00000150.{0d66b909beebca58./0.md8ebbd6ca011bafe.}0d66b909beebca58.");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_TOTAL,		L"DEMO1234       .50.022508.00020000000000008000./0.");
		// [#399] AU AIREAT 2008.8.7
#if (AU_VERSION) // [#2031] NZ KJW 2011.03.16	// [#2069] NH KSK 2011.06.13
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_WITHDRAWAL,	L"0.DEMO1234       .11.1226.000.00000177.022708.165106.000000.00403200.00000150.ud8A023030.");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_INQUIRY,	L"0.DEMO1234       .31.1226.000.00000177.022708.165106.000000.00403200.00000150.ud8A023030.");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_TRANSFER,	L"0.DEMO1234       .21.1226.000.00000177.022708.165106.000000.00403200.00000150.ud8A023030.");
#else
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_WITHDRAWAL,	L"0.DEMO1234       .11.1226.000.00000177.022708.165106.022708.00403200.00000150.");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_INQUIRY,	L"0.DEMO1234       .31.1226.000.00000177.022708.165106.022708.00403200.00000150.");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_TRANSFER,	L"0.DEMO1234       .21.1226.000.00000177.022708.165106.022708.00403200.00000150.");
#endif
		// end of [#399]
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_DEPOSIT,	L" ");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_REVERSAL,	L"DEMO1234       .29.1225.000./0.");
	}
	else
	{
		// [#428] [NH] KSK 2008.9.29
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_HEALTH,		L"H0.A01102.HY999999.90.20060127182910.0200.");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_CONFIG,		L"H0.A01102.HY999999.88.01272006182754.000500.0d66b909beebca58.0150.0.d8ebbd6ca011bafe");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_TOTAL,		L"H0.A01102.HY999999.87.0012005600010000.556000.00.5600.00");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_WITHDRAWAL,	L"H0.A01102.HY999999.85.0354.00.02102006210901123456784161.1205249002102006.356780.356780.0150.STANDARD1 HOST TEST ONLY.00.ud8A023030");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_INQUIRY,	L"H0.A01102.HY999999.85.0355.00.02102006211057123456788419.1255659002102006.356780.356780.0150.STANDARD1 HOST TEST ONLY.00.ud8A023030");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_TRANSFER,	L"H0.A01102.HY999999.85.0356.00.02102006211207123456789166.1268939002102006.356780.356780.0150.STANDARD1 HOST TEST ONLY.00.ud8A023030");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_DEPOSIT,	L" ");
		MemSetStr(_MEM_FLD_APP_SIMINFO, _MEM_VAR_APP_LC_REVERSAL,	L"H0.A01102.HY999999.86.00");
		// end of [#428]
	}
#endif

// end of [#247]
/////////////////////////////////////////////////////////////////////////////

	// [#2370] NH KSK 2015.09.08
//	NHDEBUG(DBG_INFO, (L"CREATE CONTROL (Dev, Tran, Ada, Rms)\n"));

//	m_pDevCmn  = new CDevCmn();									// Create Device
//	m_pTranCmn = new CTranCmn();								// Create Transaction
//	m_pAdaCtrl = new CAdaCtrl();
//	m_pRmsCtrl = new CRmsCtrl();

//	m_pAdaCtrl->fnExp_SetDevPointer(m_pDevCmn);
//	m_pRmsCtrl->SetDevCmn(m_pDevCmn);
//	m_pDevCmn->SetOwner(&Mwi);									// Set Owner(Device)
//	m_pTranCmn->SetOwner(m_pDevCmn, m_pAdaCtrl);				// Set Owner(Transaction)

//	NHDEBUG(DBG_INFO, (L"LOAD DEVICE\n"));

//	m_pDevCmn->fnAPL_LoadDevice();								// Load Device
	// end of [#2370]

	// Set AP Screen
	{
		NHDEBUG(DBG_INFO, (L"SHOW INITIALIZE SCREEN(001)\n"));
		CScrCtrl *pScrCtrl = m_arScrCtrl[SCR_FRONT];
		if (pScrCtrl != NULL)
		{
			pScrCtrl->SetActiveScreen();
			pScrCtrl->SetVariable(L"PrevSetScreenNo", L"001");
			pScrCtrl->SetVariable(L"kindLanguage", L"ENGLISH");
			pScrCtrl->SetVariable(L"APCenterMovie", L"001");
		}
	}

	NHDEBUG(DBG_INFO, (L"SETTING CONTROL\n"));

	for (int i = 0; i < SCR_LOCATION_MAX; i++)
	{
		if (m_arScrCtrl[i] != NULL)
		{
			m_pDevCmn->fnSCR_SetScrOwner((NH_SCR_TYPE)i, m_arScrCtrl[i]);

			#ifdef APP_LOCAL_MODE
				m_arScrCtrl[i]->SetVariable(L"APLocalMovie", L"DEMO VERSION");		
			#endif

			#ifdef APP_AGING_MODE
				m_arScrCtrl[i]->SetVariable(L"APLocalMovie", L"AGING VERSION");
			#endif
		}
	}

#ifdef APP_AGING_MODE
	MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_MSGTYPE, MSG_TRITON_TYPE);

	#ifdef AGING_DIAL
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE,		NETWORK_DIALUP);		// DIAL
	MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_EOTCHECKMODE, EOT_REQUIRED );
	#endif

	#ifdef AGING_TCPIP
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE,		NETWORK_TCPIP);		// TCP/IP
	MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DHCP,			0);
	#endif
	
	if (m_pConfig->GetOSVersion() == NH_OS_NH2700_B
		|| m_pConfig->GetOSVersion() == NH_OS_NH1800SE_B
		|| m_pConfig->GetOSVersion() == NH_OS_NH1500SE)	// [#11] NH KSK 2010.10.1 [#2267] US KSK 2014.05.08
	{
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID, L"AGING_TEST_B");
		MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1, L"0588");
		MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE2, L"0588");
	}
	else if (m_pConfig->GetOSVersion() == NH_OS_NH2700_L)
	{
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID, L"AGING_TEST_L");
		MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1, L"0589");
		MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE2, L"0589");
	}
//	else if (m_pConfig->GetOSVersion() == NH_OS_NH2700_T)
//	else if (m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T
//		|| m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T)	// [#2064] NH KSK 2011.05.18
	else if (m_pDevCmn->GetDeviceType(L"SPL") == SPL_SET)													// [#GLDV-2505] US Kook 2019.05.13
	{
		MemSetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID, L"AGING_TEST_T");
		MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE1, L"0589");
		MemSetStr(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_PHONENODEFINE2, L"0589");
	}
#endif
	if (m_pDevCmn->GetDeviceType(L"SPL") == SPL_SET)															// [#GLDV-2505] US Kook 2019.05.13
	{
		CScrCtrl *pScrCtrl = m_pDevCmn->GetScreenByType(SCR_REAR);
		if (pScrCtrl != NULL)
			pScrCtrl->SetVariable(L"APCenterMovie", L"710");
	}
	m_pDevCmn->fnSCR_SetActiveMode(SCR_FRONT);

	m_pDevCmn->m_nHotKeyStatus = OPER_RUN;
	m_pDevCmn->HostOpenRetryTime = 0;

	m_pTranCmn->SetAdditionalSetting();

	// [#2186] US KMK 2013.05.06
	if (bNeedRebootOnInit)	// 폰트 설치 후 리부팅이 필요한 경우
		TerminateATM();
	// end of [#2186]

	// [#2186] US KMK 2013.04.30 폰트 유무에 따른 언어 EN/DISABLE 처리
	if (m_pDevCmn->fnSCR_IsFontLoaded(CHN_MODE-1) == FALSE)
		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_CHNMODE,		0);
	if (m_pDevCmn->fnSCR_IsFontLoaded(KOR_MODE-1) == FALSE)
		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_KORMODE,		0);
	if (m_pDevCmn->fnSCR_IsFontLoaded(JPN_MODE-1) == FALSE)
		MemSetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_JAPMODE,		0);

	// US KMK 2013.05.06 Default Language가 현재 Disable된 Language일 경우 ENGLISH로 변경
	int nCurDefLang = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE);
	if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ENGMODE + (nCurDefLang-1)) == 0)	// Default Langauge가 Disable된 항목이면
	{
		MemSetInt(_MEM_FLD_APP_ATMINFO,	_MEM_VAR_APP_ENGMODE,	ENABLE);				// ENGLISH를 ENABLE시킨 후
		MemSetInt(_MEM_FLD_OPTIONALSETTING,	_MEM_VAR_OPT_DEFAULT_LANGUAGE,	ENG_MODE);	// ENGLISH로 변경
	}


	// end of [#365]

// [#2446] AU KSK 2017.01.18 전국가 적용
	m_pDevCmn->fnSCR_SetCurrentLangMode(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE));

	// [#2075] NH KSK 2011.06.25 EPP가 512K가 아닌 경우 RKT 미 지원이므로 Disable 처리 (PIN Open후 Check해야함
	if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_512K_EP_VERSION) < 0)
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_AVAILABLE, 0);
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_ENABLE, 0);
		// [#2499] US Justin.. MOVE IN MODE CONTROL....(INIT)
		//MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DUALHOSTDCC_ENABLE, 0);	// [#2185] US Justin 2013.04.11 Dual Host DCC...=> Disable Service
	}
	else
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_AVAILABLE, ENABLE);
	}
	// end of [#2075]

	// [#2259] NH KSK 2014.03.13 PCI3.0 RKT시 Random Number 필수로 사용하도록 보완 처리
	if (MemGetVersion(_MEMKEY_EPVERSION, L"PIN").CompareNoCase(PIN_PCI30_EP_VERSION) >= 0)
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_RKT_RANDOMNUMBER_ENABLE, ENABLE);
	// end of [#2259]

	// [#2394] NH KSK 2016.02.05 OS Background Image 변경시 WinAtm Exception 발생하는 오류 수정
	// Change Background Image (Splash -> Out of service)
	//{
	//	LONG	lResult;
	//	HKEY	hKeyResult;
	//	DWORD	dwValue = 2;	// from loader

	//	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, L"ControlPanel\\Desktop", 0, KEY_ALL_ACCESS, &hKeyResult);
	//	if (lResult == ERROR_SUCCESS)
	//	{
	//		TCHAR szNewBg[256];
	//		memset(szNewBg, 0, sizeof(szNewBg));

	//		_tcscpy(szNewBg, L"\\Windows\\WindowsCE.jpg");

	//		RegSetValueEx(hKeyResult, L"Wallpaper", 0, REG_SZ, (CONST BYTE*)szNewBg, (sizeof(TCHAR)*(_tcsclen(szNewBg)+1)));

	//		RegCloseKey(hKeyResult);
	//	}

	//	// Update
	//	::PostMessage(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETDESKWALLPAPER, 0);
	//}
	// end of [#2394]

	Chk_Key.RemoveAll();					// [#29] NH PSC 2008.03.18 메모리 Leak 대책으로 위치 이동
	// [#2172] NH KSK 2012.12.27
	for(int i=0; i<= MAX_PIN_KEYINDEX; i++)
		Chk_Key.Add(MemGetStr(_MEM_FLD_SP_PIN, _MEM_VAR_PINSP_KEYCHECKSUM0+i));
	// end of [#2172]

	// [#GLDV-2683] NH Kook 2019.11.06 Detects Modem Removal
	if (m_pDevCmn->fnNET_ModemTest2() == TRUE)
	{
		NHDEBUG(1, (L"MODEM OK\n"));
		m_pDevCmn->bIsModemAvailable = true;
		NVDump('F', 'E', "0R", L"", L"MDM_INIT_OK");
	}
	else
	{
		NHDEBUG(1, (L"MODEM NG\n"));
		m_pDevCmn->bIsModemAvailable = false;
		MemSetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_NETWORKTYPE, 2);
		NVDump('F', 'E', "0R", L"", L"MDM_INIT_ERR");
		NVDump('F', 'E', "0R", L"", L"SET_TCPIP");
	}
	// end of [#GLDV-2683]

	// Load Remote Updates
#ifndef NO_REMOTE_UPDATE
	m_remoteUpdatesAvailable = false;

	CUpdateRepoConfigurationManager::InstallDefaultConfigurationFiles();

	CUpdateRepoConfigurationManager manager;
	m_updateConfiguration = new UpdateSourceInfo;
	if (manager.GetConfiguration(m_updateConfiguration) == ERR_OK)
	{
		m_updateCache = new CRepoLocalCache();
		m_updateRepo = new CUpdateRepoService(m_updateConfiguration);
		m_remoteUpdatesAvailable = true;

#ifdef UNDER_CE
		DWORD threadId;
		m_hScheduledUpdateThread = CreateThread(NULL, 0, UpdateSchedulerThread, this, 0, &threadId);
		if (m_hScheduledUpdateThread == INVALID_HANDLE_VALUE)
		{
			NHERROR((L"Could not start schedule update thread\r\n"));
		}
#endif
	}
#endif

#if (APP_LIBERTYX)
#  ifdef UNDER_CE
	DWORD ltxThread = 0;
	m_hLibertyXThread = CreateThread(NULL, 0, LibertyXHeartbeatThread, m_pDevCmn, 0, &ltxThread);
	if (m_hLibertyXThread == INVALID_HANDLE_VALUE)
	{
		NHERROR((L"Could not start LibertyX heartbeat thread\r\n"));
	}
#  endif
#endif

#if (APP_CUSTOM_CASHDEPOT)
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME).IsEmpty())
		MemSetStr(_MEM_FLD_OPTIONALSETTING_3, _MEM_VAR_OPT3_CASHDEPOT_HOSTNAME, L"cdbiab.cdlatm.com/BankInABoxApi/v1");
#endif

/////////////////////////////////////////////////////////////////////////////	
	m_pDevCmn->AtmDoing = TRUE;									// ATM Doing
	while (m_pDevCmn->AtmDoing)
	{
		Delay_Msg(10);
		AtmModeCtrl();											// ATM Mode Control
	}
}

void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_ESCAPE)
		OnClose();

	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	return CFrameWnd::PreTranslateMessage(pMsg);
}
