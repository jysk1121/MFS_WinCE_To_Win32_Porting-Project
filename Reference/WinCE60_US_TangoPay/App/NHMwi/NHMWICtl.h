#if !defined(AFX_NHMWICTL_H__F2D0FAA2_5575_4DAF_90AA_5E12319C224B__INCLUDED_)
#define AFX_NHMWICTL_H__F2D0FAA2_5575_4DAF_90AA_5E12319C224B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\H\Common\CmnLib.h"

// NHMWICtl.h : Declaration of the CNHMWICtrl ActiveX Control class.

/////////////////////////////////////////////////////////////////////////////
// CNHMWICtrl : See NHMWICtl.cpp for implementation.
/************************************************************************/
/* Common Header                                                        */
/************************************************************************/
#include "..\H\Common\CommDef.h"
#include "..\H\Common\ConstDef.h"
#include "..\H\Common\SPInform.h"
//#include "..\H\Common\MB2500DLL.h"			// [#573] NH AIREAT 2009.10.26 SW NVRAM
#include ".\DevCtrl\DevCtrl.h"

/************************************************************************/
/* Nextware Header                                                      */
/************************************************************************/
#include ".\AxNext\NxCardreaderx.h"
#include ".\AxNext\NxPinx.h"
#include ".\AxNext\NxReceiptprinterx.h"
#include ".\AxNext\NxJournalprinterx.h"
#include ".\AxNext\NxCashdispenserx.h"
#include ".\AxNext\NxAuxiliariesx.h"
#include ".\AxNext\NxDoorsx.h"
#include ".\AxNext\NxGuidlightsx.h"
#include ".\AxNext\NxIndicatorsx.h"
#include ".\AxNext\NxSensorsx.h"
#include ".\AxNext\NxTracex.h"
#include ".\AxNext\Nxrfidreaderx.h"			// [#2325] NH KSK 2015.01.20
#include ".\AxNext\nxcashacceptorx.h"		// [#GLDV-3005] US Kook 2021.10.25 Support Side Car
#include ".\AxNext\nxscannerprinterx.h"		// [#GLDV-3005] US Kook 2021.10.27 Support Side Car

//////////////////////////////////////////////////////////////////////////

class CNHMWICtrl : public COleControl
{
	DECLARE_DYNCREATE(CNHMWICtrl)

// Constructor
public:
	CNHMWICtrl();

	CDevCtrl*			m_pDevCtrl;								// CDevCtrl Class Pointer
	CMutex				m_mutex;								// For CheckDeviceAction

//////////////////////////////////////////////////////////////////////////
	CNXCardReaderX		m_pMcu;
	CNXRFIDReaderX		m_pRFID;				// [#2325] NH KSK 2015.01.20
	CNXPinX				m_pPin;
	CNXReceiptPrinterX	m_pSpr;
//-NOUSE	CNXJournalPrinterX	m_pJpr;			// AIREAT 2009.07.14
	CNXCashDispenserX	m_pCdu;
	CNXDoorsX			m_pDoor;
	CNXGuidLightsX		m_pLight;
	CNXIndicatorsX		m_pInd;
	CNXSensorsX			m_pSensor;
//-NOUSE	CNXTraceX			m_pTrace;		// AIREAT 2009.07.14

	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	CNXScannerPrinterX	m_pBcr;
	CNXCashAcceptorX	m_pBna;
	// end of [#GLDV-3005]

//////////////////////////////////////////////////////////////////////////

// Inner Variables
public:

	int					m_bMcuEntry;							// MCU에 Entry가 Enable되었는지 여부
	int					m_bPinEntry;							// PINPAD Entry Flag				2004.04.02
	int					m_bRFIDEntry;							// RFID에 Entry가 Enable되었는지 여부	[#2325] NH KSK 2015.01.20

	int					m_nErrorDevice;							// 디바이스 장애발생시 장애디바이스를 설정
	int					m_nErrorType;							// 장애형태(DeviceError/FatalError/ETC)

	int					m_nTimeoutDevice;						// 디바이스 Timeout시 설정

	int					m_nEventDevice;							// 디바이스 이벤트시 설정
	int					m_nStatusDevice;						// 디바이스 상태시 설정
	
	int					m_nSprExist;							// 명세표잔류여부(잔류/동작여부) 논리플래그
	int					m_nJprExist;							// 저널 잔류여부(저널 동작여부) 논리플래그
	int					m_nMcuExist;							// 카드잔류여부(잔류/동작여부) 논리플래그

	CString				m_strICData;							// IC카드 수신데이타
	CString				m_strBarcodeData;						// Barcode Data					[#GLDV-3005] US Kook 2022.01.06 Side Car

	int					m_bDoorOpened;							// 뒷문부오픈여부(초기값:FALSE) 
	int					m_bJprOpened;							// 저널부오픈여부(초기값:FALSE) 
	int					m_bLightOpened;							// LIGHT부오픈여부(초기값:FALSE) 
	int					m_bMcuOpened;							// 카드부오픈여부(초기값:FALSE) 
	int					m_bSprOpened;							// 명세표부오픈여부(초기값:FALSE) 
	int					m_bUpsOpened;							// 전원부오픈여부(초기값:FALSE) 
	int					m_bCduOpened;							// CDU오픈여부(초기값:FALSE) 
	int					m_bPinOpened;							// PINPAD오픈여부(초기값:FALSE) 
	int					m_bSensorOpened;						// SENSOR오픈여부(초기값:FALSE)
	int					m_bRFIDOpened;							// RFID오픈여부(초기값:FALSE)	[#2325] NH KSK 2015.01.20

	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	int					m_bBcrOpened;
	int					m_bBnaOpened;
	// end of [#GLDV-3005]

	BOOL				m_bRefused;		// [RWC6-676] SKKim 2024.05.27 Accept중 Refuse 발생시 사용하는 변수 추가

																// 장치디바이스 오픈 이벤트 수신여부 플래그 : 2005.03.30
	int					m_bDoorOpenedEvent;						// 뒷문부오픈이벤트수신여부(초기값:FALSE) 
	int					m_bLightOpenedEvent;					// LIGHT부오픈이벤트수신여부(초기값:FALSE) 
	int					m_bIndOpenedEvent;						// INDICATOR부오픈이벤트수신여부(초기값:FALSE) 
	int					m_bSensorOpenedEvent;					// SENSOR오픈이벤트수신여부(초기값:FALSE) 

																// 장치디바이스 클로우즈 이벤트 수신여부 플래그 : 2005.03.30
	int					m_bDoorClosedEvent;						// 뒷문부클로우즈이벤트수신여부(초기값:FALSE) 
	int					m_bLightClosedEvent;					// LIGHT부클로우즈이벤트수신여부(초기값:FALSE) 
	int					m_bIndClosedEvent;						// INDICATOR부클로우즈이벤트수신여부(초기값:FALSE) 
	int					m_bSensorClosedEvent;					// SENSOR클로우즈이벤트수신여부(초기값:FALSE) 
																
																// 기기 매체수취대기 FLAG : 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	int					m_bMcuWaitTaken;						// 카드부매체수취대기여부(초기값:FALSE) 
	int					m_bSprWaitTaken;						// 명세표부매체수취대기여부(초기값:FALSE) 
	int					m_bCduWaitTaken;						// 현금부(CDU)매체수취대기여부(초기값:FALSE) 

	int					m_nCSTMinimum4Mix6;						// 보정매수값						2004.11.05
	int					m_nCduDispenseAmount;					// CDU 방출금액						2004.04.01
	VARIANT				m_vMixResult;							// CDU Mix결과						2004.04.09
	BOOL				m_bDispenseSuccecded;					// CDU Dispense 결과 성공여부		V01.02.25

	int					m_nPINEntryMode;						// PinPad입력모드(NORMAL/EPP etc)	2004.04.02
	CString				m_strPinEppCardData;					// PINPAD EPP시 사용할 카드데이타	2004.04.02
	CString				m_strPinKeyData;						// PinPad로 부터 올라온 값을 저장(초기값:NULL)
	CString				m_strPinMacingData;						// PinPad Macing Data				2004.04.02
	CString				m_strPinLastKey;						// PinPad 올린 마지막 키값			2005.04.19

	CString				m_strCurrency;							// Registry에 설정된 카세트 Currency를 세팅 저장함. 2005.12.16 PJH
	int					m_GetAvailableAmount;					// Fast Cash Mix를 위해 장애가 발생하지 않도록 조치함. 2006.01.06 PJH
	LPSPInform			m_pSPInform;							// Error Code, M-Data NVRAM structure pointer

	int					m_iBLRecCnt;							// 블랙리스트 레코드 개수
	char				m_caBLRec[10][16];						// 블랙리스트 버퍼

	// RKT 관련 전역 변수 추가 [#554] NH KSK 2009.08.10
	CString				m_strEPP_ExportedKey;
	CString				m_strEPP_Signed;
	CString				m_strEPP_RandomNumber;			// [#2259] Random Number RSA 미지원으로 인해 변수 Name 변경
	CString				m_strEPP_KeyCheckValue;
	// end of [#554]
	CString				m_strEPP_CertificateData;		// [#RWC6-151] US Kook 2020.07.20 TR34

	CString				m_strTemp;

// Several Functions 
public:
	// ------------------------------------------------------------------------
	// 기본정보 관련
	// ------------------------------------------------------------------------
	int			RegSetMwiInformation();							// Registry에 MWI정보 갱신
	void		WriteMwiErrorCode_NVRam(CString strEntry, char* szTemp);		
	// ------------------------------------------------------------------------
	// Matrix 관련
	// ------------------------------------------------------------------------
	BOOL		SetEventMatrix(INT nDeviceID, char* szMethodName);

