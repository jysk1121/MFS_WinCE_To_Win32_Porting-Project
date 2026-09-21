#ifndef __DEV_CMN_CAM_H__
#define __DEV_CMN_CAM_H__

#define NO_VALUE		-1

#ifdef UNDER_CE

private:
	bool	m_bCamInit;
	bool	m_bCamPreviewInProgress;
	CString m_strCamLastErrorCode;

	CMap<int, int, int, int> m_camImageIdx_Pin;
	CMap<int, int, int, int> m_camImageIdx_Rcpt;

	CString	GetCamImageNameFormat(	int pImgIndex = NO_VALUE,
									int pTranNum = NO_VALUE,
									NH_CAM_TAKEN_TIME pTakenTime = TIME_NO_VALUE);
	CString GetCamImageFolder(int nImgIndex);
	void	fnCAM_SetErrorCode(int nErrorCode);

public:
	// Device Management
	bool	fnCAM_Initialize();
	bool	fnCAM_Deinitialize();
	int		fnCAM_GetDeviceStatus(NH_CAM_TYPE eCamType);
	CString fstrCAM_GetErrorCode();
	void	fnCAM_ClearErrorCode();


	// Camera Functions
	int		fnCAM_StartAntiShoulderSurfing(	long x = NH_CAM_POSITION_X_CENTER,
											long y = NH_CAM_POSITION_Y_CENTER,
											long width = NH_CAM_RESOLUTION_WIDTH,
											long height = NH_CAM_RESOLUTION_HEIGHT);
	int		fnCAM_StopAntiShoulderSurfing();
	int		fnCAM_CaptureFace(int pTranNum, NH_CAM_TAKEN_TIME eMemorizeTime);


	// Helpers
	CString	fnCAM_GetFaceImageName(int pImgIndex);
	CString	fnCAM_GetFaceImageName(int pTranNum, NH_CAM_TAKEN_TIME eMemorizeTime);
	int		fnCAM_GetTranNumFromJnl(int pJnlIndex);

	void	fnCAM_DeleteCamImage(int pImgIndex);
	void	fnCAM_DeleteCamImage(int pTranNum, NH_CAM_TAKEN_TIME eMemorizeTime);
	void	fnCAM_ClearAllCamImages();

	int		fnCAM_BackupImages(CString pDestPath);

	LONGLONG fnCAM_GetFreeSpaceInfo();

#endif __DEV_CMN_CAM_H__

#endif