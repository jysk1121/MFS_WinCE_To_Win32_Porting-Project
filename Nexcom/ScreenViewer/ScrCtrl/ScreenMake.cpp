#include "stdafx.h"
#include "ScreenDisplay.h"


///////////////////////////////////////////////////////////////////
//	Help Macro Define
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//	Extern Global Variable Define
///////////////////////////////////////////////////////////////////

extern int 		g_nMaxLocale;

///////////////////////////////////////////////////////////////////
//	Global Variable Define
///////////////////////////////////////////////////////////////////

CNScreen		*g_pNScreen = NULL;
CNTextBox		*g_pNTextBox = NULL;
CNEffectTextBox	*g_pNEffectTextBox = NULL;
CNEditTextBox	*g_pNEditTextBox = NULL;
CNChangePWBox	*g_pNChangePWBox = NULL;
CNButtonBox		*g_pNButtonBox = NULL;
CNSWKeyBox		*g_pNSWKeyBox = NULL;
CNToggleBox		*g_pNToggleBox = NULL;
CNEJNLBox		*g_pNEJNLBox = NULL;
CNDateBox		*g_pNDateBox = NULL;
CNIPBox			*g_pNIPBox = NULL;
CNBinBox		*g_pNBinBox = NULL;
CNCentBox		*g_pNCentBox = NULL;
CNDollarBox		*g_pNDollarBox = NULL;
CNAdvBox		*g_pNAdvBox = NULL;
CNAnimation		*g_pNAniBox = NULL;
CNTableEditTextBox	*g_pNTableEditBox = NULL;
CNVKeyButtonBox	*g_pNVKeyButtonBox = NULL;
CNAlphaKeyBox	*g_pNAlphaKeyBox = NULL;
CNTableKeyBox	*g_pNTableKeyBox = NULL;
CNThreeStateBox *g_pNThreeStateBox = NULL;
CNBinIndexBox   *g_pNBinIndexBox = NULL;	//[#453] SOOK 2009.07.19
CNDecimalBox	*g_pNDecimalBox = NULL;		//[#774] Justin 2014.12.16
CNCentClearBox	*g_pNCentClearBox = NULL;	//[#810] Justin 2015.10.09
CNLocaleText	*g_pNLocaleText = NULL;
CNKeyTestBox	*g_pNKeyTestBox = NULL;
CNInputEditBox	*g_pNInputEditBox = NULL;
CNThreeTypeBox	*g_pNThreeTypeBox = NULL;
CNGifBox		*g_pNGifBox = NULL;			// V1.0.2.3 2018.06.26
CNImageBox		*g_pNImageBox = NULL;
CNEditButtonBox	*g_pNEditButtonBox = NULL;


///////////////////////////////////////////////////////////////////
//	Function prototype define
///////////////////////////////////////////////////////////////////
BOOL	LoadMasterScreenValue(LPCTSTR FileName, CString strLoadSection);
BOOL	LoadScreenValue(LPCTSTR FileName, CNHReadiniFile::OpModeFlags Type);
BOOL	CreateResource(PINIVALUE pIniValue);
BOOL	CreateScreen(PINIVALUE	pIniValue);
BOOL	CreateTextBox(PINIVALUE	pIniValue);
BOOL	CreateEditBox(PINIVALUE	pIniValue);
BOOL	CreateEffectTextBox(PINIVALUE	pIniValue);
BOOL	CreateChangePWBox(PINIVALUE	pIniValue);
BOOL	CreateButtonBox(PINIVALUE	pIniValue);
BOOL	CreateSWKeyBox(PINIVALUE	pIniValue);
BOOL	CreateEnDisableBox(PINIVALUE	pIniValue);
BOOL	CreateEJNLBox(PINIVALUE	pIniValue);
BOOL	CreateDateBox(PINIVALUE	pIniValue);
BOOL	CreateIPBox(PINIVALUE	pIniValue);
BOOL	CreateBinBox(PINIVALUE	pIniValue);
BOOL	CreateCentBox(PINIVALUE	pIniValue);
BOOL	CreateDollarBox(PINIVALUE	pIniValue);
BOOL	CreateAdvBox(PINIVALUE	pIniValue);
BOOL	CreateAniBox(PINIVALUE	pIniValue);
BOOL	CreateTableEditBox(PINIVALUE	pIniValue);
BOOL	CreateVKeyButtonBox(PINIVALUE	pIniValue);
BOOL	CreateAlphaKeyBox(PINIVALUE	pIniValue);
BOOL	CreateTableKeyBox(PINIVALUE	pIniValue);
BOOL	CreateThreeStateBox(PINIVALUE	pIniValue);
BOOL	CreateBinIndexBox(PINIVALUE	pIniValue); //[#543] SOOK 2009.07.19 BIN INDEX BOX 추가 
BOOL	CreateDecimalBox(PINIVALUE	pIniValue); //[#774] Justin 2014.12.16  Add Decimal Box
BOOL	CreateCentClearBox(PINIVALUE pIniValue); //[#810] Justin 2015.10.09  Add CentClearBox
BOOL	CreateKeyTestBox(PINIVALUE pIniValue);
BOOL	CreateInputEditBox(PINIVALUE pIniValue);
BOOL	CreateThreeTypeBox(PINIVALUE pIniValue);
BOOL	CreateGifBox(PINIVALUE	pIniValue);		// V1.0.2.3 2018.06.26
BOOL	CreateImageBox(PINIVALUE	pIniValue);		// JPG,BMP File 표시하는 컨트롤
BOOL	CreateEditButtonBox(PINIVALUE	pIniValue);	// Button Control을 Edit Box처럼 사용할 수 있도록. touch기능을 유용하게 사용하기 위해서

// V1.0.2.4 2018.07.12 - ScreenControl.dat 파일 추가로 인한 제어 함수 추가
VOID	SetTextBox(PINIVALUE pIniValue);
VOID	SetEditBox(PINIVALUE pIniValue);
VOID	SetEffectTextBox(PINIVALUE pIniValue);
VOID	SetChangePWBox(PINIVALUE pIniValue);
VOID	SetButtonBox(PINIVALUE pIniValue);
VOID	SetSWKeyBox(PINIVALUE pIniValue);
VOID	SetEnDisableBox(PINIVALUE pIniValue);
VOID	SetEJNLBox(PINIVALUE pIniValue);
VOID	SetDateBox(PINIVALUE pIniValue);
VOID	SetIPBox(PINIVALUE pIniValue);
VOID	SetBinBox(PINIVALUE pIniValue);
VOID	SetCentBox(PINIVALUE pIniValue);
VOID	SetDollarBox(PINIVALUE pIniValue);
VOID	SetAdvBox(PINIVALUE pIniValue);
VOID	SetAniBox(PINIVALUE pIniValue);
VOID	SetTableEditBox(PINIVALUE pIniValue);
VOID	SetVKeyButtonBox(PINIVALUE pIniValue);
VOID	SetAlphaKeyBox(PINIVALUE pIniValue);
VOID	SetTableKeyBox(PINIVALUE pIniValue);
VOID	SetThreeStateBox(PINIVALUE pIniValue);
VOID	SetBinIndexBox(PINIVALUE pIniValue);
VOID	SetDecimalBox(PINIVALUE pIniValue);
VOID	SetCentClearBox(PINIVALUE pIniValue);
VOID	SetKeyTestBox(PINIVALUE pIniValue);
VOID	SetInputEditBox(PINIVALUE pIniValue);
VOID	SetThreeTypeBox(PINIVALUE pIniValue);
VOID	SetGifBox(PINIVALUE pIniValue);
VOID	SetImageBox(PINIVALUE pIniValue);
VOID	SetEditButtonBox(PINIVALUE pIniValue);

VOID	SearchScreenControlInfo(PINIVALUE pIniValue);	// ScreenControl.dat 파일을 검색한 후, 해당 ControlID의 Key정보를 세팅한다.
//////////////////////////////////////////////////////////////////////////

BOOL	LoadScreenText(LPCTSTR FileName);
//BOOL	CreateLocaleText(int LocaleCount, PINIVALUE pwIniValue);	// [#469][NH] AIREAT 2008.12.20 AP,OP Text 다국어 통합 작업

BOOL	SettingControl(CNControl *pControl, PINIVALUE	pIniValue);
BOOL	SettingShape(CNShape *pShape, PINIVALUE pIniValue);
BOOL	SettingText(CNTextBox *pTextBox, PINIVALUE	pIniValue);
BOOL	SettingVKey(CNVirtualKeyBox *pVKeyBox, PINIVALUE pIniValue);
BOOL	SettingControlEx(CNControl *pControl, PINIVALUE	pIniValue);

BOOL	NHScreenMake(CString strLoadSection, CString strMasterPath)
{
	/*
	CString strMasterFile;

	strMasterFile = strMasterPath + MASTER_SCREEN_FILE;

	// Master File Parsing
	if (!LoadMasterScreenValue(strMasterFile, strLoadSection))
		goto ErrorReturn;
	*/

	if (!LoadMasterScreenValue(_T(""), _T("")))
		goto ErrorReturn;

	return TRUE;

ErrorReturn:
	return FALSE;
}

BOOL	LoadMasterScreenValue(LPCTSTR FileName, CString strLoadSection)
{
	CString strClientPath;
	CString strRootPath;
	CString strSearchFile;
	CString	strFullFileName;
	HANDLE hSearch = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA	FileData;
	CNHReadiniFile	Readini;

// MasterScreenDesc.dat file 제거
// 
// 	LoadScreenText(_T("\\ScreenText.dat"));
// 
// 	LoadScreenValue(_T("\\Client.dat"), CReadiniFile::modeASCII);
// 
// 	LoadScreenValue(_T("\\Supervisor.dat"), CReadiniFile::modeASCII);
// 
// 	LoadScreenValue(_T("\\Update.dat"), CReadiniFile::modeASCII);

	strClientPath.Format(_T("\\%d_%d\\Client"), g_sizeScreen.cx, g_sizeScreen.cy);
	strRootPath.Format(_T("\\%d_%d"), g_sizeScreen.cx, g_sizeScreen.cy);
	
	///////////////////////////////
	// 고객이 수정 불가능

	LoadScreenText(strRootPath + _T("\\ReceiptPrintText.dat"));
	LoadScreenText(strRootPath + _T("\\HostErrorText.dat"));

	LoadScreenValue(strRootPath + _T("\\Supervisor\\Supervisor.dat"), CNHReadiniFile::modeASCII);
	LoadScreenValue(strRootPath + _T("\\Client_AdvScreen.dat"), CNHReadiniFile::modeASCII);

	////////////////////////////////
	// 고객이 수정 가능
	LoadScreenText(strClientPath + _T("\\Client_ScreenText.dat"));

	LoadScreenValue(strClientPath + _T("\\Client_Resource.dat"), CNHReadiniFile::modeASCII);
	LoadScreenValue(strClientPath + _T("\\Client_Common.dat"), CNHReadiniFile::modeASCII);

	strSearchFile.Format(_T("%s%s\\*.dat"), g_strMasterPath, strClientPath);
	hSearch = FindFirstFile(strSearchFile, &FileData);

	if(hSearch != INVALID_HANDLE_VALUE)
	{
		CString strSrcFile;
		
		while(TRUE)
		{
			strSrcFile.Empty();
			
			strSrcFile.Format(_T("%s"), FileData.cFileName);

			if( (_T("Client_Resource.dat") == strSrcFile) || (_T("Client_Common.dat") == strSrcFile) || (_T("Client_ScreenText.dat") == strSrcFile) )
			{
				if (FindNextFile(hSearch, &FileData) == FALSE)
					break;
				continue;
			}
			
			LoadScreenValue(strClientPath + _T("\\") + strSrcFile, CNHReadiniFile::modeASCII);
			
			if (FindNextFile(hSearch, &FileData) == FALSE)
				break;
		}
		
		FindClose(hSearch);
	}
	else
		AfxMessageBox(_T("File not found!!!"));	//*LEHTEST	
	/////////////////////////////////////////////////////////////////////////////////////////
	
	LoadScreenValue(_T("\\Update.dat"), CNHReadiniFile::modeASCII);

	return TRUE;
/*
	CNHReadiniFile	Readini;
	PINIVALUE	pIniValue;
	CString		strResPath;

	// open Master Screen File
	if (!Readini.Open(FileName, CNHReadiniFile::modeRead, CNHReadiniFile::modeASCII))
	{
		NHDEBUG(1, (_T("[SCR_IN] File Open Failed (%s) in MASTER_RES\n"), MASTER_SCREEN_FILE));
		goto ErrorReturn;
	}

	// Create public controls.
	while (NULL != (pIniValue = Readini.ReadiniValue()))
	{
		// [RESOURCE] Section
		if (pIniValue->Section == strLoadSection)
		{
			// File Key
			if (pIniValue->Key == DES_FILE)
			{
				if (!LoadScreenValue(pIniValue->Values[0], CNHReadiniFile::modeASCII))
					goto ErrorReturn;
			}
			// UText Key
			if (pIniValue->Key == DES_UFILE)
			{
				if (!LoadScreenValue(pIniValue->Values[0], CNHReadiniFile::modeLittleUNICODE))
					goto ErrorReturn;
			}
			// UText Key
			if (pIniValue->Key == DES_SCRTEXT)
			{
				if (!LoadScreenText(pIniValue->Values[0]))
					goto ErrorReturn;
			}
		}
	}

	Readini.Close();
	return TRUE;

ErrorReturn:
	Readini.Close();
	return FALSE;
*/

}

