#include "stdafx.h"
#include "StreamSource.h"

//#include "MainService.h"
//extern CMainService* g_pMainService;

static const char* TAG = "StreamSource";

#ifdef _DEBUG
#pragma comment(lib, "libEasySrcLDRd.lib")
#pragma comment(lib, "libEasySourceD.lib")
//#pragma comment(lib, "librtmpd.lib")
//#pragma comment(lib, "../../bin_debug/zlibd.lib")
//#pragma comment(lib, "../../bin_debug/libeay32.lib")
//#pragma comment(lib, "../../bin_debug/ssleay32.lib")
//#pragma comment(lib, "../../bin_debug/libfaac.lib")
//#pragma comment(lib, "../../bin_debug/libcapaudio.lib")
//#pragma comment(lib, "../../bin_debug/libEzCodec.lib")
#else
#pragma comment(lib, "libEasySrcLDR.lib")
#pragma comment(lib, "libEasySource.lib")
//#pragma comment(lib, "librtmp.lib")
//#pragma comment(lib, "../../bin/libEzCodec.lib")
#endif

#pragma comment(lib, "librtmp.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "winmm.lib")

#include "bswap.h"

/************************************************
* Buffer writing funcs
************************************************/
static inline void flv_put_byte(uint8_t **data, uint8_t val)
{
	*data[0] = val;
	*data += 1;
}

static inline void flv_put_be16(uint8_t **data, uint16_t val)
{
#if (defined ARCH_ARM) && !(defined HAVE_ARMV6)
	flv_put_byte(data, val >> 8);
	flv_put_byte(data, val & 0x0ff);
#else
	*(uint16_t *) (*data) = me2be_16(val);
	*data += 2;
#endif
}

static inline void flv_put_be24(uint8_t **data, uint32_t val)
{
	flv_put_be16(data, val >> 8);
	flv_put_byte(data, val & 0x0ff);
}

static inline void flv_put_be32(uint8_t **data, uint32_t val)
{
#if (defined ARCH_ARM) && !(defined HAVE_ARMV6)
	flv_put_be16(data, val >> 16);
	flv_put_be16(data, val & 0x0ffff);
#else
	*(uint32_t *) (*data) = me2be_32(val);
	*data += 4;
#endif
}

static inline void flv_put_buff(uint8_t **data, const uint8_t *src, int32_t srcsize)
{
	memcpy(*data, src, srcsize);
	*data += srcsize;
}

void flv_put_tag(uint8_t **data, const char *tag)
{
	while (*tag) {
		flv_put_byte(data, *tag++);
	}
}

int CStreamSource::modify_sps_pps(qboxContext *qbox, uint8_t* data)
{
	uint8_t *ptr = data;
	uint8_t *buf;
	uint32_t sps_size = 0;
	uint32_t pps_size = 0;
	const uint8_t *sps = NULL;
	const uint8_t *pps = NULL;
	uint32_t size;
	uint8_t nal_type;

	buf = qbox->data;
	while(buf - qbox->data < qbox->qbox_size)
	{
		size = beptr2me_32(buf);
		nal_type = buf[4] & 0x1f;
		if (nal_type == 7) { /* SPS */
			sps = buf + 4;
			sps_size = size;
		} else if (nal_type == 8) { /* PPS */
			pps = buf + 4;
			pps_size = size;
		}
		buf += size + 4;
	}

	//ASSERT(sps);
	//ASSERT(pps);
#ifdef EXTRA_DEBUG
	RS_OUTPUT("sps_size: %u, pps_size: %u", sps_size, pps_size);
#endif
	flv_put_byte(&ptr, 1); /* version */
	flv_put_byte(&ptr, sps[1]); /* profile */
	flv_put_byte(&ptr, sps[2]); /* profile compat */
	flv_put_byte(&ptr, sps[3]); /* level */
	flv_put_byte(&ptr, 0xff); /* 6 bits reserved (111111) + 2 bits nal size length - 1 (11) */
	flv_put_byte(&ptr, 0xe1); /* 3 bits reserved (111) + 5 bits number of sps (00001) */

	flv_put_be16(&ptr, sps_size);
	flv_put_buff(&ptr, sps, sps_size);
	flv_put_byte(&ptr, 1); /* number of pps */
	flv_put_be16(&ptr, pps_size);
	flv_put_buff(&ptr, pps, pps_size);

	return ptr - data;
}

CStreamSource::CStreamSource()
{
	m_nKeyFrameCount= 0;
	m_pSourceInst	= NULL;
	memset(&m_vDeviceInfo, 0, sizeof(m_vDeviceInfo));

	m_bRTMPInit		= false;
	m_pRTMPHandle	= NULL;

	m_pSendBufVideoReal	= new unsigned char[RTMP_MAX_VIDEO_SIZE + 64];
	//m_pSendBufVideo	= m_pSendBufVideoReal + 32;
	m_pSendBufVideo		= (unsigned char*)(((DWORD)m_pSendBufVideoReal+64)&0xffffffc0);
	m_pAudioSendBufReal	= new char[RTMP_MAX_AUDIO_SIZE + 64];
	//m_pAudioSendBuf	= m_pAudioSendBufReal + 32;
	m_pAudioSendBuf		= (char*)(((DWORD)m_pAudioSendBufReal+64)&0xffffffc0);
}

CStreamSource::~CStreamSource(void)
{
	RTMPRelease();

	if (m_pSendBufVideoReal)
	{
		delete[] m_pSendBufVideoReal;
		m_pSendBufVideoReal = NULL;
	}
	if (m_pAudioSendBufReal)
	{
		delete[] m_pAudioSendBufReal;
		m_pAudioSendBufReal = NULL;
	}
}

void CStreamSource::RTMPRelease()
{
	if (m_pRTMPHandle)
	{
		RTMP_DeleteStream(m_pRTMPHandle);
		RTMP_Close(m_pRTMPHandle);
		RTMP_Free(m_pRTMPHandle);
		m_pRTMPHandle = NULL;
	}
}

