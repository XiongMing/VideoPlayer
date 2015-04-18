#include "urenderer_video.h"
#include "uqueue.h"

#if PLATFORM_DEF != LINUX_PLATFORM
	#include "ugraphics.h"
#endif

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/time.h"
}

URendererVideo::URendererVideo(UPlayer* player) :mPlayer(player),
		UThread("video renderer") {

#if PLATFORM_DEF != LINUX_PLATFORM
	mGraphics = NULL;
#endif

	//确保播放器引用不为空
	assert(player);

	mLastFrame = (av_link)av_malloc(sizeof(*mLastFrame));
	bzero(mLastFrame,sizeof(*mLastFrame));
            
#if PLATFORM_DEF == IOS_PLATFORM
    mFirstFrame = (av_link)av_malloc(sizeof(*mFirstFrame));
    bzero(mFirstFrame,sizeof(*mFirstFrame));
            
    mLockRenderVideo = new ULock(ULOCK_TYPE_NORMAL);
    if (!mLockRenderVideo) {
        ulog_err("URendererVideo::mLockRenderVideo error");
        return;
    }
    mLockRenderVideo->lock();
    mRenderVideo = true;
    mLockRenderVideo->unlock();
#endif
}

void URendererVideo::handleRun() {

#if PLATFORM_DEF == ANDROID_PLATFORM
	//注册用于崩溃捕获的线程id
	mPlayer->mCrashHandler.registerTid();
#endif

	//视频渲染线程
	render();

	//取消注册用于崩溃捕获的线程id
#if PLATFORM_DEF == ANDROID_PLATFORM
	mPlayer->mCrashHandler.unRegisterTid();
#endif

}
void URendererVideo::process(av_link pkt) {
	//绘图操作

	if(UPLAYER_STREAM_VIDEO == mPlayer->mStreamType && !mPlayer->mIsSeeking)	mPlayer->setCurrentPosition(pkt->pts);

#if PLATFORM_DEF != LINUX_PLATFORM
    #if PLATFORM_DEF == IOS_PLATFORM
        mLockRenderVideo->lock();
        if (mRenderVideo) {
    #endif
        
		if (mGraphics)mGraphics->UpdateWindow(pkt->item);
        
    #if PLATFORM_DEF == IOS_PLATFORM
        }
        mLockRenderVideo->unlock();
    #endif
    
#else
		ulog_info("video_pts: %f", pkt->pts);
#endif

}
void URendererVideo::render() {
	av_link pkt;

	int64_t last_time = 0;

	ulog_info("URendererVideo::render enter");

	//单位微秒
	mFrameInterval = 1000000/mPlayer->mFrameRate;

	//创建绘图对象
	refreshEGL();

	ulog_info("openGL width:%d height:%d\n",mPlayer->mVideoWidth,mPlayer->mVideoHeight);

	while (!mPlayer->isStop()) {

#if PLATFORM_DEF == IOS_PLATFORM
        /*当前没有播放器结束并且没有发送缓冲开始消息并且
         如果视频或者音频的缓冲队列少于UPLAYER_VIDEO_PACKET_BUFFERRING_MIN_NUM就
         发送缓冲开始消息 Add by HuangWeiqing
         */
//        if (!mPlayer->playOver2(mPlayer->mLastPacketPts) && !mPlayer->mNeedBufferring
//            && (mPlayer->mVPacketQueue->size() <= UPLAYER_VIDEO_PACKET_BUFFERRING_MIN_NUM
//                ||((mPlayer->mStreamType & UPLAYER_STREAM_AUDIO) && mPlayer->mAPacketQueue->size() <= UPLAYER_VIDEO_PACKET_BUFFERRING_MIN_NUM))) {
//                showLastFrame();
//                mPlayer->mNeedBufferring = true;
//                mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_START);
//                usleep(UPLAYER_PAUSE_TIME);
//                ulog_info("MEDIA_INFO_BUFFERING_START in renderer_video.cpp");
//                continue;
//            }
        
        /*
         当前没有播放器结束并且没有发送缓冲开始消息并且
         如果视频或者音频的缓冲队列少于UPLAYER_VIDEO_PACKET_BUFFERRING_MIN_NUM
         又或者如果是首次启动或seek操作后，如果mYUVSlotQueue还有空槽（要保证空槽用完）
         发送缓冲开始消息 Add by HuangWeiqing
         */
        if (!mPlayer->playOver2(mPlayer->mLastPacketPts) && !mPlayer->mNeedBufferring){
            if(mPlayer->mVPacketQueue->size() <= UPLAYER_VIDEO_PACKET_BUFFERRING_MIN_NUM
                || ((mPlayer->mStreamType & UPLAYER_STREAM_AUDIO) && mPlayer->mAPacketQueue->size() <= UPLAYER_VIDEO_PACKET_BUFFERRING_MIN_NUM)
                || ((0 == mPlayer->mPreparedDone || 1 == mPlayer->mPreparedDone) && mPlayer->mYUVSlotQueue->size() != 0)){
                
                //((0 == mPlayer->mPreparedDone || 1 == mPlayer->mPreparedDone) && mPlayer->mYUVSlotQueue->size() != 0)
                //该判断保证视频yuv空槽用完，因为首次启动或者seek操作对解码要求比较高，可能出现短暂的音视频不同步现象
                showLastFrame();
                mPlayer->mNeedBufferring = true;
                mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_START);
                usleep(UPLAYER_PAUSE_TIME);
                ulog_info("MEDIA_INFO_BUFFERING_START in renderer_video.cpp");
                continue;
            }
        }
        
        /*备份第一帧视频数据
         Add By HuangWeiqing*/
        pthread_rwlock_rdlock(&mPlayer->mRWLock);
        if (!mPlayer->mIsFirstVideoFramePrepared) {
            if (mPlayer->mYUVQueue->size() == 0) {
                pthread_rwlock_unlock(&mPlayer->mRWLock);
                usleep(UPLAYER_PAUSE_TIME);
                continue;
            }else{
                pkt = (av_link)mPlayer->mYUVQueue->get();
                if (!pkt) {
                    pthread_rwlock_unlock(&mPlayer->mRWLock);
                    ulog_err("URendererVideo::render mYUVQueue->get() == NULL");
                    usleep(UPLAYER_PAUSE_TIME);
                    continue;
                }
                backupLastFrame(pkt);
                ulog_info("backup last frame =====================================");
                mPlayer->mYUVSlotQueue->put(pkt);
                mPlayer->mIsFirstVideoFramePrepared = true;
            }
        }
        pthread_rwlock_unlock(&mPlayer->mRWLock);
#endif
        
#if PLATFORM_DEF != IOS_PLATFORM
		if ( mPlayer->mNeedYUVBufferFull ) {
			usleep(UPLAYER_PAUSE_TIME);
			continue;
		}
#endif

		if ( mPlayer->isPause() || mPlayer->mNeedBufferring) {
            showLastFrame();
			usleep(UPLAYER_PAUSE_TIME);
			continue;
		}
        
#if PLATFORM_DEF != IOS_PLATFORM
		if(0 == mPlayer->mYUVQueue->size()){
			showLastFrame();

			if(!mPlayer->playOver2(mPlayer->getCurrentPosition()) && 0 == mPlayer->mYUVQueue->size() && 0 == mPlayer->mVPacketQueue->size() && 0 == mPlayer->mPCMQueue->size() && 0 == mPlayer->mAPacketQueue->size()){
				if( 0 == last_time ){
					last_time = av_gettime();
				}else if( av_gettime() - last_time > UPLAYER_BUFFERRING_CHECK_TIME ){
					mPlayer->mNeedBufferring = true;
					last_time = 0;
					mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_START);
				}
			}
			usleep(UPLAYER_PAUSE_TIME);
			continue;
		}
