// MainService.h: interface for the CMainService class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _EASY_RTMP_MAIN_SERVICE_INCLUDE__
#define _EASY_RTMP_MAIN_SERVICE_INCLUDE__

#pragma once

#include <rscommon.h>
#include "../../utils/libEasyRTMP/StreamSource.h"
#include <EasyRTMPBase.h>

#include "../../../../RsCommon/common/RsUtility.h"

#define LOG_BUFFER_SIZE 1024*8

#include <list>
#include <vector>
#include <pthread.h>
using namespace std;

#ifdef WIN32
#define PATH_SEPERATOR '\\'
#else
#define PATH_SEPERATOR '/'
#endif


#ifdef __cplusplus
extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}
#endif 

class CMainService  
{
public:
	CMainService();
	virtual ~CMainService();

public:
	bool			InitService();
	bool			StartCapture(int nChannel, bool bEnableAudio, const char* strDeviceName, const char* strMixerLine, STREAMPROFILE* pProfile);
	bool			StopCapture(int nChannel);

	MEDIA_PROFILE	m_vRTMPProfile;
	CProductInfo	m_vProductInfo;
	bool			m_bSystemExit;
	RTMP_SVR_INFO	m_vRTMPSvrInfo;
	//////////////////////////////////////////////////////////////////////////
	//Log
	static void		stcLogReport(int nLogLevel, const char* szLogType, const char* szProgramID, const char* pszFormat, ...);
	int				m_nLogFailureCount;
	pthread_mutex_t	m_vLogArrayMutex;
	list<string*>	m_lstLogArray;
	bool			WriteLogToFile();
	void			LogDataFunction();
	static void*	ThreadLogData(void* pParam);
	pthread_t		m_tLogThread;
	char			TAG[32];
	//////////////////////////////////////////////////////////////////////////
	//Sound devices
	static int		DS_CameraCount();
	static int		DS_CameraName(int nCamID, char* sName, int nBufferSize);

	//////////////////////////////////////////////////////////////////////////
	//Utilitys
	bool			GotoURL(string url, int showcmd);

	//////////////////////////////////////////////////////////////////////////
	//Timer related
	void			Timer_Main();
	void			Timer_CheckLicense();
	int				m_nTimeCount;
#define EASYRTMP_MAX_DEMO_TIME	(365L*24L*60L*60L)

private:
	list<CStreamSource*> m_lstSource;
	bool			InitSockets();
	void			CleanupSockets();
	string			GetAppPath();
	bool			GetNetTime(SYSTEMTIME& vTime);
};

#endif