BOOL	LoadScreenValue(LPCTSTR FileName, CNHReadiniFile::OpModeFlags Type)
{
	CNHReadiniFile	Readini;
	PINIVALUE	pIniValue;
	CString		strResPath;
	CString		strFullFileName = g_strMasterPath + FileName;

	// open Master Screen File
	if (!Readini.Open(strFullFileName, CNHReadiniFile::modeRead, Type))
	{
		NHDEBUG(1, (_T("[SCR_IN] File Open Failed (%s) in SCR_RES\n"), FileName));
		goto ErrorReturn;
	}

	strResPath.Format(_T("RES_%d_%d"), g_sizeScreen.cx, g_sizeScreen.cy);

	// Create public controls.
	while (NULL != (pIniValue = Readini.ReadiniValue()))
	{
		// [RESOURCE] Section
		if (pIniValue->Section == DES_RESOURCE || pIniValue->Section == strResPath)
		{
			if (!CreateResource(pIniValue))
				goto ErrorReturn;
		}
		// [SCREEN] Section
		else if (pIniValue->Section == DES_SCREEN)
		{
			if (!CreateScreen(pIniValue))
				goto ErrorReturn;
		}
		// [TEXTBOX] Section
		else if (pIniValue->Section == DES_TEXTBOX)
		{
			if (!CreateTextBox(pIniValue))
				goto ErrorReturn;
		}
		// [EFFECTTEXTBOX] Section
		else if (pIniValue->Section == DES_EFFECTTEXTBOX)
		{
			if (!CreateEffectTextBox(pIniValue))
				goto ErrorReturn;
		}
		// [EDITBOX] Section
		else if (pIniValue->Section == DES_EDITBOX)
		{
			if (!CreateEditBox(pIniValue))
				goto ErrorReturn;
		}
		// [CHANGEPWBOX] Section
		else if (pIniValue->Section == DES_CHANGE_PW_BOX)
		{
			if (!CreateChangePWBox(pIniValue))
				goto ErrorReturn;
		}
		// [BUTTON] Section
		else if (pIniValue->Section == DES_BUTTON)
		{
			if (!CreateButtonBox(pIniValue))
				goto ErrorReturn;
		}
		// [SWKEY] Section
		else if (pIniValue->Section == DES_SW_KEY)
		{
			if (!CreateSWKeyBox(pIniValue))
				goto ErrorReturn;
		}
		// [TOGGLEBOX] Section
		else if (pIniValue->Section == DES_TOGGLE_BOX)
		{
			if (!CreateEnDisableBox(pIniValue))
				goto ErrorReturn;
		}
		// [EJNLBOX] Section
		else if (pIniValue->Section == DES_EJNL_BOX)
		{
			if (!CreateEJNLBox(pIniValue))
				goto ErrorReturn;
		}
		// [DATEBOX] Section
		else if (pIniValue->Section == DES_DATE_BOX)
		{
			if (!CreateDateBox(pIniValue))
				goto ErrorReturn;
		}
		// [IPBOX] Section
		else if (pIniValue->Section == DES_IP_BOX)
		{
			if (!CreateIPBox(pIniValue))
				goto ErrorReturn;
		}
		// [BINBOX] Section
		else if (pIniValue->Section == DES_BIN_BOX)
		{
			if (!CreateBinBox(pIniValue))
				goto ErrorReturn;
		}
		// [CENTBOX] Section
		else if (pIniValue->Section == DES_CENT_BOX)
		{
			if (!CreateCentBox(pIniValue))
				goto ErrorReturn;
		}
		// [DOLLARBOX] Section
		else if (pIniValue->Section == DES_DOLLAR_BOX)
		{
			if (!CreateDollarBox(pIniValue))
				goto ErrorReturn;
		}
		// [ADVBOX] Section
		else if (pIniValue->Section == DES_ADV_BOX)
		{
			if (!CreateAdvBox(pIniValue))
				goto ErrorReturn;
		}
		// [ANIBOX] Section
		else if (pIniValue->Section == DES_ANI_BOX)
		{
			if (!CreateAniBox(pIniValue))
				goto ErrorReturn;
		}
		// [TABLEEDITBOX]
		else if (pIniValue->Section == DES_TABLE_EDITBOX)
		{
			if (!CreateTableEditBox(pIniValue))
				goto ErrorReturn;
		}
		// [VKEYBUTTON]
		else if (pIniValue->Section == DES_VKEYBUTTON)
		{
			if (!CreateVKeyButtonBox(pIniValue))
				goto ErrorReturn;
		}
		// [ALPHAKEYBOX]
		else if (pIniValue->Section == DES_ALPHA_KEYBOX)
		{
			if (!CreateAlphaKeyBox(pIniValue))
				goto ErrorReturn;
		}
		// [TABLEKEYBOX]
		else if (pIniValue->Section == DES_TABLE_KEYBOX)
		{
			if (!CreateTableKeyBox(pIniValue))
				goto ErrorReturn;
		}
		// [THREESTATEBOX]
		else if (pIniValue->Section == DES_THREE_STATEBOX)
		{
			if (!CreateThreeStateBox(pIniValue))
				goto ErrorReturn;
		}
		// [BININDEXBOX] Section //[#543] SOOK 
		else if (pIniValue->Section == DES_BININDEX_BOX)
		{
			if (!CreateBinIndexBox(pIniValue))
				goto ErrorReturn;
		}
		// [DECIMALBOX] Section //[#774] JUSTIN
		else if (pIniValue->Section == DES_DECIMAL_BOX)
		{
			if (!CreateDecimalBox(pIniValue))
				goto ErrorReturn;
		}
		// [CENTCLEARBOX] Section //[#810] JUSTIN
		else if (pIniValue->Section == DES_CENTCLEARL_BOX)
		{
			if (!CreateCentClearBox(pIniValue))
				goto ErrorReturn;
		}
		else if (pIniValue->Section == DES_KEYTEST_BOX)
		{
			if (!CreateKeyTestBox(pIniValue))
				goto ErrorReturn;
		}
		else if (pIniValue->Section == DES_INPUTEDIT_BOX)
		{
			if (!CreateInputEditBox(pIniValue))
				goto ErrorReturn;
		}
		else if (pIniValue->Section == DES_THREETYPE_BOX)
		{
			if (!CreateThreeTypeBox(pIniValue))
				goto ErrorReturn;
		}
		// V1.0.2.3 2018.06.26 - [GIFBOX] Section
		else if (pIniValue->Section == DES_GIF_BOX)
		{
			if (!CreateGifBox(pIniValue))
				goto ErrorReturn;
		}
		// 2019.04.05 - [IMAGEBOX] Section
		else if (pIniValue->Section == DES_IMAGE_BOX)
		{
			if (!CreateImageBox(pIniValue))
				goto ErrorReturn;
		}
		// 2019.04.17 - [EDITBUTTONBOX] Section
		else if (pIniValue->Section == DES_EDITBUTTON_BOX)
		{
			if (!CreateEditButtonBox(pIniValue))
				goto ErrorReturn;
		}
	}
	
	Readini.Close();
	return TRUE;

ErrorReturn:
	Readini.Close();
	return FALSE;
}

