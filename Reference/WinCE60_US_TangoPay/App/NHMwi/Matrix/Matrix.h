#if !defined(AFX_MATRIX_H__NAUTILUS_HYOSUNG__INCLUDED_)
#define AFX_MATRIX_H__NAUTILUS_HYOSUNG__INCLUDED_

// matrix.h : header file
//

#include ".\Common\StructDef.h"
/////////////////////////////////////////////////////////////////////////////
// CMatrix Class

class CAtmUtility;

class CMatrix
{
// Construction
public:
	CMatrix();

// Attributes
public:
	CAtmUtility*	m_pUtil;									// Utility Pointer

private:
	int		m_nDeviceID;										// 장치 고유ID
	CString	m_strDeviceName;									// 장치 고유명

	int		m_nMethodCount;										// 장치에서 사용가능한 메소드갯수(Kal Matrix기준)
	int		m_nEventCount;										// 장치에서 발생가능한 이벤트갯수(Kal Matrix기준)
	int		m_nCurrMethod;										// 현재 수행하는 메소드의 인덱스
	
	ITEMLIST	m_MethodList;
	ITEMLIST	m_EventList;

	ITEMLIST**	m_ppMatrix;										// 기준메트릭스의 저장포인터

	int**	m_nppMasterMatrix;									// 기준이 되는 메소드/이벤트 메트릭스
	int**	m_nppEventMatrix;									// 동작Event 메트릭스


// Operations
public:
	void	SetDeviceInfo(int nDevice, char* szName);			// 디바이스의 ID와 이름을 설정
	int		GetDeviceID();										// 디바이스 ID조회
	CString	GetDeviceName();									// 디바이스 이름 조회

	void	SetMethodCount(int nCount);							// 메소드 갯수를 설정
	void	SetEventCount(int nCount);							// 이벤트 갯수를 설정
	int		GetMethodCount();									// 메소드 갯수를 조회
	int		GetEventCount();									// 이벤트 갯수를 조회

	int		GetCurrMethodIndex();								// 현재수행중인 메소드의 인덱스를 조회

	BOOL	SetMasterMatrix(ITEMLIST* ppList);					// MasterMatrix 설정

	BOOL	ResetMethodEventMatrix(int nMethodIndex);			// 이벤트메트릭스를 초기화한다.
	BOOL	SetMethodEventMatrix(int nMethodIndex);				// 메소드호출시 이벤트 메트릭스를 설정
	BOOL	SetEventMatrixValue(int nEventIndex);
	int		GetEventIndex(int nMethodIndex);

	int		GetMasterMatrixValue(int nMethodIndex, int nEventIndex);	// 장치 MasterMatrix(m_nppMasterMatrix)의 값조회
	int		GetEventMatrixValue(int nMethodIndex, int nEventIndex);		// 장치 EventMatrix(m_nppEventMatrix)의 값조회

// Implementation
public:
	~CMatrix();

};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_MATRIX_H__NAUTILUS_HYOSUNG__INCLUDED_)
