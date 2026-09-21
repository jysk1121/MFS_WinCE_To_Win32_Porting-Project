#pragma once
#include "UpdateTypes.h"

class AFX_CLASS_EXPORT CRepoLocalCache
{
public:
	CRepoLocalCache(void);
	~CRepoLocalCache(void);

	/**
	 * Clears the cached data
	 */
	void ClearCache();

public:
	CList<Manifest*> Manifests;

	PackageList Packages;

	// Is the cache primed?
	bool initialized;
};