/*
bool CStreamSource::StartCapture(int nChannel, bool bPreview)
{
	bool ret = false;

	m_nKeyFrameCount = 0;

	StopCapture();

//#ifdef _DEBUG
//	m_bSaveFLV	= true;
//	CTime vTime = CTime::GetCurrentTime();
//	m_strFileFLV.Format("%04d%02d%02d%02d%02d%02d.flv", vTime.GetYear(), vTime.GetMonth(), vTime.GetDay(), vTime.GetHour(), vTime.GetMinute(), vTime.GetSecond());
//#else
//	m_bSaveFLV	= false;
//#endif
	//test
//	m_bSave264	= true;
//	m_strFile264.Format("%04d%02d%02d%02d%02d%02d.264", vTime.GetYear(), vTime.GetMonth(), vTime.GetDay(), vTime.GetHour(), vTime.GetMinute(), vTime.GetSecond());

	//////////////////////////////////////////////////////////////////////////
	//1. start capture video
	//////////////////////////////////////////////////////////////////////////
//	m_pCamera = new CRsCameraDS();
	ret = m_vRsCamera.OpenCamera(m_nChannelNo, bPreview, m_vStreamProfile.wImageWidth, m_vStreamProfile.wImageHeight);
	if (!ret )
	{
		if (m_fpLogReport)
			m_fpLogReport(LOGLEVEL0, "E", TAG, "Capture Camera %d Failed(%d:%d)", m_vStreamProfile.wImageWidth, m_vStreamProfile.wImageHeight);
		return false; 
	}

	char uri[256];
	sprintf(uri, "rtmp://%s:%d/live/%d_%dx%d", m_vStreamProfile.szServerIP, m_vStreamProfile.nServerPort, m_nChannelNo, m_vStreamProfile.wImageWidth, m_vStreamProfile.wImageHeight); 
	m_strRtmpURL = "";
	m_strRtmpURL += uri;

	m_bCapturing = true;

	int res = pthread_create(&m_tThreadCapture, NULL, ThreadCapture, this);
	if (res == 0)
		ret = true;

	//////////////////////////////////////////////////////////////////////////
	// Start capture audio
	//////////////////////////////////////////////////////////////////////////
	if (m_fpLogReport)
		m_fpLogReport(LOGLEVEL0, "I", TAG, "Start AudioCapture");
#ifdef _USE_WAVEIN
	if (!m_pAudio)
	{
		EZCA2_CreateInstance(&m_pAudio, this);
	}
	if (m_pAudio)
	{
		m_vAudioParam.nChannels		= m_vStreamProfile.iChannels;
		m_vAudioParam.nBitsPerSample= m_vStreamProfile.iBitsPerSample;
		m_vAudioParam.nSamplePerSec	= m_vStreamProfile.iSamplePerSec;
		m_vAudioParam.nWaveInSize	= m_vStreamProfile.iBytesPerCapture;
		//m_pAudio->Connect(0);
		EZCA2_InitCapture(m_pAudio, &m_vAudioParam, InputRawAudio);

		ret = EZCA2_StartCapture(m_pAudio);
		if (ret)
		{
			if (m_fpLogReport)
				m_fpLogReport(LOGLEVEL0, "I", TAG, "Start Capture Audio Successfully!");
		}
		else
		{
			if (m_fpLogReport)
				m_fpLogReport(LOGLEVEL0, "E", TAG, "Start Capture Audio Failed!");
		}
	}
#else
	if (!m_pAudio)
	{
		EZCA_CreateInstance(&m_pAudio, this);
	}
	if (m_pAudio)
	{
		//m_pAudio->Connect(0);
		return EZCA_StartCapture(m_pAudio, nIndex, &m_vAudioParam, stcCaptureCB, NULL);
	}
#endif

	if (ret)
	{
		//		if (m_cbEventCB)
		//			m_cbEventCB(m_lpEventParam, EVENT_AUDIO_OPENED, NULL, 0, 0, 0);
	}

	if (m_fpLogReport)
		m_fpLogReport(LOGLEVEL0, "I", TAG,"Capture thread started successfully!");

	return ret;
}

void* CStreamSource::ThreadCapture(void* param)
{
	CStreamSource* p = (CStreamSource*)param;
	uint8_t* pFrame;

	int w = p->m_vRsCamera.GetWidth();//m_vStreamProfile.wImageWidth;
	int h = p->m_vRsCamera.GetHeight();//m_vStreamProfile.wImageHeight;
	p->m_vStreamProfile.wImageWidth	= w; 
	p->m_vStreamProfile.wImageHeight= h; 
	int s = w*h*3;
	int n = 1000/p->m_vStreamProfile.wFrameRate;

	if (m_fpLogReport)
		m_fpLogReport(LOGLEVEL0, "I", TAG, "Capture Thread Started!");

	while (1)
	{
		if (!p->m_bCapturing)
			break;

		pFrame = p->m_vRsCamera.QueryFrame();
		if (!pFrame)
			continue;

		p->InputRawVideo(pFrame, s, w, h, PIX_FMT_BGR24, GetTickCount(), 1);

		Sleep(n);
	}

	if (m_fpLogReport)
		m_fpLogReport(LOGLEVEL0, "I", TAG, "Capture Thread Terminated!");

	if (p->m_pEnc_v)
	{
		delete p->m_pEnc_v;
		p->m_pEnc_v= NULL;
	}

	pthread_detach(pthread_self());

	return NULL;
}

bool CStreamSource::StopCapture()
{
	m_bCapturing = false;

//	if (m_pCamera)
	{
		m_vRsCamera.CloseCamera();

		Sleep(200);
//		if (m_tThreadCapture)
//		{
//			void* ret = NULL;
//			int n =pthread_join(m_tThreadCapture, &ret);
//			printf("adfadf: %d\n", n);
//		}

//		delete m_pCamera;
//		m_pCamera = NULL;
	}

	if (m_fpLogReport)
		m_fpLogReport(LOGLEVEL0, "I", TAG,"Capture thread stopped!");

	if (m_pAudio)
	{
#ifdef _USE_WAVEIN
		EZCA2_StopCapture(m_pAudio);
		EZCA2_DestroyInstance(m_pAudio);
		EZCA2_Terminate();
#else
		EZCA_DestroyInstance(m_pAudio);
#endif
		m_pAudio = NULL;
	}


	return true;
}
*/

