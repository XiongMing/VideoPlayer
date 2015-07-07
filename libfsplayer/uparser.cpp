#include "uparser.h"
#include "uplayer.h"
#include <assert.h>
#if PLATFORM_DEF == IOS_PLATFORM
#include "urenderer_audioqueue.h"
#endif
extern "C" {
#include "libavutil/time.h"
} // end of extern C

UParser::UParser(char* name, UPlayer* player) :mPlayer(player),UThread(name) {

	//确保播放器引用不为空
	assert(player);

	mNeedSendingPlayEndCode = true;
}

UParser::~UParser() {
}

void UParser::stop() {
	ulog_info("in UParser::stop()");
	if (!isRunning())
		return;

	//退出拆包线程
	if (mPlayer && mPlayer->mASlotQueue)
		mPlayer->mASlotQueue->abort();
	if (mPlayer && mPlayer->mVSlotQueue)
		mPlayer->mVSlotQueue->abort();

	//等待线程退出
	wait();
	ulog_info("UParser::stop() over");
}

void UParser::handleRun() {

#if PLATFORM_DEF == ANDROID_PLATFORM
    //注册用于崩溃捕获的线程id
	mPlayer->mCrashHandler.registerTid();
#endif

	//拆包线程
	parse();

#if PLATFORM_DEF == ANDROID_PLATFORM
	//取消注册用于崩溃捕获的线程id
	mPlayer->mCrashHandler.unRegisterTid();
#endif
}

