#include "stdafx.h"
#include "PicassoCtrl.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

extern CAssetArchive* g_pTarScreenAsset;			// global variable used in ScrCtrl
//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: LoadSection()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::LoadSection(CString strScreenPath, CString strSection)
{
	NHUIDBG(DBG_CALL, (_T("ScreenPath(%s), Section(%s)\n"), strScreenPath, strSection));

	BOOL			bRes = TRUE;
	CNHReadiniFile	Readini;
	PINIVALUE		pIniValue;
	CString			strMasterFile;

	CHECK_TIME_START;

	m_strScreenPath = strScreenPath;
	strMasterFile = m_strScreenPath + MASTER_SCREEN_FILE;
	m_strLoadSection = _T("");

	// try TAR first.
	BOOL bDatOpened = FALSE;
	if (g_pTarScreenAsset
		&& strScreenPath != L"")	// in case of Update.exe, which accesses its own "MasterScreenDesc.dat".
	{
		unsigned int fileSize = g_pTarScreenAsset->GetFileSize(MASTER_SCREEN_FILE);
		if (fileSize > 0)
		{
			BYTE* pTarBuffer = new BYTE[fileSize + 1]();
			g_pTarScreenAsset->ReadFile(MASTER_SCREEN_FILE, pTarBuffer);
			bDatOpened = Readini.Open(pTarBuffer, CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII);
			delete[] pTarBuffer;
		}
	}

	// try FILE next.
	if (!bDatOpened)
	{
		bDatOpened = Readini.Open(strMasterFile, CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII);
	}

	// TAR/FILE all failed.
	if (!bDatOpened)
	{
		NHDBG((_T("[SCR_IN] File Open Failed (%s) in TEXT_RES\n"), MASTER_SCREEN_FILE));
		return FALSE;
	}

	// open Master Screen File
 	//if (Readini.Open(strMasterFile, CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII))
 	if (bDatOpened)
	{
		while (NULL != (pIniValue = Readini.ReadiniValue()))
		{
			// [RESOURCE] Section
			if (pIniValue->Section == strSection)
			{
				// File Key
				if (pIniValue->Key == DES_FILE)
				{
					if (!LoadScreenValue(pIniValue->Values[0], CNHReadiniFile::modeASCII))
					{
						bRes = FALSE;
						break;
					}
				}
				// UText Key
				else if (pIniValue->Key == DES_UFILE)
				{
					if (!LoadScreenValue(pIniValue->Values[0], CNHReadiniFile::modeLittleUNICODE))
					{
						bRes = FALSE;
						break;
					}
				}
				// UText Key
				else if (pIniValue->Key == DES_SCRTEXT)
				{
					if (!LoadScreenText(pIniValue->Values[0]))
					{
						bRes = FALSE;
						break;
					}
				}
				// Load Control
				else if (pIniValue->Key == DES_LOAD_CONTROL)
				{
					m_strLoadSection = pIniValue->Values[0];
				}
			}
		} // end of while (NULL != (pIniValue = Readini.ReadiniValue()))
	}
	else
	{
		NHERROR((_T("[SCR_IN] File Open Failed (%s) in MASTER_RES\n"), strMasterFile));
		bRes = FALSE;
	}

	Readini.Close();

	CHECK_TIME_END;

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: LoadScreenValue()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::LoadScreenValue(LPCTSTR FileName, CNHReadiniFile::OpModeFlags Type)
{
	NHUIDBG(DBG_CALL, (_T("FileName(%s), OpenType(%d)\n"), FileName, Type));

	BOOL		bRes = TRUE;
	CNHReadiniFile	Readini;
	PINIVALUE	pIniValue;
	CString		strResPath;
	CString		strScreenSection_WidthOnly, strScreenSection_WidthHeight;
	CString		strFullFileName = m_strScreenPath + FileName;

	CHECK_TIME_START;

	m_bLoadControl = TRUE;
	m_strSupportLanguage.Empty();

	// open Master Screen File
	strFullFileName.Trim();

	// try TAR first.
	BOOL bDatOpened = FALSE;
	if (g_pTarScreenAsset)
	{
		CString strFileName(FileName);
		strFileName.Trim();
		
		unsigned int fileSize = g_pTarScreenAsset->GetFileSize(strFileName);
		if (fileSize > 0)
		{
			BYTE* pTarBuffer = new BYTE[fileSize + 1]();
			g_pTarScreenAsset->ReadFile(strFileName, pTarBuffer);
			bDatOpened = Readini.Open(pTarBuffer, CNHReadiniFile::modeRead, Type);
			delete[] pTarBuffer;
		}
	}

	// try FILE next.
	if (!bDatOpened)
	{
		bDatOpened = Readini.Open(strFullFileName, CNHReadiniFile::modeRead, Type);
	}

	// TAR/FILE all failed.
	if (!bDatOpened)
	{
		NHDBG((_T("[SCR_IN] File Open Failed (%s) in TEXT_RES\n"), FileName));
		return FALSE;
	}

// 	if (Readini.Open(strFullFileName, CNHReadiniFile::modeRead, Type))
	if (bDatOpened)
	{
		strResPath.Format(_T("RES_%d_%d"), m_nScreenWidth, m_nScreenHeight);

		// [#RWC6-262] DAT integration
		strScreenSection_WidthHeight.Format(_T("SCREEN_%d_%d"), m_nScreenWidth, m_nScreenHeight);		// [SCREEN_1024_600]

		// Create public controls.
		while (NULL != (pIniValue = Readini.ReadiniValue()))
		{
			if (m_bLoadControl == TRUE)
			{
				// [SUPPORT] section
				if (pIniValue->Section == DES_SUPPORT)
				{
					if (pIniValue->Key.Left(2) == m_strLoadCountry && _ttoi(pIniValue->Values[0]) == 1)
					{
						m_strSupportLanguage += pIniValue->Key.Right(3);
						m_strSupportLanguage += _T(",");
					}
				}
				// [OVERRIDE] section
				else if (pIniValue->Section == DES_OVERRIDE)
				{
					if (pIniValue->Key == AP_TYPE)
					{
						LoadScreenValue(pIniValue->Values[0], Type);
					}
				}
				// [RESOURCE] Section
				if (pIniValue->Section == DES_RESOURCE || pIniValue->Section == strResPath)
				{
					// [#RWC6-262]
					m_bOverrideResourceValue = false;

					// override resource value when it is given from [RES_width_height] section like below.
					// [Resource]				[RES_640_480]				(then 1500SE will use)
					// Position=POS1,1,1,1,1							=>	Position=POS1,1,1,1,1
					// Position=POS2,2,2,2,2	Position=POS2,4,4,4,4		Position=POS2,4,4,4,4
					if (pIniValue->Section == strResPath)
						m_bOverrideResourceValue = true;
					// end of [#RWC6-262]

					if (!CreateResource(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [SCREEN] Section
				else if (pIniValue->Section == DES_SCREEN)
				{
					if (!CreateScreen(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [#RWC6-262] DAT integration
				// find [SCREEN_xxxx] or [SCREEN_xxxx_yyy]
				else if (pIniValue->Section.Find(L"SCREEN_") != -1)
				{
					// when given resolution is 800x600, check if current section is a piece of "[SCREEN_800_600]"
					//  - welcome:
					//		[SCREEN_800_600]
					//		[SCREEN_800]
					//  - not welcome:
					//		[SCREEN_1024_600]
					//		[SCREEN]			<< filtered by Find("SCREEN_") in advance

					// - not welcome
					if (strScreenSection_WidthHeight.Find(pIniValue->Section) == -1
						&& !(m_nScreenWidth == 720 && pIniValue->Section.Find(L"SCREEN_1024_600") != -1))		// 720x480 uses 1024x600 screen
					{
						// create a dummy screen to keep screen engine going on
						// otherwise, screen engine will stop working with "LOAD_FAIL" while processing SCREEN-dependent components, like TEXTBOX
						if (!m_pNScreen)
							m_pNScreen = (CPicassoScreen*) new CPicassoScreen;

						//if (pIniValue->Key == DES_SCREEN && !Asc2Int(pIniValue->Values[0]))		// [/SCREEN]
						//	m_pNScreen = NULL;
					}
					// - welcome
					else if (!CreateScreen(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// end of [#RWC6-262]
				// [TEXTBOX] Section
				else if (pIniValue->Section == DES_TEXTBOX)
				{
					if (!CreateTextBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [MULTITEXTBOX] Section
				else if (pIniValue->Section == DES_MULTITEXTBOX)
				{
					if (!CreateMultiTextBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [HIDETEXTBOX] Section
				else if (pIniValue->Section == DES_HIDETEXTBOX)
				{
					if (!CreateHideTextBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [EFFECTTEXTBOX] Section
				else if (pIniValue->Section == DES_EFFECTTEXTBOX)
				{
					if (!CreateEffectTextBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [EDITBOX] Section
				else if (pIniValue->Section == DES_EDITBOX)
				{
					if (!CreateEditBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [CHANGEPWBOX] Section
				else if (pIniValue->Section == DES_CHANGE_PW_BOX)
				{
					if (!CreateChangePWBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [BUTTON] Section
				else if (pIniValue->Section == DES_BUTTON)
				{
					if (!CreateButtonBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [SWKEY] Section
				else if (pIniValue->Section == DES_SW_KEY)
				{
					if (!CreateSWKeyBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [TOGGLEBOX] Section
				else if (pIniValue->Section == DES_TOGGLE_BOX)
				{
					if (!CreateEnDisableBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [EJNLBOX] Section
				else if (pIniValue->Section == DES_EJNL_BOX)
				{
					if (!CreateEJNLBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [DATEBOX] Section
				else if (pIniValue->Section == DES_DATE_BOX)
				{
					if (!CreateDateBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [IPBOX] Section
				else if (pIniValue->Section == DES_IP_BOX)
				{
					if (!CreateIPBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [BINBOX] Section
				else if (pIniValue->Section == DES_BIN_BOX)
				{
					if (!CreateBinBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [CENTBOX] Section
				else if (pIniValue->Section == DES_CENT_BOX)
				{
					if (!CreateCentBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [DOLLARBOX] Section
				else if (pIniValue->Section == DES_DOLLAR_BOX)
				{
					if (!CreateDollarBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [ADVBOX] Section
				else if (pIniValue->Section == DES_ADV_BOX)
				{
					if (!CreateAdvBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [ADVBOX] Section
				else if (pIniValue->Section == DES_TRANADV_BOX)
				{
					if (!CreateTranAdvBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [ANIBOX] Section
				else if (pIniValue->Section == DES_ANI_BOX)
				{
					if (!CreateAniBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [TABLEEDITBOX]
				else if (pIniValue->Section == DES_TABLE_EDITBOX)
				{
					if (!CreateTableEditBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [VKEYBUTTON]
				else if (pIniValue->Section == DES_VKEYBUTTON)
				{
					if (!CreateVKeyButtonBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [ALPHAKEYBOX]
				else if (pIniValue->Section == DES_ALPHA_KEYBOX)
				{
					if (!CreateAlphaKeyBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [NUMBERKEYBOX]
				else if (pIniValue->Section == DES_NUMBER_KEYBOX)
				{
					if (!CreateNumberKeyBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [TABLEKEYBOX]
				else if (pIniValue->Section == DES_TABLE_KEYBOX)
				{
					if (!CreateTableKeyBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [THREESTATEBOX]
				else if (pIniValue->Section == DES_THREE_STATEBOX)
				{
					if (!CreateThreeStateBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [BININDEXBOX] Section //[#543] SOOK 
				else if (pIniValue->Section == DES_BININDEX_BOX)
				{
					if (!CreateBinIndexBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [VKEYBOARDBOX] Section
				else if (pIniValue->Section == DES_VKEYBOARD_BOX)
				{
					if (!CreateVKeyBoardBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [IMAGELISTBOX] Section
				else if (pIniValue->Section == DES_IMAGELIST_BOX)
				{
					if (!CreateImageListBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [BUTTONIMAGELISTBOX] Section
				else if (pIniValue->Section == DES_BUTTON_IMAGELIST_BOX)
				{
					if (!CreateButtonImageListBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [AMOUNTBOX] Section
				else if (pIniValue->Section == DES_AMOUNT_BOX)
				{
					if (!CreateAmountBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [KEYLISTBOX] Section
				else if (pIniValue->Section == DES_KEY_LIST_BOX)
				{
					if (!CreateKeyListBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [WEATHERBOX] Section
				else if (pIniValue->Section == DES_WEATHER_BOX)
				{
					if (!CreateWeatherBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// [#2065] NH KJW 2011.05.26
				// [CUSTOMEDITBOX] Section
				else if (pIniValue->Section == DES_CUSTOMEDIT_BOX)
				{
					if (!CreateCustomEditBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// end of [#2065]
				// [#2316] JUSTIN 2014.12.17 Decimal Box
				// [DECIMALBOX] Section 
				else if (pIniValue->Section == DES_DECIMAL_BOX)
				{
					if (!CreateDecimalBox(pIniValue))
					{
						bRes = FALSE;
						break;
					}
				}
				// End of [#2316]
				else
				{
					if (pIniValue->Section == DES_LOAD_CONTROL)
						LoadControlSetting(pIniValue);
				}
			}
			else
			{
				if (pIniValue->Section == DES_LOAD_CONTROL)
					LoadControlSetting(pIniValue);
			}
		} // end of while (NULL != (pIniValue = Readini.ReadiniValue()))
	}
	else
	{
		NHERROR((_T("[SCR_IN] File Open Failed (%s) in SCR_RES\n"), FileName));
		bRes = FALSE;
	}

	Readini.Close();
	
	CHECK_TIME_END;

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateResource()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateResource(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	COLORREF	crColor;
	CPicassoResource	*pRes;

	// Font=ID,Name,Size,Bold
	if (pIniValue->Key == DES_FONT)
	{
		LOGFONT	lf;
		HFONT	hFont;
		int		nBold;

		if (pIniValue->nValue == 4)
		{
			memset(&lf, 0, sizeof(lf));

			// Face Name
			// i want to a fixed width font
			if (pIniValue->Values[1] == DES_FIXED_FONT)
			{
				// Name
				_tcscpy(lf.lfFaceName, _T("Courier New"));

				// SIZE
				int fontsize = Asc2Int(pIniValue->Values[2]);

				// SIZE
				lf.lfHeight = GET_FONTSIZE(fontsize);
				lf.lfWidth = (LONG)(lf.lfHeight * (2/3));		// [#2165] NH KSK 2012.11.18 Width값 계산시에 2/3의 기준은?
				if (lf.lfWidth < 0)
					lf.lfWidth *= -1;

				// set quality
#ifdef UNDER_CE
				lf.lfQuality = CLEARTYPE_COMPAT_QUALITY;
#else
				lf.lfQuality = DEFAULT_QUALITY;
#endif

				nBold = Asc2Int(pIniValue->Values[3]);				
				// BOLD
				if (nBold >= 3)
					lf.lfWeight = FW_HEAVY;
				else if (nBold >= 1)
					lf.lfWeight = FW_BOLD;
			}
			else
			{
				// Name
				_tcsncpy(lf.lfFaceName, pIniValue->Values[1], LF_FACESIZE-1);

				// [#2518] US Kook 2018.01.20 Support MX-2800SE
				if (pIniValue->Values[1] == _T("WYVERNFONT"))
				{
					_tcscpy(lf.lfFaceName, _T("Tahoma"));
				}
				// end of [#2518]
				
				// SIZE
				int fontsize = Asc2Int(pIniValue->Values[2]);

				// [#2165] NH KSK 2012.11.16 현재 Font는 해상도별로 별도로 설정하게 되어져 있으므로 Resolution 하지 않도록 수정해야함
				//         전체적으로 dat file 수정이 불가피하여 Next Version Up을 위해 주석만 추가함
				// Adjust Scale
				{
					int currentX = m_nScreenWidth;
					int currentY = m_nScreenHeight;
		
					float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);

					// [#2300] US KSK 2014.1.04
					float ScreenRateY = 0;
					if (currentX == 1024 && currentY == 768)
						ScreenRateY = (float)((float) MX5200SE_VIRTUAL_SCREEN_Y / (float)BASIC_SCREEN_Y);		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
					else
						ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);
					// end of [#2300]

					if (ScreenRateX > ScreenRateY)
						fontsize = (int)((float)ScreenRateX * fontsize + 0.5);
					else
						fontsize = (int)((float)ScreenRateY * fontsize + 0.5);
				}
				// end of [#2165]

				lf.lfHeight = GET_FONTSIZE(fontsize);

				// [#2518] US Kook 2018.01.20 Support MX-2800SE
				if (pIniValue->Values[1] == _T("WYVERNFONT"))
				{
					// maintain original font width for WYVERNFONT (Tahoma).
				}
				else
				// end of [#2518]
				{
					lf.lfWidth = (LONG)(lf.lfHeight * 0.43);	// [#2165] NH KSK 2012.11.18 Width값 계산시에 0.43의 기준은?
				}

				// set quality
#ifdef UNDER_CE
				lf.lfQuality = CLEARTYPE_QUALITY;
#else
				lf.lfQuality = DEFAULT_QUALITY;
#endif

				if (lf.lfWidth < 0)
					lf.lfWidth *= -1;
				
				// BOLD
				if (Asc2Int(pIniValue->Values[3]))
				{
#ifdef UNDER_CE
					lf.lfWeight = FW_BOLD;
#else
					lf.lfWeight = FW_SEMIBOLD;
#endif
					// [#2518] US Kook 2018.01.19 Support bold values larger than 1.
					int nBoldWeight = Asc2Int(pIniValue->Values[3]) - 1;
					lf.lfWeight += (nBoldWeight * 100);
					// end of [#2518]
				}
			}

			// [#2186] US KMK 2013.04.08 Asian Language Support
// 			hFont = ::CreateFontIndirect(&lf);
// 			if (hFont)
// 			{
// 				pIniValue->Values[0].MakeUpper();
// 				pRes = (CPicassoResource*) new CPicassoFont(RES_FONT, pIniValue->Values[0], hFont);
// 				if (pRes)
// 					m_PicassoResources.PutOnTailOfList(pRes->GetID(), pRes);
// 			}

			pIniValue->Values[0].MakeUpper();

			CString strFontIDs[MAX_LOCALE];	// language define을 0부터 시작했으면 좋았을텐데 ...
			for (int i=0; i<MAX_LOCALE; i++)	
			{
				// 2013.05.03 폰트 호환성 개선
				// 영어는 'ID', 다른 언어는 'ID_(언어명)'의 형식으로 폰트를 등록한다. ex) ID, ID_SPN, ID_FRN, ...
				// (기존 PicassoControl에서는 ID를 사용하므로)
				if (i == ENG_MODE-1)
					strFontIDs[i].Format(_T("%s"), pIniValue->Values[0]); 
				else
					strFontIDs[i].Format(_T("%s_%s"), pIniValue->Values[0], GET_LANGUAGE_SHORT_STRING(i+1)); 

				// 각 ID마다 언어별 폰트(m_strLangFonts 배열)를 할당한다
				// m_strLangFonts 배열의 기본값은 공백, ExtraFonts로 폰트를 지정한 언어만 미리 할당됨
				// FIXEDFONT는 별도로 처리함
				if (pIniValue->Values[1] != DES_FIXED_FONT)
				{
					if (m_strLangFonts[i].GetLength() > 0)
						// ExtraFont로 미리 할당된 폰트가 있으면 그것을 사용함
						_tcsncpy(lf.lfFaceName, m_strLangFonts[i], LF_FACESIZE-1);
					else
						// m_strLangFonts 값이 공백일 경우 기본 FONTNAME을 가져옴 (FONT=ID, FONTNAME, SIZE, BOLD)
						// 현재 DAT에서는 대부분 폰트가 Arial로 되어 있음
						_tcsncpy(lf.lfFaceName, pIniValue->Values[1], LF_FACESIZE-1);
				}

#ifdef UNDER_CE
				// [#GLDV-2853] AU Kook 2021.03.15 WEC7 Asian Fonts Supplements
				// lfCharSet should be set 
				if (GetConfigFuncPointer()->GetCEVersion() == WINCE_7)
				{
					if (pIniValue->Values[1] != DES_FIXED_FONT)
					{
						if (i == KOR_MODE-1)			lf.lfCharSet = HANGUL_CHARSET;
						else if (i == JPN_MODE-1)		lf.lfCharSet = SHIFTJIS_CHARSET;
						else if (i == CHN_MODE-1)		lf.lfCharSet = CHINESEBIG5_CHARSET;

						// F_ApButtonGulim, F_ApButtonSimsun
						if (pIniValue->Values[0].CompareNoCase(L"F_ApButtonGulim") == 0)
						{
							lf.lfCharSet = HANGUL_CHARSET;
							_tcsncpy(lf.lfFaceName, L"Gulim", LF_FACESIZE-1);
						}

						if (pIniValue->Values[0].CompareNoCase(L"F_ApButtonSimsun") == 0)
						{
							lf.lfCharSet = CHINESEBIG5_CHARSET;
							_tcsncpy(lf.lfFaceName, L"Simsun", LF_FACESIZE-1);
						}
					}
				}
				// end of [#GLDV-2853]
#endif

				hFont = ::CreateFontIndirect(&lf);
				if (hFont)
				{
					// ID, ID_SPN, ID_KOR, ... 등의 폰트 리소스 추가
					pRes = (CPicassoResource*) new CPicassoFont(RES_FONT, strFontIDs[i], hFont);
					if (pRes)
						//m_PicassoResources.PutOnTailOfList(pRes->GetID(), pRes);
						m_PicassoResourcesCMap.SetAt(pRes->GetID(), pRes);

				}
			}
			// end of [#2186]

		}
	}
	// Pen=ID,Width,RGB
	else if (pIniValue->Key == DES_PEN)
	{
		int		Width;
		HPEN	hPen;

		if (pIniValue->nValue == 5)
		{
			// Width
			Width = Asc2Int(pIniValue->Values[1]);

			// RGB
			crColor = RGB(Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
							Asc2Int(pIniValue->Values[4]));

			hPen = ::CreatePen(PS_SOLID, Width, crColor);
			if (hPen)
			{
				pIniValue->Values[0].MakeUpper();
				pRes = (CPicassoResource*) new CPicassoPen(RES_PEN, pIniValue->Values[0], hPen);
				if (pRes)
					//m_PicassoResources.PutOnTailOfList(pRes->GetID(), pRes);
					m_PicassoResourcesCMap.SetAt(pRes->GetID(), pRes);
			}
		}
	}
	// Color=ID,RGB
	else if (pIniValue->Key == DES_COLOR)
	{
		if (pIniValue->nValue == 4)
		{
			pIniValue->Values[0].MakeUpper();
			pRes = (CPicassoResource*) new CPicassoColor(pIniValue->Values[0], 
											 Asc2Int(pIniValue->Values[1]),
											 Asc2Int(pIniValue->Values[2]), 
											 Asc2Int(pIniValue->Values[3]));
			if (pRes)
				//m_PicassoResources.PutOnTailOfList(pRes->GetID(), pRes);
				m_PicassoResourcesCMap.SetAt(pRes->GetID(), pRes);

		}
	}
	// Brush=ID,RGB
	else if (pIniValue->Key == DES_BRUSH)
	{
		HBRUSH	hBrush;

		if (pIniValue->nValue == 4)
		{
			// RGB
			crColor = RGB(Asc2Int(pIniValue->Values[1]), Asc2Int(pIniValue->Values[2]),
							Asc2Int(pIniValue->Values[3]));

			hBrush = CreateSolidBrush(crColor);
			if (hBrush)
			{
				pIniValue->Values[0].MakeUpper();
				pRes = (CPicassoResource*) new CPicassoBrush(RES_BRUSH, pIniValue->Values[0], hBrush);
				if (pRes)
					//m_PicassoResources.PutOnTailOfList(pRes->GetID(), pRes);
					m_PicassoResourcesCMap.SetAt(pRes->GetID(), pRes);
			}
		}
	}
	// Picture=ID,File
	else if ((pIniValue->Key == DES_PICTURE) && (pIniValue->nValue == 3))
	{
		CString strCountry, strLanguage;
		strCountry = pIniValue->Values[0].Left(2);
		strLanguage = pIniValue->Values[0].Right(3);

		if ((strCountry == m_strLoadCountry || strCountry == _T("US")) &&
			(m_strSupportLanguage.Find(strLanguage) != -1))
		{
			CPicassoPicture	*pPicture = NULL;
			CPicassoResource *pRes2 = NULL;

			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], (CPicassoResource**)&pPicture);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pRes2);

			pPicture = (CPicassoPicture*) pRes2;

			if (pPicture == NULL)
			{
				pPicture = (CPicassoPicture*) new CPicassoPicture(pIniValue->Values[1]);
				if (pPicture == NULL)
					return FALSE;

				//m_PicassoResources.PutOnTailOfList(pIniValue->Values[1], pPicture);
				m_PicassoResourcesCMap.SetAt(pIniValue->Values[1], pPicture);
				NHDEBUG(DBG_CALL, (_T("Picture Set: (%s)\n"), pIniValue->Values[1]));
			}

			// [#RWC6-262] DAT integration
			{
				if (m_bOverrideResourceValue)
				{
					// no additional codes required. because pPicture->AddImage() below overwrites old value naturally.
					// since pPicture is not storing image in the memory (just uses given filepath) there would be no memory leaks. so I don't think we need to use "delete pPicture" here.
					//
					// Note: If you want to use "delete/new pPicture" for override, please be sure that 1st IMAGE (US_ENG) could be removed when 2nd IMAGE (US_FRN) is added. it should be handled.
				}

				// convert MACRO values into real numbers (i.e. "%WD%_%HT%" -> "1024_600")
				if (pIniValue->Values[2].Find(_T("%WD%")) != -1)
				{
					CString strScreenWidth, strScreenHeight;
					strScreenWidth.Format(_T("%d"), m_nScreenWidth);
					strScreenHeight.Format(_T("%d"), m_nScreenHeight);

					pIniValue->Values[2].Replace(_T("%WD%"), strScreenWidth);
					pIniValue->Values[2].Replace(_T("%HT%"), strScreenHeight);
				}
			}
			// end of [#RWC6-262]

			if (strLanguage == _T("ENG"))
			{
				pPicture->AddImage(ENG_MODE, (m_strScreenPath + pIniValue->Values[2]));
				NHDEBUG(DBG_CALL, (_T("Picture Added: (%s): (%s %s)\n"), pIniValue->Values[1], m_strScreenPath, pIniValue->Values[2]));
			}
			else if (strLanguage == _T("SPN"))
				pPicture->AddImage(SPN_MODE, (m_strScreenPath + pIniValue->Values[2]));
			else if (strLanguage == _T("FRN"))
				pPicture->AddImage(FRN_MODE, (m_strScreenPath + pIniValue->Values[2]));
			else if (strLanguage == _T("CHN"))
				pPicture->AddImage(CHN_MODE, (m_strScreenPath + pIniValue->Values[2]));
			else if (strLanguage == _T("KOR"))
				pPicture->AddImage(KOR_MODE, (m_strScreenPath + pIniValue->Values[2]));
			else if (strLanguage == _T("JPN"))
				pPicture->AddImage(JPN_MODE, (m_strScreenPath + pIniValue->Values[2]));
		}
	}
	// [#2186] US KMK 2014.04.08 Asian Language Support
	// ExtraFont=Language,FontName,FontPath
	// ex) ExtraFont=KOR,Gulim,\ATM\Gulim.ttc
	else if ((pIniValue->Key == DES_EXTRA_FONT) && (pIniValue->nValue == 3))
	{
		CString strLanguage = pIniValue->Values[0];
		CString strFontName = pIniValue->Values[1];
		CString strFontPath = pIniValue->Values[2];

//		if (IsExistFile(ASIAN_FONT_DEST_PATH) == TRUE)	// 2013.05.03 KMK 예외처리 보강
		{
			if (AddFontResource(strFontPath))
			{
				NHUIDBG(DBG_CALL, (_T("Font has been added. (%s)"), strFontPath));
	 			//::SendMessage(HWND_BROADCAST, WM_FONTCHANGE, NULL, NULL);	// AP 기동 중 멈춰서 PostMessage()로 대체
 				//::PostMessage(HWND_BROADCAST, WM_FONTCHANGE, NULL, NULL);	// WM_FONTCHANGE 메시지 안보내도 폰트 잘 표시됨. 제거함.

				// 폰트명을 m_strLangFonts 배열에 지정한다
				// CHN, KOR, JPN의 경우 폰트 등록이 성공했을 경우에만 ENABLE시켜
				// 폰트가 없을때 사용되는 경우가 없도록 한다
				if (strLanguage == _T("ENG"))	{
					m_strLangFonts[ENG_MODE-1] = strFontName;
	// 				m_bIsFontLoaded[ENG_MODE-1] = TRUE;	// 기본값이 이미 TRUE
				}
				else if (strLanguage == _T("SPN"))	{
					m_strLangFonts[SPN_MODE-1] = strFontName;
	// 				m_bIsFontLoaded[SPN_MODE-1] = TRUE;	// 기본값이 이미 TRUE
				}
				else if (strLanguage == _T("FRN"))	{
					m_strLangFonts[FRN_MODE-1] = strFontName;
	// 				m_bIsFontLoaded[FRN_MODE-1] = TRUE;	// 기본값이 이미 TRUE
				}
				else if (strLanguage == _T("CHN"))	{
					m_strLangFonts[CHN_MODE-1] = strFontName;
					m_bIsFontLoaded[CHN_MODE-1] = TRUE;
				}
				else if (strLanguage == _T("KOR"))	{
					m_strLangFonts[KOR_MODE-1] = strFontName;
					m_bIsFontLoaded[KOR_MODE-1] = TRUE;
				}
				else if (strLanguage == _T("JPN"))	{
					m_strLangFonts[JPN_MODE-1] = strFontName;
					m_bIsFontLoaded[JPN_MODE-1] = TRUE;
				}
			}
			else
			{
				NHUIDBG(DBG_CALL, (_T("Failed to add font. (%s)"), strFontPath));
			}
		}
	}
	// end of [#2186]
	// [#2529] NH Justin 2018.02.22 Add 2800 SE
	// Position=ID,xx,yy,width,height
	else if (pIniValue->Key == DES_POSITION)
	{
		NHUIDBG(DBG_CALL, (_T("POSITION Resource (nValue = %d)"), pIniValue->nValue));

		if (pIniValue->nValue == 5)
		{
			pIniValue->Values[0].MakeUpper();

			CPicassoResource	*pPos = NULL;
// 			m_PicassoResources.FindDataOfList(pIniValue->Values[0], (CPicassoResource**)&pPos);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pPos);

			// [#RWC6-262] DAT integration, override with new "Position" values when it's given from [RES_width_height].
			if (m_bOverrideResourceValue)
			{
				if (pPos != NULL)
				{
					NHUIDBG(DBG_CALL, (_T("POSITION Resource will be overrided as (%s, %d, %d, %d, %d)"), pIniValue->Values[0], Asc2Int(pIniValue->Values[1]), 
						Asc2Int(pIniValue->Values[2]), 	 Asc2Int(pIniValue->Values[3]), Asc2Int(pIniValue->Values[4]) ) );

					// way 1) delete and let it be made as new one?
					//delete (CPicassoPosition*)pPos;
					//pPos = NULL;

					// way 2) or overwrite existing one? (it's just property modification, no memory leaks)
					CPicassoPosition* pCurPos = (CPicassoPosition*)pPos;
					pCurPos->m_nXX = Asc2Int(pIniValue->Values[1]);
					pCurPos->m_nYY = Asc2Int(pIniValue->Values[2]);
					pCurPos->m_nWid = Asc2Int(pIniValue->Values[3]);
					pCurPos->m_nHgt = Asc2Int(pIniValue->Values[4]);
				}
			}
			// end of [#RWC6-262]

			if (pPos == NULL)
			{
				pPos = new CPicassoPosition(pIniValue->Values[0], 
																Asc2Int(pIniValue->Values[1]),
																Asc2Int(pIniValue->Values[2]), 
																Asc2Int(pIniValue->Values[3]), 
																Asc2Int(pIniValue->Values[4]));

				/*m_PicassoResources.PutOnTailOfList(pPos->GetID(), pPos);*/
				m_PicassoResourcesCMap.SetAt(pPos->GetID(), pPos);

				NHUIDBG(DBG_CALL, (_T("POSITION Resource added (%s, %d, %d, %d, %d)"), pIniValue->Values[0], Asc2Int(pIniValue->Values[1]), 
					Asc2Int(pIniValue->Values[2]), 	 Asc2Int(pIniValue->Values[3]), Asc2Int(pIniValue->Values[4]) ) );
			}
		}
	}
	// End of [#2592]
	// [#RWC6-399] US ryan.payton 2022.09.15 DigitalMint
	// TextSet=ID,MultiLine,Align,Text Color
	else if (pIniValue->Key == DES_TEXTSET)
	{
		NHUIDBG(DBG_CALL, (_T("TEXTSET Resource (nValue = %d)"), pIniValue->nValue));

		if (pIniValue->nValue == 4)
		{
			pIniValue->Values[0].MakeUpper();

			CPicassoResource *pPos = NULL;

			// m_PicassoResources.FindDataOfList(pIniValue->Values[0], (CPicassoResource**)&pPos);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pPos);

			if (pPos == NULL)
			{
				pPos = new CPicassoTextSet(pIniValue->Values[0], Asc2Int(pIniValue->Values[1]), pIniValue->Values[2], pIniValue->Values[3]);

				/*m_PicassoResources.PutOnTailOfList(pPos->GetID(), pPos);*/
				m_PicassoResourcesCMap.SetAt(pPos->GetID(), pPos);

				NHUIDBG(DBG_CALL, (_T("TEXTSET Resource added (%s, %d, %s, %s)"), pIniValue->Values[0], Asc2Int(pIniValue->Values[1]), pIniValue->Values[2], pIniValue->Values[3]));
			}
		}
	}
	// End of [#RWC6-399]

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateScreen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateScreen(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	// ID =?
	if (pIniValue->Key == DES_ID)
	{
		// check valide.
		if (m_pNScreen)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoScreen don't Deallocate in SCREEN(%s)\n"), m_pNScreen->GetID()));
			return FALSE;
		}

#ifndef UNDER_CE
		CPicassoScreen *pTempScreen=NULL;
		pIniValue->Values[0].MakeUpper();
		//if (m_PicassoScreens.FindDataOfList(pIniValue->Values[0], &pTempScreen) != NULL)
		if (m_PicassoScreensCMap.Lookup(pIniValue->Values[0], pTempScreen) == TRUE)
		{
// 			MessageBox(pIniValue->Values[0], _T("Duplicated Screen Number"), MB_ICONWARNING | MB_OK);

			// OP screen #809 will encounter this (integrated OP.DAT has two screens, [SCREEN] and [SCREEN_640_480])
			NHDEBUG(1, (_T("duplicated screen found [%s], override with new one\n"), pIniValue->Values[0]));	

			// override (remove old one & create new one)
			delete pTempScreen;
			m_PicassoScreensCMap.RemoveKey(pIniValue->Values[0]);
		}
#endif

		// allocate.
		m_pNScreen = (CPicassoScreen*) new CPicassoScreen;
		if (!m_pNScreen)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoScreen allocate Failed in SCREEN\n")));
			return FALSE;
		}

		// Register.
		pIniValue->Values[0].MakeUpper();
		m_pNScreen->SetID(pIniValue->Values[0]);
		m_pNScreen->SetRequestQueue(&m_Requests);
		m_pNScreen->SetInvalidRect(&m_InvalidRect);

		//m_PicassoScreens.PutOnTailOfList(m_pNScreen->GetID(), m_pNScreen);
		NHUIDBG(1, (_T("[%dx%d] Screen Added: ID: %s\n"), m_nScreenWidth, m_nScreenHeight, m_pNScreen->GetID()));
		m_PicassoScreensCMap.SetAt(m_pNScreen->GetID(), m_pNScreen);

	}
	// Back=ID
	else if (pIniValue->Key == DES_BACK)
	{
		// check valide.
		if (!m_pNScreen)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoScreen don't allocate in SCREEN\n")));
			return FALSE;
		}

		CPicassoScreen	*pBackScreen = NULL;
		pIniValue->Values[0].MakeUpper();
		//if (m_PicassoScreens.FindDataOfList(pIniValue->Values[0], &pBackScreen))
		if (m_PicassoScreensCMap.Lookup(pIniValue->Values[0], pBackScreen) == TRUE)
			m_pNScreen->SetBackScreen(pBackScreen);		
	}
	// [SCREEN][/SCREEN]
	if (pIniValue->Key == DES_SCREEN)
	{
		// END..
		if(!Asc2Int(pIniValue->Values[0]))
		{
			if (!m_pNScreen)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoScreen don't allocate in SCREEN\n")));
				return FALSE;
			}

			m_pNScreen = NULL;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateTextBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox need to SCREEN. TEXTBOX\n")));
		return FALSE;
	}

	// [TEXTBOX][/TEXTBOX]
	if (pIniValue->Key == DES_TEXTBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't Deallocate in TEXTBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNTextBox = (CPicassoTextBox*) new CPicassoTextBox;
			if (!m_pNTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox allocate Failed in TEXTBOX\n")));
				return FALSE;
			}

			// [#2434] AU Kook 2016.07.18 Speed Up AP Loading
			//m_pNTextBox->SetResourcesQueue(&m_PicassoResources);
//			m_pNTextBox->SetResourcesQueue(&m_PicassoResourcesText);
			m_pNTextBox->SetResourcesQueue(&m_PicassoResourcesCMap);
			// end of [#2434]

			// Register.
			m_pNScreen->SetAddControl(m_pNTextBox);
		}
		else
		{
			if (!m_pNTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't allocate in TEXTBOX\n")));
				return FALSE;
			}
			
			m_pNTextBox = NULL;
		}
	}
	else
	{
		if (!m_pNTextBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't allocate 2 in TEXTBOX\n")));
			return FALSE;
		}

		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pNTextBox, pIniValue))
			return TRUE;

		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pNTextBox, pIniValue))
			return TRUE;

		// Setting Text.
		if (SettingText(m_pNTextBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateHideTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateHideTextBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoHideTextBox need to SCREEN. HIDETEXTBOX\n")));
		return FALSE;
	}

	// [TEXTBOX][/TEXTBOX]
	if (pIniValue->Key == DES_HIDETEXTBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNHideTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't Deallocate in HIDETEXTBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNHideTextBox = (CPicassoHideTextBox*) new CPicassoHideTextBox;
			if (!m_pNHideTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoHideTextBox allocate Failed in HIDETEXTBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNHideTextBox);
		}
		else
		{
			if (!m_pNHideTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoHideTextBox don't allocate in HIDETEXTBOX\n")));
				return FALSE;
			}
			
			m_pNHideTextBox = NULL;
		}
	}
	else
	{
		if (!m_pNHideTextBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't allocate 2 in HIDETEXTBOX\n")));
			return FALSE;
		}

		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pNHideTextBox, pIniValue))
			return TRUE;

		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pNHideTextBox, pIniValue))
			return TRUE;

		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pNHideTextBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateEffectTextBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateEffectTextBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox need to SCREEN. TEXTBOX\n")));
		return FALSE;
	}

	// [TEXTBOX][/TEXTBOX]
	if (pIniValue->Key == DES_EFFECTTEXTBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNEffectTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't Deallocate in EFFECTTEXTBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNEffectTextBox = (CPicassoEffectTextBox*) new CPicassoEffectTextBox;
			if (!m_pNEffectTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox allocate Failed in EFFECTTEXTBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNEffectTextBox);
		}
		else
		{
			if (!m_pNEffectTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't allocate in EFFECTTEXTBOX\n")));
				return FALSE;
			}
			
			m_pNEffectTextBox = NULL;
		}
	}
	else
	{
		if (!m_pNEffectTextBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't allocate 2 in EFFECTTEXTBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_EFFECT)
		{
			if (pIniValue->Values[0] == DES_SLIDING && pIniValue->Values[1] == DES_LEFT)
				m_pNEffectTextBox->SetEffectType(EFFECT_SLIDING_LEFT);
			else if (pIniValue->Values[0] == DES_SLIDING && pIniValue->Values[1] == DES_RIGHT)
				m_pNEffectTextBox->SetEffectType(EFFECT_SLIDING_RIGHT);
			else if (pIniValue->Values[0] == DES_TYPING && pIniValue->Values[1] == DES_LEFT)
				m_pNEffectTextBox->SetEffectType(EFFECT_TYPING_LEFT);
			else if (pIniValue->Values[0] == DES_TYPING && pIniValue->Values[1] == DES_CENTER)
				m_pNEffectTextBox->SetEffectType(EFFECT_TYPING_CENTER);
		}
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNEffectTextBox, pIniValue))
				return TRUE;

			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNEffectTextBox, pIniValue))
				return TRUE;

			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNEffectTextBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateEditBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateEditBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoEditBox need to SCREEN. EDITBOX\n")));
		return FALSE;
	}

	// [EDITBOX][/EDITBOX]
	if (pIniValue->Key == DES_EDITBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNEditTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoEditBox don't Deallocate in EDITBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNEditTextBox = (CPicassoEditTextBox*) new CPicassoEditTextBox;
			if (!m_pNEditTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoEditBox allocate Failed in EDITBOX\n")));
				return FALSE;
			}

			// [#2434] AU Kook 2016.07.18 Speed Up AP Loading
			//m_pNEditTextBox->SetResourcesQueue(&m_PicassoResources);
//			m_pNEditTextBox->SetResourcesQueue(&m_PicassoResourcesText);
			m_pNEditTextBox->SetResourcesQueue(&m_PicassoResourcesCMap);
			// end of [#2434]

			// Register.
			m_pNScreen->SetAddControl(m_pNEditTextBox);
		}
		else
		{
			if (!m_pNEditTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoEditBox don't allocate in EDITBOX\n")));
				return FALSE;
			}
			
			m_pNEditTextBox = NULL;
		}
	}
	else
	{
		if (!m_pNEditTextBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoEditBox don't allocate 2 in EDITBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_MAXINCHAR)
		{
			m_pNEditTextBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
		}
		// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
		else if(pIniValue->Key == DES_MINCHAR)
		{
			m_pNEditTextBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
		}
		// end of [#182]
		else if (pIniValue->Key == DES_TYPE)
		{
			if (pIniValue->Values[0] == DES_PASSWORD)
				m_pNEditTextBox->SetTextType(TTYPE_PASSWORD);
			else if (pIniValue->Values[0] == DES_ADDRESS)				// [#470] NH AIREAT 2008.12.22 : IP ADDRESS Type 추가.
				m_pNEditTextBox->SetTextType(TTYPE_ADDRESS);
		}
		else if (pIniValue->Key == DES_AUTO_RUN)
		{
			m_pNEditTextBox->SetAutoRun(Asc2Int(pIniValue->Values[0]));
		}
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNEditTextBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNEditTextBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNEditTextBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateChangePWBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateChangePWBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoChangePWBox need to SCREEN. CHANGEPWBOX\n")));
		return FALSE;
	}

	// [EDITBOX][/EDITBOX]
	if (pIniValue->Key == DES_CHANGE_PW_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNChangePWBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoChangePWBox don't Deallocate in EDITBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNChangePWBox = (CPicassoChangePasswordBox*) new CPicassoChangePasswordBox;
			if (!m_pNChangePWBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoChangePWBox allocate Failed in EDITBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNChangePWBox);
		}
		else
		{
			if (!m_pNChangePWBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoChangePWBox don't allocate in EDITBOX\n")));
				return FALSE;
			}
			
			m_pNChangePWBox = NULL;
		}
	}
	else
	{
		if (!m_pNChangePWBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoChangePWBox don't allocate 2 in EDITBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_MAXINCHAR)
		{
			m_pNChangePWBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
		}
		// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
		else if(pIniValue->Key == DES_MINCHAR)
		{
			m_pNChangePWBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
		}
		// end of [#182]
		else if (pIniValue->Key == DES_POS_CUR_BOX && pIniValue->nValue == 4)
		{
			m_pNChangePWBox->SetPWBoxRect(CPTYPE_CUR,
									Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
									Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
									m_LoadScreenSize.cx, m_LoadScreenSize.cy);
		}
		else if (pIniValue->Key == DES_POS_NEW_BOX && pIniValue->nValue == 4)
		{
			m_pNChangePWBox->SetPWBoxRect(CPTYPE_NEW,
									Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
									Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
									m_LoadScreenSize.cx, m_LoadScreenSize.cy);
		}
		else if (pIniValue->Key == DES_POS_NEW_RE_BOX && pIniValue->nValue == 4)
		{
			m_pNChangePWBox->SetPWBoxRect(CPTYPE_NEWRE,
									Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
									Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
									m_LoadScreenSize.cx, m_LoadScreenSize.cy);
		}
		// [#16] NH KGS 2008.03.13 Password change
		else if (pIniValue->Key == DES_AUTO_RUN)
		{
			m_pNChangePWBox->SetAutoRun(Asc2Int(pIniValue->Values[0]));
		}
		// end of [#16]
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNChangePWBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNChangePWBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNChangePWBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateButtonBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateButtonBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoButtonBox need to SCREEN. BUTTONBOX\n")));
		return FALSE;
	}

	// [BUTTON][/BUTTON]
	if (pIniValue->Key == DES_BUTTON)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNButtonBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoButtonBox don't Deallocate in BUTTONBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNButtonBox = (CPicassoButtonBox*) new CPicassoButtonBox;
			if (!m_pNButtonBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoButtonBox allocate Failed in BUTTONBOX\n")));
				return FALSE;
			}

			// [#2434] AU Kook 2016.07.18 Speed Up AP Loading
			//m_pNButtonBox->SetResourcesQueue(&m_PicassoResources);
//			m_pNButtonBox->SetResourcesQueue(&m_PicassoResourcesText);
			m_pNButtonBox->SetResourcesQueue(&m_PicassoResourcesCMap);
			// end of [#2434]

			// Register.
			m_pNScreen->SetAddControl(m_pNButtonBox);
		}
		else
		{
			if (!m_pNButtonBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoButtonBox don't allocate in BUTTONBOX\n")));
				return FALSE;
			}
			
			m_pNButtonBox = NULL;
		}
	}
	else
	{
		if (!m_pNButtonBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoButtonBox don't allocate 2 in BUTTONBOX\n")));
			return FALSE;
		}
		
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pNButtonBox, pIniValue))
			return TRUE;
		
		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pNButtonBox, pIniValue))
			return TRUE;
		
		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pNButtonBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateVKeyButtonBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateVKeyButtonBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoVKeyButtonBox need to SCREEN. VKEYBUTTONBOX\n")));
		return FALSE;
	}

	// [VKEYBUTTON][/VKEYBUTTON]
	if (pIniValue->Key == DES_VKEYBUTTON)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNVKeyButtonBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoVKeyButtonBox don't Deallocate in BUTTONBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNVKeyButtonBox = (CPicassoVirtualKeyButtonBox*) new CPicassoVirtualKeyButtonBox;
			if (!m_pNVKeyButtonBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoVKeyButtonBox allocate Failed in VKEYBUTTONBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNVKeyButtonBox);
		}
		else
		{
			if (!m_pNVKeyButtonBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoVKeyButtonBox don't allocate in VKEYBUTTONBOX\n")));
				return FALSE;
			}
			
			m_pNVKeyButtonBox = NULL;
		}
	}
	else
	{
		if (!m_pNVKeyButtonBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoVKeyButtonBox don't allocate 2 in VKEYBUTTONBOX\n")));
			return FALSE;
		}
		
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pNVKeyButtonBox, pIniValue))
			return TRUE;
		
		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pNVKeyButtonBox, pIniValue))
			return TRUE;
		
		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pNVKeyButtonBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateSWKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateSWKeyBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	SIZE	szTemp;
	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoSWKeyBox need to SCREEN. SWKEYBOX\n")));
		return FALSE;
	}

	// [SWKEY][/SWKEY]
	if (pIniValue->Key == DES_SW_KEY)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNSWKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoSWKeyBox don't Deallocate in SWKEYBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNSWKeyBox = (CPicassoSoftwareKeyBox*) new CPicassoSoftwareKeyBox;
			if (!m_pNSWKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoSWKeyBox allocate Failed in SWKEYBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNSWKeyBox);
		}
		else
		{
			if (!m_pNSWKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoSWKeyBox don't allocate in SWKEYBOX\n")));
				return FALSE;
			}
			
			m_pNSWKeyBox = NULL;
		}
	}
	else
	{
		if (!m_pNSWKeyBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoSWKeyBox don't allocate 2 in SWKEYBOX\n")));
			return FALSE;
		}
		
		// NumberSize=?,?
		if (pIniValue->Key == DES_NUMBER_SIZE)
		{
			if (pIniValue->nValue == 2)
			{
				szTemp.cx = Asc2Int(pIniValue->Values[0]);
				szTemp.cy = Asc2Int(pIniValue->Values[1]);

				// Adjust Scale
				{
					int currentX = m_nScreenWidth;
					int currentY = m_nScreenHeight;
		
					float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);

					// [#2300] US KSK 2014.11.04					
					float ScreenRateY = 0;

					if (currentX == 1024 && currentY == 768)
						ScreenRateY = (float)((float)MX5200SE_VIRTUAL_SCREEN_Y / (float)BASIC_SCREEN_Y);		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
					else
						ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);
					// end of [#2300]

					if (ScreenRateX > ScreenRateY)
					{
						szTemp.cx = (int)((float)ScreenRateX * szTemp.cx);
						szTemp.cy = (int)((float)ScreenRateX * szTemp.cy);
					}
					else
					{
						szTemp.cx = (int)((float)ScreenRateY * szTemp.cx);
						szTemp.cy = (int)((float)ScreenRateY * szTemp.cy);
					}
				}

				m_pNSWKeyBox->SetNumberSize(szTemp);
			}
		}
		// GuideSize=?,?
		else if (pIniValue->Key == DES_GUIDE_SIZE)
		{
			if (pIniValue->nValue == 2)
			{
				szTemp.cx = Asc2Int(pIniValue->Values[0]);
				szTemp.cy = Asc2Int(pIniValue->Values[1]);

				// Adjust Scale
				{
					int currentX = m_nScreenWidth;
					int currentY = m_nScreenHeight;
		
					float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);

					// [#2300] US KSK 2014.11.03
					float ScreenRateY = 0;

					if (currentX == 1024 && currentY == 768)
						ScreenRateY = (float)((float)MX5200SE_VIRTUAL_SCREEN_Y / (float)BASIC_SCREEN_Y);		// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
					else
						ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);
					// end of [#2300]

					if (ScreenRateX > ScreenRateY)
					{
						szTemp.cx = (int)((float)ScreenRateX * szTemp.cx);
						szTemp.cy = (int)((float)ScreenRateX * szTemp.cy);
					}
					else
					{
						szTemp.cx = (int)((float)ScreenRateY * szTemp.cx);
						szTemp.cy = (int)((float)ScreenRateY * szTemp.cy);
					}
				}

				m_pNSWKeyBox->SetGuideSize(szTemp);
			}
		}
		// SpaceSize=?,?
		else if (pIniValue->Key == DES_SPACE_SIZE)
		{
			if (pIniValue->nValue == 2)
			{
				szTemp.cx = Asc2Int(pIniValue->Values[0]);
				szTemp.cy = Asc2Int(pIniValue->Values[1]);

				m_pNSWKeyBox->SetSpaceSize(szTemp);
			}
		}
		else
		{
			
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNSWKeyBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNSWKeyBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNSWKeyBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateEnDisableBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateEnDisableBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoEnDisableBox need to SCREEN. ENABLEBOX\n")));
		return FALSE;
	}

	// [ENABLEBOX][/ENABLEBOX]
	if (pIniValue->Key == DES_TOGGLE_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNToggleBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoEnDisableBox don't Deallocate in ENABLEBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNToggleBox = (CPicassoToggleBox*) new CPicassoToggleBox;
			if (!m_pNToggleBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoEnDisableBox allocate Failed in ENABLEBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNToggleBox);
		}
		else
		{
			if (!m_pNToggleBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoEnDisableBox don't allocate in ENABLEBOX\n")));
				return FALSE;
			}
			
			m_pNToggleBox = NULL;
		}
	}
	else
	{
		if (!m_pNToggleBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoEnDisableBox don't allocate 2 in ENABLEBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_ENABLE_TEXT)
		{
			m_pNToggleBox->SetEnableText(pIniValue->Values[0]);
		}
		else if (pIniValue->Key == DES_DISABLE_TEXT)
		{
			m_pNToggleBox->SetDisableText(pIniValue->Values[0]);
		}
		else
		{			
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNToggleBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNToggleBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNToggleBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateEJNLBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateEJNLBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoEJNLBox need to SCREEN. EJNLBOX\n")));
		return FALSE;
	}

	// [EJNLBOX][/EJNLBOX]
	if (pIniValue->Key == DES_EJNL_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNEJNLBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoEJNLBox don't Deallocate in EJNLBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNEJNLBox = (CPicassoEJNLBox*) new CPicassoEJNLBox;
			if (!m_pNEJNLBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoEJNLBox allocate Failed in EJNLBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNEJNLBox);
		}
		else
		{
			if (!m_pNEJNLBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoEJNLBox don't allocate in EJNLBOX\n")));
				return FALSE;
			}
			
			m_pNEJNLBox = NULL;
		}
	}
	else
	{
		if (!m_pNEJNLBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoEJNLBox don't allocate 2 in EJNLBOX\n")));
			return FALSE;
		}
					
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pNEJNLBox, pIniValue))
			return TRUE;
		
		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pNEJNLBox, pIniValue))
			return TRUE;
		
		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pNEJNLBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateDateBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateDateBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoDateBox need to SCREEN. DATEBOX\n")));
		return FALSE;
	}

	// [EJNLBOX][/EJNLBOX]
	if (pIniValue->Key == DES_DATE_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNDateBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoDateBox don't Deallocate in DATEBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNDateBox = (CPicassoDateBox*) new CPicassoDateBox;
			if (!m_pNDateBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoDateBox allocate Failed in DATEBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNDateBox);
		}
		else
		{
			if (!m_pNDateBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoDateBox don't allocate in DATEBOX\n")));
				return FALSE;
			}
			
			m_pNDateBox = NULL;
		}
	}
	else
	{
		if (!m_pNDateBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoDateBox don't allocate 2 in DATEBOX\n")));
			return FALSE;
		}
					
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pNDateBox, pIniValue))
			return TRUE;
		
		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pNDateBox, pIniValue))
			return TRUE;
		
		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pNDateBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateIPBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateIPBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoIPBox need to SCREEN. IPBOX\n")));
		return FALSE;
	}

	// [EJNLBOX][/EJNLBOX]
	if (pIniValue->Key == DES_IP_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNIPBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoIPBox don't Deallocate in IPBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNIPBox = (CPicassoIPBox*) new CPicassoIPBox;
			if (!m_pNIPBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoIPBox allocate Failed in IPBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNIPBox);
		}
		else
		{
			if (!m_pNIPBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoIPBox don't allocate in IPBOX\n")));
				return FALSE;
			}
			
			m_pNIPBox = NULL;
		}
	}
	else
	{
		if (!m_pNIPBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoIPBox don't allocate 2 in IPBOX\n")));
			return FALSE;
		}
					
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pNIPBox, pIniValue))
			return TRUE;
		
		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pNIPBox, pIniValue))
			return TRUE;
		
		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pNIPBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateBinBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateBinBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoBinBox need to SCREEN. BINBOX\n")));
		return FALSE;
	}

	// [BINBOX][/BINBOX]
	if (pIniValue->Key == DES_BIN_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNBinBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoBinBox don't Deallocate in BINBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNBinBox = (CPicassoBinBox*) new CPicassoBinBox;
			if (!m_pNBinBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoBinBox allocate Failed in BINBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNBinBox);
		}
		else
		{
			if (!m_pNBinBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoBinBox don't allocate in BINBOX\n")));
				return FALSE;
			}
			
			m_pNBinBox = NULL;
		}
	}
	else
	{
		if (!m_pNBinBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoBinBox don't allocate 2 in BINBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_POS_INDEX && pIniValue->nValue == 4)
		{
			m_pNBinBox->SetBinBoxRect(BNTYPE_INDEX,
									Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
									Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
									m_LoadScreenSize.cx, m_LoadScreenSize.cy);
		}
		else if (pIniValue->Key == DES_POS_BIN && pIniValue->nValue == 4)
		{
			m_pNBinBox->SetBinBoxRect(BNTYPE_BIN,
									Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
									Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
									m_LoadScreenSize.cx, m_LoadScreenSize.cy);
		}
		else
		{
			
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNBinBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNBinBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNBinBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateCentBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateCentBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoCentBox need to SCREEN. CENTBOX\n")));
		return FALSE;
	}

	// [CENTBOX][/CENTBOX]
	if (pIniValue->Key == DES_CENT_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNCentBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoCentBox don't Deallocate in CENTBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNCentBox = (CPicassoCentBox*) new CPicassoCentBox;
			if (!m_pNCentBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoCentBox allocate Failed in CENTBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNCentBox);
		}
		else
		{
			if (!m_pNCentBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoCentBox don't allocate in CENTBOX\n")));
				return FALSE;
			}
			
			m_pNCentBox = NULL;
		}
	}
	else
	{
		if (!m_pNCentBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoCentBox don't allocate 2 in CENTBOX\n")));
			return FALSE;
		}
		
		if (pIniValue->Key == DES_MAXINCHAR)
		{
			m_pNCentBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
		}
		// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
		else if(pIniValue->Key == DES_MINCHAR)
		{
			m_pNCentBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
		}
		// end of [#182]
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNCentBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNCentBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNCentBox, pIniValue))
				return TRUE;
		}
		
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateDollarBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateDollarBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoDollarBox need to SCREEN. DOLLARBOX\n")));
		return FALSE;
	}

	// [DOLLARBOX][/DOLLARBOX]
	if (pIniValue->Key == DES_DOLLAR_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNDollarBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoDollarBox don't Deallocate in DOLLARBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNDollarBox = (CPicassoDollarBox*) new CPicassoDollarBox;
			if (!m_pNDollarBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] m_pNDollarBox allocate Failed in DOLLARBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNDollarBox);
		}
		else
		{
			if (!m_pNDollarBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] NDollarBox don't allocate in DOLLARBOX\n")));
				return FALSE;
			}
			
			m_pNDollarBox = NULL;
		}
	}
	else
	{
		if (!m_pNDollarBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] NDollarBox don't allocate 2 in DOLLARBOX\n")));
			return FALSE;
		}
		
		if (pIniValue->Key == DES_MAXINCHAR)
		{
			m_pNDollarBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
		}
		// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
		else if (pIniValue->Key == DES_MINCHAR)
		{
			m_pNDollarBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
		}
		// end of [#182]
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNDollarBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNDollarBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNDollarBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateAdvBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateAdvBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoAdvBox need to SCREEN. ADVBOX\n")));
		return FALSE;
	}

	// [ADVBOX][/ADVBOX]
	if (pIniValue->Key == DES_ADV_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNAdvBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAdvBox don't Deallocate in ADVBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNAdvBox = (CPicassoAdvBox*) new CPicassoAdvBox;
			if (!m_pNAdvBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAdvBox allocate Failed in ADVBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNAdvBox);
		}
		else
		{
			if (!m_pNAdvBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAdvBox don't allocate in ADVBOX\n")));
				return FALSE;
			}
			
			m_pNAdvBox = NULL;
		}
	}
	else
	{
		// [#478] NH AIREAT 2009.01.07 : 광고 화면 해상도별 지원
		CString	strFile;
		strFile.Format(_T("%s_%d_%d"), DES_FILE, m_nScreenWidth, m_nScreenHeight);
		// end of [#478]

		if (!m_pNAdvBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoAdvBox don't allocate 2 in ADVBOX\n")));
			return FALSE;
		}
		
		if (pIniValue->Key == strFile)
		{
			m_pNAdvBox->SetFileName(pIniValue->Values[0]);
		}
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNAdvBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNAdvBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNAdvBox, pIniValue))
				return TRUE;
		}
		
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateAdvBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateTranAdvBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoTranAdvBox need to SCREEN. DES_TRANADV_BOX\n")));
		return FALSE;
	}

	// [DES_TRANADV_BOX][/DES_TRANADV_BOX]
	if (pIniValue->Key == DES_TRANADV_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pTranAdvBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTranAdvBox don't Deallocate in DES_TRANADV_BOX\n")));
				return FALSE;
			}

			// allocate.
			m_pTranAdvBox = (CPicassoTranAdvBox*) new CPicassoTranAdvBox;
			if (!m_pTranAdvBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTranAdvBox allocate Failed in DES_TRANADV_BOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pTranAdvBox);
		}
		else
		{
			if (!m_pTranAdvBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTranAdvBox don't allocate in DES_TRANADV_BOX\n")));
				return FALSE;
			}
			
			m_pTranAdvBox = NULL;
		}
	}
	else
	{
		if (!m_pTranAdvBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoTranAdvBox don't allocate 2 in DES_TRANADV_BOX\n")));
			return FALSE;
		}
		
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pTranAdvBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pTranAdvBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pTranAdvBox, pIniValue))
				return TRUE;
		}
		
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateAniBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateAniBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoAniBox need to SCREEN. ANIBOX\n")));
		return FALSE;
	}

	// [ANIBOX][/ANIBOX]
	if (pIniValue->Key == DES_ANI_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNAniBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAniBox don't Deallocate in ANIBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNAniBox = (CPicassoAnimation*) new CPicassoAnimation;
			if (!m_pNAniBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAniBox allocate Failed in ANIBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNAniBox);
		}
		else
		{
			if (!m_pNAniBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAniBox don't allocate in ANIBOX\n")));
				return FALSE;
			}
			
			m_pNAniBox = NULL;
		}
	}
	else
	{
		CString	strFrame;
		strFrame.Format(_T("%s_%d_%d"), DES_FRAME, m_nScreenWidth, m_nScreenHeight);

		if (!m_pNAniBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoAniBox don't allocate 2 in ANIBOX\n")));
			return FALSE;
		}
		
		if (pIniValue->Key == strFrame)
		{
			CString strFilePrefix;
			strFilePrefix = m_strScreenPath + pIniValue->Values[1];
			
			if (pIniValue->nValue == 2)
				m_pNAniBox->SetAniImage(Asc2Int(pIniValue->Values[0]), strFilePrefix, _T("JPG"));
			else if (pIniValue->nValue >= 3)
				m_pNAniBox->SetAniImage(Asc2Int(pIniValue->Values[0]), strFilePrefix, pIniValue->Values[2]);
		}
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNAniBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateTableEditBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateTableEditBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoTableEditBox need to SCREEN. TABLEEDITBOX\n")));
		return FALSE;
	}

	// [TABLEEDITBOX][/TABLEEDITBOX]
	if (pIniValue->Key == DES_TABLE_EDITBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNTableEditBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTableEditBox don't Deallocate in TABLEEDITBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNTableEditBox = (CPicassoTableEditTextBox*) new CPicassoTableEditTextBox;
			if (!m_pNTableEditBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTableEditBox allocate Failed in TABLEEDITBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNTableEditBox);
		}
		else
		{
			if (!m_pNTableEditBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTableEditBox don't allocate in TABLEEDITBOX\n")));
				return FALSE;
			}
			
			m_pNTableEditBox = NULL;
		}
	}
	else
	{
		CString	strFrame;
		strFrame.Format(_T("%s_%d_%d"), DES_FRAME, m_nScreenWidth, m_nScreenHeight);

		if (!m_pNTableEditBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoTableEditBox don't allocate 2 in TABLEEDITBOX\n")));
			return FALSE;
		}
		
		//CPicassoResource *pNormal = NULL, *pSelect = NULL;
		CPicassoResource	*pDisable=NULL, *pEnable=NULL, *pSelect=NULL;

		// AllowRowColumn=row,column
		if (pIniValue->Key == DES_ALLOW_ROW_COL)
		{
			if (pIniValue->nValue == 2)
				m_pNTableEditBox->SetAllowAmount(Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]));
		}
		// GuideTextColor=normal,select
		else if (pIniValue->Key == DES_GUIDE_TEXT_COLOR)
		{
			// normal
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pEnable);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pEnable);

			}
			// select
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);
			}

			m_pNTableEditBox->SetGuideTextColor((CPicassoColor*)pEnable, (CPicassoColor*)pSelect);
		}
		// GuideFont=normal,select
		else if (pIniValue->Key == DES_GUIDE_FONT)
		{
			// normal
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pEnable);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pEnable);
			}
			// select
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);
			}

			m_pNTableEditBox->SetGuideFont(pEnable, pSelect);
		}
		// ElementPen=normal
		else if (pIniValue->Key == DES_ELEMENT_PEN)
		{
			// Disable
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pDisable);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pDisable);
			}
			// Enable
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pEnable);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pEnable);
			}
			// Select
			if (pIniValue->nValue >= 3)
			{
				pIniValue->Values[2].MakeUpper();
				if (pIniValue->Values[2] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[2], &pSelect);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[2], pSelect);
			}

			m_pNTableEditBox->SetElementPen(pDisable, pEnable, pSelect);
		}
		// ElementFont=disable,enable,select
		else if (pIniValue->Key == DES_ELEMENT_FONT)
		{
			// Disable
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pDisable);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pDisable);
			}
			// Enable
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pEnable);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pEnable);

			}
			// Select
			if (pIniValue->nValue >= 3)
			{
				pIniValue->Values[2].MakeUpper();
				if (pIniValue->Values[2] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[2], &pSelect);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[2], pSelect);
			}

			m_pNTableEditBox->SetElementFont(pDisable, pEnable, pSelect);
		}
		// ElementBrush=disable,enable,select
		else if (pIniValue->Key == DES_ELEMENT_BRUSH)
		{
			// Disable
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pDisable);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pDisable);
			}
			// Enable
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pEnable);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pEnable);
			}
			// Select
			if (pIniValue->nValue >= 3)
			{
				pIniValue->Values[2].MakeUpper();
				if (pIniValue->Values[2] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[2], &pSelect);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[2], pSelect);

			}

			m_pNTableEditBox->SetElementBrush(pDisable, pEnable, pSelect);
		}
		// ElementTextColor=disable,enable,select
		else if (pIniValue->Key == DES_ELEMENT_TEXT_COLOR)
		{
			// Disable
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pDisable);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pDisable);

			}
			// Enable
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pEnable);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pEnable);

			}
			// Select
			if (pIniValue->nValue >= 3)
			{
				pIniValue->Values[2].MakeUpper();
				if (pIniValue->Values[2] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[2], &pSelect);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[2], pSelect);

			}

			m_pNTableEditBox->SetElementTextColor((CPicassoColor*)pDisable, (CPicassoColor*)pEnable, (CPicassoColor*)pSelect);
		}
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNTableEditBox, pIniValue))
				return TRUE;

			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNTableEditBox, pIniValue))
				return TRUE;
		}		
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateAlphaKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateAlphaKeyBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoAlphaKeyBox need to SCREEN. ALPHAKEYBOX\n")));
		return FALSE;
	}

	// [ALPHAKEYBOX][/ALPHAKEYBOX]
	if (pIniValue->Key == DES_ALPHA_KEYBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNAlphaKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAlphaKeyBox don't Deallocate in ALPHAKEYBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNAlphaKeyBox = (CPicassoAlphaKeyBox*) new CPicassoAlphaKeyBox;
			if (!m_pNAlphaKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAlphaKeyBox allocate Failed in ALPHAKEYBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNAlphaKeyBox);
		}
		else
		{
			if (!m_pNAlphaKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAlphaKeyBox don't allocate in ALPHAKEYBOX\n")));
				return FALSE;
			}
			
			m_pNAlphaKeyBox = NULL;
		}
	}
	else
	{
		if (!m_pNAlphaKeyBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoAlphaKeyBox don't allocate 2 in ALPHAKEYBOX\n")));
			return FALSE;
		}
		
		if (pIniValue->Key == DES_VKEY_TYPE)
		{
			CString strVKeyType = pIniValue->Values[0];

			strVKeyType.MakeUpper();
			if (strVKeyType == _T("UPPER"))
				m_pNAlphaKeyBox->SetAlphaKeyType(VKEY_ALPHA_UPPER);
			else if (strVKeyType == _T("LOWER"))
				m_pNAlphaKeyBox->SetAlphaKeyType(VKEY_ALPHA_LOWER);
			else if (strVKeyType == _T("NUMBER"))
				m_pNAlphaKeyBox->SetAlphaKeyType(VKEY_ALPHA_NUMBER);
		}
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNAlphaKeyBox, pIniValue))
				return TRUE;

			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNAlphaKeyBox, pIniValue))
				return TRUE;

			// Setting Virtual Key
			if (SettingVKey((CPicassoVirtualKeyBox*)m_pNAlphaKeyBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateNumberKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateNumberKeyBox(PINIVALUE pIniValue)
{
	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoNumberBox need to SCREEN. NUMBERKEYBOX\n")));
		return FALSE;
	}

	// [NUMBERKEYBOX][/NUMBERKEYBOX]
	if (pIniValue->Key == DES_NUMBER_KEYBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNNumberKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoNumberBox don't Deallocate in NUMBERKEYBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNNumberKeyBox = (CPicassoNumberKeyBox*) new CPicassoNumberKeyBox;
			if (!m_pNNumberKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoNumberBox allocate Failed in NUMBERKEYBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNNumberKeyBox);
		}
		else
		{
			if (!m_pNNumberKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoNumberBox don't allocate in NUMBERKEYBOX\n")));
				return FALSE;
			}
			
			m_pNNumberKeyBox = NULL;
		}
	}
	else
	{
		if (!m_pNNumberKeyBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoNumberBox don't allocate 2 in NUMBERKEYBOX\n")));
			return FALSE;
		}
		
		if (pIniValue->Key == DES_VKEY_TYPE)
		{
			CString strVKeyType = pIniValue->Values[0];

			strVKeyType.MakeUpper();
			if (strVKeyType == _T("TELENUMBER"))
				m_pNNumberKeyBox->SetNumberKeyType(NUMBER_KEY_TELE_NUMBER);
			else
				m_pNNumberKeyBox->SetNumberKeyType(NUMBER_KEY_NUMBER);
		}
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNNumberKeyBox, pIniValue))
				return TRUE;

			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNNumberKeyBox, pIniValue))
				return TRUE;

			// Setting Virtual Key
			if (SettingVKey((CPicassoVirtualKeyBox*)m_pNNumberKeyBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateTableKeyBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateTableKeyBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoTableKeyBox need to SCREEN. TABLEKEYBOX\n")));
		return FALSE;
	}

	// [TABLEKEYBOX][/TABLEKEYBOX]
	if (pIniValue->Key == DES_TABLE_KEYBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNTableKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTableKeyBox don't Deallocate in TABLEKEYBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNTableKeyBox = (CPicassoTableKeyBox*) new CPicassoTableKeyBox;
			if (!m_pNTableKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTableKeyBox allocate Failed in TABLEKEYBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNTableKeyBox);
		}
		else
		{
			if (!m_pNTableKeyBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTableKeyBox don't allocate in TABLEKEYBOX\n")));
				return FALSE;
			}
			
			m_pNTableKeyBox = NULL;
		}
	}
	else
	{
		if (!m_pNTableKeyBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoTableKeyBox don't allocate 2 in TABLEKEYBOX\n")));
			return FALSE;
		}
		
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pNTableKeyBox, pIniValue))
			return TRUE;

		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pNTableKeyBox, pIniValue))
			return TRUE;

		// Setting Virtual Key
		if (SettingVKey((CPicassoVirtualKeyBox*)m_pNTableKeyBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateThreeStateBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateThreeStateBox(PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoThreeStateBox need to SCREEN. THREESTATEBOX\n")));
		return FALSE;
	}

	// [THREESTATEBOX][/THREESTATEBOX]
	if (pIniValue->Key == DES_THREE_STATEBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNThreeStateBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoThreeStateBox don't Deallocate in THREESTATEBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNThreeStateBox = (CPicassoThreeStateBox*) new CPicassoThreeStateBox;
			if (!m_pNThreeStateBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoThreeStateBox allocate Failed in THREESTATEBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNThreeStateBox);
		}
		else
		{
			if (!m_pNThreeStateBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoThreeStateBox don't allocate in THREESTATEBOX\n")));
				return FALSE;
			}
			
			m_pNThreeStateBox = NULL;
		}
	}
	else
	{
		CPicassoResource *pHide = NULL, *pOn = NULL, *pOff = NULL;

		if (!m_pNThreeStateBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoThreeStateBox don't allocate 2 in THREESTATEBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_STATE_IMAGE)
		{
			// Hide
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pHide);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pHide);

			}
			
			// On
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pOn);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pOn);
			}

			// Off
			if (pIniValue->nValue >= 3)
			{
				pIniValue->Values[2].MakeUpper();
				if (pIniValue->Values[2] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[2], &pOff);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[2], pOff);
			}

			m_pNThreeStateBox->SetStateImage((CPicassoPicture*)pHide, (CPicassoPicture*)pOn, (CPicassoPicture*)pOff);
		}
		else if (pIniValue->Key == DES_STATE_PEN)
		{
			// Hide
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pHide);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pHide);
			}
			
			// On
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pOn);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pOn);
			}

			// Off
			if (pIniValue->nValue >= 3)
			{
				pIniValue->Values[2].MakeUpper();
				if (pIniValue->Values[2] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[2], &pOff);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[2], pOff);
			}

			m_pNThreeStateBox->SetStatePen(pHide, pOn, pOff);
		}
		else if (pIniValue->Key == DES_STATE_BRUSH)
		{
			// Hide
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pHide);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pHide);

			}
			
			// On
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pOn);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pOn);

			}

			// Off
			if (pIniValue->nValue >= 3)
			{
				pIniValue->Values[2].MakeUpper();
				if (pIniValue->Values[2] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[2], &pOff);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[2], pOff);
			}

			m_pNThreeStateBox->SetStateBrush(pHide, pOn, pOff);
		}
		else if (pIniValue->Key == DES_STATE_FONT)
		{
			// Hide
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pHide);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pHide);

			}
			
			// On
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pOn);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pOn);

			}

			// Off
			if (pIniValue->nValue >= 3)
			{
				pIniValue->Values[2].MakeUpper();
				if (pIniValue->Values[2] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[2], &pOff);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[2], pOff);

			}

			m_pNThreeStateBox->SetStateFont(pHide, pOn, pOff);
		}
		else if (pIniValue->Key == DES_STATE_TEXT_COLOR)
		{
			// Hide
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pHide);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pHide);

			}
			
			// On
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pOn);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pOn);
			}

			// Off
			if (pIniValue->nValue >= 3)
			{
				pIniValue->Values[2].MakeUpper();
				if (pIniValue->Values[2] != _T("NULL"))
					//m_PicassoResources.FindDataOfList(pIniValue->Values[2], &pOff);
					m_PicassoResourcesCMap.Lookup(pIniValue->Values[2], pOff);
			}

			m_pNThreeStateBox->SetStateTextColor((CPicassoColor*)pHide, (CPicassoColor*)pOn, (CPicassoColor*)pOff);
		}
		else if (pIniValue->Key == DES_STATE_BLINK)
		{
			BOOL	bHide = FALSE, bOn = FALSE, bOff = FALSE;
			
			// Hide
			if (pIniValue->nValue >= 1)
			{
				pIniValue->Values[0].MakeUpper();
				if (pIniValue->Values[0] == _T("ON"))
					bHide = TRUE;
			}
			
			// On
			if (pIniValue->nValue >= 2)
			{
				pIniValue->Values[1].MakeUpper();
				if (pIniValue->Values[1] == _T("ON"))
					bOn = TRUE;
			}

			// Off
			if (pIniValue->nValue >= 3)
			{
				pIniValue->Values[2].MakeUpper();
				if (pIniValue->Values[2] == _T("ON"))
					bOff = TRUE;
			}

			m_pNThreeStateBox->SetStateBlink(bHide, bOn, bOff);
		}
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNThreeStateBox, pIniValue))
				return TRUE;

			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNThreeStateBox, pIniValue))
				return TRUE;

			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNThreeStateBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateBinIndexBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateBinIndexBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoBinIndexBox need to SCREEN. BININDEXBOX\n")));
		return FALSE;
	}

	// [EJNLBOX][/EJNLBOX]
	if (pIniValue->Key == DES_BININDEX_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNBinIndexBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoBinIndexBox don't Deallocate in BININDEXBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNBinIndexBox = (CPicassoBinIndexBox *) new CPicassoBinIndexBox;
			if (!m_pNBinIndexBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoBinIndexBox allocate Failed in BININDEXBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNBinIndexBox);
		}
		else
		{
			if (!m_pNBinIndexBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoBinIndexBox don't allocate in BININDEXBOX\n")));
				return FALSE;
			}
			
			m_pNBinIndexBox = NULL;
		}
	}
	else
	{
		if (!m_pNBinIndexBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoBinIndexBox don't allocate 2 in BININDEXBOX\n")));
			return FALSE;
		}
					
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pNBinIndexBox, pIniValue))
			return TRUE;
		
		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pNBinIndexBox, pIniValue))
			return TRUE;
		
		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pNBinIndexBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

