#include "stdafx.h"
#include "PicassoCtrl.h"


//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1


//------------------------------------------------------------------
//	Define Global Variable
//------------------------------------------------------------------

//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CPicassoCtrl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : constructor
-------------------------------------------------------------------*/
CPicassoCtrl::CPicassoCtrl()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nPointCount = 0;

	// Get Country Short cut
#if	(US_VERSION)
	m_strLoadCountry = _T("US");
#elif (AU_VERSION)
	m_strLoadCountry = _T("AU");
#elif (CA_VERSION)
	m_strLoadCountry = _T("CA");
#elif (MX_VERSION)
	m_strLoadCountry = _T("MX");	// [#2015] MX KSK 2012.01.31
#endif

#ifndef UNDER_CE
	m_strLoadCountry = AfxGetApp()->GetProfileString(_T("Setting"), _T("Country"), _T("US"));
#endif


	m_bInit = FALSE;
	m_bLoadControl = TRUE;
	m_bDecodeCash = FALSE;

	m_bInputEnabled = FALSE;

	m_nScreenWidth = 0;
	m_nScreenHeight = 0;
	m_nLogPixelsY = 0;

	m_pDisplayScreen = NULL;
	m_pCurrentScreen = NULL;
	m_pNextScreen = NULL;

	m_hDisplayWnd = NULL;
	m_hParentWnd = NULL;

	m_AdvertiseInfo.bEnable = FALSE;
	m_AdvertiseInfo.nEnableCount = 0;
	m_AdvertiseInfo.nRefreshTime = 5;
	memset(m_AdvertiseInfo.bEnableAdv, 0, sizeof(m_AdvertiseInfo.bEnableAdv));
	memset(m_AdvertiseInfo.pAdvScreen, 0, sizeof(m_AdvertiseInfo.pAdvScreen));
	
	m_NoticeInfo.bEnable = FALSE;
	m_NoticeInfo.pScreen = NULL;

	m_TimeoutMode = SCR_TMODE_STOP;

	m_bBackRefresh = FALSE;
	m_bBackDrawed = FALSE;
	m_strBackID = _T("");
	m_strScreenID = _T("");

	m_bmBackBuffer = NULL;
	m_bmScreenBuffer = NULL;
	
	for (int i = 0; i < MAX_CACHE_SCREEN; i++)
	{
		m_eCacheScreens[i].strID = L"";
		m_eCacheScreens[i].hBitmap = NULL;
	}

	m_nCurrentLocale = ENG_MODE;
	m_nMaxLocale = 0;

	m_bLocalMode = FALSE;
	m_hLocalFont = NULL;

//	m_DebugPoint.x = 0;
//	m_DebugPoint.y = 0;
	m_strDebugMsg = _T("");

	m_pNScreen = NULL;
	m_pNTextBox = NULL;
	m_pNHideTextBox = NULL;
	m_pNEffectTextBox = NULL;
	m_pNEditTextBox = NULL;
	m_pNChangePWBox = NULL;
	m_pNButtonBox = NULL;
	m_pNVKeyButtonBox = NULL;
	m_pNSWKeyBox = NULL;
	m_pNToggleBox = NULL;
	m_pNEJNLBox = NULL;
	m_pNDateBox = NULL;
	m_pNIPBox = NULL;
	m_pNBinBox = NULL;
	m_pNCentBox = NULL;
	m_pNDollarBox = NULL;
	m_pNAdvBox = NULL;
	m_pNAniBox = NULL;
	m_pNTableEditBox = NULL;
	m_pNAlphaKeyBox = NULL;
	m_pNNumberKeyBox = NULL;
	m_pNTableKeyBox = NULL;
	m_pNThreeStateBox = NULL;
	m_pNBinIndexBox = NULL;
	m_pNLocaleText = NULL;
	m_pVKeyBoardBox = NULL;
	m_pImageListBox = NULL;
	m_pButtonImageListBox = NULL;
	m_pAmountBox = NULL;
	m_pKeyListBox = NULL;
	m_pWeatherBox = NULL;
	m_pTranAdvBox = NULL;
	m_pNCustomEditBox = NULL;	// [#2065] NH KJW 2011.05.26		// [#2350] Justin Change Name
	m_pNDecimalBox = NULL;		// [#2316] NH Justin 2014.12.16
	m_pMultiTextBox = NULL;

	// [#2186] US KMK 2013.04.30 Asian Language Support
	// US KMK 2013.05.06 변수 초기화, Initialize()에서 생성자로 위치 이동 (로직 영향 없음)
	m_bIsFontLoaded[ENG_MODE-1] = TRUE;
	m_bIsFontLoaded[SPN_MODE-1] = TRUE;
	m_bIsFontLoaded[FRN_MODE-1] = TRUE;
	m_bIsFontLoaded[CHN_MODE-1] = FALSE;
	m_bIsFontLoaded[KOR_MODE-1] = FALSE;
	m_bIsFontLoaded[JPN_MODE-1] = FALSE;
	// end of [#2186]

	// [#2186] US KMK 2013.05.06 폰트명 배열 초기화 추가
	for (int i=0; i<MAX_LOCALE; i++)
		m_strLangFonts[i] = _T("");
	// end of [#2186]

	m_isWorkingThread = true;
	m_pPicassoThread = AfxBeginThread(ThPicassoCtrl, this);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: ~CPicassoCtrl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : destructor
