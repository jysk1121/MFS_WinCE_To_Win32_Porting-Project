#pragma once


//
// 장치 관련 DLL for CE
//
#define EAGLE_ATM_CDR_DLL			_T("ICM330_1270CE.dll")
#define EAGLE_ATM_CDM_DLL			_T("MFS_CDM_WEC7.dll")
//#define EAGLE_ATM_DIO_DLL			_T("MFS_DIO_WEC7.dll")
#define EAGLE_ATM_EJR_DLL			_T("MFS_EJL_WEC7.dll")
#define EAGLE_ATM_EPP_DLL			_T("EPPDLL4CRYPTERA.dll")
#define EAGLE_ATM_PRT_DLL			_T("posdll.dll")
#define EAGLECE_ATM_HOST_DLL		_T("EagleCE_Host.dll")
#define EAGLE_ATM_MUB_DLL			_T("MFS_MUB_WEC7.dll")
#define EAGLE_ATM_EMV_KERNEL_DLL	_T("EMVKernel_API.dll")

//
// 장치 TYPE
//
// Receipt Printer
#define PRT_T080		("0")
#define PRT_TP27		("1")
// EPP
#define EPP_MFS			("0")
#define EPP_CRYPTERA	("1")
// CDM
#define CDM_STD			("0")
#define CDM_WDM			("1")