	// ------------------------------------------------------------------------
	// DEVICE CONTROL : NXCARDREADER
	// ------------------------------------------------------------------------
	int			DevMcuOpenConnection();							// MCU PowerOn Initialize : Session Start
	int			DevMcuCloseConnection();						// MCU Session end
	int			DevMcuEject();									// MCU Card Eject
	int			DevMcuEject(int nWaitTakenSec);					// MCU Card Eject (With Taken Time)
	int			DevMcuAcceptAndReadAvailableTracks(int nWaitTimeSec);					// MCU EntryEnable : 유효트랙읽기
	int			DevMcuICAcceptAndReadAvailableTracks(int nWaitTimeSec);
	int			DevMcuAcceptAndReadAvailableTracks(LPCTSTR szTracks, int nWaitTimeSec);	//	MCU EntryEnable : 유효트랙읽기
	int			DevMcuCancelAccept();							// MCU EntryDiable
	CString		DevMcuGetTrackData(int nTrack);					// MCU GetData
	int			DevMcuGetStatus(int nStatus);					// MCU GetStatus(1:DEVICE, 2:MEDIA)
	int			DevMcuRetract();								// MCU Retract
	int			DevMcuWriteTrack(LPCTSTR szTrackData);			// MCU Track Write
	int			DevMcuUpdateStatus();							// MCU All Property Updates
	int			DevMcuCancelWaitTaken();						// MCU CancelWait For CardTaken
	int			DevMcuEnableStatusEvents();						// MCU StatusChanged Event Enable
	int			DevMcuDisableStatusEvents();					// MCU StatusChanged Event Disable
	int			DevMcuResetBinCount();							// MCU 카드회수정보 Clear
	CString		DevMcuGetErrorCode();							// MCU Get Device ErrorCode
	int			DevMcuClearErrorCode();							// MCU Clear Device ErrorCode
	CString		DevMcuGetSensorInfo();							// MCU Get Sensor Information
	int			DevMcuPreCancelWaitTaken();						// MCU CnacleWaitTaken for Other CMD
	int			DevMcuICChipInitialize();						// MCU IC ChipInitialse
	int			DevMcuICChipPower(long nActType);				// MCU IC ChipPower
	int			DevMcuICSendData(int ProtocolID, LPCTSTR szSendData, int nWaitSec);	// IC Send Data

	// [#2325] NH KSK 2015.01.20 RFID Added
	// ------------------------------------------------------------------------
	// DEVICE CONTROL : NXRFID
	// ------------------------------------------------------------------------
	int			DevRFIDOpenConnection();						// MCU PowerOn Initialize : Session Start
	int			DevRFIDCloseConnection();						// MCU Session end
	int			DevRFIDAcceptAndReadAvailableTracks(int nWaitTimeSec);					// MCU EntryEnable : 유효트랙읽기
	int			DevRFIDAcceptAndReadAvailableTracks(LPCTSTR szTracks, int nWaitTimeSec);	//	MCU EntryEnable : 유효트랙읽기
	int			DevRFIDCancelAccept();							// MCU EntryDiable
	CString		DevRFIDGetTrackData(int nTrack);				// MCU GetData
	int			DevRFIDGetStatus(int nStatus);					// MCU GetStatus(1:DEVICE, 2:MEDIA)
	int			DevRFIDUpdateStatus();							// MCU All Property Updates
	CString		DevRFIDGetErrorCode();							// MCU Get Device ErrorCode
	int			DevRFIDClearErrorCode();						// MCU Clear Device ErrorCode
	// end of [#2325]

	// ------------------------------------------------------------------------
	// DEVICE CONTROL : NXRECEIPTPRINTER
	// ------------------------------------------------------------------------
	int			DevSprOpenConnection();							// SPR Power-On Initialize : Session start
	int			DevSprCloseConnection();						// SPR Session End
	int			DevSprReset();									// SPR Reset
	int			DevSprGetStatus(int nStatus);					// SPR GetStatus(1:Device, 2:Media, 3:Paper)
	int			DevSprPrint(LPCTSTR szData);					// SPR Print
	//int		DevSprImagePrint();								// SPR Image Print (Head Only, Form File Usage)
	int			DevSprImagePrint(int nImageType);				// SPR Image Print (Image Header and Coupon, Form File Usage)		// [#2373] US Justin 2015.09.22 Support Coupon Image on a receipt
	int			DevSprEject(int nWaitSec);						// SPR Eject & Wait Taken
	int			DevSprRetract();								// SPR Retract
	int			DevSprUpdateStatus();							// SPR Status Updates
	int			DevSprCancelWaitTaken();						// SPR CancelWait For CardTaken
	int			DevSprEnableStatusEvents();						// SPR StatusChanged Event Enable
	int			DevSprDisableStatusEvents();					// SPR StatusChanged Event Disable
	int			DevSprResetBinCount();							// SPR 명세표회수정보 Clear
	CString		DevSprGetErrorCode();							// SPR Get Device ErrorCode
	int			DevSprClearErrorCode();							// SPR Clear Device ErrorCode
	CString		DevSprGetSensorInfo();							// SPR Get Sensor Information
	int			DevSprPreCancelWaitTaken();						// SPR CnacleWaitTaken for Other CMD

	// ------------------------------------------------------------------------
	// DEVICE CONTROL : NXJOURNALPRINTER
	// ------------------------------------------------------------------------
	int			DevJprOpenConnection();							// Jpr Power-On Initialize : Session start
	int			DevJprCloseConnection();						// Jpr Session End
	int			DevJprGetStatus(int nStatus);					// Jpr GetStatus(1:Device, 2:Media, 3:Paper)
	int			DevJprEmbossPrint();							// Jpr Emboss Print
	int			DevJprPrint(LPCTSTR szData);					// Jpr Print
	int			DevJprUpdateStatus();							// Jpr Status Updates
	int			DevJprEnableStatusEvents();						// JPR StatusChanged Event Enable
	int			DevJprDisableStatusEvents();					// JPR StatusChanged Event Disable
	CString		DevJprGetErrorCode();							// JPR ErrorCode Search 
	int			DevJprClearErrorCode();							// JPR Clear Device ErrorCode
	CString		DevJprGetSensorInfo();							// JPR Get Sensor Information

	// ------------------------------------------------------------------------
	// DEVICE CONTROL : REARDOOR
	// ------------------------------------------------------------------------
	BOOL		DevDoorGetDoorSwitch();							// 뒷문의 열림상태를 조회한다.
	int			DevDoorOpenConnection();						// 세션을 연결(초기화)
	int			DevDoorCloseConnection();						// 세션을 종료
	int			DevDoorGetStatus();								// 장치상태를 조회한다.
	int			DevDoorEnableStatusEvents();					// 상태변경 이벤트가능
	int			DevDoorDisableStatusEvnets();					// 상태변경 이벤트 불가
	int			DevDoorUpdateStatus();							// Update Status
	int			DevDoorClearErrorCode();						// Clear Device ErrorCode
	CString		DevDoorGetErrorCode();							// 장애코드 조회

	// ------------------------------------------------------------------------
	// DEVICE CONTROL : GUIDELIGHT + INDICATOR
	// ------------------------------------------------------------------------
	int			DevLightOpenConnection();						// 세션을 연결(초기화)
	int			DevLightCloseConnection();						// 세션을 종료
	void		DevLightFlickerOnOff(int nFlickerIndex, int nOnOffFlag);
	void		DevLightSetIndicator(int nValue);				// 거래표시등 표시(Guidelight + Indicator)
	int			DevLightGetStatus();							// 장치상태를 조회한다.
	int			DevLightClearErrorCode();						// Clear Device ErrorCode
	CString		DevLightGetErrorCode();							// 장애코드 조회

	// ------------------------------------------------------------------------
	// DEVICE CONTROL : CDU										// 2004.01.16_1.2_2003
	// ------------------------------------------------------------------------
	int			DevCduOpenConnection();							// CDU 세션연결
	int			DevCduCloseConnection();						// CDU 세션종료
	int			DevCduEnableStatusEvents();						// CDU StatusChanged Event Enable
	int			DevCduDisableStatusEvents();					// CDU StatusChanged Event Disable
	int			DevCduOpenShutter();							// CDU Open Shutter
	int			DevCduCloseShutter();							// CDU Close Shutter
	int			DevCduDispense(VARIANT NoteCnt);				// CDU Dispense
	int			DevCduDispenseCount(int nCST1Cnt, int nCST2Cnt = 0, int nCST3Cnt = 0, int nCST4Cnt = 0);
																// CDU DispenseCount
	int			DevCduDispenseAmount(int nAmount);				// CDU DispenseAmount
	int			DevCduDispenseAndPresentAmount(VARIANT NoteCnt, int nWaitSec);
	int			DevCduDispenseAndPresentCount(int nCST1Cnt, int nCST2Cnt /* = 0 */, int nCST3Cnt /* = 0 */, int nCST4Cnt /* = 0 */, int nWaitSec); //-- SJKWONNOTE 2006-02-07 --//
	int			DevCduPresent(int nWaitSec);					// CDU 출금현금 방출
	int			DevCduRetract();								// CDU Retract
	int			DevCduCancelWaitTaken();						// CDU Cancel Wait for cash taken

