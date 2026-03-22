#ifndef _APP_GST_H_ 
#define _APP_GST_H_

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

class AppGST : public AppThread
{
public:
	AppGST();
	virtual ~AppGST();
	virtual void mainLoop();
	virtual void initInstance();
	virtual void exitInstance();
	
	static AppGST* instance();
	void release();

	void setConfig(const AppConfig_T& cfg);
	void StartRTSPServer();
		
private:
	int GstInit();
	int GstExit();

public:
	GstFBDataFrameInfo_T	m_pGstFBDataFrameInfo;

private:
    static AppGST 	*m_pInstance;
    GstRTSPServer*   m_rtspServer = nullptr;
    char             m_rtspHost[64];
    int              m_rtspPort;
    int              m_fps;
    CodecType_E      m_codec;
    SourceType_E     m_source;
    char             m_device[64];
};

inline AppGST* AppGST::instance()
{
	if (!m_pInstance) {
		m_pInstance = new AppGST();	
	}
	return m_pInstance;
}

inline void AppGST::release()
{
	if (m_pInstance != NULL) {
		delete m_pInstance;
		m_pInstance = NULL;
	}
}
#endif // _APP_GST_H_


