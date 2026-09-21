#pragma once

#include "AMSCtrl_Define.h"
#include "..\\..\\include\\Device\DEV_EJL_Define.h"

#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleDataManager.h"
#ifdef _WIN32_WCE
#include "../../Template/pkfuncs.h"
#endif // _WIN32_WCE

// Ping Test 관련 정보
#include <Icmpapi.h>
#pragma comment(lib, "Iphlpapi.lib")

/** **********************************************************
*	@brief	Calling convention of the EagleCE_Host.dll API.
*
*	Every Set_FlowInform/Sync_xxx/Async_xxx export in EagleCE_Host.h is
*	declared "extern "C" __declspec(dllexport)" with no calling-convention
*	keyword, i.e. __cdecl (the project has no CallingConvention override).
*	The function-pointer typedefs below were written as WINAPI (__stdcall),
*	which on WinCE/ARM compiles identically to __cdecl and so never showed a
*	problem there. On Win32/x86 __stdcall and __cdecl differ in who cleans
*	the stack; the mismatch corrupts the stack pointer on every Host API call.
************************************************************/
#ifdef _WIN32_WCE
#define EAGLE_HOST_API	WINAPI
#else
#define EAGLE_HOST_API
#endif	// _WIN32_WCE


//////////////////////////////////
// AMS의 연결 상태를 상위국에 Event로 올리기 위한 User Define
//////////////////////////////////
#define WM_AMS_NOTIFY						(WM_USER+0x3000)


//////////////////////////////////
// AMS의 진행상태 표시(인증 CMD, MAIN CMD, Additional CMD 중인지의 상태)
//////////////////////////////////
#define AMS_STATUS_INIT			0
#define AMS_STATUS_AUTH			1
#define AMS_STATUS_MAIN			2
#define AMS_STATUS_ADDITIONAL	3
#define AMS_STATUS_SEND			4	// Terminal -> AMS Server 로 먼저 전송


class CDEV_HOST
{
public:
	// 생성자
	CDEV_HOST();

	// 소멸자
	virtual ~CDEV_HOST();

	// DLL 함수
	BOOL	IsDllLoaded();
	void	HOST_Set_Flow_LineType(int CtrlType, int LineType);
	int		HOST_Sync_Send(BYTE *pSendBuffer, int nSendLen, LPCTSTR sHostInfo, LPCTSTR sHostPort, LPCTSTR sSSLOption, BOOL bAMSSend=FALSE);
	// [PCI-SSF Fix] pRecvBuffer의 실제 용량(nBufSize)을 함께 전달 - 오버플로우 방지
	int		HOST_Sync_Recv(BYTE *pRecvBuffer, int nBufSize, BOOL bAMSRecv=FALSE);
	int		HOST_Sync_ConnectTest(LPCTSTR sHostInfo, LPCTSTR sHostPort, LPCTSTR sSSLOption);
	int		HOST_Sync_PingTest(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply);
	int		HOST_Async_PreDialStart(LPCTSTR sHostInfo);
	void	HOST_Sync_PreDialCancelbyUser();
	void	HOST_Sync_CloseLine();
	
	BOOL	AMS_Sync_Communication(int nOpenType, CString strSendCommand=AMS_REQ_STATUS);
	int		AMS_Sync_ConnectOpen(LPCTSTR OpenType, LPCTSTR pDestInfo = _T(""), LPCTSTR port=_T("0"), LPCTSTR option=_T("1"));
	int		AMS_Sync_ConnectClose();
	BOOL	AMS_Async_ConnectCheck();


	// API Type Define
protected:

	//// HOST API
	typedef void(EAGLE_HOST_API *Set_FlowInform)(int, int);
	typedef int(EAGLE_HOST_API	*Sync_Send_Host)(BYTE*, int, LPCTSTR, LPCTSTR, LPCTSTR, BOOL);
	typedef int(EAGLE_HOST_API	*Sync_Recv_Host)(BYTE*, int, BOOL);
	typedef int(EAGLE_HOST_API	*Sync_ConnectTest_Host)(LPCTSTR, LPCTSTR, LPCTSTR);
	typedef int(EAGLE_HOST_API	*Sync_PingTest_Host)(LPCTSTR, ICMP_ECHO_REPLY&);
	typedef int(EAGLE_HOST_API	*Async_PreDialStart)(LPCTSTR);
	typedef int(EAGLE_HOST_API	*Sync_PreDialCancelbyUser)(void);
	typedef BOOL(EAGLE_HOST_API *Sync_CloseLine)(void);