-------------------------------------------------------------------*/
CPicassoCtrl::~CPicassoCtrl()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_bExit = TRUE;
	m_isWorkingThread = false;

	if (m_hLocalFont != NULL)
	{
		::DeleteObject(m_hLocalFont);
		m_hLocalFont = NULL;
	}

#ifndef UNDER_CE
	::KillTimer(GetSafeHwnd(), UM_BLINKING_TIME);	// necessary for Screen Viewer reload

	// remove some memory leaks during screen reload
	{
		CString key;
		POSITION pos;

		{
			CPicassoResource* res;
			pos = m_PicassoResourcesCMap.GetStartPosition();
			while(pos)
			{
				m_PicassoResourcesCMap.GetNextAssoc(pos, key, res);
				RES_TYPE resType = res->GetTpye();
				switch (resType)
				{
				case RES_PEN: delete ((CPicassoPen*)res); break;
				case RES_FONT: delete ((CPicassoFont*)res); break;
				case RES_BRUSH: delete ((CPicassoBrush*)res); break;
				case RES_PICTURE: delete ((CPicassoPicture*)res); break;
				case RES_LOCALETEXT: delete ((CPicassoLocaleText*)res); break;
				case RES_COLOR: delete ((CPicassoColor*)res); break;
				case RES_POSITION: delete ((CPicassoPosition*)res); break;
				default:
					delete res;
				}
			}
		}

		{
			CPicassoScreen* resScr;
			pos = m_PicassoScreensCMap.GetStartPosition();
			while(pos)
			{
				m_PicassoScreensCMap.GetNextAssoc(pos, key, resScr);
				delete resScr;
			}
		}

		for (int i=0; i<MAX_CACHE_SCREEN; i++)
		{
			::DeleteObject(m_eCacheScreens[i].hBitmap);
			m_eCacheScreens[i].hBitmap = NULL;
		}
	}
#endif

}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::Initialize(int nWidth, int nHeight)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_nScreenWidth = nWidth;
	m_nScreenHeight = nHeight;
	m_LoadScreenSize.cx = nWidth;
	m_LoadScreenSize.cy = nHeight;

	// Get Amount of the Logical Pixels Y
	{
		HDC hDC = ::GetDC(NULL);
		m_nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);

		// Create bitmap buffer for background.
		if (m_bmBackBuffer == NULL)
			m_bmBackBuffer = MakeDIBSection(hDC, m_nScreenWidth, m_nScreenHeight);

		// Create bitmap buffer for screen.
		// [#GLDV-2505] These bitmap buffers should be kept by "CreateCompatibleBitmap" to process font aliasing as intended. ('MakeDIBSection' shows weird font shape.)
		// https://microsoft.public.windowsce.embedded.vc.narkive.com/19JnYqqD/how-to-drawtext-by-cleartype-quality-to-a-dib-that-by-createdibsec
		if (m_bmScreenBuffer == NULL)
			m_bmScreenBuffer = ::CreateCompatibleBitmap(hDC, m_nScreenWidth, m_nScreenHeight);

		::ReleaseDC(NULL, hDC);
	}

	// Local Mode Font Create.
	{
		LOGFONT	lf;

		memset(&lf, 0, sizeof(lf));
		
		// Face Name
		_tcscpy(lf.lfFaceName, _T("Arial"));
		lf.lfHeight = GET_FONTSIZE(40);
		lf.lfWeight = FW_BOLD;
		
		m_hLocalFont = ::CreateFontIndirect(&lf);
	}

	//LoadSection(_T("\\ATM\\SCREEN"), _T("INIT"));

	::SetTimer(GetSafeHwnd(), UM_BLINKING_TIME, BLINKING_TIME, NULL);

	//m_hParentWnd = GetParent()->m_hWnd;
	m_hParentWnd = GetParent()->GetSafeHwnd();
	m_hDisplayWnd = GetSafeHwnd();
	m_bInit = TRUE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::SetVariable(CString strCmd, CString strData)
{
	NHUIDBG(DBG_CALL, (_T("strCmd(%s), strData(%s)\n"), strCmd, strData));

	return m_VariableQueue.Enqueue(CScrMsg(strCmd, strData));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::SetVariable(CScrMsg &ApMsg)
{
	return m_VariableQueue.Enqueue(ApMsg);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SetLoadScreenSize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::SetLoadScreenSize(int nWidth, int nHeight)
{
	m_LoadScreenSize.cx = nWidth;
	m_LoadScreenSize.cy = nHeight;
}
