#include "urenderer_audio_linux.h"

URendererAudioLinux::URendererAudioLinux(UPlayer* player,int channels, int samplerate,int samplefmt):\
IRendererAudio(player,channels,samplerate,samplefmt),UThread("AudioRenderer:URendererAudioLinux"){

	//设置播放器对象指针
	mPlayer = player;

	if(!mPlayer)
		ulog_err("URendererAudioLinux::URendererAudioLinux mPlayer == NULL");

}


URendererAudioLinux::~URendererAudioLinux(){

}

void URendererAudioLinux::start(){
	init();
	this->UThread::start();
    return;
}

void URendererAudioLinux::init(){
	return;
}
void URendererAudioLinux::stop(){
	//释放音频渲染线程
	if(mPlayer && mPlayer->mPCMQueue)mPlayer->mPCMQueue->abort();
}
void URendererAudioLinux::render(){

	ulog_info("URendererAudioLinux::render enter");
	av_link	pcm_pkt;
	int length;

	ulog_info("URendererAudioLinux::render enter");

	while(!mPlayer->isStop()){

		if(mPlayer->isPause()){
			usleep(UPLAYER_PAUSE_TIME);
			continue;
		}
		//获取YUV数据包
		pcm_pkt = (av_link)mPlayer->mPCMQueue->get();

		if(!pcm_pkt){
			ulog_err("URendererAudioLinux::render mPCMQueue->get() == NULL");
			continue;
		}
		//处理数据包
		length = 0;
		if (pcm_pkt->size > 0) {
		//	ulog_info("audio_pts: %f", pcm_pkt->pts);
			double  audio_render_duration = (double)pcm_pkt->size / (mSampleRate * mChannels * mSampleFmt) * 1000 * 1000;
			//ulog_info("audio_frame_duration: %f", audio_render_duration);
                 	usleep(audio_render_duration);
		}

		//将数据包还回YUV空槽队列
		mPlayer->mPCMSlotQueue->put(pcm_pkt);
	}

	ulog_info("URendererAudioLinux::render exit");
}
void URendererAudioLinux::handleRun(){

	//音频渲染线程
	render();

}
