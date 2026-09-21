#include "StdAfx.h"
#include "RepoLocalCache.h"

CRepoLocalCache::CRepoLocalCache(void)
{
	initialized = false;
}

CRepoLocalCache::~CRepoLocalCache(void)
{
	ClearCache();
}

void CRepoLocalCache::ClearCache()
{
	initialized = false;
	
	for (int i = 0; i < Manifests.GetCount(); i++)
	{
		POSITION pos = Manifests.FindIndex(i);
		Manifest* m = Manifests.GetAt(pos);
		Manifests.RemoveAt(pos);
		delete m;
		m = NULL;
	}

	Manifests.RemoveAll();
	Packages.RemoveAll();
}