/////////////////////////////////////////////////////////////////////////////
//	DevCmn_Cam.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "..\..\H\Dev\DevCmn.h"

#ifdef UNDER_CE

#include ".\camera\\huc_api.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include "..\..\H\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

bool CDevCmn::fnCAM_Initialize()
{
	NHDEBUG(DBG_CALL, (L"fnCAM_Initialize()\r\n"));

	if (!m_bCamInit)
	{
#ifdef NH_DEBUG
		MCSDP_SET(MCS_DP_LOG_ONLY);
#else
		MCSDP_SET(MCS_DP_NO_LOG);
#endif

		MCS_Config	McsConfig;
		/*
		bool	bUseCaption ;		//Default 1
		bool	bUseBMP ;			//Default 0
		bool	bUseJPEG;			//Default 1
		bool	bLogEnable;			//Default 0
		char	CaptionColor ;		//Default 1
		char	JPEGQuality ;		//Default 2
		char	LogPath[MAX_PATH];	//Default "\\ATM2\\LOG\\"
		char	LogMaxFiles;		//Default 30
		*/

		McsConfig.bUseCaption = MCS_YES;
		McsConfig.bLogEnable = MCS_NO;
		McsConfig.bUseBMP = MCS_NO;
		McsConfig.bUseJPEG = MCS_YES;
		McsConfig.CaptionColor = COLOR_WHITE;
		McsConfig.JPEGQuality = QUALITY_LEVEL2;
		if (m_pConfig->GetCEVersion() == WINCE_7)	// allow larger image on WEC7
			McsConfig.JPEGQuality = QUALITY_LEVEL4;
		strcpy(McsConfig.LogPath, "\\ATM2\\LOG\\");
		McsConfig.LogMaxFiles = 30;
		McsConfig.SkipVidRenderFrame = 0;

		if (MCSCORE_SetHucConfig(&McsConfig) == MCS_SUCESS)
		{
			MCSCORE_InitThreads();
			m_bCamInit = true;
		}
	}

	NHDEBUG(DBG_CALL, (L"CamStartPerson()\r\n"));
	if (CamStartPerson() == MCS_SUCESS)
	{
		fnCAM_ClearErrorCode();
		Delay_Msg(500);
	}
	else
	{
		fnCAM_SetErrorCode(MCS_ERR_COM_FAIL);
	}

	NHDEBUG(DBG_CALL, (L"fnCAM_Initialize() Complete!\r\n"));

	return m_bCamInit;
}

bool CDevCmn::fnCAM_Deinitialize()
{
	NHDEBUG(DBG_CALL, (L"fnCAM_Deinitialize()\r\n"));

	bool bRet = false;

	if (MCSCORE_DeinitThreads() == MCS_SUCESS)
	{
		NHDEBUG(DBG_CALL, (L"CamStopPerson()\r\n"));
		CamStopPerson();

		m_bCamInit = false;
		bRet = true;
	}

	NHDEBUG(DBG_CALL, (L"fnCAM_Deinitialize() Complete!\r\n"));

	return bRet;
}

int CDevCmn::fnCAM_GetDeviceStatus(NH_CAM_TYPE pCamType)
{
	int nStatus = GetCameraStatus(pCamType);
	NHDEBUG(DBG_CALL, (L"fnCAM_GetDeviceStatus(%d) : %s!\r\n", pCamType,
														(nStatus == MCS_SUCESS) ? _T("NORMAL") : _T("DOWN")
														));

	if (nStatus == MCS_SUCESS)
		return NORMAL;
	else
	{
		fnCAM_SetErrorCode(MCS_ERR_COM_FAIL);
		return DOWN;
	}
}

CString CDevCmn::fstrCAM_GetErrorCode()
{
	return m_strCamLastErrorCode;
}

void CDevCmn::fnCAM_ClearErrorCode()
{
	m_strCamLastErrorCode = _T("0000000");
}

int CDevCmn::fnCAM_StartAntiShoulderSurfing(long x, long y, long width, long height)
{
	if (!m_bCamInit)
	{
		fnCAM_SetErrorCode(MCS_ERR_COM_FAIL);
		return T_ERROR;
	}

	if (m_bCamPreviewInProgress == true)
		return T_OK;

	int nRet = StartPreviewPerson(x, y, width, height);
	NHDEBUG(DBG_CALL, (L"StartPreviewPerson[%d, %d, %d, %d] :%d\r\n", x, y, width, height, nRet));

	if (nRet == MCS_SUCESS)
	{
		m_bCamPreviewInProgress = true;
		return T_OK;
	}
	else
	{
		nRet = 9702071;		// temporary value (DLL needs to be updated)
		fnCAM_SetErrorCode(nRet);
		m_bCamPreviewInProgress = false;
		return T_ERROR;
	}
}

