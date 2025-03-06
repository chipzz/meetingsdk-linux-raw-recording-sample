#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <sstream>
#include <thread>
#include <sys/syscall.h>
#include <sys/syscall.h>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <map>
#include <algorithm>

#include "zoom_sdk.h"
#include "auth_service_interface.h"
#include "meeting_service_interface.h"
#include "meeting_service_components/meeting_audio_interface.h"
#include "meeting_service_components/meeting_participants_ctrl_interface.h"
#include "meeting_service_components/meeting_video_interface.h"
#include "setting_service_interface.h"

//used to accept prompts
#include "MeetingReminderEventListener.h"
//used to listen to callbacks from meeting related matters
#include "MeetingServiceEventListener.h"
//used to listen to callbacks from authentication related matters
#include "AuthServiceEventListener.h"
//used for connection helper
#include "NetworkConnectionHandler.h"

//used for event listener
#include "MeetingParticipantsCtrlEventListener.h"
#include "MeetingRecordingCtrlEventListener.h"

//references for GetVideoRawData
#include "ZoomSDKRenderer.h"
#include "rawdata/rawdata_renderer_interface.h"
#include "rawdata/zoom_rawdata_api.h"

//references for GetAudioRawData
#include "ZoomSDKAudioRawData.h"
#include "meeting_service_components/meeting_recording_interface.h"

//references for SendVideoRawData
#include "ZoomSDKVideoSource.h"

//references for SendAudioRawData
#include "ZoomSDKVirtualAudioMicEvent.h"

#include <mutex>

USING_ZOOM_SDK_NAMESPACE

//references for SendAudioRawData
std::string DEFAULT_AUDIO_SOURCE = "yourwavefile.wav";

//references for SendVideoRawData
std::string DEFAULT_VIDEO_SOURCE = "yourmp4file.mp4";

GMainLoop* loop;

//These are needed to readsettingsfromTEXT named config.txt
std::string meeting_number, token, meeting_password, recording_token;

//Services which are needed to initialize, authenticate and configure settings for the SDK
IAuthService* m_pAuthService;
IMeetingService* m_pMeetingService;
ISettingService* m_pSettingService;
INetworkConnectionHelper* network_connection_helper;

//references for GetVideoRawData
ZoomSDKRendererDelegate *zoomSDKRendererDelegate = new ZoomSDKRendererDelegate();
IZoomSDKRenderer* ZoomSDKRenderer;
IMeetingRecordingController* m_pRecordController;
IMeetingParticipantsController* m_pParticipantsController;

//references for GetAudioRawData
ZoomSDKAudioRawData* audio_source = new ZoomSDKAudioRawData();
IZoomSDKAudioRawDataHelper* audioHelper;

//this is used to get a userID, there is no specific proper logic here. It just gets the first userID.
//userID is needed for video subscription.
unsigned int userID;

//this will enable or disable logic to get raw video and raw audio
//do note that this will be overwritten by config.txt
bool GetVideoRawData = true;
bool GetAudioRawData = true;
bool SendVideoRawData = false;
bool SendAudioRawData = false;

//this is a helper method to get the first User ID, it is just an arbitary UserID
uint32_t getUserID() {
	m_pParticipantsController = m_pMeetingService->GetMeetingParticipantsController();
	int returnvalue = m_pParticipantsController->GetParticipantsList()->GetItem(0);
	std::cout << "UserID is : " << returnvalue << std::endl;
	return returnvalue;
}

IUserInfo* getMyself() {
	m_pParticipantsController = m_pMeetingService->GetMeetingParticipantsController();
	IUserInfo* returnvalue = m_pParticipantsController->GetMySelfUser();
	//std::cout << "UserID is : " << returnvalue << std::endl;
	return returnvalue;
}

//this is a helper method to get the first User Object, it is just an arbitary User Object
IUserInfo* getUserObj() {
	m_pParticipantsController = m_pMeetingService->GetMeetingParticipantsController();
	int userID = m_pParticipantsController->GetParticipantsList()->GetItem(0);
	IUserInfo* returnvalue = m_pParticipantsController->GetUserByUserID(userID);
	std::cout << "UserID is : " << returnvalue << std::endl;
	return returnvalue;
}

