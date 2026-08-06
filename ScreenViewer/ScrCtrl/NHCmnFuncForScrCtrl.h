#ifndef __NH_CMN_FUNC_FOR_SCRCTRL_H__
#define __NH_CMN_FUNC_FOR_SCRCTRL_H__

void	NH_WideToMulti(LPSTR multi, LPCWSTR wide, int size);

int		NH_Asc2Int(LPCTSTR lpvoid);
int		NH_Asc2Int(LPCTSTR lpvoid, int length);
int		NH_Asc2Int(void* lpvoid, int length);

CString NH_GetCurrencySymbol(int nLength=0, int nBracket=0);

int		NH_SplitString(CString cstrSrc, CString cstrSplit, CStringArray& acstrRet);

#endif // __NH_CMN_FUNC_FOR_SCRCTRL_H__