
// ConfigEncryptConverterDlg.h : 헤더 파일
//

#pragma once

#include "DES.h"

#define		ADS_CFG_PATH_NAME						_T("ADS.CFG")
#define		BINLIST_CFG_PATH_NAME					_T("BINLIST.CFG")
#define		CBXINFO_CFG_PATH_NAME					_T("CBXINFO.CFG")
#define		DEVICE_CFG_PATH_NAME					_T("DEVICE.CFG")
#define		DISPENSE_CFG_PATH_NAME					_T("DISPENSE.CFG")
#define		EAGLE_ATM_EMV_CONFIG_PATH_NAME			_T("EMV_CONFIG.CFG")
#define		EAGLE_ATM_EMV_US_COMM_AID_PATH_NAME		_T("EMV_US_COMM_AID.CFG")
#define		HOST_CFG_PATH_NAME						_T("HOST.CFG")
#define		LASTTRANSINFO_CFG_PATH_NAME				_T("LASTTRANSINFO.CFG")
#define		MESSAGE_CFG_PATH_NAME					_T("MESSAGE.CFG")
#define		OPTION_CFG_PATH_NAME					_T("OPTION.CFG")
#define		SYSTEM_CFG_PATH_NAME					_T("SYSTEM.CFG")
#define		TOTAL_CFG_PATH_NAME						_T("TOTALS.CFG")
#define		BACKUP_CONFIGFILE_NAME					_T("BACKUPCONFIG.DAT")

#define		DATA_PADDING_SIZE		8
#define		CONFIG_KEY_32		_T("7582B3484971AE4B57291D30485F9423")


// CConfigEncryptConverterDlg 대화 상자
class CConfigEncryptConverterDlg : public CDialog
{
// 생성입니다.
public:
	CConfigEncryptConverterDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_CONFIGENCRYPTCONVERTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

	CString	MergeConfigData(CString strConfigDirectory, CString strFileName);
	BOOL	SeparateConfigData(CString strConfigDirectory, CString strFileName, CString strAllConfigData);

	BYTE	MakePack(BYTE bySrc);
	void	ConvertStringToHex(CString strSrc, BYTE *pByDest);
	void	EncryptKeyData(BYTE *pbyInData, int nInDataSize, BYTE *pbyOutData, BYTE *pbyKey);
	void	DecryptKeyData(BYTE *pbyInData, int nInDataSize, BYTE *pbyOutData, BYTE *pbyKey);
	unsigned int crc16(unsigned char *string, int length);


	CString m_strConfigFileDir;
	CString m_strEncryptFileDir;


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnConfigFileDir();
	afx_msg void OnBnClickedBtnEncryptFileDir();
	afx_msg void OnBnClickedBtnEncryp();
	afx_msg void OnBnClickedBtnDecrypt();
};
