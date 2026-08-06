#include "stdafx.h"
#include <windows.h> 
#ifdef _WIN32_WCE
#include "captureframework.h"
#include <commctrl.h>
#include "./INC/enumdef.h"
#endif // _WIN32_WCE

#include "DEV_CAMERA.h"
#include "DeviceSimUtil.h"

#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleDataManager.h"
#include "../../EagleCE_Screen/EagleCE_Screen/SCR_Manager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT hr;
DWORD g_dwFilters = 0;
BOOL g_bCameraSelected = FALSE;
UINT32  g_NumofCaptureResolutions;
UINT32  g_NumofStillResolutions;
UINT8	g_CaptureResolutionIndex;
UINT8	g_StillResolutionIndex;

#ifdef _WIN32_WCE
AM_MEDIA_TYPE *g_CurrentVideoProfile;
AM_MEDIA_TYPE *g_CurrentStillProfile;

CAPTUREFRAMEWORK		m_CapFramework;
#else
// ---------------------------------------------------------------------------
// CAMERA SIMULATOR (Win32)
//
// There is no camera on a desktop PC and the DirectShow capture framework is a
// WinCE-only component, so every camera entry point below simply reports success.
// No dialog is raised: the camera is driven by screen transitions and the
// transaction flow, not by an operator action.
// ---------------------------------------------------------------------------
#endif // _WIN32_WCE

/**
*	@author		Si-Hwan, Park
*	@date		2017-10-30
*	@brief		CDEV_CAMERA 생성자
*/
CDEV_CAMERA::CDEV_CAMERA()
{
	m_hWnd		= NULL;

	m_dwLastError	= 0;

	m_bCameraOpened = FALSE;

	m_bIsCameraExist = FALSE;

	m_hStillCaptureThread = NULL;

	m_bExecStillCapture = FALSE;

	m_bSupervisorMode = FALSE;

	m_CaptureFileName.Empty();
	m_FlipCaptureFileName.Empty();

	memset(m_tchBuffer, 0, sizeof(m_tchBuffer));

	m_strLastError = _T("");

	m_hStillCaptureThread = NULL;
	m_hStillCaptureThread = AfxBeginThread((AFX_THREADPROC)StillCapture_Thread, this);

	LOG(Info, _T("[Open] Start Still Capture Thread"));

};

/**
*	@author		Si-Hwan, Park
*	@date		2017-10-30
*	@brief		CDEV_CAMERA 소멸자
*/
CDEV_CAMERA::~CDEV_CAMERA()
{
	m_bThreadExit = TRUE;

	// Thread를 먼저 종료 후 진행함.
	if (m_hStillCaptureThread != NULL)
	{
		CUtil::Sleep_Wait(1000);	// 종료를 위해 1초 Delay

		LOG(Info, _T("[Close] Exit Still Capture Thread"));

		CloseHandle(m_hStillCaptureThread);
		m_hStillCaptureThread = NULL;		
	}

#ifdef _WIN32_WCE
	 m_CapFramework.Cleanup();
#endif
}



void CDEV_CAMERA::SetHwnd(HWND hWnd)
{
	m_hWnd		= hWnd;
}


// Camera Driver Init
BOOL CDEV_CAMERA::InitCameraDriver()
{
#ifndef _WIN32_WCE
	// Win32 simulator: report a camera as present without touching DirectShow.
	m_bIsCameraExist = TRUE;
	return TRUE;
#else
	m_bIsCameraExist = FALSE;

	if (S_OK != m_CapFramework.InitCameraDriver())
	{
		m_strLastError.Format(_T("I00001"));
		LOG(Error, _T("[Open] InitCameraDriver Error"));
		return FALSE;
	}

	TCHAR **ptszCameraDrivers;
	int nDriverCount = 0;

	// Open후에 Camera 초기화 수행해 놓음
	if( FAILED(m_CapFramework.GetDriverList(&ptszCameraDrivers, &nDriverCount)) )
	{
		//Get Driver List FAIL
		m_strLastError.Format(_T("I00001"));
		LOG(Error, _T("[Open] GetDriverList Error"));
		return FALSE;
	}

	hr = m_CapFramework.SelectCameraDevice(0);
	if (FAILED(hr))
	{
		LOG(Error, _T("Camera device select is failed (%x)"), hr);
		return FALSE;
	}

	m_bIsCameraExist = TRUE;

	return TRUE;
#endif // _WIN32_WCE
}


