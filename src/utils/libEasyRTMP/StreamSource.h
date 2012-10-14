#ifndef _EASY_RTMP_STREAMSOURCE_INCLUDE__
#define _EASY_RTMP_STREAMSOURCE_INCLUDE__

#include <EasyRTMPBase.h>
#include "../../../../RsCommon/libEasySource/BaseSource.h"
#include "../../../../RsCommon/libEasySrcLDR/SourceLoader.h"

#ifdef __cplusplus
extern "C" {
#include <librtmp/rtmp_sys.h>
#include <librtmp/log.h>
#include <librtmp/amf.h>
}
#endif

#include <pthread.h>

#define RTMP_MAX_VIDEO_SIZE 64*1024
#define RTMP_MAX_AUDIO_SIZE 64*1024

#pragma once

class AFX_EXT_CLASS CStreamSource 
{
public:
	CStreamSource();
	virtual ~CStreamSource(void);

public:
	void				SetRTMPSvrINfo(RTMP_SVR_INFO* svr);
	RTMP_SVR_INFO		m_vRTMPSvrInfo;

	CSourceLoader		m_vSourceLoader;

	uint8_t*			m_pSendBufVideoReal;
	uint8_t*			m_pSendBufVideo;
	char*				m_pAudioSendBufReal;
	char*				m_pAudioSendBuf;
/*
	uint64_t			largest_pts;
	bool				m_bSave264;
	CString				m_strFile264;
	bool				Save264(uint8_t* pBuf, int nBytes);
	CString				m_strFileFLV;
	bool				m_bSaveFLV;
	bool				SaveFLV(char* pBuf, int nBytes);
*/

	//Encode
	//STREAMPROFILE		m_vStreamProfile;
	MEDIA_PROFILE		m_vMediaProfile;
	CBaseVideo*			m_pSourceInst;
	DEVICE_INFO			m_vDeviceInfo;
	bool				m_bCapturing;

	//////////////////////////////////////////////////////////////////////////
	//Video
	bool				Start(string strIP, int nPort, string strUser, string strPass, string strRTSPURL, int nRTSPPort, int nRTPType, MEDIA_PROFILE* pProfile, int nChannel);
	bool				Stop();
	static void			stcVideoEncodeCallback(void* pParam, uint8_t* pFrame, int nBytes, int nCodecID, int vDuringAlarm, int16_t vFrameType, int bStretchMode, int nImageWidth, int nImageHeight, int32_t ts);
	static void			stcAudioEncodeCallback(void* pParam, uint8_t* pFrame, int nBytes, int nCodecID, int nBitsPerSample, int nSampleRate, int nChannel, int nBitRate, int32_t ts);

	void				SetLog(EASYLOG log);
	EASYLOG				m_fpLogReport;

	RTMP *				m_pRTMPHandle; 
	RTMPPacket			m_vRTMPacketV;
	RTMPPacket			m_vRTMPacketA;
	bool				m_bRTMPInit;
	unsigned int		nTimes;
	unsigned int		oldTick;
	unsigned int		newTick;
	int					nFramsInPack;
	string				m_strRtmpURL;
	int					m_nKeyFrameCount;

	//Send
	bool				RTMPInit(const char* uri, uint8_t* pBuffer, int nBytes);
	bool				RTMPSendV(int keyframe, uint8_t *pEncBuffer, int nFrameSize);
	bool				RTMPSendV_old(int keyframe, uint8_t *pEncBuffer, int nFrameSize);
	int					RTMPSendA(uint8_t* buf, int size);
	void				RTMPRelease();

private:
	void				H264RemoveStarter(uint8_t* p264, unsigned int nSize, unsigned int& nOut);
	void				H264RemoveStarterSPSPPS(uint8_t* p264, unsigned int nSize, unsigned int &nOut);


#define QBOX_HDR_SIZE (6*4)

	enum {
		SAMPLE_TYPE_AAC = 0x1,
		SAMPLE_TYPE_H264,
		SAMPLE_TYPE_PCM,
		SAMPLE_TYPE_DEBUG,
		SAMPLE_TYPE_H264_SLICE,
		SAMPLE_TYPE_QMA,
		SAMPLE_TYPE_VIN_STATS_GLOBAL,
		SAMPLE_TYPE_VIN_STATS_MB,
		SAMPLE_TYPE_Q711,
		SAMPLE_TYPE_Q728,
		SAMPLE_TYPE_MAX,
	};
#define SAMPLE_FLAGS_CONFIGURATION_INFO 0x01
#define SAMPLE_FLAGS_CTS_PRESENT 0x02
#define SAMPLE_FLAGS_SYNC_POINT 0x04
#define SAMPLE_FLAGS_DISPOSABLE 0x08
#define SAMPLE_FLAGS_MUTE 0x10
#define SAMPLE_FLAGS_BASE_CTS_INCREMENT 0x20
#define SAMPLE_FLAGS_META_INFO 0x40
#define SAMPLE_FLAGS_END_OF_SEQUENCE 0x80
#define SAMPLE_FLAGS_END_OF_STREAM 0x100
#define SAMPLE_FLAGS_PADDING_MASK 0xFF000000
	typedef struct qboxContext {
		uint32_t qbox_size;     ///< will be decreased
		uint32_t boxflags;
		uint8_t  version;
		uint16_t sample_stream_type;
		uint16_t sample_stream_id;
		uint32_t sample_flags;
		uint32_t sample_cts;
		uint8_t  *data;
	} qboxContext;

