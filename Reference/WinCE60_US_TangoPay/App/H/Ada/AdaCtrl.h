#ifndef __ADA_CTRL_H__
#define __ADA_CTRL_H__

#ifndef _ADACTRL_OWNER_
#pragma comment(lib, "AdaCtrl.lib")
#endif

#include ".\Dev\DevCmn.h"
#include ".\Common\NHCtrlThread.h"
#include ".\Dll\NHCollection.h"
#include ".\Ada\WavePlay.h"

// [#RWC6-399] US ryan.payton 2022.10.03 DigitalMint
#if (SUPPORT_SOFTWARE_TTS && !UNDER_CE)
	#include "sapi.h"
#endif
// End of [#RWC6-399]

class AFX_CLASS_EXPORT CAdaCtrl : public CNHCtrlThread
{
// Construction
public:
	CAdaCtrl();
	~CAdaCtrl();

public:
	void fnExp_TTS_Initialize(HWND hWnd);	// [#2368] US KSK 2015.08.31
	BOOL fnExp_TTS_IsWaveDataExist(CString strWaveID);	// [#2375] US Justin

	void fnExp_SetDevPointer(CDevCmn* pDevCmn);
	void fnExp_AddWaveFile(int nScrNo, CString strFileName);
	void ADA_NUMBER(int nScrNo, int number);	// Number만 Play할때 필요함
//	void fnExp_AddWaveFileBreakDownNumber(int nScrNo, CString strNumber, bool bDot=false);
	void fnExp_AddWaveFileBreakDownNumber(int nScrNo, CString strNumber, CString strCurrencyType = CURRENCY_TYPE, BOOL bUseDecimalPoint = FALSE);	// [#2368] US KSK 2015.1021 Multi Currency 지원을 위해 Parameter 변경
	void fnExp_ResetScrWaveFile();		// [#165] KSK 2008.04.23
	void fnExp_SndPlay(CString strFileName,BOOL Nation=0);
	BOOL fnExp_PlayScreenWave(int nScrNo);
	BOOL fnExp_StopPlay();
	BOOL IsPlaying();
	void fnExp_WaitUntilStop();											// [#167] KSK 2008.04.24
	void fnExp_StopAndResetWaveFile();									// [#2375] US Justin 2015.11.04 US ADA
	void fnExp_ResetAndAddPlay(CString strFileName1, BOOL bWaitUntilStop = TRUE, CString strFileName2 = L"", CString strFileName3 = L"");			// [#2375] US Justin 2015.11.04 US ADA
//	BOOL fnExp_CheckValidKey(int nScrNo, CString strKey);				// KSK 2011.07.12 사용안함
//	CString fnExp_GetFDKMappingByNumeric(int nScrNo, CString strKey);	// KSK 2011.07.12 사용안함

	void	fnExp_SetVolume(int nLevel);
	void	fnExp_SetVolumeUp();
	void	fnExp_SetVolumeDown();

	void	fnExp_SetLanguage(int language);

	CString	fnExp_TTS_GetWaveData(CString strWaveID);
private:
	void LoadDataFromFile(int languageIndex, CString FilePath, CString FileName);

protected:
	unsigned ThreadHandlerProc(void);

private:
	CDevCmn*	m_pDevCtrl;
	CWavePlay	m_WavePlay;
	BOOL		m_bExit;
	BOOL		m_bPlay;
	CString		m_strCurrPlayList;
	int			m_language;

// [#RWC6-399] US ryan.payton 2022.10.03 DigitalMint
#if (SUPPORT_SOFTWARE_TTS && !UNDER_CE)
	ISpVoice*	pVoice;
#endif
// End of [#RWC6-399]

	CNHLinkedList< CString >	m_colWaveScrData[LANGUAGE_MODE_MAX+1];			// [#413] AIREAT 2008.09.05
	CNHLinkedList< CString >	m_SavecolWaveScrData[LANGUAGE_MODE_MAX+1];		// [#413] AIREAT 2008.09.05

	CNHEvent		m_eStartToPlay;
	CNHEvent		m_eDoneToPlay;
};

/////////////////////////////////////////////////////////////////////////////

#endif //__ADA_CTRL_H__
