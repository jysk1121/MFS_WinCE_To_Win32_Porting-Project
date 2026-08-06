#include "Stdafx.h"
#include "NHCmnFuncForScrCtrl.h"

void	NH_WideToMulti(LPSTR multi, LPCWSTR wide, int size)
{
    WideCharToMultiByte(CP_ACP, 0, wide, -1, multi, size, NULL, NULL);
}

int		NH_Asc2Int(LPCTSTR lpvoid)
{
	char szTemp[128]; memset(szTemp, NULL, sizeof(szTemp));

#if defined(UNDER_CE) || defined(UNICODE)
	NH_WideToMulti(szTemp, lpvoid, sizeof(szTemp));
#else
	strcpy(szTemp, lpvoid);
#endif

	return (NH_Asc2Int((void*)szTemp, strlen(szTemp)));
}

int		NH_Asc2Int(LPCTSTR lpvoid, int length)
{
	char szTemp[128]; memset(szTemp, NULL, sizeof(szTemp));

#if defined(UNDER_CE) || defined(UNICODE)
	NH_WideToMulti(szTemp, lpvoid, sizeof(szTemp));
#else
	strcpy(szTemp, lpvoid);
#endif
		
	return (NH_Asc2Int((void*)szTemp, strlen(szTemp)));
}

int		NH_Asc2Int(void* lpvoid, int length)
{
    int		ivalue=0,bminus=FALSE;
    int		i;
	char*	asc = (char* )lpvoid;

	if (!length)
		return 0;

    for(i=0;i<length;i++)
    {
        if(asc[i]>='0' && asc[i]<='9')
        {
            ivalue = (ivalue*10) + (asc[i]-'0');
            continue;
        }
        if(asc[i]=='-') bminus = TRUE;
    }

    return (bminus) ? 0-ivalue : ivalue;
}


CString NH_GetCurrencySymbol(int nLength, int nBracket)
{
	CString	strSymbol;

	strSymbol = _T("$");

	if (nBracket == 1)
	{
		strSymbol = _T("(") + strSymbol + _T(")");
	}

	if ( nLength > strSymbol.GetLength() )
	{
		strSymbol = strSymbol + CString(' ', nLength - strSymbol.GetLength());
	}

	return strSymbol;
}


int	NH_SplitString(CString cstrSrc, CString cstrSplit, CStringArray& acstrRet)
{
	CString cstrTemp = cstrSrc;
	CString cstrData("");
	int		nIndex=0, nCnt=0;

	acstrRet.RemoveAll();										// 20030703 : 초기화하지 않으면 원래배열의 뒤에 추가됨
	
	while (TRUE)												// 20031030 : 무조건처리로변경
	{
		nIndex = cstrTemp.Find(cstrSplit);
		if(nIndex == -1)
		{
			acstrRet.Add(cstrTemp);								// 20031030 : 무조건처리로변경
			nCnt++;												// 20031030 : 마지막처리추가
// [CS#1] NH AIREAT 2008.3.14
			//return nCnt;
			break;
// end of [CS#1]
		}
		else
		{
			cstrData = cstrTemp.Left(nIndex);
			cstrTemp = cstrTemp.Right(cstrTemp.GetLength() - (nIndex + cstrSplit.GetLength()));  
			acstrRet.Add(cstrData);
			nCnt++;
		}
	}
	return nCnt;
}