
#include "stdafx.h"

#include <math.h>
#include "EasyFMS.h"
#include "EasyFMSDlg.h"
#include "MainService.h"
#include "../../../RsCommon/libEasyCapture/RsCameraDS.h"

extern CEasyRTMPDlg* g_pDlg;

//#ifdef _DEBUG
//#pragma comment(lib, "libEasyRTMPd.lib")
//#else
//#pragma comment(lib, "libEasyRTMP.lib")
//#endif
#pragma comment(lib, "libEasyRTMP.lib")

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swscale.lib")

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "pthreadVC2.lib")

CMainService* g_pMainService = NULL;

static int av_log_level = AV_LOG_DEBUG;

void ddddd(void* ptr, int level, const char* fmt, va_list vl)
{
#ifdef _DEBUG
	static int print_prefix=1;
	static int count;
	static char line[1024], prev[1024];
	AVClass* avc= ptr ? *(AVClass**)ptr : NULL;
	if(level >= av_log_level)
		return;
#undef fprintf
	if(print_prefix && avc) {
		_snprintf_s(line, sizeof(line),sizeof(line), "[%s @ %p]", avc->item_name(ptr), ptr);
	}else
		line[0]=0;

	_vsnprintf_s(line + strlen(line), sizeof(line) - strlen(line), sizeof(line)-strlen(line), fmt, vl);

	print_prefix= line[strlen(line)-1] == '\n';
	if(print_prefix && !strcmp(line, prev)){
		count++;
		return;
	}
	if(count>0)
	{
		fprintf(stderr, "    Last message repeated %d times\n", count);
		count=0;
	}
	//    fputs(line, stderr);
	strcat_s(line, sizeof(line), "\n");
	OutputDebugStr(line);
	strcpy_s(prev, sizeof(prev), line);
#endif
}

CMainService::CMainService()
{
	strcpy(TAG, "Main");
	g_pMainService	= this;
	m_bSystemExit	= false;
	CoInitialize(NULL); 

//	av_log_set_callback(ddddd);

	pthread_mutex_init(&m_vLogArrayMutex, NULL);

}

CMainService::~CMainService()
{
	CMainService::stcLogReport(LOGLEVEL0, "I", TAG, "System terminated mannually!");

	CleanupSockets();

	list<CStreamSource*>::iterator iter = m_lstSource.begin();
	while (iter != m_lstSource.end())
	{
		CStreamSource* p = *iter;
		iter ++;
		m_lstSource.remove(p);
		if (p)
		{
			p->Stop();
			delete p;
			p = NULL;
		}
	}
	m_lstSource.clear();

	CoUninitialize();

	m_bSystemExit = true;

	//	pthread_join(m_tLogThread, NULL);
	pthread_mutex_destroy(&m_vLogArrayMutex);
}

string CMainService::GetAppPath()
{
	char path[255];
	string strPath;
#ifdef WIN32
	::GetModuleFileName(NULL, path, MAX_PATH);
#else
	getcwd(path, sizeof(path));
#endif
	strPath = (string)path;
	size_t pos = strPath.rfind(PATH_SEPERATOR);
	if (pos == -1)
		return strPath;
	strPath = strPath.substr(0, pos);


	return strPath;

	//	//////////////////////////////////////////////////////////////////////////
	//	//boost::filesystem of boost 1.41.0 seems can not support fedora core 6
	//	//so i hard code path value to /home/jacky/EzStreamSvr/bin temp
	//#ifdef WIN32
	//	//对当前的目录的操作
	//	boost::filesystem::path strAppPath =boost::filesystem::current_path(); //取得当前目录
	//#ifdef _DEBUG
	//	strAppPath = strAppPath.parent_path();
	//	strAppPath = strAppPath.parent_path();
	//	strAppPath = strAppPath.parent_path();
	//#else
	//	strAppPath = strAppPath.parent_path();
	//#endif
	//	return strAppPath.directory_string();
	//
	//#else
	//	bf::path strAppPath =bf::current_path();
	//	strAppPath = strAppPath.parent_path();
	//	RS_OUTPUT("Current Path=%s\n", strAppPath.directory_string());
	//
	//	return "/home/jacky/EzStreamSvr";
	//#endif
}

