// ADACtrl.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include ".\Ada\AdaCtrl.h"

#if (SUPPORT_SOFTWARE_TTS && UNDER_CE)
#  include ".\Ada\pttsE.h"			// [#2368] US KSK 2015.08.31
#endif

//#define NH_DEBUG					// [#11] NH KSK 2010.10.7 ADA관련 AP Hodling현상이 발생하여 추후 Holding시 Debug로 확인하기 위해 풀음. (Release판에도 Delay주는 용도로 Release함)
#include ".\Common\NHDbgApi.h"

#define DBG_INFO	1
#define DBG_CALL	1

#define		FS	0x1C
//#define		SCREEN_WITHAMOUNT		204		// [#2368] NH KSK 2015.10.19 불필요 Define 삭제
//#define		SCREEN_TRANSFERAMOUNT	401		// [#2368] NH KSK 2015.10.19 불필요 Define 삭제

//[#2077] CA PCS 2011.07.02 Allay For Language Selection 
const CString gLanguageFolder[LANGUAGE_MODE_MAX+1] =  
{
	_T(""),
	_T("ENG\\"),		// ENG_MODE 
	_T("SPN\\"),		// SPN_MODE 
	_T("FRN\\"),		// FRN_MODE
	_T("CHN\\"),		// CHN_MODE
	_T("KOR\\"),		// KOR_MODE
	_T("JPN\\")			// JPN_MODE
};// END OF [#2077] 

//[#2368] US KSK 2015.08.31
#define TTS_TEXT_BUFFER_SIZE	4096	// [#2386] NH KSK 2015.10.26

HWND				m_hWnd = NULL;
CString				m_strPlayText;
CMultiString		m_VoiceString;
int					m_nVolume = 0;
char				chTTSText[TTS_TEXT_BUFFER_SIZE] = { 0,};	// 기존 wave buffer 4KB로 할당되어 기존 사양대로 함
char				myTagString[128] = { 0,};						// play xml format을 위한 buffer
// end of [#2368]

#if (SUPPORT_SOFTWARE_TTS && UNDER_CE)
#  pragma comment(lib, "pttsE.lib")	// [#2368] US KSK 2015.08.31
#endif

CAdaCtrl::CAdaCtrl() : m_eStartToPlay(FALSE, FALSE), m_eDoneToPlay(TRUE, TRUE)
{
// [#2368] US KSK 2015.08.31
	CString strFilePath;	 //[#2077] CA PCS 2011.07.02

	m_hWnd = NULL;
	m_bPlay = FALSE;

	m_pDevCtrl = NULL;
	m_strCurrPlayList.Empty();

#if (SUPPORT_SOFTWARE_TTS)
	#if UNDER_CE
		PTTS_SetOemKey("B3EBC6BFB7AFBDBAC8BFBCBA-3135-41544D-BFB5BEEE-C7D5BCBAB1E2");
		PTTS_Initialize();

		int nResult = -1;
		nResult = PTTS_LoadEngine(EN_US, ADA_TTS_DB_FILE_PATH, 0);

		if (nResult != 0)
		{
			NVDump('F', 'Q', "04", L"", L"TTS_LOAD_FAIL" );
			NHDEBUG(DBG_CALL, (L"CALL : LOAD TTS Engine Failed)\n"));
		}
	#else
		pVoice = NULL;

		HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);

		if (FAILED(hr))
		{
			NVDump('F', 'Q', "04", L"", L"TTS_LOAD_FAIL");
			NHDEBUG(DBG_CALL, (L"CALL : LOAD TTS Engine Failed)\n"));
		}
	#endif

	LoadDataFromFile(ENG_MODE, ADA_WAVE_FILE_PATH, ADA_TTS_DAT_FILE); // 2번째 Parameter의 Path는 의미없음

	m_strPlayText.Empty();
	m_nVolume = 0;
#else
	for (int i = 1; i < LANGUAGE_MODE_MAX+1; i++) //[#2077] CA PCS 2011.07.02
	{
	#if (CA_VERSION)
		strFilePath.Format(_T("%s%s"), ADA_WAVE_FILE_PATH, gLanguageFolder[i]);//[#2077] CA PCS 2011.07.02
	#else
		strFilePath.Format(_T("%s"), ADA_WAVE_FILE_PATH);	// US, AU는 \\ATM\\Wave\\ 아래 있음
	#endif
		LoadDataFromFile(i, strFilePath, ADA_WAVE_DAT_FILE); //[#2077] CA PCS 2011.07.02
	}