#define HEX2BIN(a)      (((a)&0x40)?((a)&0xf)+9:((a)&0xf)) 
int hex2bin(char *str, char **hex) 
{ 
	char *ptr; 
	int i, l = strlen(str); 
	if (l & 1) 
		return 0; 
	*hex = (char *)malloc(l/2); 
	ptr = *hex; 
	if (!ptr) 
		return 0; 
	for (i=0; i<l; i+=2) 
		*ptr++ = (HEX2BIN(str[i]) << 4) | HEX2BIN(str[i+1]); 
	return l/2; 
} 
char * put_byte( char *output, uint8_t nVal ) 
{ 
	output[0] = nVal; 
	return output+1; 
} 
char * put_be16(char *output, uint16_t nVal ) 
{ 
	output[1] = nVal & 0xff; 
	output[0] = nVal >> 8; 
	return output+2; 
} 
char * put_be24(char *output,uint32_t nVal ) 
{ 
	output[2] = nVal & 0xff; 
	output[1] = nVal >> 8; 
	output[0] = nVal >> 16; 
	return output+3; 
} 
char * put_be32(char *output, uint32_t nVal ) 
{ 
	output[3] = nVal & 0xff; 
	output[2] = nVal >> 8; 
	output[1] = nVal >> 16; 
	output[0] = nVal >> 24; 
	return output+4; 
} 
char *  put_be64( char *output, uint64_t nVal ) 
{ 
	output=put_be32( output, nVal >> 32 ); 
	output=put_be32( output, nVal ); 
	return output; 
} 
char * put_amf_string( char *c, const char *str ) 
{ 
	uint16_t len = strlen( str ); 
	c=put_be16( c, len ); 
	memcpy(c,str,len); 
	return c+len; 
} 
char * put_amf_double( char *c, double d ) 
{ 
	*c++ = AMF_NUMBER;  /* type: Number */ 
	{ 
		uint8_t *ci, *co; 
		ci = (uint8_t *)&d; 
		co = (uint8_t *)c; 
		co[0] = ci[7]; 
		co[1] = ci[6]; 
		co[2] = ci[5]; 
		co[3] = ci[4]; 
		co[4] = ci[3]; 
		co[5] = ci[2]; 
		co[6] = ci[1]; 
		co[7] = ci[0]; 
	} 
	return c+8; 
}

void CStreamSource::SetLog(EASYLOG log)
{
	m_fpLogReport = log;
}

