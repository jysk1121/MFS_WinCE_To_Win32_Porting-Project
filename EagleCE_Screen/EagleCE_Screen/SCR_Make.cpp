#include "stdafx.h"
#include "ScreenDisplay.h"

#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

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
CNBinIndexBox   *g_pNBinIndexBox = NULL;
CNDecimalBox	*g_pNDecimalBox = NULL;
CNCentClearBox	*g_pNCentClearBox = NULL;
CNLocaleText	*g_pNLocaleText = NULL;
CNKeyTestBox	*g_pNKeyTestBox = NULL;
CNInputEditBox	*g_pNInputEditBox = NULL;
CNThreeTypeBox	*g_pNThreeTypeBox = NULL;
CNGifBox		*g_pNGifBox = NULL;			// V1.0.2.4 2018.06.26
CNImageBox		*g_pNImageBox = NULL;
CNEditButtonBox	*g_pNEditButtonBox = NULL;


///////////////////////////////////////////////////////////////////
//	Function prototype define
///////////////////////////////////////////////////////////////////
BOOL	LoadMasterScreenValue(LPCTSTR FileName, CString strLoadSection);
BOOL	LoadScreenValue(LPCTSTR FileName, CReadiniFile::OpModeFlags Type);
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
BOOL	CreateBinIndexBox(PINIVALUE	pIniValue);		// BIN INDEX BOX
BOOL	CreateDecimalBox(PINIVALUE	pIniValue);		// Add Decimal Box
BOOL	CreateCentClearBox(PINIVALUE pIniValue);	// Add CentClearBox
BOOL	CreateKeyTestBox(PINIVALUE pIniValue);		// @ADD: 2017.11.17 LEH
BOOL	CreateInputEditBox(PINIVALUE pIniValue);	// @ADD: 2017.11.21 LEH
BOOL	CreateThreeTypeBox(PINIVALUE pIniValue);	// @ADD: 2017.11.22 LEH
BOOL	CreateGifBox(PINIVALUE	pIniValue);		// V1.0.2.4 2018.06.26
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

BOOL	SettingControl(CNControl *pControl, PINIVALUE	pIniValue);
BOOL	SettingShape(CNShape *pShape, PINIVALUE pIniValue);
BOOL	SettingText(CNTextBox *pTextBox, PINIVALUE	pIniValue);
BOOL	SettingVKey(CNVirtualKeyBox *pVKeyBox, PINIVALUE pIniValue);
BOOL	SettingControlEx(CNControl *pControl, PINIVALUE	pIniValue);

// [PCI-SSF Fix] 예외 처리 가드 - 화면 스크립트(.dat) 파싱 진입점을 SEH로 감싼다.
// __try/__except는 C++ 객체(소멸자)가 있는 함수 안에서 쓸 수 없으므로(C2712),
// 실제 파싱(CString/CReadiniFile 등 사용)은 트램폴린 함수로 분리하고, SEH는
// POD 인자만 받는 별도 wrapper에서만 감싼다.
struct LoadScreenValueCtx
{
	LPCTSTR						pFileName;
	CReadiniFile::OpModeFlags	type;
};

static void TrampolineLoadScreenValue(LPVOID pArg)
{
	LoadScreenValueCtx* pCtx = (LoadScreenValueCtx*)pArg;
	LoadScreenValue(pCtx->pFileName, pCtx->type);
}

static void SafeLoadScreenValue(LPCTSTR pFileName, CReadiniFile::OpModeFlags type)
{
	LoadScreenValueCtx ctx;
	ctx.pFileName = pFileName;
	ctx.type = type;

	__try
	{
		TrampolineLoadScreenValue(&ctx);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// LOG?? CString ????? ?????? ??????? ?????? ???Ŀ? ????? ???? ???.
		LOG(Error, _T("LoadScreenValue: unhandled exception (0x%08X) caught while parsing screen script - skipped"), GetExceptionCode());
	}
}

BOOL	NHScreenMake(CString strLoadSection, CString strMasterPath)
{
	CString strMasterFile;

	//strMasterFile = strMasterPath + MASTER_SCREEN_FILE;

	// Master File Parsing
	if (!LoadMasterScreenValue(strMasterFile, strLoadSection))
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
	HANDLE hSearch = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA	FileData;

	strClientPath.Format(_T("\\%d_%d\\Client"), g_sizeScreen.cx, g_sizeScreen.cy);
	strRootPath.Format(_T("\\%d_%d"), g_sizeScreen.cx, g_sizeScreen.cy);
	
	///////////////////////////////
	// 고객이 수정 불가능
	LoadScreenText(strRootPath + _T("\\ReceiptPrintText.dat"));
	LoadScreenText(strRootPath + _T("\\HostErrorText.dat"));

	LoadScreenText(strClientPath + _T("\\Client_ScreenText.dat"));
	SafeLoadScreenValue(strClientPath + _T("\\Client_Resource.dat"), CReadiniFile::modeASCII);
	SafeLoadScreenValue(strClientPath + _T("\\Client_Common.dat"), CReadiniFile::modeASCII);

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

			SafeLoadScreenValue(strClientPath + _T("\\") + strSrcFile, CReadiniFile::modeASCII);

			if (FindNextFile(hSearch, &FileData) == FALSE)
				break;
		}

		FindClose(hSearch);
	}
	else
	{
		LOG(Error, _T("Script File is not found."));
	}

	SafeLoadScreenValue(strRootPath + _T("\\Client_AdvScreen.dat"), CReadiniFile::modeASCII);

	SafeLoadScreenValue(strRootPath + _T("\\Supervisor\\Supervisor.dat"), CReadiniFile::modeASCII);

	SafeLoadScreenValue(_T("\\Update.dat"), CReadiniFile::modeASCII);

	return TRUE;
}

