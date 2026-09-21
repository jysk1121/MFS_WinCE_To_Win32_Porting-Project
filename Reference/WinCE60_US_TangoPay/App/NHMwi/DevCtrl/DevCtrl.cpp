// DevCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "..\nhmwi.h"
#include "DevCtrl.h"

#include "..\Matrix\Matrix.h"
#include "..\System\EventQue.h"
#include ".\Common\CommDef.h"
#include ".\Common\ConstDef.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDevCtrl

// ----------------------------------------------------------------------------
// 함 수 명 : 
// DESCRIPT : 생성자 함수
// 매개변수 : 
// 반 환 값 : 
// ----------------------------------------------------------------------------
CDevCtrl::CDevCtrl()
{
	m_pMcuMatrix	= new CMatrix;
	m_pRFIDMatrix	= new CMatrix;						// [#2325] NH KSK 2015.01.28
	m_pSprMatrix	= new CMatrix;
//-NOUSE	m_pJprMatrix	= new CMatrix;				// AIREAT 2009.07.14
	m_pCduMatrix	= new CMatrix;						// 2004.01.15_1.2_2003 : CDU추가
	m_pPinMatrix	= new CMatrix;
	m_pBcrMatrix	= new CMatrix;						// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	m_pBnaMatrix	= new CMatrix;						// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	
	m_pMcuEventQ	= new CEventQue;
	m_pRFIDEventQ	= new CEventQue;					// [#2325] NH KSK 2015.01.21
	m_pSprEventQ	= new CEventQue;
//-NOUSE	m_pJprEventQ	= new CEventQue;			// AIREAT 2009.07.14
	m_pScrEventQ	= new CEventQue;
	m_pCduEventQ	= new CEventQue;					// 2004.01.15_1.2_2003 : CDU추가
	m_pPinEventQ	= new CEventQue;
	m_pBcrEventQ	= new CEventQue;					// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	m_pBnaEventQ	= new CEventQue;					// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	// end of [#GLDV-3005]

	SetOwner(/*NULL*/);											// Set Owner

	// Set Device Method-Event Matrix : Load Matrix
																// 2004.01.15_1.2_2003 : CDU추가	
																// 2004.12.24 VFD추가
																// [#2325] NH KSK 2015.01.28 RFID 추가
																// [#GLDV-3005] US Kook 2021.12.21 Add BCR/BNA
	SetDeviceMatrix(DEV_MCU  | \
					DEV_RFID | \
					DEV_SPR  | \
					DEV_JPR  | \
					DEV_CDU	 | \
					DEV_BCR  | \
					DEV_BNA  | \
					DEV_PIN	  );
}

// ----------------------------------------------------------------------------
// 함 수 명 : 
// DESCRIPT : 소멸자 함수
// 매개변수 : 
// 반 환 값 : 
// ----------------------------------------------------------------------------
CDevCtrl::~CDevCtrl()
{
	if (m_pMcuMatrix != NULL)	{	delete m_pMcuMatrix;		m_pMcuMatrix = NULL;	}
	if (m_pRFIDMatrix != NULL)	{	delete m_pRFIDMatrix;		m_pRFIDMatrix = NULL;	}	// [#2325] NH KSK 2015.01.28
	if (m_pSprMatrix != NULL)	{	delete m_pSprMatrix;		m_pSprMatrix = NULL;	}
//-NOUSE	if (m_pJprMatrix != NULL)	{	delete m_pJprMatrix;		m_pJprMatrix = NULL;	}			// AIREAT 2009.07.14
	if (m_pCduMatrix != NULL)	{	delete m_pCduMatrix;		m_pCduMatrix = NULL;	}	// 2004.01.15_1.2_2003 : CDU추가
	if (m_pPinMatrix != NULL)	{	delete m_pPinMatrix;		m_pPinMatrix = NULL;	}
	if (m_pBcrMatrix != NULL)	{	delete m_pBcrMatrix;		m_pBcrMatrix = NULL;	}	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	if (m_pBnaMatrix != NULL)	{	delete m_pBnaMatrix;		m_pBnaMatrix = NULL;	}	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car

	if (m_pMcuEventQ != NULL)	{	delete m_pMcuEventQ;		m_pMcuEventQ = NULL;	}
	if (m_pRFIDEventQ != NULL)	{	delete m_pRFIDEventQ;		m_pRFIDEventQ = NULL;	}	// [#2325] NH KSK 2015.01.21
	if (m_pSprEventQ != NULL)	{	delete m_pSprEventQ;		m_pSprEventQ = NULL;	}
//-NOUSE	if (m_pJprEventQ != NULL)	{	delete m_pJprEventQ;		m_pJprEventQ = NULL;	}			// AIREAT 2009.07.14
	if (m_pScrEventQ != NULL)	{	delete m_pScrEventQ;		m_pScrEventQ = NULL;	}
	if (m_pCduEventQ != NULL)	{	delete m_pCduEventQ;		m_pCduEventQ = NULL;	}	// 2004.01.15_1.2_2003 : CDU추가
	if (m_pPinEventQ != NULL)	{	delete m_pPinEventQ;		m_pPinEventQ = NULL;	}
	if (m_pBcrEventQ != NULL)	{	delete m_pBcrEventQ;		m_pBcrEventQ = NULL;	}	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
	if (m_pBnaEventQ != NULL)	{	delete m_pBnaEventQ;		m_pBnaEventQ = NULL;	}	// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
}


BEGIN_MESSAGE_MAP(CDevCtrl, CWnd)
	//{{AFX_MSG_MAP(CDevCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDevCtrl message handlers

// ----------------------------------------------------------------------------
// 함 수 명 : void CDevCtrl::SetOwner(CNHMWICtrl* pOwner /* = NULL */)
// DESCRIPT : 초기화 처리루틴
// 매개변수 : 상위 포인터(CHNMWI Class)
// 반 환 값 : 없음
// ----------------------------------------------------------------------------
void CDevCtrl::SetOwner(CNHMWICtrl* pOwner /* = NULL */)
{
	m_pOwner = pOwner;
}