#endif
	// end of [#2368]

	m_language = ENG_MODE;	//[#2077] CA PCS 2011.07.02 영어거 디폴트로 설정됨. 

	Resume();
}

CAdaCtrl::~CAdaCtrl()
{
	// [#2368] US KSK 2015.08.31
#if (SUPPORT_SOFTWARE_TTS)
	#if UNDER_CE
		PTTS_UnLoadEngine(EN_US);
		PTTS_UnInitialize();
	#else
		pVoice->Release();
		pVoice = NULL;
	#endif

	NHDEBUG(DBG_CALL, (L"CALL : UNLOAD TTS Engine)\n"));
#endif
	// end of [#2368]

	m_bExit = TRUE;
	WaitForExitThread();
}

// [#2368] US KSK 2015.08.31
void CAdaCtrl::fnExp_TTS_Initialize(HWND hWnd)
{
	m_hWnd = hWnd;
}
// end of [#2368]

// [#2375] US Justin 2015.10.29
BOOL CAdaCtrl::fnExp_TTS_IsWaveDataExist(CString strWaveID)
{
#if (SUPPORT_SOFTWARE_TTS)
	CString strRtnText = m_VoiceString.GetStringByTextID(strWaveID);
	NHDEBUG(DBG_CALL, (L"Check Wave Data, Check =[%s], returned=[%s])\n", strWaveID, strRtnText ));
	if( (strRtnText.CompareNoCase(strWaveID)==0) || (strRtnText.GetLength() <=0) )
		return FALSE;
	return TRUE;
#else
	NHDEBUG(DBG_CALL, (L"CALL : UNLOAD TTS Engine)\n"));
	return FALSE;
#endif
}
// End of [#2375]


void CAdaCtrl::fnExp_SetDevPointer(CDevCmn *pDevCmn)
{
	m_pDevCtrl = pDevCmn;
}

void CAdaCtrl::LoadDataFromFile(int languageIndex, CString FilePath, CString FileName)//[#2077] CA PCS 2011.07.02 
{
#ifdef APP_LOCAL_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !(US_VERSION || AU_VERSION || CA_VERSION) // [#2047] AU KJW 2011.04.20 AU ADA지원, [#2077] CA PCS 2011,06,26 CA ADA지원
	return;
#endif
// end of [#214]

	CString			strTemp;
	CFile			DatFile;
	CFileException	e;
	CString			strData;
	CString			strFullPath = FilePath + FileName; //[#2077] CA PCS 2011.07.02

	// [#2368] US KSK 2015.09.01 TTS Text File은 다국어를 고려하여 Unicode File Read하는 방식으로 변경
	if(FileName == ADA_WAVE_DAT_FILE)
	{
		if(DatFile.Open(strFullPath, CFile::modeRead, &e))//[#2077] CA PCS 2011.07.02
		{
			// [2ND] AIREAT 2009.04.15
			BYTE	*bBuffer = (BYTE*) new BYTE[(unsigned int)DatFile.GetLength()+1]();
			DatFile.Read(bBuffer, (unsigned int)DatFile.GetLength());
			// end of [2ND]
			strData = CString(&bBuffer[0]);
			delete [] bBuffer;
			DatFile.Close();
		}

		//0d,0a를 삭제한다.
		//Screen data 편집시 newline이 없으면 불편하므로 file에는 insert되어 있어나, 실제 data에서는 필요 없으므로 삭제한다.
		char szCarriageReturn[3] = {};
		szCarriageReturn[0] = 0x0d;
		szCarriageReturn[1] = 0x0a;
		CString strCarriageReturn = CString(szCarriageReturn);
		strData.Replace(strCarriageReturn, L"");

		CStringArray	arrTemp;
		CString			strKey;
		int				nLastIndex = 0;
		int				i = 0;

		SplitString(strData, (TCHAR)FS, arrTemp);

		nLastIndex = arrTemp.GetUpperBound();

		// 2008-01-08 V01.02.24 SRC-12
		//arrTemp[] = Screen Number 4자리 + wave file name
		for(i=0; i<=nLastIndex; i++)
		{
			if(arrTemp[i].GetLength() >= 4)
			{
				strKey = arrTemp[i].Left(4);
				strTemp = arrTemp[i].Mid(4);	//Screen number 3자리는 제외하고 data만 add한다.

				m_colWaveScrData[languageIndex].PutOnTailOfList(strKey, strTemp);		// [#413] AIREAT 2008.09.05 //[#2077] CA PCS 2011.07.02
			}
		}
		m_SavecolWaveScrData[languageIndex] = m_colWaveScrData[languageIndex];	// Save ColData //[#2077] CA PCS 2011.07.02
	}
	else if (FileName == ADA_TTS_DAT_FILE)
	{
		strTemp.Format(L"%s%s", ADA_WAVE_FILE_PATH, ADA_TTS_DAT_FILE);
		m_VoiceString.SetLoadingFileName(strTemp);
		m_VoiceString.SetLocale(m_language);
		m_VoiceString.Initialize();
	}
	// end of [#2368]
}