BOOL	CreateResource(PINIVALUE pIniValue)
{
	COLORREF	crColor;
	CNResource	*pRes;

	// Font=ID,Name,Size,Bold
	if (pIniValue->Key == DES_FONT)
	{
		LOGFONT	lf;
		HFONT	hFont;

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

				// Adjust Scale
				{
					int currentX = g_sizeScreen.cx;
					int currentY = g_sizeScreen.cy;
		
					//float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);
					//float ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);

					float ScreenRateX = (float)((float)currentX / (float)g_BaseSizeScreen.cx);
					float ScreenRateY = (float)((float)currentY / (float)g_BaseSizeScreen.cy);

					if (ScreenRateX > ScreenRateY)
						fontsize = (int)((float)ScreenRateX * fontsize + 0.5);
					else
						fontsize = (int)((float)ScreenRateY * fontsize + 0.5);
				}

				// SIZE
				lf.lfHeight = GET_FONTSIZE(fontsize);
				lf.lfWidth = (LONG)(lf.lfHeight * (2/3));
				if (lf.lfWidth < 0)
					lf.lfWidth *= -1;

				// set quality
#ifdef UNDER_CE
				lf.lfQuality = CLEARTYPE_COMPAT_QUALITY;
#else
				lf.lfQuality = DEFAULT_QUALITY;
#endif

				// BOLD
				if (Asc2Int(pIniValue->Values[3]))
//#ifdef UNDER_CE
					lf.lfWeight = FW_BOLD;
//#else
//					lf.lfWeight = FW_SEMIBOLD;
//#endif
			}
			else
			{
				// Name
				_tcsncpy(lf.lfFaceName, pIniValue->Values[1], LF_FACESIZE-1);
				
				// SIZE
				int fontsize = Asc2Int(pIniValue->Values[2]);

				// Adjust Scale
				{
					int currentX = g_sizeScreen.cx;
					int currentY = g_sizeScreen.cy;
		
					//float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);
					//float ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);

					float ScreenRateX = (float)((float)currentX / (float)g_BaseSizeScreen.cx);
					float ScreenRateY = (float)((float)currentY / (float)g_BaseSizeScreen.cy);

					if (ScreenRateX > ScreenRateY)
						fontsize = (int)((float)ScreenRateX * fontsize + 0.5);
					else
						fontsize = (int)((float)ScreenRateY * fontsize + 0.5);
				}

				lf.lfHeight = GET_FONTSIZE(fontsize);
				// set quality
#ifdef UNDER_CE
				lf.lfQuality = CLEARTYPE_QUALITY;
#else
				lf.lfQuality = DEFAULT_QUALITY;
#endif

#ifdef UNDER_CE
				lf.lfWidth = (LONG)(lf.lfHeight * 0.5);
#else
				lf.lfWidth = (LONG)(lf.lfHeight * (0.45));
#endif

				if (lf.lfWidth < 0)
					lf.lfWidth *= -1;
				
				// BOLD
				if (Asc2Int(pIniValue->Values[3]))
//#ifdef UNDER_CE
					lf.lfWeight = FW_BOLD;
//#else
//					lf.lfWeight = FW_SEMIBOLD;
//#endif
			}

			hFont = ::CreateFontIndirect(&lf);
			if (hFont)
			{
				pIniValue->Values[0].MakeUpper();
				pRes = (CNResource*) new CNFont(RES_FONT, pIniValue->Values[0], hFont);
				if (pRes)
					g_NHResources.PutOnTailOfList(pRes->GetID(), pRes);
			}
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
				pRes = (CNResource*) new CNPen(RES_PEN, pIniValue->Values[0], hPen);
				if (pRes)
					g_NHResources.PutOnTailOfList(pRes->GetID(), pRes);
			}
		}
	}
	// Color=ID,RGB
	else if (pIniValue->Key == DES_COLOR)
	{
		if (pIniValue->nValue == 4)
		{
			pIniValue->Values[0].MakeUpper();
			pRes = (CNResource*) new CNColor(pIniValue->Values[0], 
											 Asc2Int(pIniValue->Values[1]),
											 Asc2Int(pIniValue->Values[2]), 
											 Asc2Int(pIniValue->Values[3]));
			if (pRes)
				g_NHResources.PutOnTailOfList(pRes->GetID(), pRes);
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
				pRes = (CNResource*) new CNBrush(RES_BRUSH, pIniValue->Values[0], hBrush);
				if (pRes)
					g_NHResources.PutOnTailOfList(pRes->GetID(), pRes);
			}
		}
	}
	// Picture=ID,File
	else if (pIniValue->Key == DES_PICTURE)
	{
		IMG_TYPE	ImgType = NH_IMG_UNKNOWN;
		CString		strType;
		CString		strFileName;

		if (pIniValue->nValue == 2)
		{
			// GetImgType
			strType = pIniValue->Values[1].Right(3);
			strType.MakeUpper();

			if (strType == _T("BMP"))
				ImgType = NH_IMG_BMP;
			else if (strType == _T("JPG"))
				ImgType = NH_IMG_JPG;
			else if (strType == _T("PNG"))
				ImgType = NH_IMG_PNG;
			else
				ImgType = NH_IMG_UNKNOWN;

			strFileName = g_strMasterPath + pIniValue->Values[1];

			pIniValue->Values[0].MakeUpper();
			pRes = (CNResource*) new CNPicture(pIniValue->Values[0], strFileName);
			if (pRes)
				g_NHResources.PutOnTailOfList(pRes->GetID(), pRes);
		}
	}

	return TRUE;

//ErrorReturn:
//	return FALSE;
}

BOOL	CreateScreen(PINIVALUE	pIniValue)
{
	// ID =?
	if (pIniValue->Key == DES_ID)
	{
		// check valide.
		if (g_pNScreen)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNScreen don't Deallocate in SCREEN(%s)\n"), g_pNScreen->GetID()));
			goto ErrorReturn;
		}

#ifndef UNDER_CE
		CNScreen *pTempScreen=NULL;
		pIniValue->Values[0].MakeUpper();
		if (g_NHScreens.FindDataOfList(pIniValue->Values[0], &pTempScreen) != NULL)
		{
			MessageBox(NULL, pIniValue->Values[0], _T("Duplicated Screen Number"), MB_ICONWARNING | MB_OK);
		}
#endif

		// allocate.
		g_pNScreen = (CNScreen*) new CNScreen;
		if (!g_pNScreen)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNScreen allocate Failed in SCREEN\n")));
			goto ErrorReturn;
		}

		// Register.
		pIniValue->Values[0].MakeUpper();
		g_pNScreen->SetID(pIniValue->Values[0]);

		g_NHScreens.PutOnTailOfList(g_pNScreen->GetID(), g_pNScreen);

		g_strScreenID = pIniValue->Values[0];	// V1.0.2.4 2018.07.19 - Camera의 좌표를 Script의 Pos 값으로 설정하도록 하기 위함.
	}
	// Back=ID
	else if (pIniValue->Key == DES_BACK)
	{
		// check valide.
		if (!g_pNScreen)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNScreen don't allocate in SCREEN\n")));
			goto ErrorReturn;
		}

		CNScreen	*pBackScreen = NULL;
		pIniValue->Values[0].MakeUpper();
		if (g_NHScreens.FindDataOfList(pIniValue->Values[0], &pBackScreen))
			g_pNScreen->SetBackScreen(pBackScreen);		
	}
	// [SCREEN][/SCREEN]
	if (pIniValue->Key == DES_SCREEN)
	{
		// END..
		if(!Asc2Int(pIniValue->Values[0]))
		{
			if (!g_pNScreen)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNScreen don't allocate in SCREEN\n")));
				goto ErrorReturn;
			}

			g_pNScreen = NULL;
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

BOOL	CreateTextBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNTextBox need to SCREEN. TEXTBOX\n")));
		goto ErrorReturn;
	}

	// [TEXTBOX][/TEXTBOX]
	if (pIniValue->Key == DES_TEXTBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNTextBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTextBox don't Deallocate in TEXTBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNTextBox = (CNTextBox*) new CNTextBox;
			if (!g_pNTextBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTextBox allocate Failed in TEXTBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNTextBox);
		}
		else
		{
			if (!g_pNTextBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTextBox don't allocate in TEXTBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNTextBox = NULL;
		}
	}
	else
	{
		if (!g_pNTextBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNTextBox don't allocate 2 in TEXTBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetTextBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetTextBox(PINIVALUE pIniValue)
{
	// Setting Control.
	SettingControl((CNControl*)g_pNTextBox, pIniValue);
	
	// Setting Shape.
	SettingShape((CNShape*)g_pNTextBox, pIniValue);
	
	// Setting Text.
	SettingText(g_pNTextBox, pIniValue);
}

BOOL	CreateEffectTextBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNTextBox need to SCREEN. TEXTBOX\n")));
		goto ErrorReturn;
	}

	// [TEXTBOX][/TEXTBOX]
	if (pIniValue->Key == DES_EFFECTTEXTBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNEffectTextBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTextBox don't Deallocate in EFFECTTEXTBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNEffectTextBox = (CNEffectTextBox*) new CNEffectTextBox;
			if (!g_pNEffectTextBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTextBox allocate Failed in EFFECTTEXTBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNEffectTextBox);
		}
		else
		{
			if (!g_pNEffectTextBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTextBox don't allocate in EFFECTTEXTBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNEffectTextBox = NULL;
		}
	}
	else
	{
		if (!g_pNEffectTextBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNTextBox don't allocate 2 in EFFECTTEXTBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetEffectTextBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetEffectTextBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_EFFECT)
	{
		if (pIniValue->Values[0] == DES_SLIDING && pIniValue->Values[1] == DES_LEFT)
			g_pNEffectTextBox->SetEffectType(EFFECT_SLIDING_LEFT);
		else if (pIniValue->Values[0] == DES_SLIDING && pIniValue->Values[1] == DES_RIGHT)
			g_pNEffectTextBox->SetEffectType(EFFECT_SLIDING_RIGHT);
		else if (pIniValue->Values[0] == DES_TYPING && pIniValue->Values[1] == DES_LEFT)
			g_pNEffectTextBox->SetEffectType(EFFECT_TYPING_LEFT);
		else if (pIniValue->Values[0] == DES_TYPING && pIniValue->Values[1] == DES_CENTER)
			g_pNEffectTextBox->SetEffectType(EFFECT_TYPING_CENTER);
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNEffectTextBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNEffectTextBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNEffectTextBox, pIniValue);
	}
}

BOOL	CreateEditBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNEditBox need to SCREEN. EDITBOX\n")));
		goto ErrorReturn;
	}

	// [EDITBOX][/EDITBOX]
	if (pIniValue->Key == DES_EDITBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNEditTextBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNEditBox don't Deallocate in EDITBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNEditTextBox = (CNEditTextBox*) new CNEditTextBox;
			if (!g_pNEditTextBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNEditBox allocate Failed in EDITBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNEditTextBox);
		}
		else
		{
			if (!g_pNEditTextBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNEditBox don't allocate in EDITBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNEditTextBox = NULL;
		}
	}
	else
	{
		if (!g_pNEditTextBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNEditBox don't allocate 2 in EDITBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetEditBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID SetEditBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_MAXINCHAR)
	{
		g_pNEditTextBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
	}
	// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
	else if(pIniValue->Key == DES_MINCHAR)
	{
		g_pNEditTextBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
	}
	// end of [#182]
	else if (pIniValue->Key == DES_TYPE)
	{
		if (pIniValue->Values[0] == DES_PASSWORD)
			g_pNEditTextBox->SetTextType(TTYPE_PASSWORD);
		else if (pIniValue->Values[0] == DES_IPADDRESS)				// [#470] NH AIREAT 2008.12.22 : IP ADDRESS Type 추가.
			g_pNEditTextBox->SetTextType(TTYPE_IPADDRESS);
	}
	else if (pIniValue->Key == DES_AUTO_RUN)
	{
		g_pNEditTextBox->SetAutoRun(Asc2Int(pIniValue->Values[0]));
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNEditTextBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNEditTextBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNEditTextBox, pIniValue);
	}
}

BOOL	CreateChangePWBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNChangePWBox need to SCREEN. CHANGEPWBOX\n")));
		goto ErrorReturn;
	}

	// [EDITBOX][/EDITBOX]
	if (pIniValue->Key == DES_CHANGE_PW_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNChangePWBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNChangePWBox don't Deallocate in EDITBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNChangePWBox = (CNChangePWBox*) new CNChangePWBox;
			if (!g_pNChangePWBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNChangePWBox allocate Failed in EDITBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNChangePWBox);
		}
		else
		{
			if (!g_pNChangePWBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNChangePWBox don't allocate in EDITBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNChangePWBox = NULL;
		}
	}
	else
	{
		if (!g_pNChangePWBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNChangePWBox don't allocate 2 in EDITBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetChangePWBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetChangePWBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_MAXINCHAR)
	{
		g_pNChangePWBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
	}
	// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
	else if(pIniValue->Key == DES_MINCHAR)
	{
		g_pNChangePWBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
	}
	// end of [#182]
	else if (pIniValue->Key == DES_POS_CUR_BOX && pIniValue->nValue == 4)
	{
		g_pNChangePWBox->SetPWBoxRect(CPTYPE_CUR,
			Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
			Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]));
	}
	else if (pIniValue->Key == DES_POS_NEW_BOX && pIniValue->nValue == 4)
	{
		g_pNChangePWBox->SetPWBoxRect(CPTYPE_NEW,
			Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
			Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]));
	}
	else if (pIniValue->Key == DES_POS_NEW_RE_BOX && pIniValue->nValue == 4)
	{
		g_pNChangePWBox->SetPWBoxRect(CPTYPE_NEWRE,
			Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
			Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]));
	}
	// [#16] NH KGS 2008.03.13 Password change
	else if (pIniValue->Key == DES_AUTO_RUN)
	{
		g_pNChangePWBox->SetAutoRun(Asc2Int(pIniValue->Values[0]));
	}
	// end of [#16]
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNChangePWBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNChangePWBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNChangePWBox, pIniValue);
	}
}