#endif
		last_time = 0;

#if PLATFORM_DEF == IOS_PLATFORM
        pthread_rwlock_rdlock(&mPlayer->mRWLock);
        /*
         如果快尽快退后还没有解码一帧音频，直接返回
         要不然你用的时快进快退前的音频解码时间做同步
         */
        if (!mPlayer->mFirstAudioPacketDecoded && (mPlayer->mStreamType & UPLAYER_STREAM_AUDIO)) {
            pthread_rwlock_unlock(&mPlayer->mRWLock);
            usleep(UPLAYER_PAUSE_TIME);
            continue;
        }
#endif
 
#if PLATFORM_DEF == IOS_PLATFORM
        if (mPlayer->mYUVQueue->size() == 0) {
            pthread_rwlock_unlock(&mPlayer->mRWLock);
            usleep(UPLAYER_PAUSE_TIME);
            continue;
        }
#endif
        //获取YUV数据包
		pkt = (av_link) mPlayer->mYUVQueue->get();

		if (!pkt) {
			ulog_err("UDecoderVideo::render mYUVQueue->get() == NULL");
#if PLATFORM_DEF == IOS_PLATFORM
            pthread_rwlock_unlock(&mPlayer->mRWLock);
#endif
			continue;
		}

		//刷新EGL，如果需要刷新的话
		refreshEGL();

		//同步音视频
		if (pkt->pts >= 0) {
			if (synchronize(pkt)) {
				//执行seek操作
				mPlayer->mVPacketQueue->flush();
			}
		}

		//备份数据
		backupLastFrame(pkt);
		//显示图像
		process(pkt);

		//将数据包还回YUV空槽队列
		mPlayer->mYUVSlotQueue->put(pkt);
