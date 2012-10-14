#ifndef __RG4_EZSTREAMSVR_BASE_SOURCE_H
#define __RG4_EZSTREAMSVR_BASE_SOURCE_H

#pragma once

#include <stdint.h>
#include <rscommon.h>
#include <string>
using namespace std;

#ifdef LIBEASYSOURCE_EXPORTS
#	define LIBEASYSOURCE_API __declspec(dllexport)
#else
#	define LIBEASYSOURCE_API __declspec(dllimport)
#endif

class LIBEASYSOURCE_API CBaseVideo
{
public:
	CBaseVideo();
	virtual ~CBaseVideo();

// Overrides
public:
	virtual BOOL_		Start() = 0;
	virtual BOOL_		Stop() = 0;

	virtual BOOL_		CheckReceiveStatus();

	// Operations
public:
	//should be a public interface, but i don't know why EzStreamSvr can not find the interface
	//so move to virtual temporary
	void				SetupDeviceInfo(DEVICE_INFO* pCamInfo);
	void				SetMedieProfileCB(void* pParam, PROFILE_CALLBACK fpMediaProfile);
	void				SetupCallback(void* pParam, FRAME_CALLBACK fpVideoFunc, FRAME_CALLBACK_A fpAudioFunc, RTPKT_CALLBACK fpRTPPktCB);
	void				SetupLogCallback(void* pParam, RsLogReport fpLogReport);

	//for libV4R capture only
	void				SetupRawCallback(void* pParam, RAWVIDEO_CALLBACK fpRawVideo, RAWAUDIO_CALLBACK fpRawAudio);
//	void				SetupProfile(STREAMPROFILE* pProfile);
	void				SetupProfile(MEDIA_PROFILE* pProfile);
	void				SetupOSDParameters(BOOL_ bText, BOOL_ bTime, string strFilter);

	// Attributes
public:
	DEVICE_INFO			m_vDeviceInfo;

	COMMAND_CALLBACK	m_fpCommandFunc;
	void*				m_pCommandParam;

	EVENT_CALLBACK		m_fpEventFunc;
	void*				m_pEventParam;

	void*				m_pProfileParam;
	PROFILE_CALLBACK	m_fpMediaProfileCB;
	MEDIA_PROFILE		m_vMediaProfile;

	void*				m_pParam;
	FRAME_CALLBACK		m_fpFrameVCB;
	RTPKT_CALLBACK		m_fpRtpPktCB;
	FRAME_CALLBACK_A	m_fpFrameACB;

	void*				m_pRawParam;
	RAWVIDEO_CALLBACK	m_fpRawVideoCB;
	RAWAUDIO_CALLBACK	m_fpRawAudioCB;

	void*				m_pLogParam;
	RsLogReport			m_fpLogReport;
	char				TAG[32];

	unsigned int		m_nConnectStatus;
	unsigned int		m_nStopReceiveCount;
	unsigned int		m_nReconnectPeriod;
	int					m_bCompleteImage;
	int					m_bReceiveVideo;
	int					m_bStopVideo;
	unsigned short		m_wPreFrameType;
	int					m_bSelfKilled;
	SYSTEMTIME_			m_stConnectTime;

	//for libV4R capture only
	//	STREAMPROFILE		m_vStreamProfile;//for init Capture, the real one will be re-init after get stream source.
	BOOL_				m_bOSDText;
	BOOL_				m_bOSDTime;
#define MAX_OSD_FILTER_SIZE 256
	char				m_pOSDFilter[MAX_OSD_FILTER_SIZE];
};

#endif // __RG4_EZSTREAMSVR_BASE_SOURCE_H