bool CMainService::InitService()
{
	bool ret = false;

	if (m_vProductInfo.m_szLogPath.empty())
	{
		m_vProductInfo.m_szLogPath = GetAppPath();
		m_vProductInfo.m_szLogPath += PATH_SEPERATOR;
		m_vProductInfo.m_szLogPath += "log";

		CreateDirectory(m_vProductInfo.m_szLogPath.c_str(), NULL);
	}
	m_vProductInfo.m_nLogLevel = 3;

	//pthread_create(&m_tLogThread, NULL, ThreadLogData, this);

	ret = InitSockets();
	if (!ret)
	{
		CMainService::stcLogReport(LOGLEVEL0, "E", TAG, "Main Service Startup Failed Due To Init Sockets Failed!");
		return false;
	}

	memset(&m_vRTMPProfile, 0, sizeof(m_vRTMPProfile));

	m_vProductInfo.m_nLogLevel						= 2;
	m_vRTMPProfile.bHasAudio						= true;
	m_vRTMPProfile.bHasVideo						= true;

	m_vRTMPProfile.vAudioProfile.nBitsPerSample		= 16;
	m_vRTMPProfile.vAudioProfile.nBytesPerCapture	= 640;
	m_vRTMPProfile.vAudioProfile.nChannels			= 2;
	m_vRTMPProfile.vAudioProfile.nSampleRate		= 44100;
	m_vRTMPProfile.vAudioProfile.nCodecID			= CODEC_ID_AAC;//CODEC_ID_PCM_ALAW=0x10007;
	m_vRTMPProfile.vAudioProfile.dFPS				= 50.0;

	m_vRTMPProfile.vVideoProfile.nCodecID			= CODEC_ID_H264;//13;//CODEC_ID_MPEG4;//28;//CODEC_ID_H264;
	m_vRTMPProfile.vVideoProfile.nBitRate			= 512;
	m_vRTMPProfile.vVideoProfile.dFPS				= 30.0;
	m_vRTMPProfile.vVideoProfile.nGOP				= 3*m_vRTMPProfile.vVideoProfile.dFPS;
	m_vRTMPProfile.vVideoProfile.nWidth				= 320;
	m_vRTMPProfile.vVideoProfile.nHeight			= 240;
	m_vRTMPProfile.vVideoProfile.nLevel				= 21;//12;

	CMainService::stcLogReport(LOGLEVEL0, "I", TAG, "Main Service Startup Successfully!");

	return ret;
}

bool CMainService::InitSockets()
{
#ifdef WIN32 
	WORD version; 
	WSADATA wsaData; 
	version = MAKEWORD(1, 1); 
	return (WSAStartup(version, &wsaData) == 0); 
#else 
	return true; 
#endif 
}

void CMainService::CleanupSockets()
{
#ifdef WIN32
	WSACleanup();
#endif
}

bool CMainService::StopCapture(int nChannel)
{
	list<CStreamSource*>::iterator iter = m_lstSource.begin();
	while (iter != m_lstSource.end())
	{
		CStreamSource* p = *iter;
		iter ++;
		if (p->m_vDeviceInfo.m_wChannelNumber == nChannel)
		{
			CMainService::stcLogReport(LOGLEVEL0, "I", TAG, "Stop Capture Channel %d!", nChannel);

			m_lstSource.remove(p);
			CMainService::stcLogReport(LOGLEVEL2, "I", TAG, "Stop Capture Channel %d! 2", nChannel);
			p->Stop();
			CMainService::stcLogReport(LOGLEVEL2, "I", TAG, "Stop Capture Channel %d! 3", nChannel);
			delete p;
			p = NULL;
			CMainService::stcLogReport(LOGLEVEL2, "I", TAG, "Stop Capture Channel %d! 4", nChannel);
			break;
		}
	}

	m_nTimeCount = 0;

	CMainService::stcLogReport(LOGLEVEL0, "I", TAG, "Stop Capture Channel %d! 5", nChannel);

	return true;
}

