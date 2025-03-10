# Use the official Ubuntu image as the base image
FROM ubuntu:22.04 AS build

RUN	\
	apt-get update \
# Install build dependencies \
&&	apt-get install -y --no-install-recommends --no-install-suggests \
	libdbus-1-3 \
	libxcb-shape0 libxcb-randr0 libxcb-image0 libxcb-keysyms1 libxcb-xtest0 \
	libgbm1 libgl1 \
	g++ make binutils cmake pkgconf \
	libglib2.0-dev \
	libswscale-dev libavutil-dev libavcodec-dev libavformat-dev \
&&	apt-get clean \
&&	rm -fR /var/lib/apt/lists /var/lib/dpkg/status-old

# Specify the run script as the CMD
CMD ["/app/run.sh"]

RUN mkdir /app

# Define a shell script to run multiple commands
RUN	\
	echo '#!/bin/sh' > /app/run.sh \
&&	echo '/app/demo/setup-pulseaudio.sh' >> /app/run.sh \
&&	echo 'cd /app/demo' >> /app/run.sh \
&&	echo 'exec /app/demo/bin/meetingSDKDemo' >> /app/run.sh

# Make the run script executable
RUN chmod +x /app/run.sh

# Copy your application files to the container
COPY demo/ /app/demo/

RUN chmod +x /app/demo/setup-pulseaudio.sh

WORKDIR /app/demo

# Build
RUN	\
	rm -rf bin build \
&&	cmake -B build -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=../bin \
&&	cmake --build build

RUN	\
	rm -fR build CMakeLists.txt Makefile *.cpp *.h include bin/libmeetingsdk.so* bin/qt_libs config-txt-update \
# Remove build dependencies \
&&	apt-get -y purge \
	g++ make binutils cmake pkgconf \
	libglib2.0-dev \
	libswscale-dev libavutil-dev libavcodec-dev libavformat-dev \
&&	apt-get -y --purge autoremove

FROM ubuntu:22.04 AS run

RUN	\
	apt-get update \
# Install run-time dependencies \
&&	apt-get install -y --no-install-recommends --no-install-suggests \
	pulseaudio pulseaudio-utils \
	libglib2.0-0 \
	libdbus-1-3 \
	libxcb-shape0 libxcb-randr0 libxcb-image0 libxcb-keysyms1 libxcb-xtest0 \
	libgbm1 libgl1 \
	libswscale5 libavutil56 libavcodec58 libavformat58 libatomic1 \
&&	apt-get clean \
&&	rm -fR /var/lib/apt/lists /var/lib/dpkg/status-old

COPY --from=build /app /app
CMD ["/app/demo/bin/meetingSDKDemo"]

# Crap? Keep for now

#	libegl-mesa0 \
#	libx11-xcb1 \
#	libxcb-xfixes0 \
#	libxcb-shm0 \
#	libxfixes3 \
#	libdrm2 \
#	libgssapi-krb5-2 \
#	openssl \
#	ca-certificates \
#	pkg-config \
#	libsdl2-dev \
#	g++-multilib \
#&&	apt-get install -y libcurl4-openssl-dev

#RUN apt-get install -y libasound2 libasound2-plugins alsa alsa-utils alsa-oss