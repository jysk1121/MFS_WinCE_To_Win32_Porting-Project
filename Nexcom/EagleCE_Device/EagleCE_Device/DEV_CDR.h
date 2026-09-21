#pragma once

#include <afxmt.h>

#include "DEV_CDR_Define.h"

class CDEV_CDR
{
public:
	// 생성자
	CDEV_CDR();
	// 소멸자
	virtual ~CDEV_CDR();

	// Is DLL Loaded
	BOOL IsDllLoaded(void);

	// 열기
	BOOL Open(HWND hWnd, int nPortNum, int nTimeout);
	
	// 닫기
	BOOL Close();

	// 초기화
	BOOL Initialize();
	
	// LED 설정
	BOOL SetLed(BOOL bOn);
	
	// 카드 투입
	BOOL InsertCard();

	// Give up waiting for a card - Win32 simulator only, no-op on real
	// hardware. Called when the ATM leaves in-service while InsertCard() is
	// still being polled, so the "Card Reader" panel does not linger on
	// screen after the terminal has moved on.
	void CancelInsertCard();

	// Bracket the "Take your card" wait - Win32 simulator only, no-op on real
	// hardware. On a real terminal the customer physically pulls the card and
	// the sensor clears; the simulator needs a button to stand in for that.
	void ShowRemoveCardWait(BOOL bShow);

	// IC Power On
	BOOL ICCPowerOn(byte * ATR, DWORD &dwDataSize);

	// IC Power Off
	BOOL ICCPowerOff();
	
	// 카드 제거
	BOOL RemoveCard();

	// 카드 읽기 (ISO1)
	int ReadCardData_ISO1(CString* pISO1);

	// 카드 읽기 (ISO2)
	int ReadCardData_ISO2(CString* pISO2);
	
	// 카드 읽기 (ISO3)
	int ReadCardData_ISO3(CString* pISO3);

	// 버퍼 클리어
	BOOL ClearBuffer();

	// 취소
	BOOL Cancel();

	// Latch 설정 확인
	BOOL ReadLatch(BYTE *mode);

	// Latch 설정
	BOOL SetLatch(BOOL bOn);

	// Reset Command
	BOOL DeviceReset();

	// 에러코드 취득
	CString GetLastError(void);

	// 에러코드 clear
	void ClearErrorCode(void);
	
	// 통신 에러 설정
	void SetCommError(DWORD dwRetCode);
	
	// DLL 버전 취득
	BOOL DLLInformation(LPDLL_INFORMATION lpdll_information);
	
	// FW 버전 취득
	BOOL GetVersionInfo();
	
	// Card Media Status 취득
	BYTE GetMediaStatus();

	DWORD GetReplyType();

	DWORD GetReturnValue();

	BOOL GetCDR_DeviceStatus();

	// EMV Function
	BOOL IsEMV_Enable();

	void Load_AIDs_InformFromFile();

	void Save_AIDs_InformToFile();

	void Load_US_Common_AIDs_InformFromFile();

	void Init_EMVKernel_Varialbe();

	int	 EMV_SelectApplication();
	
	int	 EMV_MakeAppLists();

	void EMV_RemoveAppList(int nAIDIndex);

	void EMV_Set_SelectAID_Index(int nIndex);

	int	 EMV_FinalApplication();

	int  EMV_StoreValFromRecord();

	int  EMV_InitApplication();

	int  EMV_ReadAppData();

	void EMV_StoreTransValues();

	int  EMV_OfflineDataAuth();

	int  EMV_ProcessRestrict();

	int  EMV_CardholderVerify();

	int  EMV_TerminalRiskMgmt();

	BYTE EMV_TerminalActionAnalysis();

	int  EMV_CardActionAnalysis(unsigned char *OnlineFlag);

	int  EMV_OnlineProcess(unsigned char Acquirer_CID);

	int  EMV_Completion();

	int  EMV_TLV_GetValue(unsigned char *tag, int *len, unsigned char *val);

	int  EMV_TLV_StoreValue(unsigned char *tag, int len, unsigned char*val);

	CString Get_BrandNameForADA(int nIndex);

	// 암호화 인증을 위한 사전 작업(키 교환)
	BOOL GenerateKeyofDeviceAttestationDataAndKey();		// Key4MagneticData 와 Key4APDUData 생성을 위한 키 교환

	BOOL GenerateKey4MagneticData();	// MagneticData 암/복호화를 위한 키 교환

	BOOL GenerateKey4APDUData();		// APDUData 암/복호화를 위한 키 교환

	void GenerateKeyInfoAndAuthData();	// 처음 암호화 Section 연결시, 필요한 key 정보 및 검증 data 생성