void CMainService::stcLogReport(int nLogLevel, const char* szLogType, const char* szProgramID, const char* pszFormat, ...)
{
	if (!g_pMainService) return;
	//if (!g_pMainService->m_bIAmWorking) return;
	if (nLogLevel > g_pMainService->m_vProductInfo.m_nLogLevel)
		return;

	char pBuffer[LOG_BUFFER_SIZE];
	memset(pBuffer, 0, LOG_BUFFER_SIZE);
	va_list arglist;
	va_start(arglist, pszFormat);
	//_vsnprintf(pBuffer, LOG_BUFFER_SIZE-1, pszFormat, arglist);
	vsnprintf(pBuffer, LOG_BUFFER_SIZE-1, pszFormat, arglist);
	va_end(arglist);

	using namespace std; // For time_t, time and ctime;
	time_t now = time(0);
	struct tm* tm = localtime(&now);

	char log[LOG_BUFFER_SIZE+1024];
	memset(log, 0, LOG_BUFFER_SIZE+1024);
	sprintf(log,"%04d-%02d-%02d %02d:%02d:%02d  %s  %s  %s\r\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, szLogType, szProgramID, pBuffer);

	g_pDlg->ShowLog(log);
	return;
}

void* CMainService::ThreadLogData(void* pParam)
{
	CMainService* pMainService = (CMainService*)pParam;

	if (pMainService)
		pMainService->LogDataFunction();

	return 0;
}

void CMainService::LogDataFunction()
{
	UINT nTimerCount = 0;

	int nSleepTime;
	if (m_vProductInfo.m_nLogLevel <= 2)
		nSleepTime = 1000;//3000;
	else if (m_vProductInfo.m_nLogLevel >= 3 && m_vProductInfo.m_nLogLevel <= 4)
		nSleepTime = 300;//1000;
	else
		nSleepTime = 100;//300;

	while(!m_bSystemExit)
	{
		if (!WriteLogToFile())
			stcLogReport(LOGLEVEL0, "E", TAG, "Write LogFile Error");
		Sleep(nSleepTime);
	}

	WriteLogToFile();
}

bool CMainService::WriteLogToFile()
{
	pthread_mutex_lock(&m_vLogArrayMutex);
	list<string*>::size_type nSize = m_lstLogArray.size();
	pthread_mutex_unlock(&m_vLogArrayMutex);
	if ( nSize <= 0)
		return true;

	bool ret = true;

#ifdef WIN32
	// Create Location Folder
	CString szLogPath = g_pMainService->m_vProductInfo.m_szLogPath.c_str();

	BOOL hr = ::SHCreateDirectoryEx(NULL, szLogPath, NULL);
	if (hr != ERROR_ALREADY_EXISTS && hr != ERROR_SUCCESS)
	{
		return false;
	}

	// Open Log File
	CTime ctDateTime = CTime::GetCurrentTime();
	CString szLogName;
	if (m_vProductInfo.m_nLogLevel <= 2)	// Log file Per day
		szLogName.Format("%s\\EasyRTMP_%s.txt", szLogPath, ctDateTime.Format("%Y-%m-%d"));
	else if (m_vProductInfo.m_nLogLevel >= 3 && m_vProductInfo.m_nLogLevel <= 4)	// Log file Per hour
		szLogName.Format("%s\\EasyRTMP_%s.txt", szLogPath, ctDateTime.Format("%Y-%m-%d_%H"));
	else	// Log file Per 10 minutes
		szLogName.Format("%s\\EasyRTMP_%s_%.2d.txt", szLogPath, ctDateTime.Format("%Y-%m-%d_%H"), ctDateTime.GetMinute() / 10);

	BOOL bFileException = FALSE;

	CFile vLogFile;
	if (vLogFile.Open(szLogName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
	{
		pthread_mutex_lock(&m_vLogArrayMutex);
		list<string*>::iterator iter = m_lstLogArray.begin();
		while (iter != m_lstLogArray.end())
		{
			string* szLogMsg = *iter;
			iter ++;
			m_lstLogArray.remove(szLogMsg);

			try
			{
				// Write to Log File
				if (!bFileException)
				{
					vLogFile.SeekToEnd();
					vLogFile.Write(szLogMsg->c_str(), szLogMsg->length());
				}
				//CEasyRTMPDlg* pDlg = (CEasyRTMPDlg*)AfxGetApp()->GetMainWnd();
				
				if (g_pDlg->m_lstLog.AddString(szLogMsg->c_str()) == LB_ERRSPACE)
				{
					g_pDlg->m_lstLog.ResetContent();
					g_pDlg->m_lstLog.AddString(szLogMsg->c_str());
				}

				int nCount = g_pDlg->m_lstLog.GetCount();
				if (nCount > 200) g_pDlg->m_lstLog.DeleteString(0);
				nCount = g_pDlg->m_lstLog.GetCount();
				if (nCount > 0) g_pDlg->m_lstLog.SetCurSel(nCount-1);
			}
			catch (CFileException* pEx)
			{
				TCHAR   szCause[255];
				pEx->GetErrorMessage(szCause, 255);
				pEx->Delete();

				if (!bFileException)
				{
					bFileException = TRUE;
					m_nLogFailureCount ++;
				}

				if (m_nLogFailureCount == 1)
				{
					stcLogReport(LOGLEVEL3, "E", TAG, "Write LogFile Error, Description : %s", (const char*)szCause);
				}
				ret = false;
			}
			catch (...)
			{
				if (!bFileException)
				{
					bFileException = TRUE;
					m_nLogFailureCount ++;
				}

				if (m_nLogFailureCount == 1)
				{
					stcLogReport(LOGLEVEL3, "E", TAG, "Write LogFile Error");
				}
				ret = false;
			}
			delete szLogMsg;
			szLogMsg = NULL;
		}
		pthread_mutex_unlock(&m_vLogArrayMutex);

		vLogFile.Close();
	}
	else
		return true;

#endif

	return ret;
}

bool CMainService::GotoURL(string url, int showcmd)
{
#ifdef WIN32
	// First try ShellExecute()
	HINSTANCE result = ShellExecute(NULL, _T("open"), url.c_str(), NULL,NULL, showcmd);

	//	TCHAR key[MAX_PATH + MAX_PATH];
	// First try ShellExecute()
	//// If it failed, get the .htm regkey and lookup the program
	//if ((UINT)result <= HINSTANCE_ERROR) {
	//	if (GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) {
	//		lstrcat(key, _T("\\shell\\open\\command"));
	//		if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
	//			TCHAR *pos;
	//			pos = _tcsstr(key, _T("\"%1\""));
	//			if (pos == NULL) {                     // No quotes found
	//				pos = _tcsstr(key, _T("%1"));      // Check for %1, without quotes 
	//				if (pos == NULL)                   // No parameter at all...
	//					pos = key+lstrlen(key)-1;
	//				else
	//					*pos = '\0';                   // Remove the parameter
	//			}
	//			else
	//				*pos = '\0';                       // Remove the parameter
	//			lstrcat(pos, _T(" "));
	//			lstrcat(pos, url);
	//			USES_CONVERSION;
	//			result = (HINSTANCE) WinExec(T2A(key),showcmd);
	//		}
	//	}
	//}

	return result;
#endif
}

bool CMainService::GetNetTime(SYSTEMTIME& vTime)
{
	//时间同步
	CSocket sockClient;
	TIME_ZONE_INFORMATION tzinfo;
	DWORD dwStandardDaylight;
	long bias,sminute,shour;
	vector <string> vi;
	string strServer;
	BOOL ret;
	//将时间服务器放在一个VECTOR中。
	strServer = "time.nist.gov";
	vi.push_back(strServer); 
	strServer = "time-a.nist.gov";
	vi.push_back(strServer); 
	strServer = "time-b.nist.gov";
	vi.push_back(strServer); 
	strServer = "time-nw.nist.gov";
	vi.push_back(strServer); 
	strServer = "nist1.nyc.certifiedtime.com";
	vi.push_back(strServer); 
	strServer = "time-b.nist.gov";
	vi.push_back(strServer); 

	//初始化CSocket
	AfxSocketInit();
	sockClient.Create();   //创建socket

	dwStandardDaylight = GetTimeZoneInformation(&tzinfo); //获取时区与UTC的时间差 应该返回-8
	bias = tzinfo.Bias;
	if (dwStandardDaylight == TIME_ZONE_ID_INVALID) //函数执行失败
	{
		return false;
	}
	if (dwStandardDaylight == TIME_ZONE_ID_STANDARD) //标准时间有效
		bias += tzinfo.StandardBias;

	if (dwStandardDaylight == TIME_ZONE_ID_DAYLIGHT) //夏令时间
		bias += tzinfo.DaylightBias;

	shour	= bias/60;
	sminute = fmod((float)bias,(float)60.00);

	//循环判断服务器是否连接成功
	for (int n=0; n < vi.size(); ++ n)
	{
		ret = sockClient.Connect(vi[n].c_str(),13);
		if (1 == ret)
			break;   
	}

	if (!ret)
		return false;

	unsigned char nTime[1024];   //临时接收数据要求足够的大
	memset(nTime, 0, 1024);

	int nBytes = sockClient.Receive(nTime, sizeof(nTime)); //接收服务器发送来得的数据
	sockClient.Close();    //关闭socket
	if (nBytes < 12)
		return false;

	CString strTime;
	strTime.Format("%s",nTime);
	if (strTime.IsEmpty())
		return false;

	int first	= strTime.Find("-");
	int second	= strTime.Find("-",first+1);

	int tfirst	= strTime.Find(":");
	int tsecond = strTime.Find(":",tfirst+1);

	int hyear	= 2000+atoi(strTime.Mid(first-2,2));
	int hmonth	= atoi(strTime.Mid(first+1,2));
	int hday	= atoi(strTime.Mid(second+1,2));
	int hhour	= atoi(strTime.Mid(tfirst-2,2))-shour;
	int hminute	= atoi(strTime.Mid(tfirst+1,2))-sminute;
	int hsecond	= atoi(strTime.Mid(tsecond+1,2));

	TRACE("Current Time:%04d-%02d-%02d %02d:%02d:%02d\n", hyear, hmonth, hday, hhour, hminute, hsecond);

	CTime temptime(hyear,hmonth,hday,hhour,hminute,hsecond);

	vTime.wYear			= temptime.GetYear();
	vTime.wMonth		= temptime.GetMonth();
	vTime.wDay			= temptime.GetDay();
	vTime.wHour			= temptime.GetHour();
	vTime.wMinute		= temptime.GetMinute();
	vTime.wSecond		= temptime.GetSecond();
	vTime.wMilliseconds = 0;

	/*
	//设置系统时间
	if(0 != ::SetLocalTime(&systm))
	{
	TRACE("本地时间设置成功！！！");
	}
	else
	{
	TRACE("本地时间设置失败！！！");
	}
	*/
	return true;
}

void CMainService::Timer_Main()
{
	Timer_CheckLicense();
}

void CMainService::Timer_CheckLicense()
{
	m_nTimeCount ++;

	SYSTEMTIME vNow;
	::GetSystemTime(&vNow);
	if (vNow.wMinute == 5)
	{
		if (GetNetTime(vNow))
		{
			if (vNow.wYear > 2011)
				m_nTimeCount = EASYRTMP_MAX_DEMO_TIME;
		}
	}
}

int CMainService::DS_CameraCount()
{
	int count = 0;
	CoInitialize(NULL);

	// enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEm, 0);
	if (hr != NOERROR) 
	{
		return count;
	}

	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
	{
		count ++;
	}

	pCreateDevEnum	= NULL;
	pEm				= NULL;

	return count;
}

int CMainService::DS_CameraName(int nCamID, char* sName, int nBufferSize)
{
	int count = 0;
	CoInitialize(NULL);

	// enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR)
		return 0;
	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
	{
		if (count == nCamID)
		{
			IPropertyBag *pBag=0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr))
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL); //还有其他属性,像描述信息等等...
				if(hr == NOERROR)
				{
					//获取设备名称			
					WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1,sName, nBufferSize ,"",NULL);

					SysFreeString(var.bstrVal);				
				}
				pBag->Release();
			}
			pM->Release();

			break;
		}
		count++;
	}

	pCreateDevEnum = NULL;
	pEm = NULL;

	return 1;
}