/**
*	@author		Si-Hwan, Park
*	@date		2017-10-30
*	@brief		CAMERA Open
*	@param		HWND hWnd	camera preview 가 open 될 핸들
*	@return		TRUE	open 성공
*	@return		FALSE	open 실패
*/
BOOL CDEV_CAMERA::Open()
{
#ifndef _WIN32_WCE
	// Camera open is simulated: always succeeds.
	return TRUE;
#else

	if (m_hWnd == NULL)
	{
		LOG(Error, _T("[Open] Hwnd is null"));
		return FALSE;
	}

	if (m_bCameraOpened == TRUE)
	{
		LOG(Info, _T("[Open] Camera is already opened"));
		return TRUE;
	}

	//Set Filters
	g_dwFilters = VIDEO_CAPTURE_FILTER | VIDEO_RENDERER | STILL_IMAGE_SINK | FILE_WRITER | VIDEO_ENCODER ;

	// 초기 camera initial시의 rc값은 없으므로 설정
	RECT rcCoordinates = { 0, 0, 0, 0};

	if( FAILED(m_CapFramework.Init(m_hWnd, &rcCoordinates, g_dwFilters )))
	{
		// Init failed
		m_strLastError.Format(_T("I00002"));
		LOG(Error, _T("[Open] Init Error"));
		return FALSE;
	}

	m_bCameraOpened = TRUE;

	g_bCameraSelected = TRUE;

	m_bExecStillCapture = FALSE;

	m_bThreadExit = FALSE;

	LOG(Info, _T("Camera Open Success"));

	return TRUE;
#endif // _WIN32_WCE
}

/**
*	@author		Si-Hwan, Park
*	@date		2017-10-30
*	@brief		CAMERA close
*	@return		TRUE	Close 성공
*	@return		FALSE	Close 실패
*/
BOOL CDEV_CAMERA::Close()
{
#ifndef _WIN32_WCE
	// Camera close is simulated: always succeeds.
	return TRUE;
#else

	m_CapFramework.Cleanup();

	g_bCameraSelected = FALSE;

	m_bCameraOpened = FALSE;

	LOG(Info, _T("Camera Close Success"));

	return TRUE;
#endif // _WIN32_WCE
}


/** **********************************************************
*	@brief		Camera Open 상태 확인
*	@retval		없음
************************************************************/
BOOL CDEV_CAMERA::IsOpened()
{
	return m_bCameraOpened;
}


/** **********************************************************
*	@brief		카메라 드라이버로 존재 유무 판단
*	@retval		TRUE: 존재, FALSE: 미존재
************************************************************/
BOOL CDEV_CAMERA::IsExistCameraDriver()
{
	// Driver Init은 AP 최초 기동시에만 수행 (중간에 Camera 제거시에는 미감지하도록 사양 정함 - Driver쪽 Load 최소화)
	//return m_CapFramework.IsExistCameraDriver();

#if !(USE_CAMERA_DEVICE)	// Camera 미 사용시 미존재로 처리
	return FALSE;
#endif

	return m_bIsCameraExist;
}


BOOL CDEV_CAMERA::SetUseCamera(BOOL bUse)
{
	if (bUse == TRUE)
		CEagleDataManager::GetInstance()->m_Config.m_Device.strCAM_Enable = S_ENABLE;
	else
		CEagleDataManager::GetInstance()->m_Config.m_Device.strCAM_Enable = S_DISABLE;

	CEagleDataManager::GetInstance()->m_Config.SaveDeviceData(_T("CAM"), CEagleDataManager::GetInstance()->m_Config.m_Device.strCAM_Enable);

	return TRUE;
}


