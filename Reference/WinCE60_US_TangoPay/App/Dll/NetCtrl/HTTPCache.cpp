#include "stdafx.h"
#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"
#include ".\Net\HTTPCache.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#ifdef UNDER_CE
#  define CACHE_BASE_DIR L"\\ATM2\\_cache"
#  define CACHE_INDEX_EXTENSION L"cache-index"
#  define CACHE_KEY_LEN 32
#  define DEFAULT_CACHE_EXPIRATION_DAYS 7
#else
#  define CACHE_BASE_DIR L".\\_cache"
#  define CACHE_INDEX_EXTENSION L"cache-index"
#  define CACHE_KEY_LEN 32
#  define DEFAULT_CACHE_EXPIRATION_DAYS 1
#endif

CHTTPCache::CHTTPCache(CacheMode mode, CString cacheName)
{
	this->cacheMode = mode;
	this->cacheName = cacheName;

	RestoreCacheIndex();
}

CHTTPCache::~CHTTPCache(void)
{
	BackupCacheIndex();
}


/**
 * Stores the response in the system cache
 * @param url [in] the URL of the request
 * @param method [in] the HTTP method used for the request
 * @param etag [in] the etag from the response
 * @param data [in] the data from the response
 * @param size [in] the size of the response buffer
 */
void CHTTPCache::CacheResponse(CString url, CString method, CString etag, byte* data, unsigned __int64 size)
{
	CString cacheKey = CreateCacheKeyFromInput(cacheName, url, method);

	CacheObject item;
	item.CacheKey = cacheKey;
	item.ETag = etag;
	item.Method = method;
	item.Url = url;
	item.Expiration = GetUnixTimestamp() + (DEFAULT_CACHE_EXPIRATION_DAYS * 24 * 60 * 60 * 1000);

	CString filename = GetFileNameForCacheKey(cacheKey);
	HANDLE file = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD written = 0;
	WriteFile(file, data, (DWORD)size, &written, NULL);
	CloseHandle(file);
	if (written <= 0)
	{
		NHERROR((L"File write failed with %d\r\n", GetLastError()));
		return;
	}
	
	cacheMap.SetAt(cacheKey, item);
	BackupCacheIndex();
}

/**
 * Determines whether data from the cache can be used for the response
 * @param[in] url the URL of the request
 * @param[in] method the HTTP method used for the request
 * @param[out] etag the etag from the response
 * @param[out] key the cache key, if the cache is found
 * @returns true if the cached data is valid
 */
bool CHTTPCache::IsCached(CString url, CString method, CString &etag, CString &key)
{
	CString cacheKey = CreateCacheKeyFromInput(cacheName, url, method);

	CacheObject item;
	if (cacheMap.Lookup(cacheKey, item))
	{
		key = item.CacheKey;
		etag = item.ETag;

		NHDBG((L"%s was cached\r\n", url));
		return true;
	}
	return false;
}

/**
 * Gets the size of the cache item for the key
 * @param cacheKey [in] the cache item's key
 * @returns the size of the item
 */
unsigned int CHTTPCache::GetCacheItemSize(CString cacheKey)
{
	CString filename = GetFileNameForCacheKey(cacheKey);
	HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD size = GetFileSize(file, NULL);
	CloseHandle(file);
	return size;
}

/**
 * Reads the cache item into the buffer
 * @param cacheKey [in] the key for the cache item
 * @param buffer [in] the buffer into which to read the item's data
 * @returns the number of bytes read
 */
int CHTTPCache::ReadCacheItem(CString cacheKey, byte *buffer, int size)
{
	CString filename = GetFileNameForCacheKey(cacheKey);
	HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD read = 0;
	ReadFile(file, buffer, size, &read, NULL);
	CloseHandle(file);

	return read;
}

//
// Private Methods
//

CString CHTTPCache::GetFileNameForCacheKey(CString cacheKey)
{
	CString cacheFilePath;
	cacheFilePath.Format(L"%s\\%s-%s.cache", CACHE_BASE_DIR, SanitizeCacheName(this->cacheName), cacheKey);
	return cacheFilePath;
}

/**
 * Writes the cache index to the disk
 * @returns the number of bytes written
 */
int CHTTPCache::BackupCacheIndex()
{
	try
	{
		CString indexFilePath = CHTTPCache::GetCacheIndexPath(cacheName);
		CStdioFile indexFile(indexFilePath, CFile::modeWrite | CFile::typeText | CFile::modeCreate);
		POSITION pos = cacheMap.GetStartPosition();
		CacheObject item;
		CString cacheKey;

		CString line;
		while (pos != NULL)
		{
			cacheMap.GetNextAssoc(pos, cacheKey, item);
			line.Format(L"%s,%s,%s,%s,%llu\n", cacheKey, item.Url, item.Method, item.ETag, item.Expiration);
			indexFile.WriteString(line);			
		}

		indexFile.Close();
	}
	catch (CFileException* ex)
	{
		NHERROR((L"Could not open file %s: %d\r\n", ex->m_strFileName, ex->m_cause));
		return 0;
	}

	return 0;
}

/**
 * Reads the index from the disk into memory
 * @returns the number of bytes read
 */
