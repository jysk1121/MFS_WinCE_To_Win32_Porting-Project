#pragma once

#include "EPPDef.h"

#define EPP_OK 0       
// Operation failed for an unspecified reason.
#define EPP_ERROR					0x80000002
#define EPP_COM_PORT_NOT_OPEN		0x80000003
#define EPP_EXCEPTION_CALLED		0x80000004
#define EPP_DRIVER_NOT_READY		0x80000005
#define EPP_BRIDGE_NOT_OPEN  		0x80000006
#define EPP_BLOCKED_BY_SERVICE		0x80000007
#define EPP_SC_INIT_FAILED          0x80000008
#define EPP_SC_PROTOCOL_FAILED      0x80000009
#define EPP_SC_WHITE_LIST_FAILED    0x8000000A
#define EPP_ALREADY_INITIALIZED     0x8000000B
#define EPP_IN_USE_BY_OTHER_PROCESS 0x8000000C

/* values of WFSPINFUNCKEYDETAIL.ulFuncMask */

#define WFS_PIN_FK_0                                (0x00000001)
#define WFS_PIN_FK_1                                (0x00000002)
#define WFS_PIN_FK_2                                (0x00000004)
#define WFS_PIN_FK_3                                (0x00000008)
#define WFS_PIN_FK_4                                (0x00000010)
#define WFS_PIN_FK_5                                (0x00000020)
#define WFS_PIN_FK_6                                (0x00000040)
#define WFS_PIN_FK_7                                (0x00000080)
#define WFS_PIN_FK_8                                (0x00000100)
#define WFS_PIN_FK_9                                (0x00000200)
#define WFS_PIN_FK_ENTER                            (0x00000400)
#define WFS_PIN_FK_CANCEL                           (0x00000800)
#define WFS_PIN_FK_CLEAR                            (0x00001000)
#define WFS_PIN_FK_BACKSPACE                        (0x00002000)
#define WFS_PIN_FK_HELP                             (0x00004000)
#define WFS_PIN_FK_DECPOINT                         (0x00008000)
#define WFS_PIN_FK_00                               (0x00010000)
#define WFS_PIN_FK_000                              (0x00020000)
#define WFS_PIN_FK_RES1                             (0x00040000)
#define WFS_PIN_FK_RES2                             (0x00080000)
#define WFS_PIN_FK_RES3                             (0x00100000)
#define WFS_PIN_FK_RES4                             (0x00200000)
#define WFS_PIN_FK_RES5                             (0x00400000)
#define WFS_PIN_FK_RES6                             (0x00800000)
#define WFS_PIN_FK_RES7                             (0x01000000)
#define WFS_PIN_FK_RES8                             (0x02000000)
#define WFS_PIN_FK_OEM1                             (0x04000000)
#define WFS_PIN_FK_OEM2                             (0x08000000)
#define WFS_PIN_FK_OEM3                             (0x10000000)
#define WFS_PIN_FK_OEM4                             (0x20000000)
#define WFS_PIN_FK_OEM5                             (0x40000000)
#define WFS_PIN_FK_OEM6                             (0x80000000)

/* additional values of WFSPINFUNCKEYDETAIL.ulFuncMask */

#define WFS_PIN_FK_UNUSED                           (0x00000000)

#define WFS_PIN_FK_A                                WFS_PIN_FK_RES1
#define WFS_PIN_FK_B                                WFS_PIN_FK_RES2
#define WFS_PIN_FK_C                                WFS_PIN_FK_RES3
#define WFS_PIN_FK_D                                WFS_PIN_FK_RES4
#define WFS_PIN_FK_E                                WFS_PIN_FK_RES5
#define WFS_PIN_FK_F                                WFS_PIN_FK_RES6
#define WFS_PIN_FK_SHIFT                            WFS_PIN_FK_RES7

/* values of WFSPINFDK.ulFDK */

