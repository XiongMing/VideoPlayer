#include "urenderer_opensl_es.h"

URendererAudioOpenSLES::URendererAudioOpenSLES(UPlayer* player,int channels, int samplerate,int samplefmt):IRendererAudio(player,channels,samplerate,samplefmt),UThread("AudioRenderer:OpenSLES"){

	mPlayer = (UPlayer*)player;

	//确保播放器引用有值
	assert(mPlayer);

	mPlayerObject = NULL;
	mPlayerPlay = NULL;
	mOutputMixObject = NULL;
	mEngineObject = NULL;
	mEngine = NULL;
	mPrepared = false;
}
int URendererAudioOpenSLES::createEngine(){
    SLresult result;

    //创建OpenSL ES 音频引擎
    result = slCreateEngine(&mEngineObject, 0, NULL, 0, NULL, NULL);
    if(SL_RESULT_SUCCESS != result){
    	ulog_err("slCreateEngine failed");
    	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
    	return FAILED;
    }

    //调整同步模式
    result = (*mEngineObject)->Realize(mEngineObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
    	ulog_err("(*mEngineObject)->Realize failed");
    	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
    	return FAILED;
    }

    //获取引擎接口
    result = (*mEngineObject)->GetInterface(mEngineObject, SL_IID_ENGINE, &mEngine);
    if(SL_RESULT_SUCCESS != result){
       	ulog_err("Get SL_IID_ENGINE failed");
       	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
       	return FAILED;
    }

    //创建音频混合器
    const SLInterfaceID ids[1] = {SL_IID_VOLUME};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*mEngine)->CreateOutputMix(mEngine, &mOutputMixObject, 1, ids, req);
    if(SL_RESULT_SUCCESS != result){
    	ulog_err("CreateOutputMix failed");
    	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
    	return FAILED;
    }

    //调整混合器
    result = (*mOutputMixObject)->Realize(mOutputMixObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
    	ulog_err("(*mOutputMixObject)->Realize");
    	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
    	return FAILED;
    }

    return NO_ERROR;
}

int URendererAudioOpenSLES::createBufferQueueAudioPlayer(int channels, int samplerate){

	ulog_info("in URendererAudioOpenSLES::createBufferQueueAudioPlayer()");
    SLresult result;

    //配置音频播放参数
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 1};
    SLDataFormat_PCM pcm;
    //数据源格式
    pcm.formatType = SL_DATAFORMAT_PCM;
    //声道数
    pcm.numChannels = channels;//跟下面的channelMask 要配对，不然会报错
    //美妙采样数
    pcm.samplesPerSec = samplerate * 1000;
    //精度
    pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    if(pcm.numChannels == 2)
    {	//立体声 这是参照audiotrack CHANNEL_OUT_STEREO = (CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT)得到的
    	pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }
    else
    {
    	pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSrc = {&loc_bufq, &pcm};

    //音频sink参数
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, mOutputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    //创建音频播放器
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    ulog_info("after call CreateAudioPlayer()");

    result = (*mEngine)->CreateAudioPlayer(mEngine, &mPlayerObject, &audioSrc, &audioSnk,
            sizeof( ids ) / sizeof( *ids ), ids, req);

    ulog_info("call CreateAudioPlayer() over");

    if(SL_RESULT_SUCCESS != result){
    	ulog_err("CreateAudioPlayer failed");
    	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
    	return FAILED;
    }


    //调整播放器
    result = (*mPlayerObject)->Realize(mPlayerObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
    	ulog_err("(*mPlayerObject)->Realize failed");
    	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
    	return FAILED;
    }
    //获得播放接口
    result = (*mPlayerObject)->GetInterface(mPlayerObject, SL_IID_PLAY, &mPlayerPlay);
    if(SL_RESULT_SUCCESS != result){
    	ulog_err("(*mPlayerObject)->GetInterface SL_IID_PLAY failed");
    	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
    	return FAILED;
    }

    //获取缓冲队列接口
    result = (*mPlayerObject)->GetInterface(mPlayerObject, SL_IID_BUFFERQUEUE,
            &mPlayerBufferQueue);
    if(SL_RESULT_SUCCESS != result){
    	ulog_err("(*mPlayerObject)->GetInterface SL_IID_BUFFERQUEUE failed");
    	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
    	return FAILED;
    }
     //注册回调函数
	result = (*mPlayerBufferQueue)->RegisterCallback(mPlayerBufferQueue, AudioPlayerCallback, mPlayer);
	if(SL_RESULT_SUCCESS != result){
	 	ulog_err("mPlayerBufferQueue RegisterCallback failed");
	 	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
	 	return FAILED;
	}
	//设置播放状态
    result = (*mPlayerPlay)->SetPlayState(mPlayerPlay, SL_PLAYSTATE_PLAYING);
	if(SL_RESULT_SUCCESS != result){
	 	ulog_err("SetPlayState failed");
	 	set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
	 	return FAILED;
	}

	return NO_ERROR;
}