BOOL CDEV_CAMERA::GetCameraSetting()
{
	if (CEagleDataManager::GetInstance()->m_Config.m_Device.strCAM_Enable == S_ENABLE)
		return TRUE;

	return FALSE;
}


/** **********************************************************
*	@brief		Camera Window Position setting
*	@retval		없음
************************************************************/
BOOL CDEV_CAMERA::SetVideoWindosPos(RECT *rc)
{
#ifdef _WIN32_WCE
	hr = m_CapFramework.SetVideoWindowPosition(rc);
	if (FAILED(hr))
	{
		LOG(Error, _T("Window Position Setting Error (%x)"), hr);
		return FALSE;
	}
#endif // _WIN32_WCE

	return TRUE;
}


/**
*	@author		Si-Hwan, Park
*	@date		2017-10-30
*	@brief		Preview 시작
*	@param		RECT *rcCoordinates	핸들의 좌표값 ex) RECT rc={0,0,0,0};
*	@return		TRUE	Preview 성공
*	@return		FALSE	Preview 실패
*/
BOOL CDEV_CAMERA::RunGraph()
{
#ifndef _WIN32_WCE
	// Preview graph start is simulated: always succeeds.
	return TRUE;
#else

	if (g_bCameraSelected == FALSE)
	{
		LOG(Info, _T("Camera is not selected"));
		return FALSE;
	}

	if(FAILED(m_CapFramework.RunGraph()))
	{
		//RETAILMSG(1, ( TEXT("CameraDShowApp: Starting the capture graph failed.")));
		m_strLastError.Format(_T("I00003"));	// Vidro preview run fail.

		return FALSE;
	}

	LOG(Info, _T("Camera is Running OK"));

	return TRUE;
#endif // _WIN32_WCE
}


/**
*	@author		Si-Hwan, Park
*	@date		2017-10-30
*	@brief		Preview 종료
*	@return		TRUE	Preview 종료 성공
*	@return		FALSE	Preview 종료 실패
*	@remark		해당 함수를 사용하면 Preview 가 종료 되면서 화면이 사라진다. \n
만약 window 와 같이 일시정지와 같은것으로 하려면 수정 필요.
*/
BOOL CDEV_CAMERA::StopGraph(BOOL bErrorClear/* =TRUE */)
{
#ifndef _WIN32_WCE
	// Preview graph stop is simulated: always succeeds.
	return TRUE;
#else

	if (g_bCameraSelected == FALSE)
	{
		LOG(Info, _T("Camera is not selected"));
		return FALSE;
	}

	// Phase 명령 수행 후 Stop 수행
	hr = m_CapFramework.PauseGraph();

	if(FAILED(hr))
	{
		RETAILMSG(1, (_T("PauseGraph is failed\n")));

		m_strLastError.Format(_T("I00004"));	// Video stop is failed.
		LOG(Error, _T("PauseGraph is failed (%x)"), hr);

		return FALSE;
	}

	LOG(Info, _T("PauseGraph SUCCESS"));

// StopGraph시 해당 함수 Holding 현상으로 인해 Pause만 수행하도록 변경
/*
	hr = m_CapFramework.StopGraph();

	if (FAILED(hr))
	{
		m_strLastError.Format(_T("I00004"));	// Video stop is failed.
		LOG(Error, _T("StopGrah is failed (%x)"), hr);

		return FALSE;
	}
	LOG(Info, _T("StopGrah is OK"));
*/

	if(TRUE == bErrorClear)
		m_strLastError.Format(_T(""));

	return TRUE;
#endif // _WIN32_WCE
}