void CAdaCtrl::fnExp_AddWaveFile(int nScrNo, CString strFileName)
{
#ifdef APP_LOCAL_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !(US_VERSION || AU_VERSION || CA_VERSION) // [#2047] AU KJW 2011.04.20 AU ADA지원, [#2077] CA PCS 2011,06,26 CA ADA지원 2
	return;
#endif
// end of [#214]

	NHDEBUG(DBG_CALL, (L"CALL : ScrNo(%d), FileName(%s)\n", nScrNo, strFileName));

	CString strScrNo;

	strScrNo.Format(L"%04d", nScrNo); // 2008-01-08 V01.02.24 SRC-12

	CString strWaveData;
	m_colWaveScrData[m_language].FindDataOfList(strScrNo, strWaveData);				// [#413] AIREAT 2008.09.05 //[#2077] CA PCS 2011.07.02
	
	if(strWaveData.GetLength())
	{
#if SUPPORT_SOFTWARE_TTS	// [#2368] NH KSK 2015.10.19 TTS인 경우 Text에 "," 존재 가능하므로 FIELD_DELIMITER로 구분자 변경
		strWaveData += FIELD_DELIMITER;
#else
		strWaveData += ",";												//구분자 , 추가
#endif						// end of [#2368]

		strWaveData = strWaveData + strFileName;						//data 추가
		m_colWaveScrData[m_language].PutOnTailOfList(strScrNo, strWaveData);		// [#413] AIREAT 2008.09.05//[#2077] CA PCS 2011.07.02
	}
	else	//새로 추가한다.
	{
		m_colWaveScrData[m_language].PutOnTailOfList(strScrNo, strFileName);		// [#413] AIREAT 2008.09.05//[#2077] CA PCS 2011.07.02
	}

	NVDump('O', 'Q', "04", L"", strScrNo );
}