std::ostream &operator<<(std::ostream &os, SDKError &err)
{
	switch (err)
	{
		case SDKERR_SUCCESS: os << "SUCCESS"; break;
		case SDKERR_NO_IMPL: os << "NO_IMPL"; break;
		case SDKERR_WRONG_USAGE: os << "WRONG_USAGE"; break;
		case SDKERR_INVALID_PARAMETER: os << "INVALID_PARAMETER"; break;
		case SDKERR_MODULE_LOAD_FAILED: os << "MODULE_LOAD_FAILED"; break;
		case SDKERR_MEMORY_FAILED: os << "MEMORY_FAILED"; break;
		case SDKERR_SERVICE_FAILED: os << "SERVICE_FAILED"; break;
		case SDKERR_UNINITIALIZE: os << "UNINITIALIZE"; break;
		case SDKERR_UNAUTHENTICATION: os << "UNAUTHENTICATION"; break;
		case SDKERR_NORECORDINGINPROCESS: os << "NORECORDINGINPROCESS"; break;
		case SDKERR_TRANSCODER_NOFOUND: os << "TRANSCODER_NOFOUND"; break;
		case SDKERR_VIDEO_NOTREADY: os << "VIDEO_NOTREADY"; break;
		case SDKERR_NO_PERMISSION: os << "NO_PERMISSION"; break;
		case SDKERR_UNKNOWN: os << "UNKNOWN"; break;
		case SDKERR_OTHER_SDK_INSTANCE_RUNNING: os << "OTHER_SDK_INSTANCE_RUNNING"; break;
		case SDKERR_INTERNAL_ERROR: os << "INTERNAL_ERROR"; break;
		case SDKERR_NO_AUDIODEVICE_ISFOUND: os << "NO_AUDIODEVICE_ISFOUND"; break;
		case SDKERR_NO_VIDEODEVICE_ISFOUND: os << "NO_VIDEODEVICE_ISFOUND"; break;
		case SDKERR_TOO_FREQUENT_CALL: os << "TOO_FREQUENT_CALL"; break;
		case SDKERR_FAIL_ASSIGN_USER_PRIVILEGE: os << "FAIL_ASSIGN_USER_PRIVILEGE"; break;
		case SDKERR_MEETING_DONT_SUPPORT_FEATURE: os << "MEETING_DONT_SUPPORT_FEATURE"; break;
		case SDKERR_MEETING_NOT_SHARE_SENDER: os << "MEETING_NOT_SHARE_SENDER"; break;
		case SDKERR_MEETING_YOU_HAVE_NO_SHARE: os << "MEETING_YOU_HAVE_NO_SHARE"; break;
		case SDKERR_MEETING_VIEWTYPE_PARAMETER_IS_WRONG: os << "MEETING_VIEWTYPE_PARAMETER_IS_WRONG"; break;
		case SDKERR_MEETING_ANNOTATION_IS_OFF: os << "MEETING_ANNOTATION_IS_OFF"; break;
		case SDKERR_SETTING_OS_DONT_SUPPORT: os << "SETTING_OS_DONT_SUPPORT"; break;
		case SDKERR_EMAIL_LOGIN_IS_DISABLED: os << "EMAIL_LOGIN_IS_DISABLED"; break;
		case SDKERR_HARDWARE_NOT_MEET_FOR_VB: os << "HARDWARE_NOT_MEET_FOR_VB"; break;
		case SDKERR_NEED_USER_CONFIRM_RECORD_DISCLAIMER: os << "NEED_USER_CONFIRM_RECORD_DISCLAIMER"; break;
		case SDKERR_NO_SHARE_DATA: os << "NO_SHARE_DATA"; break;
		case SDKERR_SHARE_CANNOT_SUBSCRIBE_MYSELF: os << "SHARE_CANNOT_SUBSCRIBE_MYSELF"; break;
		case SDKERR_NOT_IN_MEETING: os << "NOT_IN_MEETING"; break;
		case SDKERR_NOT_JOIN_AUDIO: os << "NOT_JOIN_AUDIO"; break;
		case SDKERR_HARDWARE_DONT_SUPPORT: os << "HARDWARE_DONT_SUPPORT"; break;
		case SDKERR_DOMAIN_DONT_SUPPORT: os << "DOMAIN_DONT_SUPPORT"; break;
		case SDKERR_MEETING_REMOTE_CONTROL_IS_OFF: os << "MEETING_REMOTE_CONTROL_IS_OFF"; break;
		case SDKERR_FILETRANSFER_ERROR: os << "FILETRANSFER_ERROR"; break;
		default: ;
	}
	return os;
}

