//#include "stdafx.h"
//#include <afxwin.h>
#include "BaseSource.h"

#define DEFAULT_RECONNECT	10

CBaseVideo::CBaseVideo()
{
	m_fpRawVideoCB		= NULL;

	m_pParam			= NULL;
	m_fpFrameVCB		= NULL;
	m_fpFrameACB		= NULL;
	m_fpRtpPktCB		= NULL;

	m_nConnectStatus	= CONNECT_NOT_READY;
	m_nStopReceiveCount = 0;
	m_bReceiveVideo		= 0;
	m_bCompleteImage	= 0;
	m_bStopVideo		= 0;
	m_bSelfKilled		= 0;
	m_nReconnectPeriod	= DEFAULT_RECONNECT;
	memset(&m_vDeviceInfo, 0, sizeof(DEVICE_INFO));
	memset(&m_vMediaProfile, 0, sizeof(MEDIA_PROFILE));

	m_bOSDText			= false;
	m_bOSDTime			= false;
	//m_pOSDFilter		= NULL;

	m_pProfileParam		= NULL;
	m_fpMediaProfileCB	= NULL;
	memset(&m_vMediaProfile, 0, sizeof(m_vMediaProfile));

	m_pLogParam			= NULL;
	m_fpLogReport		= NULL;

	m_pCommandParam		= NULL;
	m_fpCommandFunc		= NULL;
	m_fpEventFunc		= NULL;
	m_pEventParam		= NULL;
}

CBaseVideo::~CBaseVideo()
{
	/*
	if (m_pOSDFilter)
	{
		delete[] m_pOSDFilter;
		m_pOSDFilter = NULL;
	}
	*/
}

void CBaseVideo::SetMedieProfileCB(void* pParam, PROFILE_CALLBACK fpMediaProfile)
{
	m_pProfileParam		= pParam;
	m_fpMediaProfileCB	= fpMediaProfile;
}

void CBaseVideo::SetupCallback(void* pParam, FRAME_CALLBACK fpVideoFunc, FRAME_CALLBACK_A fpAudioFunc, RTPKT_CALLBACK fpRTPPktCB)
{
	m_pParam		= pParam;
	m_fpFrameVCB	= fpVideoFunc;
	m_fpRtpPktCB	= fpRTPPktCB;
	m_fpFrameACB	= fpAudioFunc;
}

void CBaseVideo::SetupLogCallback(void* pParam, RsLogReport fpLogReport)
{
	m_pLogParam		= pParam;
	m_fpLogReport	= fpLogReport;
}

void CBaseVideo::SetupDeviceInfo(DEVICE_INFO *pCamInfo)
{
	memcpy(&m_vDeviceInfo, pCamInfo, sizeof(DEVICE_INFO));
}


//void CBaseVideo::SetupProfile(STREAMPROFILE* pProfile)
//{
//	memcpy(&m_vStreamProfile, pProfile, sizeof(STREAMPROFILE));
//}

void CBaseVideo::SetupProfile(MEDIA_PROFILE* pProfile)
{
	memcpy(&m_vMediaProfile, pProfile, sizeof(MEDIA_PROFILE));
}

void CBaseVideo::SetupRawCallback(void* pParam, RAWVIDEO_CALLBACK fpRawVideo, RAWAUDIO_CALLBACK fpRawAudio)
{
	m_pRawParam		= pParam;
	m_fpRawVideoCB	= fpRawVideo;
	m_fpRawAudioCB	= fpRawAudio;
}

void CBaseVideo::SetupOSDParameters(BOOL_ bText, BOOL_ bTime, string strFilter)
{
	m_bOSDText		= bText;
	m_bOSDTime		= bTime;
	/*
	if (m_pOSDFilter)
	{
		delete[] m_pOSDFilter;
		m_pOSDFilter = NULL;
	}
	m_pOSDFilter = new char[strFilter.length() + 1];
	*/
	memset(m_pOSDFilter, 0, MAX_OSD_FILTER_SIZE);
	strncpy(m_pOSDFilter, strFilter.c_str(), MAX_OSD_FILTER_SIZE - 1);
}

// This method is called every 3 seconds
BOOL_ CBaseVideo::CheckReceiveStatus()
{
	if (m_bReceiveVideo)
	{
		m_nStopReceiveCount = 0;
		m_bReceiveVideo = FALSE_;
	}
	else
	{
		if (++m_nStopReceiveCount >= m_nReconnectPeriod)
		{
			m_nStopReceiveCount = 0;
			return FALSE_;
		}
	}

	return TRUE_;
}

/*
BOOL CBaseVideo::IsDeviceStatusOK()
{
	if (!m_bReceiveVideo && m_nStopReceiveCount >= 10)
		return FALSE;

	return TRUE;
}

UINT CBaseVideo::GetReconnectPeriod(int nTimePeriod)
{
	return m_nReconnectPeriod;
}

void CBaseVideo::SetReconnectPeriod(int nTimePeriod)
{
	if (m_nReconnectPeriod < DEFAULT_RECONNECT)
		m_nReconnectPeriod = DEFAULT_RECONNECT;
	else
		m_nReconnectPeriod = nTimePeriod;
}

UINT CBaseVideo::GetConnectStatus()
{
	return m_nConnectStatus;
}

void CBaseVideo::SetConnectStatus(UINT nConnectStatus)
{
	m_nConnectStatus = nConnectStatus;
}


BOOL CBaseVideo::StartCmd()
{
return TRUE;
}

BOOL CBaseVideo::StopCmd()
{
return TRUE;
}

void CBaseVideo::OnCmdReceive(int nErrorCode)
{
	
}

void CBaseVideo::OnCmdClose(int nErrorCode)
{

}

void CBaseVideo::OnCmdConnect(int nErrorCode)
{

}

void CBaseVideo::OnCmdSend(int nErrorCode)
{

}

void CBaseVideo::SetCommandFunction(LPVOID pCmdParam, COMMANDFUNC fpCommandFunc, ONCMDCONNECTFUNC fpCmdConnectFunc)
{
	m_pCmdParam = pCmdParam;
	m_fpCommandFunc = fpCommandFunc;
	m_fpCmdConnectFunc = fpCmdConnectFunc;
}

void CBaseVideo::SetEventFunction(LPVOID pEvtParam, ONEVENTFUNC fpEventFunc)
{
	m_pEvtParam = pEvtParam;
	m_fpEventFunc = fpEventFunc;
}

BOOL CBaseVideo::ExecParameter()
{
	return TRUE;
}

BOOL CBaseVideo::ExecControl()
{
	return TRUE;
}

void CBaseVideo::SetStatusFunction(LPVOID pStaParam, ONSTATUSFUNC fpStatusFunc)
{
	m_pStatParam = pStaParam;
	m_fpStatusFunc = fpStatusFunc;
}

BOOL CBaseVideo::ConnectUploadStream(CString szCGI)
{
	return FALSE;
}

BOOL CBaseVideo::UploadAudioStream(BYTE* pBuffer, int nSize)
{
	return FALSE;
}
*/