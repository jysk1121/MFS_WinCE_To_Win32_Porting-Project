#include "stdafx.h"
#include "ScreenViewer.h"
#include "ScreenViewerDlg.h"

//
// EVENT HANDLERS
//
void CScreenViewerDlg::OnBnClickedBtnLoadOp()
{
	// TODO: Add your control notification handler code here
	(GetDlgItem(IDC_BTN_LOAD_OP))->SetWindowText(_T("Loading..."));
	(GetDlgItem(IDC_BTN_LOAD_OP))->EnableWindow(FALSE);

	if (m_bScreenLoaded[SCR_OP])
		UnloadScreen(SCR_OP);

	LoadScreen(SCR_OP);
	(GetDlgItem(IDC_BTN_LOAD_OP))->SetWindowText(_T("Reload"));
	(GetDlgItem(IDC_BTN_LOAD_OP))->EnableWindow(TRUE);
}

void CScreenViewerDlg::OnCbnSelchangeComboScrnumOp()
{
	// TODO: Add your control notification handler code here
	LoadPresetValues(SCR_OP);
	ApplyScreenValues(SCR_OP);
}

void CScreenViewerDlg::OnBnClickedCheckOp1024600()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_OP, RES_1024_600);
}

void CScreenViewerDlg::OnBnClickedCheckOp800600()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_OP, RES_800_600);
}

void CScreenViewerDlg::OnBnClickedCheckOp1024768_W()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_OP, RES_1024_768_W);
}

void CScreenViewerDlg::OnBnClickedCheckOp1024768()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_OP, RES_1024_768);
}

void CScreenViewerDlg::OnBnClickedCheckOp720480()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_OP, RES_720_480);
}

void CScreenViewerDlg::OnBnClickedCheckOp640480()
{
	// TODO: Add your control notification handler code here
	UpdateScreenVisibility(SCR_OP, RES_640_480);
}

void CScreenViewerDlg::OnBnClickedButtonLanguageOp()
{
	// TODO: Add your control notification handler code here
	if (m_nLanguage[SCR_OP] == 0)
		m_nLanguage[SCR_OP] = 2;
	else
		m_nLanguage[SCR_OP] = 0;

	m_btnLang_Op.SetWindowText(GET_LANG_STRING(m_nLanguage[SCR_OP]));
	ApplyScreenValues(SCR_OP);
}

void CScreenViewerDlg::OnBnClickedBtnPrevOp()
{
	// TODO: Add your control notification handler code here
	ShowPrevScreen(SCR_OP);
}

void CScreenViewerDlg::OnBnClickedBtnNextOp()
{
	// TODO: Add your control notification handler code here
	ShowNextScreen(SCR_OP);
}