BOOL CDEV_CAMERA::StillCapture(BOOL bMode, BOOL bAsync)
{
#ifndef _WIN32_WCE
	// Still capture is simulated: always reports a successful capture.
	return TRUE;
#else

	DWORD startTime,endTime;
	BOOL bRet = FALSE;

	CFile datFile;
	CFileException e;
	int nFileSize = 0;
	CString strFileName, strFlipFileName;

	if (g_bCameraSelected == FALSE)	// Camera Open 실패시 FALSE 처리
	{
		LOG(Error, _T("[StillCapture] Camera is not selected"));
		return FALSE;
	}

	if (bAsync == TRUE)
	{
		if (m_bExecStillCapture == TRUE)
		{
			// 현재 Capture가 진행 중이므로 임시사양은 Skip 처리
			LOG(Info, _T("Now is processing still capture image - return FALSE"));
			return FALSE;
		}

		LOG(Info, _T("StillCaputure flag is set (%d)"), bMode);

		m_bExecStillCapture = TRUE;
		
		if (bMode == FALSE)
			m_bSupervisorMode = TRUE;
		else
			m_bSupervisorMode = FALSE;

		return TRUE;
	}

	// SuperVisor Mode 초기화
	m_bSupervisorMode = FALSE;

	if (bMode == FALSE)	// Supervisor Mode
	{
		LOG(Info, _T("StillCaputure start - Supervisor Mode"));
		strFileName.Format(_T("%s\\%s"), CAPTURE_IMAGE_FILE_PATH_ABS, SUPERVIOSR_CAPTURE_TEMP_IMAGE_NAME);
	}
	else				// Transaction Mode
	{
		LOG(Info, _T("StillCaputure start - Transaction Mode"));
		strFileName.Format(_T("%s\\%s"), CAPTURE_IMAGE_FILE_PATH_ABS, CLIENT_CAPTURE_TEMP_IMAGE);
	}

	memset(m_tchBuffer, 0, sizeof(m_tchBuffer));
	wcscpy(m_tchBuffer, strFileName);
	m_CapFramework.SetStillCaptureFileName(m_tchBuffer);

	for(int i=0; i<10; i++)		// Retry 10회 수행
	{
		startTime = GetTickCount();

		hr = m_CapFramework.CaptureStillImage();

		endTime = GetTickCount();

		if(FAILED(hr))
		{
			m_strLastError.Format(_T("I00005"));	// Image Capture is failed.
			LOG(Info, _T("Camera Capture Fail - I00005 (%x) - TickCount : %d"), hr, (endTime - startTime));
			bRet = FALSE;
			break;	// 실패시 retry 안함
		}
		else
		{
			m_strLastError.Format(_T(""));
			LOG(Info, _T("Camera Capture OK - TickCount : %d"), (endTime - startTime));
			bRet = TRUE;

			// Image File Validation 수행 (Green Image 처리)
			// 1. 현재는 File Size가 잘못된 경우 10KB 미만이므로 Size로 Check - Max 10번 Try
			// 2. Green Image인 경우 File Image가 일정한 값으로 채워지므로 해당 값으로 Validation을 Check하도록 로직 수정 필요
			if(datFile.Open(strFileName, CFile::modeRead, &e))
			{
				nFileSize = (int)datFile.GetLength();

				datFile.Close();

				if(nFileSize > (1024*10))	// 10KB 미만시 Image File이 잘못된 것으로 판단하여 다시 시도
				{
					// File Validation is OK
					LOG(Info, _T("StllCapture Image is OK - Count : %d, Size :%dKB"), i+1, (nFileSize / 1024));
					break;					
				}
				else
				{
					LOG(Info, _T("StllCapture Image is Invalid - Count : %d, Size :%dKB"), i+1, (nFileSize / 1024));
					// File Delete
					::DeleteFile(strFileName);
				}
			}
		}

		CUtil::Sleep_Wait(100);
	}

	if (bRet == TRUE)
	{
		BOOL bResult = FALSE;
		
		if (bMode == FALSE)
			strFlipFileName.Format(_T("%s\\%s"), CAPTURE_IMAGE_FILE_PATH_ABS, SUPERVISOR_CAPTURE_IMAGE_NAME);
		else
			strFlipFileName.Format(_T("%s\\%s"), CAPTURE_IMAGE_FILE_PATH_ABS, CLIENT_CAPTURE_IMAGE);

		// CS130은 화면을 180도 회전 처리하여 상하 반전 처리를 해야 하나 MF200은 처리하면 안됨.
		if (GetSystemMetrics(SM_CXSCREEN) == CS130_WIDTH_RESOLUTION)
		{
			bResult = CSCR_Manager::GetInstance()->FlipImage(strFileName, strFlipFileName);	// CS130은 상하 반전해서 저장 처리
			LOG(Info, _T("Flip Image result : (%d)"), bResult);
		}
		else
		{
			bResult = ::CopyFile(strFileName, strFlipFileName, FALSE);
			LOG(Info, _T("Copy Image result : (%d)"), bResult);
			::DeleteFile(strFileName);
		}


		// Check Flip File Size
		//nFileSize = 0;
		//if (bFlipSuccess == TRUE)
		//{
		//	if(datFile.Open(strFlipFileName, CFile::modeRead, &e))
		//	{
		//		nFileSize = (int)datFile.GetLength();

		//		datFile.Close();
		//	}
		//}

		//LOG(Info, _T("FlipImage result : (%d), File Size (%dKB)"), bFlipSuccess, (nFileSize/1024));
	}

	return bRet;
#endif // _WIN32_WCE
}


