#!/bin/bash

######################################################
# ARM V5
# ARM v6 
# ARM v6+VFP
# ARM v7+VFP 
# ARM v7+VFPv3-d16 (Tegra2) 
# ARM v7+Neon (Cortex-A8)
######################################################

###############################################################
#for format list:
#ARM V5: mp4\avi\wmv\flv\mpg\rmvb\gif
#other: mp4\avi\wmv\flv\mpg\rmvb\mov\3gp\3gp2\mkv\avs\gif
###############################################################

############## common functions begin ################



if [ ! -d "$ANDROID_NDK" ]; then
    echo "ANDROID_NDK does not exist, please export correct ANDROID_NDK in your machine!"
    exit
fi

INSTALL_PREFIX=$(pwd)/../thirdpart_lib
#echo $INSTALL_PREFIX

PLATFORM=$ANDROID_NDK/platforms/android-9/arch-arm

#echo $PLATFORM

PREBUILT=$ANDROID_NDK/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86
GPATH=$PREBUILT/lib/gcc/arm-linux-androideabi/4.4.3

function build-target
{
		./configure --target-os=linux	\
		--prefix=$PREFIX \
		--enable-cross-compile 	\
		--extra-libs="-lgcc" \
		--cc=$PREBUILT/bin/arm-linux-androideabi-gcc \
		--cross-prefix=$PREBUILT/bin/arm-linux-androideabi- \
		--nm=$PREBUILT/bin/arm-linux-androideabi-nm \
		--sysroot=$PLATFORM \
		--extra-cflags="-I$PLATFORM/usr/include -O2 -fpic -DANDROID " \
		--enable-gpl \
		--enable-version3 \
		--disable-shared \
		--enable-static \
		--extra-ldflags="-L$PLATFORM/usr/lib -nostdlib $GPATH/crtbegin.o $GPATH/crtend.o -lc -lm -ldl -llog " \
		--enable-parsers \
		--enable-network \
		--enable-protocols \
		--disable-protocol=gopher \
		--disable-protocol=mmst \
		--disable-protocol=mmsh \
		--disable-protocol=md5	\
		--disable-protocol=crypto	\
		--disable-protocol=concat	\
		--disable-protocol=rtmp	\
		--disable-protocol=rtp	\
		--disable-ffmpeg \
		--disable-ffplay \
		--disable-ffprobe \
		--disable-ffserver \
		--disable-devices \
		--disable-avdevice \
		--disable-postproc \
		--disable-avfilter \
		--disable-filters \
		--disable-swscale-alpha \
		--disable-bsfs \
		--disable-encoders \
		--disable-muxers \
		--disable-indevs \
		--disable-debug \
		--disable-doc \
		--disable-bzlib \
		--disable-zlib \
        --enable-avutil \
		--enable-swscale  \
		--enable-swresample \
		--enable-avformat \
		--enable-avcodec \
		--enable-optimizations \
        --enable-runtime-cpudetect \
		--disable-demuxers \
		--disable-decoders \
		$SUPPORTED_FORMAT \
		$ADDITIONAL_CONFIGURE_FLAG \
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
    --disable-yasm 
	
	if [ $? != 0 ]
	then
		echo "!!!!!!configure faild"
		exit
	fi
	make clean
	
	make -j4
	make install
}


############## build arm v7 neon begin ######
function config_arm_v7_neon
{
		#arm v7n
    CPU=armv7-a
    OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=neon -march=$CPU -mtune=cortex-a8 -Wno-multichar -fno-exceptions "
    PREFIX=$INSTALL_PREFIX/android/$CPU 	
    ADDITIONAL_CONFIGURE_FLAG="--arch=arm --cpu=armv7-a --disable-armv5te --disable-armv6 --disable-vfp --enable-memalign-hack --enable-neon --enable-small"
    
    build-target
}


function config_common
{
	#common
    CPU=arm
    OPTIMIZE_CFLAGS=" "
    PREFIX=$INSTALL_PREFIX/android/$CPU 	
    ADDITIONAL_CONFIGURE_FLAG="--arch=arm --enable-memalign-hack --enable-small"
    
    build-target
}


#echo $PREBUILT

SUPPORTED_COMMON_TWELVE_FORMAT="--enable-demuxer=flv,asf,mov,rm,avi,avs,mpegps,mpegts,mpegvideo,mpegtsraw,gif,matroska,mp3  \
	--enable-decoder=aac,ac3,h264,h264_vda,h264_crystalhd,h264_vdpau,h263p,h263i,h263,h261,vc1,vc1_crystalhd,vc1_vdpau,mvc1,mvc2 \
	--enable-decoder=wmv1,wmv2,wmv3,wmav1,wmav2,mp1,mp1float,mp2,mp2float,mp3,mp3adu,mp3float,mp3on4,mp3on4float,vorbis \
	--enable-decoder=mpeg4,mpeg1video,mpeg2video,mpeg1_vdpau,mpeg2_crystalhd,mpeg4_crystalhdb,mpeg4_vdpau,mpeg_vdpau,mpeg_xvmc \
	--enable-decoder=mpegvideo,msmpeg4_crystalhd,msmpeg4v1,msmpeg4v2,msmpeg4v3,msvideo1,amrnb,amrwb,evrc,smvjpeg,flic,flurxm,gif\
	--enable-decoder=rv10,rv20,rv30,rv40,atrac3,flv,ffv1,ffvhuff,ffwavesynth,flac,flashsv,flashsv2,vp3,vp5,vp6,vp6a,vp6f,vp8,cook \
	--enable-decoder=prores,prores_lgpl,qtrle,msrle,avui,svq1,alac,movtext,svq1,svq3,qdm2,pcm_s16be,pcm_s24be"

SUPPORTED_COMMON_SEVEN_FORMAT="--enable-demuxer=rm,flv,avi,asf,gif \
	--enable-decoder=rv10,rv20,rv30,rv40,cook,atrac3,aac,wmv1,wmv2,wmv3,wmav1,wmav2,mpeg1_vdpau,mpeg1video,mpeg2_crystalhd \
	--enable-decoder=mpeg2video,mpeg4,mpeg4_crystalhd,mpeg4_vdpau,mpeg_vdpau,mpeg_xvmc,mpegvideoflv,ffv1,ffvhuff,ffwavesynth,flashsv,flashsv2,flic,flurxm \
	--enable-decoder=flac,h264,vc1,h261,h263,h263i,h264_crystallhd,h264_vdpau,mp2,mp3,vorbis,vp3,vp5,vp6,vp6a,vp6f,vp8,vqa,gif"

SUPPORTED_FORMAT=$SUPPORTED_COMMON_TWELVE_FORMAT

function main
{
	chmod +x ./version.sh
	chmod +x ./configure
	select build_choice in "common" "arm_v7_neon" "all" "exit"
	do
	case $build_choice in
	"common")
		echo "building ffmpeg for common"
		config_common
		;;
	"arm_v7_neon")
		echo "building ffmpeg for arm_v7_neon"
		config_arm_v7_neon
		;;
	"all")
		echo "building ffmpeg for all platform"
		config_common
		config_arm_v7_neon
		;;
	"exit")
		echo "exit building" 
		break;
		;;
	*)
		echo "invalid platform choice"  
		;;
	esac
	done;
}

main

