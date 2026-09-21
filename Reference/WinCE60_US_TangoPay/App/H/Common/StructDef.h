#ifndef _STRUCT_DEF_H_
#define _STRUCT_DEF_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

///////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------------
// STRUCTURE-TYPE ARRAY 정의
// ----------------------------------------------------------------------------
#pragma pack(1)
	typedef struct _Tag_ItemList
	{
		char		szItem[64];									// 항목
	} ITEMLIST, *LPITEMLIST;
#pragma pack()
// ----------------------------------------------------------------------------
// Device MethodList
// ----------------------------------------------------------------------------
#pragma pack(1)
	typedef struct _Tag_MethodList
	{
		char		szMethodName[64];
		char		szMethodSPUse[8];
	} METHODLIST, *LPMETHODLIST;
#pragma pack()

// ----------------------------------------------------------------------------
// Device의 상태정보를 저장
// ----------------------------------------------------------------------------
#pragma pack(1)
	typedef struct _Tag_DataList
	{
		CString		strDataName;									// 데이타이름
		CString		strDataValue;									// 값
	} DATALIST, *LPDATALIST;
#pragma pack()

// ============================================================================
///////////////////////////////////////////////////////////////////////////////
#endif // _STRUCT_DEF_H_