int UParser::seek() {

	int ret = 0;

	int64_t timestamp;
	//int64_t seek_position = -1;

	ulog_info("UParser::seek enter");

	//如果seek的目标时间，大于等于视频总时长，且播放状态不是暂停时，直接发送结束播放状态，用于退出视频

#if PLATFORM_DEF == IOS_PLATFORM
    /*这里如果标示为等于号的时候，可能出现相等即发送播放结束信号
     可能那时候视频还没播放结束，你就发送这个消息
     */
    int64_t startTime = (mPlayer->mStreamType & UPLAYER_STREAM_VIDEO) ? mPlayer->mVStartTime : mPlayer->mAStartTime;
    int64_t seekPosition = (0 != startTime ) ? (mPlayer->mSeekPosition - startTime) : mPlayer->mSeekPosition;
    if (seekPosition > mPlayer->mMediaFile->duration / 1000 && mPlayer->mState != UPLAYER_PAUSED) {
#else
	if (mPlayer->mSeekPosition >= mPlayer->mMediaFile->duration / 1000
			&& mPlayer->mState != UPLAYER_PAUSED) {
#endif
		ulog_info(
				"UParser::seek mPlayer->mMediaFile->duration = %lld", mPlayer->mMediaFile->duration);
		if (mNeedSendingPlayEndCode) {
			mPlayer->notifyMsg(MEDIA_INFO_COMPLETED);
			mNeedSendingPlayEndCode = false;
		}

		mPlayer->resetSeeking();
		ret = -1;
		return ret;
	}

	//清空音视频队列
	mPlayer->flush();
	//seek

	//获得seek到的时间戳
#if PLATFORM_DEF != IOS_PLATFORM
	timestamp = mPlayer->mSeekPosition
			* mPlayer->mTimeBase[mPlayer->mSeekStreamIndex].den / 1000;
#else
        timestamp = mPlayer->mSeekPosition
        * mPlayer->mTimeBase[mPlayer->mSeekStreamIndex].den / (1000 * mPlayer->mTimeBase[mPlayer->mSeekStreamIndex].num);
#endif

	ret = av_seek_frame(mPlayer->mMediaFile, mPlayer->mSeekStreamIndex,
			timestamp, AVSEEK_FLAG_BACKWARD);
	ulog_info("call av_seek_frame over");
	if (ret < 0) {
		ulog_err("UParser::seek av_seek_frame error");
		// added by Bruce
		set_player_error_code(this,ERROR_SOFT_PLAYER_SEEK_FAILED);
		mPlayer->notifyMsg(MEDIA_INFO_SEEK_ERROR,ERROR_SOFT_PLAYER_SEEK_FAILED);
	}
#if PLATFORM_DEF == IOS_PLATFORM
	if (mPlayer->mRendererAudio != NULL) {
		((URendererAudioQueue*)mPlayer->mRendererAudio)->start();
	}
#endif


	//重置seek状态
	mPlayer->resetSeeking();

	mPlayer->notifyMsg(MEDIA_SEEK_COMPLETE);
    
//    ulog_info("(((((((((((((((((mApacketQueueSize: %d, mVPacketSize: %d, mASlotSize: %d, mVSlotSize: %d", mPlayer->mAPacketQueue->size(),
//           mPlayer->mVPacketQueue->size(), mPlayer->mASlotQueue->size(), mPlayer->mVSlotQueue->size());

	return ret;
}

void UParser::parse() {
	av_link pkt;
	AVPacket packet;
	UQueue* slot_queue, *packet_queue;

	int64_t last_packet_pts;
	int64_t begin_time = av_gettime();
	int64_t end_time = 0;
    
#if PLATFORM_DEF == IOS_PLATFORM
    bool hasSendToPlayToTheEnd = false;
#endif

	int ret;

	ulog_info("UParser::parse enter");

	while (!mPlayer->isStop()) {
//            ulog_info("apacketsize: %d, vpacketSize: %d, UPLAYERsize: %d, buffering: %s, aslotQueueSize: %d, vslotQueueSize: %d, mYUVQueue: %d", mPlayer->mAPacketQueue->size(), mPlayer->mVPacketQueue->size(), UPLAYER_VIDEO_PACKET_BUFFERRING_NUM, mPlayer->mNeedBufferring ? "true" : "false", mPlayer->mASlotQueue->size(), mPlayer->mVSlotQueue->size(), mPlayer->mYUVQueue->size());

//        ulog_info("mYUVSlotQueue: %d", mPlayer->mYUVSlotQueue->size());
        
#if PLATFORM_DEF == IOS_PLATFORM
        
//        if (mPlayer->mNeedBufferring) {
//            if ( mPlayer->playOver2(mPlayer->mLastPacketPts) ||
//                (mPlayer->mVPacketQueue->size()>=UPLAYER_VIDEO_PACKET_MAX_BUFFERRING_NUM)||
//                ((mPlayer->mStreamType & UPLAYER_STREAM_AUDIO) && mPlayer->mAPacketQueue->size()>= UPLAYER_VIDEO_PACKET_MAX_BUFFERRING_NUM)) {
//                mPlayer->mNeedBufferring = false;
//                mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_END);
//                ulog_info("MEDIA_INFO_BUFFERING_END send in urenderer_video.cpp");
//            }
//        }
//
        
        //第一次启动发送准备完成的操作
        if (0 == mPlayer->mPreparedDone){
            bool playOver2 = mPlayer->playOver2(mPlayer->mLastPacketPts, mPlayer->mAudioOrVideo);
            //缓冲
            if (mPlayer->mNeedBufferring){
                //parser结束
                if (playOver2){
                    //如果yuv空槽为0，或者不是0但是vPacketQueue的size为0
                    if(0 == mPlayer->mYUVSlotQueue->size() ||(0 != mPlayer->mYUVSlotQueue->size() && 0 == mPlayer->mVPacketQueue->size())){
                        mPlayer->mPreparedDone = 2;
                        mPlayer->notifyMsg(MEDIA_INFO_PREPARED);
                    }
                }else{
                    if ((mPlayer->mVPacketQueue->size() < mPlayer->mMaxBufferingQueueNum) && ((mPlayer->mStreamType & UPLAYER_STREAM_AUDIO) && mPlayer->mASlotQueue->size()==0)) {
                        mPlayer->mMaxBufferingQueueNum = mPlayer->mVPacketQueue->size();
                        if ((mPlayer->mMinBufferingQueueNum + 60) >= mPlayer->mMaxBufferingQueueNum) {
                            mPlayer->mMinBufferingQueueNum = mPlayer->mMaxBufferingQueueNum - 60;
                            mPlayer->mMinBufferingQueueNum = mPlayer->mMinBufferingQueueNum < 0 ? 0 : mPlayer->mMinBufferingQueueNum;
                        }
                        ulog_info("Reset mMaxBufferingQueueNum: %d", mPlayer->mVPacketQueue->size());
                    }
                    
                    //满足缓冲下限
                    if((mPlayer->mVPacketQueue->size()>=/*UPLAYER_VIDEO_PACKET_MAX_BUFFERRING_NUM*/mPlayer->mMaxBufferingQueueNum)/*||
                       ((mPlayer->mStreamType & UPLAYER_STREAM_AUDIO) && mPlayer->mAPacketQueue->size()>= mPlayer->mMaxBufferingQueueNum/*UPLAYER_VIDEO_PACKET_MAX_BUFFERRING_NUM//)*/){
                        //yuv空槽的大小为0
                        if (mPlayer->mYUVSlotQueue->size() == 0){
                            mPlayer->mPreparedDone = 2;
                            mPlayer->notifyMsg(MEDIA_INFO_PREPARED);
                        }
                    }
                }
                
            }else{
                /*
                1.如果yuv空槽为0
                2.yuv空槽不是0，但是parser结束，同时mVPacketQueue的大小为0
                 以上这两种情况发送准备完成消息
                 */
                if (0 == mPlayer->mYUVSlotQueue->size() || (0 != mPlayer->mYUVSlotQueue->size() && 0 == mPlayer->mVPacketQueue->size() && playOver2)){
                    mPlayer->mPreparedDone = 2;
                    mPlayer->notifyMsg(MEDIA_INFO_PREPARED);
                }
            }
        }
        
        
        bool sign = (0 == mPlayer->mPreparedDone || 1 == mPlayer->mPreparedDone);
        
        if (mPlayer->mNeedBufferring) {
            
            //parser结束发送缓冲结束消息
            if (mPlayer->playOver2(mPlayer->mLastPacketPts, mPlayer->mAudioOrVideo)){
                if(!sign){
                    mPlayer->mNeedBufferring = false;
                    mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_END);
                    ulog_info("MEDIA_INFO_BUFFERING_END send in urenderer_video.cpp");
                }else{
                    if (0 == mPlayer->mYUVSlotQueue->size() ||(0 != mPlayer->mYUVSlotQueue->size() && 0 == mPlayer->mVPacketQueue->size())) {
                        mPlayer->mNeedBufferring = false;
                        mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_END);
                        mPlayer->mPreparedDone = 2;
                        ulog_info("MEDIA_INFO_BUFFERING_END send in urenderer_video.cpp");
                    }
                }
            }else{
                
                if ((mPlayer->mVPacketQueue->size() < mPlayer->mMaxBufferingQueueNum) && ((mPlayer->mStreamType & UPLAYER_STREAM_AUDIO) && mPlayer->mASlotQueue->size()==0)) {
                    mPlayer->mMaxBufferingQueueNum = mPlayer->mVPacketQueue->size();
                    if ((mPlayer->mMinBufferingQueueNum + 60) >= mPlayer->mMaxBufferingQueueNum) {
                        mPlayer->mMinBufferingQueueNum = mPlayer->mMaxBufferingQueueNum - 60;
                        mPlayer->mMinBufferingQueueNum = mPlayer->mMinBufferingQueueNum < 0 ? 0 : mPlayer->mMinBufferingQueueNum;
                    }
                    ulog_info("Reset mMaxBufferingQueueNum: %d", mPlayer->mVPacketQueue->size());
                }
                
                if((mPlayer->mVPacketQueue->size()>=/*UPLAYER_VIDEO_PACKET_MAX_BUFFERRING_NUM*/mPlayer->mMaxBufferingQueueNum)/*||((mPlayer->mStreamType & UPLAYER_STREAM_AUDIO) && mPlayer->mAPacketQueue->size()>= /*UPLAYER_VIDEO_PACKET_MAX_BUFFERRING_NUM//mPlayer->mMaxBufferingQueueNum)*/){
                    
                    if (!sign || (sign && 0 == mPlayer->mYUVSlotQueue->size())) {
                        mPlayer->mNeedBufferring = false;
                        mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_END);
                        mPlayer->mPreparedDone = 2;
                        ulog_info("MEDIA_INFO_BUFFERING_END send in urenderer_video.cpp");
                    }
//                    if (!sign) {
//                        mPlayer->mNeedBufferring = false;
//                        mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_END);
//                        ulog_info("MEDIA_INFO_BUFFERING_END send in urenderer_video.cpp");
//                    }else{
//                        if(0 == mPlayer->mYUVSlotQueue->size()){
//                            mPlayer->mNeedBufferring = false;
//                            mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_END);
//                            mPlayer->mPreparedDone = 2;
//                            ulog_info("MEDIA_INFO_BUFFERING_END send in urenderer_video.cpp");
//
//                        }
//                    }
                }
            }
        }
        //Shows that you set to skip end.
        if (mPlayer->mEndPlaybackTime != 0 &&  mPlayer->playOver(mPlayer->getCurrentPosition())) {
//            pthread_rwlock_wrlock(&mPlayer->mRWLock);
//            mPlayer->flush();
//            pthread_rwlock_unlock(&mPlayer->mRWLock);
//            if (!mPlayer->mRepeatMode)
//                break;
//            else
//                mPlayer->notifyMsg(MEDIA_INFO_PLAY_TO_END);
            
            if (!hasSendToPlayToTheEnd) {
                pthread_rwlock_wrlock(&mPlayer->mRWLock);
                mPlayer->flush();
                pthread_rwlock_unlock(&mPlayer->mRWLock);

                mPlayer->notifyMsg(MEDIA_INFO_PLAY_TO_END);
                hasSendToPlayToTheEnd = true;
            }
            usleep(UPLAYER_PAUSE_TIME);
            continue;
        }
