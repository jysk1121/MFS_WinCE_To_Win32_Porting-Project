#pragma once

// Determines the type of caching performed
typedef int CacheMode;
const CacheMode CACHEMODE_DEFAULT	= 0;
const CacheMode CACHEMODE_NONE		= 1;

typedef struct _cache_object {
	CString Url;
	CString Method;
	CString ETag;

	unsigned long long Expiration;

	CString CacheKey;
} CacheObject;

/**
 * Manages the caching for the HTTP service
 */
class CHTTPCache
{
public:
	CHTTPCache(CacheMode mode, CString cacheName);
	~CHTTPCache(void);

	/**
	 * Stores the response in the system cache
	 * @param url [in] the URL of the request
	 * @param method [in] the HTTP method used for the request
	 * @param etag [in] the etag from the response
	 * @param data [in] the data from the response
	 * @param size [in] the size of the response buffer
	 */
	void CacheResponse(CString url, CString method, CString etag, byte* data, unsigned __int64 size);

	/**
	 * Determines whether data from the cache can be used for the response
	 * @param[in] url the URL of the request
	 * @param[in] method the HTTP method used for the request
	 * @param[out] etag the etag from the response
	 * @param[out] key the cache key, if the cache is found
	 * @returns true if the cached data is valid
	 */
	bool IsCached(CString url, CString method, CString &etag, CString &key);

	/**
	 * Gets the size of the cache item for the key
	 * @param cacheKey [in] the cache item's key
	 * @returns the size of the item
	 */
	unsigned int GetCacheItemSize(CString cacheKey);

	/**
	 * Reads the cache item into the buffer
	 * @param cacheKey [in] the key for the cache item
	 * @param buffer [in] the buffer into which to read the item's data
	 * @param size [in] the size of the buffer
	 * @returns the number of bytes read
	 */
	int ReadCacheItem(CString cacheKey, byte *buffer, int size);
	
private:

	// The mode of operation for the cache
	CacheMode cacheMode;

	// The name of the cache used to segment cache data
	CString cacheName;

	// The map of cache data
	CMap<CString, LPCTSTR, CacheObject, CacheObject&> cacheMap;

private:
	CString GetFileNameForCacheKey(CString cacheKey);
	CString GetCacheIndexPath();

	/**
	 * Writes the cache index to the disk
	 * @returns the number of bytes written
	 */
	int BackupCacheIndex();

	/**
	 * Reads the index from the disk into memory
	 * @returns the number of bytes read
	 */
	int RestoreCacheIndex();

	/**
	 * Deletes the cache entry if the cache item is invalid or expired
	 * @param[in] cacheKey the cache item name
	 * @param[in] expiration the cache item's expiration time
	 * @returns true if the cache item is invalid (and was deleted)
	 */
	bool DeleteExpiredCacheEntry(CString cacheKey, unsigned long long expiration);

	static CString GetCacheIndexPath(CString cacheName);
	static CString SanitizeCacheName(CString cacheName);
	static unsigned long long GetUnixTimestamp();
	static CString GenerateCacheKey();
	static CString CreateCacheKeyFromInput(CString cacheName, CString url, CString method);
};