//check if you have permission to start raw recording
void CheckAndStartRawRecording(bool isVideo, bool isAudio)
{
	if (!isVideo && !isAudio)
		return;

	m_pRecordController = m_pMeetingService->GetMeetingRecordingController();
	SDKError err(SDKERR_SUCCESS);

	if ((err = m_pRecordController->CanStartRawRecording()) != SDKERR_SUCCESS)
	{
		std::cout << "Cannot start raw recording: no permissions yet, need host, co-host, or recording privilege " << err << std::endl;
		return;
	}

	if ((err = m_pRecordController->StartRawRecording()) != SDKERR_SUCCESS)
	{
		std::cout << "Error occurred starting raw recording" << err << std::endl;
		return;
	}

	//GetVideoRawData
	if (isVideo)
	{
		if ((err = createRenderer(&ZoomSDKRenderer, zoomSDKRendererDelegate)) != SDKERR_SUCCESS)
		{
			std::cout << "Error occurred " << err << std::endl;
			// Handle error
		}
		else
		{
			std::cout << "attemptToStartRawRecording : subscribing" << std::endl;
			ZoomSDKRenderer->setRawDataResolution(ZoomSDKResolution_720P);
			// FIXME: check return for error?
			ZoomSDKRenderer->subscribe(getUserID(), RAW_DATA_TYPE_VIDEO);
		}
	}
	//GetAudioRawData
	if (isAudio)
	{
		audioHelper = GetAudioRawdataHelper();
		if (audioHelper)
		{
			if ((err = audioHelper->subscribe(audio_source)) != SDKERR_SUCCESS)
				std::cout << "Error occurred subscribing to audio: " << err << std::endl;
		}
		else
			std::cout << "Error getting audioHelper" << std::endl;
	}
}

//check if you meet the requirements to send raw data
void CheckAndStartRawSending(bool isVideo, bool isAudio) {
	std::cout << "CheckAndStartRawSending(" << isVideo << ", " << isAudio << ")" << std::endl;

	//SendVideoRawData
	if (isVideo) {
		ZoomSDKVideoSource* virtual_camera_video_source = new ZoomSDKVideoSource(DEFAULT_VIDEO_SOURCE);
		IZoomSDKVideoSourceHelper* p_videoSourceHelper = GetRawdataVideoSourceHelper();

		if (p_videoSourceHelper) {
			SDKError err = p_videoSourceHelper->setExternalVideoSource(virtual_camera_video_source);

			if (err != SDKERR_SUCCESS) {
				printf("attemptToStartRawVideoSending(): Failed to set external video source, error code: %d\n", err);
			}
			else {
				printf("attemptToStartRawVideoSending(): Success \n");
				IMeetingVideoController* meetingController = m_pMeetingService->GetMeetingVideoController();
				meetingController->UnmuteVideo();
			}
		}
		else {
			printf("attemptToStartRawVideoSending(): Failed to get video source helper\n");
		}
	}

	//SendAudioRawData
	if (isAudio) {
		ZoomSDKVirtualAudioMicEvent* audio_source = new ZoomSDKVirtualAudioMicEvent(DEFAULT_AUDIO_SOURCE);
		IZoomSDKAudioRawDataHelper* audioHelper = GetAudioRawdataHelper();
		if (audioHelper) {
			SDKError err = audioHelper->setExternalAudioSource(audio_source);
		}
	}
}

//callback when given host permission
void onIsHost() {
	printf("Is host now...\n");
	CheckAndStartRawRecording(GetVideoRawData, GetAudioRawData);
}

//callback when given cohost permission
void onIsCoHost() {
	printf("Is co-host now...\n");
	CheckAndStartRawRecording(GetVideoRawData, GetAudioRawData);
}
//callback when given recording permission
void onIsGivenRecordingPermission() {
	printf("Is given recording permissions now...\n");
	CheckAndStartRawRecording(GetVideoRawData, GetAudioRawData);
}

