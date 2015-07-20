#include "urenderer_audioqueue.h"

#define NUMBER_OF_BUFFERS 2
#define FRAME_SIZE (4096<<1)

static AudioQueueBufferRef buffers[NUMBER_OF_BUFFERS];



URendererAudioQueue::URendererAudioQueue(UPlayer* player,int channels, int samplerate,int samplefmt):IRendererAudio(player,channels,samplerate,samplefmt),UThread("URendererAudioQueue::URendererAudioQueue"),mLock(ULOCK_TYPE_NORMAL){

//    printf("URendererAudioQueue::URendererAudioQueue\n");
	//设置播放器对象指针
	mPlayer = player;

	//创建播放引擎和播放对象
	if(!mPlayer || createAudioQueuePlayer(channels,samplerate)){
		//设置出错标记
		mPrepared = false;
	}
	//设置音频渲染器状态
	mPrepared = true;
}


URendererAudioQueue::~URendererAudioQueue(){

//	AudioQueueDispose(this->mAudioQueue, false);
    AudioQueueDispose(this->mAudioQueue, true);
}

int URendererAudioQueue::createAudioQueuePlayer(int channels, int samplerate){

    OSStatus status = 0;

    // Setup the audio device.
    AudioStreamBasicDescription deviceFormat;
    deviceFormat.mSampleRate = samplerate;
    deviceFormat.mFormatID = kAudioFormatLinearPCM;
    deviceFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger; // Signed integer, little endian
//     deviceFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked; // Signed integer, little endian
    deviceFormat.mBytesPerPacket = 4;
    deviceFormat.mFramesPerPacket = 1;
    deviceFormat.mBytesPerFrame = 4;
    deviceFormat.mChannelsPerFrame = 2;
    deviceFormat.mBitsPerChannel = 16;
    deviceFormat.mReserved = 0;

    // Create a new output AudioQueue for the device.
    status = AudioQueueNewOutput(&deviceFormat,         // Format
                                 AudioQueueCallback,   					 // Callback
                                 this->mPlayer,									// User data, passed to the callback
                                 NULL, //CFRunLoopGetMain(),    					// RunLoop, CFRunLoopGetMain()
                                 kCFRunLoopCommonModes,				 // RunLoop mode, kCFRunLoopCommonModes
                                 0,                     								// Flags ; must be zero (per documentation)...
                                 &(this->mAudioQueue)); 				// Output

    // This will be used for boosting the audio without the need of a mixer (floating-point conversion is expensive on ARM)
    // AudioQueueSetParameter(p_sys->audioQueue, kAudioQueueParam_Volume, 12.0); // Defaults to 1.0

    status = AudioQueueStart(this->mAudioQueue, NULL);

    return true;

}
void URendererAudioQueue::AudioQueueCallback(void * inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer){

    OSStatus status = 0;

    UPlayer* uplayer = (UPlayer*)inUserData;
    

    //将数据包还回PCM空槽队列
    if(!uplayer->mStop && inBuffer->mUserData) {
        uplayer->mPCMSlotQueue->put(inBuffer->mUserData);
    } else {
        ulog_info("inBuffer->mUserData is empty!");
    }
	status = AudioQueueFreeBuffer(inAQ,inBuffer);
    
	if(status){
		ulog_err("AudioQueueFreeBuffer failed");
		return;
	}
}
void URendererAudioQueue::start(){

    ulog_info("URendererAudioQueue::start enter");
    if (this->UThread::isRunning()) {
        restart();
    }
    else {
        this->UThread::start();
    }
}
void URendererAudioQueue::stop(){

    //释放音频渲染线程
	if(mPlayer && mPlayer->mPCMQueue)mPlayer->mPCMQueue->abort();
	AudioQueueStop(this->mAudioQueue, false);
    wait();
    
}
void URendererAudioQueue::handleRun(){

	//音频渲染线程
	render();

}
void URendererAudioQueue::render(){


	AudioQueueBufferRef buffer = NULL;
    OSStatus status = 0;

	av_link	pcm_pkt;
    // 标记当前是否处于暂停状态
    bool is_pause = false;

	while(!mPlayer->isStop()){
        
        if (!(mPlayer->mStreamType & UPLAYER_STREAM_AUDIO)) {
            usleep(UPLAYER_PAUSE_TIME);
            continue;
        }
        
        if (mPlayer->isPause()) {
            if (!is_pause) {
                // 第一次进入暂停状态
                is_pause = true;
            }
            usleep(UPLAYER_PAUSE_TIME);
			continue;
        }
        else {
            if (is_pause) {
                // 从暂停状态恢复
                ulog_info("restart from puase status");
                is_pause = false;
                restart();
            }
        }
		//获取PCM数据包
        pthread_rwlock_rdlock(&mPlayer->mRWLock);
        if (mPlayer->mPCMQueue->size() == 0) {
            pthread_rwlock_unlock(&mPlayer->mRWLock);
            usleep(UPLAYER_PAUSE_TIME);
            continue;
        }
        
		pcm_pkt = (av_link)mPlayer->mPCMQueue->get();
		if(!pcm_pkt){
			ulog_err("URendererAudioTrack::render mPCMQueue->get() == NULL");
            pthread_rwlock_unlock(&mPlayer->mRWLock);
            usleep(UPLAYER_PAUSE_TIME);
			continue;
		}
        /*
         加锁的目的就是为了防止调用AudioQueueReset的时候，往queue里面添加数据，造成死锁
         */
        mLock.lock();
		//处理数据包
	    status = AudioQueueAllocateBuffer(this->mAudioQueue, pcm_pkt->size, &buffer);
	    if(status){
	    	ulog_err("URendererAudioTrack::render AudioQueueAllocateBuffer failed");
            mPlayer->mPCMSlotQueue->put(pcm_pkt);
            mLock.unlock();
            pthread_rwlock_unlock(&mPlayer->mRWLock);
            usleep(UPLAYER_PAUSE_TIME);
	    	continue;
	    }
        memcpy( buffer->mAudioData, pcm_pkt->item, pcm_pkt->size );
        buffer->mAudioDataByteSize = pcm_pkt->size;

        buffer->mUserData = pcm_pkt;
	    status = AudioQueueEnqueueBuffer(mAudioQueue, buffer, 0, NULL);
        mLock.unlock();
        pthread_rwlock_unlock(&mPlayer->mRWLock);
    }

    ulog_info("audio renderer exit");
}

void URendererAudioQueue::restart()
{
    /*
     为什么要加锁：
        原因在与调用AudioQueueReset的同时如果往queue里面添加buffer的时候，
     有可能会出现问题
     */
    mLock.lock();
    AudioQueueReset(this->mAudioQueue);
    OSStatus rc = AudioQueueStart(mAudioQueue, NULL);
    if (rc) {        
        if (rc == kAudioSessionNotActiveError) {
            rc = AudioSessionSetActive(true);
            if (rc) {
               ulog_info("startPlaying - AudioSessionSetActive(true) returned %ld", rc);
            } else {
                ulog_info("startPlaying - restarted Audio Session");
            }
        }
    }
    mLock.unlock();
}

#if PLATFORM_DEF == IOS_PLATFORM

void URendererAudioQueue::flush()
{
    AudioQueueFlush(this->mAudioQueue);
}

void URendererAudioQueue::setVolume(float gain)
{
    AudioQueueSetParameter(mAudioQueue, kAudioQueueParam_Volume, gain);
}

float URendererAudioQueue::getVolume()
{
    float gain;
    AudioQueueGetParameter(mAudioQueue, kAudioQueueParam_Volume, &gain);
    return gain;
}

#endif