BOOL	CreateButtonBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNButtonBox need to SCREEN. BUTTONBOX\n")));
		goto ErrorReturn;
	}

	// [BUTTON][/BUTTON]
	if (pIniValue->Key == DES_BUTTON)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNButtonBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNButtonBox don't Deallocate in BUTTONBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNButtonBox = (CNButtonBox*) new CNButtonBox;
			if (!g_pNButtonBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNButtonBox allocate Failed in BUTTONBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNButtonBox);
		}
		else
		{
			if (!g_pNButtonBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNButtonBox don't allocate in BUTTONBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNButtonBox = NULL;
		}
	}
	else
	{
		if (!g_pNButtonBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNButtonBox don't allocate 2 in BUTTONBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetButtonBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetButtonBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_TYPE)
	{
		g_pNButtonBox->SetTextType(pIniValue->Values[0]);
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNButtonBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNButtonBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNButtonBox, pIniValue);
	}
}

BOOL	CreateVKeyButtonBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNVKeyButtonBox need to SCREEN. VKEYBUTTONBOX\n")));
		goto ErrorReturn;
	}

	// [VKEYBUTTON][/VKEYBUTTON]
	if (pIniValue->Key == DES_VKEYBUTTON)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNVKeyButtonBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNVKeyButtonBox don't Deallocate in BUTTONBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNVKeyButtonBox = (CNVKeyButtonBox*) new CNVKeyButtonBox;
			if (!g_pNVKeyButtonBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNVKeyButtonBox allocate Failed in VKEYBUTTONBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNVKeyButtonBox);
		}
		else
		{
			if (!g_pNVKeyButtonBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNVKeyButtonBox don't allocate in VKEYBUTTONBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNVKeyButtonBox = NULL;
		}
	}
	else
	{
		if (!g_pNVKeyButtonBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNVKeyButtonBox don't allocate 2 in VKEYBUTTONBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetVKeyButtonBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetVKeyButtonBox(PINIVALUE pIniValue)
{
	// Setting Control.
	SettingControl((CNControl*)g_pNVKeyButtonBox, pIniValue);
	
	// Setting Shape.
	SettingShape((CNShape*)g_pNVKeyButtonBox, pIniValue);
	
	// Setting Text.
	SettingText((CNTextBox*)g_pNVKeyButtonBox, pIniValue);
}

BOOL	CreateSWKeyBox(PINIVALUE	pIniValue)
{
//	SIZE	szTemp;
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNSWKeyBox need to SCREEN. SWKEYBOX\n")));
		goto ErrorReturn;
	}

	// [SWKEY][/SWKEY]
	if (pIniValue->Key == DES_SW_KEY)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNSWKeyBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNSWKeyBox don't Deallocate in SWKEYBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNSWKeyBox = (CNSWKeyBox*) new CNSWKeyBox;
			if (!g_pNSWKeyBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNSWKeyBox allocate Failed in SWKEYBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNSWKeyBox);
		}
		else
		{
			if (!g_pNSWKeyBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNSWKeyBox don't allocate in SWKEYBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNSWKeyBox = NULL;
		}
	}
	else
	{
		if (!g_pNSWKeyBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNSWKeyBox don't allocate 2 in SWKEYBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetSWKeyBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetSWKeyBox(PINIVALUE pIniValue)
{
	SIZE	szTemp;

	// NumberSize=?,?
	if (pIniValue->Key == DES_NUMBER_SIZE)
	{
		if (pIniValue->nValue == 2)
		{
			szTemp.cx = Asc2Int(pIniValue->Values[0]);
			szTemp.cy = Asc2Int(pIniValue->Values[1]);
			
			// Adjust Scale
			{
				int currentX = g_sizeScreen.cx;
				int currentY = g_sizeScreen.cy;
				
				//float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);
				//float ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);
				
				float ScreenRateX = (float)((float)currentX / (float)g_BaseSizeScreen.cx);
				float ScreenRateY = (float)((float)currentY / (float)g_BaseSizeScreen.cy);
				
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
			
			g_pNSWKeyBox->SetNumberSize(szTemp);
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
				int currentX = g_sizeScreen.cx;
				int currentY = g_sizeScreen.cy;
				
				//float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);
				//float ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);
				
				float ScreenRateX = (float)((float)currentX / (float)g_BaseSizeScreen.cx);
				float ScreenRateY = (float)((float)currentY / (float)g_BaseSizeScreen.cy);
				
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
			
			g_pNSWKeyBox->SetGuideSize(szTemp);
		}
	}
	// SpaceSize=?,?
	else if (pIniValue->Key == DES_SPACE_SIZE)
	{
		if (pIniValue->nValue == 2)
		{
			szTemp.cx = Asc2Int(pIniValue->Values[0]);
			szTemp.cy = Asc2Int(pIniValue->Values[1]);
			
			g_pNSWKeyBox->SetSpaceSize(szTemp);
		}
	}
	else
	{
		
		// Setting Control.
		SettingControl((CNControl*)g_pNSWKeyBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNSWKeyBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNSWKeyBox, pIniValue);
	}
}

BOOL	CreateEnDisableBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNEnDisableBox need to SCREEN. ENABLEBOX\n")));
		goto ErrorReturn;
	}

	// [ENABLEBOX][/ENABLEBOX]
	if (pIniValue->Key == DES_TOGGLE_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNSWKeyBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNEnDisableBox don't Deallocate in ENABLEBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNToggleBox = (CNToggleBox*) new CNToggleBox;
			if (!g_pNToggleBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNEnDisableBox allocate Failed in ENABLEBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNToggleBox);
		}
		else
		{
			if (!g_pNToggleBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNEnDisableBox don't allocate in ENABLEBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNToggleBox = NULL;
		}
	}
	else
	{
		if (!g_pNToggleBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNEnDisableBox don't allocate 2 in ENABLEBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetEnDisableBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetEnDisableBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_ENABLE_TEXT)
	{
		g_pNToggleBox->SetEnableText(pIniValue->Values[0]);
	}
	else if (pIniValue->Key == DES_DISABLE_TEXT)
	{
		g_pNToggleBox->SetDisableText(pIniValue->Values[0]);
	}
	else
	{			
		// Setting Control.
		SettingControl((CNControl*)g_pNToggleBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNToggleBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNToggleBox, pIniValue);
	}
}

BOOL	CreateEJNLBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNEJNLBox need to SCREEN. EJNLBOX\n")));
		goto ErrorReturn;
	}

	// [EJNLBOX][/EJNLBOX]
	if (pIniValue->Key == DES_EJNL_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNEJNLBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNEJNLBox don't Deallocate in EJNLBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNEJNLBox = (CNEJNLBox*) new CNEJNLBox;
			if (!g_pNEJNLBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNEJNLBox allocate Failed in EJNLBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNEJNLBox);
		}
		else
		{
			if (!g_pNEJNLBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNEJNLBox don't allocate in EJNLBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNEJNLBox = NULL;
		}
	}
	else
	{
		if (!g_pNEJNLBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNEJNLBox don't allocate 2 in EJNLBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetEJNLBox(pIniValue);
		}

	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetEJNLBox(PINIVALUE pIniValue)
{
	// Setting Control.
	SettingControl((CNControl*)g_pNEJNLBox, pIniValue);
	
	// Setting Shape.
	SettingShape((CNShape*)g_pNEJNLBox, pIniValue);
	
	// Setting Text.
	SettingText((CNTextBox*)g_pNEJNLBox, pIniValue);
}

BOOL	CreateDateBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNDateBox need to SCREEN. DATEBOX\n")));
		goto ErrorReturn;
	}

	// [EJNLBOX][/EJNLBOX]
	if (pIniValue->Key == DES_DATE_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNDateBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNDateBox don't Deallocate in DATEBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNDateBox = (CNDateBox*) new CNDateBox;
			if (!g_pNDateBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNDateBox allocate Failed in DATEBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNDateBox);
		}
		else
		{
			if (!g_pNDateBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNDateBox don't allocate in DATEBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNDateBox = NULL;
		}
	}
	else
	{
		if (!g_pNDateBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNDateBox don't allocate 2 in DATEBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetDateBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetDateBox(PINIVALUE pIniValue)
{
	// Setting Control.
	SettingControl((CNControl*)g_pNDateBox, pIniValue);
	
	// Setting Shape.
	SettingShape((CNShape*)g_pNDateBox, pIniValue);
	
	// Setting Text.
	SettingText((CNTextBox*)g_pNDateBox, pIniValue);
}

BOOL	CreateIPBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNIPBox need to SCREEN. IPBOX\n")));
		goto ErrorReturn;
	}

	// [EJNLBOX][/EJNLBOX]
	if (pIniValue->Key == DES_IP_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNIPBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNIPBox don't Deallocate in IPBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNIPBox = (CNIPBox*) new CNIPBox;
			if (!g_pNIPBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNIPBox allocate Failed in IPBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNIPBox);
		}
		else
		{
			if (!g_pNIPBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNIPBox don't allocate in IPBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNIPBox = NULL;
		}
	}
	else
	{
		if (!g_pNIPBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNIPBox don't allocate 2 in IPBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetIPBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetIPBox(PINIVALUE pIniValue)
{
	// Setting Control.
	SettingControl((CNControl*)g_pNIPBox, pIniValue);
	
	// Setting Shape.
	SettingShape((CNShape*)g_pNIPBox, pIniValue);
	
	// Setting Text.
	SettingText((CNTextBox*)g_pNIPBox, pIniValue);
}

BOOL	CreateBinBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNBinBox need to SCREEN. BINBOX\n")));
		goto ErrorReturn;
	}

	// [BINBOX][/BINBOX]
	if (pIniValue->Key == DES_BIN_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNBinBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNBinBox don't Deallocate in BINBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNBinBox = (CNBinBox*) new CNBinBox;
			if (!g_pNBinBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNBinBox allocate Failed in BINBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNBinBox);
		}
		else
		{
			if (!g_pNBinBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNBinBox don't allocate in BINBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNBinBox = NULL;
		}
	}
	else
	{
		if (!g_pNBinBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNBinBox don't allocate 2 in BINBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetBinBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetBinBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_POS_INDEX && pIniValue->nValue == 4)
	{
		g_pNBinBox->SetBinBoxRect(BNTYPE_INDEX,
			Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
			Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]));
	}
	else if (pIniValue->Key == DES_POS_BIN && pIniValue->nValue == 4)
	{
		g_pNBinBox->SetBinBoxRect(BNTYPE_BIN,
			Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]),
			Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]));
	}
	else
	{
		
		// Setting Control.
		SettingControl((CNControl*)g_pNBinBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNBinBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNBinBox, pIniValue);
	}
}

BOOL	CreateCentBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNCentBox need to SCREEN. CENTBOX\n")));
		goto ErrorReturn;
	}

	// [CENTBOX][/CENTBOX]
	if (pIniValue->Key == DES_CENT_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNCentBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNCentBox don't Deallocate in CENTBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNCentBox = (CNCentBox*) new CNCentBox;
			if (!g_pNCentBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNCentBox allocate Failed in CENTBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNCentBox);
		}
		else
		{
			if (!g_pNCentBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNCentBox don't allocate in CENTBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNCentBox = NULL;
		}
	}
	else
	{
		if (!g_pNCentBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNCentBox don't allocate 2 in CENTBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetCentBox(pIniValue);
		}		
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetCentBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_MAXINCHAR)
	{
		g_pNCentBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
	}
	// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
	else if(pIniValue->Key == DES_MINCHAR)
	{
		g_pNCentBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
	}
	// end of [#182]
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNCentBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNCentBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNCentBox, pIniValue);
	}
}