	int			DevCduGetDeviceStatus();						// Cdu 상태 조회
	int			DevCduClearErrorCode();							// Clear Device ErrorCode
	CString		DevCduGetErrorCode();							// 장애코드 조회
	int			DevCduAvailWithdraw();							// CDU Checking Withdraw Status
	int			DevCduGetValueOfCash(int nCSTNo);				// CDU Get Value Of Cash
	int			DevCduGetNumberOfCash(int nType, int nCSTNo);	// CDU Get Number Of Cash
	int			DevCduGetPosition();							// CDU Position Information
	int			DevCduGetRecycleBoxStatus();					// CDU Get RBBox Status
	int			DevCduGetRejectCSTStatus();						// CDU Get RejectBox Status
	int			DevCduGetStatus();								// CDU Get Device Status
	CString		DevCduGetSensorInfo();							// CDU Get Sensor Information
	int			DevCduGetMaterialInfo();						// CDU 매체잔류정보
	int			DevCduGetCSTCount();							// CDU CST갯수조회				2004.03.31
	int			DevCduGetLastDispensedAmount();					// CDU Last Dispensed Amount	2006.01.08
	int			DevCduGetLastDispensedCount(int nCstNo);		// CDU Last Dispensed Count		2006.02.26
	BOOL		DevCduGetShutterStatus();						// CDU SHUTTER 유/무 체크		2006.02.08
	int			DevCduInitiateChange();							// CDU CST초기화				2004.04.01
	int			DevCduSetNumberOfCash(LPCTSTR szCashCntData);	// CDU CST별 초기매수 설정		2004.03.31
	int			DevCduAddNumberOfCash(LPCTSTR szInitialCnt, LPCTSTR szAddCnt);	
																// CDU CST별 ADD CASH 매수 설정	2006.01.23
	int			DevCduSetMinMaxThreshold(LPCTSTR szMinCntData, LPCTSTR szMaxCntData);
																// CDU CST별 Threshold설정		2004.04.09
																// CDU 내부처리 호출로직
	int			DevCduSetValueOfCash(LPCTSTR szCashValueData);	// CDU CST별 권종설정			2005.02.24
	int			DevCduPreCancelWaitTaken();						// CDU CnacleWaitTaken for Other CMD
	int			DevCduGetDispenserType();						// CDU Type : 만만수/만만만/만수/만만
	CString		DevCduGetStPosition();							// CDU 장탈착정보조회
	CString		DevCduGetCSTStatus(int nType);					// CDU CST별 상태조회
	CString		DevCduGetStMediaStatus();						// CDU 매체잔류상태 조회
	CString		DevCduGetStRecycleBoxStatus();					// CDU CST활류상태 조회
	CString		DevCduGetStAvailWithdraw();						// CDU 출금가능상태 조회

	int			DevCduUpdateStatus();							// CDU All Property Updates
	int			DevCduUpdateCstStatus();						// CDU CST상태값 갱신처리
	// 2007.11.23 V01.02.19
	CString		DevCduGetCurrencyID();							// CDU Currency Info Get	2007.10.22
	
	// ------------------------------------------------------------------------
	// DEVICE CONTROL : PIN
	// ------------------------------------------------------------------------
	int			DevPinOpenConnection();							// PINPAD 세션연결
	int			DevPinCloseConnection();						// PINPAD 세션종료
	int			DevPinClearErrorCode();							// Clear Device ErrorCode
	CString		DevPinGetErrorCode();							// 장애코드 조회
	int			DevPinGetDeviceStatus();						// 장치상태조회
	int			DevPinEntryEnable(	int		nEnableMode,		// PINPAD EntryEnable
									int		nMinKeyCount,
									int		nMaxKeyCount, 
									BOOL	bAutoEnd, 
									LPCTSTR szActiveKeys, 
									LPCTSTR szTerminatorKeys, 
									LPCTSTR szCardData, 
									int nTimeout			 );
																
	int			DevPinEntryDisable();							// PINPAD CancelUserEntry
	CString		DevPinGetKeyData();								// PINPAD GetKeyData
	int			DevPinMacing(LPCTSTR szData);					// PINPAD Macing
	CString		DevPinGetMacingData();							// PINPAD GetMacingData
	int			DevPinLoadIV(LPCTSTR szKeyName);				// PINPAD LoadIV
	int			DevPinLoadKey(LPCTSTR szKeyName, LPCTSTR szKeyValue);
																// PINPAD LoadKey
	int			DevPinLoadEncryptedKey(LPCTSTR KeyUse, LPCTSTR szKeyName, LPCTSTR szKeyValue, LPCTSTR szEncKeyName);
																// PINPAD LoadKey
	int			DevPinPreEntryDisable();						// PINPAD Pre Entry Disable
	int			DevPinUpdateStatus();							// CDU All Property Updates
	int			DevPinBuildPinBlock(LPCTSTR CardPanData, LPCTSTR XORData , short Padchar, LPCTSTR Format, LPCTSTR Key, LPCTSTR KeyEncKey); // [#68] UK HWANG 2008.04.02 EMV Level2 
	int			DevPINInitializeEDM();							// Clear All Keys // [#645] NH KJW 2010.08.24
	int			DevPINGetExtraCaps();							// [#2080] NH KSK 2011.07.13
	int			DevPIN_GenerateKCV_512K(LPCTSTR KeyName);			// [#2253] NH KSK 2014.02.21

	// ------------------------------------------------------------------------
	// DEVICE CONTROL : PIN (RKT)	[#554] NH KSK 2009.08.10
	// ------------------------------------------------------------------------
	int			DevPINExportRSAIssuerSignedItem(LPCTSTR ExportItemType, LPCTSTR KeyName);
// [#GLDV-2797] AU Kook 2020.05.07 Support Host PK Removal
//	int			DevPINImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature);
	int			DevPINImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature, LPCTSTR Usage);