void turnOnSendVideoAndAudio() {
	//testing WIP
	if (SendVideoRawData) {
		IMeetingVideoController* meetingVidController = m_pMeetingService->GetMeetingVideoController();
		meetingVidController->UnmuteVideo();
	}
	//testing WIP
	if (SendAudioRawData) {
		IMeetingAudioController* meetingAudController = m_pMeetingService->GetMeetingAudioController();
		meetingAudController->JoinVoip();
		printf("Is my audio muted: %d\n", getMyself()->IsAudioMuted());
		meetingAudController->UnMuteAudio(getMyself()->GetUserID());
	}
}
void turnOffSendVideoandAudio() {
	//testing WIP
	if (SendVideoRawData) {
		IMeetingVideoController* meetingVidController = m_pMeetingService->GetMeetingVideoController();
		meetingVidController->MuteVideo();
	}
	//testing WIP
	if (SendAudioRawData) {
		IMeetingAudioController* meetingAudController = m_pMeetingService->GetMeetingAudioController();
		meetingAudController->MuteAudio(getMyself()->GetUserID(), true);

	}
}


bool useCamera;

//callback when the SDK is inmeeting
void onInMeeting()
{
	printf("onInMeeting Invoked\n");

	//double check if you are in a meeting
	if (m_pMeetingService->GetMeetingStatus() == MEETING_STATUS_INMEETING) {
		printf("In Meeting Now...\n");

		//print all list of participants
		IList<unsigned int>* participants = m_pMeetingService->GetMeetingParticipantsController()->GetParticipantsList();
		printf("Participants count: %d\n", participants->GetCount());
	}

	//first attempt to start raw recording  / sending, upon successfully joined and achieved "in-meeting" state.
	CheckAndStartRawRecording(GetVideoRawData, GetAudioRawData);
	SendVideoRawData = !useCamera;
	SendAudioRawData = !useCamera;
	CheckAndStartRawSending(SendVideoRawData, SendAudioRawData);
}

//on meeting ended, typically by host, do something here. it is possible to reuse this SDK instance
void onMeetingEndsQuitApp() {
	// CleanSDK();
	//std::exit(0);
}

void onMeetingJoined() {

	printf("Joining Meeting...\n");
}

//get path, helper method used to read json config file
std::string getSelfDirPath()
{
	char dest[PATH_MAX];
	memset(dest, 0, sizeof(dest)); // readlink does not null terminate!
	if (readlink("/proc/self/exe", dest, PATH_MAX) == -1)
	{
	}
	char* tmp = strrchr(dest, '/');
	if (tmp)
		*tmp = 0;
/*
	printf("getpath\n");
*/
	return std::string(dest);
}

// Function to process a line containing a key-value pair
void processLine(const std::string& line, std::map<std::string, std::string>& config) {
	// Find the position of the ':' character
	size_t colonPos = line.find(':');

	if (colonPos != std::string::npos) {
		// Extract the key and value parts
		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);

		// Remove leading/trailing whitespaces from the key and value
		key.erase(0, key.find_first_not_of(" \t"));
		key.erase(key.find_last_not_of(" \t") + 1);
		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t") + 1);

		// Remove double-quote characters and carriage return ('\r') from the value
		value.erase(std::remove_if(value.begin(), value.end(), [](char c) { return c == '"' || c == '\r'; }), value.end());

		// Store the key-value pair in the map
		config[key] = value;
	}
}