BOOL CPicassoCtrl::CreateVKeyBoardBox(PINIVALUE pIniValue)
{
	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoVKeyBoardBox need to SCREEN. VKEYBOARDBOX\n")));
		return FALSE;
	}

	// [EJNLBOX][/EJNLBOX]
	if (pIniValue->Key == DES_VKEYBOARD_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pVKeyBoardBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoVKeyBoardBox don't Deallocate in VKEYBOARDBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pVKeyBoardBox = (CPicassoVKeyBoardBox *) new CPicassoVKeyBoardBox;
			if (!m_pVKeyBoardBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoVKeyBoardBox allocate Failed in VKEYBOARDBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pVKeyBoardBox);
		}
		else
		{
			if (!m_pVKeyBoardBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoVKeyBoardBox don't allocate in VKEYBOARDBOX\n")));
				return FALSE;
			}
			
			m_pVKeyBoardBox = NULL;
		}
	}
	else
	{
		CPicassoResource	*pNormal = NULL;

		if (!m_pVKeyBoardBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoVKeyBoardBox don't allocate 2 in VKEYBOARDBOX\n")));
			return FALSE;
		}

		// NormalBtnFont=ID
		if (pIniValue->Key == DES_BUTTON_GAP)
		{
			m_pVKeyBoardBox->SetButtonGap(Asc2Int(pIniValue->Values[0]), m_LoadScreenSize.cx, m_LoadScreenSize.cy);
		}
		else if (pIniValue->Key == DES_LINE_HEIGHT)
		{
			m_pVKeyBoardBox->SetLineHeight(Asc2Int(pIniValue->Values[0]), m_LoadScreenSize.cx, m_LoadScreenSize.cy);
		}
		else if (pIniValue->Key == DES_NORMAL_BTN_FONT)
		{
			pIniValue->Values[0].MakeUpper();
//			m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetNormalButtonFont((CPicassoFont*)pNormal);
		}
		else if (pIniValue->Key == DES_NORMAL_BTN_TXT_COLOR)
		{
			pIniValue->Values[0].MakeUpper();
//			m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetNormalButtonTextColor((CPicassoColor*)pNormal);
		}
		else if (pIniValue->Key == DES_NORMAL_BTN_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
//			m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetNormalButtonImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_NORMAL_BTN_SIZE)
		{
			int	nWidth = 0, nHeight = 0;

			if (pIniValue->nValue == 2)
			{
				nWidth = Asc2Int(pIniValue->Values[0]);
				nHeight = Asc2Int(pIniValue->Values[1]);

				m_pVKeyBoardBox->SetNormalButtonSize(nWidth, nHeight, m_LoadScreenSize.cx, m_LoadScreenSize.cy);
			}
		}
		/*
		else if (pIniValue->Key == DES_CONTROL_BTN_FONT)
		{
			pIniValue->Values[0].MakeUpper();
//			m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], &pNormal);

			m_pVKeyBoardBox->SetControlButtonFont((CPicassoFont*)pNormal);
		}
		else if (pIniValue->Key == DES_CONTROL_BTN_TXT_COLOR)
		{
			pIniValue->Values[0].MakeUpper();
//			m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], &pNormal);

			m_pVKeyBoardBox->SetControlButtonTextColor((CPicassoColor*)pNormal);
		}
		else if (pIniValue->Key == DES_CONTROL_BTN_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
//			m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], &pNormal);

			m_pVKeyBoardBox->SetControlButtonImage((CPicassoPicture*)pNormal);
		}
		*/
		else if (pIniValue->Key == DES_CONTROL_CLEAR_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetClearButtonImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_CONTROL_123_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->Set123ButtonImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_CONTROL_ABC_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetABCButtonImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_CONTROL_CAPS_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetCapsButtonImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_CONTROL_SYMBOL1_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetSymbol1ButtonImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_CONTROL_SYMBOL2_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetSymbol2ButtonImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_CONTROL_BTN_SIZE)
		{
			int	nWidth = 0, nHeight = 0;

			if (pIniValue->nValue == 2)
			{
				nWidth = Asc2Int(pIniValue->Values[0]);
				nHeight = Asc2Int(pIniValue->Values[1]);

				m_pVKeyBoardBox->SetControlButtonSize(nWidth, nHeight, m_LoadScreenSize.cx, m_LoadScreenSize.cy);
			}
		}
		else if (pIniValue->Key == DES_SPACE_BTN_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetSpaceButtonImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_SPACE_BTN_SIZE)
		{
			int	nWidth = 0, nHeight = 0;

			if (pIniValue->nValue == 2)
			{
				nWidth = Asc2Int(pIniValue->Values[0]);
				nHeight = Asc2Int(pIniValue->Values[1]);

				m_pVKeyBoardBox->SetSpaceButtonSize(nWidth, nHeight, m_LoadScreenSize.cx, m_LoadScreenSize.cy);
			}
		}
		else if (pIniValue->Key == DES_ARROW_LEFT_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetLeftArrowImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_ARROW_RIGHT_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetRightArrowImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_ARROW_UP_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetUpArrowImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_ARROW_DOWN_IMAGE)
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

			m_pVKeyBoardBox->SetDownArrowImage((CPicassoPicture*)pNormal);
		}
		else if (pIniValue->Key == DES_ARROW_BTN_SIZE)
		{
			int	nWidth = 0, nHeight = 0;

			if (pIniValue->nValue == 2)
			{
				nWidth = Asc2Int(pIniValue->Values[0]);
				nHeight = Asc2Int(pIniValue->Values[1]);

				m_pVKeyBoardBox->SetArrowButtonSize(nWidth, nHeight, m_LoadScreenSize.cx, m_LoadScreenSize.cy);
			}
		}
		else
		{		
			// Setting Control.
			SettingControl((CPicassoControl*)m_pVKeyBoardBox, pIniValue);
		}
		
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateImageListBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateImageListBox(PINIVALUE pIniValue)
{
	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoImageListBox need to SCREEN. IMAGELISTBOX\n")));
		return FALSE;
	}

	// [IMAGELISTBOX][/IMAGELISTBOX]
	if (pIniValue->Key == DES_IMAGELIST_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pImageListBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoImageListBox don't Deallocate in IMAGELISTBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pImageListBox = (CPicassoImageListBox *) new CPicassoImageListBox;
			if (!m_pImageListBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoImageListBox allocate Failed in IMAGELISTBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pImageListBox);
		}
		else
		{
			if (!m_pImageListBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoImageListBox don't allocate in IMAGELISTBOX\n")));
				return FALSE;
			}
			
			m_pImageListBox = NULL;
		}
	}
	else
	{
		if (!m_pImageListBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoImageListBox don't allocate 2 in IMAGELISTBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_INIT_LIST)
		{
			m_pImageListBox->SetInitList(pIniValue->Values[0]);
			return TRUE;
		}
		else if (pIniValue->Key == DES_LIST_SET)
		{
			CPicassoResource	*pImage = NULL;

			pIniValue->Values[1].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pImage);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pImage);

			m_pImageListBox->SetImageList(pIniValue->Values[0], (CPicassoPicture*)pImage);
			return TRUE;
		}
					
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pImageListBox, pIniValue))
			return TRUE;
		
		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pImageListBox, pIniValue))
			return TRUE;
		
		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pImageListBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateButtonImageListBox()
 WRITER       : AIREAT (2010.01.25)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateButtonImageListBox(PINIVALUE pIniValue)
{
	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoButtonImageListBox need to SCREEN. BUTTONIMAGELISTBOX\n")));
		return FALSE;
	}

	// [BUTTONIMAGELISTBOX][/BUTTONIMAGELISTBOX]
	if (pIniValue->Key == DES_BUTTON_IMAGELIST_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pButtonImageListBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoButtonImageListBox don't Deallocate in BUTTONIMAGELISTBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pButtonImageListBox = (CPicassoButtonImageListBox *) new CPicassoButtonImageListBox;
			if (!m_pButtonImageListBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoButtonImageListBox allocate Failed in BUTTONIMAGELISTBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pButtonImageListBox);
		}
		else
		{
			if (!m_pButtonImageListBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoButtonImageListBox don't allocate in BUTTONIMAGELISTBOX\n")));
				return FALSE;
			}
			
			m_pButtonImageListBox = NULL;
		}
	}
	else
	{
		if (!m_pButtonImageListBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoButtonImageListBox don't allocate 2 in BUTTONIMAGELISTBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_INIT_LIST)
		{
			m_pButtonImageListBox->SetInitList(pIniValue->Values[0]);
			return TRUE;
		}
		else if (pIniValue->Key == DES_LIST_SET)
		{
			CPicassoResource	*pImage = NULL;

			pIniValue->Values[1].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pImage);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pImage);

			m_pButtonImageListBox->SetImageList(pIniValue->Values[0], (CPicassoPicture*)pImage);
			return TRUE;
		}
					
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pButtonImageListBox, pIniValue))
			return TRUE;
		
		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pButtonImageListBox, pIniValue))
			return TRUE;
		
		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pButtonImageListBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateAmountBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateAmountBox(PINIVALUE pIniValue)
{
	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoAmountBox need to SCREEN. AMOUNTBOX\n")));
		return FALSE;
	}

	// [AMOUNTBOX][/AMOUNTBOX]
	if (pIniValue->Key == DES_AMOUNT_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pAmountBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAmountBox don't Deallocate in AMOUNTBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pAmountBox = (CPicassoAmountBox *) new CPicassoAmountBox;
			if (!m_pAmountBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAmountBox allocate Failed in AMOUNTBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pAmountBox);
		}
		else
		{
			if (!m_pAmountBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoAmountBox don't allocate in AMOUNTBOX\n")));
				return FALSE;
			}
			
			m_pAmountBox = NULL;
		}
	}
	else
	{
		if (!m_pAmountBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoAmountBox don't allocate 2 in AMOUNTBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_SYMBOL_NAME)
		{
			m_pAmountBox->SetCurrencySymbolName(pIniValue->Values[0]);
			return TRUE;
		}
		else if (pIniValue->Key == DES_AMOUNT_NAME)
		{
			m_pAmountBox->SetAmountTypeName(pIniValue->Values[0]);
			return TRUE;
		}
		else if (pIniValue->Key == DES_INPUT_MAX_NAME)
		{
			m_pAmountBox->SetInputMaxName(pIniValue->Values[0]);
			return TRUE;
		}
		else if (pIniValue->Key == DES_INPUT_MIN_NAME)
		{
			m_pAmountBox->SetInputMinName(pIniValue->Values[0]);
			return TRUE;
		}
		else if (pIniValue->Key == DES_AUTO_RUN_NAME)
		{
			m_pAmountBox->SetAutoRunName(pIniValue->Values[0]);
			return TRUE;
		}
					
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pAmountBox, pIniValue))
			return TRUE;
		
		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pAmountBox, pIniValue))
			return TRUE;
		
		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pAmountBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateKeyListBox()
 WRITER       : AIREAT (2010.02.21)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateKeyListBox(PINIVALUE pIniValue)
{
	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoKeyListBox need to SCREEN. KEYLISTBOX\n")));
		return FALSE;
	}

	// [KEYLISTBOX][/KEYLISTBOX]
	if (pIniValue->Key == DES_KEY_LIST_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pKeyListBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoKeyListBox don't Deallocate in KEYLISTBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pKeyListBox = (CPicassoKeyListBox *) new CPicassoKeyListBox;
			if (!m_pKeyListBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoKeyListBox allocate Failed in KEYLISTBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pKeyListBox);
		}
		else
		{
			if (!m_pKeyListBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoKeyListBox don't allocate in KEYLISTBOX\n")));
				return FALSE;
			}
			
			m_pKeyListBox = NULL;
		}
	}
	else
	{
		if (!m_pKeyListBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoKeyListBox don't allocate 2 in KEYLISTBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_LIST_SET)
		{
			m_pKeyListBox->SetKeyList(pIniValue->Values[0]);
			return TRUE;
		}
					
		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pKeyListBox, pIniValue))
			return TRUE;
		
		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pKeyListBox, pIniValue))
			return TRUE;
		
		// Setting Text.
		if (SettingText((CPicassoTextBox*)m_pKeyListBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateWeatherBox()
 WRITER       : AIREAT (2010.02.21)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateWeatherBox(PINIVALUE pIniValue)
{
	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoWeatherBox need to SCREEN. WEATHERBOX\n")));
		return FALSE;
	}

	// [WEATHERBOX][/WEATHERBOX]
	if (pIniValue->Key == DES_WEATHER_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pWeatherBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoWeatherBox don't Deallocate in WEATHERBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pWeatherBox = (CPicassoWeatherBox *) new CPicassoWeatherBox;
			if (!m_pWeatherBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoWeatherBox allocate Failed in WEATHERBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pWeatherBox);
		}
		else
		{
			if (!m_pWeatherBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoWeatherBox don't allocate in WEATHERBOX\n")));
				return FALSE;
			}
			
			m_pWeatherBox = NULL;
		}
	}
	else
	{
		if (!m_pWeatherBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoWeatherBox don't allocate 2 in WEATHERBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_BACK_POS)
		{
			// check value count. it must be 4
			if (pIniValue->nValue == 4)
			{
				m_pWeatherBox->SetBackRect(Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
										   Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
										   m_LoadScreenSize.cx, m_LoadScreenSize.cy);
			}
		}
		else if (pIniValue->Key == DES_TITLE_POS)
		{
			// check value count. it must be 4
			if (pIniValue->nValue == 5)
			{
				m_pWeatherBox->SetTitleRect(Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
										   Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
										   Asc2Int(pIniValue->Values[4]),
										   m_LoadScreenSize.cx, m_LoadScreenSize.cy);
			}
		}
		else if (pIniValue->Key == DES_DATE_POS)
		{
			// check value count. it must be 4
			if (pIniValue->nValue == 5)
			{
				m_pWeatherBox->SetDateRect(Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
										   Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
										   Asc2Int(pIniValue->Values[4]),
										   m_LoadScreenSize.cx, m_LoadScreenSize.cy);
			}
		}
		else if (pIniValue->Key == DES_WEATHER_POS)
		{
			// check value count. it must be 4
			if (pIniValue->nValue == 5)
			{
				m_pWeatherBox->SetWeatherRect(Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
										   Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
										   Asc2Int(pIniValue->Values[4]),
										   m_LoadScreenSize.cx, m_LoadScreenSize.cy);
			}
		}
		else if (pIniValue->Key == DES_TEMPER_POS)
		{
			// check value count. it must be 4
			if (pIniValue->nValue == 5)
			{
				m_pWeatherBox->SetTemperatureRect(Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
										   Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
										   Asc2Int(pIniValue->Values[4]),
										   m_LoadScreenSize.cx, m_LoadScreenSize.cy);
			}
		}
		else if (pIniValue->Key == DES_NORMAL_SET)
		{
			CPicassoResource *pFont = NULL, *pColor = NULL;

			// font
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pFont);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pFont);

			// color
			if (pIniValue->nValue > 1)
			{
				pIniValue->Values[1].MakeUpper();
				//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pColor);
				m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pColor);
			}

			m_pWeatherBox->SetNormalFontColor(pFont, pColor);
		}
		else if (pIniValue->Key == DES_ACCENT_SET)
		{
			CPicassoResource *pFont = NULL, *pColor = NULL;

			// font
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pFont);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pFont);

			// color
			if (pIniValue->nValue > 1)
			{
				pIniValue->Values[1].MakeUpper();
				//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pColor);
				m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pColor);
			}

			m_pWeatherBox->SetAccentFontColor(pFont, pColor);
		}
		else if (pIniValue->Key == DES_BACK_IMG)
		{
			CPicassoResource *pImg = NULL;
			BOOL bCelsius;

			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] == _T("C"))
				bCelsius = TRUE;
			else
				bCelsius = FALSE;

			pIniValue->Values[1].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pImg);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pImg);

			m_pWeatherBox->SetBackImage(bCelsius, pImg);
		}
		else if (pIniValue->Key == DES_WEATHER_IMG)
		{
			CPicassoResource *pImg = NULL;

			int nCode = Asc2Int(pIniValue->Values[0]);

			if (pIniValue->nValue > 1)
			{
				pIniValue->Values[1].MakeUpper();
				//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pImg);
				m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pImg);
			}

			m_pWeatherBox->SetWeatherCodeImage(nCode, pImg);
		}
	}

	return TRUE;
}