bool CMainService::StartCapture(int nChannel, bool bEnableAudio
								, const char* strDeviceName, const char* strMixerLine
								, STREAMPROFILE* pProfile)
{
	bool ret = false;

	int cam_count;

	//仅仅获取摄像头数目
	cam_count = CMainService::DS_CameraCount();

	//获取所有摄像头的名称
	for(int i=0; i < cam_count; i++)
	{
		char camera_name[1024];  
		int retval = CMainService::DS_CameraName(i, camera_name, sizeof(camera_name) );

		if (retval > 0)
			TRACE("Camera #%d's Name is '%s'.\n", i, camera_name);
		else
			TRACE("Can not get Camera #%d's name.\n", i);
	}
	if (nChannel >= cam_count)
	{
		CMainService::stcLogReport(LOGLEVEL0, "E", TAG, "You Dont Have Camera %d!", nChannel);
		return false;
	}

	StopCapture(nChannel);

	CMainService::stcLogReport(LOGLEVEL0, "I", TAG, "Start Capture Channel %d!", nChannel);

	CStreamSource* p = new CStreamSource();
	p->SetLog(CMainService::stcLogReport);
	p->SetRTMPSvrINfo(&m_vRTMPSvrInfo);
	memcpy(&(p->m_vMediaProfile), &m_vRTMPProfile, sizeof(p->m_vMediaProfile));
	p->m_vMediaProfile.bHasAudio = bEnableAudio;
	//Server
//	strcpy(p->m_vStreamProfile.szServerIP, pProfile->szServerIP);
//	p->m_vStreamProfile.nServerPort		= pProfile->nServerPort;
	/*
	//video
	p->m_vMediaProfile.vVideoProfile.nWidth	= pProfile->wImageWidth;
	p->m_vStreamProfile.wImageHeight	= pProfile->wImageHeight;
	p->m_vStreamProfile.iCodecIDV		= pProfile->iCodecIDV;
	p->m_vStreamProfile.dFrameRateV		= pProfile->dFrameRateV;
	p->m_vStreamProfile.iBitRateV		= pProfile->iBitRateV;
	//audio
	p->m_vStreamProfile.iBitsPerSample	= pProfile->iBitsPerSample;
	p->m_vStreamProfile.iChannels		= pProfile->iChannels;
	p->m_vStreamProfile.iSamplePerSec	= pProfile->iSamplePerSec;
	p->m_vStreamProfile.iBitRateA		= pProfile->iBitRateA;
	p->m_vStreamProfile.iCodecIDA		= pProfile->iCodecIDA;
	p->m_vStreamProfile.bOnFlagA		= bEnableAudio;
	*/
	//capture video
	ret = p->Start("", 0, "", "", "", 0, STREAM_SOURCE_TYPE_CAPTURE,&p->m_vMediaProfile, nChannel);
	if (!ret)
	{
		CMainService::stcLogReport(LOGLEVEL0, "E", TAG, "Start Capture Channel %d Failed!", nChannel);
		delete p;
		p = NULL;
		return false;
	}
	else
	{
		m_nTimeCount = 0;
		CMainService::stcLogReport(LOGLEVEL0, "I", TAG, "Start Capture Channel %d Successfully!", nChannel);
	}

	m_lstSource.push_back(p);

	return ret;
}
