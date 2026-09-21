// LineCtrl.cpp: implementation of the CLineCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\Common\CmnLib.h"
#include ".\Net\LineCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CLineCtrl*	CLineCtrl::m_pNetLines[MAX_LINETYPE]={NULL,};
char		CLineCtrl::m_RecvBuf[2048]={0,};

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

	return m_pNetLines[LineType];
}

void CLineCtrl::Register(enum TypeOfLine LineType, CLineCtrl *pLine)
{
	if (LineType < LINE_DIALUP || LineType >= MAX_LINETYPE)		return;

	if (pLine)		m_pNetLines[LineType] = pLine;
}