void ReadTEXTSettings()
{
	
	std::string self_dir = getSelfDirPath();
/*
	printf("self path: %s\n", self_dir.c_str());
*/
	self_dir.append("/config.txt");

	std::ifstream configFile(self_dir.c_str());
	if (!configFile) {
		std::cerr << "Error opening config file." << std::endl;
	}
	else{

/*
		std::cerr << "Readfile success." << std::endl;
*/
	}

	std::map<std::string, std::string> config;
	std::string line;

	while (std::getline(configFile, line)) {
		// Process each line to extract key-value pairs
		processLine(line, config);
		
/*
		std::cerr << "Reading.." << line <<std::endl;
*/
	}

	// Example: Accessing values by key
	if (config.find("meeting_number") != config.end()) {
		
		meeting_number=config["meeting_number"];
/*
		std::cout << "Meeting Number: " << config["meeting_number"] << std::endl;
*/
	}
	if (config.find("token") != config.end()) {
		 token=config["token"];
/*
		 	std::cout << "Token: " << token<< std::endl;
*/
	}
	if (config.find("meeting_password") != config.end()) {
		
		meeting_password=config["meeting_password"];
/*
		std::cout << "meeting_password: " << meeting_password << std::endl;
*/
	}
	if (config.find("recording_token") != config.end()) {
	
		 recording_token=config["recording_token"];
		 	std::cout << "recording_token: " << recording_token << std::endl;
	}
	if (config.find("GetVideoRawData") != config.end()) {
		std::cout << "GetVideoRawData before parsing is : " << config["GetVideoRawData"]   << std::endl;
		
	if (config["GetVideoRawData"] == "true"){
			GetVideoRawData=true;
		}
		else{
			GetVideoRawData=false;
		}
		std::cout << "GetVideoRawData: " << GetVideoRawData << std::endl;
	}
	if (config.find("GetAudioRawData") != config.end()) {
		std::cout << "GetAudioRawData before parsing is : " << config["GetAudioRawData"]  << std::endl;
		
		if (config["GetAudioRawData"] == "true"){
			GetAudioRawData=true;
		}
		else{
			GetAudioRawData=false;
		}
		std::cout << "GetAudioRawData: " << GetAudioRawData << std::endl;
	}

	if (config.find("SendVideoRawData") != config.end()) {
		std::cout << "SendVideoRawData before parsing is : " << config["SendVideoRawData"] << std::endl;

		if (config["SendVideoRawData"] == "true") {
			SendVideoRawData = true;
		}
		else {
			SendVideoRawData = false;
		}
		std::cout << "SendVideoRawData: " << SendVideoRawData << std::endl;
	}
	if (config.find("SendAudioRawData") != config.end()) {
		std::cout << "SendAudioRawData before parsing is : " << config["SendAudioRawData"] << std::endl;

		if (config["SendAudioRawData"] == "true") {
			SendAudioRawData = true;
		}
		else {
			SendAudioRawData = false;
		}
		std::cout << "SendAudioRawData: " << SendAudioRawData << std::endl;
	}

	// Additional processing or handling of parsed values can be done here

/*
	printf("directory of config file: %s\n", self_dir.c_str());
*/

}

void CleanSDK()
{
	SDKError err(SDKERR_SUCCESS);

	if (m_pAuthService)
	{
		std::cerr << "DestroyAuthService";
		IAuthService *const pAuthService = m_pAuthService;
		m_pAuthService = NULL;
		if ((err = DestroyAuthService(pAuthService)) != SDKERR_SUCCESS)
			std::cerr << " " << err;
		std::cerr << std::endl;
	}
	if (m_pSettingService)
	{
		std::cerr << "DestroySettingService";
		ISettingService *const pSettingService = m_pSettingService;
		m_pSettingService = NULL;
		if ((err = DestroySettingService(pSettingService)) != SDKERR_SUCCESS)
			std::cerr << " " << err;
		std::cerr << std::endl;
	}
	if (m_pMeetingService)
	{
		std::cerr << "DestroyMeetingService";
		IMeetingService *const pMeetingService = m_pMeetingService;
		m_pMeetingService = NULL;
		if ((err = DestroyMeetingService(pMeetingService)) != SDKERR_SUCCESS)
			std::cerr << " " << err;
		std::cerr << std::endl;
	}
	if (ZoomSDKRenderer)
	{
		std::cerr << "ZoomSDKRenderer.unSubscribe()" << std::endl;
		ZoomSDKRenderer->unSubscribe();
	}
	if (audioHelper)
	{
		std::cerr << "Unsubscribing audioHelper" << std::endl;
		audioHelper->unSubscribe();
	}
	/*
	if (network_connection_helper)
	{
		DestroyNetworkConnectionHelper(network_connection_helper);
		network_connection_helper = NULL;
	}
	*/
	//attempt to clean up SDK
	if ((err = CleanUPSDK()) != SDKERR_SUCCESS)
		std::cerr << "CleanUPSDK error " << err << std::endl;
}

