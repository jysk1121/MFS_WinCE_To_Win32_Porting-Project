#if !defined(AFX_DEVCTRL_H__1EAC6B8A_99EF_47B0_AF9C_A06C422FAD60__INCLUDED_)
#define AFX_DEVCTRL_H__1EAC6B8A_99EF_47B0_AF9C_A06C422FAD60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DevCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDevCtrl window
class CNHMWICtrl;
class CMatrix;
class CEventQue;

class CDevCtrl : public CWnd
{
// Construction
public:
	CDevCtrl();

// Attributes
public:
	// ------------------------------------------------------------------------
	// Common Control Objects 
	// ------------------------------------------------------------------------
	CNHMWICtrl*		m_pOwner;									// 상위 pointer

	// ------------------------------------------------------------------------
	// Device Control Objects
	// ------------------------------------------------------------------------
	CMatrix*		m_pMcuMatrix;								// MCU(Card Reader) Matrix Object
	CMatrix*		m_pRFIDMatrix;								// RFID [#2325] NH KSK 2015.01.20
	CMatrix*		m_pSprMatrix;								// SPR(Slip/Receipt Printer) Matrix Object
//-NOUSE	CMatrix*		m_pJprMatrix;								// JPR(Journal Printer) Matrix Object			// AIREAT 2009.07.14
	CMatrix*		m_pCduMatrix;								// CashDispenser : 2004.01.15_1.2_2003
	CMatrix*		m_pPinMatrix;								// Pinpad : 2004.02.07
	CMatrix*		m_pBcrMatrix;								// [#GLDV-3005] US Kook 2021.10.20 Support Side Car
	CMatrix*		m_pBnaMatrix;								// [#GLDV-3005] US Kook 2021.10.20 Support Side Car

	// ------------------------------------------------------------------------
	// Device Event Queue Objects
	// ------------------------------------------------------------------------
	CEventQue*		m_pMcuEventQ;								// Device Event Queue
	CEventQue*		m_pRFIDEventQ;								// [#2325] NH KSK 2015.01.20
	CEventQue*		m_pSprEventQ;								// Device Event Queue
//-NOUSE	CEventQue*		m_pJprEventQ;								// Device Event Queue			// AIREAT 2009.07.14
	CEventQue*		m_pScrEventQ;								// Screen Event Queue
	CEventQue*		m_pCduEventQ;								// CDU    Event Queue : 2004.01.15_1.2_2003
	CEventQue*		m_pPinEventQ;								// PINPAD Event Queue : 2004.02.07
	CEventQue*		m_pBcrEventQ;								// [#GLDV-3005] US Kook 2021.10.20 Support Side Car
	CEventQue*		m_pBnaEventQ;								// [#GLDV-3005] US Kook 2021.10.20 Support Side Car
	
	// ------------------------------------------------------------------------
	// Device Status Checking Objects
	// ------------------------------------------------------------------------
	CPtrList		m_lstDevStatus;								// 데이타를 저장(=>디바이스 상태저장)



// Operations
public:
	// ------------------------------------------------------------------------
	// DevCtrl.cpp	:	Constructing Functions
	// ------------------------------------------------------------------------
	void		SetOwner(CNHMWICtrl*	pOwner = NULL);			// 초기화처리

	// ------------------------------------------------------------------------
	// CheckDev.cpp	:	CheckDeviceAction
	// ------------------------------------------------------------------------
	void		ProcSetDeviceEvent(char* szDeviceName, char* szEventName, char* szEventValue);
																// 발생된 이벤트를 처리한다
	int			CheckDeviceAction(int nDevID, int nWaitSec);	// 장치디바이스 동작확인
	int			CheckDeviceMatrix(int nDevID);					// 개별장치 동작확인
	BOOL		CheckDeviceEvent(int nDevID);					// 개별장치 이벤트 확인
	int			CheckDeviceEventMatrix(int nDevID);				// 개별장치 메트릭스확인

	int			CheckDeviceOpend(int nDevID, int nWaitSec);		// 장치디바이스 오픈 체크
	int			CheckDeviceClosed(int nDevID, int nWaitSec);	// 장치디바이스 클로우즈 체크
		