	typedef int(EAGLE_HOST_API *Sync_ConnectOpen_AMS)(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR);
	typedef int(EAGLE_HOST_API *Sync_ConnectClose_AMS)();
	typedef BOOL(EAGLE_HOST_API *Async_ConnectCheck_AMS)();
	typedef int(EAGLE_HOST_API *Async_ConnectAccept_AMS)();


	HMODULE						m_hDll;


	// API Define
	Set_FlowInform					LibSet_FlowInform;
	Sync_Send_Host					LibSync_Send_Host;
	Sync_Recv_Host					LibSync_Recv_Host;
	Sync_ConnectTest_Host			LibSync_ConnectTest_Host;
	Sync_PingTest_Host				LibSync_PingTest_Host;
	Async_PreDialStart				LibAsync_PreDialStart;
	Sync_PreDialCancelbyUser		LibSync_PreDialCancelbyUser;
	Sync_CloseLine					LibSync_CloseLine;

	Sync_ConnectOpen_AMS			LibSync_ConnectOpen_AMS;
	Sync_ConnectClose_AMS			LibSync_ConnectClose_AMS;
	Async_ConnectCheck_AMS			LibAsync_ConnectCheck_AMS;
	Async_ConnectAccept_AMS			LibAsync_ConnectAccept_AMS;


protected:
	// AMS관련 Local 변수
	int m_nLineType;
	int m_nJournalCount;

	BOOL m_bChangedIP;
	int m_nAMSProcessStatus;	// AMS의 진행상태 표시(인증 CMD, MAIN CMD, Additional CMD 중인지의 상태)

	BYTE *m_pAMSSendBuffer;

	BYTE *m_pAMSRecvBuffer;

protected:
	int		AMS_Async_ConnectAccept();

	// AMS관련 Local 함수
	void MultiToWide(char *pMultiChar, LPWSTR pWideChar, int nLenWideChar, int nMaxLength=MAX_PATH);
	void MultiToWideEx(char *pMultiChar, CString &strOutput, int nLenWideChar, BOOL bNumeric=TRUE);
	void WideToMulti(LPCTSTR pWideChar, char *pMultiChar, int nLenMultiChar);
	void WideToMultiEx(LPCTSTR pWideChar, char *pMultiChar, int nLenMultiChar, BOOL bNumeric=TRUE, CString strInsert=_T(" "), BOOL bLeftInsert=FALSE);
	CString ConvertString(CString strData, int nLen, CString strInsert=_T(" "), BOOL bLeft=FALSE);
	BOOL AMS_Backup_LogFile(CString strDestDirectory);
	void Set_DeviceErrorCode(CString strDeviceKind, CString strErrorCode, CString strErrorMsg = _T(""));
	void Clear_DeviceError(CString strDeviceKind);
	void Clear_Error();

	BOOL AMS_Proc_Comm(int nOpenType, CString strSendCommand=AMS_REQ_STATUS);
	BOOL AMS_Proc_Requested();
	BOOL AMS_Proc_Send(CString strSendCommand);
	BOOL AMS_StatusSend();
	BOOL AMS_JournalSend();

	BOOL AMS_ProcReceiveData(CString &strCommand, CString &strSubCommand, BYTE *pbyRecvData, int &nRecvLength, BOOL bVerify=FALSE);

	BOOL AMS_VerifyData(BYTE *pbyData, int nRecvLen);
	BOOL AMS_ParseCMD(BYTE *pBuffer, int nRecvLen);

	BOOL AMS_MakeHeaderField(CString strCommand, CString strSubCommand, CString &strSendData);
	BOOL AMS_MakeCommonField(CString &strSendData);

	// Status Command('A')
	BOOL AMS_SendATMStatus(CString strCommand, CString strSubCommand);
	int AMS_MakeStatusMsg(CString strCommand, CString strSubCommand, int nLen, BYTE *pbyData);
	BOOL AMS_SendATMInquiryAddCmd(CString strCommand, CString strSubCommand);	// 추가 명령 존재 여부 조회

	// System Command('B')
	BOOL AMS_SendATMSystem(CString strCommand, CString strSubCommand);
	int AMS_MakeSystemMsg(CString strCommand, CString strSubCommand, int nLen, BYTE *pbyData);
	void AMS_ProcDeviceSystem(CString strSubCommand);