void changeMicrophoneAndSpeaker() {
	IAudioSettingContext* pAudioContext = m_pSettingService->GetAudioSettings();
	if (pAudioContext)
	{
		//setting speaker
		//if there are speakers detected
		if (pAudioContext->GetSpeakerList()->GetCount() >= 1) {
			std::cout << "Number of speaker(s) : " << pAudioContext->GetSpeakerList()->GetCount() << std::endl;
			ISpeakerInfo* sInfo = pAudioContext->GetSpeakerList()->GetItem(0);
			const zchar_t* deviceName = sInfo->GetDeviceName();

			//set speaker
			if (deviceName != nullptr && deviceName[0] != '\0') {
				std::cout << "Speaker(0) name : " << sInfo->GetDeviceName() << std::endl;
				std::cout << "Speaker(0) id : " << sInfo->GetDeviceId() << std::endl;
				pAudioContext->SelectSpeaker(sInfo->GetDeviceId(), sInfo->GetDeviceName());
				std::cout << "Is selected speaker? : " << pAudioContext->GetSpeakerList()->GetItem(0)->IsSelectedDevice() << std::endl;
			}
			else {
				std::cout << "Speaker(0) name is empty or null." << std::endl;
				std::cout << "Speaker(0) id is empty or null." << std::endl;
			}
		}

		//setting microphone
		//if there are microphone detected
		if (pAudioContext->GetMicList()->GetCount() >= 1) {
			IMicInfo* mInfo = pAudioContext->GetMicList()->GetItem(0);
			std::cout << "Number of mic(s) : " << pAudioContext->GetMicList()->GetCount() << std::endl;
			const zchar_t* deviceName = mInfo->GetDeviceName();

			//set microphone
			if (deviceName != nullptr && deviceName[0] != '\0') {
				std::cout << "Mic(0) name : " << mInfo->GetDeviceName() << std::endl;
				std::cout << "Mic(0) id : " << mInfo->GetDeviceId() << std::endl;
				pAudioContext->SelectMic(mInfo->GetDeviceId(), mInfo->GetDeviceName());
				std::cout << "Is selected Mic? : " << pAudioContext->GetMicList()->GetItem(0)->IsSelectedDevice() << std::endl;
			}
			else {
				std::cout << "Mic(0) name is empty or null." << std::endl;
				std::cout << "Mic(0) id is empty or null." << std::endl;
			}
		}
	}
}