#endif

		//seek功能
		if (mPlayer->isSeeking()) {

#if PLATFORM_DEF == IOS_PLATFORM
            /*
             加写锁，防止我这边刷新的时候，音频解码，视频解码，视频渲染线程进入工作转态
             原因是这些线程的执行会保存一些临时变量，临时变量保存了pts得值与快进的时候
             有冲突，有可能快进后用的时快进前的音频pts，但是视频用的是新的，或者音频用
             的时新的，但是视频的pts用的旧的，这样在做音视频同步的时候，就会出现画面丢帧
             或者慢放 
             解决方案：
             采用读写锁：快进加写锁，音频解码、视频解码、视频渲染加读锁，seek的时候音频解码、
             视频解码、视频渲染不会工作，但是如果这三者里面有一个加锁，快进就不能执行
             Add by HuangWeqing
             */
            pthread_rwlock_wrlock(&mPlayer->mRWLock);
            /*
             该标志用于标示不管是快进还是快退时候，用该动作之后的第一针音频解码时间做同步
             Add by HuangWeiqing
             */
            mPlayer->mFirstAudioPacketDecoded = false;
            mPlayer->mPreparedDone = 1;
            mPlayer->mFirstVideoFrameDecoded = false;
#endif
			ret = seek();
            
#if PLATFORM_DEF == IOS_PLATFORM
            pthread_rwlock_unlock(&mPlayer->mRWLock);
#endif

			if (ret < 0) {
				ulog_info("parse thread exit! seek failed");
				break;
			}
		}

		if( 0 == mPlayer->mVSlotQueue->size() || 0 == mPlayer->mASlotQueue->size() ){

			//ulog_info("mPlayer->mASlotQueue->size()=%d, mPlayer->mAPacketQueue->size()=%d,mPlayer->mPCMSlotQueue->size()=%d,mPlayer->mPCMQueue->size()=%d",mPlayer->mASlotQueue->size(),mPlayer->mAPacketQueue->size(),mPlayer->mPCMSlotQueue->size(),mPlayer->mPCMQueue->size());
			usleep(UPLAYER_PAUSE_TIME);
			continue;
		}

		ret = av_read_frame(mPlayer->mMediaFile, &packet);
