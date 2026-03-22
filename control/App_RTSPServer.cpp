#include "App_RTSPServer.h"
#include "include/App_Define.h"

// Static singleton instance
AppRTSPServer* AppRTSPServer::m_pInstance = nullptr;

AppRTSPServer::AppRTSPServer()
{
    strncpy(m_rtspHost, DEFAULT_RTSP_HOST, sizeof(m_rtspHost) - 1);
    m_rtspHost[sizeof(m_rtspHost) - 1] = '\0';
    strncpy(m_device, DEFAULT_DEVICE, sizeof(m_device) - 1);
    m_device[sizeof(m_device) - 1] = '\0';
    m_rtspPort = DEFAULT_RTSP_PORT;
    m_fps      = DEFAULT_FPS;
    m_codec    = CODEC_H264;
    m_source   = SOURCE_TEST;
}

void AppRTSPServer::setConfig(const AppConfig_T& cfg)
{
    strncpy(m_rtspHost, cfg.rtsp_host, sizeof(m_rtspHost) - 1);
    m_rtspHost[sizeof(m_rtspHost) - 1] = '\0';
    strncpy(m_device, cfg.device, sizeof(m_device) - 1);
    m_device[sizeof(m_device) - 1] = '\0';
    m_rtspPort = cfg.rtsp_port;
    m_fps      = cfg.fps;
    m_codec    = cfg.codec;
    m_source   = cfg.source;
}
AppRTSPServer::~AppRTSPServer() {
    if (m_rtspServer) {
        g_object_unref(m_rtspServer);
        m_rtspServer = nullptr;
    }
}

void AppRTSPServer::initInstance() {
    AppThread::initInstance();
}

void AppRTSPServer::exitInstance() {
    AppThread::exitInstance();
}

int AppRTSPServer::GstInit()
{
    GstFBDataFrameInfo_T	*pGstFBDataFrameInfo = &m_pGstFBDataFrameInfo;

	if(pGstFBDataFrameInfo->loop != NULL) {
		return -1;
	}

	gst_init (NULL,NULL);

	pGstFBDataFrameInfo->loop = g_main_loop_new(NULL, FALSE);

	if(pGstFBDataFrameInfo->loop == NULL) {
		return -1;
	}

	return 0;
}

int AppRTSPServer::GstExit()
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

// Build source bin string
static std::string make_source_bin(SourceType_E source, const char* device, int width, int height, int fps) {
    char buf[256];
    if (source == SOURCE_V4L2) {
        std::snprintf(buf, sizeof(buf),
            "v4l2src device=%s ! "
            "video/x-raw,format=NV12,width=%d,height=%d,framerate=%d/1",
            device, width, height, fps);
    } else {
        std::snprintf(buf, sizeof(buf),
            "videotestsrc is-live=true pattern=smpte ! "
            "video/x-raw,format=NV12,width=%d,height=%d,framerate=%d/1",
            width, height, fps);
    }
    return std::string(buf);
}

// Build RTSP launch pipeline for a given resolution, payload, fps and codec
static std::string make_rtsp_pipeline(int width, int height, int pt, int fps, CodecType_E codec,
                                      SourceType_E source, const char* device) {
    std::string src = make_source_bin(source, device, width, height, fps);
    char buf[512];
    if (codec == CODEC_H265) {
        std::snprintf(buf, sizeof(buf),
            "( %s ! videoconvert ! x265enc tune=zerolatency ! "
            "h265parse ! rtph265pay pt=%d name=pay0 config-interval=1 )",
            src.c_str(), pt);
    } else {
        std::snprintf(buf, sizeof(buf),
            "( %s ! videoconvert ! x264enc tune=zerolatency ! "
            "h264parse ! rtph264pay pt=%d name=pay0 config-interval=1 )",
            src.c_str(), pt);
    }
    return std::string(buf);
}

// mounts에 path 추가
static void add_stream(GstRTSPServer* server, const Stream& s, int fps, CodecType_E codec,
                       SourceType_E source, const char* device) {
    GstRTSPMountPoints* mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory* factory = gst_rtsp_media_factory_new();
    std::string launch = make_rtsp_pipeline(s.w, s.h, s.pt, fps, codec, source, device);
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
static void enable_only(GstRTSPServer* server, int idx, int fps, CodecType_E codec,
                        SourceType_E source, const char* device) {
    const int N = (int)(sizeof(kStreams)/sizeof(kStreams[0]));
    for (int i = 0; i < N; ++i) {
        if (i == idx) add_stream(server, kStreams[i], fps, codec, source, device);
        else          remove_stream(server, kStreams[i].path);
    }
}

// 모두 활성
static void enable_all(GstRTSPServer* server, int fps, CodecType_E codec,
                       SourceType_E source, const char* device) {
    for (const auto& s : kStreams) add_stream(server, s, fps, codec, source, device);
}

void AppRTSPServer::StartRTSPServer() {
    char portStr[16];
    std::snprintf(portStr, sizeof(portStr), "%d", m_rtspPort);

    m_rtspServer = GST_RTSP_SERVER(gst_rtsp_server_new());
    gst_rtsp_server_set_address(m_rtspServer, m_rtspHost);
    gst_rtsp_server_set_service(m_rtspServer, portStr);

    enable_all(m_rtspServer, m_fps, m_codec, m_source, m_device);

    gst_rtsp_server_attach(m_rtspServer, nullptr);
    g_print("RTSP Server Running on rtsp://%s:%d (%s, %s)\n",
            m_rtspHost, m_rtspPort,
            (m_codec == CODEC_H265) ? "h265" : "h264",
            (m_source == SOURCE_V4L2) ? m_device : "testsrc");
    g_print("Available paths: /cam1 /cam2 /cam3\n");
}

void AppRTSPServer::mainLoop() {
    GstInit();
    StartRTSPServer();

    // GLib 메인 루프 실행 — isRunning() 감시하며 종료 시 quit
    GMainContext* ctx = g_main_context_default();
    while (isRunning()) {
        g_main_context_iteration(ctx, FALSE);
        usleep(1000);
    }

    GstExit();
}
