#pragma once

#include <afx.h>
#include "microtar.h"

#ifdef UNDER_CE
#  define ATM_PATH							_T("\\ATM")
#  define ATM2_PATH							_T("\\ATM2")
#  define MATM_PATH							"\\ATM"
#  define MATM2_PATH						"\\ATM2"
#else
#  define ATM_PATH							_T(".\\ATM")
#  define ATM2_PATH							_T(".\\ATM2")
#  define MATM_PATH							".\\ATM"
#  define MATM2_PATH						".\\ATM2"
#endif

#if UNDER_CE
	#define SCREEN_ASSET_TARBALL ATM_PATH L"\\Screen\\SCREEN.tar"
#else
	#if _WINDOWS		// Screen Viewer
		#define SCREEN_ASSET_TARBALL (AfxGetApp()->GetProfileString(_T("Setting"), _T("Folder")) + L".\\ATM\\SCREEN\\SCREEN.tar")
	#else				// AP on Win32
		#define SCREEN_ASSET_TARBALL ATM_PATH L"\\SCREEN\\SCREEN.tar"
	#endif
#endif

class CAssetArchive
{
public:
	CAssetArchive(void);
	~CAssetArchive(void);

	static CAssetArchive* GetInstance() {
 		if (_instance == NULL)
 			_instance = new CAssetArchive();

		if (_instance->m_tarball == NULL)
		{
			if (_instance->OpenArchive(SCREEN_ASSET_TARBALL))
			{
				_instance->PrePopulateCache();
			}
			else
			{
				// returns NULL in case of absent of SCREEN.TAR.
				delete _instance;
				_instance = NULL;
			}
		}
 
 		return _instance;
	}

	/**
	 * Closes the archive
	 * @returns true if the operation is successful
	 */
	bool CloseArchive();

	/**
	 * Reads the data from the archive for the file path provided.
	 * @params filePath [in] the file patch within the archive
	 * @params stream [out] the stream into which the data will be provided
	 * @returns the handle result
	 */
	HRESULT ReadFile(CString filePath, IStream **stream);
	HRESULT ReadFile(CString filePath, BYTE *pBuffer);

	/**
	 * Gets the file size
	 * @returns the file size or -1 if the file was not found in the archive
	 */
	unsigned int GetFileSize(CString filePath);

private:
	static CAssetArchive* _instance;

	/**
	 *  The mutex for synchronizing reads to the TAR archive. This is necessary because the graphics of the AP
	 *  are multi-threaded, and there is a non-thread-safe file pointer for the TAR.
	 */
	HANDLE m_readMutex;

	// The tarball archive
	mtar_t *m_tarball;

	// A map which caches the index of a filename in the archive
	CMap<CString, LPCTSTR, unsigned int, unsigned int> indexMap;

	// The base path of the tarball file
	CString basePath;

	bool cachePrePopulated;

	/**
	 *	Converts the string to a unix-style path for reference in the TAR archive
	 */
	static void ConvertPathToUnix(CString &path);

	/**
	 * Gets the header for the file
	 * @params filePath[in] the file to read
	 * @params header[in,out] a reference to a header struct
	 * @returns true if the operation was successful.
	 */
	bool GetArchiveHeader(CString filePath, mtar_header_t &header);

	/**
	* Parses the entire archive and pre-populates the index cache
	* @returns the number of files within the archive
	*/
	int PrePopulateCache();

	/**
	* Opens a handle to the archive
	* @params archivePath [in] the file path to the archive to read
	* @returns true if the operation is successful
	*/
	bool OpenArchive(CString archivePath);
};