// [#2065] NH KJW 2011.05.26		// [#2350] US Justin Change Name
/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateCustomEditBox()
 WRITER       : KJW (2011.05.26)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateCustomEditBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CreateCustomEditBox need to SCREEN. CENTBOX\n")));
		return FALSE;
	}

	// [CUSTOMEDITBOX][/CUSTOMEDITBOX]
	if (pIniValue->Key == DES_CUSTOMEDIT_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNCustomEditBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CreateCustomEditBox don't Deallocate in CUSTOMEDITBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNCustomEditBox = (CPicassoCustomEditBox*) new CPicassoCustomEditBox;
			if (!m_pNCustomEditBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CreateCustomEditBox allocate Failed in CUSTOMEDITBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNCustomEditBox);
		}
		else
		{
			if (!m_pNCustomEditBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CreateCustomEditBox don't allocate in CUSTOMEDITBOX\n")));
				return FALSE;
			}
			
			m_pNCustomEditBox = NULL;
		}
	}
	else
	{
		if (!m_pNCustomEditBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CreateCustomEditBox don't allocate 2 in CUSTOMEDITBOX\n")));
			return FALSE;
		}
		
		if (pIniValue->Key == DES_MAXINCHAR)
		{
			m_pNCustomEditBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
		}
		// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
		else if(pIniValue->Key == DES_MINCHAR)
		{
			m_pNCustomEditBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
		}
		// end of [#182]
		// [#2350] NH Justin 2015.06.17		Support Display Format
		else if(pIniValue->Key == DES_DISPFORMAT)
		{
			m_pNCustomEditBox->SetDisplayFormat(pIniValue->Values[0]);
		}
		// end of [#2350]
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNCustomEditBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNCustomEditBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNCustomEditBox, pIniValue))
				return TRUE;
		}
	}

	return TRUE;
}
// end of [#2065]