BOOL	LoadScreenValue(LPCTSTR FileName, CReadiniFile::OpModeFlags Type)
{
	CReadiniFile	Readini;
	PINIVALUE	pIniValue;
	CString		strResPath;
	CString		strFullFileName = g_strMasterPath + FileName;

	// open Master Screen File
	if (!Readini.Open(strFullFileName, CReadiniFile::modeRead, Type))
	{
		LOG(Error, _T("File Open Failed (%s) in Screen File"), strFullFileName);
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
		// [KEYTESTBOX]
		else if (pIniValue->Section == DES_KEYTEST_BOX)
		{
			if (!CreateKeyTestBox(pIniValue))
				goto ErrorReturn;
		}
		// [INPUTEDITBOX]
		else if (pIniValue->Section == DES_INPUTEDIT_BOX)
		{
			if (!CreateInputEditBox(pIniValue))
				goto ErrorReturn;
		}
		// [THREETYPEBOX]
		else if (pIniValue->Section == DES_THREETYPE_BOX)
		{
			if (!CreateThreeTypeBox(pIniValue))
				goto ErrorReturn;
		}
		// V1.0.2.4 2018.06.26 - [GIFBOX] Section
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
				int fontsize = CUtil::StringToInt(pIniValue->Values[2]);

				// Font는 배율 미적용 (test 필요) 
				// Adjust Scale
				{
					int currentX = g_sizeScreen.cx;
					int currentY = g_sizeScreen.cy;

					// V1.0.2.4 2018.07.17 - 비율을 1로 변경함.
// 					float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);
// 					float ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);
					float ScreenRateX = (float)(1);
					float ScreenRateY = (float)(1);
					//////////////////////////////////////////////////////////////////////////

					if (ScreenRateX > ScreenRateY)
						fontsize = (int)((float)ScreenRateX * fontsize + 0.5);
					else
						fontsize = (int)((float)ScreenRateY * fontsize + 0.5);
				}

				// SIZE
				lf.lfHeight = GET_FONTSIZE(fontsize);

				// 해상도에 따른 높이 조정
				if (EAGLE_SCREEN_CX == CS130_WIDTH_RESOLUTION)
					lf.lfWidth = (LONG)(lf.lfHeight * (2/3));
				else
					lf.lfWidth = (LONG)(lf.lfHeight * (0.5));
				

				if (lf.lfWidth < 0)
					lf.lfWidth *= -1;

				// set quality
#ifdef UNDER_CE
				lf.lfQuality = CLEARTYPE_COMPAT_QUALITY;
#else
				lf.lfQuality = DEFAULT_QUALITY;
#endif

				// BOLD
				if (CUtil::StringToInt(pIniValue->Values[3]))
#ifdef UNDER_CE
					lf.lfWeight = FW_BOLD;
#else
					lf.lfWeight = FW_SEMIBOLD;
#endif
			}
			else
			{
				// Name
				_tcsncpy(lf.lfFaceName, pIniValue->Values[1], LF_FACESIZE-1);
				
				// SIZE
				int fontsize = CUtil::StringToInt(pIniValue->Values[2]);

				// Font는 배율 미적용 (test 필요)
				// Adjust Scale
				{
					int currentX = g_sizeScreen.cx;
					int currentY = g_sizeScreen.cy;

					// V1.0.2.4 2018.07.17 - 비율을 1로 변경함.
// 					float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);
// 					float ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);
					float ScreenRateX = (float)(1);
					float ScreenRateY = (float)(1);
					//////////////////////////////////////////////////////////////////////////

					if (ScreenRateX > ScreenRateY)
						fontsize = (int)((float)ScreenRateX * fontsize + 0.5);
					else
						fontsize = (int)((float)ScreenRateY * fontsize + 0.5);
				}

				lf.lfHeight = GET_FONTSIZE(fontsize);
				// set quality
#ifdef UNDER_CE
				lf.lfQuality = CLEARTYPE_QUALITY;
				//lf.lfWidth = (LONG)(lf.lfHeight * 0.43);
				lf.lfWidth = (LONG)(lf.lfHeight * 0.44 );
#else
//				lf.lfHeight -= 1;
				lf.lfQuality = DEFAULT_QUALITY;
				lf.lfWidth = (LONG)(lf.lfHeight * 0.43);
#endif

				if (lf.lfWidth < 0)
					lf.lfWidth *= -1;
				
				// BOLD
				if (CUtil::StringToInt(pIniValue->Values[3]))
#ifdef UNDER_CE
					lf.lfWeight = FW_BOLD;
#else
					lf.lfWeight = FW_SEMIBOLD;
#endif
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
			Width = CUtil::StringToInt(pIniValue->Values[1]);

			// RGB
			crColor = RGB(CUtil::StringToInt(pIniValue->Values[2]), CUtil::StringToInt(pIniValue->Values[3]),
							CUtil::StringToInt(pIniValue->Values[4]));

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
											 CUtil::StringToInt(pIniValue->Values[1]),
											 CUtil::StringToInt(pIniValue->Values[2]), 
											 CUtil::StringToInt(pIniValue->Values[3]));
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
			crColor = RGB(CUtil::StringToInt(pIniValue->Values[1]), CUtil::StringToInt(pIniValue->Values[2]), CUtil::StringToInt(pIniValue->Values[3]));

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
}

