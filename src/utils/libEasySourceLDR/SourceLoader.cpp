#define _AFXDLL

#include <afxwin.h>
#include <afxdllx.h>
#include "SourceLoader.h"

#pragma comment(lib, "pthreadVC2.lib")

CSourceLoader::CSourceLoader(void)
{
	pthread_mutex_init(&m_csLock, NULL);
}

CSourceLoader::~CSourceLoader(void)
{
	/*
	CString szLibrary;
	CString szModule;

	for (POSITION pos = m_LibraryMap.GetStartPosition(); pos != NULL; )
	{
		szModule.Empty();
		m_LibraryMap.GetNextAssoc(pos, szLibrary, szModule);

		HMODULE_ hModule = (HMODULE_)atol((const char *)szModule);
		if (hModule) FreeLibrary(hModule);
	}
	*/
	map<const char*, HMODULE_>::iterator iter = m_LibraryMap.begin();
	while (iter != m_LibraryMap.end())
	{
		HMODULE_ hModule = (HMODULE_)iter->second;
		if (hModule) FreeLib(hModule);
		iter ++;
	}

	m_LibraryMap.clear();
	pthread_mutex_destroy(&m_csLock);
}

bool CSourceLoader::Create(const char* szLibrary, CBaseVideo** pCommonVideo)
{
	string szLib = (const char*)szLibrary;

#ifdef _DEBUG
	szLib += "d";
#endif

	HMODULE_ hModule = NULL;

	pthread_mutex_lock(&m_csLock);

	if (!FindModuleInLibrary(szLib.c_str(), hModule))
	{
		if (LoadLib(szLib.c_str(), hModule))
			AddModuleToLibrary(szLib.c_str(), hModule);
		else
		{
			pthread_mutex_unlock(&m_csLock);
			return false;
		}
	}

	SOURCE_CREATE m_fnCreate = NULL;
#ifdef WIN32
	m_fnCreate = (SOURCE_CREATE)::GetProcAddress((HMODULE)hModule, SOURCE_CREATE_INTERFACE);
#else
	m_fnCreate = (SOURCE_CREATE)dlsym(hModule, SOURCE_CREATE_INTERFACE);
#endif
	if (m_fnCreate)
	{
		*pCommonVideo = NULL;
		m_fnCreate(pCommonVideo);
		if (*pCommonVideo == NULL)
		{
			pthread_mutex_unlock(&m_csLock);
			return false;
		}
	}
	else
	{
		pthread_mutex_unlock(&m_csLock);
		return false;
	}

	pthread_mutex_unlock(&m_csLock);
	return true;
}

bool CSourceLoader::LoadLib(const char* strLibrary, HMODULE_& hModule)
{
	bool bResult = true;

#ifdef WIN32
	hModule = ::LoadLibrary(strLibrary);
#else
	hModule = dlopen(strLibrary, RTLD_LAZY|RTLD_NODELETE);
#endif
	if (!hModule)
		bResult = false;

	return bResult;
}

bool CSourceLoader::FreeLib(HMODULE_ hModule)
{
	bool bResult = true;

	if (hModule)
#ifdef WIN32
		::FreeLibrary((HMODULE)hModule);
#else
		dlclose(hModule);
#endif
	else
		bResult = false;

	return bResult;
}

void CSourceLoader::AddModuleToLibrary(const char* szLibrary, HMODULE_ hModule)
{
	//CString szModule;
	//szModule.Format("%.12d", hModule);
	//m_LibraryMap.SetAt(szLibrary, szModule);
	m_LibraryMap.insert(make_pair(szLibrary, hModule));
}

bool CSourceLoader::FindModuleInLibrary(const char* szLibrary, HMODULE_& hModule)
{
	bool bReturn = false;
	//CString szModule;
	//bReturn = m_LibraryMap.Lookup(szLibrary, szModule);
	//hModule = (HMODULE)atol(szModule);
	map<const char*, HMODULE_>::iterator i = m_LibraryMap.find(szLibrary);
	if (i != m_LibraryMap.end())
	{
		hModule = i->second;
		bReturn = true;
	}

	return bReturn;
}