int CDevCmn::fnCAM_StopAntiShoulderSurfing()
{
	if (!m_bCamInit)
		return T_ERROR;

	if (m_bCamPreviewInProgress == false)
		return T_OK;

	int nRet = StopPreviewPerson();
	NHDEBUG(DBG_CALL, (L"StopPreviewPerson:%d\r\n", nRet));

	if (nRet == MCS_SUCESS)
	{
		m_bCamPreviewInProgress = false;
		return T_OK;
	}
	else
		return T_ERROR;
}

int CDevCmn::fnCAM_CaptureFace(int pTranNum, NH_CAM_TAKEN_TIME pTakenTime)
{
	if (!m_bCamInit)
	{
		fnCAM_SetErrorCode(MCS_ERR_COM_FAIL);
		return T_ERROR;
	}

	CString strTermID = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTermID.Trim();
	if (strTermID.IsEmpty())		strTermID = _T("(NO TID)");

	int nImageIndex = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_IMAGE_INDEX);

	CreateDirectoryWithIntermediate(GetCamImageFolder(nImageIndex));
	if (IsExistFile(GetCamImageFolder(nImageIndex)) == FALSE)
	{
		fnCAM_SetErrorCode(MCS_ERR_NO_PATH);
		return T_ERROR;
	}
	
	fnCAM_DeleteCamImage(nImageIndex);		// delete exist images which have transaction number to be used.

	SYSTEMTIME localTime;
	::GetLocalTime(&localTime);

	// tID_imageIdx_tranNum_takenTime_date_time
	// ex) AAPAU002_0002_9292_2_20180114_185203.jpg
	// ex) (NO TID)_0003_9292_2_20180117_091358.jpg
	CString strFilename;
	strFilename.Format(L"%s\\%s_%04d_%04d_%d_%s_%2.2d%2.2d%2.2d",
		GetCamImageFolder(nImageIndex),	//NH_CAM_FACE_IMAGE_FOLDER,
		strTermID,
		nImageIndex,
		pTranNum,
		pTakenTime,
		GetCmnLocalDate(localTime.wMonth, localTime.wDay, localTime.wYear, 0),
		localTime.wHour, localTime.wMinute, localTime.wSecond);

	CStringA szTemp(strFilename);
	int nRet = PersonStillCapture(szTemp);
	NHDEBUG(DBG_CALL, (L"PersonStillCapture [%s] :%d\r\n", strFilename, nRet));

	if (nRet == MCS_SUCESS)
	{
		// caching TranNum/ImageIndex.
		if (pTakenTime == TIME_PIN_ENTERED)					m_camImageIdx_Pin.SetAt(pTranNum, nImageIndex);
		else if (pTakenTime == TIME_RECEIPT_PROCESSING)		m_camImageIdx_Rcpt.SetAt(pTranNum, nImageIndex);

		int nImageMaxCount = NH_CAM_FACE_IMAGE_CNT_MAX;
		if ( fnCAM_GetFreeSpaceInfo() < NH_CAM_FACE_IMAGE_LOW_SPACE_SIZE )
		{
			NHDEBUG(DBG_CALL, (L"Image count is being limited to %d due to LOW SPACE: %lli\r\n", NH_CAM_FACE_IMAGE_CNT_MAX_LOW_SPACE, fnCAM_GetFreeSpaceInfo()));
			nImageMaxCount = NH_CAM_FACE_IMAGE_CNT_MAX_LOW_SPACE;
		}

		nImageIndex++;
		if (nImageIndex > nImageMaxCount) nImageIndex = 1;
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CAM_FACE_IMAGE_INDEX, nImageIndex);

		return T_OK;
	}
	else
	{
		fnCAM_SetErrorCode(nRet);
		return T_ERROR;
	}
}

void CDevCmn::fnCAM_DeleteCamImage(int pTranNum, NH_CAM_TAKEN_TIME pTakenTime)
{
	CString strSearchFile = fnCAM_GetFaceImageName(pTranNum, pTakenTime);
	BOOL bResult = DeleteFile(strSearchFile);

	NHDEBUG(DBG_CALL, (_T("Deleting %s... %s\n"), strSearchFile, (bResult == TRUE ? _T("OK") : _T("FAILED")) ));
}

