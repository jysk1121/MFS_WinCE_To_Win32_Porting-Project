// ADACtrlMain.cpp : implementation file
//

#include "stdafx.h"
#include ".\Common\CmnLib.h"
#include ".\Common\ConstDef.h"
#include ".\Dll\Devdefine.h"
#include ".\Dll\DevCmn.h"
#include ".\Dll\ADACtrlMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CADACtrlMain

CADACtrlMain::CADACtrlMain()
{

#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

#if (US_VERSION)
	m_strCurrPlayList = "";
	m_bStopThread = false;

	// [#157] KSK 2008.04.22 Ada 관련 PATH 지정
	LoadDataFromFile(ADA_WAVE_DAT_FILE);
	LoadDataFromFile(ADA_FDKMAP_DAT_FILE);
	// end of [#157]

	m_hScreenWaveThread = NULL; // 2008-01-08 V01.02.24 SRC-11
	m_pDevPtr = NULL;
	m_bIsWavePlay = FALSE;

	InitializeCriticalSection(&m_cCriticalSection); // 2008-01-08 V01.02.24 SRC-11
#endif
}

CADACtrlMain::~CADACtrlMain()
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

#if (US_VERSION)
	m_colWaveScrData.DeleteAll();		// [#413] AIREAT 2008.09.05
	m_colWaveFDKMap.DeleteAll();		// [#413] AIREAT 2008.09.05

	// 2008-01-08 V01.02.24 SRC-11
	if (m_hScreenWaveThread)
	{
		fnExp_StopPlay();

		DWORD dwExitCode = STILL_ACTIVE;

		while (dwExitCode == STILL_ACTIVE)
		{
			Delay_Msg(100);
			GetExitCodeThread(m_hScreenWaveThread, &dwExitCode);
		}

		CloseHandle(m_hScreenWaveThread);
		m_hScreenWaveThread = NULL;
	}

	DeleteCriticalSection(&m_cCriticalSection); // 2008-01-08 V01.02.24	SRC-11
#endif
}


BEGIN_MESSAGE_MAP(CADACtrlMain, CWnd)
	//{{AFX_MSG_MAP(CADACtrlMain)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: ScreenWaveThread
// RETURN TYPE  : UINT
// PARAMETER    : LPVOID pParam
// DESCRIPTION  : 화면 안내 wave file을 출력한다.
// 출력할 wave file이 2개 이상인 경우 순서대로, 출력 완료 후 다음 wave를 play한다.
// 2008-01-07 V01.02.24 SRC-11
DWORD ScreenWaveThread(LPVOID pParam)
{
	CADACtrlMain* pADAMain =  (CADACtrlMain*)pParam;
	
	HANDLE	hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
	CString strPlayList = pADAMain->m_strCurrPlayList;
	CStringArray strArr;

	SplitString(strPlayList, ",", strArr);
	int nLastIndex = strArr.GetUpperBound();

	CString strFile;
	for(int i=0; i<=nLastIndex; i++)
	{
		if(pADAMain->m_bStopThread)
			goto play_stop;
		
		strFile = ADA_WAVE_FILE_PATH + strArr[i];		// [#157] KSK 2008.04.22

		//wave file을 출력한다.
		pADAMain->fnExp_PlayWaveFile(strFile);
		
		//play완료까지 대기한다.
		while(1)
		{
			if(pADAMain->m_bStopThread)
				goto play_stop;

			if(pADAMain->m_pDevPtr)
			{
				if(pADAMain->m_pDevPtr->fnSNS_GetEnhancedAudio() == FALSE)
				{
					pADAMain->fnExp_StopPlay();
					WaitForSingleObject(hEvent, 1000);
					int nVol = MemGetInt(_MEM_FLD_INI_DEVINFO, _MEM_VAR_DEVINFO_VOLUMELEVEL);
					pADAMain->fnExp_SetVolume(nVol);
					goto play_stop;
				}
			}
			
			if(pADAMain->IsPlaying())
				WaitForSingleObject(hEvent, 50);
			else
				break;
		}
		WaitForSingleObject(hEvent, 50);
	}

play_stop:
	if (hEvent)
		CloseHandle(hEvent);
	pADAMain->m_bIsWavePlay = FALSE;

	NVDump('O', 'Q', "00", L"", L"TH_RETURN" );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CADACtrlMain message handlers

/////////////////////////////////////////////////////////////////////////////
// CADACtrlMain private methods
void CADACtrlMain::LoadDataFromFile(CString FileName)
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]
	
