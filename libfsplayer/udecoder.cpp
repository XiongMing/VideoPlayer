#include "ucommon.h"
#include "udecoder.h"


extern "C" {
#include "libavformat/avformat.h"
}

UDecoder::UDecoder(char* name,UPlayer* player):mPlayer(player),UThread(name){

	//确保播放器引用不为空
	assert(player);
}

void UDecoder::handleRun(){

	//注册用于崩溃捕获的线程id
#if PLATFORM_DEF == ANDROID_PLATFORM
	mPlayer->mCrashHandler.registerTid();
#endif

	//解码线程
	decode();

	//取消注册用于崩溃捕获的线程id
#if PLATFORM_DEF == ANDROID_PLATFORM
	mPlayer->mCrashHandler.unRegisterTid();
#endif

}


