#ifndef __EAGLE_SVC_LIB_H__
#define __EAGLE_SVC_LIB_H__

class CEagleSVCLib
{
public:
	// 인스턴스
	static CEagleSVCLib* m_pInstance;

	// 생성자, 소멸자.
	CEagleSVCLib();
	virtual ~CEagleSVCLib();

	// 인스턴스 생성
	static void CreateInstance();
	// 인스턴스 취득
	static CEagleSVCLib* GetInstance();
	// 인스턴스 해제
	static void ReleaseInstance();

	BOOL	IsTerminal_Error();
	void	Set_ErrorCode(CString strErrorCode, int nErrorDevice = 0, CString strErrorMsg = _T(""));
	void	Set_DeviceErrorCode(CString strDeviceKind, CString strErrorCode, CString strErrorMsg = _T(""));
	CString Get_ErrorCode();
	CString Get_ErrorMsg();
	void	Clear_Error();
	void	Clear_DeviceError(CString strDeviceKind);

	// 
	BOOL	Backup_LogFile(CString strDestDirectory);

	BOOL	Add_Backup_LogFile(CString strDestDirectory, CString strErrorCode = _T(""), BOOL bRemainLog = TRUE);

	BOOL	Backup_JnlFile(CString strDestDirectory);

	//
	BOOL	Backup_ConfigFile(CString strDestDirectory);

	CString	MergeConfigData(CString strDestDirectory, CString strFileName);

	BOOL	Upload_ConfigFile(CString strSrcDirectory, CString strDestDirectory);

	BOOL	SeparateConfigData(CString strDestDirectory, CString strFileName, CString strAllConfigData);

	BOOL	Backup_CaptureImageFile(CString strDestDirectory, CString strBackupDate);

	CString	Add_Backup_CaptureImageFile();

	//
	void	HexaDump(BYTE *pData, int nLen);

	//
	void	StringHexaDump(CString strData);

	// 
	BOOL	Recovery_ErrorDevice(int nErrorDevice, BOOL bCBXStatusClear = FALSE);

	// PTR Command
	BOOL	Set_PTR_Config();

	// Host Communication
	int		SendRecvHost(int nTransType);

	// Print and Save EJL Day Total Information
	BOOL	SaveEJL_N_Print_DayClose(int nType, BOOL bResult, BOOL bPrint);

	// Print CST Total Information
	BOOL	SaveEJL_N_Print_CBXClose(int nType);

	// Print CBX Add QTY Information
	BOOL	SaveEJL_N_Print_CBXAddQTY(int nAdded_Count[MAX_CST_COUNT], BOOL bPrint = TRUE);

	BOOL	PrintEJL_DetailData(CString strPrintData);

	BOOL	SaveEJL_SetDenomination(int nOldDenom[MAX_CST_COUNT], int nNewDenom[MAX_CST_COUNT]);

	//BOOL	SaveEJL_SetChangeValue(CString strOld, CString strNew);		// AMS Setup정보 변경시에도 필요하여 Dev Class로 이동

	BOOL	SaveEJL_TransactionInfo();

	BOOL	Print_SetDenomination(int nOldDenom[MAX_CST_COUNT], int nNewDenom[MAX_CST_COUNT]);

	// [V1.0.2.10] 2021.03.02 LEH - TestDispense 저널 저장 추가
	BOOL	SaveEJL_TestDispense(int nTestDispResult, BYTE byReqCount[MAX_CASSETTE], BYTE byPickupCount[MAX_CASSETTE], BYTE byRejCount[MAX_CASSETTE], CString strErrorCode = _T(""));

	void	UpdateRejectInfo(int nTestDispenseCbxNumber = 0, BYTE byTestDispenseCount = 0);

	void	CalculateRejectKind(int nCbxNumber, BYTE byRejectCode, BYTE byRejectKind[6][REJECT_KIND+1]);	// +1 : total

	// Check CST Status -> AMS에서도 참조가 필요하여 Device의 CDM쪽이로 함수 이동
	//int		Get_CBXAllStatus();

	//int		Get_CBXAllNoteCount();

	BOOL	Check_Invalid_CBXAllDenomination();

	// Setting Health Check
	void	Set_Heartbeat_Time();

	// Check Health Check Time
	UINT	Get_Heartbeat_GapTime();

	BOOL	Send_HeartbeatOrScheduleDayCloseMsg(int nMsgType);

	BOOL	IsPTR_Available(BOOL bCheckWait = FALSE);

	void	Set_Reboot_Time();		// AMS에서 System Time 변경시에도 Call 가능해야 하므로 Device쪽으로 위치 이동

	void	Device_DeInitialize();

public:
	UINT				m_DelayTime;

private:
	COleDateTime		m_oletimeNow;
	COleDateTimeSpan	m_oletimeGap;
	COleDateTime		m_oletimeStart;


};

#endif __EAGLE_SVC_LIB_H__