// [#214] UK JSW 2008.06.03
#if !US_VERSION
	return;
#endif
// end of [#214]



	CString			strTemp;
	CFile			DatFile;
	CFileException	e;
	CString			strData;

	if(DatFile.Open(FileName, CFile::modeRead, &e))
	{
		// [2ND] AIREAT 2009.04.15
//		BYTE	*bBuffer = new BYTE[DatFile.GetLength()+1];
//		memset(bBuffer, 0, DatFile.GetLength()+1);
//		DatFile.Read(bBuffer, DatFile.GetLength());
		BYTE	*bBuffer = (BYTE*) new BYTE[(unsigned int)DatFile.GetLength()+1];
		memset(bBuffer, 0, (unsigned int)DatFile.GetLength()+1);
		DatFile.Read(bBuffer, (unsigned int)DatFile.GetLength());
		// end of [2ND]
		strData = CString(&bBuffer[0]);
		delete [] bBuffer;
		DatFile.Close();
	}

	//0d,0a를 삭제한다.
	//Screen data 편집시 newline이 없으면 불편하므로 file에는 insert되어 있어나, 실제 data에서는 필요 없으므로 삭제한다.
	char szCarriageReturn[3];
	memset(szCarriageReturn, NULL, sizeof(szCarriageReturn));
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

	// [#157] KSK 2008.04.22
	if(FileName.CompareNoCase(ADA_WAVE_DAT_FILE) == 0)
	{
		// 2008-01-08 V01.02.24 SRC-12
		//arrTemp[] = Screen Number 4자리 + wave file name
		for(i=0; i<=nLastIndex; i++)
		{
			if(arrTemp[i].GetLength() >= 4)
			{
				strKey = arrTemp[i].Left(4);
				strTemp = arrTemp[i].Mid(4);	//Screen number 3자리는 제외하고 data만 add한다.

				m_colWaveScrData.PutOnTailOfList(strKey, strTemp);		// [#413] AIREAT 2008.09.05
			}
		}
		m_SavecolWaveScrData = m_colWaveScrData;	// Save ColData
	}
	else if(FileName.CompareNoCase(ADA_FDKMAP_DAT_FILE) == 0)
	{
		// 2008-01-08 V01.02.24 SRC-12
		//arrTemp[i] = Screen Number 4자리 + FDK information
		for(i=0; i<=nLastIndex; i++)
		{
			if(arrTemp[i].GetLength() >= 4)
			{
				strKey = arrTemp[i].Left(4);
				strTemp = arrTemp[i].Mid(4);	//Screen number 3자리는 제외하고 data만 add한다.

				m_colWaveFDKMap.PutOnTailOfList(strKey, strTemp);		// [#413] AIREAT 2008.09.05
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: CADACtrlMain::IsPlaying
// RETURN TYPE  : bool
// DESCRIPTION  : 
//
BOOL CADACtrlMain::IsPlaying()
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return FALSE;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !US_VERSION
	return FALSE;
#endif
// end of [#214]

	return m_waveOut.IsPlaying();
}

// [#167] KSK 2008.04.24
void CADACtrlMain::fnExp_WaitUntilStop()
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]
	
// [#214] UK JSW 2008.06.03
#if !US_VERSION
	return;
#endif
// end of [#214]

	DWORD	dwStart = GetTickCount();
	DWORD	MaxTimeout = 5 * 60 * 1000;	// Max 5분 설정함

	Delay_Msg(500);	// wave file loading 및 play check를 위해 delay를 줌

	while ((GetTickCount() - dwStart) < MaxTimeout)
	{
//		if (IsPlaying() == FALSE)	break;
		if (m_bIsWavePlay == FALSE)	break;

		Delay_Msg(50);
	}

	Delay_Msg(1000);	// Exit전 1초정도의 Delay를 줌
}
// end of [#167]

