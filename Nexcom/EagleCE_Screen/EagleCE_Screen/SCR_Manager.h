/*----------------------------------------------------------------------
	AP와 SCREEN간의 인터페이스를 제공한다.


----------------------------------------------------------------------*/

#ifndef __SCR_CTRL_H__
#define __SCR_CTRL_H__

class CSCR_Manager
{
public:
	// 인스턴스
	static CSCR_Manager* m_pInstance;

	// 생성자, 소멸자.
	CSCR_Manager();
	~CSCR_Manager();

private:
	BOOL	m_bInit;

public:
	// 인스턴스 생성
	static void CreateInstance();
	// 인스턴스 취득
	static CSCR_Manager* GetInstance();
	// 인스턴스 해제
	static void ReleaseInstance();

	// 생성, 삭제 인터페이스.									// 화면 서비스를 가동 시킨다.
	BOOL	Initialize(HINSTANCE hInstance,
						int		nWidth=0,
						int		nHeight=0,
						CString strLoadSection=_T("EAGLECE_ATM"),
						CString strMasterPath=_T("\\NAND\\EagleATM\\Screen"));

	BOOL	DeInitalize();										// 화면 서비스를 종료 한다.
	
	BOOL	GetState();											// 초가화 상태를 가져온다.

	// 화면 관련 인터페이스
	BOOL	SetVariable(CString Command, CString Data);			// AP에서 SCREEN에 화면 데이터 설정.	(AP -> SCR)
	//BOOL	SetKeyEvent(CString Key);							// AP에서 SCREEN에 Key Event 설정.		(AP -> SCR)
	BOOL	SetKeyEvent(CString Key, BOOL bADAMode = FALSE);	// AP에서 SCREEN에 Key Event 설정.		(AP -> SCR)
	BOOL	SetByPassData(CString Data);						// SCR에서 해당 데이터 처리 하지 않고 AP로 BYPASS

	BOOL	GetUserInputCheck(DWORD dwTimeout = 5);				// SCREEN에서 AP로 결과가 있는지 check.	(SCR -> AP)
	BOOL	GetUserInputData(CString &Name, CString &Data);		// SCREEN에서 AP로 주는 결과 값 가져 옴.(SCR -> AP)
	void	ClearUserInputData();

	CString	GetAPTextIDString(CString strTextID);				// APTextID에 해당하는 문자열을 반환한다.

	// 화면 정보 관련 인터페이스
	CString	GetCurrentScreenNumber();							// 현재 표시되고 있는 화면 번호를 가져 옴.
	void	SetScreenLocale(int Locale = 0);					// 화면의 LOCALE을 설정함.
	int		GetScreenLocale();									// 화면의 LOCALE을 조회함.
	BOOL	SetDemoMode(BOOL bDemo = FALSE);					// 데모모드 정보를 화면에 표시 하도록 설정

	void	DrawDebugMsg(int x, int y, CString strMsg);
	void	SetActiveScreen();

	// for ScreenViewer
	int		GetScreenCount();									// 로딩된 화면의 갯수를 가져온다.
	CString	GetFirstScreenNumber();
	CString	GetNextScreenNumber();

	// For Advertisement Setting
	void	SetAdvertisement(BOOL bActiveAdverEnable, int nRefreshTime = 5, CString strEnable = _T(""));	// Advertisements Value Setting

	// V1.0.2.5 2018.07.19 - for Camera Pos
	RECT	GetPosValue4Camera();

	// V1.0.2.5 2018.08.06 - ADV 파일을 매번 Decode/Release를 하지 않고, 원하는 시점에 Decode를 수행하기 위함.
	void	LoadAdvertisement();

	BOOL	FlipImage(CString strInputFilePath, CString strOutputFilePath);
};

#endif __SCR_CTRL_H__