	// [PCI-SSF Fix] CryptGenRandom(CSPRNG) ??? ?????? ???? ????? ???? - srand(GetTickCount())+rand() ???
	BOOL GenerateSecureRandomBytes(BYTE *pbyOutput, DWORD dwLen);

	// 암호화
	void EncryptKeyData_TDES_ECB(BYTE *pbyInput, int nInputLen, BYTE *pbyOutput, int &nOutputLen, BYTE *pbyKey);

	void EncryptKeyData_TDES_CBC(BYTE *pbyInput, int nInputLen, BYTE *pbyOutput, int &nOutputLen, BYTE *pbyKey, BYTE *byEncIV);

	void DecryptKeyData_TDES_ECB(BYTE *pbyInput, int nInputLen, BYTE *pbyOutput, BYTE *pbyKey);

	void DecryptKeyData_TDES_CBC(BYTE *pbyInput, int nInputLen, BYTE *pbyOutput, BYTE *pbyKey, BYTE *byEncIV);

	int ParsingReadData(BYTE *pbyInput, int nInputLen);

	BOOL GetSupportEncryption();

	BOOL IsUnSuccessReply();		///< 준정상코드인지 판단

public:

	int						m_nCandidateList;
	int						m_nSelectedAID;
	BOOL					m_bOnlineProcessing;
	BOOL					m_bUnknownAID;
	BOOL					m_bShowAIDList;

	CString					m_strCDR_FW_Version;

	EMV_LIST_OF_AID			m_sAIDList;
	EMV_ICC_APPS_INFORM		m_sIccAppInform;
	EMV_AID_CONFIG_DATA		m_sEMV_Config_Data;
	EMV_US_COMMON_AIDLIST	m_sEMV_US_Common_AID_List;


protected:
	typedef DWORD(WINAPI *DLL_GETDLLINFORMATION)(LPDLL_INFORMATION);
	typedef DWORD(WINAPI *DLL_CONNECTDEVICE)(LPCTSTR, CONST DWORD);
	typedef DWORD(WINAPI *DLL_DISCONNECTDEVICE)(LPCTSTR);
	typedef DWORD(WINAPI *DLL_CANCELCOMMAND)(LPCTSTR);
	typedef DWORD(WINAPI *DLL_EXECUTECOMMAND)(LPCTSTR, LPCOMMAND, CONST DWORD, LPREPLY);
	typedef DWORD(WINAPI *DLL_UPDATEFIRMWARE)(LPCTSTR, CONST DWORD, LPCTSTR, CONST BOOL, CALL_BACK_FUNCTION, LPDWORD);
	typedef DWORD(WINAPI *DLL_ICCARDTRANSMIT)(LPCTSTR, CONST DWORD, LPBYTE, CONST DWORD, LPDWORD, LPBYTE, LPDWORD, LPREPLY);
	typedef DWORD(WINAPI *DLL_SAMTRANSMIT)(LPCTSTR, CONST DWORD, LPBYTE, CONST DWORD, LPDWORD, LPBYTE, LPDWORD, LPREPLY);

	HMODULE					m_hDLL;
	DLL_GETDLLINFORMATION	DLL_GetDllInformation;
	DLL_CONNECTDEVICE		DLL_ConnectDevice;
	DLL_DISCONNECTDEVICE	DLL_DisconnectDevice;
	DLL_CANCELCOMMAND		DLL_CancelCommand;
//	DLL_EXECUTECOMMAND		DLL_ExecuteCommand;
	DLL_UPDATEFIRMWARE		DLL_UpdateFirmware;
	DLL_ICCARDTRANSMIT		DLL_ICCardTransmit;
	DLL_SAMTRANSMIT			DLL_SAMTransmit;

	typedef int		(*DLL_EMVKernel_AppSelection)(unsigned char SupportPSE, EMV_LIST_OF_AID *AIDList);
	typedef int		(*DLL_EMVKernel_MakeAppLists)(unsigned char SupportUserConfirm, EMV_ICC_APPS_INFORM *IccAppInform, int *nAppCnt);
	typedef void	(*DLL_EMVKernel_RemoveAppList)(USHORT AidLen, unsigned char *AIDVal, int *nAppCnt);
	typedef int		(*DLL_EMVKernel_FinalAppSelection)(USHORT AdiLen, unsigned char *AIDVal, int *nAppCnt, unsigned char *SW);
	typedef int		(*DLL_EMVKernel_StoreValFromRecord)(unsigned char *record, int type, int inlen, int source);
	typedef int		(*DLL_EMVKernel_InitApplication)(int *nAppCnt);
	typedef int		(*DLL_EMVKernel_ReadAppData)(void);
	typedef int		(*DLL_EMVKernel_OfflineDataAuth)(void);
	typedef int		(*DLL_EMVKernel_ProcessRestrict)(void);
	typedef int		(*DLL_EMVKernel_CardholderVerify)(int OnlinePinLen, unsigned char *EncipheredOnlinePin);
	typedef int		(*DLL_EMVKernel_TerminalRiskMgmt)(unsigned char SupportForceOnline);
	typedef byte	(*DLL_EMVKernel_TerminalActionAnalysis)(void);
	typedef int		(*DLL_EMVKernel_CardActionAnalysis)(unsigned char *OnlineFlag);
	typedef int		(*DLL_EMVKernel_OnlineProcess)(unsigned char Acquirer_CID);
	typedef int		(*DLL_EMVKernel_Completion)(void);

