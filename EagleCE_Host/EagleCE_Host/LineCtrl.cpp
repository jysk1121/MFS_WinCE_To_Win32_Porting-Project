/** **********************************************************************************************
*	@file LineCtrl.cpp
*	@date 2017/10/20
*	@author MFS
*	@brief TCP라인과 모뎀라인을 구현할 수 있게 만든 틀의 소스파일입니다. 
**************************************************************************************************/

// LineCtrl.cpp: implementation of the CLineCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LineCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CLineCtrl*	CLineCtrl::m_pNetLines[MAX_LINETYPE]={NULL,};
//char		CLineCtrl::m_RecvBuf[2048]={ 0, };
char		CLineCtrl::m_RecvBuf[4096]={ 0, };	//*LEHTEST

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLineCtrl::CLineCtrl()
{

}

CLineCtrl::~CLineCtrl()
{

}

CLineCtrl* CLineCtrl::Instance(int LineType)
{
	if (LineType >= MAX_LINETYPE)		return NULL;

	//CString strTemp;
	//strTemp.Format(_T("Instance (%X)\n"), (int)m_pNetLines[LineType]);

	return m_pNetLines[LineType];
}
/** ***********************************************************************************************
* @brief 라인타입을 저장해주는 함수입니다.
* @details 1행: 라인타입 예외처리 구현 @n 2행~6행: 받은 객체의 주소값을 int형으로 캐스팅 해주고 특정 변수에 저장
* @param int nLineType @n 라인 종류 [TCP라인, 모뎀 라인] 
* @param CLineCtrl *pLine @n 객체의 주소값을 받습니다. 
* @retval 없음 
***************************************************************************************************/
void CLineCtrl::Register(int nLineType, CLineCtrl *pLine)
{
	if (nLineType < LINE_TCP || nLineType >= MAX_LINETYPE)		return;

	//CString strTemp;
	//strTemp.Format(_T("Register (%X)\n"), (int)pLine);

	if (pLine)		m_pNetLines[nLineType] = pLine;
}