	/* offsets for packed values */
#define FLV_AUDIO_SAMPLESSIZE_OFFSET 1
#define FLV_AUDIO_SAMPLERATE_OFFSET  2
#define FLV_AUDIO_CODECID_OFFSET     4

#define FLV_VIDEO_FRAMETYPE_OFFSET   4

	/* bitmasks to isolate specific values */
#define FLV_AUDIO_CHANNEL_MASK    0x01
#define FLV_AUDIO_SAMPLESIZE_MASK 0x02
#define FLV_AUDIO_SAMPLERATE_MASK 0x0c
#define FLV_AUDIO_CODECID_MASK    0xf0

#define FLV_VIDEO_CODECID_MASK    0x0f
#define FLV_VIDEO_FRAMETYPE_MASK  0xf0

#define AMF_END_OF_OBJECT         0x09

	enum {
		FLV_HEADER_FLAG_HASVIDEO = 1,
		FLV_HEADER_FLAG_HASAUDIO = 4,
	};

	enum {
		FLV_TAG_TYPE_AUDIO = 0x08,
		FLV_TAG_TYPE_VIDEO = 0x09,
		FLV_TAG_TYPE_META  = 0x12,
	};

	enum {
		FLV_MONO   = 0,
		FLV_STEREO = 1,
	};

	enum {
		FLV_SAMPLESSIZE_8BIT  = 0,
		FLV_SAMPLESSIZE_16BIT = 1 << FLV_AUDIO_SAMPLESSIZE_OFFSET,
	};

	enum {
		FLV_SAMPLERATE_SPECIAL = 0, /**< signifies 5512Hz and 8000Hz in the case of NELLYMOSER */
		FLV_SAMPLERATE_11025HZ = 1 << FLV_AUDIO_SAMPLERATE_OFFSET,
		FLV_SAMPLERATE_22050HZ = 2 << FLV_AUDIO_SAMPLERATE_OFFSET,
		FLV_SAMPLERATE_44100HZ = 3 << FLV_AUDIO_SAMPLERATE_OFFSET,
	};

	enum {
		FLV_CODECID_PCM                  = 0,
		FLV_CODECID_ADPCM                = 1 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_MP3                  = 2 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_PCM_LE               = 3 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_NELLYMOSER_8KHZ_MONO = 5 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_NELLYMOSER           = 6 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_AAC                  = 10<< FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_SPEEX                = 11<< FLV_AUDIO_CODECID_OFFSET,
	};

	enum {
		FLV_CODECID_H263    = 2,
		FLV_CODECID_SCREEN  = 3,
		FLV_CODECID_VP6     = 4,
		FLV_CODECID_VP6A    = 5,
		FLV_CODECID_SCREEN2 = 6,
		FLV_CODECID_H264    = 7,
	};

	enum {
		FLV_FRAME_KEY        = 1 << FLV_VIDEO_FRAMETYPE_OFFSET,
		FLV_FRAME_INTER      = 2 << FLV_VIDEO_FRAMETYPE_OFFSET,
		FLV_FRAME_DISP_INTER = 3 << FLV_VIDEO_FRAMETYPE_OFFSET,
	};
	typedef struct flvContext {
		int64_t duration_offset;
		int64_t filesize_offset;
		time_t start_time;
		FILE *fh;
		int64_t ts;
		int64_t ts_last;
	} flvContext;