	// Journal Command('C', 'D')
 	BOOL AMS_ProcJournal(CString strCommand, CString strSubCommand, CString strJournalCount);
	BOOL AMS_SendATMJournal(CString strCommand, CString strSubCommand, FLSEJCOMMINFO CommInfo, FLSEJINFO JnlInfo);
 	int AMS_MakeJournalMsg(CString strCommand, CString strSubCommand, FLSEJCOMMINFO CommInfo, FLSEJINFO JnlInfo, int nLen, BYTE *pbyData);
 	void AMS_Get_JournalData(FLSEJCOMMINFO CommInfo, FLSEJINFO JnlInfo, CString &strSendData);

	// Configuration Setup read('F')
	BOOL AMS_SendATMSetupRead(CString strCommand, CString strSubCommand);
	int AMS_MakeSetupReadMsg(CString strCommand, CString strSubCommand, int nLen, BYTE *pbyData);
	BOOL AMS_MakeSetupSystemMsg(CString &strSendData);
	BOOL AMS_MakeSetupHostMsg(CString &strSendData);
	BOOL AMS_MakeSetupAMSMsg(CString &strSendData);
	BOOL AMS_MakeSetupBINListMsg(CString &strSendData);
	BOOL AMS_MakeSetupMessageMsg(CString &strSendData);
	BOOL AMS_MakeSetupOptionMsg(CString &strSendData);
	BOOL AMS_MakeSetupAdvertisementMsg(CString &strSendData);
	BOOL AMS_MakeSetupEMVMsg(CString &strSendData);
 
 	// Configuration Setup write('G')
	BOOL AMS_SendATMSetupWrite(CString strCommand, CString strSubCommand);
	int  AMS_MakeSetupWriteMsg(CString strCommand, CString strSubCommand, int nLen, BYTE *pbyData);
	BOOL AMS_ProcSetupWrite(BYTE *pbyData);
	//BOOL AMS_WriteSetupSystemMsg(CString *pstrRecvData);
	BOOL AMS_WriteSetupSystemMsg(CStringArray &strArrRecvData);
	//BOOL AMS_WriteSetupHostMsg(CString *pstrRecvData);
	BOOL AMS_WriteSetupHostMsg(CStringArray &strArrRecvData);
	//BOOL AMS_WriteSetupAMSMsg(CString *pstrRecvData);
	BOOL AMS_WriteSetupAMSMsg(CStringArray &strArrRecvData);
 	//BOOL AMS_WriteSetupISOMsg(CString *pstrRecvData);
	BOOL AMS_WriteSetupISOMsg(CStringArray &strArrRecvData);
 	//BOOL AMS_WriteSetupMessageMsg(CString *pstrRecvData);
	BOOL AMS_WriteSetupMessageMsg(CStringArray &strArrRecvData);
 	//BOOL AMS_WriteSetupOptionMsg(CString *pstrRecvData);
	BOOL AMS_WriteSetupOptionMsg(CStringArray &strArrRecvData);
 	//BOOL AMS_WriteSetupAdvertisementMsg(CString *pstrRecvData);
	BOOL AMS_WriteSetupAdvertisementMsg(CStringArray &strArrRecvData);
 	//BOOL AMS_WriteSetupEMVMsg(CString *pstrRecvData);
	BOOL AMS_WriteSetupEMVMsg(CStringArray &strArrRecvData);

	// SW Upload('H')
	BOOL AMS_ProcSWUpload(CString strSubCommand, BYTE *pbyData, int nRecvLen);

	BOOL AMS_ProcCbxClose();

// 	// Printer Image Upload('J')
// //	BOOL AMS_ProcImageUpload(BYTE *pbyData, int nRecvLen);
// 
	// Log View('M')
	BOOL AMS_ProcLog(CString strCommand, CString strSubCommand, CString strDate);
	BOOL AMS_SendATMLogMsg(CString strCommand, CString strSubCommand, BYTE *pbyFileData, int nFileLen, CString strFileName);

	// Camera Image Upload('N')
	BOOL AMS_ProcCAMImageUpload(CString strCommand, CString strSubCommand, CString strImgFileName);
	BOOL AMS_SendATMCAMImageMsg(CString strCommand, CString strSubCommand, BYTE *pbyFileData, int nFileLen, CString strFileName);


};
