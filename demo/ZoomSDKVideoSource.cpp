//SendVideoRawData

#include "ZoomSDKVideoSource.h"
#include "Log.h"
#include <iostream>
#include <iostream>
#include <string>
#include <cstdio>
#include <chrono>

extern "C"
{
#include <libavformat/avformat.h>
}

//using namespace cv;
using namespace std;

int video_play_flag = -1;
int width = WIDTH;
int height = HEIGHT;

std::string ToString(std::string &s)
{
	std::string t("\"");
	t.append(s);
	t.push_back('"');
	return t;
}

std::string ToString(const char *const s)
{
	std::string t(s);
	return ToJSONString(t);
}

std::string ToString(int i)
{
	return std::to_string(i);
}

std::string ToJSONString(std::string &s)
{
	std::string t("\"");
	t.append(s);
	t.push_back('"');
	return t;
}

std::string ToJSONString(const char *const s)
{
	std::string t(s);
	return ToJSONString(t);
}

std::string ToJSONString(unsigned long i)
{
	return std::to_string(i);
}

std::string ToJSONString(unsigned int i)
{
	return std::to_string(i);
}

std::string ToJSONString(int i)
{
	return std::to_string(i);
}

std::string ToJSONString(bool b)
{
	return b ? "true" : "false";
}

void PlayVideoFileToVirtualCamera(IZoomSDKVideoSender* video_sender, const std::string& video_source)
{
    //implement your code to read from a file, and send it using video_sender
    // you can use ffmpeg to convert your video file into yuv420p format, read the frames and send each frame using video_sender
}

void ZoomSDKVideoSource::onInitialize(IZoomSDKVideoSender* sender, IList<VideoSourceCapability>* support_cap_list, VideoSourceCapability& suggest_cap)
{
    LOG_CALLBACK("ZoomSDKVideoSource", "onInitialize");
    std::cout << "Waiting for turnOn chat command" << endl;
    video_sender_ = sender;
}

void ZoomSDKVideoSource::onPropertyChange(IList<VideoSourceCapability>* support_cap_list, VideoSourceCapability suggest_cap)
{
    LOG_CALLBACK("ZoomSDKVideoSource", "onPropertyChange");
    std::cout << "suggest frame: " << suggest_cap.frame << endl;
    std::cout << "suggest size: " << suggest_cap.width << "x" << suggest_cap.height << endl;
    width = suggest_cap.width;
    height = suggest_cap.height;
    std::cout << "calculated frameLen: " << height / 2 * 3 * width << endl;
}

#include <thread>
void ZoomSDKVideoSource::onStartSend()
{
    LOG_CALLBACK("ZoomSDKVideoSource", "onStartSend");
    if (video_sender_ && video_play_flag != 1)
    {
        while (video_play_flag > -1) {}
        video_play_flag = 1;
        thread(PlayVideoFileToVirtualCamera, video_sender_, video_source_).detach();
    }
    else
    {
        std::cout << "video_sender_ is null" << endl;
    }
}

void ZoomSDKVideoSource::onStopSend()
{
    LOG_CALLBACK("ZoomSDKVideoSource", "onStopSend");
    video_play_flag = 0;
}

void ZoomSDKVideoSource::onUninitialized()
{
    LOG_CALLBACK("ZoomSDKVideoSource", "onUninitialized");
    video_sender_ = nullptr;
}

ZoomSDKVideoSource::ZoomSDKVideoSource(string video_source)
{
    LOG_CALLBACK("ZoomSDKVideoSource", "ZoomSDKVideoSource", video_source);
    video_source_ = video_source;

    // FIXME: THIS IS WRONG. This context won't get passed to the thread. This should be done inside the thread instead
    AVFormatContext* format_ctx = nullptr;
    if (avformat_open_input(&format_ctx, video_source_.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Could not open input file: " << video_source_ << std::endl;
    }
}