/////////////////////////////////////////////////////////////////////////////
// CADACtrlMain public methods


////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: CADACtrlMain::fnExp_PlayScreenWave
// RETURN TYPE  : bool
// PARAMETER    : CString strScreenNumber
// DESCRIPTION  : 
//
BOOL CADACtrlMain::fnExp_PlayScreenWave(int nScrNo)
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return FALSE;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !US_VERSION
	return FALSE;
#endif
// end of [#214]

	CString strScrNo;

	EnterCriticalSection(&m_cCriticalSection);	// 2008-01-08 V01.02.24	SRC-11

	fnExp_StopPlay();
	strScrNo.Format(L"%04d", nScrNo);			// 2008-01-08 V01.02.24 SRC-12
	
	m_strCurrPlayList = L"";												// [#413] AIREAT 2008.09.05
	m_colWaveScrData.FindDataOfList(strScrNo, m_strCurrPlayList);			// [#413] AIREAT 2008.09.05

	if(m_strCurrPlayList.GetLength() == 0)		return FALSE;
	
	NVDump('O', 'Q', "02", L"", strScrNo );

	if (m_hScreenWaveThread)
	{
		DWORD dwExitCode = STILL_ACTIVE;

		while (dwExitCode == STILL_ACTIVE)
		{
			Delay_Msg(100);
			GetExitCodeThread(m_hScreenWaveThread, &dwExitCode);
		}

		CloseHandle(m_hScreenWaveThread);
		m_hScreenWaveThread = NULL;
	}

	m_bStopThread = false;
	m_waveOut.m_bPlaying = true;
	m_bIsWavePlay = TRUE;

	DWORD dwThreadID = 0;
	m_hScreenWaveThread = CreateThread(NULL, 0, ScreenWaveThread, this, 0, &dwThreadID);

	if (m_hScreenWaveThread == NULL)
	{
		m_bStopThread = false;
		m_waveOut.m_bPlaying = false;
		m_bIsWavePlay = FALSE;
	}

	LeaveCriticalSection(&m_cCriticalSection);	// 2008-01-08 V01.02.24	SRC-11

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: CADACtrlMain::fnExp_PlayWaveFile
// RETURN TYPE  : bool
// PARAMETER    : CString strFileName
// DESCRIPTION  : m_colWaveScrData에서 screen number에 해당하는 data를 가져와서 모두 출력한다.
//
BOOL CADACtrlMain::fnExp_PlayWaveFile(CString strFileName)
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return FALSE;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !US_VERSION
	return FALSE;
#endif
// end of [#214]

	CFile cwf;
	if(!cwf.Open(strFileName,CFile::modeRead))
	{
		NVDump('F', 'Q', "03", L"FOpnEr0", L"2:NoWavFile");
		return FALSE;
	}
	else
	{
		cwf.Close();
	}

	if ( m_waveOut.IsPlaying() ) 
	{
		if ( !m_waveOut.Close() ) 
		{
			return FALSE;
		}
	}
	//Wave file memory load
	m_monWave.Load( strFileName );

	if ( !m_monDevice.IsOutputFormat(m_monWave) ) 
	{
		return FALSE;
	}
	
	m_waveOut.SetWave(m_monWave);
	m_waveOut.SetDevice(m_monDevice);

	if ( !m_waveOut.Open() ) 
	{
		return FALSE;
	}

	m_waveOut.m_bPlaying = true;
	
	if ( !m_waveOut.FullPlay() ) 
	{
		return FALSE;
	}
	
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: CADACtrlMain::fnExp_AddWaveFile
// RETURN TYPE  : bool
// PARAMETER    : CString strScrNo
// PARAMETER    : CString strFileName
// DESCRIPTION  : m_colWaveScrData에 wave file를 add한다.
//
void CADACtrlMain::fnExp_AddWaveFile(int nScrNo, CString strFileName)
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !US_VERSION
	return;