void CAdaCtrl::fnExp_AddWaveFileBreakDownNumber(int nScrNo, CString strNumber, CString strCurrencyType, BOOL bUseDecimalPoint)
{
#ifdef APP_LOCAL_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !(US_VERSION || AU_VERSION || CA_VERSION) // [#2077] CA PCS 2011,06,26 CA ADA지원
	return;
#endif
// end of [#214]

	NHDEBUG(DBG_CALL, (L"CALL : nScrNo(%d), strNumber(%s), strCurrency(%s) bUseDecimalPoint(%d)\n", nScrNo, strNumber, strCurrencyType, bUseDecimalPoint));

	// [#2005] NH KSK 2010.11.22 
	__int64	nDollar = 0;
	int	nCent = 0;
	int nBillion = 0;
	int nMillion = 0;
	int nThousand = 0;
	int nNumber = 0;
	CString strBalance, strDollar, strCent;
	char	szDollar[20] = { 0, };

	int nDotIdx = strNumber.Find(L".");

	if(nDotIdx != -1)
	{
		strBalance = strNumber;
		strBalance.Remove(',');
		strBalance.Remove('.');

		if (strCurrencyType == CURRENCY_TYPE)
		{
			strDollar = strBalance.Left(strBalance.GetLength()-2);
			strCent = strBalance.Right(2);
		}
		else
		{
			strDollar = strBalance.Left(nDotIdx);
			strCent = strBalance.Right(strBalance.GetLength() - nDotIdx);
		}
		// end of [#2368]
	}
	else
	{
		strDollar = strNumber;
	}

	// "-" Balance 처리 추가
	if (strDollar.Left(1) == L"-")
	{
		fnExp_AddWaveFile(nScrNo, L"MINUS.wav");
		strDollar = strDollar.Mid(1);
	}

	for(int i=0; i<strDollar.GetLength(); i++)
	{
		if (strDollar.GetAt(i) != '0')
		{
			strDollar = strDollar.Mid(i);
			break;
		}
	}

	WideToMulti(szDollar, strDollar, sizeof(szDollar));

	nDollar = _atoi64(szDollar);
	nCent = Asc2Int(strCent);

	if (nDollar > 9999999999) return; // 10자리 달라까지 지원(BILLION)

	if (nDollar >= 1000000) // 백만 이상 일경우 -> MILLION, BILLION 사용
	{
		nNumber = (int)(nDollar / 1000000);

		if (nNumber > 999)		// 4자리이면 BILLION 사용
		{
			nBillion = nNumber / 1000;
			ADA_NUMBER(nScrNo, nBillion);
			fnExp_AddWaveFile(nScrNo, L"BILLION.wav");

			nMillion = nNumber % 1000;
			if (nMillion)
			{
				ADA_NUMBER(nScrNo, nMillion);
				fnExp_AddWaveFile(nScrNo, L"MILLION.wav");
			}
		}
		else
		{
			ADA_NUMBER(nScrNo, nNumber);
			fnExp_AddWaveFile(nScrNo, L"MILLION.wav");
		}

		nNumber = (int)(nDollar % 1000000);    // 백만으로 나누었을 때, 나머지
		nThousand = nNumber / 1000;			// 나머지를 1000으로 나누었을 때 몫
		if(nThousand)
		{
			ADA_NUMBER(nScrNo, nThousand);
			fnExp_AddWaveFile(nScrNo, L"THOUSAND.wav");
		}

		nNumber = nNumber % 1000;       // 나머지를 1000으로 나누었을 때의 나머지	

		// [#2052] NH KSK 2011.04.28 Dollar 문구 추가 (호주 Customers 요청사항) - 공통 적용
		if (nNumber >= 0)
		{
			ADA_NUMBER(nScrNo, nNumber);

			// [#2368] US KSK 2015.10.21 하기 공통 로직으로 변경 (Multi Currency 지원을 위해)
//			if (nNumber == 1)
//				fnExp_AddWaveFile(nScrNo, L"Dollar.wav");
//			else
//				fnExp_AddWaveFile(nScrNo, L"Dollars.wav");
			// end of [#2368]
		}
		// end of [#2052]
	}
	else if(nDollar >= 1000) // 백만 미만일경우 -> THOUSAND 사용
	{	
		nThousand = (int)(nDollar / 1000);
		ADA_NUMBER(nScrNo, nThousand);
		fnExp_AddWaveFile(nScrNo, L"THOUSAND.wav");

		nNumber = (int)(nDollar % 1000);

		// [#2052] NH KSK 2011.04.28 Dollar 문구 추가 (호주 Customers 요청사항) - 공통 적용
		if (nNumber >= 0)
		{
			ADA_NUMBER(nScrNo, nNumber);

			// [#2368] US KSK 2015.10.21 하기 공통 로직으로 변경 (Multi Currency 지원을 위해)
//			if (nNumber == 1)
//				fnExp_AddWaveFile(nScrNo, L"Dollar.wav");
//			else
//				fnExp_AddWaveFile(nScrNo, L"Dollars.wav");
			// end of [#2368]
		}
		// end of [#2052]
	}
	else if (nDollar >= 0)		// [#2052] NH KSK 2011.04.28 Dollar 문구 추가 (호주 Customers 요청사항) - 공통 적용
	{
		ADA_NUMBER(nScrNo, (int)nDollar);

		// [#2368] US KSK 2015.10.21 하기 공통 로직으로 변경 (Multi Currency 지원을 위해)
//		if (nDollar == 1)
//			fnExp_AddWaveFile(nScrNo, L"Dollar.wav");
//		else
//			fnExp_AddWaveFile(nScrNo, L"Dollars.wav");
		nNumber = (int)nDollar;	// [#2368] US KSK 2015.10.21
		// end of [#2368]
	}
	// end of [#2052]

	// [#2368] US KSK 2015.10.21 하기 공통 로직으로 변경 (Multi Currency 지원을 위해)
	if (nNumber >= 0)
	{
		if (strCurrencyType == CURRENCY_TYPE)
		{
			if (nNumber == 1)
				fnExp_AddWaveFile(nScrNo, L"Dollar.wav");
			else
				fnExp_AddWaveFile(nScrNo, L"Dollars.wav");
		}
		else
		{
			if (nCent > 0)
				fnExp_AddWaveFile(nScrNo, L"Point.wav");
		}
	}
	// end of [#2368]

	// [#2368] US KSK 2015.10.21
	if (strCurrencyType == CURRENCY_TYPE)
	{
		// Cent 표시
		if(nCent > 0)	// KSK 2011.04.28 음수값은 skip하도록 추가 수정
		{
			ADA_NUMBER(nScrNo, nCent);

			if(nCent > 1)
				fnExp_AddWaveFile(nScrNo, L"Cents.wav");
			else
				fnExp_AddWaveFile(nScrNo, L"Cent.wav");
		}
	}
	else
	{
		CString strTemp;

		if (nCent > 0)
		{
			if (bUseDecimalPoint == TRUE)
			{
				ADA_NUMBER(nScrNo, (int)nCent);
			}
			else
			{
				for (int i=0; i<strCent.GetLength(); i++)
				{
					strTemp.Format(L"%c.wav", strCent.GetAt(i));
					fnExp_AddWaveFile(nScrNo, strTemp);
				}
			}
		}

		// [#2375] 2015.11.02 US Justin
		/*
		strTemp.Format(L"%s", strCurrencyType);		
		fnExp_AddWaveFile(nScrNo, strTemp);
		*/
		// Exchange Rate.... (Cardtronics Canada "CAD ")
		if( (strCurrencyType.GetLength()==3)||(strCurrencyType.GetLength()==4) )		
		{
			// Read like E.U.R
			strTemp.Format(L"%s.%s.%s", strCurrencyType.Left(1), strCurrencyType.Mid(1,1), strCurrencyType.Mid(2,1));
			fnExp_AddWaveFile(nScrNo, strTemp);
		}
		else if (strCurrencyType.GetLength() > 4 )
		{
			fnExp_AddWaveFile(nScrNo, strCurrencyType);		// Read wave file as is "US Dollar.wav" in Exchange Rate....
		}
		// End of [#2375]
	}
	// end of [#2368]
}