bool CStreamSource::RTMPInit(const char* uri, uint8_t* p264, int n264)
{
	if (m_fpLogReport)
		m_fpLogReport(LOGLEVEL3, "I", TAG, "Start Init RTMP socket!");

	memset(&qbox, 0, sizeof(qbox));
	memset(&flv, 0, sizeof(flv));

	uint8_t *temp = p264, *sps = NULL, *pps = NULL, *iframe = NULL;
	int n, n_sps = 24, n_pps = 4, n_iframe = 0;
	bool found_sps = false, found_pps = false, found_i = false;

	//locate sps
	for (n = 0; n < n264; n ++)
	{
		if (temp[0] == 0x00 && temp[1] == 0x00 && temp[2] == 0x01 && temp[3] == 0x67)
		{
			sps = temp + 3;
			found_sps = true;
			break;
		}
		else
		{
			temp ++;
		}
	}
	if (!found_sps)
		return false;

	//locate pps
	temp = sps;
	for (n = 0; n < n264 - (sps - p264); n ++)
	{
		if (temp[0] == 0x00 && temp[1] == 0x00 && temp[2] == 0x01 && temp[3] == 0x68)
		{
			pps = temp + 3;
			found_pps = true;
			break;
		}
		else
		{
			temp ++;
		}
	}
	if (!found_pps)
		return false;
	n_sps = pps - sps - 4;

	//locate i frame
	temp = pps;
	for (n = 0; n < n264 - (pps - p264); n ++)
	{
		if (temp[0] == 0x00 && temp[1] == 0x00 && temp[2] == 0x01 && temp[3] == 0x65)
		{
			iframe = temp + 3;
			found_i = true;
			break;
		}
		else
		{
			temp ++;
		}
	}
	if (!found_i)
		return false;
	n_pps = iframe - pps - 4;
	//skip 00 00 01 06 05
	if (n_pps > 4)
		n_pps = 4;
	n_iframe = n264 - (iframe - p264);

	//////////////////////////////////////////////////////////////////////////
	//1. Init RTMP handle & connect rtmp server(red5/FMS)
	//////////////////////////////////////////////////////////////////////////
	RTMPRelease();

	m_pRTMPHandle = RTMP_Alloc(); 

	if (!m_pRTMPHandle)
		return false;
	else
	{
		try
		{
//			RTMP_debuglevel = RTMP_LOGWARNING;
//			RTMP_LogSetLevel(RTMP_debuglevel);
			RTMP_Init(m_pRTMPHandle);
			if (!RTMP_SetupURL(m_pRTMPHandle, (char*)uri))
				return false;
			RTMP_EnableWrite(m_pRTMPHandle);
			if (!RTMP_Connect(m_pRTMPHandle, NULL))
			{
				if (m_fpLogReport)
					m_fpLogReport(LOGLEVEL0, "E", TAG, "Connect Red5/FMS Server Failed!");
				m_bCapturing = false;
				return false;
			}
			if (!RTMP_ConnectStream(m_pRTMPHandle, 0))
			{
				if (m_fpLogReport)
					m_fpLogReport(LOGLEVEL0, "E", TAG, "Connect Red5/FMS Stream Failed!");
				m_bCapturing = false;
				return false;
			}
			if (m_fpLogReport)
				m_fpLogReport(LOGLEVEL3, "I", TAG, "Connect Red5/FMS Stream Successfully!");
		}
		catch (...)
		{
			m_bCapturing = false;
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//2. Send FLV header
	//////////////////////////////////////////////////////////////////////////
	memset(&m_vRTMPacketV,0,sizeof(RTMPPacket)); 
	m_vRTMPacketA.m_nChannel		= 0x04; //0x04 means Source Channel
	m_vRTMPacketA.m_headerType		= RTMP_PACKET_SIZE_LARGE; 
	m_vRTMPacketA.m_nTimeStamp		= 0; 
	m_vRTMPacketA.m_nInfoField2		= m_pRTMPHandle->m_stream_id; 
	m_vRTMPacketA.m_hasAbsTimestamp = 0; 
	m_vRTMPacketA.m_body			= (char *)m_pAudioSendBuf; 
	m_vRTMPacketA.m_packetType		= RTMP_PACKET_TYPE_AUDIO; 

	memset(&m_vRTMPacketV,0,sizeof(RTMPPacket)); 
	m_vRTMPacketV.m_nChannel		= 0x04; //0x04 means Source Channel
	m_vRTMPacketV.m_headerType		= RTMP_PACKET_SIZE_LARGE; 
	m_vRTMPacketV.m_nTimeStamp		= 0; 
	m_vRTMPacketV.m_nInfoField2		= m_pRTMPHandle->m_stream_id; 
	m_vRTMPacketV.m_hasAbsTimestamp = 0; 
	m_vRTMPacketV.m_body			= (char *)m_pSendBufVideo; 
	char * szTmp					= (char *)m_pSendBufVideo; 
	m_vRTMPacketV.m_packetType		= RTMP_PACKET_TYPE_INFO; 

	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string(szTmp, "@setDataFrame" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string(szTmp, "onMetaData" ); 
	szTmp=put_byte(szTmp, AMF_OBJECT ); 
	szTmp=put_amf_string( szTmp, "author" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_amf_string( szTmp, "copyright" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_amf_string( szTmp, "description" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_amf_string( szTmp, "keywords" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_amf_string( szTmp, "rating" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_amf_string( szTmp, "presetname" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "Custom" ); 
	szTmp=put_amf_string( szTmp, "width" ); 
	szTmp=put_amf_double( szTmp, m_vMediaProfile.vVideoProfile.nWidth); 
	szTmp=put_amf_string( szTmp, "width" ); 
	szTmp=put_amf_double( szTmp, m_vMediaProfile.vVideoProfile.nWidth); 
	szTmp=put_amf_string( szTmp, "height" ); 
	szTmp=put_amf_double( szTmp, m_vMediaProfile.vVideoProfile.nHeight); 
	szTmp=put_amf_string( szTmp, "framerate" );
	szTmp=put_amf_double( szTmp, m_vMediaProfile.vVideoProfile.dFPS);
	szTmp=put_amf_string( szTmp, "videocodecid" );
	szTmp=put_byte(szTmp, AMF_STRING );
	szTmp=put_amf_string( szTmp, "avc1" );
	szTmp=put_amf_string( szTmp, "videodatarate" );
	szTmp=put_amf_double( szTmp, m_vMediaProfile.vVideoProfile.nBitRate);
	szTmp=put_amf_string( szTmp, "avclevel" ); 
	szTmp=put_amf_double( szTmp, m_vMediaProfile.vVideoProfile.nLevel);
	szTmp=put_amf_string( szTmp, "avcprofile" );
	szTmp=put_amf_double( szTmp, 0x42 );
	szTmp=put_amf_string( szTmp, "videokeyframe_frequency" );
	szTmp=put_amf_double( szTmp, 3 );
	szTmp=put_amf_string( szTmp, "" );
	szTmp=put_byte( szTmp, AMF_OBJECT_END );

	/*
	if (m_vStreamProfile.bOnFlagA)
	{
		szTmp=put_amf_string( szTmp, "audiodatarate" );
		szTmp=put_amf_double( szTmp, m_vStreamProfile.nBitrateA/1024.0);
		szTmp=put_amf_string( szTmp, "audiosamplerate" );
		szTmp=put_amf_double( szTmp, m_vStreamProfile.iSamplePerSec);
		szTmp=put_amf_string( szTmp, "audiosamplesize" );
		szTmp=put_amf_double( szTmp, m_vStreamProfile.iBitsPerSample);
		szTmp=put_amf_string( szTmp, "stereo" );
		szTmp=put_amf_double( szTmp, m_vStreamProfile.iChannels == 2);
		szTmp=put_amf_string( szTmp, "audiocodecid" );
		if (m_vStreamProfile.iCodecIDA == CODEC_ID_AAC)
			szTmp=put_amf_double( szTmp, 10.0);//FLV_CODECID_AAC
		else if (m_vStreamProfile.iCodecIDA == CODEC_ID_MP3)
			szTmp=put_amf_double( szTmp, 2.0);//FLV_CODECID_MP3
		else
			szTmp=put_amf_double( szTmp, 10.0);//FLV_CODECID_AAC

		szTmp=put_amf_string( szTmp, "filesize" );
		szTmp=put_amf_double( szTmp, 0.0);
	}
	*/

	m_vRTMPacketV.m_nBodySize = szTmp-(char *)m_pSendBufVideo;

	RTMP_SendPacket(m_pRTMPHandle,&m_vRTMPacketV,1); 
//	SaveFLV(m_vRTMPacketV.m_body, m_vRTMPacketV.m_nBodySize);

	//////////////////////////////////////////////////////////////////////////
	//3. Send SPS & PPS
	//////////////////////////////////////////////////////////////////////////
	m_vRTMPacketV.m_packetType = RTMP_PACKET_TYPE_VIDEO;   /* VIDEO */ 
	memset(m_pSendBufVideo, 0, RTMP_MAX_VIDEO_SIZE);
	m_pSendBufVideo[ 0]=0x17; 
	m_pSendBufVideo[ 1]=0x00; 
	m_pSendBufVideo[ 2]=0x00; 
	m_pSendBufVideo[ 3]=0x00; 
	m_pSendBufVideo[ 4]=0x00; 
	m_pSendBufVideo[ 5]=0x01; 
	m_pSendBufVideo[ 6]=0x42; 
	m_pSendBufVideo[ 7]=0xC0; 
	m_pSendBufVideo[ 8]=0x15; 
	m_pSendBufVideo[ 9]=0x03; 
	m_pSendBufVideo[10]=0x01; 
	szTmp=(char *)m_pSendBufVideo+11; 

	n_sps = htons(n_sps);
	memcpy(szTmp,&n_sps,sizeof(short));
	szTmp+=sizeof(short); 
	memcpy(szTmp, sps, htons(n_sps)); 
	szTmp += htons(n_sps); 
	*szTmp = 0x01; 
	szTmp += 1; 

	n_pps = htons(n_pps); 
	memcpy(szTmp,&n_pps,sizeof(short)); 
	szTmp += sizeof(short); 
	memcpy(szTmp, pps , htons(n_pps)); 
	szTmp += htons(n_pps); 

	m_vRTMPacketV.m_nBodySize=szTmp-(char *)m_pSendBufVideo; 
	n = szTmp-(char *)m_pSendBufVideo;
	RTMP_SendPacket(m_pRTMPHandle,&m_vRTMPacketV,0); 
	
//	SaveFLV(m_vRTMPacketV.m_body, m_vRTMPacketV.m_nBodySize);

	nTimes		= 0; 
	oldTick		= GetTickCount(); 
	newTick		= 0; 
	m_vRTMPacketV.m_nTimeStamp = 0;

	//////////////////////////////////////////////////////////////////////////
	//4. Send I frame
	//////////////////////////////////////////////////////////////////////////
	memset(m_pSendBufVideo, 0, RTMP_MAX_VIDEO_SIZE);

	RTMPSendV(1, iframe, n_iframe);
	//RTMPSendV(1, p264, n264);

	return true;
}

//void CStreamSource::stcVideoEncodeCallback(int nStreamType, void* pParam, uint8_t* pEncBuffer, int nEncSize,
//										   int nCodecID,int nImageWidth,int nImageHeight, int keyframe, int nFrameNumber, 
//										   const char* szExtraData, unsigned long long llTimestamp)
void CStreamSource::stcVideoEncodeCallback(void* pParam, uint8_t* pFrame, int nBytes, int nCodecID, int vDuringAlarm, int16_t keyframe, int bStretchMode, int nWidth, int nHeight, int32_t ts)
{
	CBaseVideo* pp = (CBaseVideo*)pParam;
	CStreamSource* p = (CStreamSource*)pp->m_pParam;

	if (!p->m_bRTMPInit)
	{
		if (!keyframe)
			return;
		bool ret = p->RTMPInit(p->m_strRtmpURL.c_str(), pFrame, nBytes);
		if (!ret)
		{
			if (p->m_fpLogReport)
				p->m_fpLogReport(LOGLEVEL0, "E", TAG, "Initialize RTMP Fail");
				//m_fpLogReport(LOGLEVEL0, "E", TAG, "Initialize RTMP Fail,%s", p->m_strRtmpURL.c_str());
			return;
		}
		if (p->m_fpLogReport)
			p->m_fpLogReport(LOGLEVEL1, "I", TAG,"RTMP initialized, %s", p->m_strRtmpURL.c_str());
		p->m_bRTMPInit = true;
		return;
	}
	
	p->RTMPSendV(keyframe, pFrame, nBytes);
}

bool CStreamSource::RTMPSendV_old(int keyframe, uint8_t *pEncBuffer, int nFrameSize)
{
	if (!m_pSendBufVideo) return false;

	unsigned int size = nFrameSize;
	H264RemoveStarter(pEncBuffer, nFrameSize, size);
	qbox.data				= pEncBuffer;
	qbox.qbox_size			= size;
	qbox.sample_cts			= 0;
	qbox.sample_flags		= SAMPLE_FLAGS_CTS_PRESENT;
	if (keyframe)
		qbox.sample_flags	|= SAMPLE_FLAGS_SYNC_POINT;//SAMPLE_FLAGS_CONFIGURATION_INFO;

	qbox.sample_stream_id	= 4;
	qbox.sample_stream_type	= SAMPLE_TYPE_H264;
	qbox.version			= 1;

	//qbox2flv(&qbox, &flv, pEncBuffer);
	rtmp_flv_write(&qbox, m_pRTMPHandle, &flv, m_pSendBufVideo);

	return true;
}

int CStreamSource::rtmp_flv_write(qboxContext *qbox, RTMP *rtmp, flvContext *flv, uint8_t *buffer)
{
	int pktsize = qbox2flv(qbox, flv, buffer);
	if(pktsize < 0)
		return -1;

//	if (flv && !flv_write(flv, buffer, pktsize))
//		return -1;
	if (rtmp && !RTMP_Write(rtmp, (char *) buffer, pktsize))
		return -1;
	return pktsize;
}

int CStreamSource::qbox2flv(qboxContext *qbox, flvContext *flv, uint8_t *buffer)
{
	int64_t newts = qbox->sample_cts/90; ///< 1000/90000
	int32_t flags_size = 0;
	int size = qbox->qbox_size;
	int flags = 0;
	uint8_t *ptr = buffer;

	ASSERT(MAX_H264_FRAMESIZE > qbox->qbox_size);

	if(!(qbox->sample_flags & (SAMPLE_FLAGS_CTS_PRESENT|SAMPLE_FLAGS_CONFIGURATION_INFO)))
		newts++;

#ifdef EXTRA_DEBUG
	RS_OUTPUT("FLV: pts_in: %u", qbox->sample_cts);
	RS_OUTPUT("FLV: pts_in/90: %ld", newts);
#endif

	if(newts > flv->ts_last + 1000)
	{
		newts = flv->ts_last+1;
#ifdef EXTRA_DEBUG
		RS_OUTPUT("FLV: too large difference (pts_out: %ld, flv->ts: %ld)", newts, flv->ts);
#endif
	}

	if(newts < flv->ts_last) {
		flv->ts += flv->ts_last;
#ifdef EXTRA_DEBUG
		RS_OUTPUT("FLV: Update flv->ts (pts_out: %ld, flv->ts: %ld)", newts, flv->ts);
#endif
	}

	flv->ts_last = newts;
	newts += flv->ts;
#ifdef EXTRA_DEBUG
	RS_OUTPUT("FLV: pts_out: %ld", newts);
#endif

	if (qbox->sample_stream_type == SAMPLE_TYPE_H264) {
		flv_put_byte(&ptr, FLV_TAG_TYPE_VIDEO);

		flags = FLV_CODECID_H264;
		flags_size= 5;

		/* create sps/pps for flv */
		if(qbox->sample_flags & SAMPLE_FLAGS_CONFIGURATION_INFO)
		{
			size = modify_sps_pps(qbox, &buffer[11+flags_size]);
			if(!size)
				return -1;
			qbox->qbox_size=0;
			flags |= FLV_FRAME_KEY;
		}
		else if (qbox->sample_flags & SAMPLE_FLAGS_SYNC_POINT)
		{
			flags |= FLV_FRAME_KEY;
		}
		else
		{
			flags |= FLV_FRAME_INTER;
		}
	}
	else if(qbox->sample_stream_type == SAMPLE_TYPE_AAC)
	{
		flv_put_byte(&ptr, FLV_TAG_TYPE_AUDIO);

		flags = FLV_CODECID_AAC | FLV_SAMPLERATE_44100HZ | FLV_SAMPLESSIZE_16BIT | FLV_STEREO;
		flags_size= 2;
	}
	else
	{
		RS_OUTPUT("unknown qbox payload: 0x%08x", qbox->sample_stream_type);
		return -1;
	}

	flv_put_be24(&ptr, size + flags_size);
	flv_put_be24(&ptr, newts);
	flv_put_byte(&ptr, (newts >> 24) & 0x7F); // timestamps are 32bits _signed_
	flv_put_be24(&ptr, 0);

	flv_put_byte(&ptr, flags);
	/* for aac/avc only */
	flv_put_byte(&ptr, qbox->sample_flags & SAMPLE_FLAGS_CONFIGURATION_INFO ? 0 : 1);
	if (qbox->sample_stream_type == SAMPLE_TYPE_H264)
	{
		flv_put_be24(&ptr, 0);
		if(qbox->sample_flags & SAMPLE_FLAGS_CONFIGURATION_INFO)
			ptr+=size;
	}

	if(qbox->qbox_size)
		flv_put_buff(&ptr, qbox->data, qbox->qbox_size);
	flv_put_be32(&ptr,size+flags_size+11); // previous tag size

#ifdef EXTRA_DEBUG
	//1920 * 1000/90000
	RS_OUTPUT("FLV: sz_or: %u, sz_flv: %u, %c",
		qbox->qbox_size, ptr - buffer,
		(qbox->sample_flags & SAMPLE_FLAGS_SYNC_POINT) &&
		(qbox->sample_stream_type == SAMPLE_TYPE_H264) ? 'S' : ' ');
#endif

	return ptr - buffer;
}

bool CStreamSource::RTMPSendV(int keyframe, uint8_t *pEncBuffer, int nFrameSize)
{
	if (!m_pSendBufVideo) return false;

	if (m_fpLogReport)
		m_fpLogReport(LOGLEVEL5, "I", TAG, "Feed RTMP Video Packets to FMS/Red5.");

	m_pSendBufVideo[ 0]	= 0x17; 
	m_pSendBufVideo[ 1]	= 0x01; 
	m_pSendBufVideo[ 2]	= 0x00; 
	m_pSendBufVideo[ 3]	= 0x00; 
	m_pSendBufVideo[ 4]	= 0x42; 

	uint8_t* szTmp= ((uint8_t*)m_pSendBufVideo) + 5;
	uint8_t* pNal	= pEncBuffer;
	unsigned int nSize;

	nTimes++;
	nFramsInPack++;

	if (keyframe)//there are multi nals in a key frame
	{
		H264RemoveStarterSPSPPS(pNal, nFrameSize, nSize);
		m_vRTMPacketV.m_nBodySize=nSize+9;
		m_pSendBufVideo[ 0]=0x17;

		m_nKeyFrameCount ++;
	}
	else 
	{
		H264RemoveStarter(pNal, nFrameSize, nSize);
		m_vRTMPacketV.m_nBodySize=nSize+9;
		m_pSendBufVideo[ 0]=0x27; 
	}
	put_be32((char *)m_pSendBufVideo+5,nSize); 
	int n = pNal - pEncBuffer;
	memcpy(m_pSendBufVideo + 9,pEncBuffer,nSize); 

//	RS_OUTPUT("RTMP Send Bytes= %d, nFrameSize=%d\n", nSize, nFrameSize);

	RTMP_SendPacket(m_pRTMPHandle,&m_vRTMPacketV,0);

//	SaveFLV(m_vRTMPacketV.m_body, m_vRTMPacketV.m_nBodySize);

	newTick=GetTickCount(); 
	m_vRTMPacketV.m_headerType	= RTMP_PACKET_SIZE_MEDIUM;	//Format
//	m_vRTMPacketV.m_nChannel	= 0x04;						//Chunk Stream ID
	m_vRTMPacketV.m_nTimeStamp	+= newTick-oldTick;			//Time stamp
	m_vRTMPacketV.m_packetType	= RTMP_PACKET_TYPE_VIDEO;	//Type ID

	oldTick=newTick; 

	return true;
}

void CStreamSource::H264RemoveStarter(uint8_t* p264, unsigned int nSize, unsigned int &nOut)
{
	int j = 0, k = nSize;
	uint8_t* pNal = p264;
	bool skip = false;

	//FIXME:assume 4th BYTE & 0x60 == 0 only occurs in the first nal
	for (j = 0; j < k; j ++)
	{
		if (p264[j] == 0 && p264[j+1] == 0 && p264[j+2] == 0 && p264[j+3] == 1)
		{
			if ((p264[j + 4] & 0x60)==0)
			{
				memmove(p264+j, p264+j+4, k-4);
				k -= 4;
				skip = true;
				j --;
			}
			else
			{
				skip = false;
				memmove(p264+j, p264+j+4, k-4);
				k -= 4;
				j --;
			}
		}
		else
		{
			if (skip)
			{
				memmove(p264+j, p264+j+1, k-1);
				k -= 1;
				j --;
			}
		}
	}
	nOut = k;
}

void CStreamSource::H264RemoveStarterSPSPPS(uint8_t* p264, unsigned int nSize, unsigned int &nOut)
{
	int j = 0, k = nSize;
	uint8_t* pNal = p264;
	bool skip = false;

	//FIXME:assume 4th BYTE & 0x60 == 0 only occurs in the first nal
	for (j = 0; j < k; j ++)
	{
		if (p264[j] == 0 && p264[j+1] == 0 && p264[j+2] == 0 && p264[j+3] == 1)
		{
			if (((p264[j + 4] & 0x60)==0) || p264[j + 4] == 0x67 || p264[j + 4] == 0x68)
			{
				memmove(p264+j, p264+j+4, k-4);
				k -= 4;
				skip = true;
				j --;
			}
			else
			{
				skip = false;
				memmove(p264+j, p264+j+4, k-4);
				k -= 4;
				j --;
			}
		}
		else if (p264[j] == 0 && p264[j+1] == 0 && p264[j+2] == 1)
		{
			if (((p264[j + 3] & 0x60)==0) || p264[j + 3] == 0x67 || p264[j + 3] == 0x68)
			{
				memmove(p264+j, p264+j+3, k-3);
				k -= 3;
				skip = true;
				j --;
			}
			else
			{
				skip = false;
				memmove(p264+j, p264+j+3, k-3);
				k -= 3;
				j --;
			}
		}
		else
		{
			if (skip)
			{
				memmove(p264+j, p264+j+1, k-1);
				k -= 1;
				j --;
			}
		}
	}
	nOut = k;
}

/*
void CStreamSource::InputRawVideo(uint8_t *pDecBuffer, int nDecSize, int nImageWidth, int nImageHeight, int nPixelFormat, unsigned long long llTimeStamp,bool bKeyFrame)
{
	if (m_fpLogReport)
		m_fpLogReport(LOGLEVEL5, "I", TAG,"Input video %d x %d , %d\n", nImageWidth, nImageHeight, nDecSize);
	//largest_pts = llTimeStamp;

	if (NULL == m_pEnc_v)
	{
		if (m_fpLogReport)
			m_fpLogReport(LOGLEVEL4, "I", TAG, "Opening Video Encoder Codec...");
		CCodec::Create("libezencoder", &m_pEnc_v);
		if (m_pEnc_v)
		{
			m_pEnc_v->SetVideoEncodeCallback(this, CStreamSource::stcVideoEncodeCallback);
			string strFilter;
			strFilter = m_pEnc_v->GetFilterStr(true, true, "EasyRTMP - RG4 Media Systems","0044C0", 12);
			strFilter = m_pEnc_v->GetFilterStr(true, true, "EasyRTMP - RG4 Media Systems","FFFFFF", 13);
			m_pEnc_v->SetOSDParam(true, true, strFilter.c_str());
			m_pEnc_v->SetLogReport(NULL, CMainService::stcLogReport);
		}
	}

	if (!m_pEnc_v) return;
	if (!m_bCapturing) return;

	int nResult;
	nResult = m_pEnc_v->VideoEncodeFrame(
		pDecBuffer, 
		nDecSize, 
		nPixelFormat, 
		nImageWidth, 
		nImageHeight,
		m_vStreamProfile.iCodecIDV, 
		m_vStreamProfile.wImageWidth,
		m_vStreamProfile.wImageHeight, 
		m_vStreamProfile.wLevel,
		m_vStreamProfile.nBitrate*1000, 
		m_vStreamProfile.wFrameRate, 
		m_vStreamProfile.wGopSize, 
		m_nChannelNo, 
		llTimeStamp);

	if (nResult == CODEC_ERROR)
	{
		if (m_fpLogReport)
			m_fpLogReport(LOGLEVEL4, "I", TAG, "VideoEncodeFrame failed\n");
	}
}
*/

int CStreamSource::RTMPSendA(uint8_t* input, int inputlen)
{
	if (!m_vMediaProfile.bHasAudio) return 0;
	if (inputlen <= 0) return -1;
	if (inputlen > RTMP_MAX_AUDIO_SIZE) return -1;
	if (!m_pRTMPHandle) return -1;
	if (m_nKeyFrameCount <= 0) return -1;

	if (m_fpLogReport)
		m_fpLogReport(LOGLEVEL5, "I", TAG, "Feed RTMP Audio Packets to FMS/Red5.");

	int res = -1;

	m_vRTMPacketA.m_headerType		= RTMP_PACKET_SIZE_LARGE; 
	m_vRTMPacketA.m_headerType		= RTMP_PACKET_SIZE_MEDIUM;	//Format
	m_vRTMPacketA.m_nChannel		= 0x04;						//Chunk Stream ID, 0x04 means Source Channel
	m_vRTMPacketA.m_nTimeStamp		= 0;			//Time stamp
	m_vRTMPacketA.m_packetType		= RTMP_PACKET_TYPE_AUDIO;	//Type ID
	m_vRTMPacketA.m_nInfoField2		= m_pRTMPHandle->m_stream_id; 
	m_vRTMPacketA.m_hasAbsTimestamp = 0; 
	m_vRTMPacketA.m_body			= (char *)m_pAudioSendBuf; 

	//Jacky, send AAC header every 10 i frames
	if (m_nKeyFrameCount % 10 == 0)
	{
		unsigned char hdr[] = {0xAF, 0x00, 0x15, 0x08};
		memcpy(m_pAudioSendBuf,hdr, 4);
		m_vRTMPacketA.m_nBodySize	= 4;
		m_vRTMPacketA.m_body		= m_pAudioSendBuf;

		res = RTMP_SendPacket(m_pRTMPHandle, &m_vRTMPacketA, 0);

		m_nKeyFrameCount++;
	}

	int t = 1;
	
	if (t == 1)
	{
		m_pAudioSendBuf[0]			= 0xAF;
		m_pAudioSendBuf[1]			= 0x01;// Raw AAC frame data in UI8
		memcpy(m_pAudioSendBuf + 2,input, inputlen);
		m_vRTMPacketA.m_nBodySize	= inputlen + 2;
		m_vRTMPacketA.m_body		= m_pAudioSendBuf;

		res = RTMP_SendPacket(m_pRTMPHandle, &m_vRTMPacketA, 0);
	}
	else if (t == 22)
	{
		input += 4;
		inputlen -= 4;

		m_pAudioSendBuf[0]			= 0xAF;
		m_pAudioSendBuf[1]			= 0x01;// Raw AAC frame data in UI8
		m_pAudioSendBuf[2]			= 0x01;// Raw AAC frame data in UI8
		memcpy(m_pAudioSendBuf + 3,input, inputlen);
		m_vRTMPacketA.m_nBodySize	= inputlen + 3;
		m_vRTMPacketA.m_body		= m_pAudioSendBuf;

		res = RTMP_SendPacket(m_pRTMPHandle, &m_vRTMPacketA, 0);
	}
	else if (t == 3)
	{
		input += 7;
		inputlen -= 7;
		memcpy(m_pAudioSendBuf,input, inputlen);
		m_vRTMPacketA.m_nBodySize	= inputlen;
		m_vRTMPacketA.m_body		= m_pAudioSendBuf;

		res = RTMP_SendPacket(m_pRTMPHandle, &m_vRTMPacketA, 0);
	}
	else
	{
		input += 7;
		inputlen -= 7;

		m_pAudioSendBuf[ 0]	= 0xAF;
		m_pAudioSendBuf[ 1]	= 0x01;//0x01
		m_pAudioSendBuf[ 2]	= 0x01;
		m_pAudioSendBuf[ 3]	= 0x2E;
		m_pAudioSendBuf[ 4]	= 0x15;

		put_be32((char *)m_pAudioSendBuf+5, inputlen);
		memcpy(m_pAudioSendBuf + 9,input, inputlen);

		//////////////////////////////////////////////////////////////////////////
		m_vRTMPacketA.m_nChannel		= 0x04;//m_pRTMPHandle->m_stream_id 
		m_vRTMPacketA.m_headerType		= RTMP_PACKET_SIZE_LARGE; 
		m_vRTMPacketA.m_nTimeStamp		= 0; 
		m_vRTMPacketA.m_nInfoField2		= m_pRTMPHandle->m_stream_id; 
		m_vRTMPacketA.m_hasAbsTimestamp = 0; 
		m_vRTMPacketA.m_body			= (char *)m_pAudioSendBuf; 
		//m_vRTMPacketA.m_nTimeStamp	= (time(NULL) * 1000) -  p->timestamp;
		//m_vRTMPacketA.m_body			= pbuf + RTMP_MAX_HEADER_SIZE;
		m_vRTMPacketA.m_packetType		= RTMP_PACKET_TYPE_AUDIO;

		m_vRTMPacketA.m_nBodySize = inputlen + 1;

		res = RTMP_SendPacket(m_pRTMPHandle, &m_vRTMPacketA, 0);
	}

	//SaveFLV(m_vRTMPacketA.m_body, m_vRTMPacketA.m_nBodySize);

	return res;
}

/*
void CStreamSource::InputRawAudio(void* pParam, char* pBuf, int nSize, unsigned long lTimestamp, RAWAUDIOINFO* pFormat)
{
	CStreamSource* p = (CStreamSource*)pParam;
	if (!p->m_vStreamProfile.bOnFlagA) return;

	if (NULL == p->m_pEnc_a)
	{
		if (p->m_vStreamProfile.iCodecIDA == CODEC_ID_ADPCM_G726)
			CCodec::Create("libg726", &p->m_pEnc_a);
		else if (p->m_vStreamProfile.iCodecIDA == CODEC_ID_PCM_ALAW)
			CCodec::Create("libg711a", &p->m_pEnc_a);
		else
			CCodec::Create("libezencoder", &p->m_pEnc_a);
		if (p->m_pEnc_a)
			p->m_pEnc_a->SetAudioEncodeCallback(p, stcAudioEncodeCallback);
	}

	if (!p->m_pEnc_a) return;
	int nResult;
	nResult = p->m_pEnc_a->AudioEncodeFrame(
		(unsigned char*)pBuf, 
		nSize, 
		0,
		pFormat->nChannels, pFormat->nBitsPerSample, pFormat->nSamplePerSec, 
		p->m_vStreamProfile.iCodecIDA, 
		p->m_vStreamProfile.iChannels,
		p->m_vStreamProfile.iBitsPerSample, 
		p->m_vStreamProfile.iSamplePerSec,
		p->m_vStreamProfile.nBitrateA, //m_vStreamProfile.iChannels*m_vStreamProfile.iBitsPerSample*m_vStreamProfile.iSamplePerSec,
		0, 
		lTimestamp);

	if (nResult == CODEC_ERROR)
	{
		printf("Audio EncodeFrame error\n");
	}
}
*/
void CStreamSource::stcAudioEncodeCallback(void* pParam, uint8_t* pFrame, int nBytes, int nCodecID, int nBitsPerSample, int nSampleRate, int nChannel, int nBitRate, int32_t ts)
{
	CBaseVideo* pp = (CBaseVideo*)pParam;
	CStreamSource* p = (CStreamSource*)pp->m_pParam;
	if (!p)
		return;
	else
	{
		p->RTMPSendA(pFrame, nBytes);
	}
}

/*
bool CStreamSource::Save264(uint8_t* pBuf, int nBytes)
{
	FILE* fp = fopen(m_strFile264, "a+b");
	if (fp)
	{
//		uint8_t starter[] = {0, 0, 0, 1};
//		fwrite(starter, 1, 4, fp);
		fwrite(pBuf, sizeof(uint8_t), nBytes, fp);
		fclose(fp);
		return true;
	}
	return false;
}
bool CStreamSource::SaveFLV(char* pBuf, int nBytes)
{
if (!m_bSaveFLV) return false;

FILE* fp = fopen(m_strFileFLV, "a+b");
if (fp)
{
fwrite(pBuf, sizeof(uint8_t), nBytes, fp);
fclose(fp);
return true;
}
return false;
}

*/

bool CStreamSource::Start(string strIP, int nPort, string strUser, string strPass, string strRTSPURL, int nRTSPPort, int nRTPType, MEDIA_PROFILE* pProfile, int nChannel)
{
	m_nKeyFrameCount = 0;

	m_vDeviceInfo.m_nStreamType = STREAM_SOURCE_TYPE_CAPTURE;
	strcpy(m_vDeviceInfo.m_szCameraIP, strIP.c_str());
	m_vDeviceInfo.m_wCameraPort = nPort;
	if (!strUser.empty())
		strcpy(m_vDeviceInfo.m_szAccount, strUser.c_str());
	if (!strPass.empty())
		strcpy(m_vDeviceInfo.m_szPassword, strPass.c_str());
	string strConnParam;
	char temp[512] = {0};
	sprintf(temp, "<RG4><CGI>%s</CGI><Port>%d</Port></RG4><Type>%d</Type>"
		, strRTSPURL.c_str()
		, nRTSPPort
		, nRTPType);
	strConnParam = temp;
	strcpy(m_vDeviceInfo.m_szConnParam, strConnParam.c_str());
	if (pProfile)
		memcpy(&m_vMediaProfile, pProfile, sizeof(STREAMPROFILE));

	m_vDeviceInfo.m_wChannelNumber = nChannel;

	char uri[256];
	sprintf(uri, "rtmp://%s:%d/live/%d_%dx%d", m_vRTMPSvrInfo.szServerIP, m_vRTMPSvrInfo.nServerPort
		, nChannel
		, m_vMediaProfile.vVideoProfile.nWidth
		, m_vMediaProfile.vVideoProfile.nHeight); 
	m_strRtmpURL = "";
	m_strRtmpURL += uri;

#ifdef __USING_IOCP
	if (!m_pVideoThread)
	{
		m_pVideoThread = (CStreamThread*)AfxBeginThread(RUNTIME_CLASS(CStreamThread), THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
		m_pVideoThread->m_pParam		= this;
		m_pVideoThread->m_bAutoDelete	= FALSE;
		m_pVideoThread->ResumeThread();
		return false;
	}

	// Connect Video Channel
	if (m_pVideoThread)
		m_pVideoThread->PostThreadMessage(ID_CONNECT, 0, 0);
#else
	CStreamSource*p = this;
	if (p->m_pSourceInst)
	{
		delete p->m_pSourceInst;
		p->m_pSourceInst = NULL;
	}

	string strLibrary;
	if (m_vDeviceInfo.m_nStreamType == STREAM_SOURCE_TYPE_CAPTURE)
		strLibrary = "libEasyCapture";	//Capture from WebCam(USB Camera)
	else
		strLibrary = "libEasyRTP";		//translate RTSP/RTP stream to RTMP stream
	if (m_vSourceLoader.Create(strLibrary.c_str(), &p->m_pSourceInst))
	{
		p->m_pSourceInst->SetupDeviceInfo(&p->m_vDeviceInfo);
		p->m_pSourceInst->SetMedieProfileCB(this, NULL);
		p->m_pSourceInst->SetupCallback(p, CStreamSource::stcVideoEncodeCallback, CStreamSource::stcAudioEncodeCallback, NULL);
		//p->m_pSourceInst->SetupLogCallback(p, CMainService::stcLogReport);

		//p->m_pSourceInst->SetupRawCallback(p, CStreamSource::OnRawVideoCB, CStreamSource::OnRawAudioCB);
		p->m_pSourceInst->SetupProfile(&p->m_vMediaProfile);
		//string strFilter = g_pRsController->GetFilterStr(g_pRsController->m_vProductInfo.m_bOverlayLogo, g_pRsController->m_vProductInfo.m_bOverlayTime, g_pRsController->m_vProductInfo.m_strLogoText, g_pRsController->m_vProductInfo.m_strFontColor, g_pRsController->m_vProductInfo.m_nFontSize);
		//p->m_pSourceInst->SetupOSDParameters(g_pRsController->m_vProductInfo.m_bOverlayLogo, g_pRsController->m_vProductInfo.m_bOverlayTime, strFilter);
		p->m_pSourceInst->Start();
	}
#endif

	return true;
}

bool CStreamSource::Stop()
{
//	StopRecord();
#ifdef __USING_IOCP
	if (m_pVideoThread)
		m_pVideoThread->PostThreadMessage(ID_DISCONNECT, 0, 0);
#else
	CStreamSource* p = this;
	if (p->m_pSourceInst)
	{
		p->m_pSourceInst->Stop();
		delete p->m_pSourceInst;
		p->m_pSourceInst = NULL;
	}
#endif

	return true;
}

void CStreamSource::SetRTMPSvrINfo(RTMP_SVR_INFO* svr)
{
	memcpy(&m_vRTMPSvrInfo, svr, sizeof(RTMP_SVR_INFO));
}