	typedef int		(*DLL_EMVKernel_CDRCommFunc)(int (CALLBACK *EMV_Command)(BYTE* SendData, DWORD SendLength, BYTE* RecvData, DWORD* RecvLen));
//	typedef void	(*DLL_EMVKernel_random_number)(void);
	typedef int		(*DLL_EMVKernel_Tlv_GetVal)(unsigned char *tag, int *len, unsigned char *val);
	typedef int		(*DLL_EMVKernel_Tlv_StoreVal)(unsigned char *tag, int len, unsigned char*val);
	typedef void	(*DLL_EMVKernel_Initial_Variable)(void);
	typedef void	(*DLL_EMVKernel_SetDebug)(BYTE mode);
	typedef void	(*DLL_EMVKernel_SetKernelLogPath)(unsigned char*FilePath,unsigned char *ExtensionName);

	HMODULE					m_hEMVDLL;

	DLL_EMVKernel_AppSelection						EMVKernel_AppSelection;
	DLL_EMVKernel_MakeAppLists						EMVKernel_MakeAppLists;
	DLL_EMVKernel_RemoveAppList						EMVKernel_RemoveAppList;
	DLL_EMVKernel_FinalAppSelection					EMVKernel_FinalAppSelection;
	DLL_EMVKernel_StoreValFromRecord				EMVKernel_StoreValFromRecord;
	DLL_EMVKernel_InitApplication					EMVKernel_InitApplication;
	DLL_EMVKernel_ReadAppData						EMVKernel_ReadAppData;
	DLL_EMVKernel_OfflineDataAuth					EMVKernel_OfflineDataAuth;
	DLL_EMVKernel_ProcessRestrict					EMVKernel_ProcessRestrict;
	DLL_EMVKernel_CardholderVerify					EMVKernel_CardholderVerify;
	DLL_EMVKernel_TerminalRiskMgmt					EMVKernel_TerminalRiskMgmt;
	DLL_EMVKernel_TerminalActionAnalysis			EMVKernel_TerminalActionAnalysis;
	DLL_EMVKernel_CardActionAnalysis				EMVKernel_CardActionAnalysis;
	DLL_EMVKernel_OnlineProcess						EMVKernel_OnlineProcess;
	DLL_EMVKernel_Completion						EMVKernel_Completion;

	DLL_EMVKernel_CDRCommFunc						EMVKernel_CDRCommFunc;
//	DLL_EMVKernel_random_number						EMVKernel_random_number;
	DLL_EMVKernel_Tlv_GetVal						EMVKernel_Tlv_GetVal;
	DLL_EMVKernel_Tlv_StoreVal						EMVKernel_Tlv_StoreVal;
	DLL_EMVKernel_Initial_Variable					EMVKernel_Initial_Variable;
	DLL_EMVKernel_SetDebug							EMVKernel_SetDebug;
	DLL_EMVKernel_SetKernelLogPath					EMVKernel_SetKernelLogPath;

	DWORD m_dwReplyType;

	DWORD m_dwReturnValue;

	int	 m_nCDRStatus;

	CCriticalSection	m_CDRLock;

public:

	// 포트 번호
	TCHAR m_szPortNum[10];

	// Timeout
	int m_nTimeout;

	// 에러 코드
	CString m_strLastError;

	DLL_EXECUTECOMMAND		DLL_ExecuteCommand;

	BYTE m_byKeyExchangeKey[ENCRYPTION_KEY_LENGTH];

	BYTE m_byDeviceAuthData[ENCRYPTION_KEY_LENGTH];

	BYTE m_byMagkeyData[GENERATED_KEY_DATA_LENGTH];

	BYTE m_byAPDUkeyData[GENERATED_KEY_DATA_LENGTH];

	BOOL m_bKeyAuthError;


	static BYTE m_byTempData[1024];

	BOOL m_bSupportEncrypt4CDR;

	COMMAND	m_Cmd;		// Card Read Command Buffer

	REPLY	m_Reply;	// Card Read Receive Buffer
};