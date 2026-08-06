#pragma once

#include <afxmt.h>

typedef struct _CAMFORMATPROS
{
	int index;
	LONG width;
	LONG height;
	LONGLONG AvgTimePerFrame;
	GUID subType;
}CAMFORMATPROS,*PCAMFORMATPROS;

class CDEV_CAMERA
{
public:
	// 생성자
	CDEV_CAMERA();

	// 소멸자
	virtual ~CDEV_CAMERA();
/**
* Open *
* Close *
* RunGraph *
* StopGraph *
* StartVideoCapture
* StopVideoCapture
* StillCapture
*/

	void SetHwnd(HWND hWnd);

	// Camera Driver Init
	BOOL InitCameraDriver();

	// Open
	BOOL Open();

	// Close
	BOOL Close();

	BOOL IsOpened();

	BOOL IsExistCameraDriver();	// V1.0.2.4 2018.07.27 - 카메라 드라이버 유무 판단

	BOOL SetUseCamera(BOOL bUse);

	BOOL GetCameraSetting();

	BOOL SetVideoWindosPos(RECT *rc);

	// Start Capture
	BOOL RunGraph();

	// Stop Capture
	BOOL StopGraph(BOOL bErrorClear=TRUE);

	// StillCapture Capture
	BOOL StillCapture(BOOL bMode, BOOL bAsync);

	BOOL IsCameraProcessing();

	CString GetVersion();

	CString GetLastError();		// V1.0.2.4 2018.07.27 - 장애코드 추가

	// Start Video Capture
	//	BOOL StartVideoCapture(TCHAR *CaptureFileName);

	// Stop Video Capture
	//	BOOL StopVideoCapture();

	// Get Current Image
	//CBitmap* GetCurrentImage(void);

	// Get Device Name
	//LPCTSTR GetDeviceName(void);


	// Get Last Error
	//DWORD GetLastError(void);

	//HRESULT GetCurrentStillResolution();

	//HRESULT GetCurrentVideoResolution();

	//HRESULT GetVideoProfile(int nStream,UINT8 nCurrentRes);


protected:
	static	UINT			StillCapture_Thread(LPVOID pParam);

	void					StillCapture_Thread();

	HWND					m_hWnd;

	DWORD					m_dwLastError;

	BOOL					m_bIsCameraExist;

	BOOL					m_bCameraOpened;

	BOOL					m_bExecStillCapture;

	BOOL					m_bSupervisorMode;

	BOOL					m_bThreadExit;

	HANDLE					m_hStillCaptureThread;

	CCriticalSection		m_CameraLock;

	CString					m_CaptureFileName;

	CString					m_FlipCaptureFileName;

	TCHAR					m_tchBuffer[1024];

	CString					m_strLastError;	// V1.0.2.4 2018.07.27 - 장애코드 추가
};
