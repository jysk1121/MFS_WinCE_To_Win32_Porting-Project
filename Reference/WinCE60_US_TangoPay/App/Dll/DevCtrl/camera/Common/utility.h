#include ".\..\common\common.h"

#ifndef _UTILITY_H_
#define _UTILITY_H_

// 환형큐를 설계합니다
//#define MAX_QUEUE_ENTRYCOUNT	(16)
#define MAX_QUEUE_ENTRYCOUNT	(32)
#define MAX_BUFFER_SIZE			(320*240*2)

typedef struct _LOOPQUEUE
{
	uint32_t			dwHead;
	uint32_t			dwTail;
	CRITICAL_SECTION_t	lqCriticalSection;
	bool_t				bInitialized;
	void_t *			QueueEntry[MAX_QUEUE_ENTRYCOUNT + 1];
	uint32_t			dwEntryMaxSize[MAX_QUEUE_ENTRYCOUNT + 1];
	uint32_t			dwEntryAcceptedSize[MAX_QUEUE_ENTRYCOUNT + 1];
}LOOPQUEUE_t, *PLOOPQUEUE_t;

UTIL_API bool_t	Util_ReadyResource();
UTIL_API bool_t	Util_FreeResource();

UTIL_API bool_t Util_InitializeLoopQueue(IN PLOOPQUEUE_t pLoopQueue);
UTIL_API bool_t Util_TerminateLoopQueue(IN PLOOPQUEUE_t pLoopQueue);

UTIL_API bool_t Util_EmptyLoopQueue(IN PLOOPQUEUE_t pLoopQueue);

UTIL_API bool_t Util_IsEmptyLoopQueue(IN PLOOPQUEUE_t pLoopQueue, OUT bool_t *pbEmpty);

UTIL_API bool_t Util_IsFullLoopQueue(IN PLOOPQUEUE_t pLoopQueue, OUT bool_t *pbFull);

UTIL_API bool_t Util_CopyAndAdvanceTailToLoopQueue(IN PLOOPQUEUE_t pLoopQueue, IN uint8_t * pEntry, IN uint32_t Size);

UTIL_API bool_t Util_GetHeadAndAdvanceFromLoopQueue(IN PLOOPQUEUE_t pLoopQueue, OUT uint8_t ** ppEntry, OUT uint32_t *pSize);

#ifdef USE_LOGFILE

	#define LOG_DATA_VERSION	"USB Driver Version 1.0.0.0"
	#define LOG_INFO			L"\\ATM2\\hucUsbConfig.ini"
//	#define LOG_INFO			L"\\ATM2\\ATMinsideConfig.ini"
	#define LOG_FIELD_LOGENABLE	"[LOGENABLE]="	// value : 0, 1
	#define LOG_FIELD_LOGPATH	"[LOGPATH]="		// string : /ATM2/CamLogs/
	#define LOG_FIELD_LOGFILES	"[MAXLOGS]="		// value : 1 - 60 (Default:30)
	#define LOG_FILES_DEFAULT	(30)
	#define LOG_FIELD_LOWFRAMERATE	"[LOWFRAMERATE]="	// value : 1

// LogFile Name Convention
// HUCDRV_20180103.LOG
#endif

#ifdef USE_LOGFILE
	UTIL_API bool_t Util_PrepareLogFile();
	UTIL_API bool_t Util_WriteLog(int bIsMainCamera, char *szLogText);
#endif

#endif //_UTILITY_H_