BOOL	CreateDollarBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNDollarBox need to SCREEN. DOLLARBOX\n")));
		goto ErrorReturn;
	}

	// [DOLLARBOX][/DOLLARBOX]
	if (pIniValue->Key == DES_DOLLAR_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNDollarBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNDollarBox don't Deallocate in DOLLARBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNDollarBox = (CNDollarBox*) new CNDollarBox;
			if (!g_pNDollarBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] g_pNDollarBox allocate Failed in DOLLARBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNDollarBox);
		}
		else
		{
			if (!g_pNDollarBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] NDollarBox don't allocate in DOLLARBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNDollarBox = NULL;
		}
	}
	else
	{
		if (!g_pNDollarBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] NDollarBox don't allocate 2 in DOLLARBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetDollarBox(pIniValue);
		}		
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetDollarBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_MAXINCHAR)
	{
		g_pNDollarBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
	}
	// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
	else if (pIniValue->Key == DES_MINCHAR)
	{
		g_pNDollarBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
	}
	// end of [#182]
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNDollarBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNDollarBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNDollarBox, pIniValue);
	}
}

BOOL	CreateAdvBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNAdvBox need to SCREEN. ADVBOX\n")));
		goto ErrorReturn;
	}

	// [ADVBOX][/ADVBOX]
	if (pIniValue->Key == DES_ADV_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNAdvBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNAdvBox don't Deallocate in ADVBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNAdvBox = (CNAdvBox*) new CNAdvBox;
			if (!g_pNAdvBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNAdvBox allocate Failed in ADVBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNAdvBox);
		}
		else
		{
			if (!g_pNAdvBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNAdvBox don't allocate in ADVBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNAdvBox = NULL;
		}
	}
	else
	{
		if (!g_pNAdvBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNAdvBox don't allocate 2 in ADVBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetAdvBox(pIniValue);
		}		
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetAdvBox(PINIVALUE pIniValue)
{
	// [#478] NH AIREAT 2009.01.07 : 광고 화면 해상도별 지원
	CString	strFile;
	strFile.Format(_T("%s_%d_%d"), DES_FILE, g_sizeScreen.cx, g_sizeScreen.cy);
	// end of [#478]
	
	// [#478] NH AIREAT 2009.01.07 : 광고 화면 해상도별 지원
	//		if (pIniValue->Key == DES_FILE)
	//		{
	//			g_pNAdvBox->SetFileName(pIniValue->Values[0]);
	//		}
	if (pIniValue->Key == strFile)
	{
		g_pNAdvBox->SetFileName(pIniValue->Values[0]);
	}
	// end of [#478]
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNAdvBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNAdvBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNAdvBox, pIniValue);
	}
}

BOOL	CreateAniBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNAniBox need to SCREEN. ANIBOX\n")));
		goto ErrorReturn;
	}

	// [ANIBOX][/ANIBOX]
	if (pIniValue->Key == DES_ANI_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNAniBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNAniBox don't Deallocate in ANIBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNAniBox = (CNAnimation*) new CNAnimation;
			if (!g_pNAniBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNAniBox allocate Failed in ANIBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNAniBox);
		}
		else
		{
			if (!g_pNAniBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNAniBox don't allocate in ANIBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNAniBox = NULL;
		}
	}
	else
	{
		if (!g_pNAniBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNAniBox don't allocate 2 in ANIBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetAniBox(pIniValue);
		}		
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetAniBox(PINIVALUE pIniValue)
{
	CString	strFrame;
	strFrame.Format(_T("%s_%d_%d"), DES_FRAME, g_sizeScreen.cx, g_sizeScreen.cy);
	
	if (pIniValue->Key == strFrame && pIniValue->nValue == 2)
	{
		CString strFilePrefix;
		strFilePrefix = g_strMasterPath + pIniValue->Values[1];
		g_pNAniBox->SetAniImage(Asc2Int(pIniValue->Values[0]), strFilePrefix);
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNAniBox, pIniValue);
	}
}

BOOL	CreateTableEditBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNTableEditBox need to SCREEN. TABLEEDITBOX\n")));
		goto ErrorReturn;
	}

	// [TABLEEDITBOX][/TABLEEDITBOX]
	if (pIniValue->Key == DES_TABLE_EDITBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNTableEditBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTableEditBox don't Deallocate in TABLEEDITBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNTableEditBox = (CNTableEditTextBox*) new CNTableEditTextBox;
			if (!g_pNTableEditBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTableEditBox allocate Failed in TABLEEDITBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNTableEditBox);
		}
		else
		{
			if (!g_pNTableEditBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTableEditBox don't allocate in TABLEEDITBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNTableEditBox = NULL;
		}
	}
	else
	{
		if (!g_pNTableEditBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNTableEditBox don't allocate 2 in TABLEEDITBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetTableEditBox(pIniValue);
		}		
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetTableEditBox(PINIVALUE pIniValue)
{
	CString	strFrame;
	strFrame.Format(_T("%s_%d_%d"), DES_FRAME, g_sizeScreen.cx, g_sizeScreen.cy);
	
	//CNResource *pNormal = NULL, *pSelect = NULL;
	CNResource	*pDisable=NULL, *pEnable=NULL, *pSelect=NULL;

	// AllowRowColumn=row,column
	if (pIniValue->Key == DES_ALLOW_ROW_COL)
	{
		if (pIniValue->nValue == 2)
			g_pNTableEditBox->SetAllowAmount(Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]));
	}
	// GuideTextColor=normal,select
	else if (pIniValue->Key == DES_GUIDE_TEXT_COLOR)
	{
		// normal
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pEnable);
		}
		// select
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
		}

		g_pNTableEditBox->SetGuideTextColor((CNColor*)pEnable, (CNColor*)pSelect);
	}
	// GuideFont=normal,select
	else if (pIniValue->Key == DES_GUIDE_FONT)
	{
		// normal
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pEnable);
		}
		// select
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
		}

		g_pNTableEditBox->SetGuideFont(pEnable, pSelect);
	}
	// GuideBrush=normal,select
	else if (pIniValue->Key == DES_GUIDE_BRUSH)
	{
		// normal
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pEnable);
		}
		// select
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
		}
		
		g_pNTableEditBox->SetGuideBrush(pEnable, pSelect);
	}
	// ElementPen=normal
	else if (pIniValue->Key == DES_ELEMENT_PEN)
	{
		// Disable
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pDisable);
		}
		// Enable
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pEnable);
		}
		// Select
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pSelect);
		}

		g_pNTableEditBox->SetElementPen(pDisable, pEnable, pSelect);
	}
	// ElementFont=disable,enable,select
	else if (pIniValue->Key == DES_ELEMENT_FONT)
	{
		// Disable
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pDisable);
		}
		// Enable
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pEnable);
		}
		// Select
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pSelect);
		}

		g_pNTableEditBox->SetElementFont(pDisable, pEnable, pSelect);
	}
	// ElementBrush=disable,enable,select
	else if (pIniValue->Key == DES_ELEMENT_BRUSH)
	{
		// Disable
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pDisable);
		}
		// Enable
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pEnable);
		}
		// Select
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pSelect);
		}

		g_pNTableEditBox->SetElementBrush(pDisable, pEnable, pSelect);
	}
	// ElementTextColor=disable,enable,select
	else if (pIniValue->Key == DES_ELEMENT_TEXT_COLOR)
	{
		// Disable
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pDisable);
		}
		// Enable
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pEnable);
		}
		// Select
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pSelect);
		}

		g_pNTableEditBox->SetElementTextColor((CNColor*)pDisable, (CNColor*)pEnable, (CNColor*)pSelect);
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNTableEditBox, pIniValue);

		// Setting Shape.
		SettingShape((CNShape*)g_pNTableEditBox, pIniValue);
	}
}

BOOL	CreateAlphaKeyBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNAlphaKeyBox need to SCREEN. ALPHAKEYBOX\n")));
		goto ErrorReturn;
	}

	// [ALPHAKEYBOX][/ALPHAKEYBOX]
	if (pIniValue->Key == DES_ALPHA_KEYBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNAlphaKeyBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNAlphaKeyBox don't Deallocate in ALPHAKEYBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNAlphaKeyBox = (CNAlphaKeyBox*) new CNAlphaKeyBox;
			if (!g_pNAlphaKeyBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNAlphaKeyBox allocate Failed in ALPHAKEYBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNAlphaKeyBox);
		}
		else
		{
			if (!g_pNAlphaKeyBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNAlphaKeyBox don't allocate in ALPHAKEYBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNAlphaKeyBox = NULL;
		}
	}
	else
	{
		if (!g_pNAlphaKeyBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNAlphaKeyBox don't allocate 2 in ALPHAKEYBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetAlphaKeyBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetAlphaKeyBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_VKEY_TYPE)
	{
		CString strVKeyType = pIniValue->Values[0];
		
		strVKeyType.MakeUpper();
		if (strVKeyType == _T("UPPER"))
			g_pNAlphaKeyBox->SetAlphaKeyType(VKEY_ALPHA_UPPER);
		else if (strVKeyType == _T("LOWER"))
			g_pNAlphaKeyBox->SetAlphaKeyType(VKEY_ALPHA_LOWER);
		else if (strVKeyType == _T("NUMBER"))
			g_pNAlphaKeyBox->SetAlphaKeyType(VKEY_ALPHA_NUMBER);
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNAlphaKeyBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNAlphaKeyBox, pIniValue);
		
		// Setting Virtual Key
		SettingVKey((CNVirtualKeyBox*)g_pNAlphaKeyBox, pIniValue);
	}
}

BOOL	CreateTableKeyBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNTableKeyBox need to SCREEN. TABLEKEYBOX\n")));
		goto ErrorReturn;
	}

	// [TABLEKEYBOX][/TABLEKEYBOX]
	if (pIniValue->Key == DES_TABLE_KEYBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNTableKeyBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTableKeyBox don't Deallocate in TABLEKEYBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNTableKeyBox = (CNTableKeyBox*) new CNTableKeyBox;
			if (!g_pNTableKeyBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTableKeyBox allocate Failed in TABLEKEYBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNTableKeyBox);
		}
		else
		{
			if (!g_pNTableKeyBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNTableKeyBox don't allocate in TABLEKEYBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNTableKeyBox = NULL;
		}
	}
	else
	{
		if (!g_pNTableKeyBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNTableKeyBox don't allocate 2 in TABLEKEYBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetTableKeyBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetTableKeyBox(PINIVALUE pIniValue)
{
	// Setting Control.
	SettingControl((CNControl*)g_pNTableKeyBox, pIniValue);
	
	// Setting Shape.
	SettingShape((CNShape*)g_pNTableKeyBox, pIniValue);
	
	// Setting Virtual Key
	SettingVKey((CNVirtualKeyBox*)g_pNTableKeyBox, pIniValue);
}

BOOL	CreateThreeStateBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNThreeStateBox need to SCREEN. THREESTATEBOX\n")));
		goto ErrorReturn;
	}

	// [THREESTATEBOX][/THREESTATEBOX]
	if (pIniValue->Key == DES_THREE_STATEBOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNThreeStateBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNThreeStateBox don't Deallocate in THREESTATEBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNThreeStateBox = (CNThreeStateBox*) new CNThreeStateBox;
			if (!g_pNThreeStateBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNThreeStateBox allocate Failed in THREESTATEBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNThreeStateBox);
		}
		else
		{
			if (!g_pNThreeStateBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNThreeStateBox don't allocate in THREESTATEBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNThreeStateBox = NULL;
		}
	}
	else
	{
		if (!g_pNThreeStateBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNThreeStateBox don't allocate 2 in THREESTATEBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetThreeStateBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetThreeStateBox(PINIVALUE pIniValue)
{
	CNResource *pHide = NULL, *pOn = NULL, *pOff = NULL;

	if (pIniValue->Key == DES_STATE_IMAGE)
	{
		// Hide
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pHide);
		}
		
		// On
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pOn);
		}

		// Off
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pOff);
		}

		g_pNThreeStateBox->SetStateImage((CNPicture*)pHide, (CNPicture*)pOn, (CNPicture*)pOff);
	}
	else if (pIniValue->Key == DES_STATE_PEN)
	{
		// Hide
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pHide);
		}
		
		// On
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pOn);
		}

		// Off
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pOff);
		}

		g_pNThreeStateBox->SetStatePen(pHide, pOn, pOff);
	}
	else if (pIniValue->Key == DES_STATE_BRUSH)
	{
		// Hide
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pHide);
		}
		
		// On
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pOn);
		}

		// Off
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pOff);
		}

		g_pNThreeStateBox->SetStateBrush(pHide, pOn, pOff);
	}
	else if (pIniValue->Key == DES_STATE_FONT)
	{
		// Hide
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pHide);
		}
		
		// On
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pOn);
		}

		// Off
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pOff);
		}

		g_pNThreeStateBox->SetStateFont(pHide, pOn, pOff);
	}
	else if (pIniValue->Key == DES_STATE_TEXT_COLOR)
	{
		// Hide
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pHide);
		}
		
		// On
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pOn);
		}

		// Off
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pOff);
		}

		g_pNThreeStateBox->SetStateTextColor((CNColor*)pHide, (CNColor*)pOn, (CNColor*)pOff);
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

		g_pNThreeStateBox->SetStateBlink(bHide, bOn, bOff);
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNThreeStateBox, pIniValue);

		// Setting Shape.
		SettingShape((CNShape*)g_pNThreeStateBox, pIniValue);

		// Setting Text.
		SettingText((CNTextBox*)g_pNThreeStateBox, pIniValue);
	}
}

