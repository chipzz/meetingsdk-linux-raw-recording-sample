cmake_minimum_required(VERSION 3.16)

project(meetingSDKDemo CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif()

find_package(PkgConfig REQUIRED)

# Find GLib using pkg-config
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET libavutil libavcodec libavformat libswscale)

find_path(MEETINGSDK_INCLUDEDIR zoom_sdk.h PATHS . PATH_SUFFIXES include/h REQUIRED)
#find_path(MEETINGSDK_LIBDIR libmeetingsdk.so PATHS . PATH_SUFFIXES lib/zoom_meeting_sdk REQUIRED)
find_library(MEETINGSDK_LIBRARIES NAMES meetingsdk PATHS . PATH_SUFFIXES lib/zoom_meeting_sdk REQUIRED)

add_library(meetingsdk SHARED IMPORTED)
set_target_properties(meetingsdk PROPERTIES IMPORTED_LOCATION ${MEETINGSDK_LIBRARIES})
target_include_directories(meetingsdk INTERFACE ${MEETINGSDK_INCLUDEDIR})

add_executable(meetingSDKDemo 
              ${CMAKE_SOURCE_DIR}/meeting_sdk_demo.cpp
              ${CMAKE_SOURCE_DIR}/MeetingReminderEventListener.h
              ${CMAKE_SOURCE_DIR}/MeetingReminderEventListener.cpp
              ${CMAKE_SOURCE_DIR}/MeetingServiceEventListener.h
              ${CMAKE_SOURCE_DIR}/MeetingServiceEventListener.cpp
              ${CMAKE_SOURCE_DIR}/NetworkConnectionHandler.h
              ${CMAKE_SOURCE_DIR}/NetworkConnectionHandler.cpp
              ${CMAKE_SOURCE_DIR}/AuthServiceEventListener.h
              ${CMAKE_SOURCE_DIR}/AuthServiceEventListener.cpp
              ${CMAKE_SOURCE_DIR}/MeetingParticipantsCtrlEventListener.h
              ${CMAKE_SOURCE_DIR}/MeetingParticipantsCtrlEventListener.cpp
              ${CMAKE_SOURCE_DIR}/MeetingRecordingCtrlEventListener.h
              ${CMAKE_SOURCE_DIR}/MeetingRecordingCtrlEventListener.cpp
              ${CMAKE_SOURCE_DIR}/ZoomSDKRenderer.h
              ${CMAKE_SOURCE_DIR}/ZoomSDKRenderer.cpp
              ${CMAKE_SOURCE_DIR}/ZoomSDKAudioRawData.h
              ${CMAKE_SOURCE_DIR}/ZoomSDKAudioRawData.cpp
              ${CMAKE_SOURCE_DIR}/ZoomSDKVideoSource.h
              ${CMAKE_SOURCE_DIR}/ZoomSDKVideoSource.cpp
              ${CMAKE_SOURCE_DIR}/ZoomSDKVirtualAudioMicEvent.h
              ${CMAKE_SOURCE_DIR}/ZoomSDKVirtualAudioMicEvent.cpp
	Log.cpp
              )

# Link GLib libraries
target_link_libraries(meetingSDKDemo PkgConfig::GLIB)
target_link_libraries(meetingSDKDemo PkgConfig::FFmpeg)
target_link_libraries(meetingSDKDemo meetingsdk)

string(LENGTH "${CMAKE_SOURCE_DIR}/" CMAKE_SOURCE_DIR_LENGTH)
target_compile_definitions(meetingSDKDemo PRIVATE "CMAKE_SOURCE_DIR_LENGTH=${CMAKE_SOURCE_DIR_LENGTH}")

# Create a symbolic link
file(CREATE_LINK "${CMAKE_SOURCE_DIR}/lib/zoom_meeting_sdk/libmeetingsdk.so" "${CMAKE_SOURCE_DIR}/lib/zoom_meeting_sdk/libmeetingsdk.so.1" SYMBOLIC)

configure_file(${CMAKE_SOURCE_DIR}/config.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.txt COPYONLY)

file(COPY ${CMAKE_SOURCE_DIR}/lib/zoom_meeting_sdk/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
