#include "StdAfx.h"
#include "TimeCheck.h"


// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: CTimeCheck()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CTimeCheck::CTimeCheck()
{
	m_TargetTime = COleDateTime::GetCurrentTime();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: CTimeCheck()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CTimeCheck::CTimeCheck(COleDateTime NewTime)
{
	m_TargetTime = NewTime;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: ~CTimeCheck()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자.
-------------------------------------------------------------------*/
CTimeCheck::~CTimeCheck()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: SetTargetTime()
 RETURN TYPE  : 
 PARAMETER    : NetTime : 새로운 시간.
 DESCRIPTION  : 새로운 Target 시간을 설정한다.
-------------------------------------------------------------------*/
void CTimeCheck::SetTargetTime(COleDateTime NewTime)
{
	m_TargetTime = NewTime;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: SetTargetTime()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 시간 부터 Day 만큼 증가된 Target 시간을 설정한다.
-------------------------------------------------------------------*/
void CTimeCheck::SetTargetTimeAfterDay(long lDays)
{
	SetTargetTime(lDays, 0, 0, 0);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: SetTargetTime()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 시간 부터 Hour 만큼 증가된 Target 시간을 설정한다.
-------------------------------------------------------------------*/
void CTimeCheck::SetTargetTimeAfterHour(int nHours)
{
	SetTargetTime(0, nHours, 0, 0);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: SetTargetTime()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 시간 부터 Min 만큼 증가된 Target 시간을 설정한다.
-------------------------------------------------------------------*/
void CTimeCheck::SetTargetTimeAfterMin(int nMins)
{
	SetTargetTime(0, 0, nMins, 0);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: SetTargetTime()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 시간 부터 Sec 만큼 증가된 Target 시간을 설정한다.
-------------------------------------------------------------------*/
void CTimeCheck::SetTargetTimeAfterSec(int nSecs)
{
	SetTargetTime(0, 0, 0, nSecs);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: SetTargetTime()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 시간 부터 증가된 Target 시간을 설정한다.
-------------------------------------------------------------------*/
void CTimeCheck::SetTargetTime(long lDays, int nHours, int nMins, int nSecs)
{
	COleDateTimeSpan	RetryInterVal(lDays, nHours, nMins, nSecs);

	m_TargetTime  = COleDateTime::GetCurrentTime();
	m_TargetTime += RetryInterVal;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: AddTargetTime()
 RETURN TYPE  : 
 PARAMETER    : AddTimeSpan : 증가할 시간
 DESCRIPTION  : AddTimeSpan 만큼 Target을 증가한다.
-------------------------------------------------------------------*/
void CTimeCheck::AddTargetTime(COleDateTimeSpan AddTimeSpan)
{
	m_TargetTime += AddTimeSpan;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CTimeCheck
 FUNCTION NAME: IsElapsedTimes()
 RETURN TYPE  : TRUE  : Target 시간이 경과 되었다.
				FALSE : Target 시간이 경과 되지 않았다.
 PARAMETER    : 
 DESCRIPTION  : Target 시간이 경과 되었는가.
-------------------------------------------------------------------*/
BOOL CTimeCheck::IsElapsedTimes()
{
	m_CurTime = COleDateTime::GetCurrentTime();

	// 확실히 경과했다.
	if (m_CurTime > m_TargetTime)
		return TRUE;

	return FALSE;
}