#if PLATFORM_DEF == IOS_PLATFORM
        
        if(0 == ret && mPlayer->mEof){
            mPlayer->mEof = false;
        }
        
        if(AVERROR_EOF == ret && !mPlayer->mEof)
            mPlayer->mEof = true;
#endif

		if (ret < 0) {

				if(0 == end_time){
					end_time = av_gettime();
					usleep(UPLAYER_PAUSE_TIME);
					continue;
				}
#if PLATFORM_DEF != IOS_PLATFORM
            if (mPlayer->playOver(mPlayer->getCurrentPosition()) && mPlayer->mVPacketQueue->size() == 0
						&& mPlayer->mAPacketQueue->size() == 0 && mPlayer->mYUVQueue->size() == 0 && mPlayer->mPCMQueue->size() == 0)
#else
                if (mPlayer->mEof && mPlayer->mVPacketQueue->size() == 0
                    && mPlayer->mAPacketQueue->size() == 0 && mPlayer->mYUVQueue->size() == 0 && mPlayer->mPCMQueue->size() == 0)
#endif  
                {
                    
#if PLATFORM_DEF == IOS_PLATFORM
                    
                    if (!hasSendToPlayToTheEnd) {
                        mPlayer->notifyMsg(MEDIA_INFO_PLAY_TO_END);
                        hasSendToPlayToTheEnd = true;
                    }
                    usleep(UPLAYER_PAUSE_TIME);
                    continue;

                    
//                        if (mPlayer->isRepeatMode()) {
//                            if (!hasSendToPlayToTheEnd) {
//                                mPlayer->notifyMsg(MEDIA_INFO_PLAY_TO_END);
//                                hasSendToPlayToTheEnd = true;
//                            }
//                            usleep(UPLAYER_PAUSE_TIME);
//                            continue;
//                        }
#else
					ulog_info("uplayer buffer thread read media file over");
					break;
#endif
				}

				if (av_gettime() - end_time < UPLAYER_NETWORK_TRY_TIME) {
					//ulog_err("MEDIA_INFO_NETWORK_ERROR, continue ");
					usleep(UPLAYER_PAUSE_TIME);
					continue;
				}

				//待缓冲数据播完,发送网络错误消息
				//当缓冲区空了，才发断网消息，避免出现avi视频因为无时间戳，直接退出播放的情况
				if (mPlayer->mVPacketQueue->size() == 0 && mPlayer->mAPacketQueue->size() == 0) {
						set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_AV_READ_FRAME_FAILED);
						ulog_err("MEDIA_INFO_NETWORK_DISCONNECTED");
						mPlayer->notifyMsg(MEDIA_INFO_NETWORK_DISCONNECTED,ERROR_SOFT_PLAYER_AV_READ_FRAME_FAILED);
						break;
				}
		}
		end_time = 0;
