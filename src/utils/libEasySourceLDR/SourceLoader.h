#ifndef _RG4_EZSTREAMSVR_SOURCE_LOADER_H
#define _RG4_EZSTREAMSVR_SOURCE_LOADER_H

#include "../libEasySource/BaseSource.h"
#include <map>
#include <pthread.h>

#pragma once

#define SOURCE_CREATE_INTERFACE "LoadRsSource"
typedef bool (*SOURCE_CREATE)(CBaseVideo** pVideo);

#ifdef LIBEASYSOURCELDR_EXPORTS
#	define LIBEASYSOURCELDR_API __declspec(dllexport)
#else
#	define LIBEASYSOURCELDR_API __declspec(dllimport)
#endif

class LIBEASYSOURCELDR_API CSourceLoader
{
public:
	CSourceLoader(void);
	~CSourceLoader(void);

public:
	bool Create(const char* szLibrary, CBaseVideo** pCommonVideo);

private:
#ifdef WIN32
	typedef void* HMODULE_;
#else
	typedef HMODULE HMODULE_;
#endif
	bool LoadLib(const char* szLibrary, HMODULE_& hModule);
	bool FreeLib(HMODULE_ hModule);
	void AddModuleToLibrary(const char* szLibrary, HMODULE_ hModule);
	bool FindModuleInLibrary(const char* szLibrary, HMODULE_& hModule);

private:
	pthread_mutex_t	m_csLock;
	map<const char*, HMODULE_> m_LibraryMap;
};

#endif
