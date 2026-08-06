// **************************************************************                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // **************************************************************
//
//				Serial Communication Class For CDM
//
//					   					Written By C.H.SHIN
//
// **************************************************************

#ifndef __CCOMMCDM_H__
#define __CCOMMCDM_H__

// -------------------------------------
//		   외부용 데이터 구조체
// -------------------------------------
//#include "DEV_CDM_Define.h"
#include "..\\..\\..\\include\\Device\\DEV_CDM_Define.h"



#define FAILED_TO_SEND_COMMAND		-1
#define FAILED_TO_RECEIVE_ACK		-2
#define FAILED_TO_SEND_ENQ			-3
#define FAILED_TO_RECEIVE_REPLY		-4
#define FAILED_TO_SEND_ACK			-5
#define WFS_HARDWARE_ERROR			-14

#define CE_INFO_LENGTH		32
#define MAX_RECV_DATA_LENGTH	252

// -------------------------------------
//		   내부용 데이터 구조체
// -------------------------------------
typedef enum {
	RX_NONE = 0,
	RX_ACK,
	RX_NAK,
	RX_RESPONSE,
//	RX_RESPONSE_WITH_TEXT,	// 미사용함
	RX_ERROR
} CDMRXTYPE;

typedef struct _cdm_comm_trace {
//	CHAR txCommand[10];				// TX COMMAND
	BYTE txCommand[256];			// TX COMMAND
	UINT lenCommand;				// TX COMMAND LEN

	BYTE rxText[1000];				// Rx Text(RESET COMMAND)
	UINT lenText;					// RX TEXT LENGTH

	BYTE rxResponse[256];			// RX RESPONSE
	UINT lenResponse;				// RX COMMAND LENGTH
} CDMCMDTRACE, *LPCDMCMDTRACE;

// learn command 수행 결과 구조체
typedef struct _cdm_bill_learn_result {
	BYTE error;

	BYTE reject;					// combination is possible
	BYTE avg_size;
	BYTE avg_thickness;
} CDMBILLLEARNRESULT, *LPCDMBILLLEARNRESULT;


// *************************************
//			 CLASS DECLARATION
// *************************************



class CCommCdm {

public :
	static CCommCdm* m_pInstance;
	static void CreateInstance();
	static void ReleaseInstance();
	static CCommCdm* GetInstance();

	BOOL GetDLLversion(LPBYTE major, LPBYTE minor);
	CCommCdm();
	~CCommCdm();

	// ------------------------------
	//		    PORT SETTING
	// ------------------------------
	BOOL OpenPort(HWND hWnd, UINT nPortNum, int nWriteLogLevel);
	BOOL ClosePort();

	// ------------------------------
	//		 INTERFACE WITH CDM
	// ------------------------------
	BOOL Reset();
	BOOL CfgStatus(LPCDMCFGSTATUS lpCfgStatus, LPBYTE lpbyErrorCd);
	BOOL Status(LPCDMSTATUS lpStatus);
	int MultiDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult, LPBYTE lpbyCEInfo);
	BOOL LastDispense(LPCDMLASTDISPENSE lpResult, LPBYTE lpbyErrorCd);
	BOOL Diagnostic(LPCDMDIAGNOSTIC lpResult);

	BOOL SetBillThickness(LPCDMSETBILLTHICKNESS lpSet, LPBYTE lpbyErrorCd);
	BOOL GetBillThickness(BYTE byCbxNo, LPCDMGETBILLTHICKNESS lpGet);

	BOOL SetBillSize(LPCDMSETBILLSIZE lpSet, LPBYTE lpbyErrorCd);
	BOOL GetBillSize(BYTE byCbxNo, LPCDMGETBILLSIZE lpGet);

	BOOL Learn(BYTE byCbxNo, BYTE byItemCount, LPCDMLEARN lpResult, LPBYTE lpbyCEInfo); 
	int TestiDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult, LPBYTE lpbyCEInfo);
	BOOL CheckFirmware(LPSTR szPath, LPBYTE lpbyErrorCd);	// for LM(SP) just return OK
	BOOL UpdateFirmware(LPCTSTR szPath, LPBYTE lpbyErrorCd, LPBYTE lpbyCEInfo);	// Compare version and do update
	BOOL GetRejectLog(LPCDM_REJECT_INFO lpRejectLog, LPBYTE lpbyErrorCd);
	BOOL GetRandomNumber(LPBYTE lpbyCEInfo);
	BOOL GetDispenseLog(LPCDMRECVDATA lpRecvData, LPBYTE lpbyErrorCd);
	BOOL CEInfoCertification(LPBYTE lpbyCEInfo, LPBYTE lpbyErrorCd);
	BOOL IsSendDispense();
	BOOL GetExistCEInfo(LPBYTE lpbyCEInfo, LPBYTE lpbyErrorCd, BOOL& bExist);

	BOOL SaveLastTransInfoData(CString strKey, CString strValue);

	void StartWarningScreen();
	void EndWarningScreen();

	HANDLE m_hWarning;

	