void CDevCmn::fnCAM_DeleteCamImage(int pImgIndex)
{
	// there are several 'Taken Time' images for one index. delete them all.
	CString strSearchFile = _T("");
	int nExitCount = 0;

	do 
	{
		strSearchFile = fnCAM_GetFaceImageName(pImgIndex);
		if (strSearchFile.IsEmpty() || nExitCount > TIME_NO_VALUE)
			break;

		BOOL bResult = DeleteFile(strSearchFile);
		NHDEBUG(DBG_CALL, (_T("Deleting %s... %s\n"), strSearchFile, (bResult == TRUE ? _T("OK") : _T("FAILED")) ));

		nExitCount++;
	} while (true);
}

CString CDevCmn::fnCAM_GetFaceImageName(int pImgIndex)
{
	return GetCamImageNameFormat(pImgIndex, NO_VALUE, TIME_NO_VALUE);
}

CString CDevCmn::fnCAM_GetFaceImageName(int pTranNum, NH_CAM_TAKEN_TIME pTakenTime)
{
	if (pTranNum == NO_VALUE)
		return _T("");
	else
		return GetCamImageNameFormat(NO_VALUE, pTranNum, pTakenTime);
}

void CDevCmn::fnCAM_ClearAllCamImages()
{
	DeleteFileInDirectory(NH_CAM_FACE_IMAGE_FOLDER, L"*.*");
}

int CDevCmn::fnCAM_GetTranNumFromJnl(int pJnlIndex)
{
	int nTranNum = NO_VALUE;
	CString strJnlData = m_JNLMgr.SearchIndex(pJnlIndex);

	NHDEBUG(DBG_CALL, (L"[] strJnlData : %s\r\n", strJnlData));

	CStringArray	arJnlField;
	SplitString(strJnlData,"^", arJnlField);

	if (arJnlField.GetCount() >= 10)
	{
		arJnlField[1].TrimLeft();	// ' TA' -> 'TA'

		// Transaction JNL
		if (arJnlField[1] == NORMAL_TRX || arJnlField[1] == TRX_ERROR || arJnlField[1] == REVERSAL_TRX ||
			arJnlField[1] == TRX_NOT_APPROVED || arJnlField[1] == COMMUNICATION_ERROR )
		{
			nTranNum = Asc2Int(arJnlField[10]);
		}
	}

	return nTranNum;
}

CString	CDevCmn::GetCamImageNameFormat(int pImgIndex, int pTranNum, NH_CAM_TAKEN_TIME pTakenTime)
{
	// get image index from CMap cache when available
	int imageIndexFromMap = NO_VALUE;

	if (pTakenTime == TIME_PIN_ENTERED)					m_camImageIdx_Pin.Lookup(pTranNum, imageIndexFromMap);
	else if (pTakenTime == TIME_RECEIPT_PROCESSING)		m_camImageIdx_Rcpt.Lookup(pTranNum, imageIndexFromMap);

	if (imageIndexFromMap != NO_VALUE)
	{
		NHDEBUG(DBG_CALL, (_T("[takenTime:%d] imageIndex found from the map!: %d\n"), pTakenTime, imageIndexFromMap));
		pImgIndex = imageIndexFromMap;
	}
	else
		NHDEBUG(DBG_CALL, (_T("[takenTime:%d] imageIndex not found !!\n"), pTakenTime));


	// find proper images
	CString strFileName = _T("");
	CString strImgIndex, strTranNum, strTakenTime;

	if (pImgIndex == NO_VALUE)			strImgIndex = _T("????");
	else								strImgIndex.Format(_T("%04d"), pImgIndex);

	if (pTranNum == NO_VALUE)			strTranNum = _T("????");
	else								strTranNum.Format(_T("%04d"), pTranNum);

	if (pTakenTime == TIME_NO_VALUE)	strTakenTime = _T("?");
	else								strTakenTime.Format(_T("%d"), pTakenTime);

	NHDEBUG(DBG_CALL, (_T("ImgIndex: %s, TranNum: %s, TakenTime: %s\n"), strImgIndex, strTranNum, strTakenTime));

	WIN32_FIND_DATA	FileData;
	HANDLE			hSearch = INVALID_HANDLE_VALUE;
	CString			strSearchFile;

	CString strTermID = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID);
	strTermID.Trim();
	if (strTermID.IsEmpty())		strTermID = _T("(NO TID)");

	// make file name for search.
	for (int folderIdx = 1; folderIdx <= (NH_CAM_FACE_IMAGE_CNT_MAX / NH_CAM_FACE_IMAGE_CNT_PER_DIR); folderIdx++)
	{
		if (pImgIndex != NO_VALUE)
			folderIdx = ( (pImgIndex-1) / NH_CAM_FACE_IMAGE_CNT_PER_DIR ) + 1;

		// \\ATM\\JNL\\CAMIMAGE\\01 ~ 20\\TID_0000_0000_0_yyyymmdd_hhmmss.jpg
		strSearchFile.Format(L"%s\\%02d\\%s_%s_%s_%s_%s_%s.jpg",
			NH_CAM_FACE_IMAGE_FOLDER,
			folderIdx,
			strTermID,
			strImgIndex,
			strTranNum,
			strTakenTime,
			_T("????????"),	// GetCmnLocalDate(localTime.wMonth, localTime.wDay, localTime.wYear, '_'),
			_T("??????"));	// localTime.wHour, localTime.wMinute, localTime.wSecond);

		hSearch = FindFirstFile(strSearchFile, &FileData);
		if (hSearch == INVALID_HANDLE_VALUE)
		{
			NHDEBUG(DBG_CALL, (_T("Failed to Find File: %s\n"), strSearchFile));
		}
		else
		{
			strFileName.Format(_T("%s\\%02d\\%s"), NH_CAM_FACE_IMAGE_FOLDER, folderIdx, FileData.cFileName);
			NHDEBUG(DBG_CALL, (_T("Journal Pic Found!: %s\n"), strFileName));

			CStringArray	arrTemp;
			SplitString(FileData.cFileName, L"_", arrTemp);
			if (arrTemp.GetCount() > 2)
			{
				if (pTakenTime == TIME_PIN_ENTERED)					m_camImageIdx_Pin.SetAt(pTranNum, _ttoi(arrTemp.GetAt(1)));
				else if (pTakenTime == TIME_RECEIPT_PROCESSING)		m_camImageIdx_Rcpt.SetAt(pTranNum, _ttoi(arrTemp.GetAt(1)));				
			}

			break;
		}

		if (pImgIndex != NO_VALUE)
			break;
	}

	FindClose(hSearch);

	return strFileName;
}