// end of [#GLDV-2797]
	int			DevPINStartKeyExchange();
	int			DevPINImportRSASignedDESKey(long KeyType, LPCTSTR Recv_EPP_RSACrypt, LPCTSTR Recv_TMK_RSACrypt, LPCTSTR Recv_HSM_Signature);

	// ------------------------------------------------------------------------
	// DEVICE CONTROL : PIN (TR34)				[#RWC6-150] US Kook 2020.07.14
	// ------------------------------------------------------------------------
	int			DevPINGetCertificate(LPCTSTR Certificate);
	int			DevPINInitializeEDMEx(LPCTSTR Indent, LPCTSTR Key);
	int			DevPINImportKeyBlock(LPCTSTR Key, LPCTSTR EncKey, LPCTSTR KeyBlock);
	int			DevPINLoadCertificateEx(LPCTSTR LoadOption, LPCTSTR Signer, LPCTSTR CertificateData);
	int			DevPINImportRSAEncipheredPKCS7KeyEx(LPCTSTR RSAKeyIn, LPCTSTR Key, LPCTSTR Use, LPCTSTR LoadOption);
	int			DevPINStartAuthenticateSync(LPCTSTR MethodName, LPCTSTR Parameters);

	CString		DevPINGetCertificateState();
	CString		DevPINGetAuthenticateDataToSign();
	CString		DevPINGetAuthenticateSigner();

	void		DevPINSetAuthenticateSigner(LPCTSTR Signer);
	void		DevPINSetAuthenticateSigKey(LPCTSTR SigKey);
	void		DevPINSetAuthenticateSignedData(LPCTSTR SignedData);

	// ------------------------------------------------------------------------
	// DEVICE CONTROL : Sensor
	// ------------------------------------------------------------------------
	int			DevSnsOpenConnection();							// 세션연결
	int			DevSnsCloseConnection();						// 세션종료
	int			DevSnsClearErrorCode();							// Clear Device ErrorCode
	CString		DevSnsGetErrorCode();							// 장애코드 조회
	int			DevSnsGetDeviceStatus();						// 장치상태조회
	int			DevSnsGetOperatorSwitch();						// OperatorSwitch값 조회
	int			DevSnsClearOperatorSwitch();					// OperatorSwitch값 Clear
	int			DevSnsGetEnhancedAudio();						// Enhanced Audio 값 조회
	int			DevSnsGetProximityStatus();						// Proximity Status 값 조회		// [iTM] 2010.02.05 AIREAT

	int			DevSnsUpdateStatus();							// All Property Updates


	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	// ------------------------------------------------------------------------
	// DEVICE CONTROL : BCR
	// ------------------------------------------------------------------------
	// methods (Nextware)
	int			DevBcrOpenConnection();
	int			DevBcrCloseConnection();
	int			DevBcrGetDeviceStatus();
	int			DevBcrSendRawData();
	int			DevBcrCancelAccept();
	int			DevBcrReset();

	// methods (AP)
	int			DevBcrClearErrorCode();
	CString		DevBcrGetErrorCode();
	int			DevBcrUpdateStatus();
	CString		DevBcrGetBarcodeData();



	// ------------------------------------------------------------------------
	// DEVICE CONTROL : BNA
	// ------------------------------------------------------------------------
	// methods (Nextware)
	// TODO: remove unnecessary methods
	int			DevBnaOpenConnection();
	int			DevBnaCloseConnection();	
	//int			DevBnaRequestLock();
	//int			DevBnaReleaseLock();
	CString		DevBnaGetExtraStatus(LPCTSTR KeyName);
	//CString		DevBnaGetExtraCaps(CString KeyName);
	int			DevBnaStartCashIn();
	int			DevBnaAcceptCash(long InsertionTime, long TakenTime);
	int			DevBnaCancelAccept();
	int			DevBnaStoreCash();
	int			DevBnaRollbackCash(long TakenTimeOut);
	//int			DevBnaRetract();
	//int			DevBnaOpenShutter();
	//int			DevBnaCloseShutter();
	//int			DevBnaStartExchangeSync();
	//int			DevBnaEndExchangeSync();
	//int			DevBnaOpenSafeDoor();
	int			DevBnaReset(short UnitNumber);
	//int			DevBnaConfigureNoteTypeSync();
	//int			DevBnaCreateP6Signature();
	//int			DevBnaGetP6Signature();
	int			DevBnaConfigureNoteTypeSync();
	int			DevBnaSetCashUnitInfo();
	//int			DevBnaSetCashInLimit();

	// properties (Nextware)
	// TODO: remove unnecessary properties
	// TODO: consider its return type (string as original    vs    int as converted)
	int			DevBnaGetDeviceStatus();
	//CString		DevBnaGetSafeDoorStatus();
	CString		DevBnaGetStackerStatus();
	CString		DevBnaGetAcceptorStatus();
	//CString		DevBnaGetReaderStatus();
	//CString		DevBnaGetShutterStatus();
	//CString		DevBnaGetTransportStatus();
	//int			DevBnaGetNumberOfPositions();
	//CString		DevBnaGetDeviceType();
	int			DevBnaGetMaxCashInItem();
	//BOOL		DevBnaGetHasSafeDoor();
	//BOOL		DevBnaGetHasShutter();
	//BOOL		DevBnaGetCanShutterControl();
	int			DevBnaGetMaxStackerItem();
	//BOOL		DevBnaGetHasTakenSensor();
	//BOOL		DevBnaGetHasInsertedSensor();
	//int			DevBnaGetHasPositions();
	//int			DevBnaGetExchangeType();
	//int			DevBnaGetRetractAreas();
	//int			DevBnaGetNumberOfItemType();
	//BOOL		DevBnaGetCashUnitChangeInprogress();
	CString		DevBnaGetLastCashInStatus();
	int			DevBnaGetNumberOfCashInStatus();
	int			DevBnaGetNumberOfLogicalUnit();
	int			DevBnaGetLastRefusedCount();
	//int			DevBnaGetNumberOfP6Info();
	CString		DevBnaGetPositionStatus();	// [RWC6-676] SKKim 2024.05.17
	int			DevBnaGetRefusedStatus();	// [RWC6-676] SKKim 2024.05.27

	LPDISPATCH	DevBnaGetCashInStatus(long Index);
	LPDISPATCH	DevBnaGetLogicalUnit(long Index);

	// methods (AP)
	int			DevBnaClearErrorCode();
	CString		DevBnaGetErrorCode();
	int			DevBnaUpdateStatus();
	// end of [#GLDV-3005]


	// ------------------------------------------------------------------------
	// TYPE Conversion : BYTE* <-> VARIANT
	// ------------------------------------------------------------------------
	BYTE*       VarArrayToByteArray(LPCTSTR szToken,  const VARIANT *Array, int *Size );
	VARIANT     ByteArrayToSafeArray( BYTE *ByteArray, int ArraySize ); 

	// ------------------------------------------------------------------------
	// TYPE Conversion : CStringArray <-> VARIANT
	// ------------------------------------------------------------------------
	int			VarArrayToStringArray(const VARIANT *Array, CStringArray& SArray);
	VARIANT		StringArrayToSafeArray(CStringArray& SArray);

	int			VarArrayToIntArray(const VARIANT *Array, CArray<int, int>& IArray);
	VARIANT		IntArrayToSafeArray(CArray<int, int>& IArray);

	// ------------------------------------------------------------------------
	// LIB FOR EMV
	// ------------------------------------------------------------------------

	// EMV Kernel 함수
	CString		EMVINF_SendRecvMsgWithIFM(long ProtocolID, LPCTSTR szSendData, long nWaitSec);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNHMWICtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual void OnSetClientSite();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CNHMWICtrl();

	DECLARE_OLECREATE_EX(CNHMWICtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CNHMWICtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CNHMWICtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CNHMWICtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CNHMWICtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//
	// For Mcu(CardReader) Event
	//
	afx_msg void OnFatalErrorNxcardreader(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxcardreader(LPCTSTR Action, long Result);
	afx_msg void OnLockGrantedNxcardreader();
	afx_msg void OnLockReleasedNxcardreader();
	afx_msg void OnLockTimeoutNxcardreader();
	afx_msg void OnDeviceStatusChangedNxcardreader(LPCTSTR Value);
	afx_msg void OnMediaStatusChangedNxcardreader(LPCTSTR Value);
	afx_msg void OnRetainBinStatusChangedNxcardreader(LPCTSTR Value);
	afx_msg void OnSecurityStatusChangedNxcardreader(LPCTSTR Value);
	afx_msg void OnCardsStatusChangedNxcardreader(short Value);
	afx_msg void OnReadCompleteNxcardreader();
	afx_msg void OnWriteCompleteNxcardreader();
	afx_msg void OnEjectCompleteNxcardreader();
	afx_msg void OnRetainCompleteNxcardreader();
	afx_msg void OnChipIOCompleteNxcardreader(LPCTSTR Token, const VARIANT FAR& Data);
	afx_msg void OnChipIOFailureNxcardreader(LPCTSTR Token);
	afx_msg void OnMediaInsertedNxcardreader();
	afx_msg void OnInvalidMediaNxcardreader();
	afx_msg void OnInvalidTrackDataNxcardreader();
	afx_msg void OnMediaRemovedNxcardreader();
	afx_msg void OnRetainBinThresholdNxcardreader();
	afx_msg void OnTimeoutNxcardreader();
	afx_msg void OnChipPowerStatusChangedNxcardreader(LPCTSTR Value);
	afx_msg void OnMediaDetectedNxcardreader(LPCTSTR Value);
	afx_msg void OnResetCompleteNxcardreader();
	afx_msg void OnChipPowerCompleteNxcardreader();
	afx_msg void OnAcceptCancelledNxcardreader();
	//
	// For Pin Event
	//
	afx_msg void OnFatalErrorNxpin(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxpin(LPCTSTR Action, long Result);
	afx_msg void OnLockGrantedNxpin();
	afx_msg void OnLockReleasedNxpin();
	afx_msg void OnLockTimeoutNxpin();
	afx_msg void OnEncryptCompleteNxpin(LPCTSTR CryptData);
	afx_msg void OnDecryptCompleteNxpin(LPCTSTR CryptData);
	afx_msg void OnMACingCompleteNxpin(LPCTSTR CryptData);
	afx_msg void OnKeyImportedNxpin();
	afx_msg void OnReadPinCompleteNxpin();
	afx_msg void OnReadPinCancelledNxpin();
	afx_msg void OnKeyPressedNxpin(LPCTSTR Key, long KeyCode);
	afx_msg void OnPinBlockCompleteNxpin(LPCTSTR PinBlock);
	afx_msg void OnReadDataCompleteNxpin(LPCTSTR Data);
	afx_msg void OnReadDataCancelledNxpin();
	afx_msg void OnEDMInitializedNxpin();
	afx_msg void OnDeviceStatusChangedNxpin(LPCTSTR Value);
	afx_msg void OnTimeoutNxpin();
	afx_msg void OnIllegalKeyAccessedNxpin(LPCTSTR KeyName, long KeyCode);
	afx_msg void OnResetCompleteNxpin();
	afx_msg void OnKeyDeletedNxpin();
	afx_msg void OnRandomNumberGeneratedNxpin(LPCTSTR RandomData);
	afx_msg void OnVerifyLocalDESCompleteNxpin(long Result);
	afx_msg void OnStartKeyExchangeCompleteNxpin(LPCTSTR RandomItem);
	afx_msg void OnRSAPublicKeyImportedNxpin(LPCTSTR KeyCheckMode, LPCTSTR KeyCheckValue);
	afx_msg void OnRSAIssuerSignedItemExportedNxpin(LPCTSTR Value, LPCTSTR RSASignatureAlgorithm, LPCTSTR Signature);
	afx_msg void OnRSASignedDESKeyImportedNxpin(LPCTSTR KeyLength, LPCTSTR KeyCheckMode, LPCTSTR KeyCheckValue);
	afx_msg void OnRSAKeyPairGeneratedNxpin();
	afx_msg void OnRSAEPPSignedItemExportedNxpin(LPCTSTR Value, LPCTSTR SelfSignature, LPCTSTR Signature);
	afx_msg void OnLoadCertificateCompleteNxpin(LPCTSTR CertificateData);
	afx_msg void OnGetCertificateCompleteNxpin(LPCTSTR CertificateData);
	afx_msg void OnReplaceCertificateCompleteNxpin(LPCTSTR CertificateData);
	afx_msg void OnRSAEncipheredKeyImportedNxpin(LPCTSTR KeyLength, LPCTSTR RSAData);
	afx_msg void OnCertificateChangedNxpin(LPCTSTR Change);
	afx_msg void OnKCVGeneratedNxpin(LPCTSTR KeyName, LPCTSTR KCV);
	afx_msg void OnSecureKeyEntryCompleteNxpin(LPCTSTR KCV);
	afx_msg void OnSecureKeyEntryCancelledNxpin();
	afx_msg void OnSecureKeyEntryIncompleteNxpin();

// [#RWC6-151] US Kook 2020.07.14 TR34
	afx_msg void OnKeyBlockImported();
	afx_msg void OnRSAEncipheredKeyExImported(LPCTSTR KeyLength, LPCTSTR CertificateData, LPCTSTR RSAKeyCheckMode, LPCTSTR KeyCheckMode, LPCTSTR KeyCheckValue);
	afx_msg void OnLoadCertificateExComplete(LPCTSTR RSAKeyCheckMode, LPCTSTR RSAData);

	//
	// For Spr(ReceiptPrinter) Event
	//
	afx_msg void OnFatalErrorNxreceiptprinter(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxreceiptprinter(LPCTSTR Action, long Result);
	afx_msg void OnControlMediaCompleteNxreceiptprinter(LPCTSTR Actions);
	afx_msg void OnPrintFormCompleteNxreceiptprinter();
	afx_msg void OnFieldWarningNxreceiptprinter(LPCTSTR FormName, LPCTSTR FieldName, short FailureCode);
	afx_msg void OnFieldErrorNxreceiptprinter(LPCTSTR FormName, LPCTSTR FieldName, short FailureCode);
	afx_msg void OnTimeoutNxreceiptprinter();
	afx_msg void OnSendRawDataCompleteNxreceiptprinter(LPCTSTR ResponseData);
	afx_msg void OnDeviceStatusChangedNxreceiptprinter(LPCTSTR newValue);
	afx_msg void OnMediaStatusChangedNxreceiptprinter(LPCTSTR newValue);
	afx_msg void OnResetCompleteNxreceiptprinter();
	afx_msg void OnPaperStatusChangedNxreceiptprinter(LPCTSTR PaperSource, LPCTSTR newValue);
	afx_msg void OnMediaTakenNxreceiptprinter();
	afx_msg void OnWaitCancelledNxreceiptprinter();
	afx_msg void OnResetCountCompleteNxreceiptprinter();
	afx_msg void OnRetractMediaCompleteNxreceiptprinter(short BinNumber);
	afx_msg void OnRetractBinStatusChangedNxreceiptprinter(short BinNumber, LPCTSTR newValue);
	afx_msg void OnReadFormCompleteNxreceiptprinter(LPCTSTR FieldValues);
	afx_msg void OnMediaExtentsCompleteNxreceiptprinter(long SizeX, long SizeY);
	//
	// For Jpr(JournalPrinter) Event
	//
	afx_msg void OnFatalErrorNxjournalprinter(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxjournalprinter(LPCTSTR Action, long Result);
	afx_msg void OnLockGrantedNxjournalprinter();
	afx_msg void OnLockReleasedNxjournalprinter();
	afx_msg void OnLockTimeoutNxjournalprinter();
	afx_msg void OnControlMediaCompleteNxjournalprinter(LPCTSTR Actions);
	afx_msg void OnPrintFormCompleteNxjournalprinter();
	afx_msg void OnFieldWarningNxjournalprinter(LPCTSTR FormName, LPCTSTR FieldName, short FailureCode);
	afx_msg void OnFieldErrorNxjournalprinter(LPCTSTR FormName, LPCTSTR FieldName, short FailureCode);
	afx_msg void OnTimeoutNxjournalprinter();
	afx_msg void OnSendRawDataCompleteNxjournalprinter(LPCTSTR ResponseData);
	afx_msg void OnDeviceStatusChangedNxjournalprinter(LPCTSTR newValue);
	afx_msg void OnMediaStatusChangedNxjournalprinter(LPCTSTR newValue);
	afx_msg void OnTonerStatusChangedNxjournalprinter(LPCTSTR newValue);
	afx_msg void OnInkStatusChangedNxjournalprinter(LPCTSTR newValue);
	afx_msg void OnLampStatusChangedNxjournalprinter(LPCTSTR newValue);
	afx_msg void OnResetCompleteNxjournalprinter();
	afx_msg void OnMediaDetectedNxjournalprinter(LPCTSTR Position, short BinNumber);
	afx_msg void OnDispensePaperCompleteNxjournalprinter();
	afx_msg void OnPaperStatusChangedNxjournalprinter(LPCTSTR PaperSource, LPCTSTR newValue);
	afx_msg void OnMediaInsertedNxjournalprinter();
	afx_msg void OnAcceptCancelledNxjournalprinter();
	afx_msg void OnMediaTakenNxjournalprinter();
	afx_msg void OnWaitCancelledNxjournalprinter();
	afx_msg void OnResetCountCompleteNxjournalprinter();
	afx_msg void OnRetractMediaCompleteNxjournalprinter(short BinNumber);
	afx_msg void OnRetractBinStatusChangedNxjournalprinter(short BinNumber, LPCTSTR newValue);
	afx_msg void OnReadFormCompleteNxjournalprinter(LPCTSTR FieldValues);
	afx_msg void OnMediaExtentsCompleteNxjournalprinter(long SizeX, long SizeY);
	
	//-------------------------------------------------------------------------
	afx_msg void OnFatalErrorNxdoors(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxdoors(LPCTSTR Action, long Result);
	afx_msg void OnLockGrantedNxdoors();
	afx_msg void OnLockReleasedNxdoors();
	afx_msg void OnLockTimeoutNxdoors();
	afx_msg void OnDeviceStatusChangedNxdoors(LPCTSTR Value);
	afx_msg void OnCabinetChangedNxdoors(LPCTSTR Status);
	afx_msg void OnSafeChangedNxdoors(LPCTSTR Status);
	afx_msg void OnShieldChangedNxdoors(LPCTSTR Status);
	afx_msg void OnTimeoutNxdoors();
	afx_msg void OnResetCompleteNxdoors();

	//-------------------------------------------------------------------------	// 2004.01.16_1.2_2003
	afx_msg void OnFatalErrorNxCashDispenser(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxCashDispenser(LPCTSTR Action, long Result);
	afx_msg void OnLockGrantedNxCashDispenser();
	afx_msg void OnLockReleasedNxCashDispenser();
	afx_msg void OnLockTimeoutNxCashDispenser();
	afx_msg void OnTimeoutNxCashDispenser();
	afx_msg void OnDenominateCompleteNxCashDispenser(const VARIANT FAR& Mix);
	afx_msg void OnNotDispensableNxCashDispenser();
	afx_msg void OnNoteErrorNxCashDispenser(LPCTSTR Reason);
	afx_msg void OnInCompleteDispenseNxCashDispenser();
	afx_msg void OnDispenseCompleteNxCashDispenser();
	afx_msg void OnCountCompleteNxCashDispenser();
	afx_msg void OnItemsPresentedNxCashDispenser();
	afx_msg void OnPresentCompleteNxCashDispenser();
	afx_msg void OnItemsTakenNxCashDispenser();
	afx_msg void OnRetractCompleteNxCashDispenser();
	afx_msg void OnRejectCompleteNxCashDispenser();
	afx_msg void OnOpenShutterCompleteNxCashDispenser();
	afx_msg void OnCloseShutterCompleteNxCashDispenser();
	afx_msg void OnCashUnitErrorNxCashDispenser(short UnitNumber);
	afx_msg void OnOpenSafeDoorCompleteNxCashDispenser();
	afx_msg void OnCalibrateCompleteNxCashDispenser();
	afx_msg void OnCashUnitThresholdNxCashDispenser(short UnitNumber);
	afx_msg void OnCashUnitChangedNxCashDispenser(short UnitNumber);
	afx_msg void OnResetCompleteNxCashDispenser();
	afx_msg void OnMediaDetectedNxCashDispenser(short UnitNumber);
	afx_msg void OnDeviceStatusChangedNxCashDispenser(LPCTSTR Value);
	afx_msg void OnSafeDoorStatusChangedNxCashDispenser(LPCTSTR Value);
	afx_msg void OnDispenserStatusChangedNxCashDispenser(LPCTSTR Value);
	afx_msg void OnStackerStatusChangedNxCashDispenser(LPCTSTR Value);
	afx_msg void OnShutterStatusChangedNxCashDispenser(LPCTSTR Value);
	afx_msg void OnTransportStatusChangedNxCashDispenser(LPCTSTR Value);
	afx_msg void OnTransportStateStatusChangedNxCashDispenser(LPCTSTR Value);
	afx_msg void OnPositionStatusChangedNxCashDispenser(LPCTSTR Value);
	afx_msg void OnCountChangedNxCashDispenser();
	//------------------------------------------------------------------------- 2005.03.30
	afx_msg void OnFatalErrorNxsensors(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxsensors(LPCTSTR Action, long Result);
	afx_msg void OnDeviceStatusChangedNxsensors(LPCTSTR Value);
	afx_msg void OnOperatorSwitchChangedNxsensors(LPCTSTR Status);
	afx_msg void OnTamperChangedNxsensors(LPCTSTR Status);
	afx_msg void OnInternalTamperChangedNxsensors(LPCTSTR Status);
	afx_msg void OnSeismicChangedNxsensors(LPCTSTR Status);
	afx_msg void OnHeatChangedNxsensors(LPCTSTR Status);
	afx_msg void OnResetCompleteNxsensors();
	afx_msg void OnEnhancedAudioChangedNxsensors(LPCTSTR Status);
	afx_msg void OnProximityChangedNxsensors(LPCTSTR Status);
	afx_msg void OnAmblightChangedNxsensors(LPCTSTR Status);
	//------------------------------------------------------------------------- 2005.03.30
	afx_msg void OnFatalErrorNxguidlights(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxguidlights(LPCTSTR Action, long Result);
	afx_msg void OnLockGrantedNxguidlights();
	afx_msg void OnLockReleasedNxguidlights();
	afx_msg void OnLockTimeoutNxguidlights();
	afx_msg void OnDeviceStatusChangedNxguidlights(LPCTSTR Value);
	afx_msg void OnGuidLightChangedNxguidlights(LPCTSTR GuidLightUnit, LPCTSTR State);
	afx_msg void OnResetCompleteNxguidlights();
	
	// [#2325] NH KSK 2015.01.21
	afx_msg void OnFatalErrorNxrfidreader(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxrfidreader(LPCTSTR Action, long Result);
	afx_msg void OnLockGrantedNxrfidreader();
	afx_msg void OnLockReleasedNxrfidreader();
	afx_msg void OnLockTimeoutNxrfidreader();
	afx_msg void OnDeviceStatusChangedNxrfidreader(LPCTSTR Value);
	afx_msg void OnMediaStatusChangedNxrfidreader(LPCTSTR Value);
	afx_msg void OnRetainBinStatusChangedNxrfidreader(LPCTSTR Value);
	afx_msg void OnSecurityStatusChangedNxrfidreader(LPCTSTR Value);
	afx_msg void OnCardsStatusChangedNxrfidreader(short Value);
	afx_msg void OnReadCompleteNxrfidreader();
	afx_msg void OnWriteCompleteNxrfidreader();
	afx_msg void OnEjectCompleteNxrfidreader();
	afx_msg void OnRetainCompleteNxrfidreader();
	afx_msg void OnChipIOCompleteNxrfidreader(LPCTSTR Token, const VARIANT FAR& Data);
	afx_msg void OnChipIOFailureNxrfidreader(LPCTSTR Token);
	afx_msg void OnMediaInsertedNxrfidreader();
	afx_msg void OnInvalidMediaNxrfidreader();
	afx_msg void OnInvalidTrackDataNxrfidreader();
	afx_msg void OnMediaRemovedNxrfidreader();
	afx_msg void OnRetainBinThresholdNxrfidreader();
	afx_msg void OnTimeoutNxrfidreader();
	afx_msg void OnChipPowerStatusChangedNxrfidreader(LPCTSTR Value);
	afx_msg void OnMediaDetectedNxrfidreader(LPCTSTR Value);
	afx_msg void OnResetCompleteNxrfidreader();
	afx_msg void OnChipPowerCompleteNxrfidreader();
	afx_msg void OnAcceptCancelledNxrfidreader();
	// end of [#2325]

	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	// Barcode Reader
	afx_msg void OnFatalErrorNxBarcodeReader(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxBarcodeReader(LPCTSTR Action, long Result);
	afx_msg void OnTimeoutNxBarcodeReader();
	afx_msg void OnResetCompleteNxBarcodeReader();
	afx_msg void OnDeviceStatusChangedNxBarcodeReader(LPCTSTR newValue);
	afx_msg void OnSendRawDataCompleteNxBarcodeReader(LPCTSTR ResponseData);
	afx_msg void OnAcceptCancelledNxBarcodeReader();


	//// BillAcceptor
	afx_msg void OnFatalErrorNxCashAcceptor(LPCTSTR Action, long Result);
	afx_msg void OnDeviceErrorNxCashAcceptor(LPCTSTR Action, long Result);
	afx_msg void OnLockGrantedNxCashAcceptor();
	afx_msg void OnLockReleasedNxCashAcceptor();
	afx_msg void OnLockTimeoutNxCashAcceptor();
	afx_msg void OnTimeoutNxCashAcceptor();
	afx_msg void OnStartCashInCompleteNxCashAcceptor();
	afx_msg void OnAcceptCashCompleteNxCashAcceptor();
	afx_msg void OnStoreCashCompleteNxCashAcceptor(long CashInItems);
	afx_msg void OnRollbackCashCompleteNxCashAcceptor();

	afx_msg void OnOpenShutterCompleteNxCashAcceptor(LPCTSTR Position);
	afx_msg void OnCloseShutterCompleteNxCashAcceptor(LPCTSTR Position);
	afx_msg void OnRetractCompleteNxCashAcceptor();
	afx_msg void OnOpenSafeDoorCompleteNxCashAcceptor();
	afx_msg void OnResetCompleteNxCashAcceptor();
	afx_msg void OnCashUnitThresholdNxCashAcceptor(short UnitNumber);
	afx_msg void OnCashUnitChangedNxCashAcceptor(short UnitNumber);
	afx_msg void OnCashUnitErrorNxCashAcceptor(short UnitNumber);
	afx_msg void OnItemsTakenNxCashAcceptor();
	afx_msg void OnItemsRefusedNxCashAcceptor();

	afx_msg void OnItemsInsertedNxCashAcceptor();
	afx_msg void OnMediaDetectedNxCashAcceptor(short UnitNumber);
	afx_msg void OnStatusChangedNxCashAcceptor(LPCTSTR NewValue);
	afx_msg void OnSafeDoorStatusChangedNxCashAcceptor(LPCTSTR NewValue);
	afx_msg void OnAcceptorStatusChangedNxCashAcceptor(LPCTSTR NewValue);
	afx_msg void OnStackerStatusChangedNxCashAcceptor(LPCTSTR NewValue);
	afx_msg void OnReaderStatusChangedNxCashAcceptor(LPCTSTR NewValue);
	afx_msg void OnShutterStatusChangedNxCashAcceptor(LPCTSTR Position, LPCTSTR NewValue);
	afx_msg void OnPositionStatusChangedNxCashAcceptor(LPCTSTR Position, LPCTSTR NewValue);
	afx_msg void OnTransportStatusChangedNxCashAcceptor(LPCTSTR Position, LPCTSTR NewValue);

	afx_msg void OnAcceptCancelledNxCashAcceptor();
	afx_msg void OnCreateP6SignatureCompleteNxCashAcceptor(short NoteID, LPCTSTR Orientation, LPCTSTR Signature);
	afx_msg void OnGetP6SignatureCompleteNxCashAcceptor(short NoteID, LPCTSTR Orientation, LPCTSTR Signature);
	afx_msg void OnP6InputedNxCashAcceptor();
	afx_msg void OnItemsPresentedNxCashAcceptor();
	afx_msg void OnNoteErrorNxCashAcceptor(LPCTSTR Reason);
	afx_msg void OnSetCashUnitInfoCompleteNxCashAcceptor();
	afx_msg void OnSetCashInLimitCompleteNxCashAcceptor();
	// end of [#GLDV-3005]

	//------------------------------------------------------------------------- 2005.03.30
	afx_msg void OnDevIndConnectionOpened();
	afx_msg void OnDevIndConnectionClosed();


	DECLARE_EVENTSINK_MAP()										// AXINAX
	//-------------------------------------------------------------------------
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CNHMWICtrl)
	afx_msg BSTR TestMethod(LPCTSTR szParam1, LPCTSTR szParam2, LPCTSTR szParam3);
	afx_msg void ProcSetDeviceEvent(LPCTSTR szDeviceName, LPCTSTR szEventName, LPCTSTR szEventValue);
	afx_msg void ProcSetDeviceStatus(LPCTSTR szDeviceName, LPCTSTR szProperty, LPCTSTR szStatus);
	afx_msg long McuEntryDisable();
	afx_msg BSTR McuGetCardData();
	afx_msg long McuInitialize();
	afx_msg long McuRead();
	afx_msg long McuRetract();
	afx_msg long McuWrite(LPCTSTR szTrackData);
	afx_msg long SprGetPaperStatus();
	afx_msg long JprEmbossPrint();
	afx_msg long JprGetPaperStatus();
	afx_msg long JprInitialize();
	afx_msg long McuDeinitialize();
	afx_msg BSTR CheckDeviceAction(long nDevID, long nWaitSec);
	afx_msg long McuWaitTaken();
	afx_msg long SprDeinitialize();
	afx_msg long SprWaitTaken();
	afx_msg long ScanDeviceAction(long nDevID, long nWaitSec, long nEventKind);
	afx_msg long McuEntryEnable(long nWaitSec);
	afx_msg long GetErrorDevice(long nDevID);
	afx_msg long McuGetMaterialInfo();
	afx_msg long SprGetMaterialInfo();
	afx_msg long JprGetMaterialInfo();
	afx_msg long SprInitialize();
	afx_msg long JprDeinitialize();
	afx_msg long McuEject(long nWaitSec);
	afx_msg long SprPrint(LPCTSTR szPrintData);
	afx_msg long JprPrint(LPCTSTR szPrintData);
	afx_msg long JprGetDeviceStatus();
	afx_msg long SprGetDeviceStatus();
	afx_msg long SprEject(long nWaitSec);
	afx_msg long SprRetract();
	afx_msg long McuGetDeviceStatus();
	afx_msg long DorInitialize();
	afx_msg long DorDeinitialize();
	afx_msg long DorGetDeviceStatus();
	afx_msg long DorGetDoorStatus();
	afx_msg long LgtInitialize();
	afx_msg long LgtDeinitialize();
	afx_msg long LgtSetFlicker(long nIndex, long nValue);
	afx_msg long LgtSetIndicator(long nValue);
	afx_msg long McuCancelWaitTaken();
	afx_msg BSTR McuGetErrorCode();
	afx_msg BSTR SprGetErrorCode();
	afx_msg BSTR JprGetErrorCode();
	afx_msg long SprClearErrorCode();
	afx_msg long JprClearErrorCode();
	afx_msg long McuClearErrorCode();
	afx_msg long DorClearErrorCode();
	afx_msg BSTR JprGetSensorInfo();
	afx_msg BSTR SprGetSensorInfo();
	afx_msg BSTR McuGetSensorInfo();
	afx_msg long SprClearRetractCnt();
	afx_msg long McuClearRetractCnt();
	afx_msg BSTR DorGetErrorCode();
	afx_msg BSTR SprGetRetractCnt();
	afx_msg BSTR McuGetRetractCnt();
	afx_msg long ScrInitialize();
	afx_msg BSTR ScrGetKeyString(long nWaitSec);
	afx_msg long ScrSetData(LPCTSTR szDataName, LPCTSTR szDataValue);
	afx_msg long ScrDisplayScreen(LPCTSTR szTag, long nIndex, LPCTSTR szData);
	afx_msg long SprCancelWaitTaken();
	afx_msg long LgtClearErrorCode();
	afx_msg long LgtGetDeviceStatus();
	afx_msg BSTR LgtGetErrorCode();
	afx_msg BSTR ScrGetScreenData(long nWaitSec);
	afx_msg long ScrSetScreenData(LPCTSTR szDataName, LPCTSTR szDataValue);
	afx_msg long ScrSetDisplayData(LPCTSTR szDataName, LPCTSTR szDataValue);
	afx_msg BSTR ScrGetKeyData(long nWaitSec);
	afx_msg long ScrClearKeyData();
	afx_msg long GetTimeoutDevice(long nDevID);
	afx_msg BSTR McuGetICData();
	afx_msg long McuICEntryEnable(long nWaitSec);
	afx_msg long McuICEntryDisable();
	afx_msg long McuICSendData(long ProtocolID, LPCTSTR szSendData, long nWaitSec);
	afx_msg long McuICChipInitialize();
	afx_msg long CduInitialize(long InitialType);
	afx_msg long CduDeinitialize();
	afx_msg BSTR CduGetErrorCode();
	afx_msg long CduClearErrorCode();
	afx_msg BSTR CduGetSensorInfo();
	afx_msg long CduGetDeviceStatus();
	afx_msg long CduGetPosition();
	afx_msg long CduGetRejectCSTStatus();
	afx_msg long CduGetRecycleBoxStatus();
	afx_msg long CduGetMaterialInfo();
	afx_msg long CduGetAvailWithdraw();
	afx_msg long CduOpenShutter();
	afx_msg long CduCloseShutter();
	afx_msg long CduWaitTaken();
	afx_msg long CduCancelWaitTaken();
	afx_msg long CduRetract();
	afx_msg long CduPresent(long nWaitSec);
	afx_msg long PinInitialize();
	afx_msg long PinDeinitialize();
	afx_msg BSTR PinGetErrorCode();
	afx_msg long PinClearErrorCode();
	afx_msg long PinEntryDisable();
	afx_msg BSTR PinGetPinKeyData();
	afx_msg long CduGetCSTStatus(long nType);
	afx_msg long CduGetNumberOfCST();
	afx_msg long CduGetValueOfCash(long nCSTNo);
	afx_msg long CduGetNumberOfCash(long nCSTNo);
	afx_msg long CduGetNumberOfSetCash(long nCSTNo);
	afx_msg long CduDispenseCount(long nCST1Cnt, long nCST2Cnt, long nCST3Cnt, long nCST4Cnt);
	afx_msg long CduDispenseAmount(long nAmount);
	afx_msg long CduDispenseAndPresentAmount(long nAmount, long nWaitSec);
	afx_msg long PinEntryEnable(long nEnableMode, long nMinKeyCount, long nMaxKeyCount, BOOL bAutoEnd, LPCTSTR szActiveKeys, LPCTSTR szTerminatorKeys, LPCTSTR szCardData, long nTimeout);
	afx_msg long PinMacingData(LPCTSTR szMacData);
	afx_msg BSTR PinGetMacingData();
	afx_msg long PinLoadIV(LPCTSTR szKeyName);
	afx_msg long PinLoadKey(LPCTSTR szKeyName, LPCTSTR szKeyValue);
	afx_msg long PinGetDeviceStatus();
	afx_msg BSTR CduGetDispenseOfCST(long nAmount);
	afx_msg long SnsDeinitialize();
	afx_msg long SnsClearErrorCode();
	afx_msg BSTR SnsGetErrorCode();
	afx_msg long SnsGetDeviceStatus();
	afx_msg long SnsGetOperatorSwitch();
	afx_msg long SnsClearOperatorSwitch();
	afx_msg long ClearEventDevice(long nDevID);
	afx_msg long ClearStatusDevice(long nDevID);
	afx_msg long GetEventDevice(long nDevID);
	afx_msg long GetStatusDevice(long nDevID);
	afx_msg long CduSetMinMaxCST(LPCTSTR szMinCntData, LPCTSTR szMaxCntData);
	afx_msg long CduSetNumberOfCash(LPCTSTR szCashCntData);
	afx_msg long CduSetValueOfCash(LPCTSTR szCashValueData);
	afx_msg long SnsInitialize();
	afx_msg long CduGetLastDispensedAmount();
	afx_msg long CduAddNumberOfCash(LPCTSTR szInitialCnt, LPCTSTR szAddCnt);
	afx_msg long ProcBackupTrace(LPCTSTR szFileName, long nFileSize);
	afx_msg long CduDispenseAndPresentCount(long nCST1Cnt, long nCST2Cnt, long nCST3Cnt, long nCST4Cnt, long nWaitSec);
	afx_msg long PinLoadEncryptedKey(LPCTSTR KeyUse, LPCTSTR strKeyName, LPCTSTR strKeyValue, LPCTSTR strEncKeyName);
	afx_msg BOOL CduGetShutterStatus();
	afx_msg long SnsGetEnhancedAudio();
	afx_msg long CduGetLastDispensedCount(long nCstNum);
	afx_msg long McuICChipPower(long nActType);
	afx_msg BSTR CduGetCurrencyID(long nCSTNo);
	afx_msg long PinBuildPinBlock(LPCTSTR CardPanData, LPCTSTR XORData, short Padchar, LPCTSTR Format, LPCTSTR Key, LPCTSTR KeyEncKey);
	afx_msg long CduSetCurrencyID(LPCTSTR CurrencyID);
	afx_msg long RKT_ExportRSAIssuerSignedItem(LPCTSTR ExportItemType, LPCTSTR KeyName);
// [#GLDV-2797] AU Kook 2020.04.28 Support Host PK Removal
//	afx_msg long RKT_ImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature);
	afx_msg long RKT_ImportRSAPublicKey(LPCTSTR Recv_HSM_PublicKey, LPCTSTR Recv_HSM_Signature, LPCTSTR Usage);
// end of [#GLDV-2797]
	afx_msg long RKT_StartKeyExchange();
	afx_msg long RKT_ImportRSASignedDESKey(long KeyType, LPCTSTR Recv_EPP_RSACrypt, LPCTSTR Recv_TMK_RSACrypt, LPCTSTR Recv_HSM_Signature);
	afx_msg BSTR RKT_GetExportedKey();
	afx_msg BSTR RKT_GetExportedSignedValue();
	afx_msg BSTR RKT_GetEPP_RandomNumber();
	afx_msg BSTR RKT_GetEPP_KCV();
	afx_msg	BSTR EMV_SendRecvMsgWithIFM(long ProtocolID, LPCTSTR szSendData, long nWaitSec);
	afx_msg long SnsGetProximity();
	afx_msg	long PinInitializeEDM();
	afx_msg long RKT_GetExtraCaps();
	afx_msg BSTR PIN_GenerateKCV_512K(LPCTSTR KeyName);		// [#2253] NH KSK 2014.02.21

	// [#2325] NH KSK 2015.01.21
	afx_msg long RFIDInitialize();
	afx_msg long RFIDDeinitialize();
	afx_msg BSTR RFIDGetErrorCode();
	afx_msg long RFIDClearErrorCode();
	afx_msg long RFIDGetDeviceStatus();
	afx_msg BSTR RFIDGetCardData();
	afx_msg long RFIDEntryEnable(long nWaitSec);
	afx_msg long RFIDEntryDisable();
	// end of [#2325]

	// [#RWC6-151] US Kook 2020.07.20 TR34
	afx_msg	long PinGetCertificate(LPCTSTR Certificate);
	afx_msg BSTR PinGetCertificateData();
	afx_msg BSTR PinGetCertificateState();

	afx_msg	long PinInitializeEDMEx(LPCTSTR Indent, LPCTSTR Key);
	afx_msg	long PinImportKeyBlock(LPCTSTR Key, LPCTSTR EncKey, LPCTSTR KeyBlock);
	afx_msg	long PinLoadCertificateEx(LPCTSTR LoadOption, LPCTSTR Signer, LPCTSTR CertificateData);
	afx_msg	long PinImportRSAEncipheredPKCS7KeyEx(LPCTSTR RSAKeyIn, LPCTSTR Key, LPCTSTR Use, LPCTSTR LoadOption);
	afx_msg	long PinStartAuthenticateSync(LPCTSTR MethodName, LPCTSTR Parameters);

	afx_msg BSTR PinGetAuthenticateDataToSign();
	afx_msg BSTR PinGetAuthenticateSigner();
	afx_msg void PinSetAuthenticateSigner(LPCTSTR Signer);
	afx_msg void PinSetAuthenticateSigKey(LPCTSTR SigKey);
	afx_msg void PinSetAuthenticateSignedData(LPCTSTR SignedData);

	// [#GLDV-3005] US Kook 2021.10.25 Support Side Car
	afx_msg long BcrInitialize();
	afx_msg long BcrDeinitialize();
	afx_msg BSTR BcrGetErrorCode();
	afx_msg long BcrClearErrorCode();
	afx_msg long BcrGetDeviceStatus();
	afx_msg long BcrSendRawData();
	afx_msg long BcrCancelAccept();
	afx_msg long BcrReset();
	afx_msg BSTR BcrGetBarcodeData();

	afx_msg long BnaInitialize();
	afx_msg long BnaDeinitialize();
	afx_msg BSTR BnaGetErrorCode();
	afx_msg long BnaClearErrorCode();
	afx_msg long BnaGetDeviceStatus();
	afx_msg BSTR BnaGetExtraStatus(LPCTSTR KeyName);
	afx_msg long BnaStartCashIn();
	afx_msg long BnaAcceptCash(long InsertionTimeOut, long TakenTimeOut);
	afx_msg long BnaCancelAccept();
	afx_msg long BnaStoreCash();
	afx_msg long BnaRollbackCash(long TakenTimeOut);
	afx_msg long BnaReset(short UnitNumber);
	afx_msg long BnaConfigureNoteType();
	afx_msg long BnaSetCashUnitInfo();

	afx_msg BSTR BnaGetLastCashInStatus();
	afx_msg long BnaGetNumberOfCashInStatus();
	afx_msg long BnaGetNumberOfLogicalUnit();

	afx_msg LPDISPATCH BnaGetCashInStatus(long Index);
	afx_msg LPDISPATCH BnaGetLogicalUnit(long Index);
	afx_msg BSTR BnaGetAcceptorStatus();
	afx_msg BSTR BnaGetStackerStatus();
	afx_msg long BnaGetMaxCashInItem();
	afx_msg long BnaGetMaxStackerItem();
	afx_msg long BnaGetLastRefusedCount();
	afx_msg BSTR BnaGetPositionStatus();
	afx_msg long BnaGetRefusedStatus();

	//afx_msg long BnaOpenShutter();
	//afx_msg long BnaCloseShutter();
	//afx_msg long BnaRetract();
	//afx_msg long BnaWaitTaken();
	// end of [#GLDV-3005]

	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CNHMWICtrl)
	void FireTestEvent(LPCTSTR szData)
		{FireEvent(eventidTestEvent,EVENT_PARAM(VTS_BSTR), szData);}
	void FireOnDisplayScreen(LPCTSTR szTag, long nIndex, LPCTSTR szData)
		{FireEvent(eventidOnDisplayScreen,EVENT_PARAM(VTS_BSTR  VTS_I4  VTS_BSTR), szTag, nIndex, szData);}
	void FireOnDisplayData(LPCTSTR szDataName, LPCTSTR szDataValue)
		{FireEvent(eventidOnDisplayData,EVENT_PARAM(VTS_BSTR  VTS_BSTR), szDataName, szDataValue);}
	void FireOnDisplayPinpadData(LPCTSTR szPinpadData)
		{FireEvent(eventidOnDisplayPinpadData,EVENT_PARAM(VTS_BSTR), szPinpadData);}
	void FireOnSafeDoorChanged(LPCTSTR szStatus)
		{FireEvent(eventidOnSafeDoorChanged,EVENT_PARAM(VTS_BSTR), szStatus);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CNHMWICtrl)
	dispidTestMethod = 1L,
	dispidProcSetDeviceEvent = 2L,
	dispidProcSetDeviceStatus = 3L,
	dispidMcuEntryDisable = 4L,
	dispidMcuGetCardData = 5L,
	dispidMcuInitialize = 6L,
	dispidMcuRead = 7L,
	dispidMcuRetract = 8L,
	dispidMcuWrite = 9L,
	dispidSprGetPaperStatus = 10L,
	dispidJprEmbossPrint = 11L,
	dispidJprGetPaperStatus = 12L,
	dispidJprInitialize = 13L,
	dispidMcuDeinitialize = 14L,
	dispidCheckDeviceAction = 15L,
	dispidMcuWaitTaken = 16L,
	dispidSprDeinitialize = 17L,
	dispidSprWaitTaken = 18L,
	dispidScanDeviceAction = 19L,
	dispidMcuEntryEnable = 20L,
	dispidGetErrorDevice = 21L,
	dispidMcuGetMaterialInfo = 22L,
	dispidSprGetMaterialInfo = 23L,
	dispidJprGetMaterialInfo = 24L,
	dispidSprInitialize = 25L,
	dispidJprDeinitialize = 26L,
	dispidMcuEject = 27L,
	dispidSprPrint = 28L,
	dispidJprPrint = 29L,
	dispidJprGetDeviceStatus = 30L,
	dispidSprGetDeviceStatus = 31L,
	dispidSprEject = 32L,
	dispidSprRetract = 33L,
	dispidMcuGetDeviceStatus = 34L,
	dispidDorInitialize = 35L,
	dispidDorDeinitialize = 36L,
	dispidDorGetDeviceStatus = 37L,
	dispidDorGetDoorStatus = 38L,
	dispidLgtInitialize = 39L,
	dispidLgtDeinitialize = 40L,
	dispidLgtSetFlicker = 41L,
	dispidLgtSetIndicator = 42L,
	dispidMcuCancelWaitTaken = 43L,
	dispidMcuGetErrorCode = 44L,
	dispidSprGetErrorCode = 45L,
	dispidJprGetErrorCode = 46L,
	dispidSprClearErrorCode = 47L,
	dispidJprClearErrorCode = 48L,
	dispidMcuClearErrorCode = 49L,
	dispidDorClearErrorCode = 50L,
	dispidJprGetSensorInfo = 51L,
	dispidSprGetSensorInfo = 52L,
	dispidMcuGetSensorInfo = 53L,
	dispidSprClearRetractCnt = 54L,
	dispidMcuClearRetractCnt = 55L,
	dispidDorGetErrorCode = 56L,
	dispidSprGetRetractCnt = 57L,
	dispidMcuGetRetractCnt = 58L,
	dispidScrInitialize = 59L,
	dispidScrGetKeyString = 60L,
	dispidScrSetData = 61L,
	dispidScrDisplayScreen = 62L,
	dispidSprCancelWaitTaken = 63L,
	dispidLgtClearErrorCode = 64L,
	dispidLgtGetDeviceStatus = 65L,
	dispidLgtGetErrorCode = 66L,
	dispidScrGetScreenData = 67L,
	dispidScrSetScreenData = 68L,
	dispidScrSetDisplayData = 69L,
	dispidScrGetKeyData = 70L,
	dispidScrClearKeyData = 71L,
	dispidGetTimeoutDevice = 72L,
	dispidMcuGetICData = 73L,
	dispidMcuICEntryEnable = 74L,
	dispidMcuICEntryDisable = 75L,
	dispidMcuICSendData = 76L,
	dispidMcuICChipInitialize = 77L,
	dispidCduInitialize = 78L,
	dispidCduDeinitialize = 79L,
	dispidCduGetErrorCode = 80L,
	dispidCduClearErrorCode = 81L,
	dispidCduGetSensorInfo = 82L,
	dispidCduGetDeviceStatus = 83L,
	dispidCduGetPosition = 84L,
	dispidCduGetRejectCSTStatus = 85L,
	dispidCduGetRecycleBoxStatus = 86L,
	dispidCduGetMaterialInfo = 87L,
	dispidCduGetAvailWithdraw = 88L,
	dispidCduOpenShutter = 89L,
	dispidCduCloseShutter = 90L,
	dispidCduWaitTaken = 91L,
	dispidCduCancelWaitTaken = 92L,
	dispidCduRetract = 93L,
	dispidCduPresent = 94L,
	dispidPinInitialize = 95L,
	dispidPinDeinitialize = 96L,
	dispidPinGetErrorCode = 97L,
	dispidPinClearErrorCode = 98L,
	dispidPinEntryDisable = 99L,
	dispidPinGetPinKeyData = 100L,
	dispidCduGetCSTStatus = 101L,
	dispidCduGetNumberOfCST = 102L,
	dispidCduGetValueOfCash = 103L,
	dispidCduGetNumberOfCash = 104L,
	dispidCduGetNumberOfSetCash = 105L,
	dispidCduDispenseCount = 106L,
	dispidCduDispenseAmount = 107L,
	dispidCduDispenseAndPresentAmount = 108L,
	dispidPinEntryEnable = 109L,
	dispidPinMacingData = 110L,
	dispidPinGetMacingData = 111L,
	dispidPinLoadIV = 112L,
	dispidPinLoadKey = 113L,
	dispidPinGetDeviceStatus = 114L,
	dispidCduGetDispenseOfCST = 115L,
	dispidSnsDeinitialize = 116L,
	dispidSnsClearErrorCode = 117L,
	dispidSnsGetErrorCode = 118L,
	dispidSnsGetDeviceStatus = 119L,
	dispidSnsGetOperatorSwitch = 120L,
	dispidSnsClearOperatorSwitch = 121L,
	dispidClearEventDevice = 122L,
	dispidClearStatusDevice = 123L,
	dispidGetEventDevice = 124L,
	dispidGetStatusDevice = 125L,
	dispidCduSetMinMaxCST = 126L,
	dispidCduSetNumberOfCash = 127L,
	dispidCduSetValueOfCash = 128L,
	dispidSnsInitialize = 129L,
	dispidCduGetLastDispensedAmount = 130L,
	dispidCduAddNumberOfCash = 131L,
	dispidProcBackupTrace = 132L,
	dispidCduDispenseAndPresentCount = 133L,
	dispidPinLoadEncryptedKey = 134L,
	dispidCduGetShutterStatus = 135L,
	dispidSnsGetEnhancedAudio = 136L,
	dispidCduGetLastDispensedCount = 137L,
	dispidMcuICChipPower = 138L,
	dispidCduGetCurrencyID = 139L,
	dispidPinBuildPinBlock = 140L,
	dispidCduSetCurrencyID = 141L,
	dispidRKT_ExportRSAIssuerSignedItem = 142L,
	dispidRKT_ImportRSAPublicKey = 143L,
	dispidRKT_StartKeyExchange = 144L,
	dispidRKT_ImportRSASignedDESKey = 145L,
	dispidRKT_GetExportedKey = 146L,
	dispidRKT_GetExportedSignedValue = 147L,
	dispidRKT_GetEPP_RandomNumber = 148L,
	dispidRKT_GetEPP_KCV = 149L,
	dispidEMV_SendRecvMsgWithIFM = 150L,
	dispidSnsGetProximity = 151L,
	dispidPinInitializeEDM = 152L,
	dispidRKT_GetExtraCaps = 153L,
	dispidPIN_GenerateKCV_512K = 154L,
	dispidRFIDInitialize = 155L,
	dispidRFIDDeinitialize = 156L,
	dispidRFIDGetErrorCode = 157L,
	dispidRFIDClearErrorCode = 158L,
	dispidRFIDGetDeviceStatus = 159L,
	dispidRFIDGetCardData = 160L,
	dispidRFIDEntryEnable = 161L,
	dispidRFIDEntryDisable = 162L,

	// [#RWC6-151] US Kook 2020.07.20 TR34
	dispidPinGetCertificate = 163L,
	dispidPinInitializeEDMEx = 164L,
	dispidPinImportKeyBlock = 165L,
	dispidPinLoadCertificateEx = 166L,
	dispidPinImportRSAEncipheredPKCS7KeyEx = 167L,
	dispidPinGetCertificateData = 168L,
	dispidPinGetCertificateState = 169L,
	dispidPinStartAuthenticateSync = 170L,

	dispidPinGetAuthenticateDataToSign = 171L,
	dispidPinGetAuthenticateSigner = 172L,
	dispidPinSetAuthenticateSigner = 173L,
	dispidPinSetAuthenticateSigKey = 174L,
	dispidPinSetAuthenticateSignedData = 175L,

	// [#GLDV-3005] US Kook 2021.10.25 Support Side Car
	dispidBcrInitialize = 176L,
	dispidBcrDeinitialize = 177L,
	dispidBcrGetErrorCode = 178L,
	dispidBcrClearErrorCode = 179L,
	dispidBcrGetDeviceStatus = 180L,
	dispidBcrSendRawData = 181L,
	dispidBcrCancelAccept = 182L,
	dispidBcrReset = 183L,
	dispidBcrGetBarcodeData = 184L,

	// TODO: implement BNA methods
	dispidBnaInitialize = 185L,
	dispidBnaDeinitialize = 186L,
	dispidBnaGetErrorCode = 187L,
	dispidBnaClearErrorCode = 188L,
	dispidBnaGetDeviceStatus = 189L,
	dispidBnaGetExtraStatus = 190L,
	dispidBnaStartCashIn = 191L,
	dispidBnaAcceptCash = 192L,
	dispidBnaCancelAccept = 193L,
	dispidBnaStoreCash = 194L,
	dispidBnaRollbackCash = 195L,
	dispidBnaReset = 196L,
	dispidBnaConfigureNoteType = 197L,
	dispidBnaSetCashUnitInfo = 198L,
	
	dispidBnaGetLastCashInStatus = 199L,
	dispidBnaGetNumberOfCashInStatus = 200L,
	dispidBnaGetNumberOfLogicalUnit = 201L,
	dispidBnaGetCashInStatus = 202L,
	dispidBnaGetLogicalUnit = 203L,
	dispidBnaGetAcceptorStatus = 204L,
	dispidBnaGetStackerStatus = 205L,
	dispidBnaGetMaxCashInItem = 206L,
	dispidBnaGetMaxStackerItem = 207L,
	dispidBnaGetLastRefusedCount = 208L,
	// end of [#GLDV-3005]

	dispidBnaGetPositionStatus = 209L,
	dispidBnaGetRefusedStatus = 210L,

	eventidTestEvent = 1L,
	eventidOnDisplayScreen = 2L,
	eventidOnDisplayData = 3L,
	eventidOnDisplayPinpadData = 4L,
	eventidOnSafeDoorChanged = 5L,		// [#RWC6-14] US William 2019.09.18 Add Safe Door Status To Journal
	//}}AFX_DISP_ID
	};
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NHMWICTL_H__F2D0FAA2_5575_4DAF_90AA_5E12319C224B__INCLUDED)
