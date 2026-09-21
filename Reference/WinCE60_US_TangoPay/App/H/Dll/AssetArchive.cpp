#include "stdafx.h"

//#define NH_DEBUG
#include "..\..\H\Common\NHDbgApi.h"

#include "AssetArchive.h"

// The amount of time to wait for a read context synchronization
#define READ_WAIT_MILLIS 500

#define WideToMulti(multi, wide, size)	WideCharToMultiByte(CP_ACP, 0, wide, -1, multi, size, NULL, NULL)

CAssetArchive* CAssetArchive::_instance = NULL;

CAssetArchive::CAssetArchive(void)
{
	NHDBG((_T("CAssetArchive()\n")));

	this->m_readMutex = CreateMutex(NULL, FALSE, L"CAssetArchive");
	this->m_tarball = NULL;
	cachePrePopulated = false;
}

CAssetArchive::~CAssetArchive(void)
{
	NHDBG((_T("~CAssetArchive()\n")));

	CloseHandle(m_readMutex);
	CloseArchive();
	_instance = NULL;
}

/**
 * Parses the entire archive and pre-populates the index cache
 * @returns the number of files within the archive
 */
int CAssetArchive::PrePopulateCache()
{
	int numFiles = 0;
	mtar_header_t h;
	CString currentFilePath;

	CHECK_TIME_START;

	// Can't populate something that hasn't been loaded
	if (m_tarball == NULL)
	{
		return -1;
	}

	// Populated already
	if (cachePrePopulated)
	{
		return indexMap.GetCount();
	}

	// Go to beginning of the tarball, in the event the pointer has been moved
	mtar_seek(m_tarball, 0);

	while (mtar_read_header(m_tarball, &h) != MTAR_ENULLRECORD) 
	{
		currentFilePath.Format(L"%S", h.name);
		ConvertPathToUnix(currentFilePath);

		indexMap.SetAt(currentFilePath, m_tarball->pos);
		mtar_next(m_tarball);

		numFiles++;
	}

	CHECK_TIME_END;

	cachePrePopulated = true;

	NHDBG((_T("Cached: %d files\n"), numFiles));
	return numFiles;
}

/**
 * Opens a handle to the archive
 * @params archivePath [in] the file path to the archive to read
 * @returns true if the operation is successful
 */
bool CAssetArchive::OpenArchive(CString archivePath)
{
	int res = MTAR_EOPENFAIL;
	CHECK_TIME_START;

	// Don't allow multiple opens
	if (m_tarball != NULL)
	{
		NHDBG((_T("Already opened!\n")));
		return false;
	}

	if (GetFileAttributes(archivePath) == -1)
	{
		NHDBG((_T("TAR not found! [%s]\n"), archivePath));
		return false;
	}

	this->m_tarball = (mtar_t*) malloc(sizeof(mtar_t));

	// Convert filepath to a unix-style path
	ConvertPathToUnix(archivePath);

	// Get base path of archive file to set the base path of the archive
	int lastPathSepLocation = archivePath.ReverseFind('/');
	this->basePath = archivePath.Mid(0, lastPathSepLocation + 1);

	int filenameLen = archivePath.GetLength() + 1;
	char * filename = new char[filenameLen]();
	WideToMulti(filename, archivePath, filenameLen);

	// Open archive for reading
	res = mtar_open(m_tarball, filename, "r");

	delete [] filename;
	CHECK_TIME_END;

	if (res == MTAR_ESUCCESS)
	{
		NHDBG((_T("TAR Opened!\n")));
	}
	else
	{
		NHDBG((_T("Failed to open TAR!\n")));

		free(this->m_tarball);
		this->m_tarball = NULL;
	}

	return res == MTAR_ESUCCESS;
}

/**
 * Closes the archive
 * @returns true always, currently.
 */
bool CAssetArchive::CloseArchive()
{
	if (m_tarball != NULL) 
	{
		mtar_close(m_tarball);
		free(m_tarball);
		m_tarball = NULL;

		indexMap.RemoveAll();
		cachePrePopulated = false;

		NHDBG((_T("TAR Closed!\n")));
	}

	return true;
}

/**
 * Reads the data from the archive for the file path provided.
 * @params filePath [in] the file patch within the archive
 * @params stream [out] the stream into which the data will be provided
 * @returns the handle result
 */
