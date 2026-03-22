#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>

#if 0
#ifdef __cplusplus
#include <ev++.h>
#else
#include <ev.h>
#endif
#endif

#include "App_Define.h"
#include "include/version.h"
#include "control/App_StreamManager.h"

using namespace std;

/////////////////////////////////////////////////////////////

static int g_iRunning = 0;

/////////////////////////////////////////////////////////////
static void printUsage(const char* prog)
{
	printf("Usage: %s [OPTIONS]\n\n", prog);
	printf("Common:\n");
	printf("  -m, --mode        <rtsp|rtp>       Streaming mode        (default: rtsp)\n");
	printf("  -c, --codec       <h264|h265>       Codec format          (default: h264)\n");
	printf("  -f, --fps         <FPS>             Framerate 1-120       (default: %d)\n",   DEFAULT_FPS);
	printf("  -s, --source      <test|v4l2>       Video source          (default: test)\n");
	printf("  -d, --device      <PATH>            V4L2 device path      (default: %s)\n",   DEFAULT_DEVICE);
	printf("  -h, --help                          Show this help\n\n");
	printf("RTP mode:\n");
	printf("  -H, --host        <IP>              Destination IP        (default: %s)\n",   DEFAULT_RTP_HOST);
	printf("  -p, --port        <PORT>            Destination port      (default: %d)\n",   DEFAULT_RTP_PORT);
	printf("  -w, --width       <W>               Frame width           (default: %d)\n",   DEFAULT_WIDTH);
	printf("  -e, --height      <H>               Frame height          (default: %d)\n",   DEFAULT_HEIGHT);
	printf("  -b, --bitrate     <KBPS>            Bitrate kbps          (default: %d)\n\n", DEFAULT_BITRATE);
	printf("RTSP mode:\n");
	printf("      --rtsp-host   <IP>              Server bind address   (default: %s)\n",   DEFAULT_RTSP_HOST);
	printf("      --rtsp-port   <PORT>            Server port           (default: %d)\n\n", DEFAULT_RTSP_PORT);
	printf("Examples:\n");
	printf("  %s --mode rtsp\n", prog);
	printf("  %s --mode rtsp --rtsp-host 0.0.0.0 --codec h265\n", prog);
	printf("  %s --mode rtsp --source v4l2 --device /dev/video0\n", prog);
	printf("  %s --mode rtp --host 192.168.1.100 --port 5000 --codec h265\n", prog);
	printf("  %s --mode rtp --source v4l2 --device /dev/video0 --width 1920 --height 1080\n\n", prog);
}

static AppConfig_T parseArgs(int argc, char* argv[])
{
	AppConfig_T cfg;
	memset(&cfg, 0, sizeof(AppConfig_T));
	cfg.mode      = MODE_RTSP;
	cfg.source    = SOURCE_TEST;
	cfg.port      = DEFAULT_RTP_PORT;
	cfg.width     = DEFAULT_WIDTH;
	cfg.height    = DEFAULT_HEIGHT;
	cfg.fps       = DEFAULT_FPS;
	cfg.bitrate   = DEFAULT_BITRATE;
	cfg.codec     = CODEC_H264;
	cfg.rtsp_port = DEFAULT_RTSP_PORT;
	strncpy(cfg.host,      DEFAULT_RTP_HOST,  sizeof(cfg.host) - 1);
	strncpy(cfg.device,    DEFAULT_DEVICE,    sizeof(cfg.device) - 1);
	strncpy(cfg.rtsp_host, DEFAULT_RTSP_HOST, sizeof(cfg.rtsp_host) - 1);

	static struct option long_opts[] = {
		{"mode",      required_argument, 0, 'm'},
		{"host",      required_argument, 0, 'H'},
		{"port",      required_argument, 0, 'p'},
		{"width",     required_argument, 0, 'w'},
		{"height",    required_argument, 0, 'e'},
		{"fps",       required_argument, 0, 'f'},
		{"bitrate",   required_argument, 0, 'b'},
		{"codec",     required_argument, 0, 'c'},
		{"source",    required_argument, 0, 's'},
		{"device",    required_argument, 0, 'd'},
		{"rtsp-host", required_argument, 0,  1 },
		{"rtsp-port", required_argument, 0,  2 },
		{"help",      no_argument,       0, 'h'},
		{0, 0, 0, 0}
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "m:H:p:w:e:f:b:c:s:d:h", long_opts, NULL)) != -1) {
		switch (opt) {
			case 'm':
				if (strcasecmp(optarg, "rtp") == 0) cfg.mode = MODE_RTP;
				else                                cfg.mode = MODE_RTSP;
				break;
			case 'H':
				strncpy(cfg.host, optarg, sizeof(cfg.host) - 1);
				break;
			case 'p': {
				int v = atoi(optarg);
				if (v < 1024 || v > 65535) { fprintf(stderr, "Invalid port: %d (1024-65535)\n", v); exit(1); }
				cfg.port = v;
				break;
			}
			case 'w': {
				int v = atoi(optarg);
				if (v <= 0) { fprintf(stderr, "Invalid width: %d\n", v); exit(1); }
				cfg.width = v;
				break;
			}
			case 'e': {
				int v = atoi(optarg);
				if (v <= 0) { fprintf(stderr, "Invalid height: %d\n", v); exit(1); }
				cfg.height = v;
				break;
			}
			case 'f': {
				int v = atoi(optarg);
				if (v <= 0 || v > 120) { fprintf(stderr, "Invalid fps: %d (1-120)\n", v); exit(1); }
				cfg.fps = v;
				break;
			}
			case 'b': {
				int v = atoi(optarg);
				if (v <= 0) { fprintf(stderr, "Invalid bitrate: %d\n", v); exit(1); }
				cfg.bitrate = v;
				break;
			}
			case 'c': cfg.codec  = (strcasecmp(optarg, "h265") == 0) ? CODEC_H265 : CODEC_H264; break;
			case 's': cfg.source = (strcasecmp(optarg, "v4l2") == 0) ? SOURCE_V4L2 : SOURCE_TEST; break;
			case 'd': strncpy(cfg.device, optarg, sizeof(cfg.device) - 1); break;
			case  1 : strncpy(cfg.rtsp_host, optarg, sizeof(cfg.rtsp_host) - 1); break;
			case  2 : {
				int v = atoi(optarg);
				if (v < 1024 || v > 65535) { fprintf(stderr, "Invalid rtsp-port: %d (1024-65535)\n", v); exit(1); }
				cfg.rtsp_port = v;
				break;
			}
			case 'h':
				printUsage(argv[0]);
				exit(0);
			default:
				break;
		}
	}
	return cfg;
}

