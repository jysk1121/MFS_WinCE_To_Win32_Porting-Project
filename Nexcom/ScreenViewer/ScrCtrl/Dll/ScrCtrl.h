/*----------------------------------------------------------------------

	AP와 SCREEN간의 인터페이스를 제공한다.

	작성자 : AIREAT
	작성일 : 2008.01.16
----------------------------------------------------------------------*/

#ifndef __NH_SCR_CTRL_H__
#define __NH_SCR_CTRL_H__

#ifndef _SCRCTRL_OWNER_
#pragma comment(lib, "ScrCtrl.lib")
#endif

class AFX_CLASS_EXPORT CScrCtrl
{
public:
	// 생성자, 소멸자.
	CScrCtrl();
	~CScrCtrl();

private:
	BOOL	m_bInit;

public:
	// 생성, 삭제 인터페이스.									// 화면 서비스를 가동 시킨다.
	BOOL	Initialize(HINSTANCE hInstance,
						int		nWidth=0,
						int		nHeight=0,
						CString strLoadSection=_T("ATM"),
						CString strMasterPath=_T("\\ATM\\Screen"));

	BOOL	DeInitalize();											// 화면 서비스를 종료 한다.
	
	BOOL	GetState();											// 초가화 상태를 가져온다.

	// 화면 관련 인터페이스
	BOOL	SetVariable(CString Command, CString Data);			// AP에서 SCREEN에 화면 데이터 설정.	(AP -> SCR)
	BOOL	SetKeyEvent(CString Key);							// AP에서 SCREEN에 Key Event 설정.		(AP -> SCR)
	BOOL	SetByPassData(CString Data);						// SCR에서 해당 데이터 처리 하지 않고 AP로 BYPASS

	BOOL	GetUserInputCheck(DWORD dwTimeout = 20);				// SCREEN에서 AP로 결과가 있는지 check.	(SCR -> AP)
	BOOL	GetUserInputData(CString &Name, CString &Data);		// SCREEN에서 AP로 주는 결과 값 가져 옴.(SCR -> AP)
	void	ClearUserInputData();
	// end of [#75]

	CString	GetAPTextIDString(CString strTextID);				// APTextID에 해당하는 문자열을 반환한다.		// [#237] NH AIREAT 2008.05.27

	// 화면 정보 관련 인터페이스
	CString	GetCurrentScreenNumber();								// 현재 표시되고 있는 화면 번호를 가져 옴.
	void	SetScreenLocale(int Locale = 0);						// 화면의 LOCALE을 설정함.
	int		GetScreenLocale();										// 화면의 LOCALE을 조회함.
	BOOL	SetDemoMode(BOOL bDemo = FALSE);						// 데모모드 정보를 화면에 표시 하도록 설정

	void	DrawDebugMsg(int x, int y, CString strMsg);
	void	SetActiveScreen();

	// for Camera Pos
	RECT	GetPosValue4Camera();

	// for ScreenViewer
	int		GetScreenCount();										// 로딩된 화면의 갯수를 가져온다.
	CString	GetFirstScreenNumber();
	CString	GetNextScreenNumber();
};

#endif __NH_SCR_CTRL_H__