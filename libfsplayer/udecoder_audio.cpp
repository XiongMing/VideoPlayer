#include "ucommon.h"
#include "uplayer.h"
#include "udecoder_audio.h"
#include "urenderer_audio.h"

extern "C" {

#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"

} // end of extern C

void UDecoderAudio::process(av_link pkt) {
	av_link pcm_pkt;
	int ret;
	int size = mPlayer->mSamplesSize;
	int size_out = 0;
	int completed = 0;
	int16_t* buftmp = NULL;
	int channels = mPlayer->mMediaFile->streams[mPlayer->mAudioStreamIndex]->codec->channels;
	int sample_rate = mPlayer->mMediaFile->streams[mPlayer->mAudioStreamIndex]->codec->sample_rate;
	enum AVSampleFormat sample_fmt = mPlayer->mMediaFile->streams[mPlayer->mAudioStreamIndex]->codec->sample_fmt;


	if(UPLAYER_DECODER_FLUSH_FLAG == pkt->flag){
		//清空视频解码器缓冲区
		avcodec_flush_buffers(mPlayer->mMediaFile->streams[mPlayer->mAudioStreamIndex]->codec);
		ulog_info("UDecoderAudio::process avcodec_flush_buffers");
		return;
	}

	//音频解码
	avcodec_decode_audio4(mPlayer->mMediaFile->streams[mPlayer->mAudioStreamIndex]->codec, mPlayer->mAudioDecFrame, &completed, (AVPacket*)pkt->item);
	size = mPlayer->mAudioDecFrame->channels * mPlayer->mAudioDecFrame->nb_samples * av_get_bytes_per_sample(sample_fmt);

	if(!completed){
		//ulog_err("UDecoderAudio::process avcodec_decode_audio4 failed");
		return;
	}

	//针对采样精度不是AV_SAMPLE_FMT_S16的情况，对其进行重新采样
	init_swr(mPlayer->mAudioDecFrame);
	if (sample_fmt != AV_SAMPLE_FMT_S16 || 2 != channels) {
		size_out = swr_convert(mPlayer->mResampleEngine,
								(uint8_t**) mPlayer->mAudioFrame->data,
								mPlayer->mAudioDecFrame->nb_samples,
								(const uint8_t **) mPlayer->mAudioDecFrame->data,
								mPlayer->mAudioDecFrame->nb_samples);

		size_out = size_out * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
		//ulog_info("swr_convert.size_out=%d,mPlayer->mAudioDecFrame->nb_samples=%d",size_out,mPlayer->mAudioDecFrame->nb_samples);
		size = size_out;

	}

	//获取空槽，用来存放解码后的PCM数据
	pcm_pkt = (av_link) mPlayer->mPCMSlotQueue->get();

	if (!pcm_pkt) {
		ulog_err("UDecoderAudio::process mPCMSlotQueue->get() == NULL");
		return;
	}
	pcm_pkt->item = mPlayer->mAudioFrame->data[0];
	pcm_pkt->size = size;
	//将PCM包放到播放队列
	mPlayer->mPCMQueue->put(pcm_pkt);
}
void UDecoderAudio::updateCurrentPosition(av_link pkt) {

	double pts;
	AVPacket* packet = (AVPacket*) pkt->item;
	static bool flag = true;

	if (flag = !flag)
		return;

	//计算音频时间戳
	if (AV_NOPTS_VALUE == packet->pts) {
		ulog_err("UDecoderAudio::update_pts AV_NOPTS_VALUE == packet->pts");
		pts = 0;
	} else {


		pts = packet->pts;

		// modified by bruce
		//if(mPlayer->mStreamType & UPLAYER_STREAM_AUDIO)
		pts *= av_q2d(mPlayer->mTimeBase[mPlayer->mPtsStreamIndex]);
//		pts *= av_q2d(mPlayer->mMediaFile->streams[mPlayer->mAudioStreamIndex]->time_base);
		pts *= 1000;

	}
	//ulog_info("CurrentPostion=%f",(double)pts);
	//更新进度条
	if(!mPlayer->isSeeking()){
		mPlayer->setCurrentPosition(pts);
#if PLATFORM_DEF == IOS_PLATFORM
        //保证不管是刚开始播放还是快进的时候，解码完第一帧音频后，视频才能显示，做音视频同步
        if(!mPlayer->mFirstAudioPacketDecoded){
            mPlayer->mFirstAudioPacketDecoded = true;
        }
        mPlayer->mAudioDecodedPts = pts;
#endif
	}
}
void UDecoderAudio::decode() {

	av_link pkt;

	ulog_info("UDecoderAudio::decode enter");
	while (!mPlayer->isStop()) {
    
#if PLATFORM_DEF != IOS_PLATFORM
        if (mPlayer->isPause()) {
            usleep(UPLAYER_PAUSE_TIME);
            continue;
        }
#endif
        
#if PLATFORM_DEF == IOS_PLATFORM
        pthread_rwlock_rdlock(&mPlayer->mRWLock);
        /*判断当前mAPacketQueue是否有可解码的数据和可用的pcm空包，防止死锁，
         原因我在这块加锁，如果不释放，其他线程可能被锁住
         add by huangweiqing*/
        if (mPlayer->mAPacketQueue->size() == 0 || mPlayer->mPCMSlotQueue->size() == 0 || mPlayer->isSeeking()) {
            pthread_rwlock_unlock(&mPlayer->mRWLock);
            usleep(UPLAYER_PAUSE_TIME);
            continue;
        }
#endif
        //获取空槽，存放音频包
        //        ulog_info("mAPacketQueue size: %d", mPlayer->mAPacketQueue->size());
		pkt = (av_link) mPlayer->mAPacketQueue->get();
		if (!pkt) {
			ulog_err("UDecoderAudio::decode AudioQueue->get() == NULL");
#if PLATFORM_DEF == IOS_PLATFORM
            pthread_rwlock_unlock(&mPlayer->mRWLock);
            usleep(UPLAYER_PAUSE_TIME);
#endif
			continue;
		}
		//处理音频数据
        /*在缓冲的时候，会发现音频包一直在耗费，但是不解码，原因估计就在这
        当前如果在缓冲,原来是不进行解码，直接把包换回空槽
         add by huangweiqing*/
#if PLATFORM_DEF != IOS_PLATFORM
		if(!mPlayer->isSeeking() && !mPlayer->mNeedBufferring){
#endif
			process(pkt);
			//更新进度条
            /*之所以要判断，是因为如果是刷新包得话，音频的pts为0.这样在做同步的时候，会造成画面卡顿 
             add by huangweiqing*/
        #if PLATFORM_DEF == IOS_PLATFORM
            if (UPLAYER_DECODER_FLUSH_FLAG != pkt->flag) {
        #endif
			updateCurrentPosition(pkt);
        
        #if PLATFORM_DEF == IOS_PLATFORM
            }
        #endif
                
                
#if PLATFORM_DEF != IOS_PLATFORM
		}
#endif
		//把用完的空槽还回队列
//        ulog_info("mASlotQueue size: %d", mPlayer->mASlotQueue->size());
		mPlayer->mASlotQueue->put(pkt);
        #if PLATFORM_DEF == IOS_PLATFORM
            pthread_rwlock_unlock(&mPlayer->mRWLock);
        #endif
	}
	ulog_info("UDecoderAudio::decode exit");
}
void UDecoderAudio::stop() {

	if (!isRunning())
		return;

	//退出音频解码线程
	if (mPlayer && mPlayer->mAPacketQueue)
		mPlayer->mAPacketQueue->abort();
	if (mPlayer && mPlayer->mPCMSlotQueue)
		mPlayer->mPCMSlotQueue->abort();

	//等待线程退出
	wait();
}
void UDecoderAudio::init_swr(AVFrame* frame){

	if(!mPlayer->mResampleEngine){

		mPlayer->mResampleEngine = swr_alloc_set_opts(NULL,AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,frame->sample_rate,frame->channel_layout,(enum AVSampleFormat)mPlayer->mSampleFmt,frame->sample_rate,0,0);

		ulog_info("swr_alloc_set_opts");
		if(!mPlayer->mResampleEngine){
			ulog_err("swr_alloc_set_opts failed! Cannot convert %s sample format to AV_SAMPLE_FMT_S16 sample format\n", av_get_sample_fmt_name((enum AVSampleFormat)mPlayer->mSampleFmt));
			set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
			return;
		}
		if(swr_init(mPlayer->mResampleEngine)){
			ulog_err("swr_init failed! Cannot convert %s sample format to AV_SAMPLE_FMT_S16 sample format\n", av_get_sample_fmt_name((enum AVSampleFormat)mPlayer->mSampleFmt));
			set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
			return;
		}
		ulog_info("swr_init");
	}

}