void JoinMeeting()
{
	std::cerr << "Joining Meeting" << std::endl;
	SDKError err2(SDKERR_SUCCESS);

	//try to create the meetingservice object, 
	//this object will be used to join the meeting
	if ((err2 = CreateMeetingService(&m_pMeetingService)) != SDKERR_SUCCESS) {};
	std::cerr << "MeetingService created." << std::endl;

	//before joining a meeting, create the setting service
	//this object is used to for settings
	CreateSettingService(&m_pSettingService);
	std::cerr << "Settingservice created." << std::endl;

	// Set the event listener for meeting status
	m_pMeetingService->SetEvent(new MeetingServiceEventListener(&onMeetingJoined, &onMeetingEndsQuitApp, &onInMeeting));

	// Set the event listener for host, co-host 
	m_pParticipantsController = m_pMeetingService->GetMeetingParticipantsController();
	m_pParticipantsController->SetEvent(new MeetingParticipantsCtrlEventListener(&onIsHost, &onIsCoHost));

	// Set the event listener for recording privilege status
	m_pRecordController = m_pMeetingService->GetMeetingRecordingController();
	m_pRecordController->SetEvent(new MeetingRecordingCtrlEventListener(&onIsGivenRecordingPermission));

	// set event listnener for prompt handler 
	IMeetingReminderController* meetingremindercontroller = m_pMeetingService->GetMeetingReminderController();
	MeetingReminderEventListener* meetingremindereventlistener = new MeetingReminderEventListener();
	meetingremindercontroller->SetEvent(meetingremindereventlistener);

	//prepare params used for joining meeting
	JoinParam joinParam;
	SDKError err(SDKERR_SERVICE_FAILED);
	joinParam.userType = SDK_UT_WITHOUT_LOGIN;
	JoinParam4WithoutLogin& withoutloginParam = joinParam.param.withoutloginuserJoin;
	// withoutloginParam.meetingNumber = 1231231234;
	withoutloginParam.meetingNumber = std::stoull(meeting_number);
	withoutloginParam.vanityID = NULL;
	withoutloginParam.userName = "LinuxChun";
	// withoutloginParam.psw = "1";
	withoutloginParam.psw = meeting_password.c_str();
	withoutloginParam.customer_key = NULL;
	withoutloginParam.webinarToken = NULL;
	withoutloginParam.isVideoOff = false;
	withoutloginParam.isAudioOff = false;

	/*
	std::cerr << "JWT token is " << token << std::endl;
	std::cerr << "Recording token is " << recording_token << std::endl;
	*/

	//automatically set app_privilege token if it is present in config.txt, or retrieved from web service
	withoutloginParam.app_privilege_token = NULL;
	/*
	if (!recording_token.size() == 0)
	{
		withoutloginParam.app_privilege_token = recording_token.c_str();
		std::cerr << "Setting recording token" << std::endl;
	}
	else
	{
		withoutloginParam.app_privilege_token = NULL;
		std::cerr << "Leaving recording token as NULL" << std::endl;
	}
	*/

	if (GetAudioRawData)
	{
		//set join audio to true
		IAudioSettingContext* pAudioContext = m_pSettingService->GetAudioSettings();
		if (pAudioContext)
		{
			//ensure auto join audio
			pAudioContext->EnableAutoJoinAudio(true);
		}
	}
	if (SendVideoRawData)
	{
		//ensure video is turned on
		withoutloginParam.isVideoOff = false;
		//set join video to true
		IVideoSettingContext* pVideoContext = m_pSettingService->GetVideoSettings();
		if (pVideoContext)
		{
			pVideoContext->EnableAutoTurnOffVideoWhenJoinMeeting(false);
		}
	}
	if (SendAudioRawData)
	{
		IAudioSettingContext* pAudioContext = m_pSettingService->GetAudioSettings();
		if (pAudioContext)
		{
			//ensure auto join audio
			pAudioContext->EnableAutoJoinAudio(true);
			pAudioContext->EnableAlwaysMuteMicWhenJoinVoip(true);
			pAudioContext->SetSuppressBackgroundNoiseLevel(Suppress_BGNoise_Level_None);

		}
	}

	//attempt to join meeting
	if (m_pMeetingService)
	{
		err = m_pMeetingService->Join(joinParam);
	}
	else
	{
		std::cout << "join_meeting m_pMeetingService:Null" << std::endl;
	}

	if (SDKERR_SUCCESS == err)
	{
		std::cout << "join_meeting:success" << std::endl;
	}
	else
	{
		std::cout << "join_meeting:error" << std::endl;
	}
}

void LeaveMeeting()
{
	if (NULL == m_pMeetingService)
	{
		std::cout << "LeaveMeeting MeetingService = null" << std::endl;
		return;
	}

	MeetingStatus status = m_pMeetingService->GetMeetingStatus();
	if (status == MEETING_STATUS_IDLE ||
		status == MEETING_STATUS_ENDED ||
		status == MEETING_STATUS_FAILED)
	{
		std::cout << "LeaveMeeting() not in meeting, status " << status << std::endl;
		return;
	}

	std::cout << "MeetingService.Leave()" << std::endl;
	SDKError err = m_pMeetingService->Leave(LEAVE_MEETING);
	if (err != SDKERR_SUCCESS)
		std::cout << "MeetingService.Leave() error " << err << std::endl;
}

//callback when authentication is compeleted
void OnAuthenticationComplete()
{
	std::cout << "OnAuthenticationComplete" << std::endl;
	JoinMeeting();
}

