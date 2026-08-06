/** *************************************************************************
*	@file Queue.cpp 
*	@date 2017/10/20
*	@author MFS
*	@brief 라인으로 부터 받은 데이터를 저장하는 버퍼를 구현한 소스파일입니다.
*****************************************************************************/

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

static CRITICAL_SECTION g_cs;
/** **********************************************
* @brief 큐와 관련된 메모리들을 동적 할당 합니다. 
* @param 없음 
* @retval 없음 
* @todo InitializeCriticalSection의 역할 파악 불가.
**************************************************/
RECV_QUEUE::RECV_QUEUE()
{
	recv_kind = NULL;
	recv_kind = new char[QUEUE_SIZE];
	recv_buff = NULL;
	recv_buff = new char[QUEUE_SIZE];

	InitializeCriticalSection( &g_cs );
}

/** *******************************************
* @brief 동적으로 할당된 큐 관련 메모리들을 해체합니다.
* @param 없음 
* @retval 없음 
* @todo DeleteCriticalSection의 역할 파악 불가. 
***********************************************/
RECV_QUEUE::~RECV_QUEUE()
{
	DeleteCriticalSection( &g_cs );

	if (recv_kind != NULL)
	{
		delete [] recv_kind;
		recv_kind = NULL;
	}

	if (recv_buff != NULL)
	{
		delete [] recv_buff;
		recv_buff = NULL;
	}
}

RECV_QUEUE	m_que; //객체 생성 

int		put_data(char *recv_data, int recv_size);
int		put_event(char recv_evt);
int		get_queue(char *ctl_code);
int		clear_queue();
int		check_queue(int check_kind, char check_data);

// queue input시 input 성공한 byte를 return한다.
/** **************************************************************************************************
* @brief 데이터를 버퍼로 저장하는 함수입니다.
* @param char *recv_data 받는 데이터 
* @param int recv_size 받는 데이터의 크기 
* @retval 0 큐 에러 
* @retval i 받은 데이터를 버퍼로 저장한 크기만큼 반환. @n 정상적으로 받았다면 데이터의 사이즈가 반환됨.
* @todo 세이브 포인트와 진행 포인트의 개념이 확실하지 않음 
******************************************************************************************************/
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

/** ***********************************************************
* @brief 이벤트 데이터를 받는 함수.
* @param char recv_evt 이벤트 문자
* @retval 0 큐 에러 
* @retval 정상 
***************************************************************/
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

/** ********************************************************
* @brief 데이터와 이벤트중 어떤 것을 받았는지 구분해 주는 함수입니다.
* @param char *ctl_code
* @retval RECV_DATA 데이터를 받음.
* @retval RECV_EVT 이벤트를 받음.
* @retval 3 큐 받기 에러 
************************************************************/
int	get_queue(char *ctl_code)
{
	EnterCriticalSection( &g_cs );

	int		rtn_code = 0;
	char	get_data = 0;

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

/** ********************************************************
* @brief 큐를 초기화하는 함수입니다. 초기화 과정에 세이브 포인트와 진행 포인트도 0으로 초기화됩니다.  
* @param 없음 
* @retval 0 정상적으로 초기화됨을 의미함.
************************************************************/
int	clear_queue()
{
	EnterCriticalSection( &g_cs );	

	m_que.recv_sp = 0;
	m_que.recv_pp = 0;
	memset( m_que.recv_kind, 0x00, QUEUE_SIZE );
	memset( m_que.recv_buff, 0x00, QUEUE_SIZE );

	LeaveCriticalSection( &g_cs );
	return 0;
}

int	check_queue(int check_kind, char check_data)
{
	EnterCriticalSection( &g_cs );

	int check_pp = 0;

	if (m_que.recv_sp != m_que.recv_pp)
	{
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
	}

	LeaveCriticalSection( &g_cs );

	return 0;
}
