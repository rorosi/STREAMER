#include "App_GST.h"
#include "include/App_Define.h"

// Static singleton instance
AppGST* AppGST::m_pInstance = nullptr;

AppGST::AppGST()
{
    strncpy(m_rtspHost, DEFAULT_RTSP_HOST, sizeof(m_rtspHost) - 1);
    m_rtspHost[sizeof(m_rtspHost) - 1] = '\0';
    m_rtspPort = DEFAULT_RTSP_PORT;
    m_fps      = DEFAULT_FPS;
    m_codec    = 0;
}

void AppGST::setConfig(const AppConfig_T& cfg)
{
    strncpy(m_rtspHost, cfg.rtsp_host, sizeof(m_rtspHost) - 1);
    m_rtspHost[sizeof(m_rtspHost) - 1] = '\0';
    m_rtspPort = cfg.rtsp_port;
    m_fps      = cfg.fps;
    m_codec    = cfg.codec;
}
AppGST::~AppGST() {
    if (m_rtspServer) {
        g_object_unref(m_rtspServer);
        m_rtspServer = nullptr;
    }
}

void AppGST::initInstance() {
    AppThread::initInstance();
}

void AppGST::exitInstance() {
    AppThread::exitInstance();
}

int AppGST::GstInit()
{
    GstFBDataFrameInfo_T	*pGstFBDataFrameInfo = &m_pGstFBDataFrameInfo;

	if(pGstFBDataFrameInfo->loop != NULL) {
		////dlog_err ("XavierGst :: Failed to gst initialization, already gst init...");
		return -1;
	}

	gst_init (NULL,NULL);

	pGstFBDataFrameInfo->loop = g_main_loop_new(NULL, FALSE);

	if(pGstFBDataFrameInfo->loop == NULL) {
		////dlog_err ("XavierGst :: Failed to gst initialization...");
		return -1;
	}

	return 0;
}

int AppGST::GstExit()
{	
    GstFBDataFrameInfo_T	*pGstFBDataFrameInfo = &m_pGstFBDataFrameInfo;
	
	if(pGstFBDataFrameInfo->loop) {
		g_main_loop_unref(pGstFBDataFrameInfo->loop);
	}
	pGstFBDataFrameInfo->loop = NULL;

	return 0;
}

// ===== RTSP stream helpers (only within this translation unit) =====
namespace {
struct Stream { const char* path; int w, h, pt; };
static const Stream kStreams[] = {
    { "/cam1", 1920, 1080, 96 },
    { "/cam2", 1920, 1080, 97 },
    { "/cam3",  640,  480, 98 }
};
}

// Build RTSP launch pipeline for a given resolution, payload, fps and codec
static std::string make_rtsp_pipeline(int width, int height, int pt, int fps, int codec) {
    char buf[512];
    if (codec == 1) {	// H.265
        std::snprintf(buf, sizeof(buf),
            "( videotestsrc is-live=true pattern=smpte ! "
            "video/x-raw,format=NV12,width=%d,height=%d,framerate=%d/1 ! "
            "videoconvert ! x265enc tune=zerolatency ! "
            "h265parse ! rtph265pay pt=%d name=pay0 config-interval=1 )",
            width, height, fps, pt);
    } else {			// H.264 (default)
        std::snprintf(buf, sizeof(buf),
            "( videotestsrc is-live=true pattern=smpte ! "
            "video/x-raw,format=NV12,width=%d,height=%d,framerate=%d/1 ! "
            "videoconvert ! x264enc tune=zerolatency ! "
            "h264parse ! rtph264pay pt=%d name=pay0 config-interval=1 )",
            width, height, fps, pt);
    }
    return std::string(buf);
}

// mounts에 path 추가
static void add_stream(GstRTSPServer* server, const Stream& s, int fps, int codec) {
    GstRTSPMountPoints* mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory* factory = gst_rtsp_media_factory_new();
    std::string launch = make_rtsp_pipeline(s.w, s.h, s.pt, fps, codec);
    gst_rtsp_media_factory_set_launch(factory, launch.c_str());
    gst_rtsp_media_factory_set_shared(factory, TRUE);
#ifdef GST_RTSP_SUSPEND_MODE_RESET
    gst_rtsp_media_factory_set_suspend_mode(factory, GST_RTSP_SUSPEND_MODE_RESET);
#endif
    gst_rtsp_mount_points_add_factory(mounts, s.path, factory);
    g_object_unref(mounts);
}

// mounts에서 path 제거
static void remove_stream(GstRTSPServer* server, const char* path) {
    GstRTSPMountPoints* mounts = gst_rtsp_server_get_mount_points(server);
    gst_rtsp_mount_points_remove_factory(mounts, path);
    g_object_unref(mounts);
}

// 지정 index만 활성, 나머지는 비활성
static void enable_only(GstRTSPServer* server, int idx, int fps, int codec) {
    const int N = (int)(sizeof(kStreams)/sizeof(kStreams[0]));
    for (int i = 0; i < N; ++i) {
        if (i == idx) add_stream(server, kStreams[i], fps, codec);
        else          remove_stream(server, kStreams[i].path);
    }
}

// 모두 활성
static void enable_all(GstRTSPServer* server, int fps, int codec) {
    for (const auto& s : kStreams) add_stream(server, s, fps, codec);
}

void AppGST::StartRTSPServer() {
    char portStr[16];
    std::snprintf(portStr, sizeof(portStr), "%d", m_rtspPort);

    m_rtspServer = GST_RTSP_SERVER(gst_rtsp_server_new());
    gst_rtsp_server_set_address(m_rtspServer, m_rtspHost);
    gst_rtsp_server_set_service(m_rtspServer, portStr);

    enable_all(m_rtspServer, m_fps, m_codec);

    gst_rtsp_server_attach(m_rtspServer, nullptr);
    g_print("RTSP Server Running on rtsp://%s:%d (%s)\n",
            m_rtspHost, m_rtspPort, (m_codec == 1) ? "h265" : "h264");
    g_print("Available paths: /cam1 /cam2 /cam3\n");
}

// Main loop entry: start RTSP server and react to commands
void AppGST::mainLoop() {
    AppMsgQueue 		qModuleMain(KEY_MQ_UART_TASK);
    AppCommand		cmd;

    GstInit();
	
    StartRTSPServer();  // RTSP 서버 시작

    GMainContext *ctx = g_main_context_default();
    // GLib 이벤트가 있으면 한 번 처리
    while (g_main_context_pending(ctx))
        g_main_context_iteration(ctx, FALSE);

    while (isRunning()) {
        while (qModuleMain.peek(1)==true)
		{
			if (qModuleMain.read(1, &cmd)==true)
			{
                switch (cmd.getId())
				{
					case M_MODULE_1CH:
                        printf("M_MODULE_1CH -> enable /cam1 only\n");
                        enable_only(m_rtspServer, 0, m_fps, m_codec);
						break;
					case M_MODULE_2CH:
                        printf("M_MODULE_2CH -> enable /cam2 only\n");
                        enable_only(m_rtspServer, 1, m_fps, m_codec);
						break;
					case M_MODULE_3CH:
                        printf("M_MODULE_3CH -> enable /cam3 only\n");
                        enable_only(m_rtspServer, 2, m_fps, m_codec);
						break;
                    case M_MODULE_ALL:
                        printf("M_MODULE_ALL -> enable all\n");
                        enable_all(m_rtspServer, m_fps, m_codec);
						break;
					default:
						break;
				}
            }
        }
        // 주기적으로 GLib 이벤트 처리
        g_main_context_iteration(ctx, FALSE);
        usleep(10000);
    }

    GstExit();
}
