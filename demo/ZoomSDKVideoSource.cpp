//SendVideoRawData

#include "ZoomSDKVideoSource.h"
#include "Log.h"
#include <iostream>
#include "Serialisation.h"
#include <cstdio>
#include <chrono>

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

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

void PlayVideoFileToVirtualCamera(IZoomSDKVideoSender* video_sender, const std::string& video_source)
{
    // Initialize FFmpeg (for older versions, you might need av_register_all())
    AVFormatContext *format_ctx = nullptr;
    if (avformat_open_input(&format_ctx, video_source.c_str(), nullptr, nullptr) < 0)
    {
        std::cerr << "Could not open input file: " << video_source << std::endl;
        return;
    }

    if (avformat_find_stream_info(format_ctx, nullptr) < 0)
    {
        std::cerr << "Could not retrieve stream info from: " << video_source << std::endl;
        avformat_close_input(&format_ctx);
        return;
    }

    // Find the first video stream
    int video_stream_index = -1;
    AVCodecParameters *codecpar = nullptr;
    for (unsigned int i = 0; i < format_ctx->nb_streams; i++)
    {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            codecpar = format_ctx->streams[i]->codecpar;
            break;
        }
    }
    if (video_stream_index == -1)
    {
        std::cerr << "No video stream found in: " << video_source << std::endl;
        avformat_close_input(&format_ctx);
        return;
    }

    // Find and open the decoder for the video stream
    const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec)
    {
        std::cerr << "Unsupported codec." << std::endl;
        avformat_close_input(&format_ctx);
        return;
    }
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        std::cerr << "Could not allocate codec context." << std::endl;
        avformat_close_input(&format_ctx);
        return;
    }
    if (avcodec_parameters_to_context(codec_ctx, codecpar) < 0)
    {
        std::cerr << "Failed to copy codec parameters to decoder context." << std::endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return;
    }
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        std::cerr << "Could not open codec." << std::endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return;
    }

    // Allocate frames: one for the decoded frame, one for the converted I420 frame
    AVFrame *frame = av_frame_alloc();
    AVFrame *frame_i420 = av_frame_alloc();
    if (!frame || !frame_i420)
    {
        std::cerr << "Could not allocate frame." << std::endl;
        if(frame) av_frame_free(&frame);
        if(frame_i420) av_frame_free(&frame_i420);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return;
    }

    // Prepare conversion context: convert from codec_ctx->pix_fmt to AV_PIX_FMT_YUV420P
    struct SwsContext *sws_ctx = sws_getContext(codec_ctx->width,
                                                codec_ctx->height,
                                                codec_ctx->pix_fmt,
                                                codec_ctx->width,
                                                codec_ctx->height,
                                                AV_PIX_FMT_YUV420P,
                                                SWS_BILINEAR,
                                                nullptr,
                                                nullptr,
                                                nullptr);
    if (!sws_ctx)
    {
        std::cerr << "Could not initialize the conversion context." << std::endl;
        av_frame_free(&frame);
        av_frame_free(&frame_i420);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return;
    }

    // Allocate buffer for the I420 frame
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, codec_ctx->width, codec_ctx->height, 1);
    uint8_t *buffer = (uint8_t*)av_malloc(num_bytes * sizeof(uint8_t));
    if (!buffer)
    {
        std::cerr << "Could not allocate buffer." << std::endl;
        sws_freeContext(sws_ctx);
        av_frame_free(&frame);
        av_frame_free(&frame_i420);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return;
    }
    // Set frame_i420 data pointers and linesizes based on the buffer
    av_image_fill_arrays(frame_i420->data, frame_i420->linesize, buffer, AV_PIX_FMT_YUV420P,
                         codec_ctx->width, codec_ctx->height, 1);

    AVPacket packet;
    av_init_packet(&packet);

    // Main decoding loop
    while (av_read_frame(format_ctx, &packet) >= 0)
    {
        if (packet.stream_index == video_stream_index)
        {
            // Send packet to decoder
            if (avcodec_send_packet(codec_ctx, &packet) < 0)
            {
                std::cerr << "Error sending packet for decoding." << std::endl;
                break;
            }
            // Receive all available frames from the decoder
            while (avcodec_receive_frame(codec_ctx, frame) == 0)
            {
                // Convert the frame to I420
                sws_scale(sws_ctx,
                          frame->data,
                          frame->linesize,
                          0,
                          codec_ctx->height,
                          frame_i420->data,
                          frame_i420->linesize);

                // Calculate the frame size for I420 (width * height * 3/2)
                int frameSize = codec_ctx->width * codec_ctx->height * 3 / 2;
                int rotation = 0;
                FrameDataFormat format = FrameDataFormat_I420_FULL;

                // Send the converted frame to the virtual camera
                video_sender->sendVideoFrame((char*)buffer,
                                             codec_ctx->width,
                                             codec_ctx->height,
                                             frameSize,
                                             rotation,
                                             format);
            }
        }
        av_packet_unref(&packet);
    }

    // Cleanup
    av_free(buffer);
    sws_freeContext(sws_ctx);
    av_frame_free(&frame);
    av_frame_free(&frame_i420);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);
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
}
