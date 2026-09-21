#include "stdafx.h"
#include "ScreenViewer.h"
#include "ScreenViewerDlg.h"

void CScreenViewerDlg::OnBtnLoadScreen_Trx() 
{
	// TODO: Add your control notification handler code here
	UnloadScreen(SCR_TRX);
	LoadScreen(SCR_TRX);
	(GetDlgItem(IDC_BTN_LOAD))->SetWindowText(_T("Reload"));
}

void CScreenViewerDlg::OnBtnPrevScr_Trx() 
{
	// TODO: Add your control notification handler code here
	ShowPrevScreen(SCR_TRX);
}

void CScreenViewerDlg::OnBtnNextScr_Trx() 
{
	// TODO: Add your control notification handler code here
	ShowNextScreen(SCR_TRX);
}

void CScreenViewerDlg::OnBnClickedCheckTrx640480()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_TRX, RES_640_480);
}

void CScreenViewerDlg::OnBnClickedCheckTrx1024768()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_TRX, RES_1024_768);
}

void CScreenViewerDlg::OnBnClickedCheckTrx1024768_W()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_TRX, RES_1024_768_W);
}

void CScreenViewerDlg::OnBnClickedCheckTrx800600()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_TRX, RES_800_600);
}

void CScreenViewerDlg::OnBnClickedCheckTrx1024600()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_TRX, RES_1024_600);
}

void CScreenViewerDlg::OnBnClickedButtonLanguageTrx()
{
	// TODO: Add your control notification handler code here
	m_nLanguage[SCR_TRX]++;
	if (m_nLanguage[SCR_TRX] >= 5)
		m_nLanguage[SCR_TRX] = 0;

	m_btnLang_Trx.SetWindowText(GET_LANG_STRING(m_nLanguage[SCR_TRX]));
	ApplyScreenValues(SCR_TRX);
}