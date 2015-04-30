#include "ucommon.h"
#include "uplayer.h"
#include "udecoder_video.h"
#include "urenderer_video.h"

extern "C" {
#include "libavutil/time.h"
} // end of extern C

void UDecoderVideo::process(av_link pkt){

	av_link	yuv_pkt;
	int completed = 0;
	int64_t start_time = av_gettime(),end_time,start_time2;
	AVPacket	*packet = (AVPacket*)pkt->item;

	if(UPLAYER_DECODER_FLUSH_FLAG ==pkt->flag){
		//清空视频解码器缓冲区
		avcodec_flush_buffers(mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->codec);
		ulog_info("UDecoderVideo::process avcodec_flush_buffers");
		return;

	}else if(UPLAYER_VPACKET_FLUSH_FLAG & pkt->flag){
		//如果是最后一个丢弃包，则同时清空视频解码器缓冲区, by weixinghua
        if (UPLAYER_VPACKET_FLUSH_LAST & pkt->flag){
			avcodec_flush_buffers(mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->codec);
			ulog_info("UDecoderVideo::process UPLAYER_VPACKET_FLUSH_LAST == pkt->flag");
        }
		return;
	}

	int err = avcodec_decode_video2(mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->codec,
				 mPlayer->mDecFrame,
				 &completed,
				 (AVPacket*)pkt->item);


	if(completed){

		end_time = av_gettime();
		mPlayer->mDecodeTime = end_time - start_time;

#if DEBUG_ENABLE_H265_DECODER_TEST
		mPlayer->mTotalDecodeTime += mPlayer->mDecodeTime;
		mPlayer->mDecodeCount++;
#endif
	mPlayer->mRealVideoWidth = mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->codec->width;
	mPlayer->mRealVideoHeight = mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->codec->height;
	if( true == mNeedInitSws && ( (mPlayer->mRealVideoWidth != mPlayer->mVideoWidth || mPlayer->mRealVideoHeight != mPlayer->mVideoHeight) )){
			//获得颜色空间转换和缩放后视频帧缓冲区的大小
			mPlayer->mPixelsPerImage = avpicture_get_size(mPlayer->stream_tmp->codec->pix_fmt,/*mPlayer->mVideoWidth,
					mPlayer->mVideoHeight*/mPlayer->mRealVideoWidth,mPlayer->mRealVideoHeight);

			//分配缓冲区

			if(mPlayer->mPixels){
				av_free(mPlayer->mPixels);
				mPlayer->mPixels = NULL;
			}
			mPlayer->mPixels = (uint8_t *) av_malloc(mPlayer->mPixelsPerImage * sizeof(uint8_t));

			if (!mPlayer->mPixels) {
					ulog_err("UPlayer mPixels == NULL");
					mPlayer->notifyMsg(MEDIA_INFO_PLAYERROR);
					return;
			}

			//将YUV缓冲区填充到外壳，以便进行颜色空间转换
			if (avpicture_fill((AVPicture *) mPlayer->mFrame, mPlayer->mPixels,
			//PIX_FMT_YUV444P,
					mPlayer->stream_tmp->codec->pix_fmt,
					//PIX_FMT_RGB565,
					/*mPlayer->mVideoWidth, mPlayer->mVideoHeight*/mPlayer->mRealVideoWidth,mPlayer->mRealVideoHeight) <= 0) {
				ulog_err("UPlayer avpicture_fill failed");
				mPlayer->notifyMsg(MEDIA_INFO_PLAYERROR);
				return;
			}
			mNeedInitSws = false;
	}
	start_time2 = av_gettime();
	mPlayer->mConvertCtx = sws_getCachedContext(mPlayer->mConvertCtx,
												mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->codec->width,
												mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->codec->height,
												mPlayer->stream_tmp->codec->pix_fmt,
												mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->codec->width,
												mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->codec->height,
												PIX_FMT_YUV420P,
												SWS_POINT, NULL, NULL, NULL);

		//颜色空间转换
		sws_scale(mPlayer->mConvertCtx,
				mPlayer->mDecFrame->data,
				mPlayer->mDecFrame->linesize,
				0,/*mPlayer->mVideoHeight,*/
				mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->codec->height,
				mPlayer->mFrame->data,
				mPlayer->mFrame->linesize);

#if !DEBUG_ENABLE_H265_DECODER_TEST

		//ulog_err("UDecoderAudio::process mPlayer->mDecodeTime = %lld,sws_time=%lld",mPlayer->mDecodeTime,(end_time - start_time2));
		//获取空槽，存放解码后的YUV数据
		yuv_pkt = (av_link) mPlayer->mYUVSlotQueue->get();

		if (!yuv_pkt) {
			ulog_err("UDecoderAudio::process mYUVSlotQueue->get() == NULL");
			return;
		}
		yuv_pkt->item = mPlayer->mFrame->data[0];
		yuv_pkt->size = mPlayer->mPixelsPerImage;
		yuv_pkt->pts = getPacketPts(mPlayer->mDecFrame);

		//将YUV包放入播放队列
		mPlayer->mYUVQueue->put(yuv_pkt);
#endif
		//保存上一帧的时间
		mPlayer->mLastVideoPts = av_gettime();

		//YUV队列已满，释放缓冲
		if (mPlayer->mNeedYUVBufferFull && mPlayer->mYUVQueue->size() == mPlayer->mYUVSlotQueue->getInitItems()) {
			mPlayer->mNeedYUVBufferFull = false;
			ulog_info("video yuv buffer full");
		}
	}
}
void UDecoderVideo::decode(){

	av_link pkt;
	int ret;

	mNeedInitSws = true;
	ulog_info("UDecoderVideo::decode enter");

	while(!mPlayer->isStop()){
    
#if PLATFORM_DEF != IOS_PLATFORM
		if(mPlayer->isPause()){
			usleep(UPLAYER_PAUSE_TIME);
			continue;
		}
#endif
		//获取空槽，存放视频包
//        ulog_info("mVPacketQueue size: %d", mPlayer->mVPacketQueue->size());
    
#if PLATFORM_DEF == IOS_PLATFORM
        pthread_rwlock_rdlock(&mPlayer->mRWLock);
        if (mPlayer->mVPacketQueue->size() == 0 || mPlayer->mYUVSlotQueue->size() == 0
            || mPlayer->isSeeking()) {
            pthread_rwlock_unlock(&mPlayer->mRWLock);
            usleep(UPLAYER_PAUSE_TIME);
            continue;
        }
#endif
		pkt = (av_link)mPlayer->mVPacketQueue->get();
		if(!pkt){
			ulog_err("UDecoderVideo::decode VideoQueue->get() == NULL");
#if PLATFORM_DEF == IOS_PLATFORM
            pthread_rwlock_unlock(&mPlayer->mRWLock);
            usleep(UPLAYER_PAUSE_TIME);
#endif
			continue;
		}

		//视频处理
		process(pkt);
		//把用完的空槽还回队列
//        ulog_info("mVSlotQueue size: %d", mPlayer->mVSlotQueue->size());
		mPlayer->mVSlotQueue->put(pkt);
#if PLATFORM_DEF == IOS_PLATFORM
        pthread_rwlock_unlock(&mPlayer->mRWLock);
#endif

	}
#if DEBUG_ENABLE_H265_DECODER_TEST

	ulog_info("H265 video test:[width:%d][height:%d][duration:%f][fps:%f][average decoding time: %lld ms]" ,mPlayer->mVideoWidth, mPlayer->mVideoHeight, (double)mPlayer->mMediaFile->duration, (double)mPlayer->mFrameRate,mPlayer->mTotalDecodeTime / mPlayer->mDecodeCount);

#endif
	ulog_info("UDecoderVideo::decode exit");
}
void UDecoderVideo::stop(){

	if(!isRunning())return;

	//退出视频线程
	if(mPlayer && mPlayer->mVPacketQueue)mPlayer->mVPacketQueue->abort();
	if(mPlayer && mPlayer->mYUVSlotQueue)mPlayer->mYUVSlotQueue->abort();

	//等待线程退出
	wait();
}

double UDecoderVideo::getPacketPts(AVFrame* frame){

	double	pts;

	//计算视频时间戳
	if(AV_NOPTS_VALUE == frame->best_effort_timestamp){
		ulog_err("UDecoderVideo::getPacketPts AV_NOPTS_VALUE == packet->pts");
		pts = 0;
	}else{

		pts = frame->pkt_pts;
		// modified by bruce
		pts *= av_q2d(mPlayer->mTimeBase[mPlayer->mVideoStreamIndex]);
//		pts *= av_q2d(mPlayer->mMediaFile->streams[mPlayer->mVideoStreamIndex]->time_base);
		pts *= 1000;
	}

	return pts;
}
