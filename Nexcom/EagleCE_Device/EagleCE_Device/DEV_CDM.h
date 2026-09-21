#pragma once

#include <afxmt.h>

#include "..\\..\\include\\Device\\DEV_CDM_Define.h"

#define MAX_CBX_COUNT						6
#define MAX_CFG_CBX_COUNT					4

#define WFS_CDM_STATCUOK		0
#define WFS_CDM_STATCUFULL		1		// not use
#define WFS_CDM_STATCUHIGH		2		// not use
#define WFS_CDM_STATCULOW		3
#define WFS_CDM_STATCUEMPTY		4
#define WFS_CDM_STATCUINOP		5		// not use
#define WFS_CDM_STATCUMISSING	6

#define DISP_TYPE_ORDER		0
#define DISP_TYPE_EQUAL		1
#define DISP_TYPE_REVERSE	2

#define CE_INFO_LENGTH		32

#define CERTIFICATION_ERROR	_T("D1000B")
#define COMMUNICATION_ERROR _T("D100F0")

#define CHECK_STATUS_INTERVAL	2000	// System 부하를 줄이기 위해 2초로 조정


class CDEV_CDM
{
public:
	// 생성자
	CDEV_CDM();
	// 소멸자
	virtual ~CDEV_CDM();

	// Is DLL Loaded
	BOOL IsDllLoaded(void);

	BOOL CDM_OpenPort(HWND hWnd, int nPortNum, int nWriteLogLevel);
	BOOL CDM_ClosePort();
	BOOL CDM_Reset(BOOL bCbxStatusClear=FALSE);
	BOOL CDM_CfgStatus();
	BOOL CDM_Status(LPCDMSTATUS lpStatus, BOOL bCallbyThread=FALSE);
	int CDM_MultiDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult);
	BOOL CDM_LastDispense(LPCDMLASTDISPENSE lpResult);
	BOOL CDM_Diagnostic(LPCDMDIAGNOSTIC lpResult);
	BOOL CDM_SetBillThickness(LPCDMSETBILLTHICKNESS lpSet, LPBYTE lpbyErrorCd);
	BOOL CDM_GetBillThickness(BYTE byCbxNo, LPCDMGETBILLTHICKNESS lpGet);
	BOOL CDM_SetBillSize(LPCDMSETBILLSIZE lpSet, LPBYTE lpbyErrorCd);
	BOOL CDM_GetBillSize(BYTE byCbxNo, LPCDMGETBILLSIZE lpGet);
	BOOL CDM_LearnBill(BYTE byCbxNo, BYTE byItemCount, LPCDMLEARN lpResult);
	BOOL CDM_GetDLLVersion(LPBYTE major_num, LPBYTE minor_num);
	BOOL CDM_DispenseByAmount(UINT nAmount, UINT nCbxCount, UINT nCbxDenom[4], UINT nCbxRemain[4], LPCDMMULTIDISPENSE lpResult);	// AP에서 사용 안함
	int CDM_DispenseByAmount(UINT nAmount, LPCDMMULTIDISPENSE lpResult, UINT* nDispenseAmount);
	int CDM_DispenseByCount(LPCDMMIXRESULT lpCdmMixResult, LPCDMMULTIDISPENSE lpResult, UINT* nDispenseAmount);
	int CDM_TestDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult);
	BOOL CDM_GetNoteLog();
	BOOL CDM_GetThickLog();
	int GetSameCBXDenomCount(int nMixValue);
	int GetCBXIndex(int nMixValue, int nIndex, int nDispType=0);
	BOOL CDM_GetBillInfo(int nCbxCount, CDM_BILL_INFO &billInfo);
	BOOL CDM_SetBillInfo(int nCbxNumber, CDM_BILL_INFO billInfo);
	BOOL CDM_GetRejectLog(LPCDM_REJECT_INFO lpRejectLog);
	BOOL CDM_GetDispenseLog();
	BOOL CDM_UpdateFirmware();
	BOOL CDM_CEInfoCertification();
	BOOL CDM_IsSendDispense();
	BOOL CDM_Initialize(BOOL bCbxStatusClear=FALSE);
	BOOL CDM_GetExistCEInfo(BOOL &bExist);	///< Firmware에 CE부 인증 정보가 있는지 여부

	// 에러코드 취득
	CString GetLastError(void);

	DWORD MakeCbxStatus(int nCbxNumber, BYTE byStatus, BYTE byStatus2=0);	///< 카세트 상태 변환

	void GetCbxStatus(DWORD dwCbxStatus[MAX_CASSETTE+1]);					///< 카세트 상태 Read

	BOOL IsDispenable(int nAmount);	///< 방출 가능 여부

	BOOL ExecuteMixAlgorithm(int nAmount, LPCDMCBXITEM lpCdmCbxItem, int nDispType=0);	///< Mix Algorithm

	BOOL ExecuteMixAlgorithm4MaxAvailAmount(UINT nAmount, LPCDMMIXRESULT lpCdmMixResult, UINT &nRemainAmount, BOOL bApplyLowStatus = FALSE, int nDispType = 0);	///< 최대 방출 가능한 금액 Mix Algorithm

	void WriteLogDataToLOG(BYTE* pbyRecvData, DWORD dwRecvSize);	///< 로그 data 파일 생성(.log 파일)

	void WriteLogDataToCSV(BYTE* pbyRecvData, DWORD dwRecvSize);	///< 로그 data 파일 생성(.csv 파일)

	int	Get_CBXAllStatus();

	int	Get_CBXAllNoteCount();

	void Clear_CBXStatus();

	// Status Thread
	void CDM_QuiryStatusStart();	///< 상태조회 thead 시작

	void CDM_QuiryStatusStop();		///< 상태조회 thead 중지

	void StartCDMThread();

	void ThreadDeviceStatus();

	static UINT WatchStatus(LPVOID pParam);