	int			ScanDeviceAction(int nDevID, int nWaitSec, int nEventKind);
	CString		GetScrKeyString(int nWaitSec);					// 화면입력 데이타를 조회한다.

	int			WriteMwiErrorCode(int nDevID, char* szEntry, int nCodeLen, char* szErrorCode);
																// 레지스트리에 MWI장애코드기록

	// ------------------------------------------------------------------------
	// MatrixCtrl.cpp	:	Matrix control Functions
	// ------------------------------------------------------------------------
	CMatrix*	GetMatrix(int nDevID);
	int			GetDevIDByDevName(char* szDeviceName);		// 장치이름으로 장치ID조회
	CString		GetDevNameByDevID(INT nDevID);					// 장치ID로 장치이름조회

	int			GetMethodIndexByName(INT nDevID, LPCTSTR szMethodName);
	CString		GetMethodNameByIndex(INT nDevID, int nMethodIndex);
	int			GetEventIndexByName(INT nDevID, char* szEventName);
	CString		GetEventNameByIndex(INT nDevID, int nEventIndex);

	int			GetCurrMethodIndex(INT nDevID);					// 장치의 현재 메소드의 인덱스를 구한다.

	BOOL		SetDeviceMatrix(INT	nDeviceID);					// 장치별 메소드/이벤트 메트릭스정보를 설정한다.
	BOOL		SetEventMatrix(INT nDeviceID, char* szMethodName);
																// 장치/CMD별로 메소드호출시 이벤트메트릭스를 설정
	BOOL		SetEventMatrixValue(INT nDeviceID, INT nEventIndex);
	BOOL		SetEventMatrixValue(char* szDeviceName, char* szEventName);

	// ------------------------------------------------------------------------
	// EvtQCtrl.cpp	:	Device Event Queue Control Functions
	// ------------------------------------------------------------------------
	CEventQue*	GetEventQByDevID(int nDevID);					// 장치ID를 이용하여 해당이벤트큐를 얻는다
	BOOL		EvtQInit(int nDevID, LPCSTR	szOwner);			// 이벤트 큐를 초기설정한다.
	BOOL		EvtQReset(int nDevID);							// 이벤트큐를 초기화(큐데이타삭제처리)한다.
	int			EvtQGetCount(int nDevID);						// 큐에 쌓여있는 이벤트의 갯수를 확인한다.
	BOOL		EvtQIsEmpty(int nDevID);						// 큐가 비어있는지 확인한다.

	BOOL		EvtQPutData(char* szType, char* szName, char* szEvent, char* szValue);
																// 큐에 데이타를 삽입한다.
	BOOL		EvtQGetData(char* szType,  char* szName,  char* szEvent,  char* szValue);
																// 큐에서 데이타를 가져온다.
	BOOL		EvtQCheckDeviceDataExist(char* szType,  char* szName,  char* szEvent,  char* szValue);
																// 큐에 데이타가 있는지 확인한다.

	BOOL		EvtQPumpingEventToError(int nDevID);			// Error Event가 검지될때까지 큐를 펌핑한다.(개벌처리)
	BOOL		EvtQPumpingCheck(int nDevID, int nCheckStatus = 0);	// Error Event가 검지될때까지 큐를 펌핑한다.(일괄처리)
	
	// ------------------------------------------------------------------------
	// DevStatCtrl.cpp	:	DataList Structure(DeviceStatus) control Functions
	// ------------------------------------------------------------------------
	BOOL		ResetDeviceStatus();							// 상태버퍼 클리어
	int			GetDeviceStatusCount();							// 상태값 갯수산출
	BOOL		CheckDeviceStatusExist(LPCTSTR szName);			// 상태버퍼 비어있는지 확인
	BOOL		AddDeviceStatus(LPCTSTR szDataName, LPCTSTR szDataValue);	// 상태값 추가
	BOOL		SetDeviceStatus(LPCTSTR szDataName, LPCTSTR szDataValue);	// 상태값 갱신(직접사용금지)
	CString		GetDeviceStatus(LPCTSTR szDataName);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDevCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDevCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDevCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVCTRL_H__1EAC6B8A_99EF_47B0_AF9C_A06C422FAD60__INCLUDED_)