char	*NumFile[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
					"10", "11", "12", "13", "14", "15", "16", "17", "18", "19" };
char	*Num10File[] = { "0", "10", "20", "30", "40", "50", "60", "70", "80", "90" };

void CAdaCtrl::ADA_NUMBER(int nScrNo, int number)
{
#ifdef APP_LOCAL_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !(US_VERSION || AU_VERSION || CA_VERSION) // [#2047] AU KJW 2011.04.20 AU ADA지원, [#2077] CA PCS 2011,06,26 CA ADA지원
	return;
#endif
// end of [#214]

	NHDEBUG(DBG_CALL, (L"CALL : nScrNo(%d) number(%d)\n", nScrNo, number));

	CString strTemp;
	int	quotient = 0;

	if (number >= 100) 
	{
		quotient = number/100;
		strTemp = CString(NumFile[quotient]);
		strTemp += ".wav";

		fnExp_AddWaveFile(nScrNo, strTemp);
		fnExp_AddWaveFile(nScrNo, L"HUNDRED.wav");
		number = number % 100;
	}
	
	if (0 < number && number < 20) 
	{
		strTemp = CString(NumFile[number]);
		strTemp += ".wav";
		fnExp_AddWaveFile(nScrNo, strTemp);
	}
	else if(20 <= number && number <= 99) 
	{
		int nRemainder = 0;
		quotient = number/10;
		strTemp = CString(Num10File[quotient]);
		strTemp += ".wav";
		fnExp_AddWaveFile(nScrNo, strTemp);

		nRemainder = number % 10;
		if (nRemainder) 
		{
			strTemp = CString(NumFile[nRemainder]);
			strTemp += ".wav";
			fnExp_AddWaveFile(nScrNo, strTemp);
		}
	}
	else if(number == 0 && quotient == 0){
		strTemp = CString(NumFile[number]);
		strTemp += ".wav";
		fnExp_AddWaveFile(nScrNo, strTemp);		
	}
}

