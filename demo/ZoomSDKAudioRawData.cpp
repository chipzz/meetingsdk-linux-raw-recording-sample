//GetAudioRawData
#include "rawdata/rawdata_audio_helper_interface.h"
#include "ZoomSDKAudioRawData.h"
#include "Serialisation.h"

std::string ToJSONString(AudioRawData *audioRawData)
{
	if (!audioRawData)
		return "null";

	std::string t("{ \"canAddRef\": ");
	t.append(ToJSONString(audioRawData->CanAddRef()));
	t.append(", \"bufferLen\": ");
	t.append(ToJSONString(audioRawData->GetBufferLen()));
	t.append(", \"sampleRate\": ");
	t.append(ToJSONString(audioRawData->GetSampleRate()));
	t.append(", \"channelNum\": ");
	t.append(ToJSONString(audioRawData->GetChannelNum()));
	t.append(", \"buffer\": ");
	t.append("null");
	t.append(" }");
	return t;
}

std::string ToString(AudioRawData *audioRawData)
{
	return ToJSONString(audioRawData);
}

#include "Log.h"
#include "zoom_sdk_def.h" 
#include <iostream>
#include <fstream>

void ZoomSDKAudioRawData::onOneWayAudioRawDataReceived(AudioRawData* audioRawData, uint32_t node_id)
{
	LOG_CALLBACK("ZoomSDKAudioRawData", "onOneWayAudioRawDataReceived", audioRawData, node_id);
}

void ZoomSDKAudioRawData::onMixedAudioRawDataReceived(AudioRawData* audioRawData)
{
	LOG_CALLBACK("ZoomSDKAudioRawData", "onMixedAudioRawDataReceived", audioRawData);
	//add your code here

	static std::ofstream pcmFile;
	pcmFile.open("audio.pcm", std::ios::out | std::ios::binary | std::ios::app);

	if (!pcmFile.is_open()) {
		std::cout << "Failed to open wave file" << std::endl;
		return;
	}

	// Write the audio data to the file
	pcmFile.write((char*)audioRawData->GetBuffer(), audioRawData->GetBufferLen());
	//std::cout << "buffer length: " << audioRawData->GetBufferLen() << std::endl;
/*
	std::cout << "buffer : " << audioRawData->GetBuffer() << std::endl;
*/

	// Close the wave file
	pcmFile.close();
	pcmFile.flush();
}

void ZoomSDKAudioRawData::onShareAudioRawDataReceived(AudioRawData* data_)
{
	LOG_CALLBACK("ZoomSDKAudioRawData", "onShareAudioRawDataReceived", data_);
}

void ZoomSDKAudioRawData::onOneWayInterpreterAudioRawDataReceived(AudioRawData* data_, const zchar_t* pLanguageName)
{
	LOG_CALLBACK("ZoomSDKAudioRawData", "onOneWayInterpreterAudioRawDataReceived", data_, pLanguageName);
}
