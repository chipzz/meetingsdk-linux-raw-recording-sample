#include "MeetingServiceEventListener.h"
#include "Serialisation.h"

std::string ToString(MeetingStatus &status)
{
	switch (status)
	{
		case MEETING_STATUS_IDLE: return "IDLE"; break;
		case MEETING_STATUS_CONNECTING: return "CONNECTING"; break;
		case MEETING_STATUS_WAITINGFORHOST: return "WAITINGFORHOST"; break;
		case MEETING_STATUS_INMEETING: return "INMEETING"; break;
		case MEETING_STATUS_DISCONNECTING: return "DISCONNECTING"; break;
		case MEETING_STATUS_RECONNECTING: return "RECONNECTING"; break;
		case MEETING_STATUS_FAILED: return "FAILED"; break;
		case MEETING_STATUS_ENDED: return "ENDED"; break;
		case MEETING_STATUS_UNKNOWN: return "UNKNOWN"; break;
		case MEETING_STATUS_LOCKED: return "LOCKED"; break;
		case MEETING_STATUS_UNLOCKED: return "UNLOCKED"; break;
		case MEETING_STATUS_IN_WAITING_ROOM: return "IN_WAITING_ROOM"; break;
		case MEETING_STATUS_WEBINAR_PROMOTE: return "WEBINAR_PROMOTE"; break;
		case MEETING_STATUS_WEBINAR_DEPROMOTE: return "WEBINAR_DEPROMOTE"; break;
		case MEETING_STATUS_JOIN_BREAKOUT_ROOM: return "JOIN_BREAKOUT_ROOM"; break;
		case MEETING_STATUS_LEAVE_BREAKOUT_ROOM: return "LEAVE_BREAKOUT_ROOM"; break;
//		default: ;
	}
	return "";
}

std::string ToJSONString(MeetingStatus &status)
{
	return ToString(status); // FIXME: Should be int
}

std::string ToString(MeetingType &type)
{
	switch (type)
	{
		case MEETING_TYPE_NONE: return "NONE"; break;
		case MEETING_TYPE_NORMAL: return "NORMAL"; break;
		case MEETING_TYPE_WEBINAR: return "WEBINAR"; break;
		case MEETING_TYPE_BREAKOUTROOM: return "BREAKOUTROOM"; break;
	}
	return "";
}

std::string ToJSONString(MeetingType &type)
{
	return ToString(type);
}

std::string ToJSONString(const MeetingParameter *meeting_param)
{
	if (!meeting_param)
		return "null";

	std::string t("{ \"meeting_number\": ");
	t.append(ToJSONString(meeting_param->meeting_number));
	t.append(", \"meeting_type\": ");
	t.append(ToJSONString(meeting_param->meeting_type));
	t.append(", \"meeting_topic\": ");
	t.append(ToJSONString(meeting_param->meeting_topic));
	t.append(", \"meeting_host\": ");
	t.append(ToJSONString(meeting_param->meeting_host));
	t.append(", \"is_view_only\": ");
	t.append(ToJSONString(meeting_param->is_view_only));
	t.append(", \"is_auto_recording_local\": ");
	t.append(ToJSONString(meeting_param->is_auto_recording_local));
	t.append(", \"is_auto_recording_cloud\": ");
	t.append(ToJSONString(meeting_param->is_auto_recording_cloud));
	t.append(" }");
	return t;
}

std::string ToString(const MeetingParameter *meeting_param)
{
	return ToJSONString(meeting_param);
}

/*
std::ostream &operator<<(std::ostream &os, MeetingStatus &status)
{
	os << ToString(status);
	return os;
}

std::ostream &operator<<(std::ostream &os, const MeetingParameter &parameter)
{
	os << parameter.meeting_number;
	return os;
}
*/

#include "Log.h"
#include <rawdata/zoom_rawdata_api.h>
#include <iostream>

MeetingServiceEventListener::MeetingServiceEventListener(void (*onMeetingStarts)(), void (*onMeetingEnds)(), void (*onInMeeting)())
{
	LOG_CALLBACK("MeetingServiceEventListener", "MeetingServiceEventListener");
	onMeetingEnds_ = onMeetingEnds;
	onMeetingStarts_ = onMeetingStarts;
	onInMeeting_ = onInMeeting;
}

void MeetingServiceEventListener::onMeetingStatusChanged(MeetingStatus status, int iResult)
{
	LOG_CALLBACK("MeetingServiceEventListener", "onMeetingStatusChanged", status, iResult);
	switch (status)
	{
	case MEETING_STATUS_IDLE:
		std::cout << "No meeting is running" << std::endl;
		break;
	case MEETING_STATUS_CONNECTING:
		std::cout << "Connecting to the meeting server" << std::endl;
		break;
	case MEETING_STATUS_WAITINGFORHOST:
		std::cout << "Waiting for the host to start the meeting" << std::endl;
		break;
	case MEETING_STATUS_INMEETING:
		std::cout << "In Meeting" << std::endl;
		if (onInMeeting_) onInMeeting_();
		break;
	case MEETING_STATUS_DISCONNECTING:
		std::cout << "Disconnecting from the meeting server, leaving meeting" << std::endl;
		break;
	case MEETING_STATUS_RECONNECTING:
		std::cout << "Reconnecting meeting server" << std::endl;
		break;
	case MEETING_STATUS_FAILED:
		std::cout << "Failed to connect the meeting server" << std::endl;
		break;
	case MEETING_STATUS_ENDED:
		std::cout << "Meeting ended" << std::endl;
		if (onMeetingEnds_) onMeetingEnds_();
		break;
	case MEETING_STATUS_UNKNOWN:
		std::cout << "Unknown status" << std::endl;
		break;
	case MEETING_STATUS_LOCKED:
		std::cout << "Meeting is locked to prevent further participants from joining the meeting" << std::endl;
		break;
	case MEETING_STATUS_UNLOCKED:
		std::cout << "Meeting is unlocked and participants can join the meeting" << std::endl;
		break;
	case MEETING_STATUS_IN_WAITING_ROOM:
		std::cout << "Participants who join the meeting before the start are in the waiting room" << std::endl;
		break;
	}
}

void MeetingServiceEventListener::onMeetingStatisticsWarningNotification(StatisticsWarningType type)
{
	LOG_CALLBACK("MeetingServiceEventListener", "onMeetingStatisticsWarningNotification", type);
}

void MeetingServiceEventListener::onMeetingParameterNotification(const MeetingParameter *meeting_param)
{
	LOG_CALLBACK("MeetingServiceEventListener", "onMeetingParameterNotification", meeting_param);
	if (onMeetingStarts_) onMeetingStarts_();
}

void MeetingServiceEventListener::onSuspendParticipantsActivities()
{
	LOG_CALLBACK("MeetingServiceEventListener", "onSuspendParticipantsActivities");
}

void MeetingServiceEventListener::onAICompanionActiveChangeNotice(bool bActive)
{
	LOG_CALLBACK("MeetingServiceEventListener", "onAICompanionActiveChangeNotice", bActive);
}

void MeetingServiceEventListener::onMeetingTopicChanged(const zchar_t* sTopic)
{
	LOG_CALLBACK("MeetingServiceEventListener", "onMeetingTopicChanged", sTopic);
}

void MeetingServiceEventListener::onMeetingFullToWatchLiveStream(const zchar_t* sLiveStreamUrl)
{
	LOG_CALLBACK("MeetingServiceEventListener", "onMeetingFullToWatchLiveStream", sLiveStreamUrl);
}