/** **********************************************************
*	@brief		Camera 동작 중 상태 확인
*	@retval		TRUE : 동작중, FALSE : Idle 상태
************************************************************/
BOOL CDEV_CAMERA::IsCameraProcessing()
{
	return m_bExecStillCapture;
}


/** **********************************************************
*	@brief		버전 취득
*	@retval		버전정보
************************************************************/
CString CDEV_CAMERA::GetVersion()
{
	CString strVersion;

	strVersion = _T("V01.01");

	return strVersion;
}


/** **********************************************************
*	@brief		에러코드 취득
*	@retval		에러코드
************************************************************/
CString CDEV_CAMERA::GetLastError()
{
	return m_strLastError;
}


/** **********************************************************
*	@brief		Thread
*	@retval		0
************************************************************/
UINT CDEV_CAMERA::StillCapture_Thread(LPVOID pParam)
{

	CDEV_CAMERA* pCAMERA = (CDEV_CAMERA*)pParam;

	pCAMERA->StillCapture_Thread();

	return 0;
}


/** **********************************************************
*	@brief		거래 중 Camera Image Capture Thread
*	@retval		없음
************************************************************/
void CDEV_CAMERA::StillCapture_Thread()
{
	BOOL bResult = FALSE;

	while (m_bThreadExit == FALSE)
	{
		if (m_bExecStillCapture == TRUE)
		{		
			LOG(Info, _T("[Async] Still Capture Start"));

			m_strLastError.Empty();

			if (m_bSupervisorMode == FALSE)
				bResult = RunGraph();
			else
				bResult = TRUE;

			if (bResult == TRUE)
			{
				// Run 후에 바로 Still Capture 수행시 Error 발생하여 Delay 1초 추가함
				if (m_bSupervisorMode == FALSE)
					CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);

				if (m_bSupervisorMode == FALSE)
					bResult = StillCapture(TRUE, FALSE);
				else
					bResult = StillCapture(FALSE, FALSE);

				if (bResult == FALSE)
				{
					LOG(Error, _T("[Async] StillCapture Error - %s"), m_strLastError);		
				}

				//CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL_1000MS);
				
				// StopGraph 수행
				if (StopGraph(TRUE) == FALSE)
				{
					LOG(Error, _T("[Async] Stopgraph Error - %s"), m_strLastError);
				}
			}
			else
			{
				LOG(Error, _T("[Async] RunGraph Error - %s"), m_strLastError);
			}

			LOG(Info, _T("[Async] Still Capture End"));

			// Capture 완료 후 Flag 초기화
			m_bExecStillCapture = FALSE;
			m_bSupervisorMode = FALSE;
		}

		CUtil::Sleep_Wait(EAGLE_SLEEP_INTERVAL);
	}
}