#if PLATFORM_DEF == IOS_PLATFORM
        pthread_rwlock_unlock(&mPlayer->mRWLock);
#endif
	}

	//释放绘图对象
	#if PLATFORM_DEF != LINUX_PLATFORM
		delete mGraphics;
	#endif

	ulog_info("URendererVideo::render exit");
}
void URendererVideo::stop() {

	if (!isRunning())
		return;

	//释放视频渲染线程
	if (mPlayer && mPlayer->mYUVQueue)
		mPlayer->mYUVQueue->abort();

	//等待线程退出
	wait();
}
int URendererVideo::getRestTime(int diff){

	int rest_time = mFrameInterval;

	if(0 == diff)return mFrameInterval;

#if PLATFORM_DEF != IOS_PLATFORM
	if(diff >UPLAYER_SYNCHRONIZE_THRESHOLD_HIGH)
		skipFrame(UPLAYER_SKIP_LEVEL1);
    
	if(diff < -UPLAYER_SYNCHRONIZE_THRESHOLD_LOW)
		skipFrame(UPLAYER_SKIP_NONE);
#endif
	if (diff >= 2 * UPLAYER_SYNCHRONIZE_THRESHOLD_MAX) {
		rest_time = mFrameInterval * 10 / 40;
	} else if (diff >= UPLAYER_SYNCHRONIZE_THRESHOLD_MAX) {
		//视频慢8帧以上
		rest_time = mFrameInterval * 12 / 40;
	} else if (diff >= UPLAYER_SYNCHRONIZE_THRESHOLD_HIGH) {
		//视频慢5帧到8帧
		rest_time = mFrameInterval * 14 / 40;
	} else if (diff >= UPLAYER_SYNCHRONIZE_THRESHOLD_LOW) {
		//视频慢2帧到5帧
		rest_time = mFrameInterval * 15 / 40;
	} else if (diff >= UPLAYER_SYNCHRONIZE_THRESHOLD) {
		//视频慢1帧到2帧
		rest_time = mFrameInterval * 16 / 40;

	}else if(diff >= 0 && diff < UPLAYER_SYNCHRONIZE_THRESHOLD){
		//已同步
		rest_time = mFrameInterval;
	}else if(diff >= -UPLAYER_SYNCHRONIZE_THRESHOLD){
		//已同步
		rest_time = mFrameInterval * 11 / 10;
	}else if(diff > -(2*UPLAYER_SYNCHRONIZE_THRESHOLD)){
		//视频快1帧到2帧
		rest_time = mFrameInterval * 13 / 10;

	}else if(diff  > -(3*UPLAYER_SYNCHRONIZE_THRESHOLD)){
		//视频快2帧到5帧
		rest_time = mFrameInterval * 14 / 10;

	}else if(diff > -(4*UPLAYER_SYNCHRONIZE_THRESHOLD)){
		//视频快5帧到8帧
		rest_time = mFrameInterval * 15 / 10;

	}else if(diff <= -(4*UPLAYER_SYNCHRONIZE_THRESHOLD) ){
		//视频快8帧以上
		rest_time = mFrameInterval * 16 / 10;
	}
	return rest_time;
}
bool URendererVideo::synchronize(av_link pkt){

	double video_pts;
	double audio_pts;
	double diff;
	int interval;

	//获取音频时间戳

	if(!mPlayer->mLastVideoPts)return false;

	if(mPlayer->mStreamType & UPLAYER_STREAM_AUDIO)	audio_pts = mPlayer->getCurrentPosition();
	else if(mPlayer->mStreamType & UPLAYER_STREAM_VIDEO){
		usleep(mFrameInterval);
		return false;
	}

	video_pts = pkt->pts;

	//同步逻辑
	//单位毫秒
    /*
     这个用的是seek之后的音频解码时间做同步
     Add by HuangWeiqing
     */
#if PLATFORM_DEF == IOS_PLATFORM
    audio_pts = mPlayer->mAudioDecodedPts;
#endif
    diff = audio_pts - video_pts;
	

	//视频慢8帧以上就执行seek操作
	if(diff > UPLAYER_SYNCHRONIZE_THRESHOLD_MAX){
    #if PLATFORM_DEF != IOS_PLATFORM
		skipFrame(UPLAYER_SKIP_LEVEL1);
    #endif
		return true;
	}

	usleep( (interval = getRestTime(diff) ) < UPLAYER_SYNCHRONIZE_MAX_INTERVAL ? interval: UPLAYER_SYNCHRONIZE_MAX_INTERVAL);

	return false;
}