private :
	// cdm-related declaration
	static const BYTE CC_STX;
	static const BYTE CC_ETX;
	static const BYTE CC_ENQ;
	static const BYTE CC_ACK;
	static const BYTE CC_NAK;

	static const BYTE CMD_RESET;
	static const BYTE CMD_STATUS;
	static const BYTE CMD_DIAGNOSTIC;
	static const BYTE CMD_DISPENSE;
	static const BYTE CMD_MULTI_DISPENSE;
	static const BYTE CMD_LASTSTATUS;
	static const BYTE CMD_CONFIG;
	static const BYTE CMD_SET_THICK;
	static const BYTE CMD_GET_THICK;
	static const BYTE CMD_SET_SIZE;
	static const BYTE CMD_GET_SIZE;
	static const BYTE CMD_LEARN;
	static const BYTE CMD_TEST_DISPENSE;
	static const BYTE CMD_GET_REJECTLOG;
	static const BYTE CMD_GET_RANDOM_NUMBER;
	static const BYTE CMD_CE_CERTIFICATION;
	static const BYTE CMD_GET_DISPENSELOG;
	static const BYTE CMD_EXIST_CE_INFO;

	HWND	m_hWnd;
	OVERLAPPED	m_osRead, m_osWrite;	// 포트 파일 Overlapped structure
	HANDLE	m_hPort;
	int		m_nPortNum;		// V02.12

	BOOL m_bDownloadMode;	// V02.12
	BOOL m_bOpen;
	DWORD m_dwTimeout;

	CDMCMDTRACE m_cmdTrace;
	BYTE m_byPreStatus[100];	// V02.15: 이전 Status값
	
	CDMBILLLEARNRESULT m_cmdLearnResult;				// Learn 명령 실행 결과 구조체

	BYTE m_byRandomNumber[2];	// RN 암호화 DATA
	BYTE m_byOrgCommand;		// 평문에서의 cmd
	BYTE m_byRecvCommand;		// 응답받은 cmd
	BOOL m_bSendDispense;		// 방출 동작 중 통신장애 발생 여부
	int m_nWriteLogLevel;			// 송/수신 전문 로그 여부
	BYTE m_byUniqueKey[CE_INFO_LENGTH+1];
	BYTE m_TempDesKey[17];
	BOOL m_bCEInfoError;


	BYTE GetCheckSum(BYTE *lpText, UINT nStartPos, UINT nEndPos);
	int HandShake();
	CDMRXTYPE GetAction(BYTE *lpBuf);
//	BOOL HandShake(LPCDMRECVDATA lpRecvData);					// 대용량 Recv Data를 위한 함수
	BOOL CommandSend(BYTE *ptr, BYTE length);	// V02.12
	BOOL GetDispenseLogData(LPCDMRECVDATA lpRecvData);

	void AnalyzeGeneralCommand(BYTE cCommand, LPVOID lpParam);
	void AnalyzeE2PROMCommand(BYTE cCommand, LPVOID lpParam);

	void SetTimeout(UINT nSecond);
	BOOL ReadPort(BYTE *lpBuf, DWORD nByteToRead);
	BOOL WritePort(LPCSTR lpBuf, DWORD nByteToWrite);
	
	// Encryption
// 	BYTE ExcuteKeyTable(BYTE byData, int nKeyIndex);	// 전문 암/복호화
// 	BYTE GetEncryptKeyIndex(BYTE &byRealKeyIndex);
// 	BYTE GetDecryptKeyIndex(BYTE byEncrytKeyIndex);
// 	void ExcuteDecryptData();

	BOOL GetFWVersionInHeader(CString strFilePath, CString& strHeaderVersion);

	// T-DES 암호화
	void EncryptKeyData(BYTE *pbyKey);
	void DecryptKeyData(BYTE *pbyKey);
	BYTE MakePack(BYTE bySrc);	// Make Pack
	void ConvertStringToHex(CString strSrc, BYTE *pByDest);	// Convert from String to Hex
	BYTE GetDummyRandomNumber();
	void GetCheckSumForCryptoKey(BYTE *pbyOutData, BYTE *pbyKey);
	//////////////////////////////////////////
};

#endif	// __CCOMMCDM_H__