/**
*	@author		Si-Hwan, Park
*	@date		2017-10-30
*	@brief		Video Capture 시작
*	@param		TCHAR *CaptureFileName	Video File name (xxxx.asf)
*	@return		TRUE	Capture 시작 성공
*	@return		FALSE	Capture 시작 실패
*	@remark		추가 필요
*/
// BOOL CDEV_CAMERA::StartVideoCapture(TCHAR *CaptureFileName)
// {
// 	/*
// 	m_CapFramework.SetVideoCaptureFileName(CaptureFileName);
// 			{
// 		//		RETAILMSG(1, ( TEXT("CameraDShowApp: Failed to set the video capture file name.")));
// 				//return FALSE;
// 			}
// 			*/
// 	return TRUE;
// }

/**
*	@author		Si-Hwan, Park
*	@date		2017-10-30
*	@brief		Video Capture 종료
*	@return		TRUE	Capture 종료 성공
*	@return		FALSE	Capture 종료 실패
*	@remark		추가 필요
*/
// BOOL CDEV_CAMERA::StopVideoCapture()
// {
// return TRUE;
// }

/**
*	@author		Si-Hwan, Park
*	@date		2017-10-30
*	@brief		Still Capture 
*	@param		TCHAR *StillFileName	jpg File name (xxxx.jpg or \xxx\xxx.jpg)
*	@return		TRUE	Capture 성공
*	@return		FALSE	Capture 실패
*	@remark		추가 필요
*/


//
//  FUNCTION: GetCurrentStillResolution
//
//  PURPOSE: Get current Still Resolution
//
//  COMMENTS:
//
//HRESULT CDEV_CAMERA::GetCurrentStillResolution()
//{
//	HRESULT hr;
//	AM_MEDIA_TYPE *pmtConfig;
//	VIDEOINFOHEADER *pVih;
//	CAMFORMATPROS currentCapResolution;
//	VIDEO_STREAM_CONFIG_CAPS scc;
//	hr = m_CapFramework.GetFormat(STREAM_STILL,&pmtConfig);
//
//	if (SUCCEEDED(hr))
//	{
//		pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
//		currentCapResolution.width = pVih->bmiHeader.biWidth;
//		currentCapResolution.height = pVih->bmiHeader.biHeight;
//		//RETAILMSG(1,(TEXT("Video Width = %d,Height = %d\r\n"),pVih->bmiHeader.biWidth,abs(pVih->bmiHeader.biHeight)));
//		LOG(Info, _T("(GetCurrentStillResolution) Video Width = %d,Height = %d"), pVih->bmiHeader.biWidth,abs(pVih->bmiHeader.biHeight));
//	}
//
//	int iCount = 0, iSize = 0;
//	for (int iFormat = 0; iFormat < iCount; iFormat++)
//	{
//			hr = m_CapFramework.GetStreamCaps(STREAM_STILL,iFormat, &pmtConfig, (BYTE*)&scc);
//			if (SUCCEEDED(hr))
//			{
//				// Examine the format. 			 
//			    // since our camera don't support cropping, we use the original output
//				pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;	
//
//				LOG(Info, _T("pVih->bmiHeader.biWidth = %lu ***********"), pVih->bmiHeader.biWidth);
//
//				if (pVih->bmiHeader.biWidth == currentCapResolution.width && pVih->bmiHeader.biHeight == currentCapResolution.height)
//				{
//					g_StillResolutionIndex = iFormat;													
//				}
//			}
//
//	}
//	return hr;
//}

