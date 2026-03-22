#ifndef _APP_RTSP_SERVER_H_
#define _APP_RTSP_SERVER_H_

#include <stdio.h>
#include <string>

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#include "base/App_Thread.h"
#include "base/App_MsgQueue.h"
#include "include/App_Define.h"

using namespace std;

typedef struct
{

	GMainLoop *loop;

} GstFBDataFrameInfo_T;	// PortData

class AppRTSPServer : public AppThread
{
public:
	AppRTSPServer();
	virtual ~AppRTSPServer();
	virtual void mainLoop();
	virtual void initInstance();
	virtual void exitInstance();

	static AppRTSPServer* instance();
	void release();

	void setConfig(const AppConfig_T& cfg);
	void StartRTSPServer();

private:
	int GstInit();
	int GstExit();

public:
	GstFBDataFrameInfo_T	m_pGstFBDataFrameInfo;

private:
    static AppRTSPServer	*m_pInstance;
    GstRTSPServer*           m_rtspServer = nullptr;
    char                     m_rtspHost[64];
    int                      m_rtspPort;
    int                      m_fps;
    CodecType_E              m_codec;
    SourceType_E             m_source;
    char                     m_device[64];
};

inline AppRTSPServer* AppRTSPServer::instance()
{
	if (!m_pInstance) {
		m_pInstance = new AppRTSPServer();
	}
	return m_pInstance;
}

inline void AppRTSPServer::release()
{
	if (m_pInstance != NULL) {
		delete m_pInstance;
		m_pInstance = NULL;
	}
}
#endif // _APP_RTSP_SERVER_H_