void URendererVideo::skipFrame(UPLAYER_SKIP_FRAME level){

	switch(level){
	case UPLAYER_SKIP_LEVEL1:
		mPlayer->mVideoCodecCtx->skip_loop_filter = AVDISCARD_NONREF;
		mPlayer->mVideoCodecCtx->skip_frame = AVDISCARD_NONREF;
		mPlayer->mVideoCodecCtx->skip_idct = AVDISCARD_BIDIR;
		break;
	case UPLAYER_SKIP_LEVEL2:
		mPlayer->mVideoCodecCtx->skip_loop_filter = AVDISCARD_DEFAULT;
		mPlayer->mVideoCodecCtx->skip_frame = AVDISCARD_NONREF;
		mPlayer->mVideoCodecCtx->skip_idct = AVDISCARD_DEFAULT;
		break;
	case UPLAYER_SKIP_LEVEL3:
		mPlayer->mVideoCodecCtx->skip_loop_filter = AVDISCARD_DEFAULT;
		mPlayer->mVideoCodecCtx->skip_frame = AVDISCARD_NONREF;
		mPlayer->mVideoCodecCtx->skip_idct = AVDISCARD_DEFAULT;
		break;
	case UPLAYER_SKIP_LEVEL4:
		mPlayer->mVideoCodecCtx->skip_loop_filter = AVDISCARD_DEFAULT;
		mPlayer->mVideoCodecCtx->skip_frame = AVDISCARD_NONREF;
		mPlayer->mVideoCodecCtx->skip_idct = AVDISCARD_DEFAULT;
		break;
	case UPLAYER_SKIP_LEVEL5:
		mPlayer->mVideoCodecCtx->skip_loop_filter = AVDISCARD_DEFAULT;
		mPlayer->mVideoCodecCtx->skip_frame = AVDISCARD_NONREF;
		mPlayer->mVideoCodecCtx->skip_idct = AVDISCARD_DEFAULT;
		break;
	default:
		mPlayer->mVideoCodecCtx->skip_loop_filter = AVDISCARD_NONREF;
		mPlayer->mVideoCodecCtx->skip_frame = AVDISCARD_DEFAULT;
		mPlayer->mVideoCodecCtx->skip_idct = AVDISCARD_BIDIR;
		break;
	}
}
void	 URendererVideo::backupLastFrame(av_link frame){

	if(frame && frame->item && frame->size > 0){
		if(!mLastFrame->item){
				mLastFrame->item = av_malloc(frame->size);
				if(!mLastFrame->item){
					ulog_err("URendererVideo::backupLastFrame av_malloc failed 1");
					return;
				}
				mLastFrame->size = frame->size;
		}else if(mLastFrame->size != frame->size){
			av_free(mLastFrame->item);
			mLastFrame->item = av_malloc(frame->size);
			if(!mLastFrame->item){
				ulog_err("URendererVideo::backupLastFrame av_malloc failed 2");
				return;
			}
			mLastFrame->size = frame->size;
		}
		memcpy(mLastFrame->item,frame->item,frame->size);
	}
}
void URendererVideo::refreshEGL(){
    
	if(mPlayer->mNeedInitSurface){
        
    
#if PLATFORM_DEF == IOS_PLATFORM
        mLockRenderVideo->lock();
#endif
            
#if PLATFORM_DEF != LINUX_PLATFORM
		//释放绘图对象
		if(mGraphics)delete mGraphics;

		mGraphics = new UGraphics(mPlayer,mPlayer->mEGL, mPlayer->mVideoWidth,
				mPlayer->mVideoHeight);
#endif
		mPlayer->mNeedInitSurface = false;

		//错误处理
		#if PLATFORM_DEF != LINUX_PLATFORM
			if (!mGraphics) {
				ulog_info("URendererVideo::render new UGraphics failed");
				set_player_error_code(mPlayer, ERROR_SOFT_PLAYER_NO_MEMORY);
				mPlayer->notifyMsg(MEDIA_INFO_PLAYERROR);
			}
		#endif
    
#if PLATFORM_DEF == IOS_PLATFORM
        mLockRenderVideo->unlock();
#endif
	}
        
}
void	 URendererVideo::showLastFrame(){

	refreshEGL();

#if PLATFORM_DEF == IOS_PLATFORM
    mLockRenderVideo->lock();
    if (mRenderVideo) {
#endif
        
if(mLastFrame->item)mGraphics->UpdateWindow(mLastFrame->item);
        
#if PLATFORM_DEF == IOS_PLATFORM
    }
    mLockRenderVideo->unlock();
#endif

}