#endif
// end of [#214]

	CString strScrNo;

	strScrNo.Format(L"%04d", nScrNo); // 2008-01-08 V01.02.24 SRC-12

	CString strWaveData;
	m_colWaveScrData.FindDataOfList(strScrNo, strWaveData);				// [#413] AIREAT 2008.09.05
	
	if(strWaveData.GetLength())
	{
		strWaveData += ",";												//구분자 , 추가
		strWaveData = strWaveData + strFileName;						//data 추가
		m_colWaveScrData.PutOnTailOfList(strScrNo, strWaveData);		// [#413] AIREAT 2008.09.05
	}
	else	//새로 추가한다.
	{
		m_colWaveScrData.PutOnTailOfList(strScrNo, strFileName);		// [#413] AIREAT 2008.09.05
	}
	NVDump('O', 'Q', "04", L"", strScrNo );
}

void CADACtrlMain::fnExp_AddWaveFileBreakDownNumber(int nScrNo, CString strNumber, bool bDot)
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !US_VERSION
	return;
#endif
// end of [#214]

	int nDollar = 0;
	int nCent = 0;
	CString strDollar="", strCent="";
	int nDotIdx = 0;

	nDotIdx = strNumber.Find(L".");

	if(nDotIdx != -1)
	{
		strDollar = strNumber.Left(nDotIdx);
		strCent = strNumber.Right( strNumber.GetLength() - (nDotIdx+1) );
	}
	else
	{
		strDollar = strNumber;
	}
	nDollar = Asc2Int(strDollar);
	nCent = Asc2Int(strCent);
	
	if(strDollar.GetLength())
	{
		int	quotient = 0;
		bool isThousand = false;
		
		if (nDollar > 999999) 
			return;
		if (nDollar >= 1000) 
		{
			quotient = nDollar / 1000;
			ADA_NUMBER(nScrNo, quotient);
			
			fnExp_AddWaveFile(nScrNo, L"THOUSAND.wav");
			nDollar = nDollar % 1000;
			isThousand = true;
		}

		if (nDollar >= 0) 
		{
			if(nDollar == 0 && nCent != 0){
			}
			else if(isThousand && nDollar == 0){
			}
			else{
				ADA_NUMBER(nScrNo, nDollar);
			}
		}

		if(nDollar > 1){
			fnExp_AddWaveFile(nScrNo, L"Dollars.wav");
		}
		else{
			if(nDollar == 0 && nCent != 0){
			}
			else{
				fnExp_AddWaveFile(nScrNo, L"Dollar.wav");
			}
		}
 	}

	if(nCent)
	{
		ADA_NUMBER(nScrNo, nCent);

		if(nCent > 1)
			fnExp_AddWaveFile(nScrNo, L"Cents.wav");
		else
			fnExp_AddWaveFile(nScrNo, L"Cent.wav");
 	}

}

char	*NumFile[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
					"10", "11", "12", "13", "14", "15", "16", "17", "18", "19" };
char	*Num10File[] = { "0", "10", "20", "30", "40", "50", "60", "70", "80", "90" };

void CADACtrlMain::ADA_NUMBER(int nScrNo, int number)
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !US_VERSION
	return;
#endif
// end of [#214]

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

// [#165] KSK 2008.04.23
void CADACtrlMain::fnExp_ResetScrWaveFile()
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !US_VERSION
	return;