#if PLATFORM_DEF == IOS_PLATFORM
        if (hasSendToPlayToTheEnd) {
            hasSendToPlayToTheEnd = false;
        }
#endif
		//记录最后一帧视频帧的pts
		if (AV_NOPTS_VALUE == packet.pts) {
			ulog_err("UParser::parse AV_NOPTS_VALUE == packet.pts");
			last_packet_pts = 0;
#if PLATFORM_DEF == IOS_PLATFORM
            //如果没有视频pts得话，使用dts作为显示时间
            if (AV_NOPTS_VALUE != packet.dts) {
                last_packet_pts = packet.dts * 1000;
            }
#endif
		}else{
			last_packet_pts = packet.pts;
			last_packet_pts *= 1000;
		}
        
		if (mPlayer->mAudioStreamIndex == packet.stream_index) {
			//获得音频的空槽队列和数据包队列
			slot_queue = mPlayer->mASlotQueue;
			packet_queue = mPlayer->mAPacketQueue;

			last_packet_pts *= av_q2d(mPlayer->mTimeBase[mPlayer->mAudioStreamIndex]);
#if PLATFORM_DEF == IOS_PLATFORM
            mPlayer->mAudioOrVideo = false;
#endif
		} else if (mPlayer->mVideoStreamIndex == packet.stream_index) {
			//获得视频的空槽队列和数据包队列
			slot_queue = mPlayer->mVSlotQueue;
			packet_queue = mPlayer->mVPacketQueue;
			last_packet_pts *= av_q2d(mPlayer->mTimeBase[mPlayer->mVideoStreamIndex]);
#if PLATFORM_DEF == IOS_PLATFORM
            mPlayer->mAudioOrVideo = true;
#endif

		} else {
			ulog_err("free packet");
			//释放数据包
			av_free_packet(&packet);
			continue;
		}
