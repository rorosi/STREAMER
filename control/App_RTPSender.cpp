#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "App_RTPSender.h"
#include "include/App_Define.h"

///////////////////////////////////////////////////////////////////////

AppRTPSender* AppRTPSender::m_pInstance = NULL;

///////////////////////////////////////////////////////////////////////
AppRTPSender::AppRTPSender()
{
	m_pGstRTPDataFrameInfo.loop = NULL;

	m_stream.host      = DEFAULT_RTP_HOST;
	m_stream.port      = DEFAULT_RTP_PORT;
	m_stream.width     = DEFAULT_WIDTH;
	m_stream.height    = DEFAULT_HEIGHT;
	m_stream.framerate = DEFAULT_FPS;
	m_stream.bitrate   = DEFAULT_BITRATE;
	m_stream.codec     = 0;
	m_stream.source    = SOURCE_TEST;
	m_stream.device    = DEFAULT_DEVICE;
	m_stream.pipeline  = NULL;
	m_stream.bus       = NULL;
	m_stream.active    = false;
}

AppRTPSender::~AppRTPSender()
{
	if (m_stream.active) {
		StopStream();
	}
}

void AppRTPSender::initInstance()
{
	AppThread::initInstance();
}

void AppRTPSender::exitInstance()
{
	AppThread::exitInstance();
}

void AppRTPSender::setConfig(const AppConfig_T& cfg)
{
	m_stream.host      = std::string(cfg.host);
	m_stream.port      = cfg.port;
	m_stream.width     = cfg.width;
	m_stream.height    = cfg.height;
	m_stream.framerate = cfg.fps;
	m_stream.bitrate   = cfg.bitrate;
	m_stream.codec     = cfg.codec;
	m_stream.source    = (int)cfg.source;
	m_stream.device    = std::string(cfg.device);
}

int AppRTPSender::GstInit()
{
	GstRTPDataFrameInfo_T *pInfo = &m_pGstRTPDataFrameInfo;

	if (pInfo->loop != NULL) {
		return -1;
	}

	gst_init(NULL, NULL);

	pInfo->loop = g_main_loop_new(NULL, FALSE);

	if (pInfo->loop == NULL) {
		return -1;
	}

	return 0;
}

int AppRTPSender::GstExit()
{
	GstRTPDataFrameInfo_T *pInfo = &m_pGstRTPDataFrameInfo;

	if (m_stream.active) {
		StopStream();
	}

	if (pInfo->loop) {
		g_main_loop_unref(pInfo->loop);
	}
	pInfo->loop = NULL;

	return 0;
}

std::string AppRTPSender::buildSourceBin()
{
	char buf[256];
	if (m_stream.source == SOURCE_V4L2) {
		std::snprintf(buf, sizeof(buf),
			"v4l2src device=%s ! "
			"video/x-raw,format=NV12,width=%d,height=%d,framerate=%d/1",
			m_stream.device.c_str(), m_stream.width, m_stream.height, m_stream.framerate);
	} else {
		std::snprintf(buf, sizeof(buf),
			"videotestsrc is-live=true pattern=smpte ! "
			"video/x-raw,format=NV12,width=%d,height=%d,framerate=%d/1",
			m_stream.width, m_stream.height, m_stream.framerate);
	}
	return std::string(buf);
}

std::string AppRTPSender::buildPipeline()
{
	std::string src = buildSourceBin();
	char buf[512];
	if (m_stream.codec == 1) {	// H.265
		std::snprintf(buf, sizeof(buf),
			"%s ! videoconvert ! "
			"x265enc tune=zerolatency bitrate=%d ! "
			"rtph265pay pt=96 config-interval=1 ! "
			"udpsink host=%s port=%d sync=false",
			src.c_str(), m_stream.bitrate,
			m_stream.host.c_str(), m_stream.port);
	} else {					// H.264 (default)
		std::snprintf(buf, sizeof(buf),
			"%s ! videoconvert ! "
			"x264enc tune=zerolatency bitrate=%d ! "
			"rtph264pay pt=96 config-interval=1 ! "
			"udpsink host=%s port=%d sync=false",
			src.c_str(), m_stream.bitrate,
			m_stream.host.c_str(), m_stream.port);
	}
	return std::string(buf);
}