#endif
// end of [#214]

	m_colWaveScrData.DeleteAll();			// [#413] AIREAT 2008.09.05
	m_colWaveScrData = m_SavecolWaveScrData;
}
// end of [#165]

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: CADACtrlMain::fnExp_PlayStop
// RETURN TYPE  : bool
// DESCRIPTION  : 출력 중지
//
BOOL CADACtrlMain::fnExp_StopPlay()
{
#ifdef APP_SHOW_MODE	// [#429] [NH] KSK 2008.9.26
	return TRUE;
#endif					// end of [#429]

// [#214] UK JSW 2008.06.03
#if !US_VERSION
	return TRUE;
#endif
// end of [#214]

	m_waveOut.Pause();
	m_waveOut.Close();
	m_bStopThread = true;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: CADACtrlMain::fnExp_SndPlay
// RETURN TYPE  : void
// PARAMETER    : CString strFileName
// DESCRIPTION  : SndPlay API 함수 사용, 짧은 wave를 출력할 때 사용한다.
//
void CADACtrlMain::fnExp_SndPlay(CString strFileName)
{
	CString strFile = ADA_WAVE_FILE_PATH + strFileName;
	sndPlaySound(strFile, SND_ASYNC);
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: CADACtrlMain::fnExp_CheckValidKey
// RETURN TYPE  : bool
// PARAMETER    : int nScrNo
// PARAMETER    : CString strKey
// DESCRIPTION  : 
//
BOOL CADACtrlMain::fnExp_CheckValidKey(int nScrNo, CString strKey)
{
	//특수키
	if(strKey == "CANCEL" || strKey == "CLEAR" || strKey == "ENTER")			return TRUE;
	
	//출금, 이체 금액 입력화면에서의 숫자키
	if(nScrNo == SCREEN_WITHAMOUNT || nScrNo == SCREEN_TRANSFERAMOUNT)
	{
		if ((strKey >= "0")	&&	(strKey <= "9")	&& (strKey.GetLength() == 1))	return TRUE;
		else																	return FALSE;
	}

	//기타 menu선택 화면 : WaveFDKMap에 정의된 숫자키 확인 (0~9 사이의 숫자키)
	CString strScrNo;
	strScrNo.Format(L"%04d", nScrNo); // 2008-01-08 V01.02.24 SRC-12

	CString strFDKarr;
	m_colWaveFDKMap.FindDataOfList(strScrNo, strFDKarr);			// [#413] AIREAT 2008.09.05

	if(strFDKarr.GetLength() == 0)					return FALSE;

	if ((strKey >= "0")	&& (strKey <= "9")	&& (strKey.GetLength() == 1))
	{
		if(strFDKarr.Find(strKey) == -1)			return FALSE;
		else										return TRUE;
	}
	
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: CADACtrlMain::fnExp_GetFDKMappingByNumeric
// RETURN TYPE  : CString
// PARAMETER    : int nScrNo
// PARAMETER    : CString strKey
// DESCRIPTION  : 
//
CString CADACtrlMain::fnExp_GetFDKMappingByNumeric(int nScrNo, CString strKey)
{
	CString strReturn;
	CString strScrNo;
	CString strFDKarr;

	strScrNo.Format(L"%04d", nScrNo); // 2008-01-08 V01.02.24 SRC-12

	m_colWaveFDKMap.FindDataOfList(strScrNo, strFDKarr);			// [#413] AIREAT 2008.09.05

	if(strFDKarr.GetLength() == 0)				return strReturn;			//Invalid

	if ((strKey >= "0")	&& (strKey <= "9")	&& (strKey.GetLength() == 1))
	{
		CStringArray strArr;

		SplitString(strFDKarr, ",", strArr);
		int nLastIndex = strArr.GetUpperBound();
		
		for(int i=0; i<=nLastIndex; i++)
		{
			if(strArr[i].Left(1) == strKey)		return strArr[i].Right(2);	// [#24] KSK 2008.03.11 ScrCtrl 변경됨에 따라 FDKMap 변경
		}
	}

	return strReturn;	//Invalid
}

void CADACtrlMain::fnExp_SetVolume(int nLevel)
{
	EnterCriticalSection(&m_cCriticalSection); // 2008-01-08 V01.02.24 SRC-11
	m_waveOut.SetVolume(nLevel);
	LeaveCriticalSection(&m_cCriticalSection); // 2008-01-08 V01.02.24 SRC-11
}

void CADACtrlMain::fnExp_SetVolumeUp()
{
	EnterCriticalSection(&m_cCriticalSection); // 2008-01-08 V01.02.24 SRC-11
	m_waveOut.SetVolumeUp();
	LeaveCriticalSection(&m_cCriticalSection); // 2008-01-08 V01.02.24 SRC-11
}

void CADACtrlMain::fnExp_SetVolumeDown()
{
	EnterCriticalSection(&m_cCriticalSection); // 2008-01-08 V01.02.24 SRC-11
	m_waveOut.SetVolumeDown();
	LeaveCriticalSection(&m_cCriticalSection); // 2008-01-08 V01.02.24 SRC-11
}
