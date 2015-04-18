#!/bin/sh

#ARCHS="armv7 armv7s arm64"
ARCHS="armv7"

COMMONCONFIG="--target-os=darwin --enable-cross-compile --cc=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang  --as='/usr/bin/gas-preprocessornew.pl -as-type apple-clang -arch arm -add-prefix /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang'  --sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS8.2.sdk --enable-cross-compile --enable-nonfree --enable-gpl --enable-avutil --enable-swscale --enable-swresample --enable-avformat --enable-avcodec --disable-armv5te --disable-swscale-alpha --disable-doc --disable-ffmpeg --disable-ffplay --disable-ffprobe --disable-ffserver  --disable-debug --disable-shared --disable-bzlib --disable-zlib --disable-iconv  --enable-neon --enable-optimizations --enable-runtime-cpudetect --enable-memalign-hack  --enable-small --enable-static --disable-shared --disable-vfp "


set -e
for ARCH in ${ARCHS}
do 
	if [ "${ARCH}" == "armv7" ];
	then
		ARCHPARAM=" --extra-cflags=\"-arch armv7 -mios-version-min=5.0 -mthumb -mfloat-abi=softfp -march=armv7 -mfpu=neon -flax-vector-conversions\" --extra-ldflags=\"-arch armv7 -mios-version-min=5.0\" --prefix=ios/armv7 --arch=arm --cpu=cortex-a8 "
	elif [ "${ARCH}" == "armv7s" ];
	then
		ARCHPARAM=" --extra-cflags=\"-arch armv7s -mios-version-min=5.0 -mthumb -mfloat-abi=softfp -march=armv7s -mfpu=neon -flax-vector-conversions\" --extra-ldflags=\"-arch armv7s -mios-version-min=5.0\"  --prefix=ios/armv7s --arch=arm --cpu=cortex-a8 "
	else 
		ARCHPARAM=" --extra-cflags=\"-arch arm64 -mios-version-min=5.0 -mthumb -mfloat-abi=softfp -march=arm64 -mfpu=neon -flax-vector-conversions\" --extra-ldflags=\"-arch arm64 -mios-version-min=5.0\"  --prefix=ios/arm64 --arch=arm64 "
	fi

	PARAM="${COMMONCONFIG}${ARCHPARAM}"
	bash -c "./configure ${PARAM}"
	sudo make && sudo make install 
	#sudo make clean
done

set -e
#sudo rm -rf ios/universal
#sudo mkdir -p ios/universal/lib
#cd ios/armv7/lib
#for file in *.a
#do
 #       cd ../../
  #      sudo xcrun -sdk iphoneos lipo -output universal/lib/$file  -create -arch armv7 armv7/lib/$file -arch arm64 arm64/lib/$file -arch armv7s armv7s/lib/$file
   #     echo "Universal $file created."
    #    cd armv7/lib
#done
#cd ../../
#sudo cp -r armv7/include universal/include
echo "Done"





