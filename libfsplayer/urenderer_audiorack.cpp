#include "urenderer_audiotrack.h"

URendererAudioTrack::URendererAudioTrack(UPlayer* player,int channels, int samplerate,int samplefmt):IRendererAudio(player,channels,samplerate,samplefmt),UThread("AudioRenderer:AudioTrack"){

    mAudioTrack = NULL;
    mLibMedia = NULL;

	//设置播放器对象指针
	mPlayer = player;

	if(!mPlayer){
		ulog_err("URendererAudioTrack::URendererAudioTrack mPlayer == NULL");
		goto URendererAudioTrack_err;
	}

	//加载libmedia库
	mLibMedia = dlopen("libmedia.so", RTLD_NOW);
	if(!mLibMedia){
		ulog_err("URendererAudioTrack::URendererAudioTrack load libmedia.so failed");
		set_player_error_code(mPlayer,ERROR_LOAD_LIBMEDIA_FAILED);
		goto URendererAudioTrack_err;
	}
	//获得播放接口
	mFpGetOutputFrameCount = (AudioSystem_getOutputFrameCount)(dlsym(mLibMedia, "_ZN7android11AudioSystem19getOutputFrameCountEPii"));
	mFpGetOutputLatency = (AudioSystem_getOutputLatency)(dlsym(mLibMedia, "_ZN7android11AudioSystem16getOutputLatencyEPji"));
	mFpGetOutputSamplingRate = (AudioSystem_getOutputSamplingRate)(dlsym(mLibMedia, "_ZN7android11AudioSystem21getOutputSamplingRateEPii"));
	mFpGetMinFrameCount = (AudioTrack_getMinFrameCount)(dlsym(mLibMedia, "_ZN7android10AudioTrack16getMinFrameCountEPiij"));
	mFpCtor = (AudioTrack_ctor)(dlsym(mLibMedia, "_ZN7android10AudioTrackC1EijiiijPFviPvS1_ES1_ii"));
	mFpCtorLegacy = (AudioTrack_ctor_legacy)(dlsym(mLibMedia, "_ZN7android10AudioTrackC1EijiiijPFviPvS1_ES1_i"));
	mFpDtor = (AudioTrack_dtor)(dlsym(mLibMedia, "_ZN7android10AudioTrackD1Ev"));
	mFpInitCheck = (AudioTrack_initCheck)(dlsym(mLibMedia, "_ZNK7android10AudioTrack9initCheckEv"));
	mFpStart = (AudioTrack_start)(dlsym(mLibMedia, "_ZN7android10AudioTrack5startEv"));
	mFpStop = (AudioTrack_stop)(dlsym(mLibMedia, "_ZN7android10AudioTrack4stopEv"));
	mFpWrite = (AudioTrack_write)(dlsym(mLibMedia, "_ZN7android10AudioTrack5writeEPKvj"));
	mFpFlush = (AudioTrack_flush)(dlsym(mLibMedia, "_ZN7android10AudioTrack5flushEv"));

	//错误检测
    if (!((mFpGetOutputFrameCount && mFpGetOutputLatency && mFpGetOutputSamplingRate) || mFpGetMinFrameCount)) {
    	ulog_err("URendererAudioTrack::URendererAudioTrack get AudioTrack API failed 1");
    	set_player_error_code(mPlayer,ERROR_GET_INTERFACE_OF_PLAYER_FAILED);
    	goto URendererAudioTrack_err;
    }

    if (!((mFpCtor || mFpCtorLegacy) && mFpDtor && mFpInitCheck && mFpStart && mFpStop && mFpWrite && mFpFlush)) {
    	ulog_err("URendererAudioTrack::URendererAudioTrack get AudioTrack API failed 2");
    	set_player_error_code(mPlayer,ERROR_GET_INTERFACE_OF_PLAYER_FAILED);
    	goto URendererAudioTrack_err;
    }
    mPrepared = true;
	return;

URendererAudioTrack_err:

	if(!mLibMedia)dlclose(mLibMedia);
	if(mAudioTrack)free(mAudioTrack);
	//设置出错标记
	mPrepared = false;
	return;
}