//[#543] SOOK 2009.07.19 BIN EDIT DELETE를 공통으로 쓰기 위해 BINBINDEXBOX 컨트롤 제작함 
BOOL	CreateBinIndexBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNBinIndexBox need to SCREEN. BININDEXBOX\n")));
		goto ErrorReturn;
	}

	// [EJNLBOX][/EJNLBOX]
	if (pIniValue->Key == DES_BININDEX_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNBinIndexBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNBinIndexBox don't Deallocate in BININDEXBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNBinIndexBox = (CNBinIndexBox *) new CNBinIndexBox;
			if (!g_pNBinIndexBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNBinIndexBox allocate Failed in BININDEXBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNBinIndexBox);
		}
		else
		{
			if (!g_pNBinIndexBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNBinIndexBox don't allocate in BININDEXBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNBinIndexBox = NULL;
		}
	}
	else
	{
		if (!g_pNBinIndexBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNBinIndexBox don't allocate 2 in BININDEXBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetBinIndexBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}
//end of [#543]

// V1.0.2.4 2018.07.12
VOID	SetBinIndexBox(PINIVALUE pIniValue)
{
	// Setting Control.
	SettingControl((CNControl*)g_pNBinIndexBox, pIniValue);
	
	// Setting Shape.
	SettingShape((CNShape*)g_pNBinIndexBox, pIniValue);
	
	// Setting Text.
	SettingText((CNTextBox*)g_pNBinIndexBox, pIniValue);
}

// [#774] US Justin 2014.12.16 Add Decimal Box
BOOL	CreateDecimalBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNDecimalBox need to SCREEN. DECIMALBOX\n")));
		goto ErrorReturn;
	}

	// [DECIMALBOX][/DECIMALBOX]
	if (pIniValue->Key == DES_DECIMAL_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNDecimalBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNDecimalBox don't Deallocate in DECIMALBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNDecimalBox = (CNDecimalBox*) new CNDecimalBox;
			if (!g_pNDecimalBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNDecimalBox allocate Failed in DECIMALBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNDecimalBox);
		}
		else
		{
			if (!g_pNDecimalBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNDecimalBox don't allocate in DECIMALBOX\n")));
				goto ErrorReturn;
			}			
			g_pNDecimalBox = NULL;
		}
	}
	else
	{
		if (!g_pNDecimalBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CDECIMaLBox don't allocate 2 in DECIMALBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetDecimalBox(pIniValue);
		}		
	}

	return TRUE;

	ErrorReturn:
	return FALSE;
}
// End of [#774]

// V1.0.2.4 2018.07.12
VOID	SetDecimalBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_MAXINCHAR)
	{
		g_pNDecimalBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
	}
	else if(pIniValue->Key == DES_MINCHAR)
	{
		g_pNDecimalBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNDecimalBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNDecimalBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNDecimalBox, pIniValue);
	}
}

// [#810] US Justin 2015.10.09 
BOOL	CreateCentClearBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNCentClearBox need to SCREEN. CENTCLEARBOX\n")));
		goto ErrorReturn;
	}

	// [CENTCLEARBOX][/CENTCLEARBOX]
	if (pIniValue->Key == DES_CENTCLEARL_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNCentClearBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNCentClearBox don't Deallocate in CENTCLEARBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNCentClearBox = (CNCentClearBox*) new CNCentClearBox;
			if (!g_pNCentClearBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNCentClearBox allocate Failed in CENTCLEARBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNCentClearBox);
		}
		else
		{
			if (!g_pNCentClearBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNCentClearBox don't allocate in CENTCLEARBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNCentClearBox = NULL;
		}
	}
	else
	{
		if (!g_pNCentClearBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNCentClearBox don't allocate 2 in CENTCLEARBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetCentClearBox(pIniValue);
		}		
	}
	return TRUE;

ErrorReturn:
	return FALSE;
}
// End of [#810]

// V1.0.2.4 2018.07.12
VOID	SetCentClearBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_MAXINCHAR)
	{
		g_pNCentClearBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
	}
	// [#182] NH KGS 2008.05.07 Edit Control 하위 클래스 최소 입력 글자 설정 기능 추가
	else if(pIniValue->Key == DES_MINCHAR)
	{
		g_pNCentClearBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
	}
	// end of [#182]
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNCentClearBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNCentClearBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNCentClearBox, pIniValue);
	}
}

/** *************************************************************
*	@brief	CreateKeyTestBox
*	@param	PINIVALUE pIniValue		Script의 속성 값들
*	@retval	TRUE	성공
*	@retval	FALSE	실패
*****************************************************************/
BOOL	CreateKeyTestBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNKeyTestBox need to SCREEN. KEYTESTBOX\n")));
		goto ErrorReturn;
	}

	// [KEYTESTBOX][/KEYTESTBOX]
	if (pIniValue->Key == DES_KEYTEST_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNKeyTestBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNKeyTestBox don't Deallocate in KEYTESTBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNKeyTestBox = (CNKeyTestBox*) new CNKeyTestBox;
			if (!g_pNKeyTestBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] g_pNKeyTestBox allocate Failed in KEYTESTBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNKeyTestBox);
		}
		else
		{
			if (!g_pNKeyTestBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNKeyTestBox don't allocate in KEYTESTBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNKeyTestBox = NULL;
		}
	}
	else
	{
		if (!g_pNKeyTestBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNKeyTestBox don't allocate 2 in KEYTESTBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetKeyTestBox(pIniValue);
		}		
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetKeyTestBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_MAXINCHAR)
	{
		g_pNKeyTestBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
	}
	else if (pIniValue->Key == DES_MINCHAR)
	{
		g_pNKeyTestBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNKeyTestBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNKeyTestBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNKeyTestBox, pIniValue);
	}
}

/** *************************************************************
*	@brief	CreateInputEditBox
*	@param	PINIVALUE pIniValue		Script의 속성 값들
*	@retval	TRUE	성공
*	@retval	FALSE	실패
*****************************************************************/
BOOL	CreateInputEditBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNInputEditBox need to SCREEN. INPUTEDITBOX\n")));
		goto ErrorReturn;
	}

	// [INPUTEDITBOX][/INPUTEDITBOX]
	if (pIniValue->Key == DES_INPUTEDIT_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNInputEditBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNInputEditBox don't Deallocate in INPUTEDITBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNInputEditBox = (CNInputEditBox*) new CNInputEditBox;
			if (!g_pNInputEditBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] g_pNInputEditBox allocate Failed in INPUTEDITBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNInputEditBox);
		}
		else
		{
			if (!g_pNInputEditBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNInputEditBox don't allocate in INPUTEDITBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNInputEditBox = NULL;
		}
	}
	else
	{
		if (!g_pNInputEditBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNInputEditBox don't allocate 2 in KEYTESTBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetInputEditBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetInputEditBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_MAXINCHAR)
	{
		g_pNInputEditBox->SetMaxInputChar(Asc2Int(pIniValue->Values[0]));
	}
	else if (pIniValue->Key == DES_MINCHAR)
	{
		g_pNInputEditBox->SetMinInputChar(Asc2Int(pIniValue->Values[0]));
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNInputEditBox, pIniValue);
		
		// Setting Shape.
		SettingShape((CNShape*)g_pNInputEditBox, pIniValue);
		
		// Setting Text.
		SettingText((CNTextBox*)g_pNInputEditBox, pIniValue);
	}
}

/** *************************************************************
*	@brief	CreateThreeTypeBox
*	@param	PINIVALUE pIniValue		Script의 속성 값들
*	@retval	TRUE	성공
*	@retval	FALSE	실패
*****************************************************************/
BOOL	CreateThreeTypeBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CNThreeTypeBox need to SCREEN. THREETYPEBOX\n")));
		goto ErrorReturn;
	}

	// [THREETYPEBOX][/THREETYPEBOX]
	if (pIniValue->Key == DES_THREETYPE_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNThreeTypeBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNThreeTypeBox don't Deallocate in THREETYPEBOX\n")));
				goto ErrorReturn;
			}

			// allocate.
			g_pNThreeTypeBox = (CNThreeTypeBox*) new CNThreeTypeBox;
			if (!g_pNThreeTypeBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNThreeTypeBox allocate Failed in THREETYPEBOX\n")));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNThreeTypeBox);
		}
		else
		{
			if (!g_pNThreeTypeBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CNThreeTypeBox don't allocate in THREETYPEBOX\n")));
				goto ErrorReturn;
			}
			
			g_pNThreeTypeBox = NULL;
		}
	}
	else
	{

		if (!g_pNThreeTypeBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CNThreeTypeBox don't allocate 2 in THREETYPEBOX\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetThreeTypeBox(pIniValue);
		}
	}

	return TRUE;

ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetThreeTypeBox(PINIVALUE pIniValue)
{
	CNResource *pDisable = NULL, *pEnable = NULL, *pSelect = NULL;

	if (pIniValue->Key == DES_STATE_PEN)
	{
		// Disable
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pDisable);
		}
		
		// Enable
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pEnable);
		}

		// Select
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pSelect);
		}

		g_pNThreeTypeBox->SetStatePen(pDisable, pEnable, pSelect);
	}
	else if (pIniValue->Key == DES_STATE_BRUSH)
	{
		// Disable
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pDisable);
		}
		
		// Enable
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pEnable);
		}

		// Select
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pSelect);
		}

		g_pNThreeTypeBox->SetStateBrush(pDisable, pEnable, pSelect);
	}
	else if (pIniValue->Key == DES_STATE_FONT)
	{
		// Disable
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pDisable);
		}
		
		// Enable
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pEnable);
		}

		// Select
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pSelect);
		}

		g_pNThreeTypeBox->SetStateFont(pDisable, pEnable, pSelect);
	}
	else if (pIniValue->Key == DES_STATE_TEXT_COLOR)
	{
		// Disable
		if (pIniValue->nValue >= 1)
		{
			pIniValue->Values[0].MakeUpper();
			if (pIniValue->Values[0] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[0], &pDisable);
		}
		
		// Enable
		if (pIniValue->nValue >= 2)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pEnable);
		}

		// Select
		if (pIniValue->nValue >= 3)
		{
			pIniValue->Values[2].MakeUpper();
			if (pIniValue->Values[2] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[2], &pSelect);
		}

		g_pNThreeTypeBox->SetStateTextColor((CNColor*)pDisable, (CNColor*)pEnable, (CNColor*)pSelect);
	}
	else
	{
		// Setting Control.
		SettingControlEx((CNControl*)g_pNThreeTypeBox, pIniValue);

		// Setting Shape.
		SettingShape((CNShape*)g_pNThreeTypeBox, pIniValue);

		// Setting Text.
		SettingText((CNTextBox*)g_pNThreeTypeBox, pIniValue);
	}
}