protected:
	typedef BOOL(WINAPI *MFSCommCDM_OpenPort)(HWND, UINT, int);

	typedef BOOL(WINAPI *MFSCommCDM_ClosePort)();
	
	typedef BOOL(WINAPI *MFSCommCDM_Reset)();
	
	typedef BOOL(WINAPI *MFSCommCDM_CfgStatus)(LPCDMCFGSTATUS, LPBYTE);
	
	typedef BOOL(WINAPI *MFSCommCDM_Status)(LPCDMSTATUS);
	
	typedef int(WINAPI *MFSCommCDM_MultiDispense)(LPCDMCBXITEM, LPCDMMULTIDISPENSE, LPBYTE);
	
	typedef BOOL(WINAPI *MFSCommCDM_LastDispense)(LPCDMLASTDISPENSE, LPBYTE);
	
	typedef BOOL(WINAPI *MFSCommCDM_Diagnostic)(LPCDMDIAGNOSTIC);
	
	typedef BOOL(WINAPI *MFSCommCDM_SetBillThickness)(LPCDMSETBILLTHICKNESS, LPBYTE);
	
	typedef BOOL(WINAPI *MFSCommCDM_GetBillThickness)(BYTE, LPCDMGETBILLTHICKNESS);
	
	typedef BOOL(WINAPI *MFSCommCDM_SetBillSize)(LPCDMSETBILLSIZE, LPBYTE);
	
	typedef BOOL(WINAPI *MFSCommCDM_GetBillSize)(BYTE, LPCDMGETBILLSIZE);
	
	typedef BOOL(WINAPI *MFSCommCDM_LearnBill)(BYTE, BYTE, LPCDMLEARN, LPBYTE);
	
	typedef BOOL(WINAPI *MFSCommCDM_GetDLLVersion)(LPBYTE, LPBYTE);

	typedef int(WINAPI *MFSCommCDM_TestDispense)(LPCDMCBXITEM, LPCDMMULTIDISPENSE, LPBYTE);

	typedef BOOL(WINAPI *MFSCommCDM_GetNoteLog)();

	typedef BOOL(WINAPI *MFSCommCDM_GetThickLog)();

	typedef BOOL(WINAPI *MFSCommCDM_GetRejectLog)(LPCDM_REJECT_INFO, LPBYTE);

	typedef BOOL(WINAPI *MFSCommCDM_GetDispenseLog)(LPCDMRECVDATA, LPBYTE);

	typedef BOOL(WINAPI *MFSCommCDM_UpdateFirmware)(LPCTSTR, LPBYTE, LPBYTE);

	typedef BOOL(WINAPI *MFSCommCDM_CEInfoCertification)(LPBYTE, LPBYTE);

	typedef BOOL(WINAPI *MFSCommCDM_IsSendDispense)();

	typedef BOOL(WINAPI *MFSCommCDM_GetExistCEInfo)(LPBYTE, LPBYTE, BOOL&);


	HMODULE						m_hDll;

	MFSCommCDM_OpenPort			LibMFSCommCDM_OpenPort;

	MFSCommCDM_ClosePort		LibMFSCommCDM_ClosePort;

	MFSCommCDM_Reset			LibMFSCommCDM_Reset;

	MFSCommCDM_CfgStatus		LibMFSCommCDM_CfgStatus;

	MFSCommCDM_Status			LibMFSCommCDM_Status;

	MFSCommCDM_MultiDispense	LibMFSCommCDM_MultiDispense;

	MFSCommCDM_LastDispense		LibMFSCommCDM_LastDispense;

	MFSCommCDM_Diagnostic		LibMFSCommCDM_Diagnostic;

	MFSCommCDM_SetBillThickness	LibMFSCommCDM_SetBillThickness;

	MFSCommCDM_GetBillThickness	LibMFSCommCDM_GetBillThickness;

	MFSCommCDM_SetBillSize		LibMFSCommCDM_SetBillSize;

	MFSCommCDM_GetBillSize		LibMFSCommCDM_GetBillSize;

	MFSCommCDM_LearnBill		LibMFSCommCDM_LearnBill;

	MFSCommCDM_GetDLLVersion	LibMFSCommCDM_GetDLLVersion;

	MFSCommCDM_TestDispense		LibMFSCommCDM_TestDispense;

	MFSCommCDM_GetNoteLog		LibMFSCommCDM_GetNoteLog;

	MFSCommCDM_GetThickLog		LibMFSCommCDM_GetThickLog;

	MFSCommCDM_GetRejectLog		LibMFSCommCDM_GetRejectLog;

	MFSCommCDM_GetDispenseLog	LibMFSCommCDM_GetDispenseLog;

	MFSCommCDM_UpdateFirmware	LibMFSCommCDM_UpdateFirmware;

	MFSCommCDM_CEInfoCertification	LibMFSCommCDM_CEInfoCertification;

	MFSCommCDM_IsSendDispense	LibMFSCommCDM_IsSendDispense;

	MFSCommCDM_GetExistCEInfo	LibMFSCommCDM_GetExistCEInfo;	///< Firmware에 CE부 인증 정보가 있는지 여부



	// 에러 코드
	CString m_strLastError;

	HANDLE	m_WatchStatusThread;
	BOOL	m_bThreadExit;
	DWORD	m_dwCbxStatus[MAX_CASSETTE+1];		///< +1 => reject bin
	DWORD	m_dwPrevCbxStatus[MAX_CASSETTE+1];	///< +1 => reject bin
	BOOL	m_bCbxStatusEmpty[MAX_CASSETTE];	///< 카세트의 Empty 상태 여부
	DWORD	m_dwMaxRejectCount;
	BYTE	m_byCEInfoData[CE_INFO_LENGTH+1];					///< CE부 인증 정보
	CDMLASTDISPENSE m_lastDispenseInfo;			///< 통신장애시를 위한 Last Dispense 정보

	CCriticalSection	m_CDMLock;

	BOOL	m_bCDMExecuteQuiryStatus;

public:
	int			m_nCbxCount;
	UINT		m_nCbxDenom[4];
	UINT		m_nCbxRemain[4];

	CString		m_strWDM_FW_Version;

	BOOL		m_bNeedToRejectLog;

};