URendererAudioTrack::~URendererAudioTrack(){

	if(mPrepared && mFpDtor && mAudioTrack){
		mFpDtor(mAudioTrack);
		free(mAudioTrack);
	}

}
void URendererAudioTrack::start(){
	init();
	this->UThread::start();
}
void URendererAudioTrack::init(){
    int status;
    int afSampleRate, afFrameCount, afLatency, minBufCount, minFrameCount;
    int type, channel, rate, format, size;

    //MUSIC
    type = 3;

    rate = this->mSampleRate;
    if(rate < 4000)rate = 4000;
    if(rate > 48000)rate = 48000;



    if(this->mSampleFmt != AV_SAMPLE_FMT_U8 && this->mSampleFmt != AV_SAMPLE_FMT_S16)this->mSampleFmt = AV_SAMPLE_FMT_S16;
    // AudioSystem::PCM_16_BIT = 1
    // AudioSystem::PCM_8_BIT = 2
    format = (this->mSampleFmt == AV_SAMPLE_FMT_S16) ? 1 : 2;

    if(this->mChannels > 2){
    	this->mChannels = 2;
    }
    // AudioSystem::CHANNEL_OUT_STEREO = 12
    // AudioSystem::CHANNEL_OUT_MONO = 4
    channel = (this->mChannels == 2) ? 12 : 4;

    // use the minium value
    if (!mFpGetMinFrameCount) {
        status = mFpGetOutputSamplingRate(&afSampleRate, type);
        status ^= mFpGetOutputFrameCount(&afFrameCount, type);
        status ^= mFpGetOutputLatency((uint32_t*)(&afLatency), type);
        if (status != 0) {
        	ulog_err("URendererAudioTrack::URendererAudioTrack API failed 1");
        	goto URendererAudioTrack_init_err;
        }
        minBufCount = afLatency / ((1000 * afFrameCount) / afSampleRate);
        if (minBufCount < 2)
            minBufCount = 2;
        minFrameCount = (afFrameCount * rate * minBufCount) / afSampleRate;
        size = minFrameCount;
    }
    else {
        status = mFpGetMinFrameCount(&size, type, rate);
        if (status != 0) {
        	ulog_err("URendererAudioTrack::URendererAudioTrack API failed 2");
        	goto URendererAudioTrack_init_err;
        }
    }
    ulog_info("URendererAudioTrack::URendererAudioTrack step 1");
    size <<= 1;
    // sizeof(AudioTrack) == 0x58 (not sure) on 2.2.1, this should be enough
    mAudioTrack = malloc(512);
    if (!mAudioTrack) {
    	ulog_err("URendererAudioTrack::URendererAudioTrack malloc failed");
    	goto URendererAudioTrack_init_err;
    }
    ulog_info("URendererAudioTrack::URendererAudioTrack step 2");
    // higher than android 2.2
    if (mFpCtor){
    	ulog_info("URendererAudioTrack::URendererAudioTrack type=%d,rate=%d,format=%d,channel=%d,size=%d",type,rate,format,channel,size);
    	mFpCtor(mAudioTrack, type, rate, format, channel, size, 0, NULL, NULL, 0, 0);
        ulog_info("URendererAudioTrack::URendererAudioTrack step 3");
    }
    // higher than android 1.6
    else if (mFpCtorLegacy)mFpCtorLegacy(mAudioTrack,type,rate,format,channel, size, 0, NULL, NULL, 0);


    status = mFpInitCheck(mAudioTrack);

    ulog_info("URendererAudioTrack::URendererAudioTrack step 4");
    if (status != 0) {
    	ulog_err("URendererAudioTrack::URendererAudioTrack mFpInitCheck failed 1");
        channel = (channel == 12) ? 2 : 1;
        mFpCtorLegacy(mAudioTrack, type, rate, format, channel, size, 0, NULL, NULL, 0);
        status = mFpInitCheck(mAudioTrack);
    }
    if (status != 0) {
    	ulog_err("URendererAudioTrack::URendererAudioTrack mFpInitCheck failed 2");
    	goto URendererAudioTrack_init_err;
    }
    ulog_info("URendererAudioTrack::URendererAudioTrack ok");
    mFpStart(mAudioTrack);
  	//设置音频渲染器状态
	mPrepared = true;
	return;

URendererAudioTrack_init_err:

		this->mPlayer->notifyMsg(MEDIA_INFO_PLAYERROR);
		if(!mLibMedia)dlclose(mLibMedia);
		if(mAudioTrack)free(mAudioTrack);
		//设置出错标记
		mPrepared = false;
		return;
}
void URendererAudioTrack::stop(){
	//释放音频渲染线程
	if(mPlayer && mPlayer->mPCMQueue)mPlayer->mPCMQueue->abort();

	if(mFpStop)mFpStop(mAudioTrack);
	if(mFpFlush)mFpFlush(mAudioTrack);
}
void URendererAudioTrack::render(){


	av_link	pcm_pkt;
	int length;

	ulog_info("URendererAudioTrack::render enter");

	while(!mPlayer->isStop()){

		if(mPlayer->isPause()){
			usleep(UPLAYER_PAUSE_TIME);
			continue;
		}
		//获取YUV数据包
		pcm_pkt = (av_link)mPlayer->mPCMQueue->get();

		if(!pcm_pkt){
			ulog_err("URendererAudioTrack::render mPCMQueue->get() == NULL");
			continue;
		}
		//处理数据包
		length = 0;
		while (mFpWrite && length < pcm_pkt->size) {
			length += mFpWrite(mAudioTrack,(char*)(pcm_pkt->item) + length, pcm_pkt->size -length);
		}

		//将数据包还回YUV空槽队列
		mPlayer->mPCMSlotQueue->put(pcm_pkt);
	}

	ulog_info("URendererAudioTrack::render exit");
}
void URendererAudioTrack::handleRun(){

	//音频渲染线程
	render();

}
