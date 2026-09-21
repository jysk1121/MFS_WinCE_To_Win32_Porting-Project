#include "stdafx.h"
#include <stdio.h>
#include <windows.h>

#ifdef UNDER_CE

#include "huc_api.h"
#include ".\common\common.h"
#include "mcscore.h"

#include "..\..\H\Common\CmnLib.h"
#include "..\..\H\Common\NHDbgApi.h"


#define MAX_RETRY_CNT_USB_CONNECT	(100)

#define DEBUGMASK(bit)      (1 << (bit))

#define MASK_ERROR          DEBUGMASK(0)
#define MASK_WARN           DEBUGMASK(1)
#define MASK_INIT           DEBUGMASK(2)
#define MASK_BATTERY        DEBUGMASK(3)
#define MASK_IOCTL          DEBUGMASK(4)
#define MASK_HARDWARE       DEBUGMASK(5)
#define MASK_RESUME         DEBUGMASK(6)
#define MASK_FUNCTION       DEBUGMASK(7)
#define MASK_PDD            DEBUGMASK(8)

DBGPARAM dpCurSettings = {
    _T("callhuc"), 
    {
        _T("Errors"), _T("Warnings"), _T("Init"), _T("Battery"), 
        _T("Ioctl"), _T("Hardware"), _T("Resume"), _T("Function"),
        _T("PDD"),_T(""),_T(""),_T(""),
        _T(""),_T(""),_T(""),_T("") 
    },
    MASK_ERROR | MASK_WARN
}; 

HANDLE g_hMainCam = (HANDLE)-1;
HANDLE g_hSubCam = (HANDLE)-1;

HANDLE g_MainCamThreadHandle  = (HANDLE)-1;
HANDLE g_SubCamThreadHandle  = (HANDLE)-1;

unsigned char g_MainCam_Image[MAX_FRAME_BUFFER_SIZE];
unsigned char g_SubCam_Image[MAX_FRAME_BUFFER_SIZE];

bool g_bExitMainCamThread = false;
bool g_bExitSubCamThread = false;
int g_DispX;
int g_DispY;


