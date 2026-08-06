#include "stdafx.h"
#include "DEV_USBMEM.h"
#include "../../Eagle_Framework/Eagle_Framework/Util.h"
#include <Dbt.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/************************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CDEV_USBMEM::CDEV_USBMEM(void)
{
	// 통지 윈도우
	m_hNotifyWnd = NULL;

	// 연결
	m_bConnected = FALSE;

	// 드라이브 문자
	m_tDriveLetter = NULL/*_T('')*/;
}


/************************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CDEV_USBMEM::~CDEV_USBMEM(void)
{
}


/************************************************************
*	@brief		열기
*	@retval		없음
************************************************************/
BOOL CDEV_USBMEM::Open(HWND hWnd)
{
	m_hNotifyWnd = hWnd;

	CString strDriveLetter = CUtil::GetRemovableDriveLetter();

	if (strDriveLetter.IsEmpty())
	{
		m_bConnected = FALSE;

		m_tDriveLetter = NULL/*_T('')*/;
	}
	else
	{
		m_bConnected = TRUE;

		m_tDriveLetter = strDriveLetter[0];
	}

	return TRUE;
}


/************************************************************
*	@brief		닫기
*	@retval		없음
************************************************************/
BOOL CDEV_USBMEM::Close(void)
{
	return TRUE;
}


/************************************************************
*	@brief		연결 확인
*	@retval		없음
************************************************************/
BOOL CDEV_USBMEM::IsConnected()
{
	return m_bConnected;
}


/************************************************************
*	@brief		드라이브 문자 취득
*	@retval		없음
************************************************************/
TCHAR CDEV_USBMEM::GetDriveLetter()
{
	return m_tDriveLetter;
}


/************************************************************
*	@brief		디바이스 변경
*	@retval		없음
************************************************************/
BOOL CDEV_USBMEM::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	// 디바이스 연결
	if (DBT_DEVICEARRIVAL == nEventType)
	{
		CString strDriveLetter = CUtil::GetRemovableDriveLetter();

		if (strDriveLetter.IsEmpty())
		{
			m_bConnected = FALSE;

			m_tDriveLetter = NULL/*_T('')*/;
		}
		else
		{
			m_bConnected = TRUE;

			m_tDriveLetter = strDriveLetter[0];
		}

		CWnd::FromHandle(m_hNotifyWnd)->PostMessage(WM_USBMEM_NOTIFY, m_bConnected, m_tDriveLetter);
	}

	// 디바이스 해제
	else if (DBT_DEVICEREMOVECOMPLETE == nEventType)
	{
		m_bConnected = FALSE;

		m_tDriveLetter = NULL/*_T('')*/;

		CWnd::FromHandle(m_hNotifyWnd)->PostMessage(WM_USBMEM_NOTIFY, m_bConnected, m_tDriveLetter);
	}

	return TRUE;
}