	flvContext flv;
	qboxContext qbox;
	int qbox2flv(qboxContext *qbox, flvContext *flv, uint8_t *buffer);
	int modify_sps_pps(qboxContext *qbox, uint8_t* buf);
	int rtmp_flv_write(qboxContext *qbox, RTMP *rtmp, flvContext *flv, uint8_t *buffer);

#define MAX_H264_FRAMESIZE 131072
	//from http://code.google.com/p/crusher264/
	/*
#define QBOX_HDR_SIZE (6*4)

	enum {
		SAMPLE_TYPE_AAC = 0x1,
		SAMPLE_TYPE_H264,
		SAMPLE_TYPE_PCM,
		SAMPLE_TYPE_DEBUG,
		SAMPLE_TYPE_H264_SLICE,
		SAMPLE_TYPE_QMA,
		SAMPLE_TYPE_VIN_STATS_GLOBAL,
		SAMPLE_TYPE_VIN_STATS_MB,
		SAMPLE_TYPE_Q711,
		SAMPLE_TYPE_Q728,
		SAMPLE_TYPE_MAX,
	};
#define SAMPLE_FLAGS_CONFIGURATION_INFO 0x01
#define SAMPLE_FLAGS_CTS_PRESENT 0x02
#define SAMPLE_FLAGS_SYNC_POINT 0x04
#define SAMPLE_FLAGS_DISPOSABLE 0x08
#define SAMPLE_FLAGS_MUTE 0x10
#define SAMPLE_FLAGS_BASE_CTS_INCREMENT 0x20
#define SAMPLE_FLAGS_META_INFO 0x40
#define SAMPLE_FLAGS_END_OF_SEQUENCE 0x80
#define SAMPLE_FLAGS_END_OF_STREAM 0x100
#define SAMPLE_FLAGS_PADDING_MASK 0xFF000000
	typedef struct qboxContext {
		uint32_t qbox_size;     ///< will be decreased
		uint32_t boxflags;
		uint8_t  version;
		uint16_t sample_stream_type;
		uint16_t sample_stream_id;
		uint32_t sample_flags;
		uint32_t sample_cts;
		uint8_t  *data;
	} qboxContext;

	// offsets for packed values
#define FLV_AUDIO_SAMPLESSIZE_OFFSET 1
#define FLV_AUDIO_SAMPLERATE_OFFSET  2
#define FLV_AUDIO_CODECID_OFFSET     4

#define FLV_VIDEO_FRAMETYPE_OFFSET   4

	// bitmasks to isolate specific values
#define FLV_AUDIO_CHANNEL_MASK    0x01
#define FLV_AUDIO_SAMPLESIZE_MASK 0x02
#define FLV_AUDIO_SAMPLERATE_MASK 0x0c
#define FLV_AUDIO_CODECID_MASK    0xf0

#define FLV_VIDEO_CODECID_MASK    0x0f
#define FLV_VIDEO_FRAMETYPE_MASK  0xf0

#define AMF_END_OF_OBJECT         0x09

	enum {
		FLV_HEADER_FLAG_HASVIDEO = 1,
		FLV_HEADER_FLAG_HASAUDIO = 4,
	};

	enum {
		FLV_TAG_TYPE_AUDIO = 0x08,
		FLV_TAG_TYPE_VIDEO = 0x09,
		FLV_TAG_TYPE_META  = 0x12,
	};

	enum {
		FLV_MONO   = 0,
		FLV_STEREO = 1,
	};

	enum {
		FLV_SAMPLESSIZE_8BIT  = 0,
		FLV_SAMPLESSIZE_16BIT = 1 << FLV_AUDIO_SAMPLESSIZE_OFFSET,
	};

	enum {
		FLV_SAMPLERATE_SPECIAL = 0, //< signifies 5512Hz and 8000Hz in the case of NELLYMOSER
		FLV_SAMPLERATE_11025HZ = 1 << FLV_AUDIO_SAMPLERATE_OFFSET,
		FLV_SAMPLERATE_22050HZ = 2 << FLV_AUDIO_SAMPLERATE_OFFSET,
		FLV_SAMPLERATE_44100HZ = 3 << FLV_AUDIO_SAMPLERATE_OFFSET,
	};

	enum {
		FLV_CODECID_PCM                  = 0,
		FLV_CODECID_ADPCM                = 1 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_MP3                  = 2 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_PCM_LE               = 3 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_NELLYMOSER_8KHZ_MONO = 5 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_NELLYMOSER           = 6 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_AAC                  = 10<< FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_SPEEX                = 11<< FLV_AUDIO_CODECID_OFFSET,
	};

	enum {
		FLV_CODECID_H263    = 2,
		FLV_CODECID_SCREEN  = 3,
		FLV_CODECID_VP6     = 4,
		FLV_CODECID_VP6A    = 5,
		FLV_CODECID_SCREEN2 = 6,
		FLV_CODECID_H264    = 7,
	};

	enum {
		FLV_FRAME_KEY        = 1 << FLV_VIDEO_FRAMETYPE_OFFSET,
		FLV_FRAME_INTER      = 2 << FLV_VIDEO_FRAMETYPE_OFFSET,
		FLV_FRAME_DISP_INTER = 3 << FLV_VIDEO_FRAMETYPE_OFFSET,
	};
	typedef struct flvContext {
		int64_t duration_offset;
		int64_t filesize_offset;
		time_t start_time;
		FILE *fh;
		int64_t ts;
		int64_t ts_last;
	} flvContext;

#define QBOX_MAKE_STARTCODES 0x01
#define QBOX_STRICT_NALS 0x02

	void*			crusher;
	flvContext flv;
	qboxContext qbox;
	int				modify_sps_pps(qboxContext *qbox, uint8_t *data);
	int				rtmp_flv_write(qboxContext *qbox, RTMP *rtmp, flvContext *flv, uint8_t *buffer, int pktsize);
	int				flv_hdr(void*crusher, flvContext *flv, uint8_t *buffer);
	void			flv_open(flvContext *flv, const char *filename);
	int				flv_write(flvContext *flv, uint8_t *buffer, int size);
	int				qbox_parse(qboxContext *qbox, uint8_t *input_data, int data_size, int flags, bool bKeyFrame);
	int				qbox2flv(qboxContext *qbox, flvContext *flv, uint8_t *buffer);
	void			flv_close(flvContext *flv);
	*/
};


#endif