/** *************************************************************
*	@brief	CreateGifBox
*	@param	PINIVALUE pIniValue		Script의 속성 값들
*	@retval	TRUE	성공
*	@retval	FALSE	실패
*****************************************************************/
BOOL	CreateGifBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CreateGifBox need to SCREEN\n")));
		goto ErrorReturn;
	}
	
	// [GIFBOX][/GIFBOX]
	if (pIniValue->Key == DES_GIF_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNGifBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CreateGifBox don't deallocate is failed to SCREEN\n")));
				goto ErrorReturn;
			}
			
			// allocate.
			g_pNGifBox = (CNGifBox*) new CNGifBox;
			if (!g_pNGifBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CreateGifBox allocate is failed to SCREEN\n")));
				goto ErrorReturn;
			}
			
			// Register.
			g_pNScreen->SetAddControl(g_pNGifBox);
		}
		else
		{
			if (!g_pNGifBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CreateGifBox don't allocate is failed to SCREEN\n")));
				goto ErrorReturn;
			}
			
			g_pNGifBox = NULL;
		}
	}
	else
	{		
		if (!g_pNGifBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CreateGifBox don't allocate(2) is failed to SCREEN\n")));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetGifBox(pIniValue);
		}		
	}
	
	return TRUE;
	
ErrorReturn:
	return FALSE;
}

// V1.0.2.4 2018.07.12
VOID	SetGifBox(PINIVALUE pIniValue)
{
	CNResource *pNormal = NULL;
	CNPicture *pPicture = NULL;
	
//	if (pIniValue->Key == strFile && pIniValue->nValue == 1)
	if (pIniValue->Key == DES_PICTURE)
	{
		pIniValue->Values[0].MakeUpper();
		g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		
		pPicture = (CNPicture *)pNormal;

		g_pNGifBox->SetAniImage(pPicture->GetImageFileName());
	}
	else
	{
		// Setting Control.
		SettingControl((CNControl*)g_pNGifBox, pIniValue);
	}
}

/** *************************************************************
*	@brief	CreateImageBox
*	@param	PINIVALUE pIniValue		Script의 속성 값들
*	@retval	TRUE	성공
*	@retval	FALSE	실패
*****************************************************************/
BOOL	CreateImageBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CreateImageBox need to SCREEN\n")));
		goto ErrorReturn;
	}
	
	// [IMAGEBOX][/IMAGEBOX]
	if (pIniValue->Key == DES_IMAGE_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNImageBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CreateImageBox don't deallocate is failed to SCREEN\n")));
				goto ErrorReturn;
			}
			
			// allocate.
			g_pNImageBox = (CNImageBox*) new CNImageBox;
			if (!g_pNImageBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CreateImageBox allocate is failed to SCREEN\n")));
				goto ErrorReturn;
			}
			
			// Register.
			g_pNScreen->SetAddControl(g_pNImageBox);
		}
		else
		{
			if (!g_pNImageBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CreateImageBox don't allocate is failed to SCREEN\n")));
				goto ErrorReturn;
			}
			
			g_pNImageBox = NULL;
		}
	}
	else
	{		
		if (!g_pNImageBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CreateImageBox don't allocate(2) is failed to SCREEN\n")));
			goto ErrorReturn;
		}
		
		if (pIniValue->Key == DES_CONTROL_ID)
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetImageBox(pIniValue);
		}		
	}
	
	return TRUE;
	
ErrorReturn:
	return FALSE;
}

VOID	SetImageBox(PINIVALUE pIniValue)
{
	// Setting Control.
	SettingControl((CNControl*)g_pNImageBox, pIniValue);
	
	// Setting Shape.
	SettingShape((CNShape*)g_pNImageBox, pIniValue);
	
	// Setting Text.
	SettingText((CNTextBox*)g_pNImageBox, pIniValue);
}

/** *************************************************************
*	@brief	CreateEditButtonBox
*	@param	PINIVALUE pIniValue		Script의 속성 값들
*	@retval	TRUE	성공
*	@retval	FALSE	실패
*****************************************************************/
BOOL	CreateEditButtonBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		NHDEBUG(1, (_T("[SCR_IN] CreateEditButtonBox need to SCREEN\n")));
		goto ErrorReturn;
	}
	
	// [EDITBUTTONBOX][/EDITBUTTONBOX]
	if (pIniValue->Key == DES_EDITBUTTON_BOX)
	{
		if (Asc2Int(pIniValue->Values[0]))
		{
			if (g_pNEditButtonBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CreateEditButtonBox don't deallocate is failed to SCREEN\n")));
				goto ErrorReturn;
			}
			
			// allocate.
			g_pNEditButtonBox = (CNEditButtonBox*) new CNEditButtonBox;
			if (!g_pNEditButtonBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CreateEditButtonBox allocate is failed to SCREEN\n")));
				goto ErrorReturn;
			}
			
			// Register.
			g_pNScreen->SetAddControl(g_pNEditButtonBox);
		}
		else
		{
			if (!g_pNEditButtonBox)
			{
				NHDEBUG(1, (_T("[SCR_IN] CreateEditButtonBox don't allocate is failed to SCREEN\n")));
				goto ErrorReturn;
			}
			
			g_pNEditButtonBox = NULL;
		}
	}
	else
	{		
		if (!g_pNEditButtonBox)
		{
			NHDEBUG(1, (_T("[SCR_IN] CreateEditButtonBox don't allocate(2) is failed to SCREEN\n")));
			goto ErrorReturn;
		}
		
		if (pIniValue->Key == DES_CONTROL_ID)
		{
			SearchScreenControlInfo(pIniValue);
		}
		else
		{
			SetEditButtonBox(pIniValue);
		}		
	}
	
	return TRUE;
	
ErrorReturn:
	return FALSE;
}

VOID	SetEditButtonBox(PINIVALUE pIniValue)
{
	// Setting Control.
	SettingControl((CNControl*)g_pNEditButtonBox, pIniValue);
	
	// Setting Shape.
	SettingShape((CNShape*)g_pNEditButtonBox, pIniValue);
	
	// Setting Text.
	SettingText((CNTextBox*)g_pNEditButtonBox, pIniValue);
}


BOOL	SettingControl(CNControl *pControl, PINIVALUE	pIniValue)
{
	if (!pControl)
		return FALSE;

	// Name=""
	if (pIniValue->Key == DES_NAME)
	{
		pControl->SetName(pIniValue->Values[0]);

		// V1.0.2.4 2018.07.19 - Camera의 좌표를 Script의 Pos 값으로 설정하도록 하기 위함.
		if( (_T("502") == g_strScreenID) && (_T("APState1") == pIniValue->Values[0]) )
			g_bPosValue4Camera = TRUE;
		////////////////////////////////////////////////////////////////////////////////
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
									Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]));

			// V1.0.2.4 2018.07.19 - Camera의 좌표를 Script의 Pos 값으로 설정하도록 하기 위함.
			if(TRUE == g_bPosValue4Camera)
			{
				g_rect4Camera.left = Asc2Int(pIniValue->Values[0]);
				g_rect4Camera.top = Asc2Int(pIniValue->Values[1]);
				g_rect4Camera.right = Asc2Int(pIniValue->Values[2]);
				g_rect4Camera.bottom = Asc2Int(pIniValue->Values[3]);

				g_bPosValue4Camera = FALSE;
			}
		////////////////////////////////////////////////
		}
	}

	return TRUE;
}

BOOL	SettingShape(CNShape *pShape, PINIVALUE pIniValue)
{
	CNResource *pNormal = NULL, *pSelect = NULL;

	if (!pShape)
		return FALSE;

	// Brush=normal ID,select ID
	if (pIniValue->Key == DES_BRUSH)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
		}

		pShape->SetBrush(pNormal, pSelect);
	}
	// Pen=normal ID,select ID
	else if (pIniValue->Key == DES_PEN)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
		}

		pShape->SetPen(pNormal, pSelect);
	}
	// Picture=ID
	else if (pIniValue->Key == DES_PICTURE)
	{
		pIniValue->Values[0].MakeUpper();
		g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);

		pShape->SetPicture(pNormal);
	}
	//*LEH
	// Round=xx,xx
	else if (pIniValue->Key == DES_ROUND)
	{
		pShape->SetRoundSize(Asc2Int(pIniValue->Values[0]), Asc2Int(pIniValue->Values[1]));
	}
	////////////////////////

	return TRUE;
}

BOOL	SettingText(CNTextBox *pTextBox, PINIVALUE	pIniValue)
{
	CNResource *pNormal = NULL, *pSelect = NULL;

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
		g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		pTextBox->SetLocaleText(pNormal);
	}
	// Font=Normal ID, Select ID
	else if (pIniValue->Key == DES_FONT)
	{
		pIniValue->Values[0].MakeUpper();
		g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);

		if (pIniValue->nValue == 2)
		{
			pIniValue->Values[1].MakeUpper();
			g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
		}
		
		pTextBox->SetFont(pNormal, pSelect);
	}
	// TextSet=Multiline, Align, Text color
	else if (pIniValue->Key == DES_TEXTSET)
	{
		// Multiline
// 		if (pIniValue->nValue >= 1)
// 		{
// 			pTextBox->SetMultiLine(Asc2Int(pIniValue->Values[0]));
// 		}
		pTextBox->SetMultiLine(TRUE);	// V1.0.2.4 2018.07.17 - 무조건 multi line으로 변경

		// Align
		if (pIniValue->nValue >= 1)
		{
			if (pIniValue->Values[0] == DES_LEFT)
				pTextBox->SetTextAlign(LEFT);
			else if (pIniValue->Values[0] == DES_CENTER)
				pTextBox->SetTextAlign(CENTER);
			else if (pIniValue->Values[0] == DES_RIGHT)
				pTextBox->SetTextAlign(RIGHT);
		}
/*
		// Text Color
		if (pIniValue->nValue >= 5)
		{
			if (pIniValue->nValue == 8)
			{
				pTextBox->SetTextColor(RGB(Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]), Asc2Int(pIniValue->Values[4])),
									   RGB(Asc2Int(pIniValue->Values[5]), Asc2Int(pIniValue->Values[6]), Asc2Int(pIniValue->Values[7])));

			}
			else
			{
				pTextBox->SetTextColor(RGB(Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]), Asc2Int(pIniValue->Values[4])),
									   RGB(Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]), Asc2Int(pIniValue->Values[4])));
			}
		}
*/
		// Normal Text Color
		if (pIniValue->nValue >= 2)
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
			g_NHResources.FindDataOfList(strNormalColor, &pNormal);
			if (pNormal != NULL && pNormal->GetTpye() != RES_COLOR)
				pNormal = NULL;
			g_NHResources.FindDataOfList(strSelectColor, &pSelect);
			if (pSelect != NULL && pSelect->GetTpye() != RES_COLOR)
				pSelect = NULL;

			pTextBox->SetTextColor((CNColor*)pNormal, (CNColor*)pSelect);
		}
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
		CRequest	req;

		req.SetKinD(REQ_NONE);

		// type
		if (pIniValue->nValue >= 1)
		{
			if (pIniValue->Values[0] == DES_FSCMD)
				req.SetKinD(REQ_FSCMD);
			// [#75] NH KGS 2008.04.02 Full FS Command 추가
			else if (pIniValue->Values[0] == DES_FULLFSCMD)
				req.SetKinD(REQ_FULLFSCMD);
			// end of [#75]
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

	return TRUE;
}

