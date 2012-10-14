#ifndef __EASYRTMP_BASIC_DEFINITION_INCLUDE
#define __EASYRTMP_BASIC_DEFINITION_INCLUDE

#ifdef WIN32
#include <stdio.h>
#include <tchar.h>
#endif

#include <string>
#include <cctype>
#include <algorithm>
#include <list>
#include <vector>
#include <map>
#include <set>
using namespace std;

//Buffer length define
#define MAX_LEN_RTP_SDP			2*1024
#define MAX_LEN_PROFILE_NAME	32
#define MAX_LEN_STREAM_ID		32
#define MAX_LEN_SESSION_ID		64
#define MAX_LEN_SERVER_ID		32
#define MAX_LEN_PASSWORD		32
#define MAX_LEN_USER_ID			16
#define MAX_LEN_AUDIO_RAW		192000
#define MAX_LEN_VIDEO_RAW		1280*1024*3

typedef void (*EASYLOG)(int nLogLevel, const char* szLogType, const char* szProgramID, const char* pszFormat, ...);

class CProductInfo
{
public:
	string				m_strProductName;
	string				m_strVersion;
	string				m_strServerID;
	//auth
	string				m_strAuthUser;
	string				m_strAuthPass;

	bool				m_bEnableAudio;
	//command settings
	bool				m_bEnableCmd;
	int					m_nCmdTimeout;

	//text overlay
	bool				m_bOverlayTime;
	bool				m_bOverlayLogo;
	string				m_strLogoText;

	//log
	int					m_nLogLevel;
	string				m_szLogPath;
	int					m_nLogReverseDay;
	//multiple EzStreamSvr load-balancing related, ip settings
	string				m_strInternalIP;
	string				m_szInternalRtspPort;
	string				m_szInternalCmdPort;
	string				m_strExternalIP;
	string				m_szExternalRtspPort;
	string				m_szExternalCmdPort;

	bool				m_bHeartBeatFlag;
	string				m_szHeartBeatInterval;
	bool				m_bUIConnectionFlag;
	bool				m_bUICapabilityFlag;
	int					m_nCmdTimeOut;
	int					m_szRemotePort;
	int					m_nTimerInterval;//unit=milli second

	int					m_nSourceDeleteWait;
	int					m_nStreamDeleteWait;
};

typedef struct T_RTMP_SVR_INFO
{
	char				szServerIP[128];
	int					nServerPort;
}RTMP_SVR_INFO;
/*
typedef struct T_RTMPROFILE
{
	unsigned short	wType;
	char			cProfileID[32];
	//Video Related
	bool			bOnFlagV;			//false->close, true->open
	unsigned int	iCodecIDV;
	unsigned short	wImageWidth;		//0->audio
	unsigned short	wImageHeight;		//0->audio
	unsigned short	wGopSize;
	int				nBitrate;
	double			wFrameRate;
	unsigned short	wLevel;				//1, 2, 3
	char			sdpConfig[1024];		//sprop_parameter_sets
	//Audio Related
	bool			bOnFlagA;
	unsigned int	iCodecIDA;
	unsigned short	wFrameRateA;
	int				nBitrateA;
	unsigned short	iChannels;
	unsigned int	iBitsPerSample;
	unsigned int	iSamplePerSec;
	unsigned int	iBytesPerCapture;
	//Red5/FMS server info
	char			szServerIP[64];
	unsigned int	nServerPort;
}RTMPROFILE;
*/

typedef struct T_STREAMPROFILE
{
	unsigned short	wType;
	char			cProfileID[MAX_LEN_PROFILE_NAME];
	int				nProfileID;
	//Video Related
	bool			bOnFlagV;			//false->close, true->open
	unsigned int	iCodecIDV;
	unsigned short	wImageWidth;		//0->audio
	unsigned short	wImageHeight;		//0->audio
	unsigned short	wGopSize;
	int				iBitRateV;
	double			dFrameRateV;
	unsigned short	wLevel;				//1, 2, 3
	char			sdpConfig[MAX_LEN_RTP_SDP];		//sprop_parameter_sets
	unsigned int	iSampleRateV;
	//Audio Related
	bool			bOnFlagA;
	unsigned int	iCodecIDA;
	unsigned short	iChannels;
	unsigned int	iBitsPerSample;
	unsigned int	iSamplePerSec;
	unsigned int	iBytesPerCapture;
	double			dFrameRateA;
	int				iBitRateA;
}STREAMPROFILE;

#endif