HRESULT CAssetArchive::ReadFile(CString filePath, IStream **stream)
{
	mtar_header_t h;
	BYTE* pbLocked = NULL;
	HRESULT hrRet = -1;
    HGLOBAL hg = NULL;

	if (WaitForSingleObject(m_readMutex, READ_WAIT_MILLIS) != WAIT_OBJECT_0)
	{
		// Synchronization failure
		goto error;
	}

	if (!GetArchiveHeader(filePath, h))
	{
		goto error;
	}

	 // Open a memory object
    hg = GlobalAlloc(GMEM_MOVEABLE, h.size + 1);
    if (NULL == hg)
    {
        hrRet = 0x80000000 + GetLastError();
        goto error;
    }

    // Ge a pointer to the memory we just allocated
    pbLocked = (BYTE*) GlobalLock(hg);
    if (NULL == pbLocked)
    {
        hrRet = 0x80000000 + GetLastError();
        goto error;
    }

	mtar_read_data(m_tarball, pbLocked, h.size);
	ReleaseMutex(m_readMutex);

	GlobalUnlock(hg);
    
    // Create the stream
    hrRet = CreateStreamOnHGlobal(hg, TRUE, stream);

    return hrRet;

error:
	ReleaseMutex(m_readMutex);
    if (pbLocked) GlobalUnlock(hg);
    if (hg) GlobalFree(hg);
    return hrRet;
}

/**
* Reads the data from the archive for the file path provided.
* @params filePath [in] the file patch within the archive
* @params pByte [out] the byte array into which the data will be provided
* @params pSize [out] the integer into which the data length will be provided
* @returns the handle result
*/
HRESULT CAssetArchive::ReadFile(CString filePath, BYTE *pBuffer)
{
	mtar_header_t h;
	HRESULT hrRet = -1;

	CHECK_TIME_START;

	if (WaitForSingleObject(m_readMutex, READ_WAIT_MILLIS) != WAIT_OBJECT_0)
	{
		// Synchronization failure
		goto error;
	}

	if (!GetArchiveHeader(filePath, h))
	{
		goto error;
	}
	
	if (mtar_read_data(m_tarball, pBuffer, h.size) != MTAR_ESUCCESS)
	{
		goto error;
	}

	hrRet = S_OK;

error:
	CHECK_TIME_END;

	ReleaseMutex(m_readMutex);
	return hrRet;
}

/**
 * Gets the file size
 * @returns the file size or -1 if the file was not found in the archive
 */
unsigned int CAssetArchive::GetFileSize(CString filePath)
{
	mtar_header_t h;

	CHECK_TIME_START;

	if (WaitForSingleObject(m_readMutex, READ_WAIT_MILLIS) != WAIT_OBJECT_0)
	{
		// Synchronization failure
		return 0;
	}

	if (!GetArchiveHeader(filePath, h))
	{
		// Failed to get file data
		ReleaseMutex(m_readMutex);
		return 0;
	}

	CHECK_TIME_END;

 	NHDBG((L"%s (%d bytes)\n", filePath, h.size));
	ReleaseMutex(m_readMutex);
	return h.size;
}

/**
 *	Converts the string to a unix-style path for reference in the TAR archive
 */
void CAssetArchive::ConvertPathToUnix(CString &path)
{
	path.Replace('\\', '/');
	path.TrimLeft(L".\\");
	path.TrimLeft(L"./");
	path.MakeLower();
}

/**
 * Gets the header for the file
 * @params filePath[in] the file to read
 * @params header[in,out] a reference to a header struct
 * @returns true if the operation was successful.
 */
bool CAssetArchive::GetArchiveHeader(CString filePath, mtar_header_t &header)
{
	CHECK_TIME_START;

	// Convert filepath to a unix-style path
	ConvertPathToUnix(filePath);

	// Remove the path prefix, in case the full path of the image has been used.
	// i.e. \\ATM\\SCREEN\\800_600\\image.png will still reference \\800_600\\image.png
	//		if the tarball is named \\ATM\\SCREEN\\x.tar
	filePath.Replace(this->basePath, L"");
#ifndef UNDER_CE
	filePath.Replace(L".restructured_us/", L"");		// screen viewer
#endif

	unsigned int seekPosition = 0;
	if (indexMap.Lookup(filePath, seekPosition))
	{
		// Cache hit :)
		mtar_seek(m_tarball, seekPosition);
		mtar_read_header(m_tarball, &header);

		if (seekPosition != m_tarball->pos)
		{
			NHERROR((L"TAR position does not match requested seek value!\r\n"));
			NHERROR((L"File: %s; Seek: %d; Pos: %d\r\n", filePath, seekPosition, m_tarball->pos));
		}
	}
	else
	{
		// Cache miss :(
		// Get information about the tar-embedded file
		int filenameLen = filePath.GetLength() + 1;
		char * filename = new char[filenameLen]();
		WideToMulti(filename, filePath, filenameLen);

		int res = mtar_find(m_tarball, filename, &header);

		delete [] filename;
		if (res != MTAR_ESUCCESS)
		{
			return false;
		}

		indexMap.SetAt(filePath, (unsigned int) m_tarball->pos);
	}

	CHECK_TIME_END;

	return true;
}