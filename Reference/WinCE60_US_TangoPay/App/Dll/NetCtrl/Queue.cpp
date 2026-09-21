// Queue.cpp: implementation of the CQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Queue.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// [#2042] NH KJW 2011.04.04 queue 버퍼를 동적할당함에 따라 RECV_QUEUE를 struct -> class로 변경하여 생성자에서 queue 동적할당토록 수정함.
static CRITICAL_SECTION g_cs;

RECV_QUEUE::RECV_QUEUE()
{
	recv_kind = NULL;	// [###2]
	recv_kind = new char[QUEUE_SIZE];
	recv_buff = NULL;	// [###2]
	recv_buff = new char[QUEUE_SIZE];

	InitializeCriticalSection( &g_cs );
}

RECV_QUEUE::~RECV_QUEUE()
{
	DeleteCriticalSection( &g_cs );

	// [###2]
	if (recv_kind != NULL)
	{
		delete [] recv_kind;
		recv_kind = NULL;
	}

	// [###2]
	if (recv_buff != NULL)
	{
		delete [] recv_buff;
		recv_buff = NULL;
	}
}
// end of [#2042]

RECV_QUEUE	m_que;

int		put_data(char *recv_data, int recv_size);
int		put_event(char recv_evt);
int		get_queue(char *ctl_code);
int		clear_queue();
int		check_queue(int check_kind, char check_data);		// [#595] NH KSK 2009.12.09
int		get_size();											// [#GLDV-2683] NH Kook 2019.11.19 Modem Removal Detection

// queue input시 input 성공한 byte를 return한다.
int	put_data(char *recv_data, int recv_size)
{
	EnterCriticalSection( &g_cs );

	int	chk_que = m_que.recv_sp + 1;
	chk_que %= QUEUE_SIZE;
	// buffer full check
	if (chk_que == m_que.recv_pp)
	{
		// buffer clear (put queue error)
		clear_queue();
		LeaveCriticalSection( &g_cs );
		return 0;
	}

	for(int i=0; i<recv_size; i++)
	{
		m_que.recv_kind[m_que.recv_sp] = RECV_DATA;
		m_que.recv_buff[m_que.recv_sp] = recv_data[i];
		m_que.recv_sp++;
		m_que.recv_sp %= QUEUE_SIZE;

		// buffer full check
		if (m_que.recv_sp == m_que.recv_pp)
		{
			LeaveCriticalSection( &g_cs );
			return i;
		}
	}

	LeaveCriticalSection( &g_cs );

	return recv_size;
}

int	put_event(char recv_evt)
{
	EnterCriticalSection( &g_cs );

	int	chk_que = m_que.recv_sp + 1;
	chk_que %= QUEUE_SIZE;
	// buffer full check
	if (chk_que == m_que.recv_pp)
	{
		// buffer clear (put queue error)
		clear_queue();
		LeaveCriticalSection( &g_cs );
		return 0;
	}

	m_que.recv_kind[m_que.recv_sp] = RECV_EVENT;
	m_que.recv_buff[m_que.recv_sp] = recv_evt;
	m_que.recv_sp++;
	m_que.recv_sp %= QUEUE_SIZE;

	LeaveCriticalSection( &g_cs );

	return 1;
}

int	get_queue(char *ctl_code)
{
	EnterCriticalSection( &g_cs );	// [#2356] NH KSK 2015.07.09 Local 변수도 Sync되도록 위치 수정

	int		rtn_code = 0;
	char	get_data = 0;

//	EnterCriticalSection( &g_cs );

	if (m_que.recv_sp != m_que.recv_pp)
	{
		if (m_que.recv_kind[m_que.recv_pp] == RECV_DATA)
		{
			// data exist
			get_data = m_que.recv_buff[m_que.recv_pp];
			*ctl_code = get_data;
			// char buffer clear
			m_que.recv_buff[m_que.recv_pp] = 0;
			rtn_code = RECV_DATA;
		}
		else if (m_que.recv_kind[m_que.recv_pp] == RECV_EVENT)
		{
			// event exist
			get_data = m_que.recv_buff[m_que.recv_pp];
			*ctl_code = get_data;
			// char buffer clear
			m_que.recv_buff[m_que.recv_pp] = 0;
			rtn_code = RECV_EVENT;
		}
		else
		{
			// logic error
			LeaveCriticalSection( &g_cs );
			return 3;
		}
		m_que.recv_pp++;
		m_que.recv_pp %= QUEUE_SIZE;
	}

	LeaveCriticalSection( &g_cs );

	return rtn_code;
}

int	clear_queue()
{
	EnterCriticalSection( &g_cs );			// [#2356] NH KSK 2015.07.09 clear queue 동기화 로직 추가

	// [#2042] NH KJW 2011.04.04
	m_que.recv_sp = 0;
	m_que.recv_pp = 0;
	memset( m_que.recv_kind, 0x00, QUEUE_SIZE );
	memset( m_que.recv_buff, 0x00, QUEUE_SIZE );
	// end of [#2042]

	LeaveCriticalSection( &g_cs );			// [#2356] NH KSK 2015.07.09 clear queue 동기화 로직 추가
	return 0;
}

// [#595] NH KSK 2009.12.09
int	check_queue(int check_kind, char check_data)
{
	EnterCriticalSection( &g_cs );		// [#2356] NH KSK 2015.07.09 Local 변수도 Sync되도록 위치 수정

	int check_pp = 0;

//	EnterCriticalSection( &g_cs );

	if (m_que.recv_sp != m_que.recv_pp)
	{
		// [#2356] NH KSK 2015.07.09 QUEUE_SIZE 매크로 관련 연산 우선순위 문제로 인해 check_pp값이 정상적으로 증가되지 않는 Bug Fix
		// 추가적으로 QUEUE Loop시 첫번째 data를 access하지 못하는 잠재 버그 추가 수정
//		for(check_pp = m_que.recv_pp; check_pp != m_que.recv_sp;check_pp = (check_pp++ % QUEUE_SIZE))
//		{
//			if (m_que.recv_kind[check_pp] == check_kind)
//			{
//				if (m_que.recv_buff[check_pp] == check_data)
//				{
//					LeaveCriticalSection( &g_cs );
//					return 1;
//				}
//			}
//		}
		for(check_pp = m_que.recv_pp; check_pp != m_que.recv_sp; check_pp++)
		{
			check_pp %= QUEUE_SIZE;

			if (m_que.recv_kind[check_pp] == check_kind)
			{
				if (m_que.recv_buff[check_pp] == check_data)
				{
					LeaveCriticalSection( &g_cs );
					return 1;
				}
			}
		}
		// end of [#2356]
	}

	LeaveCriticalSection( &g_cs );

	return 0;
}
// end of [#595]

// [#GLDV-2683] NH Kook 2019.11.19 Modem Removal Detection
int	get_size()
{
	return (m_que.recv_sp - m_que.recv_pp + QUEUE_SIZE) % QUEUE_SIZE;
}
// end of [#GLDV-2683]
