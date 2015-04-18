#!/bin/sh

NDK=~/workspace/android-ndk-r10c
PLATFORM=$NDK/platforms/android-9/arch-arm
PREBUILT=$NDK/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86
SYSROOT=$NDK/platforms/android-9/arch-arm
PROJECT_ROOT=~/workspace/ffmpeg-2.2.10
GPATH=$PREBUILT/lib/gcc/arm-linux-androideabi/4.6

./configure \
	--disable-programs \
	--disable-doc \
	--disable-hwaccels \
	--disable-avdevice \
	--disable-postproc \
	--disable-avfilter \
	--disable-encoders \
	--disable-muxers \
	--disable-filters \
	--disable-indevs \
	--disable-devices \
	--enable-memalign-hack \
	--disable-yasm \
	--enable-cross-compile \
	--target-os=linux \
	--arch=arm \
	--cpu=armv7-a \
	--ld=$PREBUILT/bin/arm-linux-androideabi-ld \
	--cc=$PREBUILT/bin/arm-linux-androideabi-gcc \
	--cross-prefix=$PREBUILT/bin/arm-linux-androideabi- \
	--extra-cflags="-I$SYSROOT/usr/include -fPIC -DANDROID -std=gnu99 -mfpu=neon -mfloat-abi=softfp" \
	--extra-ldflags="-L$PLATFORM/usr/lib -nostdlib $GPATH/crtbegin.o $GPATH/crtend.o -lc -lm -ldl -lz"
