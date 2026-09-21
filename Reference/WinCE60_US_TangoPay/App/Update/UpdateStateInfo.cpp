#include "StdAfx.h"
#include "UpdateStateInfo.h"

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateStateInfo
 FUNCTION NAME: CUpdateStateInfo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUpdateStateInfo::CUpdateStateInfo()
{
	Clear();
}


/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateStateInfo
 FUNCTION NAME: ~CUpdateStateInfo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자.
-------------------------------------------------------------------*/
CUpdateStateInfo::~CUpdateStateInfo()
{
	Clear();
}

void CUpdateStateInfo::GetGlobalVariable()
{

}

/*-------------------------------------------------------------------
 CLASS    NAME: CUpdateStateInfo
 FUNCTION NAME: Clear()
 RETURN TYPE  : none
 PARAMETER    : none
 DESCRIPTION  : 설정된 데이터를 삭제한다.
-------------------------------------------------------------------*/
void CUpdateStateInfo::Clear()
{
	m_bUpdateSuccessful = FALSE;
	m_eUpdateSource = FROM_NONE;
	m_eUpdateType = UT_NONE;
}