HANDLE OpenCam(int index)
{
	WCHAR DeviceName[MAX_PATH] = {0,};
	HANDLE hCam = (HANDLE)-1;

	wsprintf(DeviceName, L"HUC%d:", index);
	hCam = CreateFile(DeviceName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	return hCam;
}

void CloseCam(HANDLE hCam)
{
	CloseHandle( hCam );
}

DWORD GetCamPID(HANDLE hCam)
{
	DWORD PID = 0; // PID의 값을 통해서 Master와 Sub 카메라를 구분합니다
	DWORD dwRet = 0;
	BOOL bRet = FALSE;

	bRet = DeviceIoControl(hCam, IOCTL_HUC_GET_PID, &PID, sizeof(DWORD), NULL, 0, &dwRet, 0 );		
	if( bRet == FALSE )
		return MCS_ERROR_NO_USB_CAM;

	return PID;
}

DWORD ClearCamBuffer(HANDLE hCam)
{
	DWORD dwRet = 0;
	BOOL bRet = FALSE;

	bRet = DeviceIoControl(hCam, IOCTL_HUC_CLEAR_QUEUE, NULL, 0, NULL, 0, &dwRet, 0 );
	if( bRet == FALSE )
		return MCS_ERROR_NO_USB_CAM;
	return 1;
}

DWORD InternalCamStart(HANDLE hCam)
{
	DWORD dwRet = 0;
	BOOL bRet = FALSE;

	bRet = DeviceIoControl(hCam, IOCTL_HUC_CAM_START, NULL, 0, NULL, 0, &dwRet, 0 );
	if( bRet == FALSE )
		return MCS_ERROR_NO_USB_CAM;
	return 1;
}

DWORD InternalCamStop(HANDLE hCam)
{
	DWORD dwRet = 0;
	BOOL bRet = FALSE;

	bRet = DeviceIoControl(hCam, IOCTL_HUC_CAM_STOP, NULL, 0, NULL, 0, &dwRet, 0 );
	if( bRet == FALSE )
		return MCS_ERROR_NO_USB_CAM;
	return 1;
}

//------- new released by birdstar
// 메인카메라와 서브카메라의 ID가 정해져있기때문에, 아래와 같이 Open하는 새로운 함수를 제공합니다
// Close작업은 기존과 동일합니다
HANDLE OpenCam_MainCamera()
{
	DWORD PID; // PID의 값을 통해서 Main와 Sub 카메라를 구분합니다
	int Count = 0;
	HANDLE hDevice = (HANDLE)-1;

	for (Count = 0; Count < MAX_CAMERA_COUNT; Count++)
	{
		hDevice = OpenCam(Count + 1);
		if (hDevice != (HANDLE)-1)
		{
			// Main Camara를 찾기 위해서 아래와 같은 방법을 사용합니다
			PID = GetCamPID(hDevice);
			if (PID == PID_MAIN_CAMERA)
			{
				break;
			}
			CloseCam(hDevice);
			hDevice = (HANDLE)-1;
		}
	}
	return hDevice;
}

HANDLE OpenCam_SubCamera()
{
	DWORD PID; // PID의 값을 통해서 Main와 Sub 카메라를 구분합니다
	int Count = 0;
	HANDLE hDevice = (HANDLE)-1;

	for (Count = 0; Count < MAX_CAMERA_COUNT; Count++)
	{
		hDevice = OpenCam(Count + 1);
		if (hDevice != (HANDLE)-1)
		{
			// Main Camara를 찾기 위해서 아래와 같은 방법을 사용합니다
			PID = GetCamPID(hDevice);
			if (PID != PID_MAIN_CAMERA)
			{
				break;
			}
			CloseCam(hDevice);
			hDevice = (HANDLE)-1;
		}
	}
	return hDevice;
}

DWORD GetCamImage(HANDLE hCam, void * pBuffer, int RequestSize)
{
	DWORD dwRet = TRUE;
	BOOL bRet = FALSE;

	bRet = ReadFile(hCam, pBuffer, RequestSize, &dwRet, 0);

	if(bRet == FALSE)
	{
		MCSDP("%s %d MCS_ERROR_NO_USB_CAM  \r\n", __FUNCTION__, __LINE__);	
		return MCS_ERROR_NO_USB_CAM;
	}

	return dwRet;
}

DWORD WINAPI MainCamThread(__in LPVOID lpParameter)
{
	DWORD ImageSize = 0;
	int	retry_cnt_usb_connect = 0;

	NKDbgPrintfW(L"MainCamThread!\r\n");

	// Main 카메라를 열어봅니다
	g_hMainCam = OpenCam_MainCamera();
	if (g_hMainCam == (HANDLE)-1)
	{
		// 카메라가 발견되지 않습니다
		NKDbgPrintfW(L"NO Main PID!\r\n");
		return MCS_ERR_NO_CAM;
	}

	ClearCamBuffer(g_hMainCam);
 
	do
	{
		ImageSize = GetCamImage(g_hMainCam, g_MainCam_Image, MAX_FRAME_BUFFER_SIZE);

		switch (ImageSize)
		{
		case MAX_FRAME_BUFFER_SIZE:
			MCSDDRAW_VideoRenderP(g_MainCam_Image);

			break;

		case MCS_ERROR_NO_USB_CAM:
			MCSDP("MAIN_CAM %s %d MCS_ERROR_NO_USB_CAM\r\n", __FUNCTION__, __LINE__);	

			g_hMainCam = (HANDLE)-1;
			Delay_Msg(1000);

			++retry_cnt_usb_connect;
			g_hMainCam = OpenCam_MainCamera();
			if (g_hMainCam == (HANDLE)-1)
			{
				// 카메라가 발견되지 않습니다
				NHDBG((L"NO MAIN_CAM PID : %d!\r\n", retry_cnt_usb_connect));
			}
			else
			{
				NHDBG((L"FOUND MAIN_CAM PID : %d!\r\n",retry_cnt_usb_connect));
				retry_cnt_usb_connect = 0;
				ClearCamBuffer(g_hMainCam);
				CamStartPerson();			//+++ DY Moh 2018-02-20
			}

			break;

		default:
			if (ImageSize > 0)
				MCSDP("MAIN_CAM %s %d length:%d\r\n", __FUNCTION__, __LINE__, ImageSize);	

			break;
		}

		Delay_Msg(30);

	} while(g_bExitMainCamThread == FALSE);

	return TRUE;
}

DWORD WINAPI SubCamThread(__in LPVOID lpParameter)
{
	DWORD ImageSize = 0;
	int	retry_cnt_usb_connect = 0;

	NKDbgPrintfW(L"SubCamThread!\r\n");

	// 서브 카메라를 열어봅니다
	g_hSubCam = OpenCam_SubCamera();
	if (g_hSubCam == (HANDLE)-1)
	{
		// 카메라가 발견되지 않습니다
		NKDbgPrintfW(L"NO Sub PID!\r\n");
		return MCS_ERR_NO_CAM;
	}

	ClearCamBuffer(g_hSubCam);

	do
	{
		ImageSize = GetCamImage(g_hSubCam, g_SubCam_Image, MAX_FRAME_BUFFER_SIZE);
		if( ImageSize == MAX_FRAME_BUFFER_SIZE )
		{
			MCSDDRAW_VideoRenderE(g_SubCam_Image);
		}
		else if(ImageSize == MCS_ERROR_NO_USB_CAM)
		{
			MCSDP("SUB_CAM %s %d MCS_ERROR_NO_USB_CAM\r\n", __FUNCTION__, __LINE__);	
			
			g_hSubCam = (HANDLE)-1;
			
			Delay_Msg(1000);

			++retry_cnt_usb_connect;
			g_hSubCam = OpenCam_SubCamera();
			if (g_hSubCam == (HANDLE)-1)
			{
				// 카메라가 발견되지 않습니다
				NHDBG((L"NO SUB_CAM PID : %d!\r\n", retry_cnt_usb_connect));
			}
			else
			{
				retry_cnt_usb_connect = 0;
				ClearCamBuffer(g_hSubCam);
				NHDBG((L"FOUND SUB_CAM PID : %d!\r\n",retry_cnt_usb_connect));
				CamStartExitSlot();			//+++ DY Moh 2018-02-20
			}
		}
		else
		{
			MCSDP("SUB_CAM %s %d length:%d\r\n", __FUNCTION__, __LINE__, ImageSize);	
		}

		Delay_Msg(30);

	} while(g_bExitSubCamThread == FALSE);

	return MCS_SUCESS;
}


HJAPI int MCSCORE_SetHucConfig(MCS_Config	* pMcsConfig)
{
	int Ret = MCS_SUCESS;

	NKDbgPrintfW(L"Read_SetHucConfig_INI!\r\n");
	
	Ret = MCSCORE_Set_Config(pMcsConfig);

	return Ret;
}


HJAPI int MCSCORE_InitThreads()
{
	int Ret = MCS_SUCESS;

	DWORD MainCamThreadID;
	//DWORD SubCamThreadID;

	g_bExitMainCamThread = FALSE;
	g_bExitSubCamThread = FALSE;

	g_DispX = GetSystemMetrics(SM_CXSCREEN);
	g_DispY = GetSystemMetrics(SM_CYSCREEN);

	NHDBG((L"MCSCORE_InitThreads DispX:%d DispY:%d!\r\n", g_DispX, g_DispY));

	MCSCORE_Init();

	g_MainCamThreadHandle = CreateThread(0, 0, MainCamThread, 0, 0, &MainCamThreadID);

	// 2018.03.22 disable sub camera thread, which is not activated yet.
	//g_SubCamThreadHandle = CreateThread(0, 0, SubCamThread, 0, 0, &SubCamThreadID);

	return Ret;
}

HJAPI int MCSCORE_DeinitThreads()
{
	int Ret = MCS_SUCESS;
	
	g_bExitMainCamThread = TRUE;
	g_bExitSubCamThread = TRUE;
	
	CloseHandle(g_MainCamThreadHandle);
	CloseHandle(g_SubCamThreadHandle);

	CloseCam(g_hMainCam);
	g_hMainCam = (HANDLE)-1;

	CloseCam(g_hSubCam);
	g_hSubCam = (HANDLE)-1;

	Ret = MCSCORE_Deinit();

	return Ret;
}

// nlX_width is mutiples of 8
HJAPI int StartPreviewPerson (long nlX, long nlY, long nlX_width, long nlY_height)
{
	int Ret = MCS_SUCESS;

	NHDBG((L"StartPreviewPerson!\r\n"));

	if(nlX+nlX_width > g_DispX  || nlX < 0 || nlX_width < 80  || nlX_width%4)
	{
		NHDBG((L"StartPreviewPerson nlX:%d nlX_width:%d FAILED!\r\n", nlX,nlX_width));

		return int MCS_ERR_PRE_ARG_ERR;
	}

	if(nlY+nlY_height > g_DispY || nlY < 0 || nlY_height < 60 )
	{
		NHDBG((L"StartPreviewPerson nlY:%d nlY_height:%d FAILED!\r\n", nlX,nlX_width));

		return int MCS_ERR_PRE_ARG_ERR;
	}
	if(g_hMainCam == (HANDLE)-1)
	{
		NHDBG((L"StartPreviewPerson! FAILED NO CAMERA\r\n"));
		return MCS_ERR_NO_CAM;
	}

	Ret = MCSDDRAW_InitP( nlX, nlY, nlX_width, nlY_height);
	return Ret;
}

// nlX_width is mutiples of 8
HJAPI int StartPreviewExitSlot (long nlX, long nlY, long nlX_width, long nlY_height)
{
	int Ret = MCS_SUCESS;
	NHDBG((L"StartPreviewExitSlot!\r\n"));

	if(nlX+nlX_width > g_DispX  || nlX < 0 || nlX_width < 80  || nlX_width%4)
	{
		NHDBG((L"StartPreviewExitSlot nlX:%d nlX_width:%d FAILED!\r\n", nlX,nlX_width));

		return int MCS_FAIL;
	}

	if(nlY+nlY_height > g_DispY || nlY < 0 || nlY_height < 60 )
	{
		NHDBG((L"StartPreviewExitSlot nlY:%d nlY_height:%d FAILED!\r\n", nlX,nlX_width));

		return int MCS_FAIL;
	}

	if(g_hSubCam == (HANDLE)-1)
	{
		NHDBG((L"StartPreviewExitSlot! FAILED NO CAMERA\r\n"));
		return MCS_ERR_NO_CAM;
	}
	Ret = MCSDDRAW_InitE( nlX, nlY, nlX_width, nlY_height);
	return Ret;
}

HJAPI int StopPreviewPerson ()
{
	int Ret = MCS_SUCESS;
	NHDBG((L"StopPreviewPerson!\r\n"));
	if(g_hMainCam == (HANDLE)-1)
	{
		NHDBG((L"StopPreviewPerson! FAILED NO CAMERA\r\n"));
	}
	Ret = MCSDDRAW_DeinitP(); 
	return Ret;
}

HJAPI int StopPreviewExitSlot ()
{
	int Ret = MCS_SUCESS;
	if(g_hSubCam == (HANDLE)-1)
	{
		NHDBG((L"StopPreviewExitSlot! FAILED NO CAMERA\r\n"));
	}
	NHDBG((L"StopPreviewExitSlot!\r\n"));
	Ret = MCSDDRAW_DeinitE(); 
	return Ret;
}

HJAPI int PersonStillCapture(LPCSTR pFIleName)
{
	int Ret = MCS_SUCESS;
	NHDBG((L"PersonStillCapture!\r\n"));
	if(g_hMainCam == (HANDLE)-1)
	{
		NHDBG((L"PersonStillCapture! FAILED NO CAMERA\r\n"));
		return MCS_ERR_NO_CAM;
	}
	Ret = MCSIMG_ImgCaptureP(pFIleName);
	return Ret;
}


HJAPI int ExitSlotStillCapture(LPCSTR pFIleName)
{
	int Ret = MCS_SUCESS;
	NHDBG((L"ExitSlotStillCapture!\r\n"));

	if(g_hSubCam == (HANDLE)-1)
	{
		NHDBG((L"ExitSlotStillCapture! FAILED NO CAMERA\r\n"));
		return MCS_ERR_NO_CAM;
	}
	
	Ret = MCSIMG_ImgCaptureE(pFIleName);

	return MCS_SUCESS;
}


HJAPI int GrayColorImageOutputP(ULONG ulValLo, ULONG ulVaHi)
{
	int Ret = MCS_SUCESS;
	if(g_hMainCam == (HANDLE)-1)
	{
		NHDBG((L"GrayColorImageOutputP! FAILED NO CAMERA\r\n"));
		return MCS_ERR_NO_CAM;
	}

	if(ulVaHi == 0x08)
		Ret = MCSIMG_GrayColorP(TRUE);
	else
		Ret = MCSIMG_GrayColorP(FALSE);

	return Ret;
}

HJAPI int GrayColorImageOutputE(ULONG ulValLo, ULONG ulVaHi)
{
	int Ret = MCS_SUCESS;

	if(g_hSubCam == (HANDLE)-1)
	{
		NHDBG((L"GrayColorImageOutputE! FAILED NO CAMERA\r\n"));
		return MCS_ERR_NO_CAM;
	}

	if(ulVaHi == 0x08)
		Ret = MCSIMG_GrayColorE( TRUE);
	else
		Ret = MCSIMG_GrayColorE( FALSE);

	return Ret;
}

HJAPI int InsertTextAndColorP(int nColor, long x, long y, TCHAR* szText)
{
	int Ret = MCS_SUCESS;


	if(x < 0 || y < 0)
	{
		NHDBG((L"InsertTextAndColorP x:%d y:%d FAILED!\r\n", x, y));

		return int MCS_ERR_ARG_ERR;
	}

	if(g_hMainCam == (HANDLE)-1)
	{
		NHDBG((L"InsertTextAndColorP! FAILED NO CAMERA\r\n"));
		return MCS_ERR_NO_CAM;
	}

	Ret = MCSTXT_InsertAndColorP(nColor,  x,  y,  szText);

	return Ret;
}

HJAPI int InsertTextAndColorE(int nColor, long x, long y, TCHAR* szText)
{
	int Ret = MCS_SUCESS;


	if(x < 0 || y < 0)
	{
		NHDBG((L"InsertTextAndColorE x:%d y:%d FAILED!\r\n", x, y));

		return int MCS_ERR_ARG_ERR;
	}

	if(g_hSubCam == (HANDLE)-1)
	{
		NHDBG((L"InsertTextAndColorE! FAILED NO CAMERA\r\n"));
		return MCS_ERR_NO_CAM;
	}

	Ret = MCSTXT_InsertAndColorE(nColor,  x,  y,  szText);

	return Ret;
}


/*
[Parameters]
CamNum = 1 : Main Camera
CamNum = 2 : Sub Camera

[Return]
1 : OK
0 : Error
*/
HJAPI int GetCameraStatus(int CamNum)
{
	int Ret = MCS_FAIL;
	HANDLE hCam = (HANDLE)-1;

	if ((CamNum != 0) && (CamNum != 1))
		return MCS_ERR_NO_CAM;

	switch (CamNum)
	{
	case 0: // Main
		hCam = OpenCam_MainCamera();
		if (hCam != (HANDLE)-1)
		{
			Ret = MCS_SUCESS;
			CloseCam(hCam);
			break;
		}
		break;
	case 1: // Sub
		hCam = OpenCam_SubCamera();
		if (hCam != (HANDLE)-1)
		{
			Ret = MCS_SUCESS;
			CloseCam(hCam);
			break;
		}
		break;
	}


	return Ret;
}

HJAPI int NhUsbCamCE_Initialize()
{
	int Ret = MCS_SUCESS;;

	return Ret;
}

HJAPI int NhUsbCamCE_Release()
{
	int Ret = MCS_SUCESS;

	return Ret;
}

// 2018-02-01 추가된 함수입니다
HJAPI int CamStartPerson()
{
	int Ret = MCS_FAIL;
	HANDLE hCam = (HANDLE)-1;

	hCam = OpenCam_MainCamera();

	if (hCam != (HANDLE)-1)
	{
		InternalCamStart(hCam);
		Ret = MCS_SUCESS;
		CloseCam(hCam);
	}

	return Ret;
}

HJAPI int CamStopPerson()
{
	int Ret = MCS_FAIL;
	HANDLE hCam = (HANDLE)-1;

	hCam = OpenCam_MainCamera();

	if (hCam != (HANDLE)-1)
	{
		InternalCamStop(hCam);
		Ret = MCS_SUCESS;
		CloseCam(hCam);
	}

	return Ret;

}

HJAPI int CamStartExitSlot()
{
	int Ret = MCS_FAIL;
	HANDLE hCam = (HANDLE)-1;

	hCam = OpenCam_SubCamera();

	if (hCam != (HANDLE)-1)
	{
		InternalCamStart(hCam);
		Ret = MCS_SUCESS;
		CloseCam(hCam);
	}

	return Ret;
}

HJAPI int CamStopExitSlot()
{
	int Ret = MCS_FAIL;
	HANDLE hCam = (HANDLE)-1;

	hCam = OpenCam_SubCamera();

	if (hCam != (HANDLE)-1)
	{
		InternalCamStop(hCam);
		Ret = MCS_SUCESS;
		CloseCam(hCam);
	}

	return Ret;
}

#endif // UNDER_CE