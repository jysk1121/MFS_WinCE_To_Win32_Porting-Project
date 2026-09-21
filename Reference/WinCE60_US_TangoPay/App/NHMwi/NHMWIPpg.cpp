// NHMWIPpg.cpp : Implementation of the CNHMWIPropPage property page class.

#include "stdafx.h"
#include "NHMWI.h"
#include "NHMWIPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CNHMWIPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CNHMWIPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CNHMWIPropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CNHMWIPropPage, "NHMWI.NHMWIPropPage.1",
	0x886901f2, 0xf332, 0x42f5, 0x8a, 0xb6, 0xe8, 0x3b, 0x95, 0xb3, 0x6e, 0x98)


/////////////////////////////////////////////////////////////////////////////
// CNHMWIPropPage::CNHMWIPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CNHMWIPropPage

BOOL CNHMWIPropPage::CNHMWIPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_NHMWI_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CNHMWIPropPage::CNHMWIPropPage - Constructor

CNHMWIPropPage::CNHMWIPropPage() :
	COlePropertyPage(IDD, IDS_NHMWI_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CNHMWIPropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CNHMWIPropPage::DoDataExchange - Moves data between page and properties

void CNHMWIPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CNHMWIPropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CNHMWIPropPage message handlers
