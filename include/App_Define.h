#ifndef _APP_DEFINE_H__
#define _APP_DEFINE_H__

// RTP Sender Task
#define KEY_MQ_RTP_TASK			0x20000002

// RTP Sender module IDs
const int M_RTP_STREAM_START	= 2010;
const int M_RTP_STREAM_STOP		= 2011;
const int M_RTP_STREAM_RESTART	= 2012;

// Default values (single source of truth)
#define DEFAULT_RTP_HOST    "127.0.0.1"
#define DEFAULT_RTP_PORT    5000
#define DEFAULT_RTSP_HOST   "127.0.0.1"
#define DEFAULT_RTSP_PORT   8554
#define DEFAULT_WIDTH       1280
#define DEFAULT_HEIGHT      720
#define DEFAULT_FPS         30
#define DEFAULT_BITRATE     2000
#define DEFAULT_DEVICE      "/dev/video0"

// Streaming mode
typedef enum {
	MODE_RTSP = 0,
	MODE_RTP,
} StreamMode_E;

// Video source type
typedef enum {
	SOURCE_TEST = 0,	// videotestsrc (기본)
	SOURCE_V4L2,		// v4l2src (실제 카메라)
} SourceType_E;

// Codec type
typedef enum {
	CODEC_H264 = 0,
	CODEC_H265,
} CodecType_E;

// CLI argument config
typedef struct {
	StreamMode_E mode;
	SourceType_E source;
	char         host[64];		// [rtp] 목적지 IP
	int          port;			// [rtp] 목적지 UDP 포트
	char         device[64];	// [v4l2] 장치 경로 (/dev/video0)
	int          width;
	int          height;
	int          fps;
	int          bitrate;		// kbps
	CodecType_E  codec;
	char         rtsp_host[64];	// [rtsp] 서버 바인드 주소
	int          rtsp_port;		// [rtsp] 서버 포트
} AppConfig_T;

// Message Index

#endif /* _APP_DEFINE_H__ */
