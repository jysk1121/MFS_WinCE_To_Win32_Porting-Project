#pragma once

#include "DEV_CDM.h"
#include "DEV_CDR.h"
#include "DEV_Define.h"
#include "DEV_DIO.h"
#include "DEV_EPP.h"
#include "DEV_HOST.h"
#include "DEV_PRT.h"
#include "DEV_USBMEM.h"
#include "CrypteraComm.h"
#include "DEV_EJL.h"
#include "DEV_CAMERA.h"
#include "DEV_MUB.h"

#include "WavePlay.h"
#include "../../Template/MFSCtrlThread.h"

/** **********************************************************
*	@file		DEV_Manager.h
*	@brief		장치 관리자
************************************************************/
class CDEV_Manager : public CMFSCtrlThread
{
	// 인스턴스
	static CDEV_Manager* m_pInstance;

	// 생성자
	CDEV_Manager(void);
	// 소멸자
	~CDEV_Manager(void);

public:

	// 인스턴스 생성
	static void CreateInstance();

	// 인스턴스 취득
	static CDEV_Manager* GetInstance();

	// 인스턴스 해제
	static void ReleaseInstance();

	// 초기화
	BOOL Initialize();

	void GetAPPVersion();

	// 종결화
	BOOL Finalize();

	// 통지 윈도우 설정
	void SetNotifyWnd(CWnd* pNotifyWnd);

	//// 볼륨 제어
	static void	SetVolume(int nLevel);

	static void	SetVolumeUp();

	static void	SetVolumeDown();

	// 볼륨 상태 조회
	static int	GetVolume();

	// wave file 제어
	static void	ResetWaveFile();

	static void AddWaveFile(CString strFileName);

	static void AddNumberWaveFiles(int nNumber);

	static void AddAmountWaveFiles(CString strNumber, BOOL bUseDecimalPoint = FALSE);

	static void PlayWaveFile(BOOL bWaitUntilEnd = FALSE);

	static void StopPlayWaveFile();

	// wave file 동작 중 확인
	static BOOL IsPlayingWaveFile();

	// System Reboot (Software and Hardware)
	static void System_Reboot(BOOL bForced = FALSE);

	// AMS Remote Command - Save EJ
	static BOOL SaveEJL_SetTextValue(DWORD dwJnl_Code, CString strTextValue, BOOL bUseLocalTime = TRUE, CString strStackTime = _T(""));

	static BOOL	SaveEJL_SetChangeValue(CString strOld, CString strNew);		// AMS Setup정보 변경시에도 필요하여 Dev Class로 이동

	// Supervisor 진입 전 Terminal 정보 Save - 추후 구현
	static void Save_PreviousTerminalInfo();

	// Supervisor Exit시 Terminal 정보 변경시 Save - 추후 구현
	static void SaveEJL_SetChangedTerminalInfo(int nDest = 0);

	// Setting network
	static BOOL	Set_Network_Config(BOOL bReloadLanCard = TRUE);

	// Memory Check
	static void	CheckMemoryStatus();

	// CDM
	CDEV_CDM		m_DEV_CDM;

	// CDR
	CDEV_CDR		m_DEV_CDR;

	// DIO
	CDEV_DIO		m_DEV_DIO;

	// EPP
	CDEV_EPP		m_DEV_EPP;

	// PRINT
	CDEV_PRT		m_DEV_PRT;

	// Cryptera EPP
	CCrypteraComm	m_DEV_CREPP;

	// EJL
	CDEV_EJL		m_DEV_EJL;

	// Network
	CDEV_HOST		m_DEV_HOST;

	// Camera
	CDEV_CAMERA		m_DEV_CAMERA;

	// MUB(Multi UART Board)
	CDEV_MUB		m_DEV_MUB;

	int				m_nPrevServiceMode;

	CString			m_strAP_Version;

private:
	static CWavePlay		m_WavePlay;

	static CMFSEvent		m_eStartToPlay;

	static CMFSEvent		m_eDoneToPlay;

	static CMFSLinkedList< CString >	m_colWaveScrData[6+1];

	// Network Setting Thread - Network Setting시 간헐적 Holding 현상 대응
	HANDLE			m_hNetworkSettingThread;

	static UINT Thread_SetNetwork(LPVOID pParam);

	void Thread_SetNetwork(void);

protected:
	// 통지 윈도우
	CWnd* m_pNotifyWnd;

	unsigned ThreadHandlerProc(void);
};
