# streamer

GStreamer 기반 영상 스트리밍 데몬. RTSP 서버와 RTP 송신 두 가지 모드를 지원한다.

## 요구사항

- GStreamer 1.0
  - `gstreamer-1.0`
  - `gstreamer-app-1.0`
  - `gstreamer-rtsp-server-1.0`
  - `glib-2.0`, `gobject-2.0`
- g++ (C++11 이상)
- pkg-config

```bash
# Ubuntu/Debian
sudo apt install \
  libgstreamer1.0-dev \
  libgstreamer-plugins-bad1.0-dev \
  libgstrtspserver-1.0-dev \
  gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad \
  gstreamer1.0-libav
```

## 빌드

```bash
make
```

빌드 결과물은 `bin/stream_daemon`에 생성된다.

```bash
make clean   # 빌드 산출물 제거
```

## 사용법

```
stream_daemon [OPTIONS]

Common:
  -m, --mode        <rtsp|rtp>       스트리밍 모드        (기본: rtsp)
  -c, --codec       <h264|h265>      코덱                 (기본: h264)
  -f, --fps         <FPS>            프레임레이트 1-120   (기본: 30)
  -h, --help                         도움말

RTP mode:
  -H, --host        <IP>             목적지 IP            (기본: 127.0.0.1)
  -p, --port        <PORT>           목적지 UDP 포트      (기본: 5000)
  -w, --width       <W>              가로 해상도          (기본: 1280)
  -e, --height      <H>              세로 해상도          (기본: 720)
  -b, --bitrate     <KBPS>           비트레이트 kbps      (기본: 2000)
  -s, --source      <test|v4l2>      영상 소스            (기본: test)
  -d, --device      <PATH>           V4L2 장치 경로       (기본: /dev/video0)

RTSP mode:
      --rtsp-host   <IP>             서버 바인드 주소     (기본: 127.0.0.1)
      --rtsp-port   <PORT>           서버 포트            (기본: 8554)
```

## 예시

### RTSP 서버

```bash
# 기본 실행 (h264, 30fps, rtsp://127.0.0.1:8554)
./bin/stream_daemon --mode rtsp

# 외부 접근 허용 + H.265
./bin/stream_daemon --mode rtsp --rtsp-host 0.0.0.0 --rtsp-port 8554 --codec h265
```

수신 확인:
```bash
gst-launch-1.0 rtspsrc location=rtsp://127.0.0.1:8554/cam1 ! decodebin ! videoconvert ! autovideosink
```

접속 가능한 경로: `/cam1`, `/cam2`, `/cam3`

### RTP 송신

```bash
# videotestsrc → 127.0.0.1:5000
./bin/stream_daemon --mode rtp

# V4L2 카메라 → 원격 수신자
./bin/stream_daemon --mode rtp --source v4l2 --device /dev/video0 \
  --host 192.168.1.100 --port 5000 --width 1920 --height 1080 --codec h265 --bitrate 4000
```

수신 확인:
```bash
# H.264
gst-launch-1.0 udpsrc port=5000 \
  caps="application/x-rtp,media=video,encoding-name=H264,payload=96" \
  ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! autovideosink

# H.265
gst-launch-1.0 udpsrc port=5000 \
  caps="application/x-rtp,media=video,encoding-name=H265,payload=96" \
  ! rtph265depay ! h265parse ! avdec_h265 ! videoconvert ! autovideosink
```

## 프로젝트 구조

```
streamer/
├── include/
│   └── App_Define.h        # 공용 상수, 구조체, 열거형
├── base/                   # 스레딩/IPC 기반 라이브러리
│   ├── App_Thread          # POSIX 스레드 래퍼
│   ├── App_Mutex           # 뮤텍스
│   ├── App_Condition       # 조건변수
│   ├── App_MsgQueue        # System V 메시지큐
│   └── App_Command         # 메시지 커맨드
├── control/                # 스트리밍 제어
│   ├── App_GSTManager      # 모드 분기 및 스레드 관리
│   ├── App_GST             # RTSP 서버 (gst-rtsp-server)
│   └── App_RTPSender       # RTP 송신 (udpsink)
├── main/
│   └── main.cpp            # 진입점, CLI 파싱
├── Rules.make
└── Makefile
```

## V4L2 카메라 연결

현재는 `videotestsrc`(테스트 패턴)를 기본 소스로 사용한다.
실제 카메라 연결 시 `--source v4l2 --device /dev/video0` 옵션을 사용하거나,
`control/App_RTPSender.cpp`의 `buildSourceBin()` 한 곳만 수정하면 소스를 교체할 수 있다.