//
//  FUNCTION: GetCurrentVideoResolution
//
//  PURPOSE: Get Current Video Resolution
//
//  COMMENTS:
//
//HRESULT CDEV_CAMERA::GetCurrentVideoResolution()
//{
//	HRESULT hr;
//	AM_MEDIA_TYPE *pmtConfig;
//	VIDEOINFOHEADER *pVih;
//	CAMFORMATPROS curCapResolution;
//	VIDEO_STREAM_CONFIG_CAPS scc;
//	hr = m_CapFramework.GetFormat(STREAM_CAPTURE,&pmtConfig);
//
//	if (SUCCEEDED(hr))
//	{
//		pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;								
//		curCapResolution.width = pVih->bmiHeader.biWidth;
//		curCapResolution.height = pVih->bmiHeader.biHeight;
//		curCapResolution.subType = pmtConfig->subtype;
//
//		//RETAILMSG(1,(TEXT("Video Width = %d,Height = %d\r\n"),pVih->bmiHeader.biWidth,abs(pVih->bmiHeader.biHeight)));
//		LOG(Info, _T("(GetCurrentVideoResolution) Video Width = %d,Height = %d"), pVih->bmiHeader.biWidth,abs(pVih->bmiHeader.biHeight));
//	}
//
//	int iCount = 0, iSize = 0;
//
//	for (int iFormat = 0; iFormat < iCount; iFormat++)
//	{
//			hr = m_CapFramework.GetStreamCaps(STREAM_CAPTURE,iFormat, &pmtConfig, (BYTE*)&scc);
//			if (SUCCEEDED(hr))
//			{
//				// Examine the format. 			 
//			    // since our camera don't support cropping, we use the original output
//				pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;	
//
//				//RETAILMSG(1,(TEXT("pVih->bmiHeader.biWidth = %lu***********\r\n"),pVih->bmiHeader.biWidth));
//				LOG(Info, _T("pVih->bmiHeader.biWidth = %lu***********"), pVih->bmiHeader.biWidth);
//
//				if (pVih->bmiHeader.biWidth == curCapResolution.width && pVih->bmiHeader.biHeight == curCapResolution.height)
//				{
//					g_CaptureResolutionIndex = iFormat;													
//				}
//			}
//
//	}
//	return hr;
//}


//
//  FUNCTION: GetVideoProfile
//
//  PURPOSE: Get current Video Profile
//
//  COMMENTS:
//
// HRESULT CDEV_CAMERA::GetVideoProfile(int nStream,UINT8 nCurrentRes)
// {
// 	HRESULT hr;
// 	//RETAILMSG(1,(TEXT("GetVideoProfile nCurrentRes = %d \r\n"),nCurrentRes));
// 	VIDEO_STREAM_CONFIG_CAPS scc;
// 	AM_MEDIA_TYPE *pmtConfig;
// 	VIDEOINFOHEADER *pVih;
// 	hr = m_CapFramework.GetStreamCaps(nStream,nCurrentRes, &pmtConfig, (BYTE*)&scc);
// 	if (SUCCEEDED(hr))
// 	{
// 		// Examine the format. 			 
// 	    // since our camera don't support cropping, we use the original output
// 		pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;	
// 		//RETAILMSG(1,(TEXT("pVih->bmiHeader.biWidth = %lu***********\r\n"),pVih->bmiHeader.biWidth));
// 		
// 			//RETAILMSG(1,(TEXT("GetStreamCaps = %lu\r\n"),pVih->bmiHeader.biWidth));
// 			if(nStream == STREAM_CAPTURE)
// 			{
// 				m_CapFramework.GetStreamCaps(nStream,nCurrentRes,&g_CurrentVideoProfile, (BYTE*)&scc);
// 			}
// 			if(nStream == STREAM_STILL)
// 			{
// 				m_CapFramework.GetStreamCaps(nStream,nCurrentRes,&g_CurrentStillProfile, (BYTE*)&scc);
// 			}
// 											
// 		
// 	}
// 	
// 	//RETAILMSG(1,(TEXT("ChangeCurrentResolution DOne\r\n")));
// 	return hr;
// }