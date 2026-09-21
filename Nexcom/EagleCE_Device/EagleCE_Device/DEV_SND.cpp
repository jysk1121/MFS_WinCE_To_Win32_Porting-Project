#include "stdafx.h"
#include "DEV_SND.h"
//#include <mmdeviceapi.h>
//#include <endpointvolume.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/************************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CDEV_SND::CDEV_SND()
{
}


/************************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CDEV_SND::~CDEV_SND()
{
}


/************************************************************
*	@brief		Play Sound
*	@retval		없음
************************************************************/
BOOL CDEV_SND::PlaySound(CString strFile, DWORD dwPlayMode)
{
	return sndPlaySound(strFile, SND_ASYNC | SND_NODEFAULT);
}


/************************************************************
*	@brief		Stop Sound
*	@retval		없음
************************************************************/
BOOL CDEV_SND::StopSound()
{
	return sndPlaySound(NULL, SND_ASYNC);
}


/************************************************************
*	@brief		Set Volume
*	@retval		없음
************************************************************/
BOOL CDEV_SND::SetVolume(DWORD nVolume)
{
	BOOL bResult = TRUE;


	IMMDeviceEnumerator *deviceEnumerator = NULL;

	IMMDevice *defaultDevice = NULL;

	IAudioEndpointVolume *endpointVolume = NULL;


	BOOL bProcessedOnlyOnce = TRUE;

	while (bProcessedOnlyOnce)
	{
		bProcessedOnlyOnce = FALSE;


		if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator)))
		{
			bResult = FALSE;

			break;
		}


		if (FAILED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice)))
		{
			bResult = FALSE;

			break;
		}


		if (FAILED(defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume)))
		{
			bResult = FALSE;

			break;
		}


		float fNewVolume = ((float)nVolume) / 100;

		if (FAILED(endpointVolume->SetMasterVolumeLevelScalar(fNewVolume, NULL)))
		{
			bResult = FALSE;

			break;
		}
	}


	if (deviceEnumerator)
	{
		deviceEnumerator->Release();
	}

	if (defaultDevice)
	{
		defaultDevice->Release();
	}

	if (endpointVolume)
	{
		endpointVolume->Release();
	}


	return bResult;
}


/************************************************************
*	@brief		Get Volume
*	@retval		없음
************************************************************/
BOOL CDEV_SND::GetVolume(DWORD& nVolume)
{
	BOOL bResult = TRUE;


	IMMDeviceEnumerator *deviceEnumerator = NULL;

	IMMDevice *defaultDevice = NULL;

	IAudioEndpointVolume *endpointVolume = NULL;


	BOOL bProcessedOnlyOnce = TRUE;

	while (bProcessedOnlyOnce)
	{
		bProcessedOnlyOnce = FALSE;


		if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator)))
		{
			bResult = FALSE;

			break;
		}


		if (FAILED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice)))
		{
			bResult = FALSE;

			break;
		}


		if (FAILED(defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume)))
		{
			bResult = FALSE;

			break;
		}


		float currentVolume = 0;

		if (FAILED(endpointVolume->GetMasterVolumeLevelScalar(&currentVolume)))
		{
			bResult = FALSE;

			break;
		}

		nVolume = (int)floor((currentVolume * 100) + 0.5);
	}


	if (deviceEnumerator)
	{
		deviceEnumerator->Release();
	}

	if (defaultDevice)
	{
		defaultDevice->Release();
	}

	if (endpointVolume)
	{
		endpointVolume->Release();
	}


	return bResult;
}