void CAdaCtrl::fnExp_ResetScrWaveFile()
{
#ifdef APP_LOCAL_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !(US_VERSION || AU_VERSION || CA_VERSION) // [#2047] AU KJW 2011.04.20 AU ADA지원, [#2077] CA PCS 2011,06,26 CA ADA지원
	return;
#endif
// end of [#214]

	NHDEBUG(DBG_CALL, (L"CALL\n"));

	m_colWaveScrData[m_language].DeleteAll();			// [#413] AIREAT 2008.09.05 //[#2077] CA PCS 2011.07.02
	m_colWaveScrData[m_language] = m_SavecolWaveScrData[m_language]; //[#2077] CA PCS 2011.07.02

#if SUPPORT_SOFTWARE_TTS	// [#2368] US KSK 2015.10.13
	m_strPlayText.Empty();
#endif						// end of [#2368]
}

BOOL CAdaCtrl::fnExp_StopPlay()
{
	CNHAutoLock	Sync(m_eMethodUse);

	NHDEBUG(DBG_CALL, (L"CALL\n"));

	m_bPlay = FALSE;

	if (m_eDoneToPlay.Wait(INFINITE) != WAIT_OBJECT_0)
		return FALSE;

	return TRUE;
}

// [#2375] US Justin 2015.11.04 US ADA
void CAdaCtrl::fnExp_StopAndResetWaveFile()									
{
	fnExp_StopPlay();
	fnExp_ResetScrWaveFile();
}

void CAdaCtrl::fnExp_ResetAndAddPlay(CString strFileName1, BOOL bWaitUntilStop, CString strFileName2, CString strFileName3)
{
	// Stop and Reset Loaded Wave Files
	fnExp_StopAndResetWaveFile();

	// Add Files
	fnExp_AddWaveFile(1, strFileName1);
	if(strFileName2.GetLength()>0)		fnExp_AddWaveFile(1, strFileName2);
	if(strFileName3.GetLength()>0)		fnExp_AddWaveFile(1, strFileName3);

	// Play
	fnExp_PlayScreenWave(1);

	// Wait Until Stop
	if(bWaitUntilStop)		
		fnExp_WaitUntilStop();
}
// End of [#2375]

BOOL CAdaCtrl::IsPlaying()
{
	if (m_eDoneToPlay.Wait(0) == WAIT_OBJECT_0)
		return FALSE;

	return TRUE;
}

void CAdaCtrl::fnExp_WaitUntilStop()
{
	NHDEBUG(DBG_CALL, (L"CALL\n"));

	m_eDoneToPlay.Wait(INFINITE);
}

BOOL CAdaCtrl::fnExp_PlayScreenWave(int nScrNo)
{
#ifdef APP_LOCAL_MODE	// [#429] [NH] KSK 2008.9.26
	return FALSE;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !(US_VERSION || AU_VERSION || CA_VERSION) // [#2047] AU KJW 2011.04.20 AU ADA지원, [#2077] CA PCS 2011,06,26 CA ADA지원
	return FALSE;
#endif
// end of [#214]

	CNHAutoLock	Sync(m_eMethodUse);

	NHDEBUG(DBG_CALL, (L"CALL : nScrNo(%d)\n", nScrNo));

	if (m_bPlay == TRUE)
	{
		m_bPlay = FALSE;
		m_eDoneToPlay.Wait(INFINITE);
	}

	CString strScrNo;
	strScrNo.Format(L"%04d", nScrNo);			// 2008-01-08 V01.02.24 SRC-12
	m_strCurrPlayList.Empty();
	m_colWaveScrData[m_language].FindDataOfList(strScrNo, m_strCurrPlayList);			// [#413] AIREAT 2008.09.05//[#2077] CA PCS 2011.07.02


	if(m_strCurrPlayList.GetLength() == 0)
	{
		NHDEBUG(DBG_INFO, (L"CurrentPlayList is empty..\n"));
		return FALSE;
	}

	NVDump('O', 'Q', "02", L"", strScrNo );

	m_bPlay = TRUE;
	m_eDoneToPlay.Reset();
	m_eStartToPlay.Set();

	return TRUE;
}

