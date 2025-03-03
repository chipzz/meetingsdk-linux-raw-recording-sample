//GetVideoRawData

#include "rawdata/rawdata_video_source_helper_interface.h"
#include "ZoomSDKRenderer.h"
#include "Serialisation.h"

std::string ToString(IZoomSDKRendererDelegate::RawDataStatus &status)
{
	switch (status)
	{
		case IZoomSDKRendererDelegate::RawDataStatus::RawData_On: return "On"; break;
		case IZoomSDKRendererDelegate::RawDataStatus::RawData_Off: return "Off"; break;
	}
	return "";
}

std::string ToJSONString(IZoomSDKRendererDelegate::RawDataStatus &status)
{
	return ToString(status); // FIXME: Should be int
}

std::string ToJSONString(YUVRawDataI420 *yuvRawDataI420)
{
	if (!yuvRawDataI420)
		return "null";

	return ToJSONString
	(
		"canAddRef", yuvRawDataI420->CanAddRef(),
		"bufferLen", yuvRawDataI420->GetBufferLen(),
		"limitedI420", yuvRawDataI420->IsLimitedI420(),
		"streamWidth", yuvRawDataI420->GetStreamWidth(),
		"streamHeight", yuvRawDataI420->GetStreamHeight(),
		"rotation", yuvRawDataI420->GetRotation(),
		"sourceID", yuvRawDataI420->GetSourceID(),
		"buffer", ""
	);
}

std::string ToString(YUVRawDataI420 *yuvRawDataI420)
{
	return ToJSONString(yuvRawDataI420);
}

#include "Log.h"
#include "zoom_sdk_def.h" 
#include <iostream>


#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstdio>

void ZoomSDKRenderer::onRawDataFrameReceived(YUVRawDataI420* data)
{
	LOG_CALLBACK("ZoomSDKRenderer", "onRawDataFrameReceived", data);

	if (data->GetStreamHeight() == 720) {
		SaveToRawYUVFile(data);
	}
}

void ZoomSDKRenderer::onRawDataStatusChanged(RawDataStatus status)
{
	LOG_CALLBACK("ZoomSDKRenderer", "onRawDataStatusChanged", status);
}

void ZoomSDKRenderer::onRendererBeDestroyed()
{
	LOG_CALLBACK("ZoomSDKRenderer", "onRendererBeDestroyed");
}

void ZoomSDKRenderer::SaveToRawYUVFile(YUVRawDataI420* data)
{
	//method 1

/*
	// Open the file for writing
	std::ofstream outputFile("output.yuv", std::ios::out | std::ios::binary | std::ios::app);
	if (!outputFile.is_open())
	{
		error opening file
		return;
	}

	char* _data = new char[data->GetStreamHeight() * data->GetStreamWidth() * 3 / 2];

	memset(_data, 0, data->GetStreamHeight() * data->GetStreamWidth() * 3 / 2);

	// Copy Y buffer
	memcpy(_data, data->GetYBuffer(), data->GetStreamHeight() * data->GetStreamWidth());

	// Copy U buffer
	size_t loc = data->GetStreamHeight() * data->GetStreamWidth();
	memcpy(&_data[loc], data->GetUBuffer(), data->GetStreamHeight() * data->GetStreamWidth() / 4);

	// Copy V buffer
	loc = (data->GetStreamHeight() * data->GetStreamWidth()) + (data->GetStreamHeight() * data->GetStreamWidth() / 4);
	memcpy(&_data[loc], data->GetVBuffer(), data->GetStreamHeight() * data->GetStreamWidth() / 4);

	//outputFile.write((char*)data->GetBuffer(), data->GetBufferLen());
	// Write the Y plane
	outputFile.write(_data, data->GetStreamHeight() * data->GetStreamWidth() * 3 / 2);

	// Close the file
	outputFile.close();
	outputFile.flush();
	//cout << "YUV420 buffer saved to file." << endl;
	std::cout << "Saving Raw Data" << std::endl;
*/

	//method 2

	// Open the file for writing
	std::ofstream outputFile("output.yuv", std::ios::out | std::ios::binary | std::ios::app);
	if (!outputFile.is_open())
	{
		std::cout << "Error opening file." << std::endl;
		return;
	}
	// Calculate the sizes for Y, U, and V components
	size_t ySize = data->GetStreamWidth() * data->GetStreamHeight();
	size_t uvSize = ySize / 4;

	// Write Y, U, and V components to the output file
	outputFile.write(data->GetYBuffer(), ySize);
	outputFile.write(data->GetUBuffer(), uvSize);
	outputFile.write(data->GetVBuffer(), uvSize);

	// Close the file
	outputFile.close();
	outputFile.flush();
	//cout << "YUV420 buffer saved to file." << endl;
}
