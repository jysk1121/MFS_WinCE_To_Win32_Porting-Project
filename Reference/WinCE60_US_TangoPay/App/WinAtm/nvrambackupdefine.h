#ifndef __NVRAM_BACKUP_DEFINE_H__
#define __NVRAM_BACKUP_DEFINE_H__

//int	gnAccessNvramID[] = {2, 3, 4, 6, 7, 8};
int	gnAccessNvramID[] = {2, 3, 4, 6, 7, 8, 9, 12, 13};
#define COUNT_ACCESS_NVRAM (sizeof(gnAccessNvramID)/sizeof(int))

#define	FILE_ID		(0x73708003)
#define	FILE_ID_AU_NZ	(0x73708004) // [#2046] AU KJW 2011.04.19

#pragma pack(1)

typedef struct _NVRAM_BACKUP_HEADER_
{
	long	bIdentity;
	short	nCountRecode;
} NVRAM_BACKUP_HEADER, *PNVRAM_BACKUP_HEADER;

typedef struct _NVRAM_BACKUP_RECORD_
{
	long    nCrc;
	BYTE	nTI;
	long	nID;
	long	nSize;
	BYTE	Data[4096];			// 4K 로 고정. 추후 변경 가능성 있으면 수정 필요.
} NVRAM_BACKUP_RECORD, *PNVRAM_BACKUP_RECORD;

//[#553] SOOK 2009.08.08  NVRAM RESERVED 영역 BACKUP & RESTROE
//기존 사양 변경하지 않도록 추가함 
#define	FILE_RESERVED_ID		(0x73708004)
#define RESERVED_RECORD_ID		100

#pragma pack()

#endif __NVRAM_BACKUP_DEFINE_H__