// [#2316] US Justin 2014.12.17 Decimal Box
/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: CreateDecimalBox()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::CreateDecimalBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CreateDecimalBox need to SCREEN. DECIMALBOX\n")));
		return FALSE;
	}

	// [DECIMALBOX][/DECIMALBOX]
	if (pIniValue->Key == DES_DECIMAL_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (m_pNDecimalBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoDecimalBox don't Deallocate in DECIMALBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pNDecimalBox = (CPicassoDecimalBox*) new CPicassoDecimalBox;
			if (!m_pNDecimalBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoDecimalBox allocate Failed in DECIMALBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pNDecimalBox);
		}
		else
		{
			if (!m_pNDecimalBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoDecimalBox don't allocate in DECIMALBOX\n")));
				return FALSE;
			}

			m_pNDecimalBox = NULL;
		}
	}
	else
	{
		if (!m_pNDecimalBox)
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoDecimalBox don't allocate 2 in DECIMALBOX\n")));
			return FALSE;
		}
		
		if (pIniValue->Key == DES_MAXINCHAR)
		{
			m_pNDecimalBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
		}
		else if(pIniValue->Key == DES_MINCHAR)
		{
			m_pNDecimalBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
		}
		else
		{
			// Setting Control.
			if (SettingControl((CPicassoControl*)m_pNDecimalBox, pIniValue))
				return TRUE;
			
			// Setting Shape.
			if (SettingShape((CPicassoShape*)m_pNDecimalBox, pIniValue))
				return TRUE;
			
			// Setting Text.
			if (SettingText((CPicassoTextBox*)m_pNDecimalBox, pIniValue))
				return TRUE;
		}		
	}
	return TRUE;
}
// End of [#2316]



























