#ifndef __TIME_CHECK_H__
#define __TIME_CHECK_H__

//////////////////////////////////////////////////////////////////////////
//	설정된 시간이 경과 되었는지 관리하는 class

class CTimeCheck
{
public:
	CTimeCheck();
	CTimeCheck(COleDateTime NewTime);
	~CTimeCheck();

private:
	COleDateTime	m_TargetTime;
	COleDateTime	m_CurTime;

public:
	/**
	 * Returns the number of seconds remaining in the timeout period
	 */
	double	GetSecondsRemaining();
	void	SetTargetTime(COleDateTime NewTime);								// 새로운 Target 시간을 설정한다.
	void	SetTargetTimeAfterDay(long lDays);									// 새로운 Target 시간을 설정한다.
	void	SetTargetTimeAfterHour(int nHours);									// 새로운 Target 시간을 설정한다.
	void	SetTargetTimeAfterMin(int nMins);									// 새로운 Target 시간을 설정한다.
	void	SetTargetTimeAfterSec(int nSecs);									// 새로운 Target 시간을 설정한다.
	void	SetTargetTime(long lDays, int nHours, int nMins, int nSecs);		// 새로운 Target 시간을 설정한다.
	void	AddTargetTime(COleDateTimeSpan AddTimeSpan);						// AddTimeSpan 만큼 Target을 증가한다.
	BOOL	IsElapsedTimes();													// Target 시간이 경과 되었는가.
};

#endif __TIME_CHECK_H__