void CAdaCtrl::fnExp_SndPlay(CString strFileName,BOOL Nation) //[#2077] CA PCS 2011.07.15
{
	CString strFile;

#if (CA_VERSION)
	if (Nation == 1)
		strFile = ADA_WAVE_FILE_PATH + gLanguageFolder[m_language] + strFileName;
	else
		strFile = ADA_WAVE_FILE_PATH + strFileName;
#else
	strFile = ADA_WAVE_FILE_PATH + strFileName;
#endif

	sndPlaySound(strFile, SND_ASYNC);
}

void CAdaCtrl::fnExp_SetVolume(int nLevel)
{
	m_WavePlay.SetVolume(nLevel);

	m_nVolume = nLevel;

#if (SUPPORT_SOFTWARE_TTS && !UNDER_CE)
	HRESULT hr = pVoice->SetVolume(nLevel * 10);

	if (FAILED(hr))
	{
		NHDEBUG(DBG_CALL, (L"CALL ERROR : Win32 TTS Engine SetVolume (%d).\n", m_nVolume * 10));
	}
#endif
}

void CAdaCtrl::fnExp_SetVolumeUp()
{
	m_WavePlay.VolumeUp();

	// [#2368] US KSK 2015.10.13
	m_nVolume++;
	if (m_nVolume > 10)
		m_nVolume = 10;
	// end of [#2368]

#if (SUPPORT_SOFTWARE_TTS && !UNDER_CE)
	HRESULT hr = pVoice->SetVolume(m_nVolume * 10);

	if (FAILED(hr))
	{
		NHDEBUG(DBG_CALL, (L"CALL ERROR : Win32 TTS Engine SetVolume (%d).\n", m_nVolume * 10));
	}
#endif
}

void CAdaCtrl::fnExp_SetVolumeDown()
{
	m_WavePlay.VolumeDown();

	// [#2368] US KSK 2015.10.13
	m_nVolume--;
	if (m_nVolume < 0)
		m_nVolume = 0;
	// end of [#2368]

#if (SUPPORT_SOFTWARE_TTS && !UNDER_CE)
	HRESULT hr = pVoice->SetVolume(m_nVolume * 10);

	if (FAILED(hr))
	{
		NHDEBUG(DBG_CALL, (L"CALL : SetVolume (%d) failed.\n", m_nVolume * 10));
	}
#endif
}