URendererAudioOpenSLES::~URendererAudioOpenSLES(){

	//释放播放对象
	if (mPlayerObject != NULL) {
		(*mPlayerObject)->Destroy(mPlayerObject);
		mPlayerObject = NULL;
		mPlayerPlay = NULL;
		mPlayerBufferQueue = NULL;
	}

	//释放混合器
	if (mOutputMixObject != NULL) {
		(*mOutputMixObject)->Destroy(mOutputMixObject);
		mOutputMixObject = NULL;
	}

	//释放音频引擎
	if (mEngineObject != NULL) {
		(*mEngineObject)->Destroy(mEngineObject);
		mEngineObject = NULL;
		mEngine = NULL;
	}
}
void URendererAudioOpenSLES::AudioPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context){

	SLresult result;

	av_link	pcm_pkt;

	//获取播放器
	UPlayer* player = (UPlayer*)context;

	URendererAudioOpenSLES*	audio_renderer = (URendererAudioOpenSLES*)player->mRendererAudio;

	UThreadID tid = gettid();

	if( audio_renderer->mTids[0] != tid && audio_renderer->mTids[1] != tid){
		audio_renderer->mTids[1] = tid;
		player->mCrashHandler.registerTid(audio_renderer->mTids[1]);
	}

	//如果播放器已经停止则退出
	if(player->isStop())return;

	//将上一次的PCM缓冲区还回PCM空槽队列
	player->mPCMSlotQueue->put(player->mLastPCM);

	//获取PCM空槽
	pcm_pkt = (av_link)player->mPCMQueue->get();

	//错误处理
	if(!pcm_pkt){
		ulog_err("AudioPlayerCallback !pcm_pkt");
		return;
	}
	//保存这次的PCM缓冲区，留待下一次还回空槽队列，或播放器退出时换回空槽队列
	player->mLastPCM = pcm_pkt;

	//播放声音
	result = (*bq)->Enqueue(bq, pcm_pkt->item, pcm_pkt->size);

	if(SL_RESULT_SUCCESS != result){
		player->notifyMsg(MEDIA_INFO_PLAYERROR,ERROR_SOFT_PLAYER_OPENSLES);
	 	ulog_err("mPlayerBufferQueue Enqueue failed");
	 	return;
	}
}
void URendererAudioOpenSLES::start(){

	//创建播放引擎和播放对象
	if(!mPlayer || createEngine() || createBufferQueueAudioPlayer(this->mChannels,this->mSampleRate)){
		//设置出错标记
		mPrepared = false;
		this->mPlayer->notifyMsg(MEDIA_INFO_PREPARE_ERROR,ERROR_SOFT_PLAYER_OPENSLES);
		ulog_err("URendererAudioOpenSLES::start failed");
	}else{
		//设置音频渲染器状态
		ulog_info("URendererAudioOpenSLES::start\n");
		mPrepared = true;
		this->UThread::start();
		//AudioPlayerCallback(this->mPlayerBufferQueue,this->mPlayer);
	}

}
void URendererAudioOpenSLES::stop(){

	SLresult result;

	if(!mPrepared)return;

	//释放音频渲染线程
	if(mPlayer && mPlayer->mPCMQueue)mPlayer->mPCMQueue->abort();

	//错误处理
	if(!mPlayerBufferQueue){
	 	ulog_err("URendererAudioOpenSLES release failed");
	 	return;
	}
	//设置停止状态
    result = (*mPlayerPlay)->SetPlayState(mPlayerPlay, SL_PLAYSTATE_STOPPED);
	if(SL_RESULT_SUCCESS != result){
	 	ulog_err("SetPlayState stop failed");
	}

	mPlayer->mCrashHandler.unRegisterTid(mTids[1]);
}
void URendererAudioOpenSLES::handleRun(){

	//记录线程ID
	mTids[0] = gettid();

	//注册用于崩溃捕获的线程id
	mPlayer->mCrashHandler.registerTid();

	//音频渲染线程
	AudioPlayerCallback(this->mPlayerBufferQueue,this->mPlayer);

	//取消注册用于崩溃捕获的线程id
	mPlayer->mCrashHandler.unRegisterTid();
}