#if 0
struct ev_loop *g_EvDefaultLoop;
static ev_timer g_EvDefaultTimer;
#endif

/////////////////////////////////////////////////////////////
void sighdlr_exit(int s)
{
	printf("SIGNAL :: EXIT(%d, %d) \n", s, getpid());
	g_iRunning = 0;
}

void sighdlr_ignore(int s)
{
	//printf( "SIGNAL :: IGNORE(%d) \n", s);
}

void sighdlr_init(int s)
{
	printf( "SIGNAL ::  Initialize(%d) \n", s);
}

void sighdlr_permit(int s)
{
	g_iRunning = 0;
	printf( "SIGNAL :: PERMIT Change(%d) \n",  s);
	//exit(1);
}

void sigaction_segv(int s, siginfo_t* siginfo, void* t)
{
	printf( "SIGNAL :: SEGV(%d, %d) \n", getpid(), s);
	exit(1);
}


void SetSignal()
{
	struct sigaction act_print_exit;
	struct sigaction act_print_ignore;
	struct sigaction act_print_init;
	struct sigaction act_print_permit;
	struct sigaction act_print_segv;

	memset(&act_print_exit, 0, sizeof(struct sigaction));
	memset(&act_print_ignore, 0, sizeof(struct sigaction));
	memset(&act_print_init, 0, sizeof(struct sigaction));
	memset(&act_print_permit, 0, sizeof(struct sigaction));
	memset(&act_print_segv, 0, sizeof(struct sigaction));

	act_print_exit.sa_handler=sighdlr_exit;
	act_print_ignore.sa_handler=sighdlr_ignore;
	act_print_init.sa_handler=sighdlr_init;
	act_print_permit.sa_handler=sighdlr_permit;
	act_print_segv.sa_sigaction=sigaction_segv;

	sigemptyset(&(act_print_exit.sa_mask));
	sigemptyset(&(act_print_ignore.sa_mask));
	sigemptyset(&(act_print_init.sa_mask));
	sigemptyset(&(act_print_permit.sa_mask));
	sigemptyset(&(act_print_segv.sa_mask));

	act_print_exit.sa_flags=0;
	act_print_ignore.sa_flags=0;
	act_print_init.sa_flags=0;
	act_print_permit.sa_flags=0;
	act_print_segv.sa_flags=SA_SIGINFO;


	sigaction(SIGINT, &act_print_exit, NULL);		// 2
	sigaction(SIGILL, &act_print_exit, NULL);		// 4
	sigaction(SIGTRAP, &act_print_exit, NULL);		// 5
	sigaction(SIGABRT, &act_print_exit, NULL);		// 6
	sigaction(SIGFPE, &act_print_exit, NULL);		// 8
	sigaction(SIGSEGV, &act_print_segv, NULL);      // 11
	sigaction(SIGSYS, &act_print_exit, NULL);		// 12
	sigaction(SIGXCPU, &act_print_exit, NULL);		// 30
	sigaction(SIGXFSZ, &act_print_exit, NULL);		// 31

	sigaction(SIGQUIT, &act_print_ignore, NULL);	// 3
	sigaction(SIGPIPE, &act_print_ignore, NULL);	// 13
	sigaction(SIGTERM, &act_print_ignore, NULL);	// 15
	sigaction(SIGUSR1, &act_print_ignore, NULL);	// 16
	sigaction(SIGCHLD, &act_print_ignore, NULL);	// 18

	sigaction(SIGHUP, &act_print_init, NULL);		// 1
	sigaction(SIGUSR2, &act_print_permit, NULL);	// 17	
}

int main(int argc, char **argv)
{
	AppConfig_T cfg = parseArgs(argc, argv);

	printf( "\n+--------------------------------------------+ \n");
	printf( "|                                            |\n");
	printf( "|     D A S H - B O A R D   S T A R T        |\n");
	printf( "|                                            |\n");
	printf( "|      VERSION %s           	     |\n", EOIR_SW_BUILD_VER);
	printf( "|                                            |\n");
	printf( "|      MODE: %-30s |\n", (cfg.mode == MODE_RTP) ? "RTP" : "RTSP");
	printf( "+--------------------------------------------+\n\n");

	SetSignal();

	g_iRunning = 1;

	AppStreamManager::instance()->setConfig(cfg);
	AppStreamManager::instance()->start();

	while(g_iRunning){
		usleep(10000);
	};

	// For APPGSTMnager End
	AppStreamManager::instance()->stop();
	AppStreamManager::instance()->join();    
	AppStreamManager::instance()->release();
	// --

	printf( "\n+--------------------------------------------+ \n");
	printf( "|                                            |\n");
	printf( "|     D A S H - B O A R D   E N D            |\n");
	printf( "|                                            |\n");
	printf( "+--------------------------------------------+\n\n");
	
	return 0;
}