BOOL	SettingVKey(CNVirtualKeyBox *pVKeyBox, PINIVALUE pIniValue)
{
	CNResource *pNormal = NULL, *pSelect = NULL;

	if (pVKeyBox == NULL)
		return FALSE;

	if (pIniValue->Key == DES_GUIDE_FONT)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
		}

		pVKeyBox->SetGuideFont(pNormal, pSelect);
	}
	else if (pIniValue->Key == DES_GUIDE_BRUSH)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
		}

		pVKeyBox->SetGuideBrush(pNormal, pSelect);
	}
	else if (pIniValue->Key == DES_GUIDE_PEN)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
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
		g_NHResources.FindDataOfList(strNormalColor, &pNormal);
		if (pNormal != NULL && pNormal->GetTpye() != RES_COLOR)
			pNormal = NULL;
		g_NHResources.FindDataOfList(strSelectColor, &pSelect);
		if (pSelect != NULL && pSelect->GetTpye() != RES_COLOR)
			pSelect = NULL;

		pVKeyBox->SetGuideTextColor((CNColor*)pNormal, (CNColor*)pSelect);
	}
	else if (pIniValue->Key == DES_VKEY_FONT)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
		}

		pVKeyBox->SetVKeyFont(pNormal, pSelect);
	}
	else if (pIniValue->Key == DES_VKEY_BRUSH)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		if (pIniValue->Values[0] != _T("NULL"))
			g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
		}

		pVKeyBox->SetVKeyBrush(pNormal, pSelect);
	}
	else if (pIniValue->Key == DES_VKEY_PEN)
	{
		// normal
		pIniValue->Values[0].MakeUpper();
		if (pIniValue->Values[0] != _T("NULL"))
			g_NHResources.FindDataOfList(pIniValue->Values[0], &pNormal);
		
		// select.
		if (pIniValue->nValue > 1)
		{
			pIniValue->Values[1].MakeUpper();
			if (pIniValue->Values[1] != _T("NULL"))
				g_NHResources.FindDataOfList(pIniValue->Values[1], &pSelect);
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
		g_NHResources.FindDataOfList(strNormalColor, &pNormal);
		if (pNormal != NULL && pNormal->GetTpye() != RES_COLOR)
			pNormal = NULL;
		g_NHResources.FindDataOfList(strSelectColor, &pSelect);
		if (pSelect != NULL && pSelect->GetTpye() != RES_COLOR)
			pSelect = NULL;

		pVKeyBox->SetVKeyTextColor((CNColor*)pNormal, (CNColor*)pSelect);
	}

	return TRUE;
}

/** **********************************************
*	@brief		SettingControl 확장버전 
*	@details	Name 속성을 확장
*	@param CNControl *pControl	resource control 종류 
*	@param PINIVALUE pIniValue	각 속성값
***************************************************/  
BOOL	SettingControlEx(CNControl *pControl, PINIVALUE	pIniValue)
{
	if (!pControl)
		return FALSE;

	// Name=APEditStateXX, APValueXX
	if (pIniValue->Key == DES_NAME)
	{
		if (1 == pIniValue->nValue)
			pControl->SetNameEx(pIniValue->Values[0], NULL);
		else if (2 == pIniValue->nValue)
			pControl->SetNameEx(pIniValue->Values[0], pIniValue->Values[1]);
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
									Asc2Int(pIniValue->Values[2]), Asc2Int(pIniValue->Values[3]));
		}
	}

	return TRUE;
}

// 화면에 사용할 Text를 읽어 리소스로 저장한다.
// [#469][NH] AIREAT 2008.12.20 AP,OP Text 다국어 통합 작업으로 Text Load 루틴 전체적으로 변경
BOOL	LoadScreenText(LPCTSTR FileName)
{
	CNHReadiniFile	Readini;
	PINIVALUE	pwIniValue;
	CString		strFullFileName = g_strMasterPath + FileName;
	CString		strSupportLanguage, strLoadCountry;
	CString		strTemp;

	// Get Country Short cut
	strLoadCountry = _T("US");

	// open Screen Text File
	if (!Readini.Open(strFullFileName, CNHReadiniFile::modeRead, CNHReadiniFile::modeLittleUNICODE))
	{
		NHDEBUG(1, (_T("[SCR_IN] File Open Failed (%s) in TEXT_RES\n"), FileName));
		goto ErrorReturn;
	}

	// Load Screen Text
	while (NULL != (pwIniValue = Readini.ReadiniValue()))
	{
		// [SUPPORT] section
		if (pwIniValue->Section == DES_SUPPORT)
		{
			if (pwIniValue->Key.Left(2) == strLoadCountry && _ttoi(pwIniValue->Values[0]) == 1)
			{
				strSupportLanguage += pwIniValue->Key.Right(3);
				strSupportLanguage += _T(",");
			}
		}		
		// [TEXT] section
		else if (pwIniValue->Section == DES_TEXT)
		{
			if (pwIniValue->Key == DES_ID)
			{
				pwIniValue->Values[0].MakeUpper();
				g_pNLocaleText = (CNLocaleText*) new CNLocaleText(pwIniValue->Values[0], LANGUAGE_MODE_MAX);
				if (g_pNLocaleText)
					g_NHResources.PutOnTailOfList(g_pNLocaleText->GetID(), g_pNLocaleText);
			}
			else if (strSupportLanguage.Find(pwIniValue->Key.Right(3)) != -1)
			{
				strTemp = pwIniValue->Key.Left(2);
				if (strTemp == strLoadCountry || strTemp == _T("US"))
				{
					strTemp = pwIniValue->Key.Right(3);

					if (strTemp == _T("ENG"))
						g_pNLocaleText->AddLocaleText(ENG_MODE, pwIniValue->Values[0]);
					else if (strTemp == _T("SPN"))
						g_pNLocaleText->AddLocaleText(SPN_MODE, pwIniValue->Values[0]);
					else if (strTemp == _T("FRN"))
						g_pNLocaleText->AddLocaleText(FRN_MODE, pwIniValue->Values[0]);
					else if (strTemp == _T("CHN"))
						g_pNLocaleText->AddLocaleText(CHN_MODE, pwIniValue->Values[0]);
					else if (strTemp == _T("KOR"))
						g_pNLocaleText->AddLocaleText(KOR_MODE, pwIniValue->Values[0]);
					else if (strTemp == _T("JPN"))
						g_pNLocaleText->AddLocaleText(JPN_MODE, pwIniValue->Values[0]);
				}
			}
		}
	}

	return TRUE;

ErrorReturn:

	Readini.Close();

	return FALSE;
}
// end of [#469]

// [#469][NH] AIREAT 2008.12.20 AP,OP Text 다국어 통합 작업
//-- 통합작업으로 인해 사용하지 않는 함수 삭제
/*
BOOL	CreateLocaleText(int nLocaleCount, PINIVALUE pwIniValue)
{

	if (pwIniValue->Key == DES_TEXT)
	{
		;
	}
	// ID=""
	else if (pwIniValue->Key == DES_ID)
	{
		g_pNLocaleText = (CNLocaleText*) new CNLocaleText(pwIniValue->Values[0], nLocaleCount);
		if (g_pNLocaleText)
			g_NHResources.PutOnTailOfList(g_pNLocaleText->GetID(), g_pNLocaleText);
	}
	// 1=""
	else
	{
		if (g_pNLocaleText)
		{
			g_pNLocaleText->AddLocaleText(_ttoi(pwIniValue->Key), pwIniValue->Values[0]);
		}
	}

	return TRUE;
}
*/
// end of [#469]

// V1.0.2.4 2018.07.12 - ScreenControl.dat 파일을 검색한 후, 해당 ControlID의 Key정보를 세팅한다.
// Resource 관련 정보(Font, Pos, Text, TextID, TextSet, Picture, Color) 이외의 정보 세팅
VOID SearchScreenControlInfo(PINIVALUE pIniValue)
{
	CString strLog;
	CNHReadiniFile	Readini;
	PINIVALUE	pControlIniValue;
	CString		strResPath;
	CString		strRootPath;
	CString		strFullFileName;
	BOOL bFindControl = FALSE;
	
	strRootPath.Format(_T("\\%d_%d"), g_sizeScreen.cx, g_sizeScreen.cy);
	
	strFullFileName = g_strMasterPath + strRootPath + _T("\\ScreenControl.dat");
	
	// open Screen Control File
	if (!Readini.Open(strFullFileName, CNHReadiniFile::modeRead, CNHReadiniFile::modeLittleUNICODE))
	{
		NHDEBUG(1, (_T("ScreenControl File Open Failed [%s]\n"), strFullFileName));
		strLog.Format(_T("ScreenControl File Open Failed [%s]"), strFullFileName);
		AfxMessageBox(strLog);
		return;
	}
	
	while (NULL != (pControlIniValue = Readini.ReadiniValue()))
	{
		if (pControlIniValue->Key == DES_CONTROL_ID)
		{
			if(TRUE == bFindControl)
			{
				bFindControl = FALSE;
				break;
			}
			
			if (pIniValue->Values[0] == pControlIniValue->Values[0])
			{
				bFindControl = TRUE;
			}
		}
		else if(TRUE == bFindControl)
		{

			if (pIniValue->Section == DES_TEXTBOX)
			{
				SetTextBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_EFFECTTEXTBOX)
			{
				SetEffectTextBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_EDITBOX)
			{
				SetEditBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_CHANGE_PW_BOX)
			{
				SetChangePWBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_BUTTON)
			{
				SetButtonBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_SW_KEY)
			{
				SetSWKeyBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_TOGGLE_BOX)
			{
				SetEnDisableBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_EJNL_BOX)
			{
				SetEJNLBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_DATE_BOX)
			{
				SetDateBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_IP_BOX)
			{
				SetIPBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_BIN_BOX)
			{
				SetBinBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_CENT_BOX)
			{
				SetCentBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_DOLLAR_BOX)
			{
				SetDollarBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_ADV_BOX)
			{
				SetAdvBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_ANI_BOX)
			{
				SetAniBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_TABLE_EDITBOX)
			{
				SetTableEditBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_VKEYBUTTON)
			{
				SetVKeyButtonBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_ALPHA_KEYBOX)
			{
				SetAlphaKeyBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_TABLE_KEYBOX)
			{
				SetTableKeyBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_THREE_STATEBOX)
			{
				SetThreeStateBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_BININDEX_BOX)
			{
				SetBinIndexBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_DECIMAL_BOX)
			{
				SetDecimalBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_CENTCLEARL_BOX)
			{
				SetCentClearBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_KEYTEST_BOX)
			{
				SetKeyTestBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_INPUTEDIT_BOX)
			{
				SetInputEditBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_THREETYPE_BOX)
			{
				SetThreeTypeBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_GIF_BOX)
			{
				SetGifBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_IMAGE_BOX)
			{
				SetImageBox(pControlIniValue);
			}
			else if (pIniValue->Section == DES_EDITBUTTON_BOX)
			{
				SetEditButtonBox(pControlIniValue);
			}
		}
	}
	
	Readini.Close();
}