#if PLATFORM_DEF == IOS_PLATFORM
        //get last packet pts  used by "BUFFERING_START" message in "renderer_video.h" module.
        mPlayer->mLastPacketPts = last_packet_pts;
#endif
        
		//获取数据包空槽
		pkt = (av_link) slot_queue->get();

		if (!pkt) {
			ulog_err("UParser::parse queue->get() == NULL");
			av_free_packet(&packet);
			continue;
		}
		//装载数据包
		pkt->item = &packet;

		//初始化数据包flag
		if (packet.flags & AV_PKT_FLAG_KEY) {
			pkt->flag = UPLAYER_PKT_KEY_FRAME_FLAG;
		} else {
			pkt->flag = UPLAYER_VPACKET_NORMAL_FLAG;
		}

		//将数据包放到队列，等待解码线程获取后解码
		packet_queue->put(pkt);

#if PLATFORM_DEF != IOS_PLATFORM
		if(mPlayer->mMediaFile->duration <= 0){
				if( mPlayer->mNeedBufferring ){
					mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_END);
					mPlayer->mNeedBufferring = false;
				}
		}else	if( mPlayer->mNeedBufferring && ( mPlayer->mAPacketQueue->size() >= UPLAYER_VIDEO_PACKET_BUFFERRING_NUM || mPlayer->mVPacketQueue->size() >= UPLAYER_VIDEO_PACKET_BUFFERRING_NUM ||  mPlayer->playOver2(last_packet_pts))){
				mPlayer->notifyMsg(MEDIA_INFO_BUFFERING_END);
				mPlayer->mNeedBufferring = false;
		}
#endif
//        ulog_info("apacketsize: %d, vpacketSize: %d, UPLAYERsize: %d, buffering: %s, aslotQueueSize: %d, vslotQueueSize: %d", mPlayer->mAPacketQueue->size(), mPlayer->mVPacketQueue->size(), UPLAYER_VIDEO_PACKET_BUFFERRING_NUM, mPlayer->mNeedBufferring ? "true" : "false", mPlayer->mASlotQueue->size(), mPlayer->mVSlotQueue->size());
        
	}
	ulog_info("UParser::parse exit");
	//通知界面播放完成
	if (!mPlayer->isResetting()) {
		if (mNeedSendingPlayEndCode) {
			mPlayer->notifyMsg(MEDIA_INFO_COMPLETED);
			mNeedSendingPlayEndCode = false;
		}
	}
	mPlayer->mState = UPLAYER_PLAYBACK_COMPLETE;

}
