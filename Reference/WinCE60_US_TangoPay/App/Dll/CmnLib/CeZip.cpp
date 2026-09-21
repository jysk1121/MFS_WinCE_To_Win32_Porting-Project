#include "stdafx.h"
#include ".\Common\CmnLib.h"

#include "zip.h"

// ZIP
HZIP	WINAPI	zCreateZip(void *buf,unsigned int len, const char *password)
{
	return CreateZip(buf, len, password);
}
HZIP	WINAPI	zCreateZip(const TCHAR *fn, const char *password)
{
	return CreateZip(fn, password);
}
ZRESULT WINAPI	zZipAdd(HZIP hz,const TCHAR *dstzn, const TCHAR *fn)
{
	return ZipAdd(hz, dstzn, fn);
}
ZRESULT WINAPI	zZipAdd(HZIP hz,const TCHAR *dstzn, void *src,unsigned int len)
{
	return ZipAdd(hz, dstzn, src, len);
}
ZRESULT WINAPI	zZipAddFolder(HZIP hz,const TCHAR *dstzn)
{
	return ZipAddFolder(hz, dstzn);
}
ZRESULT WINAPI	zZipGetMemory(HZIP hz, void **buf, unsigned long *len)
{
	return ZipGetMemory(hz, buf, len);
}
ZRESULT WINAPI	zCloseZip(HZIP hz)
{
	return CloseZip(hz);
}