BOOL CPicassoCtrl::CreateMultiTextBox(PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	CPicassoResource *pNormal = NULL, *pSelect = NULL;

	if (!m_pNScreen)
	{
		NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox need to SCREEN. TEXTBOX\n")));
		return FALSE;
	}

	// [MULTITEXTBOX][/MULTITEXTBOX]
	if (pIniValue->Key == DES_MULTITEXTBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))	// [MULTITEXT]를 만났음
		{
			if (m_pMultiTextBox)	// 해당 multitextbox 객체가 생성되어 있으면
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't Deallocate in TEXTBOX\n")));
				return FALSE;
			}

			// allocate.
			m_pMultiTextBox = (CPicassoMultiTextBox*) new CPicassoMultiTextBox;
			if (!m_pMultiTextBox)
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox allocate Failed in MULTITEXTBOX\n")));
				return FALSE;
			}

			// Register.
			m_pNScreen->SetAddControl(m_pMultiTextBox);
		}
		else	// [/MULTITEXT]
		{
			if (!m_pMultiTextBox)	// 만들어진 multitextbox 객체가 없으면
			{
				NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't allocate in TEXTBOX\n")));
				return FALSE;
			}
			
			m_pMultiTextBox = NULL;
		}
	}
	else	// [MULTITEXT] ~ [/MULTITEXT} 에 있는 key / value들
	{
		if (!m_pMultiTextBox)		// multitextbox 객체 없으면 예외처리
		{
			NHUIDBG(1, (_T("[SCR_IN] CPicassoTextBox don't allocate 2 in TEXTBOX\n")));
			return FALSE;
		}

		if (pIniValue->Key == DES_TEXT)
		{
			m_pMultiTextBox->SetMultiText(Asc2Int(pIniValue->Values[0]), pIniValue->Values[1]);

		}
		else if (pIniValue->Key == DES_TEXTID)
		{
			pIniValue->Values[1].MakeUpper();
			// [#2434] AU Kook 2016.07.18 Speed up AP loading
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pNormal);
//			m_PicassoResourcesText.FindDataOfList(pIniValue->Values[1], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pNormal);
			// end of [#2434]
			m_pMultiTextBox->SetMultiLocaleText(Asc2Int(pIniValue->Values[0]), pNormal);
		}
		// Font=Normal ID, Select ID
		else if (pIniValue->Key == DES_FONT)
		{
			pIniValue->Values[1].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pNormal);

			if (pIniValue->nValue == 3)
			{
				pIniValue->Values[2].MakeUpper();
				//m_PicassoResources.FindDataOfList(pIniValue->Values[2], &pSelect);
				m_PicassoResourcesCMap.Lookup(pIniValue->Values[2], pSelect);

			}
			
			m_pMultiTextBox->SetMultiFont(Asc2Int(pIniValue->Values[0]), pNormal, pSelect);
		}
		// Color=ID,RGB
		else if (pIniValue->Key == DES_MULTITEXT_COLOR)
		{
			CString strNormalColor, strSelectColor;

			if (pIniValue->nValue >= 2)
			{
				strNormalColor = pIniValue->Values[1];
				strSelectColor = pIniValue->Values[1];
			}

			if (pIniValue->nValue >= 3)
				strSelectColor = pIniValue->Values[2];

			strNormalColor.MakeUpper();
			strSelectColor.MakeUpper();

			// find color resource
			//m_PicassoResources.FindDataOfList(strNormalColor, &pNormal);
			m_PicassoResourcesCMap.Lookup(strNormalColor, pNormal);

			if (pNormal != NULL && pNormal->GetTpye() != RES_COLOR)
				pNormal = NULL;
			//m_PicassoResources.FindDataOfList(strSelectColor, &pSelect);
			m_PicassoResourcesCMap.Lookup(strSelectColor, pSelect);

			if (pSelect != NULL && pSelect->GetTpye() != RES_COLOR)
				pSelect = NULL;

			m_pMultiTextBox->SetMultiTextColor(Asc2Int(pIniValue->Values[0]), (CPicassoColor*)pNormal, (CPicassoColor*)pSelect);
		}

		// MultiTextAlign=Value
		else if (pIniValue->Key == DES_MULTITEXT_ALIGN)
		{
			CString alignValue = pIniValue->Values[0];
			alignValue.MakeUpper();

			if (alignValue == DES_LEFT)
				m_pMultiTextBox->SetMultiTextAlign(LEFT);
			else if (alignValue == DES_RIGHT)
				m_pMultiTextBox->SetMultiTextAlign(RIGHT);
			else
				m_pMultiTextBox->SetMultiTextAlign(CENTER);
			
		}

		// Setting Control.
		if (SettingControl((CPicassoControl*)m_pMultiTextBox, pIniValue))
			return TRUE;

		// Setting Shape.
		if (SettingShape((CPicassoShape*)m_pMultiTextBox, pIniValue))
			return TRUE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SettingControl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::SettingControl(CPicassoControl *pControl, PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	CPicassoResource *pSelect = NULL;		// [#2529] NH Justin 2018.02.22 Add 2800 SE

	if (!pControl)
		return FALSE;

	// Name=""
	if (pIniValue->Key == DES_NAME)
	{
		pControl->SetName(pIniValue->Values[0]);
	}
	// AFocus=0,1
	else if (pIniValue->Key == DES_AFOCUS)
	{
		pControl->SetAutoFocus(Asc2Int(pIniValue->Values[0]));
	}
	// Pos=left,top,width,height
	else if (pIniValue->Key == DES_POS)
	{
		// check value count. it must be 4
		if (pIniValue->nValue == 4)
		{
			pControl->SetDisplayRect(Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
									Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]),
									m_LoadScreenSize.cx, m_LoadScreenSize.cy);
		}
		// [#2529] NH Justin 2018.02.22 Add 2800 SE
		else if(pIniValue->nValue == 1)	// Get position from resource
		{
			pIniValue->Values[0].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pSelect);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pSelect);
			if (pSelect != NULL && pSelect->GetTpye() != RES_POSITION)
				pSelect = NULL;
			
			if(pSelect != NULL)
			{
				CPicassoPosition* pPos = (CPicassoPosition*) pSelect;
				pControl->SetDisplayRect(pPos->m_nXX, pPos->m_nYY, pPos->m_nWid, pPos->m_nHgt, m_LoadScreenSize.cx, m_LoadScreenSize.cy);
			}
		}
		// End of [#2529]
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SettingShape()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::SettingShape(CPicassoShape *pShape, PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	CPicassoResource *pNormal = NULL, *pSelect = NULL;

	if (!pShape)
		return FALSE;

	// Brush=normal ID,select ID
	if (pIniValue->Key == DES_BRUSH)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);
		}

		pShape->SetBrush(pNormal, pSelect);
	}
	// Pen=normal ID,select ID
	else if (pIniValue->Key == DES_PEN)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);
		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);
		}

		pShape->SetPen(pNormal, pSelect);
	}
	// Picture=ID
	else if (pIniValue->Key == DES_PICTURE)
	{
		pIniValue->Values[0].MakeUpper();
		//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

		pShape->SetPicture(pNormal);
	}
	// Icon=ID, Width, Height
	else if (pIniValue->Key == DES_ICON)
	{
		pIniValue->Values[0].MakeUpper();
// 		m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

		pShape->SetIcon(pNormal,	Asc2Int(pIniValue->Values[1]),
									Asc2Int(pIniValue->Values[2]));
	}
	else if (pIniValue->Key == DES_ROUND)
	{
		pShape->SetRoundSize(Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]));
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SettingText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::SettingText(CPicassoTextBox *pTextBox, PINIVALUE	pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	CPicassoResource *pNormal = NULL, *pSelect = NULL;

	if (!pTextBox)
		return FALSE;

	// Text=""
	if (pIniValue->Key == DES_TEXT)
	{
		pTextBox->SetText(pIniValue->Values[0]);
	}
	// TextID=ID
	else if (pIniValue->Key == DES_TEXTID)
	{
		pIniValue->Values[0].MakeUpper();
		// [#2434] AU Kook 2016.07.18 Speed up AP loading
		//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
//		m_PicassoResourcesText.FindDataOfList(pIniValue->Values[0], &pNormal);	
		m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);
		// end of [#2434]
		pTextBox->SetLocaleText(pNormal);
	}
	// Font=Normal ID, Select ID
	else if (pIniValue->Key == DES_FONT)
	{
// 		pIniValue->Values[0].MakeUpper();
// 		m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
// 
// 		if (pIniValue->nValue == 2)
// 		{
// 			pIniValue->Values[1].MakeUpper();
// 			m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
// 		}
// 
// 		pTextBox->SetFont(pNormal, pSelect);


		// [#2186] US KMK 2013.04.08 Asian Language Support
		// ID뒤에 "_(언어명)"을 추가해야 함
		// ex) F_ApTitle : F_ApTitle_KOR, F_ApTitle_ENG, F_ApTitle_SPN, ...
		CPicassoResource *pNormalArray[MAX_LOCALE], *pSelectArray[MAX_LOCALE];
		CString strNormalFontID[MAX_LOCALE], strSelectFontID[MAX_LOCALE];

		// [#2186] US KMK 2013.04.25 포인터배열 초기화 추가. (패스워드 입력 폰트 버그 수정)
		memset(pNormalArray, 0x00, sizeof(pNormalArray));
		memset(pSelectArray, 0x00, sizeof(pSelectArray));

 		pIniValue->Values[0].MakeUpper();

		for (int i=0; i<MAX_LOCALE; i++)
		{
			// ID, ID_SPN, ID_FRN, ID_CHN, ID_KOR, ID_JPN
			if (i == ENG_MODE-1)	// US KMK 2013.05.03 다른 PicassoControl과의 호환을 위해 영어는 ID_ENG가 아닌 ID 사용.
				strNormalFontID[i].Format(_T("%s"), pIniValue->Values[0]); 
			else
				strNormalFontID[i].Format(_T("%s_%s"), pIniValue->Values[0], GET_LANGUAGE_SHORT_STRING(i+1)); 

			// pNormalArray 배열에 각 언어별 폰트 정보를 담은 뒤 TextBox에 보내준다
			// TextBox에선 LocaleNum에 따라 필요한 폰트를 꺼내 씀
			//m_PicassoResources.FindDataOfList(strNormalFontID[i], &pNormalArray[i]);
			m_PicassoResourcesCMap.Lookup(strNormalFontID[i], pNormalArray[i]);

		}
		//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);	// 기존 부분
		m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);	// 기존 부분

		if (pIniValue->nValue == 2)
		{
			pIniValue->Values[1].MakeUpper();

			for (int i=0; i<MAX_LOCALE; i++)
			{
				// ID, ID_SPN, ID_FRN, ID_CHN, ID_KOR, ID_JPN
				if (i == ENG_MODE-1)	// US KMK 2013.05.03 다른 PicassoControl과의 호환을 위해 영어는 ID_ENG가 아닌 ID 사용.
					strSelectFontID[i].Format(_T("%s"), pIniValue->Values[1]); 
				else
					strSelectFontID[i].Format(_T("%s_%s"), pIniValue->Values[1], GET_LANGUAGE_SHORT_STRING(i+1)); 

// 				m_PicassoResources.FindDataOfList(strSelectFontID[i], &pSelectArray[i]);
				m_PicassoResourcesCMap.Lookup(strSelectFontID[i], pSelectArray[i]);
			}
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);	// 기존 부분
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);
		}

		pTextBox->SetFont(pNormal, pSelect);
		pTextBox->SetFonts(pNormalArray, pSelectArray);
		// end of [#2186]

	}
	// TextSet=Multiline, Align, Text color
	else if (pIniValue->Key == DES_TEXTSET)
	{
		if (pIniValue->nValue == 3) // [#RWC6-399] US ryan.payton 2022.09.15 DigitalMint
		{
			// Multiline
			if (pIniValue->nValue >= 1)
			{
				pTextBox->SetMultiLine(Asc2Int(pIniValue->Values[0]));
			}

			// Align
			if (pIniValue->nValue >= 2)
			{
				if (pIniValue->Values[1] == DES_LEFT)
					pTextBox->SetTextAlign(LEFT);
				else if (pIniValue->Values[1] == DES_CENTER)
					pTextBox->SetTextAlign(CENTER);
				else if (pIniValue->Values[1] == DES_RIGHT)
					pTextBox->SetTextAlign(RIGHT);
			}

			/*
			// Text Color
			if (pIniValue->nValue >= 5)
			{
				if (pIniValue->nValue == 8)
				{
					pTextBox->SetTextColor(
						RGB(Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]), Asc2Int(pIniValue->Values[4])),
						RGB(Asc2Int(pIniValue->Values[5]), Asc2Int(pIniValue->Values[6]), Asc2Int(pIniValue->Values[7])));
				}
				else
				{
					pTextBox->SetTextColor(
						RGB(Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]), Asc2Int(pIniValue->Values[4])),
						RGB(Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]), Asc2Int(pIniValue->Values[4])));
				}
			}
			*/

			// Normal Text Color
			if (pIniValue->nValue >= 3)
			{
				CString strNormalColor, strSelectColor;

				if (pIniValue->nValue >= 3)
				{
					strNormalColor = pIniValue->Values[2];
					strSelectColor = pIniValue->Values[2];
				}

				if (pIniValue->nValue >= 4)
					strSelectColor = pIniValue->Values[3];

				strNormalColor.MakeUpper();
				strSelectColor.MakeUpper();

				// find color resource
				// m_PicassoResources.FindDataOfList(strNormalColor, &pNormal);
				m_PicassoResourcesCMap.Lookup(strNormalColor, pNormal);

				if (pNormal != NULL && pNormal->GetTpye() != RES_COLOR)
					pNormal = NULL;

				// m_PicassoResources.FindDataOfList(strSelectColor, &pSelect);
				m_PicassoResourcesCMap.Lookup(strSelectColor, pSelect);

				if (pSelect != NULL && pSelect->GetTpye() != RES_COLOR)
					pSelect = NULL;

				pTextBox->SetTextColor((CPicassoColor *)pNormal, (CPicassoColor *)pSelect);
			}
		}
		// [#RWC6-399] US ryan.payton 2022.09.15 DigitalMint
		else if (pIniValue->nValue == 1) // Get TextSet from resource
		{
			pIniValue->Values[0].MakeUpper();

			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pSelect);

			if (pSelect != NULL && pSelect->GetTpye() != RES_TEXTSET)
				pSelect = NULL;

			if (pSelect != NULL)
			{
				CPicassoTextSet *pTextSet = (CPicassoTextSet *)pSelect;

				// Multiline
				pTextBox->SetMultiLine(pTextSet->m_nMulti);

				// Align
				if (pTextSet->m_strAlignment == DES_LEFT)
					pTextBox->SetTextAlign(LEFT);
				else if (pTextSet->m_strAlignment == DES_CENTER)
					pTextBox->SetTextAlign(CENTER);
				else if (pTextSet->m_strAlignment == DES_RIGHT)
					pTextBox->SetTextAlign(RIGHT);

				// Normal Text Color
				CString strNormalColor, strSelectColor;

				strNormalColor = pTextSet->m_strColor;
				strSelectColor = pTextSet->m_strColor;

				strNormalColor.MakeUpper();
				strSelectColor.MakeUpper();

				// find color resource
				// m_PicassoResources.FindDataOfList(strNormalColor, &pNormal);
				m_PicassoResourcesCMap.Lookup(strNormalColor, pNormal);

				if (pNormal != NULL && pNormal->GetTpye() != RES_COLOR)
					pNormal = NULL;

				// m_PicassoResources.FindDataOfList(strSelectColor, &pSelect);
				m_PicassoResourcesCMap.Lookup(strSelectColor, pSelect);

				if (pSelect != NULL && pSelect->GetTpye() != RES_COLOR)
					pSelect = NULL;

				pTextBox->SetTextColor((CPicassoColor *)pNormal, (CPicassoColor *)pSelect);
			}
		}
		// End of [#RWC6-399]
	}
	// Blink=On/off
	else if (pIniValue->Key == DES_BLINK)
	{
		if (pIniValue->nValue >= 1)
		{
			pTextBox->SetBlinkMode(Asc2Int(pIniValue->Values[0]));
		}
	}
	// ActKey=""
	else if (pIniValue->Key == DES_ACT_KEY)
	{
		for (int i = 0; i < pIniValue->nValue; i++)
		{
			pTextBox->SetActKey(pIniValue->Values[i]);
		}
	}
	// Act=type,Name,Data
	else if (pIniValue->Key == DES_ACT)
	{
		CPicassoRequest	req;

		req.SetKinD(REQ_NONE);

		// type
		if (pIniValue->nValue >= 1)
		{
			if (pIniValue->Values[0] == DES_FSCMD)
				req.SetKinD(REQ_FSCMD);
			else if (pIniValue->Values[0] == DES_FULLFSCMD)
				req.SetKinD(REQ_FULLFSCMD);
			else if (pIniValue->Values[0] == DES_ALLFSCMD)
				req.SetKinD(REQ_ALLFSCMD);
			else if (pIniValue->Values[0] == DES_KEYEVENT)
				req.SetKinD(REQ_KEYEVENT);
			else if (pIniValue->Values[0] == DES_SETFOCUS)
				req.SetKinD(REQ_SETFOCUS);
			else if (pIniValue->Values[0] == DES_BTNFIND)
				req.SetKinD(REQ_BTNFIND);
			else if (pIniValue->Values[0] == DES_SETVALUE)
				req.SetKinD(REQ_SETVALUE);
			else if (pIniValue->Values[0] == DES_BINACT)
				req.SetKinD(REQ_BINACT);
		}
		// Name
		if (pIniValue->nValue >= 2)
		{
			req.SetName(pIniValue->Values[1]);
		}
		// Data
		if (pIniValue->nValue >= 3)
		{
			req.SetData(pIniValue->Values[2]);
		}

		if (req.GetKind() != REQ_NONE)
		{
			pTextBox->SetAct(req.GetKind(), req.GetName(), req.GetData());
		}
	}
	// outline=width,color
	else if (pIniValue->Key == DES_OUTLINE)
	{
		CString strColor;

		if (pIniValue->nValue >= 2)
			strColor = pIniValue->Values[1];		// [#2529] NH Justin 2018.02.23 Add 2800 SE
			//strColor = pIniValue->Values[2];

		strColor.MakeUpper();

		// find color resource
		//m_PicassoResources.FindDataOfList(strColor, &pNormal);
		m_PicassoResourcesCMap.Lookup(strColor, pNormal);


		if (pNormal != NULL && pNormal->GetTpye() != RES_COLOR)
			pNormal = NULL;

		pTextBox->SetOutline(Asc2Int(pIniValue->Values[0]), (CPicassoColor*)pNormal);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SettingVKey()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::SettingVKey(CPicassoVirtualKeyBox *pVKeyBox, PINIVALUE pIniValue)
{
	NHUIDBG(DBG_CALL, (_T("Section(%s), Key(%s), Value(%s)\n"), pIniValue->Section, pIniValue->Key, pIniValue->Values[0]));

	CPicassoResource *pNormal = NULL, *pSelect = NULL;

	if (pVKeyBox == NULL)
		return FALSE;

	if (pIniValue->Key == DES_GUIDE_FONT)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);

		}

		pVKeyBox->SetGuideFont(pNormal, pSelect);
	}
	else if (pIniValue->Key == DES_GUIDE_BRUSH)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);

		}

		pVKeyBox->SetGuideBrush(pNormal, pSelect);
	}
	else if (pIniValue->Key == DES_GUIDE_PEN)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);
		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);

		}

		pVKeyBox->SetGuidePen(pNormal, pSelect);
	}
	else if (pIniValue->Key == DES_GUIDE_TEXT_COLOR)
	{
		CString strNormalColor, strSelectColor;

		if (pIniValue->nValue >= 1)
		{
			strNormalColor = pIniValue->Values[0];
			strSelectColor = pIniValue->Values[1];
		}

		if (pIniValue->nValue >= 2)
			strSelectColor = pIniValue->Values[1];

		strNormalColor.MakeUpper();
		strSelectColor.MakeUpper();

		// find color resource
		//m_PicassoResources.FindDataOfList(strNormalColor, &pNormal);
		m_PicassoResourcesCMap.Lookup(strNormalColor, pNormal);


		if (pNormal != NULL && pNormal->GetTpye() != RES_COLOR)
			pNormal = NULL;
		//m_PicassoResources.FindDataOfList(strSelectColor, &pSelect);
		m_PicassoResourcesCMap.Lookup(strSelectColor, pSelect);


		if (pSelect != NULL && pSelect->GetTpye() != RES_COLOR)
			pSelect = NULL;

		pVKeyBox->SetGuideTextColor((CPicassoColor*)pNormal, (CPicassoColor*)pSelect);
	}
	else if (pIniValue->Key == DES_VKEY_FONT)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);

		}

		pVKeyBox->SetVKeyFont(pNormal, pSelect);
	}
	else if (pIniValue->Key == DES_VKEY_BRUSH)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		if (pIniValue->Values[0] != _T("NULL"))
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
				m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);

		}

		pVKeyBox->SetVKeyBrush(pNormal, pSelect);
	}
	else if (pIniValue->Key == DES_VKEY_PEN)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		if (pIniValue->Values[0] != _T("NULL"))
			//m_PicassoResources.FindDataOfList(pIniValue->Values[0], &pNormal);
			m_PicassoResourcesCMap.Lookup(pIniValue->Values[0], pNormal);

		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				//m_PicassoResources.FindDataOfList(pIniValue->Values[1], &pSelect);
				m_PicassoResourcesCMap.Lookup(pIniValue->Values[1], pSelect);

		}

		pVKeyBox->SetVKeyPen(pNormal, pSelect);
	}
	else if (pIniValue->Key == DES_VKEY_TEXT_COLOR)
	{
		CString strNormalColor, strSelectColor;

		if (pIniValue->nValue >= 1)
		{
			strNormalColor = pIniValue->Values[0];
			strSelectColor = pIniValue->Values[1];
		}

		if (pIniValue->nValue >= 2)
			strSelectColor = pIniValue->Values[1];

		strNormalColor.MakeUpper();
		strSelectColor.MakeUpper();

		// find color resource
		//m_PicassoResources.FindDataOfList(strNormalColor, &pNormal);
		m_PicassoResourcesCMap.Lookup(strNormalColor, pNormal);

		if (pNormal != NULL && pNormal->GetTpye() != RES_COLOR)
			pNormal = NULL;

		//m_PicassoResources.FindDataOfList(strSelectColor, &pSelect);
		m_PicassoResourcesCMap.Lookup(strSelectColor, pSelect);

		if (pSelect != NULL && pSelect->GetTpye() != RES_COLOR)
			pSelect = NULL;

		pVKeyBox->SetVKeyTextColor((CPicassoColor*)pNormal, (CPicassoColor*)pSelect);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: LoadScreenText()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::LoadScreenText(LPCTSTR FileName)
{
	NHUIDBG(DBG_CALL, (_T("FileName(%s)\n"), FileName));

	CNHReadiniFile	Readini;
	PINIVALUE	pwIniValue;
	CString		strFullFileName = m_strScreenPath + FileName;
	CString		strTemp;

	CHECK_TIME_START;

	// try TAR first.
	BOOL bDatOpened = FALSE;
	if (g_pTarScreenAsset)
	{
		CString strFileName(FileName);
		strFileName.Trim();

		unsigned int fileSize = g_pTarScreenAsset->GetFileSize(strFileName);
		if (fileSize > 0)
		{
			BYTE* pTarBuffer = new BYTE[fileSize + 1]();
			g_pTarScreenAsset->ReadFile(strFileName, pTarBuffer);
			bDatOpened = Readini.Open(pTarBuffer, CNHReadiniFile::modeRead, CNHReadiniFile::modeLittleUNICODE);
			delete[] pTarBuffer;
		}
	}

	// try FILE next.
	if (!bDatOpened)
	{
		bDatOpened = Readini.Open(strFullFileName, CNHReadiniFile::modeRead, CNHReadiniFile::modeLittleUNICODE);
	}

	// open Screen Text File
// 	if (!Readini.Open(strFullFileName, CNHReadiniFile::modeRead, CNHReadiniFile::modeLittleUNICODE))
	if (!bDatOpened)
	{
		NHUIDBG(1, (_T("[SCR_IN] File Open Failed (%s) in TEXT_RES\n"), FileName));
		return FALSE;
	}

	// Load Screen Text
	while (NULL != (pwIniValue = Readini.ReadiniValue()))
	{
		// [SUPPORT] section
		if (pwIniValue->Section == DES_SUPPORT)
		{
			if (pwIniValue->Key.Left(2) == m_strLoadCountry && _ttoi(pwIniValue->Values[0]) == 1)
			{
				m_strSupportLanguage += pwIniValue->Key.Right(3);
				m_strSupportLanguage += _T(",");
			}
		}
		// [TEXT] section
		else if (pwIniValue->Section == DES_TEXT)
		{
			pwIniValue->Values[0].Trim(L"\r\t \n");			// TAR compatibility

			if (pwIniValue->Key == DES_ID)
			{
				pwIniValue->Values[0].MakeUpper();
				m_pNLocaleText = (CPicassoLocaleText*) new CPicassoLocaleText(pwIniValue->Values[0], MAX_LOCALE);	// KSK_TEMP
				if (m_pNLocaleText)
				{
					// [#2434] AU Kook 2016.07.18 Speed up AP loading
					//m_PicassoResources.PutOnTailOfList(m_pNLocaleText->GetID(), m_pNLocaleText);
//					m_PicassoResourcesText.PutOnTailOfList(m_pNLocaleText->GetID(), m_pNLocaleText);
					m_PicassoResourcesCMap.SetAt(m_pNLocaleText->GetID(), m_pNLocaleText);		// 20160708
					// end of [#2434]
				}
			}
			else if (m_strSupportLanguage.Find(pwIniValue->Key.Right(3)) != -1)
			{
				strTemp = pwIniValue->Key.Left(2);
				if (strTemp == m_strLoadCountry || strTemp == _T("US"))
				{
					strTemp = pwIniValue->Key.Right(3);

					if (strTemp == _T("ENG"))
						m_pNLocaleText->AddLocaleText(ENG_MODE, pwIniValue->Values[0]);
					else if (strTemp == _T("SPN"))
						m_pNLocaleText->AddLocaleText(SPN_MODE, pwIniValue->Values[0]);
					else if (strTemp == _T("FRN"))
						m_pNLocaleText->AddLocaleText(FRN_MODE, pwIniValue->Values[0]);
					else if (strTemp == _T("CHN"))
						m_pNLocaleText->AddLocaleText(CHN_MODE, pwIniValue->Values[0]);
					else if (strTemp == _T("KOR"))
						m_pNLocaleText->AddLocaleText(KOR_MODE, pwIniValue->Values[0]);
					else if (strTemp == _T("JPN"))
						m_pNLocaleText->AddLocaleText(JPN_MODE, pwIniValue->Values[0]);
				}
			}
		}
	}
	
	Readini.Close();

	CHECK_TIME_END;

	return TRUE;
}

BOOL CPicassoCtrl::LoadControlSetting(PINIVALUE pIniValue)
{
	CString value;
	bool isEnabled = true;

	if (m_strLoadSection.GetLength() <= 0)
		return FALSE;

	if (pIniValue->Key == _T("ON"))
	{
		// All values should be AND'd together
		for (int i = 0; i < pIniValue->nValue; i++)
		{
			value = pIniValue->Values[i];

			// Touch/Function set in MasterScreenDesc.dat
			if (value == L"Touch" || value == L"Function")
			{
				isEnabled &= value == m_strLoadSection;
			}
			// Key Type
			else if (value.Left(wcslen(DES_KEY_TYPE)) == DES_KEY_TYPE)
			{
				// ADA is for US only
				// ISO is all other territories
				if (value == DES_KEY_TYPE_ADA)
				{
					isEnabled &= m_strLoadCountry == _T("US");
				}
				else if (value == DES_KEY_TYPE_ISO)
				{
					isEnabled &= m_strLoadCountry != _T("US");
				}
				else 
				{
					isEnabled &= FALSE;
				}
			}
#ifdef UNDER_CE
			// OS Type
			else if (value.Left(wcslen(DES_OS_TYPE)) == DES_OS_TYPE)
			{
				if (value == DES_OS_TYPE_60)
				{
					isEnabled &= GetConfigFuncPointer()->GetCEVersion() == WINCE_6;
				}
				if (value == DES_OS_TYPE_70)
				{
					isEnabled &= GetConfigFuncPointer()->GetCEVersion() == WINCE_7;
				}
				else 
				{
					isEnabled &= FALSE;
				}
			}
#endif
		}

		m_bLoadControl = isEnabled;
	}
	else if (pIniValue->Key == _T("OFF"))
	{
		m_bLoadControl = TRUE;
	}

	return TRUE;
}