bool AppRTPSender::StartStream()
{
	if (m_stream.active) {
		return true;
	}

	GError* err = NULL;
	std::string desc = buildPipeline();

	m_stream.pipeline = gst_parse_launch(desc.c_str(), &err);
	if (!m_stream.pipeline || err) {
		fprintf(stderr, "AppRTPSender: gst_parse_launch failed: %s\n",
				err ? err->message : "unknown");
		if (err) g_error_free(err);
		return false;
	}

	m_stream.bus = gst_element_get_bus(m_stream.pipeline);
	if (!m_stream.bus) {
		fprintf(stderr, "AppRTPSender: gst_element_get_bus failed\n");
		gst_object_unref(m_stream.pipeline);
		m_stream.pipeline = NULL;
		return false;
	}

	GstStateChangeReturn ret = gst_element_set_state(m_stream.pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		fprintf(stderr, "AppRTPSender: set_state PLAYING failed\n");
		if (m_stream.bus)  { gst_object_unref(m_stream.bus);      m_stream.bus      = NULL; }
		if (m_stream.pipeline) { gst_object_unref(m_stream.pipeline); m_stream.pipeline = NULL; }
		return false;
	}

	m_stream.active = true;
	printf("AppRTPSender: stream started -> rtp://%s:%d (%dx%d %dfps %dkbps %s %s)\n",
		   m_stream.host.c_str(), m_stream.port,
		   m_stream.width, m_stream.height, m_stream.framerate, m_stream.bitrate,
		   (m_stream.codec == 1) ? "h265" : "h264",
		   (m_stream.source == SOURCE_V4L2) ? m_stream.device.c_str() : "testsrc");
	return true;
}

bool AppRTPSender::StopStream()
{
	if (!m_stream.active) {
		return true;
	}

	gst_element_set_state(m_stream.pipeline, GST_STATE_NULL);

	if (m_stream.bus) {
		gst_object_unref(m_stream.bus);
		m_stream.bus = NULL;
	}
	if (m_stream.pipeline) {
		gst_object_unref(m_stream.pipeline);
		m_stream.pipeline = NULL;
	}

	m_stream.active = false;
	printf("AppRTPSender: stream stopped\n");
	return true;
}

bool AppRTPSender::RestartStream()
{
	StopStream();
	return StartStream();
}

void AppRTPSender::pollBusErrors()
{
	if (!m_stream.active || !m_stream.bus) return;

	GstMessage* msg = gst_bus_pop_filtered(m_stream.bus, GST_MESSAGE_ERROR);
	if (!msg) return;

	GError* err = NULL;
	gchar*  dbg = NULL;
	gst_message_parse_error(msg, &err, &dbg);
	fprintf(stderr, "AppRTPSender: bus error: %s\n", err ? err->message : "unknown");
	if (err) g_error_free(err);
	if (dbg) g_free(dbg);
	gst_message_unref(msg);

	// 에러 발생 시 자동 재시작
	RestartStream();
}

void AppRTPSender::mainLoop()
{
	AppMsgQueue   qRtpMain(KEY_MQ_RTP_TASK);
	AppCommand    cmd;

	if (GstInit() != 0) {
		fprintf(stderr, "AppRTPSender: GstInit failed\n");
		return;
	}

	StartStream();	// 시작 시 즉시 RTP 송신 시작

	GMainContext* ctx = g_main_context_default();

	while (isRunning()) {
		while (g_main_context_pending(ctx))
			g_main_context_iteration(ctx, FALSE);

		pollBusErrors();

		while (qRtpMain.peek(1) == true) {
			if (qRtpMain.read(1, &cmd) == true) {
				switch (cmd.getId()) {
					case M_RTP_STREAM_START:
						printf("M_RTP_STREAM_START\n");
						StartStream();
						break;
					case M_RTP_STREAM_STOP:
						printf("M_RTP_STREAM_STOP\n");
						StopStream();
						break;
					case M_RTP_STREAM_RESTART:
						printf("M_RTP_STREAM_RESTART\n");
						RestartStream();
						break;
					default:
						break;
				}
			}
		}

		usleep(10000);
	}

	GstExit();
}