void CDevCmn::fnCAM_SetErrorCode(int nErrorCode)
{
	m_strCamLastErrorCode.Format(_T("%07d"), nErrorCode);
	NHDEBUG(DBG_CALL, (_T("Cam Error Set: %s\n"), m_strCamLastErrorCode));
}

CString CDevCmn::GetCamImageFolder(int nImgIndex)
{
	if (nImgIndex < 1 || nImgIndex > NH_CAM_FACE_IMAGE_CNT_MAX)
		nImgIndex = 1;

	// \\ATM2\\JNL\\CAMIMAGE\\01 ~ 20
	int camFolderIdx = ( (nImgIndex-1) / NH_CAM_FACE_IMAGE_CNT_PER_DIR ) + 1;
	
	CString camFolder = _T("");
	camFolder.Format(_T("%s\\%02d"), NH_CAM_FACE_IMAGE_FOLDER, camFolderIdx);

	return camFolder;
}

int CDevCmn::fnCAM_BackupImages(CString destPath)
{
	NHDEBUG(DBG_CALL, (_T("STARTING BACKUP CAM IMAGES...\n")));

	CString			strSrcFolder = _T("");
	CString			strDestFolder = _T("");

	for (int folderIdx = 1; folderIdx <= ( NH_CAM_FACE_IMAGE_CNT_MAX / NH_CAM_FACE_IMAGE_CNT_PER_DIR); folderIdx++)
	{
		strSrcFolder.Format(_T("%s\\%02d"), NH_CAM_FACE_IMAGE_FOLDER, folderIdx);

		if (IsExistFile(strSrcFolder))
		{
			strDestFolder.Format(_T("%s\\%s\\%02d"), destPath, _T("CAMIMAGE"), folderIdx);
			CreateDirectoryWithIntermediate(strDestFolder);

			DeleteFileInDirectory(strDestFolder, _T("*.*"));
			CopyFileInDirectory(strSrcFolder, _T("*.*"), strDestFolder);
		}
	}

	NHDEBUG(DBG_CALL, (_T("BACKUP COMPLETE!\n")));

	return T_OK;
}

LONGLONG CDevCmn::fnCAM_GetFreeSpaceInfo()
{
	// returns free space of 'ATM2' partition, by the unit of 'Byte'.
	CString strDrive = _T("\\ATM2"); 
	ULARGE_INTEGER  _64FreeBytesToCaller, _64TotalBytes, _64FreeBytes;
	BOOL bResult = GetDiskFreeSpaceEx((LPCTSTR)strDrive, &_64FreeBytesToCaller, &_64TotalBytes, &_64FreeBytes);

	if (bResult)
	{
		LONGLONG nDiskFree = (UINT)(_64FreeBytes.QuadPart);
		return nDiskFree;
	}

	return -1;
}

#endif // UNDER_CE