#define WFS_PIN_FK_FDK01                            (0x00000001)
#define WFS_PIN_FK_FDK02                            (0x00000002)
#define WFS_PIN_FK_FDK03                            (0x00000004)
#define WFS_PIN_FK_FDK04                            (0x00000008)
#define WFS_PIN_FK_FDK05                            (0x00000010)
#define WFS_PIN_FK_FDK06                            (0x00000020)
#define WFS_PIN_FK_FDK07                            (0x00000040)
#define WFS_PIN_FK_FDK08                            (0x00000080)
#define WFS_PIN_FK_FDK09                            (0x00000100)
#define WFS_PIN_FK_FDK10                            (0x00000200)
#define WFS_PIN_FK_FDK11                            (0x00000400)
#define WFS_PIN_FK_FDK12                            (0x00000800)
#define WFS_PIN_FK_FDK13                            (0x00001000)
#define WFS_PIN_FK_FDK14                            (0x00002000)
#define WFS_PIN_FK_FDK15                            (0x00004000)
#define WFS_PIN_FK_FDK16                            (0x00008000)
#define WFS_PIN_FK_FDK17                            (0x00010000)
#define WFS_PIN_FK_FDK18                            (0x00020000)
#define WFS_PIN_FK_FDK19                            (0x00040000)
#define WFS_PIN_FK_FDK20                            (0x00080000)
#define WFS_PIN_FK_FDK21                            (0x00100000)
#define WFS_PIN_FK_FDK22                            (0x00200000)
#define WFS_PIN_FK_FDK23                            (0x00400000)
#define WFS_PIN_FK_FDK24                            (0x00800000)
#define WFS_PIN_FK_FDK25                            (0x01000000)
#define WFS_PIN_FK_FDK26                            (0x02000000)
#define WFS_PIN_FK_FDK27                            (0x04000000)
#define WFS_PIN_FK_FDK28                            (0x08000000)
#define WFS_PIN_FK_FDK29                            (0x10000000)
#define WFS_PIN_FK_FDK30                            (0x20000000)
#define WFS_PIN_FK_FDK31                            (0x40000000)
#define WFS_PIN_FK_FDK32                            (0x80000000)


typedef int (*EventCallBackFunction)(unsigned short usEventID, byte *lpByte, unsigned short usDataLength);

#define		ENTRY_KEY_STATUS_NONE		0
#define		ENTRY_KEY_STATUS_ENABLE		1
#define		ENTRY_KEY_STATUS_DISABLE	2
#define		ENTRY_KEY_STATUS_ENCRYPT	3

/** **********************************************************
*	@brief Define EPP State
************************************************************/
#define		EPP_STATE_REMOVED		1
#define		EPP_STATE_PREACTIVATED	2
#define		EPP_STATE_ACTIVATED		3

typedef struct tag_RKL_Terminal_Info 
{
	void	Clear(void);

	CString strEPP_SerialNo;
	CString strEPP_RanDomNumber;

}RKL_TERMINAL_INFO;



class CCrypteraComm
{
public:
	CCrypteraComm(void);
	~CCrypteraComm(void);

	int OpenDevice(int nPortNum, DWORD dwBaudRate);

	int CloseDevice();
	int ExecuteCommand(int nCommand, LPBYTE pData, DWORD dwLen, LPBYTE pResp, DWORD &dwRet, BOOL bCheckStatus=TRUE, BOOL bWaitResponse=TRUE);
	int RegistEventCallBackFunction(EventCallBackFunction pFunc);
	int GetCommandResultData(LPBYTE lpData, int nBuffLeng, int* nLength);
	
	int		StartEntryClear();
	BOOL	StartEntryReadPin();
	CString	MakePinBlock(CString strAccountNo);
	int		StopEntry();
	int		GetVersionInfo();

	int		GetDeviceStatus();
	BOOL	SetActivate(int nActiveCmd, CString strUserID, CString strActivationCode);
	BOOL	Remove_RegisteredMasterKey(CString strMasterKeyName);
	int		GetKeyBlockHeaderData(KEYBLOCKHEADER *pHeader, int nType = 0);
	BOOL	StartEntry_Master_PartKey();
	BOOL	Store_Master_PartKey();
	BOOL	Store_Master_FinalKey();

	BOOL	Store_PIN_WorkingKey(CString strPINWorkingKey);

	CString	Get_KeyCheckValue(CString strKeyName);

	CString	GetEPPSerialNo();

	CString GetEPPNonCEValue(int nType = 0);

	int		SetRegisterValue(CString strRegisterName, CString strValue);
	CString GetRegisterValue(CString strRegisterName);

	CString m_strLastError;

	// 에러코드 취득
	CString GetLastError(void);

	unsigned int GetEPPCertificate(char* szCertVal, unsigned short usBufferSize, unsigned short *usCertSize);
	unsigned int LoadHostCertificate(char* szCertVal, unsigned short usCertSize);
	unsigned int LoadRSAEncryptedKey4RKL(char *szKeyBlock, unsigned short usKeyBlocksize, char *szReceiptMsg, unsigned short *usReceiptMsgSize);

	int		m_nEntryKeyStatus;

	int		m_nEPPState;

	int		m_bRemovalSensorStatus;

	BOOL	m_bEPPReady;

	BOOL	m_bEPPStuckState;

	BOOL	m_bPINMasterKey_Injected;

	BOOL	m_bPINWorkingKey_Injected;

	BOOL	m_bMACMasterKey_Injected;

	BOOL	m_bMACWorkingKey_Injected;

	CString m_strREPP_FW_Version;

	RKL_TERMINAL_INFO	m_sRKL_TerminalInfo;

private:
	HINSTANCE	m_hDLL;
};