unsigned CAdaCtrl::ThreadHandlerProc(void)
{
	int				i;
	int				nCountOfPlayList;
	CString			strPlayFile;
	CStringArray	strPlayListArr;

	m_bExit = FALSE;

	while(m_bExit == FALSE)
	{
		if (m_eStartToPlay.Wait(100) == WAIT_OBJECT_0)
		{
			strPlayListArr.RemoveAll();

#if SUPPORT_SOFTWARE_TTS	// [#2368] NH KSK 2015.10.19 TTS인 경우 Text에 "," 존재 가능하므로 FIELD_DELIMITER로 구분자 변경
			SplitString(m_strCurrPlayList, FIELD_DELIMITER, strPlayListArr);
#else
			SplitString(m_strCurrPlayList, ",", strPlayListArr);
#endif						// end of [#2368]

			nCountOfPlayList = strPlayListArr.GetUpperBound();

			for (i = 0; i <= nCountOfPlayList && m_bPlay == TRUE; i++)
			{
#if SUPPORT_SOFTWARE_TTS		// [#2368] US KSK 2015.10.16
				// 해당 wave file로 되어져 있는 file text를 조합하여 Play함
				int		nResult = -1;
				
				memset(myTagString, 0, sizeof(myTagString));
				memset(chTTSText, 0, sizeof(chTTSText));

				// dat file에서 text loading
				strPlayFile.Format(_T("%s"), strPlayListArr[i]);
				NHDEBUG(DBG_CALL, (L"CALL : Play File Name(%s)\n", strPlayFile));
				m_strPlayText = m_VoiceString.GetStringByTextID(strPlayFile);

				// Volume 범위가 0 ~ 200이므로 설정 정보에 20단위로 조절함
				sprintf_s(myTagString, sizeof(myTagString), "<speed=\"100\"><volume=\"%d\">", (m_nVolume*20));

				// Text가 Buffer Size 초과시 Buffer만큼만 Play하도록 예외처리
				if (m_strPlayText.GetLength() >= TTS_TEXT_BUFFER_SIZE)
				{
					WideToMulti(chTTSText, m_strPlayText.Left(TTS_TEXT_BUFFER_SIZE-1), sizeof(chTTSText));
				}
				else
				{
					if (m_strPlayText.GetLength() <= 0)
					{
						if (strPlayFile.GetLength() >= TTS_TEXT_BUFFER_SIZE)
							WideToMulti(chTTSText, strPlayFile.Left(TTS_TEXT_BUFFER_SIZE-1), sizeof(chTTSText));
						else
							WideToMulti(chTTSText, strPlayFile, sizeof(chTTSText));
					}
					else
					{
						WideToMulti(chTTSText, m_strPlayText, sizeof(chTTSText));
					}
				}

				NHDEBUG(DBG_CALL, (L"CALL : Play Text (%S)\n", chTTSText));
			
				#if UNDER_CE
					if (m_hWnd != NULL)
						nResult = PTTS_PlayTTS(m_hWnd, 1501, chTTSText, myTagString, EN_US, 4);
				#else
					if (pVoice != NULL)
						nResult = pVoice->Speak(CString(chTTSText), 0, NULL);
				#endif

				if (nResult == 0)
				{
					while (m_bPlay == TRUE)
					{
						if (m_pDevCtrl->fnSNS_GetEnhancedAudio() == FALSE)
							break;

						#if UNDER_CE
							if (PTTS_GetPlaybackStatus() != PTTS_PLAYBACK_PLAY)
								break;
						#else
							SPVOICESTATUS eventStatus;
							pVoice->GetStatus(&eventStatus, NULL);

							if (eventStatus.dwRunningState)
								break;
						#endif

						Delay_Msg(50);
					}
				}
#else
				//[#2077] CA PCS 2011.06.27 다국어ADA 지원시, 재생 파일을 "\언어\****.wav" 파일에서 찾기 위한 구현 
#if (CA_VERSION)
				strPlayFile.Format(_T("%s%s%s"), ADA_WAVE_FILE_PATH, gLanguageFolder[m_language], strPlayListArr[i]); //[#2077] CA PCS 2011.07.02
#else
				strPlayFile.Format(_T("%s%s"), ADA_WAVE_FILE_PATH, strPlayListArr[i]); // US, AU는 \\ATM\\WAVE\\ 위치 유지 (이전사양)
#endif
				//END OF [#2077]

				if (m_WavePlay.Play(strPlayFile) == TRUE)
				{
					while (m_bPlay == TRUE)
					{
						if (m_pDevCtrl->fnSNS_GetEnhancedAudio() == FALSE)
							break;

						if (m_WavePlay.IsPlaying(50) == FALSE)
							break;
					}
				}
#endif							// end of [#2368]

				if (m_pDevCtrl->fnSNS_GetEnhancedAudio() == FALSE)
					break;
			}

#if SUPPORT_SOFTWARE_TTS		// [#2368] US KSK 2015.10.16
			#if UNDER_CE
				PTTS_StopTTS();
			#else
				// pVoice->Speak(NULL, 0, NULL);
			#endif
#else
			m_WavePlay.Stop();
#endif							// end of [#2368]

			if (m_pDevCtrl->fnSNS_GetEnhancedAudio() == FALSE)
			{
				m_WavePlay.SetVolume(MemGetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_VOLUMELEVEL));
				m_nVolume = MemGetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_VOLUMELEVEL);	// [#2368] US KSK 2015.10.19
			}

			m_eDoneToPlay.Set();
			m_bPlay = FALSE;
		}
	}

	return 0;
}

// [#2077] CA PCS 2011.07.02 For Refactoring By CHA.
void CAdaCtrl::fnExp_SetLanguage(int language)
{
	NHDEBUG(DBG_INFO, (_T("[fnExp_SetLanguage][%d]\n"), language));

	if (m_language < 1 || m_language > 6)
		return;

	m_language = language;
}
// End of [#2077]

CString CAdaCtrl::fnExp_TTS_GetWaveData(CString strWaveID)
{
	if (fnExp_TTS_IsWaveDataExist(strWaveID))
	{
		return m_VoiceString.GetStringByTextID(strWaveID);
	}

	return L"";
}