void AuthMeetingSDK()
{
	SDKError err(SDKERR_SUCCESS);

	//create auth service
	if ((err = CreateAuthService(&m_pAuthService)) != SDKERR_SUCCESS)
	{
		std::cerr << "CreateAuthService() failed: " << err << std::endl;
	}

	//Create a param to insert jwt token
	AuthContext param;

	//set the event listener for onauthenticationcompleted
	if ((err = m_pAuthService->SetEvent(new AuthServiceEventListener(&OnAuthenticationComplete))) != SDKERR_SUCCESS)
	{
		std::cerr << "AuthService.SetEvent(AuthServiceEventListener) failed: " << err << std::endl;
	}
	std::cout << "AuthServiceEventListener added." << std::endl;

	if (!token.size() == 0){
		param.jwt_token = token.c_str();
		std::cerr << "AuthSDK:token extracted from config file " <<param.jwt_token  << std::endl;
	}
	if ((err = m_pAuthService->SDKAuth(param)) != SDKERR_SUCCESS)
	{
		std::cerr << "AuthService.SDKAuth({ jwt_token: " << param.jwt_token << " }) failed: " << err << std::endl;
		LeaveMeeting();
		CleanSDK();
		exit(1);
	}
	/*
	if (const IZoomLastError *lerr = GetZoomLastError())
		printf("Error %p\n", lerr->GetErrorDescription());
	*/

	//if (SDKERR_SUCCESS != sdkErrorResult){
	//	std::cerr << "AuthSDK:error " << std::endl;
	//}
	//else{
	//	std::cerr << "AuthSDK:send success, awaiting callback " << std::endl;
	//}
}

const char *const proxy = "127.0.0.1:8080";

void InitMeetingSDK()
{
	SDKError err(SDKERR_SUCCESS);
	InitParam initParam;

	// set domain
	initParam.strWebDomain = "https://zoom.us";
	initParam.strSupportUrl = "https://zoom.us";

	// set language id
	initParam.emLanguageID = LANGUAGE_English;

	//set logging perferences
	initParam.enableLogByDefault = true;
	initParam.enableGenerateDump = true;

	// attempt to initialize
	err = InitSDK(initParam);
	if (err != SDKERR_SUCCESS){
		std::cerr << "Init meetingSdk:error " << std::endl;
	}
	else{
		std::cerr << "Init meetingSdk:success" << std::endl;
	}

	//use connection helper
	if ((err = CreateNetworkConnectionHelper(&network_connection_helper)) == SDKERR_SUCCESS) {
		std::cout << "CreateNetworkConnectionHelper created." << std::endl;
	}
	if ((err = network_connection_helper->RegisterNetworkConnectionHandler(new NetworkConnectionHandler(&AuthMeetingSDK))) == SDKERR_SUCCESS) {
		std::cout << "NetworkConnectionHelper registering NetworkConnectionHandler." << std::endl;
	}
	/*
	ProxySettings proxy_setting;
	proxy_setting.proxy = proxy;
//	proxy_setting.auto_detect = true;
	if ((err = network_connection_helper->ConfigureProxy(proxy_setting)) == SDKERR_SUCCESS) {
		std::cout << "Proxy configured" << std::endl;
	}
	else
		std::cerr << err << endl;
	*/
}

//used for non headless app 
void StartMeeting()
{
	StartParam startParam;
	startParam.userType = SDK_UT_NORMALUSER;
	startParam.param.normaluserStart.vanityID = NULL;
	startParam.param.normaluserStart.customer_key = NULL;
	startParam.param.normaluserStart.isVideoOff = false;
	startParam.param.normaluserStart.isAudioOff = false;

	SDKError err = m_pMeetingService->Start(startParam);
	if (SDKERR_SUCCESS == err)
	{
		std::cerr << "StartMeeting:success " << std::endl;
	}
	else
	{
		std::cerr << "StartMeeting:error " << std::endl;
	}
}

// Define a struct to hold the response data
struct ResponseData {
	std::ostringstream stream;
};

// Callback function to write response data into the stringstream
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t totalSize = size * nmemb;
	ResponseData* response = static_cast<ResponseData*>(userp);
	response->stream.write(static_cast<const char*>(contents), totalSize);
	return totalSize;
}

gboolean timeout_callback(gpointer data)
{
	return TRUE;
}

//this catches a break signal, such as Ctrl + C
void my_handler(int s)
{
	printf("\nCaught signal %d\n", s);
	LeaveMeeting();
	printf("Leaving session.\n");
	CleanSDK();

	std::exit(0);
}

void initAppSettings()
{
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
}

int main(int argc, char* argv[])
{
	useCamera = (argc > 1);
	ReadTEXTSettings();
	InitMeetingSDK();
	AuthMeetingSDK();
	initAppSettings();
	loop = g_main_loop_new(NULL, FALSE);
	// add source to default context
	g_timeout_add(1000, timeout_callback, loop);
	g_main_loop_run(loop);
	return 0;
}