int CHTTPCache::RestoreCacheIndex()
{
	CString indexFilePath = CHTTPCache::GetCacheIndexPath(cacheName);
	if (!CreateDirectory(CACHE_BASE_DIR, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		return  -1;
	}

	try
	{
		CStdioFile indexFile(indexFilePath, CFile::modeRead | CFile::typeText | CFile::modeCreate | CFile::modeNoTruncate);

		CString line;
		int pos = 0;
		CacheObject item;
		while(indexFile.ReadString(line))
		{
			if (line.IsEmpty())
			{
				continue;
			}

			item.CacheKey = line.Tokenize(L",", pos);
			item.Url = line.Tokenize(L",", pos);
			item.Method = line.Tokenize(L",", pos);
			item.ETag = line.Tokenize(L",", pos);
			item.Expiration = (unsigned long long) Asc2LongLong(line.Tokenize(L",", pos));

			pos = 0;

			// Only add the cache item to the in-memory cache if it is valid.
			if (!DeleteExpiredCacheEntry(item.CacheKey, item.Expiration))
			{
				cacheMap.SetAt(item.CacheKey, item);
			}
			else
			{
				NHINFO((L"Deleted cache item %s\r\n", item.CacheKey));
			}

			line = L"";
		}

		indexFile.Close();
	}
	catch (CFileException* ex)
	{
		NHERROR((L"Could not open file %s: %d\r\n", ex->m_strFileName, ex->m_cause));
		return 0;
	}
	catch (COleException* ex)
	{
		NHERROR((L"Error while reading file %s: %d\r\n", indexFilePath, ex->m_sc));
		NHINFO((L"Deleting %s due to invalid content\r\n", indexFilePath));
		DeleteFile(indexFilePath);
		return 0;
	}

	return 1;
}

/**
 * Deletes the cache entry if the cache item is invalid or expired
 * @param[in] cacheKey the cache item name
 * @param[in] expiration the cache item's expiration time
 * @returns true if the cache item is invalid (and was deleted)
 */
bool CHTTPCache::DeleteExpiredCacheEntry(CString cacheKey, unsigned long long expiration)
{
	unsigned long long currTime = GetUnixTimestamp();
	if (((long long)(currTime - expiration)) >= 0)
	{
		CString path = GetFileNameForCacheKey(cacheKey);
		DeleteFile(path);
		return true;
	}

	return false;
}

CString CHTTPCache::GetCacheIndexPath(CString cacheName)
{
	CString cacheFilePath;
	cacheFilePath.Format(L"%s\\%s.%s", CACHE_BASE_DIR, SanitizeCacheName(cacheName), CACHE_INDEX_EXTENSION);
	return cacheFilePath;
}

CString CHTTPCache::SanitizeCacheName(CString cacheName)
{
	cacheName.Replace(L"\\", L"_");
	cacheName.Replace(L"/", L"_");
	cacheName.Replace(L".", L"-");

	return cacheName;
}

unsigned long long CHTTPCache::GetUnixTimestamp()
{
	const __int64 UNIX_TIME_START = 116444736000000000; //January 1, 1970 (start of Unix epoch) in "ticks" // A Windows tick is 100 nanoseconds. Windows epoch 1601-01-01T00:00:00Z, 11644473600 seconds before Unix epoch 1970-01-01T00:00:00Z.
	const __int64 TICKS_PER_MILLISECOND = 10000; //a tick is 100ns

	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);

	ULARGE_INTEGER li;
	li.LowPart  = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

    return (li.QuadPart - UNIX_TIME_START) / TICKS_PER_MILLISECOND;	//Convert ticks since 1/1/1970 into seconds
}

CString CHTTPCache::GenerateCacheKey()
{
	int generated = 0;
	char output[CACHE_KEY_LEN] = {};
	unsigned char rand[CACHE_KEY_LEN] = {};

	while (generated < CACHE_KEY_LEN)
	{
		if (RAND_bytes((unsigned char *)rand, CACHE_KEY_LEN) <= 0)
		{
			continue;
		}

		for (int i = 0; i < CACHE_KEY_LEN; i++)
		{
			char data = (char) rand[i];

			if (data >= '0' && data <= '9' || data >= 'a' && data <= 'z' || data >= 'A' && data <= 'Z')
			{
				output[generated++] = data;
			}

			if (generated >= CACHE_KEY_LEN)
			{
				break;
			}
		}
	}

	return CString(output);
}

CString CHTTPCache::CreateCacheKeyFromInput(CString cacheName, CString url, CString method)
{
	CString digestInput;
	digestInput.Format(L"%s:%s:%s", cacheName, url, method);

	int inputLen = digestInput.GetLength() + 1;
	unsigned char * input = new unsigned char[inputLen]();
	WideToMulti((char *)input, digestInput, inputLen);

	unsigned int digestLen = 0u;
	unsigned char *output = new unsigned char[16]();

	EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
	const EVP_MD *md = EVP_md5();
	EVP_DigestInit_ex(mdctx, md, NULL);
	EVP_DigestUpdate(mdctx, input, inputLen);
	delete [] input;
	input = NULL;

	EVP_DigestFinal(mdctx, output, &digestLen);
	EVP_MD_CTX_destroy(mdctx);

	int hexlen = digestLen * 2 + 1;
	char *hexdigest = new char[hexlen]();
	for (unsigned int i = 0; i < digestLen; i++)
	{
		sprintf_s(hexdigest + i, hexlen - i, "%02x", output[i]);
	}

	CString hexOutput(hexdigest);
	delete [] hexdigest;
	delete [] output;
	return hexOutput;
}