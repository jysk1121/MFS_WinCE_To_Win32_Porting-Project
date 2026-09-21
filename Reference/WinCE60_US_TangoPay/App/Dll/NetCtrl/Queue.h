// Queue.h: interface for the CQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUEUE_H__77721570_5C0E_49DF_B939_278BFD2B0FEC__INCLUDED_)
#define AFX_QUEUE_H__77721570_5C0E_49DF_B939_278BFD2B0FEC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define		QUEUE_SIZE		(1024 * 500)		// [#2356] NH KSK 2015.07.09 Queue Buffer 연산 우선순위 오류 수정

//#define		NET_BUFF_SIZE	2048	// V01.02.29 BUFFER SIZE 변경 REVIEW8 [#89] KSK 2008.04.14 // [#2012] NH KJW 2011.03.31 미사용 삭제

#define		DISCONNECT_EVT	'D'
#define		CONNECT_EVT		'C'
#define		ACCEPT_EVT		'A'
#define		NEWCALL_EVT		'N'
#define		TIMEOUT_EVT		'O'

// MODE DISCONNECT DETAIL INFO
#define		USER_NODIALTONE		0x01
#define		USER_NOANSWER		0x02
#define		USER_LINEBUSY		0x04

#define		RECV_DATA		1
#define		RECV_EVENT		2

// [#2042] NH KJW 2011.04.04 queue 버퍼를 동적할당함에 따라 RECV_QUEUE를 struct -> class로 변경하여 생성자에서 queue 동적할당토록 수정함.
//typedef	struct _recv_queue {
//	int		recv_sp;							// SAVE POINT
//	int		recv_pp;							// PROCESS POINT
//	char	recv_kind[QUEUE_SIZE];				// DATA or EVENT
//	char	recv_buff[QUEUE_SIZE];				// RECEIVE BUFFER
//}RECV_QUEUE;

class RECV_QUEUE
{
public:
	RECV_QUEUE();
	virtual ~RECV_QUEUE();

public:
	int		recv_sp;				// SAVE POINT
	int		recv_pp;				// PROCESS POINT
	char*	recv_kind;				// DATA or EVENT
	char*	recv_buff;				// RECEIVE BUFFER
};
// end of [#2042]

#endif // !defined(AFX_QUEUE_H__77721570_5C0E_49DF_B939_278BFD2B0FEC__INCLUDED_)