URendererVideo::~URendererVideo(){

	if(mLastFrame->item){
		av_free(mLastFrame->item);
		mLastFrame->item = NULL;
	}
	if(mLastFrame)av_free(mLastFrame);
	mLastFrame = NULL;
    
#if PLATFORM_DEF == IOS_PLATFORM
    
    if(mFirstFrame->item){
        av_free(mFirstFrame->item);
        mFirstFrame->item = NULL;
    }
    if(mFirstFrame)av_free(mFirstFrame);
    mFirstFrame = NULL;
    
    if (mLockRenderVideo) {
        delete mLockRenderVideo;
    }
    mLockRenderVideo = NULL;
#endif

}

#if PLATFORM_DEF == IOS_PLATFORM
void URendererVideo::setRenderVideo(bool shown){
    mLockRenderVideo->lock();
    mRenderVideo = shown;
    mLockRenderVideo->unlock();

}

void URendererVideo::backupFirstFrame(av_link frame){
    if(frame && frame->item && frame->size > 0){
        if(!mFirstFrame->item){
            mFirstFrame->item = av_malloc(frame->size);
            if(!mFirstFrame->item){
                ulog_err("URendererVideo::backupLastFrame av_malloc failed 1");
                return;
            }
            mFirstFrame->size = frame->size;
        }else if(mFirstFrame->size != frame->size){
            av_free(mFirstFrame->item);
            mFirstFrame->item = av_malloc(frame->size);
            if(!mFirstFrame->item){
                ulog_err("URendererVideo::backupLastFrame av_malloc failed 2");
                return;
            }
            mFirstFrame->size = frame->size;
        }
        memcpy(mFirstFrame->item,frame->item,frame->size);
    }
}

void URendererVideo::showFirstFrame(){
    refreshEGL();
    
#if PLATFORM_DEF == IOS_PLATFORM
    mLockRenderVideo->lock();
    if (mRenderVideo) {
#endif
        if(mFirstFrame->item)mGraphics->UpdateWindow(mFirstFrame->item);
        
#if PLATFORM_DEF == IOS_PLATFORM
    }
    mLockRenderVideo->unlock();
#endif
}

#endif