BOOL	CreateScreen(PINIVALUE	pIniValue)
{
	// ID =?
	if (pIniValue->Key == DES_ID)
	{
		// check valide.
		if (g_pNScreen)
		{
			LOG(Error, _T("CNScreen don't Deallocate in SCREEN(%s)"), g_pNScreen->GetID());
			goto ErrorReturn;
		}

		// allocate.
		g_pNScreen = (CNScreen*) new CNScreen;
		if (!g_pNScreen)
		{
			LOG(Error, _T("[SCR_IN] CNScreen allocate Failed in SCREEN"));
			goto ErrorReturn;
		}

		// Register.
		pIniValue->Values[0].MakeUpper();
		g_pNScreen->SetID(pIniValue->Values[0]);

		g_NHScreens.PutOnTailOfList(g_pNScreen->GetID(), g_pNScreen);

		//g_strScreenID = pIniValue->Values[0];	// V1.0.2.4 2018.07.19 - Camera의 좌표를 Script의 Pos 값으로 설정하도록 하기 위함.
	}
	// Back=ID
	else if (pIniValue->Key == DES_BACK)
	{
		// check valide.
		if (!g_pNScreen)
		{
			LOG(Error, _T("CNScreen don't allocate in SCREEN"));
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
		if(!(CUtil::StringToInt(pIniValue->Values[0])))
		{
			if (!g_pNScreen)
			{
				LOG(Error, _T("CNScreen don't allocate in SCREEN"));
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
		LOG(Error, _T("CNTextBox need to SCREEN. TEXTBOX"));
		goto ErrorReturn;
	}

	// [TEXTBOX][/TEXTBOX]
	if (pIniValue->Key == DES_TEXTBOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNTextBox)
			{
				LOG(Error, _T("CNTextBox don't Deallocate in TEXTBOX"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNTextBox = (CNTextBox*) new CNTextBox;
			if (!g_pNTextBox)
			{
				LOG(Error, _T("CNTextBox allocate Failed in TEXTBOX"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNTextBox);
		}
		else
		{
			if (!g_pNTextBox)
			{
				LOG(Error, _T("CNTextBox allocate Failed in TEXTBOX"));
				goto ErrorReturn;
			}
			
			g_pNTextBox = NULL;
		}
	}
	else
	{
		if (!g_pNTextBox)
		{
			LOG(Error, _T("CNTextBox allocate Failed in TEXTBOX"));
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
		LOG(Error, _T("CreateEffectTextBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [TEXTBOX][/TEXTBOX]
	if (pIniValue->Key == DES_EFFECTTEXTBOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNEffectTextBox)
			{
				LOG(Error, _T("CreateEffectTextBox allocated failed"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNEffectTextBox = (CNEffectTextBox*) new CNEffectTextBox;
			if (!g_pNEffectTextBox)
			{
				LOG(Error, _T("CreateEffectTextBox allocated failed"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNEffectTextBox);
		}
		else
		{
			if (!g_pNEffectTextBox)
			{
				LOG(Error, _T("CreateEffectTextBox allocated failed"));
				goto ErrorReturn;
			}
			
			g_pNEffectTextBox = NULL;
		}
	}
	else
	{
		if (!g_pNEffectTextBox)
		{
			LOG(Error, _T("CreateEffectTextBox allocated failed"));
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
		LOG(Error, _T("CreateEditBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [EDITBOX][/EDITBOX]
	if (pIniValue->Key == DES_EDITBOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNEditTextBox)
			{
				LOG(Error, _T("CreateEditBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNEditTextBox = (CNEditTextBox*) new CNEditTextBox;
			if (!g_pNEditTextBox)
			{
				LOG(Error, _T("CreateEditBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNEditTextBox);
		}
		else
		{
			if (!g_pNEditTextBox)
			{
				LOG(Error, _T("CreateEditBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNEditTextBox = NULL;
		}
	}
	else
	{
		if (!g_pNEditTextBox)
		{
			LOG(Error, _T("CreateEditBox allocate is failed to SCREEN"));
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
		g_pNEditTextBox->SetMaxInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
	else if(pIniValue->Key == DES_MINCHAR)
	{
		g_pNEditTextBox->SetMinInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
	else if (pIniValue->Key == DES_TYPE)
	{
		if (pIniValue->Values[0] == DES_PASSWORD)
			g_pNEditTextBox->SetTextType(TTYPE_PASSWORD);
		else if (pIniValue->Values[0] == DES_IPADDRESS)	
			g_pNEditTextBox->SetTextType(TTYPE_IPADDRESS);
	}
	else if (pIniValue->Key == DES_AUTO_RUN)
	{
		g_pNEditTextBox->SetAutoRun(CUtil::StringToInt(pIniValue->Values[0]));
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
		LOG(Error, _T("CreateChangePWBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [EDITBOX][/EDITBOX]
	if (pIniValue->Key == DES_CHANGE_PW_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNChangePWBox)
			{
				LOG(Error, _T("CreateChangePWBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNChangePWBox = (CNChangePWBox*) new CNChangePWBox;
			if (!g_pNChangePWBox)
			{
				LOG(Error, _T("CreateChangePWBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNChangePWBox);
		}
		else
		{
			if (!g_pNChangePWBox)
			{
				LOG(Error, _T("CreateChangePWBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNChangePWBox = NULL;
		}
	}
	else
	{
		if (!g_pNChangePWBox)
		{
			LOG(Error, _T("CreateChangePWBox allocate is failed to SCREEN"));
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
		g_pNChangePWBox->SetMaxInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
	else if(pIniValue->Key == DES_MINCHAR)
	{
		g_pNChangePWBox->SetMinInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
	else if (pIniValue->Key == DES_POS_CUR_BOX && pIniValue->nValue == 4)
	{
		g_pNChangePWBox->SetPWBoxRect(CPTYPE_CUR,
			CUtil::StringToInt(pIniValue->Values[0]), CUtil::StringToInt(pIniValue->Values[1]),
			CUtil::StringToInt(pIniValue->Values[2]), CUtil::StringToInt(pIniValue->Values[3]));
	}
	else if (pIniValue->Key == DES_POS_NEW_BOX && pIniValue->nValue == 4)
	{
		g_pNChangePWBox->SetPWBoxRect(CPTYPE_NEW,
			CUtil::StringToInt(pIniValue->Values[0]), CUtil::StringToInt(pIniValue->Values[1]),
			CUtil::StringToInt(pIniValue->Values[2]), CUtil::StringToInt(pIniValue->Values[3]));
	}
	else if (pIniValue->Key == DES_POS_NEW_RE_BOX && pIniValue->nValue == 4)
	{
		g_pNChangePWBox->SetPWBoxRect(CPTYPE_NEWRE,
			CUtil::StringToInt(pIniValue->Values[0]), CUtil::StringToInt(pIniValue->Values[1]),
			CUtil::StringToInt(pIniValue->Values[2]), CUtil::StringToInt(pIniValue->Values[3]));
	}
	else if (pIniValue->Key == DES_AUTO_RUN)
	{
		g_pNChangePWBox->SetAutoRun(CUtil::StringToInt(pIniValue->Values[0]));
	}
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
		LOG(Error, _T("CreateButtonBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [BUTTON][/BUTTON]
	if (pIniValue->Key == DES_BUTTON)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNButtonBox)
			{
				LOG(Error, _T("CreateButtonBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNButtonBox = (CNButtonBox*) new CNButtonBox;
			if (!g_pNButtonBox)
			{
				LOG(Error, _T("CreateButtonBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNButtonBox);
		}
		else
		{
			if (!g_pNButtonBox)
			{
				LOG(Error, _T("CreateButtonBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNButtonBox = NULL;
		}
	}
	else
	{
		if (!g_pNButtonBox)
		{
			LOG(Error, _T("CreateButtonBox allocate is failed to SCREEN"));
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
		LOG(Error, _T("CreateVKeyButtonBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [VKEYBUTTON][/VKEYBUTTON]
	if (pIniValue->Key == DES_VKEYBUTTON)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNVKeyButtonBox)
			{
				LOG(Error, _T("CreateVKeyButtonBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNVKeyButtonBox = (CNVKeyButtonBox*) new CNVKeyButtonBox;
			if (!g_pNVKeyButtonBox)
			{
				LOG(Error, _T("CreateVKeyButtonBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNVKeyButtonBox);
		}
		else
		{
			if (!g_pNVKeyButtonBox)
			{
				LOG(Error, _T("CreateVKeyButtonBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNVKeyButtonBox = NULL;
		}
	}
	else
	{
		if (!g_pNVKeyButtonBox)
		{
			LOG(Error, _T("CreateVKeyButtonBox allocate is failed to SCREEN"));
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
		LOG(Error, _T("CreateSWKeyBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [SWKEY][/SWKEY]
	if (pIniValue->Key == DES_SW_KEY)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNSWKeyBox)
			{
				LOG(Error, _T("CreateSWKeyBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNSWKeyBox = (CNSWKeyBox*) new CNSWKeyBox;
			if (!g_pNSWKeyBox)
			{
				LOG(Error, _T("CreateSWKeyBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNSWKeyBox);
		}
		else
		{
			if (!g_pNSWKeyBox)
			{
				LOG(Error, _T("CreateSWKeyBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNSWKeyBox = NULL;
		}
	}
	else
	{
		if (!g_pNSWKeyBox)
		{
			LOG(Error, _T("CreateSWKeyBox allocate is failed to SCREEN"));
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
			szTemp.cx = CUtil::StringToInt(pIniValue->Values[0]);
			szTemp.cy = CUtil::StringToInt(pIniValue->Values[1]);

			// Adjust Scale
			{
				int currentX = g_sizeScreen.cx;
				int currentY = g_sizeScreen.cy;

				// V1.0.2.4 2018.07.17 - 비율을 1로 변경함.
// 				float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);
// 				float ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);
				float ScreenRateX = (float)(1);
				float ScreenRateY = (float)(1);
				//////////////////////////////////////////////////////////////////////////

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
			szTemp.cx = CUtil::StringToInt(pIniValue->Values[0]);
			szTemp.cy = CUtil::StringToInt(pIniValue->Values[1]);

			// Adjust Scale
			{
				int currentX = g_sizeScreen.cx;
				int currentY = g_sizeScreen.cy;

				// V1.0.2.4 2018.07.17 - 비율을 1로 변경함.
// 				float ScreenRateX = (float)((float)currentX / (float)BASIC_SCREEN_X);
// 				float ScreenRateY = (float)((float)currentY / (float)BASIC_SCREEN_Y);
				float ScreenRateX = (float)(1);
				float ScreenRateY = (float)(1);
				//////////////////////////////////////////////////////////////////////////

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
			szTemp.cx = CUtil::StringToInt(pIniValue->Values[0]);
			szTemp.cy = CUtil::StringToInt(pIniValue->Values[1]);

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
		LOG(Error, _T("CreateEnDisableBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [ENABLEBOX][/ENABLEBOX]
	if (pIniValue->Key == DES_TOGGLE_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNSWKeyBox)
			{
				LOG(Error, _T("CreateEnDisableBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNToggleBox = (CNToggleBox*) new CNToggleBox;
			if (!g_pNToggleBox)
			{
				LOG(Error, _T("CreateEnDisableBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNToggleBox);
		}
		else
		{
			if (!g_pNToggleBox)
			{
				LOG(Error, _T("CreateEnDisableBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNToggleBox = NULL;
		}
	}
	else
	{
		if (!g_pNToggleBox)
		{
			LOG(Error, _T("CreateEnDisableBox allocate is failed to SCREEN"));
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
		LOG(Error, _T("CreateEJNLBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [EJNLBOX][/EJNLBOX]
	if (pIniValue->Key == DES_EJNL_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNEJNLBox)
			{
				LOG(Error, _T("CreateEJNLBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNEJNLBox = (CNEJNLBox*) new CNEJNLBox;
			if (!g_pNEJNLBox)
			{
				LOG(Error, _T("CreateEJNLBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNEJNLBox);
		}
		else
		{
			if (!g_pNEJNLBox)
			{
				LOG(Error, _T("CreateEJNLBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNEJNLBox = NULL;
		}
	}
	else
	{
		if (!g_pNEJNLBox)
		{
			LOG(Error, _T("CreateEJNLBox allocate is failed to SCREEN"));
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
		LOG(Error, _T("CreateDateBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [DATEBOX][/DATEBOX]
	if (pIniValue->Key == DES_DATE_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNDateBox)
			{
				LOG(Error, _T("CreateDateBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNDateBox = (CNDateBox*) new CNDateBox;
			if (!g_pNDateBox)
			{
				LOG(Error, _T("CreateDateBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNDateBox);
		}
		else
		{
			if (!g_pNDateBox)
			{
				LOG(Error, _T("CreateDateBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNDateBox = NULL;
		}
	}
	else
	{
		if (!g_pNDateBox)
		{
			LOG(Error, _T("CreateDateBox allocate is failed to SCREEN"));
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
		LOG(Error, _T("CreateIPBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [IPBOX][/IPBOX]
	if (pIniValue->Key == DES_IP_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNIPBox)
			{
				LOG(Error, _T("CreateIPBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNIPBox = (CNIPBox*) new CNIPBox;
			if (!g_pNIPBox)
			{
				LOG(Error, _T("CreateIPBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNIPBox);
		}
		else
		{
			if (!g_pNIPBox)
			{
				LOG(Error, _T("CreateIPBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNIPBox = NULL;
		}
	}
	else
	{
		if (!g_pNIPBox)
		{
			LOG(Error, _T("CreateIPBox allocate is failed to SCREEN"));
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
		LOG(Error, _T("CreateBinBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [BINBOX][/BINBOX]
	if (pIniValue->Key == DES_BIN_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNBinBox)
			{
				LOG(Error, _T("CreateBinBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNBinBox = (CNBinBox*) new CNBinBox;
			if (!g_pNBinBox)
			{
				LOG(Error, _T("CreateBinBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNBinBox);
		}
		else
		{
			if (!g_pNBinBox)
			{
				LOG(Error, _T("CreateBinBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNBinBox = NULL;
		}
	}
	else
	{
		if (!g_pNBinBox)
		{
			LOG(Error, _T("CreateBinBox allocate is failed to SCREEN"));
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
			CUtil::StringToInt(pIniValue->Values[0]), CUtil::StringToInt(pIniValue->Values[1]),
			CUtil::StringToInt(pIniValue->Values[2]), CUtil::StringToInt(pIniValue->Values[3]));
	}
	else if (pIniValue->Key == DES_POS_BIN && pIniValue->nValue == 4)
	{
		g_pNBinBox->SetBinBoxRect(BNTYPE_BIN,
			CUtil::StringToInt(pIniValue->Values[0]), CUtil::StringToInt(pIniValue->Values[1]),
			CUtil::StringToInt(pIniValue->Values[2]), CUtil::StringToInt(pIniValue->Values[3]));
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
		LOG(Error, _T("CreateCentBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [CENTBOX][/CENTBOX]
	if (pIniValue->Key == DES_CENT_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNCentBox)
			{
				LOG(Error, _T("CreateCentBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNCentBox = (CNCentBox*) new CNCentBox;
			if (!g_pNCentBox)
			{
				LOG(Error, _T("CreateCentBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNCentBox);
		}
		else
		{
			if (!g_pNCentBox)
			{
				LOG(Error, _T("CreateCentBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNCentBox = NULL;
		}
	}
	else
	{
		if (!g_pNCentBox)
		{
			LOG(Error, _T("CreateCentBox allocate is failed to SCREEN"));
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
		g_pNCentBox->SetMaxInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
	else if(pIniValue->Key == DES_MINCHAR)
	{
		g_pNCentBox->SetMinInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
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
		LOG(Error, _T("CreateDollarBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [DOLLARBOX][/DOLLARBOX]
	if (pIniValue->Key == DES_DOLLAR_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNDollarBox)
			{
				LOG(Error, _T("CreateDollarBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNDollarBox = (CNDollarBox*) new CNDollarBox;
			if (!g_pNDollarBox)
			{
				LOG(Error, _T("CreateDollarBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNDollarBox);
		}
		else
		{
			if (!g_pNDollarBox)
			{
				LOG(Error, _T("CreateDollarBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNDollarBox = NULL;
		}
	}
	else
	{
		if (!g_pNDollarBox)
		{
			LOG(Error, _T("CreateDollarBox allocate is failed to SCREEN"));
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
		g_pNDollarBox->SetMaxInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
	else if (pIniValue->Key == DES_MINCHAR)
	{
		g_pNDollarBox->SetMinInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
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
		LOG(Error, _T("CreateAdvBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [ADVBOX][/ADVBOX]
	if (pIniValue->Key == DES_ADV_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNAdvBox)
			{
				LOG(Error, _T("CreateAdvBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNAdvBox = (CNAdvBox*) new CNAdvBox;
			if (!g_pNAdvBox)
			{
				LOG(Error, _T("CreateAdvBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNAdvBox);
		}
		else
		{
			if (!g_pNAdvBox)
			{
				LOG(Error, _T("CreateAdvBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNAdvBox = NULL;
		}
	}
	else
	{
		if (!g_pNAdvBox)
		{
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
	CString	strFile;
	strFile.Format(_T("%s_%d_%d"), DES_FILE, g_sizeScreen.cx, g_sizeScreen.cy);

	if (pIniValue->Key == strFile)
	{
		g_pNAdvBox->SetFileName(pIniValue->Values[0]);
	}
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
		LOG(Error, _T("CreateAniBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [ANIBOX][/ANIBOX]
	if (pIniValue->Key == DES_ANI_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNAniBox)
			{
				LOG(Error, _T("CreateAniBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNAniBox = (CNAnimation*) new CNAnimation;
			if (!g_pNAniBox)
			{
				LOG(Error, _T("CreateAniBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNAniBox);
		}
		else
		{
			if (!g_pNAniBox)
			{
				LOG(Error, _T("CreateAniBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNAniBox = NULL;
		}
	}
	else
	{

		if (!g_pNAniBox)
		{
			LOG(Error, _T("CreateAniBox allocate is failed to SCREEN"));
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
		g_pNAniBox->SetAniImage(CUtil::StringToInt(pIniValue->Values[0]), strFilePrefix);
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
		LOG(Error, _T("CreateTableEditBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [TABLEEDITBOX][/TABLEEDITBOX]
	if (pIniValue->Key == DES_TABLE_EDITBOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNTableEditBox)
			{
				LOG(Error, _T("CreateTableEditBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNTableEditBox = (CNTableEditTextBox*) new CNTableEditTextBox;
			if (!g_pNTableEditBox)
			{
				LOG(Error, _T("CreateTableEditBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNTableEditBox);
		}
		else
		{
			if (!g_pNTableEditBox)
			{
				LOG(Error, _T("CreateTableEditBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNTableEditBox = NULL;
		}
	}
	else
	{

		if (!g_pNTableEditBox)
		{
			LOG(Error, _T("CreateTableEditBox allocate is failed to SCREEN"));
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
			g_pNTableEditBox->SetAllowAmount(CUtil::StringToInt(pIniValue->Values[0]), CUtil::StringToInt(pIniValue->Values[1]));
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
		LOG(Error, _T("CreateAlphaKeyBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [ALPHAKEYBOX][/ALPHAKEYBOX]
	if (pIniValue->Key == DES_ALPHA_KEYBOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNAlphaKeyBox)
			{
				LOG(Error, _T("CreateAlphaKeyBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNAlphaKeyBox = (CNAlphaKeyBox*) new CNAlphaKeyBox;
			if (!g_pNAlphaKeyBox)
			{
				LOG(Error, _T("CreateAlphaKeyBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNAlphaKeyBox);
		}
		else
		{
			if (!g_pNAlphaKeyBox)
			{
				LOG(Error, _T("CreateAlphaKeyBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNAlphaKeyBox = NULL;
		}
	}
	else
	{
		if (!g_pNAlphaKeyBox)
		{
			LOG(Error, _T("CreateAlphaKeyBox allocate is failed to SCREEN"));
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
		LOG(Error, _T("CreateTableKeyBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [TABLEKEYBOX][/TABLEKEYBOX]
	if (pIniValue->Key == DES_TABLE_KEYBOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNTableKeyBox)
			{
				LOG(Error, _T("CreateTableKeyBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNTableKeyBox = (CNTableKeyBox*) new CNTableKeyBox;
			if (!g_pNTableKeyBox)
			{
				LOG(Error, _T("CreateTableKeyBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNTableKeyBox);
		}
		else
		{
			if (!g_pNTableKeyBox)
			{
				LOG(Error, _T("CreateTableKeyBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNTableKeyBox = NULL;
		}
	}
	else
	{
		if (!g_pNTableKeyBox)
		{
			LOG(Error, _T("CreateTableKeyBox allocate is failed to SCREEN"));
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
		LOG(Error, _T("CreateThreeStateBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [THREESTATEBOX][/THREESTATEBOX]
	if (pIniValue->Key == DES_THREE_STATEBOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNThreeStateBox)
			{
				LOG(Error, _T("CreateThreeStateBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNThreeStateBox = (CNThreeStateBox*) new CNThreeStateBox;
			if (!g_pNThreeStateBox)
			{
				LOG(Error, _T("CreateThreeStateBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNThreeStateBox);
		}
		else
		{
			if (!g_pNThreeStateBox)
			{
				LOG(Error, _T("CreateThreeStateBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNThreeStateBox = NULL;
		}
	}
	else
	{
// 		CNResource *pHide = NULL, *pOn = NULL, *pOff = NULL;

		if (!g_pNThreeStateBox)
		{
			LOG(Error, _T("CreateThreeStateBox allocate is failed to SCREEN"));
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

BOOL	CreateBinIndexBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		LOG(Error, _T("CreateBinIndexBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [BININDEXBOX][/BININDEXBOX]
	if (pIniValue->Key == DES_BININDEX_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNBinIndexBox)
			{
				LOG(Error, _T("CreateBinIndexBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNBinIndexBox = (CNBinIndexBox *) new CNBinIndexBox;
			if (!g_pNBinIndexBox)
			{
				LOG(Error, _T("CreateBinIndexBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNBinIndexBox);
		}
		else
		{
			if (!g_pNBinIndexBox)
			{
				LOG(Error, _T("CreateBinIndexBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNBinIndexBox = NULL;
		}
	}
	else
	{
		if (!g_pNBinIndexBox)
		{
			LOG(Error, _T("CreateBinIndexBox allocate is failed to SCREEN"));
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

BOOL	CreateDecimalBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		LOG(Error, _T("CreateDecimalBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [DECIMALBOX][/DECIMALBOX]
	if (pIniValue->Key == DES_DECIMAL_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNDecimalBox)
			{
				LOG(Error, _T("CreateDecimalBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNDecimalBox = (CNDecimalBox*) new CNDecimalBox;
			if (!g_pNDecimalBox)
			{
				LOG(Error, _T("CreateDecimalBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNDecimalBox);
		}
		else
		{
			if (!g_pNDecimalBox)
			{
				LOG(Error, _T("CreateDecimalBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}			
			g_pNDecimalBox = NULL;
		}
	}
	else
	{
		if (!g_pNDecimalBox)
		{
			LOG(Error, _T("CreateDecimalBox allocate is failed to SCREEN"));
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

// V1.0.2.4 2018.07.12
VOID	SetDecimalBox(PINIVALUE pIniValue)
{
	if (pIniValue->Key == DES_MAXINCHAR)
	{
		g_pNDecimalBox->SetMaxInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
	else if(pIniValue->Key == DES_MINCHAR)
	{
		g_pNDecimalBox->SetMinInputChar(CUtil::StringToInt(pIniValue->Values[0]));
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

BOOL	CreateCentClearBox(PINIVALUE	pIniValue)
{
	if (!g_pNScreen)
	{
		LOG(Error, _T("CreateCentClearBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [CENTCLEARBOX][/CENTCLEARBOX]
	if (pIniValue->Key == DES_CENTCLEARL_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNCentClearBox)
			{
				LOG(Error, _T("CreateCentClearBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNCentClearBox = (CNCentClearBox*) new CNCentClearBox;
			if (!g_pNCentClearBox)
			{
				LOG(Error, _T("CreateCentClearBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNCentClearBox);
		}
		else
		{
			if (!g_pNCentClearBox)
			{
				LOG(Error, _T("CreateCentClearBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}
			
			g_pNCentClearBox = NULL;
		}
	}
	else
	{
		if (!g_pNCentClearBox)
		{
			LOG(Error, _T("CreateCentClearBox allocate is failed to SCREEN"));
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
		g_pNCentClearBox->SetMaxInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
	else if(pIniValue->Key == DES_MINCHAR)
	{
		g_pNCentClearBox->SetMinInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
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
		LOG(Error, _T("CNKeyTestBox need to SCREEN. KEYTESTBOX"));
		goto ErrorReturn;
	}

	// [KEYTESTBOX][/KEYTESTBOX]
	if (pIniValue->Key == DES_KEYTEST_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNKeyTestBox)
			{
				LOG(Error, _T("CNKeyTestBox don't Deallocate in KEYTESTBOX"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNKeyTestBox = (CNKeyTestBox*) new CNKeyTestBox;
			if (!g_pNKeyTestBox)
			{
				LOG(Error, _T("g_pNKeyTestBox allocate Failed in KEYTESTBOX"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNKeyTestBox);
		}
		else
		{
			if (!g_pNKeyTestBox)
			{
				LOG(Error, _T("CNKeyTestBox don't allocate in KEYTESTBOX"));
				goto ErrorReturn;
			}
			
			g_pNKeyTestBox = NULL;
		}
	}
	else
	{
		if (!g_pNKeyTestBox)
		{
			LOG(Error, _T("CNKeyTestBox don't allocate 2 in KEYTESTBOX"));
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
		g_pNKeyTestBox->SetMaxInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
	else if (pIniValue->Key == DES_MINCHAR)
	{
		g_pNKeyTestBox->SetMinInputChar(CUtil::StringToInt(pIniValue->Values[0]));
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
		LOG(Error, _T("CNInputEditBox need to SCREEN. INPUTEDITBOX"));
		goto ErrorReturn;
	}

	// [INPUTEDITBOX][/INPUTEDITBOX]
	if (pIniValue->Key == DES_INPUTEDIT_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNInputEditBox)
			{
				LOG(Error, _T("CNInputEditBox don't Deallocate in INPUTEDITBOX"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNInputEditBox = (CNInputEditBox*) new CNInputEditBox;
			if (!g_pNInputEditBox)
			{
				LOG(Error, _T("g_pNInputEditBox allocate Failed in INPUTEDITBOX"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNInputEditBox);
		}
		else
		{
			if (!g_pNInputEditBox)
			{
				LOG(Error, _T("CNInputEditBox don't allocate in INPUTEDITBOX"));
				goto ErrorReturn;
			}
			
			g_pNInputEditBox = NULL;
		}
	}
	else
	{
		if (!g_pNInputEditBox)
		{
			LOG(Error, _T("CNInputEditBox don't allocate 2 in INPUTEDITBOX"));
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
		g_pNInputEditBox->SetMaxInputChar(CUtil::StringToInt(pIniValue->Values[0]));
	}
	else if (pIniValue->Key == DES_MINCHAR)
	{
		g_pNInputEditBox->SetMinInputChar(CUtil::StringToInt(pIniValue->Values[0]));
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
		LOG(Error, _T("CNThreeTypeBox need to SCREEN. THREETYPEBOX"));
		goto ErrorReturn;
	}

	// [THREETYPEBOX][/THREETYPEBOX]
	if (pIniValue->Key == DES_THREETYPE_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNThreeTypeBox)
			{
				LOG(Error, _T("CNThreeTypeBox don't Deallocate in THREETYPEBOX"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNThreeTypeBox = (CNThreeTypeBox*) new CNThreeTypeBox;
			if (!g_pNThreeTypeBox)
			{
				LOG(Error, _T("CNThreeTypeBox allocate Failed in THREETYPEBOX"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNThreeTypeBox);
		}
		else
		{
			if (!g_pNThreeTypeBox)
			{
				LOG(Error, _T("CNThreeTypeBox don't allocate in THREETYPEBOX"));
				goto ErrorReturn;
			}
			
			g_pNThreeTypeBox = NULL;
		}
	}
	else
	{
// 		CNResource *pDisable = NULL, *pEnable = NULL, *pSelect = NULL;

		if (!g_pNThreeTypeBox)
		{
			LOG(Error, _T("CNThreeTypeBox don't allocate 2 in THREETYPEBOX"));
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
		LOG(Error, _T("CreateGifBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [GIFBOX][/GIFBOX]
	if (pIniValue->Key == DES_GIF_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNGifBox)
			{
				LOG(Error, _T("CreateGifBox don't deallocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNGifBox = (CNGifBox*) new CNGifBox;
			if (!g_pNGifBox)
			{
				LOG(Error, _T("CreateGifBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNGifBox);
		}
		else
		{
			if (!g_pNGifBox)
			{
				LOG(Error, _T("CreateGifBox don't allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			g_pNGifBox = NULL;
		}
	}
	else
	{
		if (!g_pNGifBox)
		{
			LOG(Error, _T("CreateGifBox don't allocate(2) is failed to SCREEN"));
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
		LOG(Error, _T("CreateImageBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [IMAGEBOX][/IMAGEBOX]
	if (pIniValue->Key == DES_IMAGE_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNImageBox)
			{
				LOG(Error, _T("CreateImageBox don't deallocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNImageBox = (CNImageBox*) new CNImageBox;
			if (!g_pNImageBox)
			{
				LOG(Error, _T("CreateImageBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNImageBox);
		}
		else
		{
			if (!g_pNImageBox)
			{
				LOG(Error, _T("CreateImageBox don't allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			g_pNImageBox = NULL;
		}
	}
	else
	{
		if (!g_pNImageBox)
		{
			LOG(Error, _T("CreateImageBox don't allocate(2) is failed to SCREEN"));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// CONTROL ID 추가
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
		LOG(Error, _T("CreateEditButtonBox need to SCREEN"));
		goto ErrorReturn;
	}

	// [EDITBUTTONBOX][/EDITBUTTONBOX]
	if (pIniValue->Key == DES_EDITBUTTON_BOX)
	{
		if (CUtil::StringToInt(pIniValue->Values[0]))
		{
			if (g_pNEditButtonBox)
			{
				LOG(Error, _T("CreateEditButtonBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// allocate.
			g_pNEditButtonBox = (CNEditButtonBox*) new CNEditButtonBox;
			if (!g_pNEditButtonBox)
			{
				LOG(Error, _T("CreateEditButtonBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			// Register.
			g_pNScreen->SetAddControl(g_pNEditButtonBox);
		}
		else
		{
			if (!g_pNEditButtonBox)
			{
				LOG(Error, _T("CreateEditButtonBox allocate is failed to SCREEN"));
				goto ErrorReturn;
			}

			g_pNEditButtonBox = NULL;
		}
	}
	else
	{
		if (!g_pNEditButtonBox)
		{
			LOG(Error, _T("CreateEditButtonBox allocate is failed to SCREEN"));
			goto ErrorReturn;
		}

		if (pIniValue->Key == DES_CONTROL_ID)	// V1.0.2.4 2018.07.12 - CONTROL ID 추가
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
		
		if (pIniValue->Values[0] == _T("APState502"))
		{
			g_bPosValue4Camera = TRUE;
		}
		////////////////////////////////////////////////////////////////////////////////
	}
	// AFocus=0,1
	else if (pIniValue->Key == DES_AFOCUS)
	{
		pControl->SetAutoFocus(CUtil::StringToInt(pIniValue->Values[0]));
	}
	// Pos=left,top,width,height
	else if (pIniValue->Key == DES_POS)
	{
		// check value count. it must be 4
		if (pIniValue->nValue == 4)
		{
			pControl->SetDisplayRect(CUtil::StringToInt(pIniValue->Values[0]), CUtil::StringToInt(pIniValue->Values[1]),
									CUtil::StringToInt(pIniValue->Values[2]), CUtil::StringToInt(pIniValue->Values[3]));
		}

		// V1.0.2.4 2018.07.19 - Camera의 좌표를 Script의 Pos 값으로 설정하도록 하기 위함.
		if (TRUE == g_bPosValue4Camera)
		{
			g_rect4Camera.left = CUtil::StringToInt(pIniValue->Values[0]);
			g_rect4Camera.top = CUtil::StringToInt(pIniValue->Values[1]);

			int nWidth, nHeight;
			nWidth = CUtil::StringToInt(pIniValue->Values[2]);

			g_rect4Camera.right = nWidth + g_rect4Camera.left;

			nHeight = CUtil::StringToInt(pIniValue->Values[3]);
			g_rect4Camera.bottom = nHeight + g_rect4Camera.top;

			g_bPosValue4Camera = FALSE;

		}
		////////////////////////////////////////////////
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
// 			pTextBox->SetMultiLine(CUtil::StringToInt(pIniValue->Values[0]));
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
			pTextBox->SetBlinkMode(CUtil::StringToInt(pIniValue->Values[0]));
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
		pControl->SetAutoFocus(CUtil::StringToInt(pIniValue->Values[0]));
	}
	// Pos=left,top,width,height
	else if (pIniValue->Key == DES_POS)
	{
		// check value count. it must be 4
		if (pIniValue->nValue == 4)
		{
			pControl->SetDisplayRect(CUtil::StringToInt(pIniValue->Values[0]), CUtil::StringToInt(pIniValue->Values[1]),
									CUtil::StringToInt(pIniValue->Values[2]), CUtil::StringToInt(pIniValue->Values[3]));
		}
	}

	return TRUE;
}

// 화면에 사용할 Text를 읽어 리소스로 저장한다.
BOOL	LoadScreenText(LPCTSTR FileName)
{
	CReadiniFile	Readini;
	PINIVALUE	pwIniValue;
	CString		strFullFileName = g_strMasterPath + FileName;
	CString		strSupportLanguage, strLoadCountry;
	CString		strTemp;

	// Get Country Short cut
	strLoadCountry = _T("US");

	// open Screen Text File
	if (!Readini.Open(strFullFileName, CReadiniFile::modeRead, CReadiniFile::modeLittleUNICODE))
	{
		LOG(Error, _T("LoadScreenText file read is failed"));
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
				g_pNLocaleText = (CNLocaleText*) new CNLocaleText(pwIniValue->Values[0], MFS_MAXLOCALE);
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
						g_pNLocaleText->AddLocaleText(1, pwIniValue->Values[0]);
					else if (strTemp == _T("SPN"))
						g_pNLocaleText->AddLocaleText(2, pwIniValue->Values[0]);
					else if (strTemp == _T("FRN"))
						g_pNLocaleText->AddLocaleText(3, pwIniValue->Values[0]);
					else if (strTemp == _T("CHN"))
						g_pNLocaleText->AddLocaleText(4, pwIniValue->Values[0]);
					else if (strTemp == _T("KOR"))
						g_pNLocaleText->AddLocaleText(5, pwIniValue->Values[0]);
					else if (strTemp == _T("JPN"))
						g_pNLocaleText->AddLocaleText(6, pwIniValue->Values[0]);
				}
			}
		}
	}

	return TRUE;

ErrorReturn:

	Readini.Close();

	return FALSE;
}

/** **********************************************
*	@brief		ScreenControl.dat 파일을 검색한 후, 해당 ControlID의 Key정보를 세팅한다.
*	@details	Resource 관련 정보(Font, Pos, Text, TextID, TextSet, Picture, Color) 이외의 정보 세팅
*	@param PINIVALUE pIniValue	각 속성값
***************************************************/ 
VOID SearchScreenControlInfo(PINIVALUE pIniValue)
{
	CReadiniFile	Readini;
	PINIVALUE	pControlIniValue;
	CString		strResPath;
	CString		strRootPath;
	CString		strFullFileName;
	BOOL bFindControl = FALSE;

	strRootPath.Format(_T("\\%d_%d"), g_sizeScreen.cx, g_sizeScreen.cy);

	strFullFileName = g_strMasterPath + strRootPath + _T("\\ScreenControl.dat");

	// open Screen Control File
	if (!Readini.Open(strFullFileName, CReadiniFile::modeRead, CReadiniFile::modeLittleUNICODE))
	{
		LOG(Error, _T("ScreenControl File Open Failed. FileName : [%s]"), strFullFileName);
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