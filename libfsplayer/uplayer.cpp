#include "uplayer.h"
#include "uqueue.h"
#include "uparser.h"
#include "udecoder_audio.h"
#include "urenderer_audio.h"


#if PLATFORM_DEF == ANDROID_PLATFORM
#include "urenderer_opensl_es.h"
#include "ucrash_handler.h"
#elif PLATFORM_DEF == IOS_PLATFORM
#include "urenderer_audioqueue.h"
#elif PLATFORM_DEF == LINUX_PLATFORM
#include "urenderer_audio_linux.h"
#else
//windows or other platform
#endif

#include "udecoder_video.h"
#include "urenderer_video.h"

extern "C" {
#include "libavutil/log.h"
#include "libswresample/swresample.h"
#include "libavformat/url.h"
#include "libavcodec/hevcdsp.h"

    void libde265dec_register(void);
}
extern URLProtocol ff_ucache_protocol;
AVPacket UPlayer::flush_pkt = {};

int get_player_error_code(void *obj){
    
    UPlayer* player = (UPlayer*)obj;
    
    if(!player)return FAILED;
    
    return player->mErrorCode;
}

void set_player_error_code(void *obj,int err){
    
    UPlayer* player = (UPlayer*)obj;
    
    if(!player)return;
    
    if( ERROR_NO == player->mErrorCode ){
    		player->mErrorCode = err;
    }
}
#if PLATFORM_DEF == ANDROID_PLATFORM
UPlayer::UPlayer(const char* dumpPath):mCrashHandler(dumpPath),mLock(ULOCK_TYPE_RECURSIVE),mListener(NULL),mEGL(NULL),mMsgQueue(NULL),mPlayerPreparedTimer(NULL),mNetWorkCheckTimer(NULL)
#else
UPlayer::UPlayer(const char* dumpPath):mLock(ULOCK_TYPE_RECURSIVE),mListener(NULL),mEGL(NULL),mMsgQueue(NULL),mPlayerPreparedTimer(NULL),mNetWorkCheckTimer(NULL)
#endif
{
    ulog_info("UPlayer::UPlayer enter");

    //创建消息队列
    mMsgQueue = new UMsgQueue();
    
    if(!mMsgQueue){
        ulog_err("UPlayer::UPlayer new mMsgQueue failed");
    }
    //启动消息循环线程
    if (pthread_create(&mMsgThread, NULL, msgThread, this)) {
        ulog_err("pthread_create msgThread failed:start");
    }
    
    mIsDataSourceError = true;
    //初始化成员变量
    
	//超时初始化
	mTimeout[UPLAYER_PREPARE_TIMEOUT_TYPE] = UPLAYER_PREPARE_CHECK_TIME;
	mTimeout[UPLAYER_NETWORK_DISCONNECTED_TIMEOUT_TYPE] = UPLAYER_NETWORK_TRY_TIME;

    //设置网路中断，不中断
    interrupt_var_ = 0;
    
#if PLATFORM_DEF == IOS_PLATFORM
    if(0 != pthread_rwlock_init(&mRWLock, NULL)){
        ulog_err("mRWLock initialization failure");
    }
#endif

    init();
}
UPlayer::~UPlayer() {
    
    ulog_info("UPlayer::~UPlayer enter");
    
    //通知消息循环退出
    sendMsg(MEDIA_INFO_EXIT);
    
    //等待消息线程退出
    pthread_join(mMsgThread,NULL);
    
    //关闭timer
    ulog_info("UPlayer::~UPlayer enter 2");
    
    if (mPlayerPreparedTimer) {
        delete mPlayerPreparedTimer;
        mPlayerPreparedTimer = NULL;
    }
    if(mNetWorkCheckTimer){
        delete mNetWorkCheckTimer;
        mNetWorkCheckTimer = NULL;
    }
    
    delete mMsgQueue;
    mMsgQueue = NULL;
    
#if PLATFORM_DEF == ANDROID_PLATFORM
    //释放界面Listener对象
    if (mListener) {
        // For iphone, delete Listener obj is processed in the oc level
        delete mListener;
        mListener = NULL;
    }
    //释放EGL
    if (mEGL) {
        delete mEGL;
        mEGL = NULL;
    }
#endif
    
#if PLATFORM_DEF == IOS_PLATFORM
    pthread_rwlock_destroy(&mRWLock);
#endif

    ulog_info("UPlayer::~UPlayer exit");
}

status_t UPlayer::setDataSource(const char *url,const char *localCachePath){
    
    int len;
    
    //上锁
    lock();
    
    //检测播放器状态
    if (UPLAYER_IDLE != mState) {
        ulog_err("UPlayer::setDataSource UPLAYER_IDLE != mState");
        set_player_error_code(this,ERROR_SOFT_PLAYER_BAD_INVOKE);
        goto setDataSource_err;
    }
#if PLATFORM_DEF != IOS_PLATFORM
    //初始化错误代码
    pfun_set_player_error_code = set_player_error_code;
#endif
    //记录缓存路径
    do{
        
        if(!localCachePath)break;
        
        len = strlen(localCachePath);
        if (!len) {
            ulog_err("UPlayer::setDataSource strlen(localCachePath) == 0");
            localCachePath = NULL;
            break;
        }
        len++;
        mLocalCachePath = (char*)malloc(len);
        if (!mLocalCachePath) {

        	set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
            ulog_err("UPlayer::setDataSource malloc failed");
            goto setDataSource_err;
        }
        strncpy(mLocalCachePath, localCachePath, len);
        
    }while(0);
    
    if(!url){
        ulog_err("UPlayer::setDataSource url == NULL");
        set_player_error_code(this,ERROR_SOFT_PLAYER_ADDRESS_NULL);
        goto setDataSource_err;
    }
    ulog_info("UPlayer::setDataSource %s", url);
    
    //获得播放文件的地址长度
    len = strlen(url);
    
    //错误处理
    if (!len) {
        ulog_err("UPlayer::setDataSource strlen(url) == 0");
        set_player_error_code(this,ERROR_SOFT_PLAYER_ADDRESS_NULL);
        goto setDataSource_err;
    }
    //增加C字符串终止符的长度
    ++len;
    
    
    //分配空间存放播放文件
    if(localCachePath){
        mUrl = (char*) malloc(len + 7);
    }else{
        mUrl = (char*) malloc(len);
    }
    
    
    //错误处理
    if (!mUrl) {
    	set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        ulog_err("UPlayer::setDataSource malloc failed");
        goto setDataSource_err;
    }
    
    //拷贝从界面传递过来的播放文件地址

    if(localCachePath && !strncmp(url,"http",4)){
        strncpy(mUrl, "ucache:", 7);
        strncpy(mUrl+7, url, len);
    }else if(localCachePath && !strncmp(url,"/",1)){
        strncpy(mUrl, "ucache:", 7);
        strncpy(mUrl+7, url, len);
    }else{
        strncpy(mUrl, url, len);
    }

    
    //设置播放器状态为初始化完毕
    mState = UPLAYER_INITIALIZED;
    
    ulog_info("UPlayer::setDataSource done");
    
    //解锁
    unlock();
    return NO_ERROR;
    
setDataSource_err:
    //设置播放器状态为出错状态
    mState = UPLAYER_STATE_ERROR;
    //解锁
    unlock();
    //通知界面
    this->notifyMsg(MEDIA_INFO_DATA_SOURCE_ERROR);
    return FAILED;
    
}

void UPlayer::changeVideoSize(const int width, const int height){
    
    mNewVideoWidth = width;
    mNewVideoHeight = height;
    ulog_info("UPlayer::changeVideoSize enter !!! mNewVideoWidth = %d, mNewVideoHeight = %d", width, height);
    mRestSize = true;
    
}

#if PLATFORM_DEF == ANDROID_PLATFORM
void UPlayer::setVideoSurface(JNIEnv* env, jobject jsurface) {
    
    //上锁
    
    ulog_info("UPlayer::setVideoSurface enter");
    
    mNeedInitSurface = true;
}
#else
void UPlayer::setVideoSurface() {
    
    //上锁
    
    ulog_info("UPlayer::setVideoSurface enter");
    
    mNeedInitSurface = true;
}
#endif

void UPlayer::stop() {
    
    ulog_info("UPlayer::stop enter");
    
    //如果已经退出就不用重复调用
    if (mStop)
        return;

	if(mNetWorkCheckTimer)mNetWorkCheckTimer->stop();
	if(mPlayerPreparedTimer)mPlayerPreparedTimer->stop();

    ulog_info("UPlayer::stop 1");
    //设置网络中断回调
    //avio_set_interrupt_cb(interrupt_cb);
    //设置网路中断，中断
    interrupt_var_ = 1;
    
    ulog_info("UPlayer::stop 1.1");
    pthread_join(mThread, NULL);
    
    
    ulog_info("UPlayer::stop 2");
    
    //上锁
    lock();
    
    mIsDataSourceError = false;
    
    ulog_info("UPlayer::stop 3");
    
    //通知所有线程，播放器需要退出
    mStop = true;
    
    //退出音频渲染线程
    if (mRendererAudio)mRendererAudio->stop();
    
    ulog_info("UPlayer::stop mRendererAudio stop");
    
    //退出视频渲染线程
    if (mRendererVideo)
        mRendererVideo->stop();
    ulog_info("UPlayer::stop mRendererVideo stop");
    
    //退出视频解码线程
    if (mDecoderVideo)
        mDecoderVideo->stop();
    ulog_info("UPlayer::stop mDecoderVideo stop");
    
    //退出音频解码线程
    if (mDecoderAudio)
        mDecoderAudio->stop();
    ulog_info("UPlayer::stop mDecoderAudio stop");
    
    //退出拆包线程
    if (mParser)
        mParser->stop();
    ulog_info("UPlayer::stop mParser stop");
    
    
    //释放视频解码线程对象
    if (mDecoderVideo) {
        delete mDecoderVideo;
        mDecoderVideo = NULL;
    }
    //释放音频解码线程对象
    if (mDecoderAudio) {
        delete mDecoderAudio;
        mDecoderAudio = NULL;
    }
    //释放视频渲染线程对象
    if (mRendererVideo) {
        delete mRendererVideo;
        mRendererVideo = NULL;
    }
    //释放拆包线程对象
    if (mParser) {
        delete mParser;
        mParser = NULL;
    }
    //释放视频数据包队列
    if (mVPacketQueue) {
        delete mVPacketQueue;
        mVPacketQueue = NULL;
    }
    //#if !UPLAYER_FOR_IOS
    //释放音频数据包队列
    if (mAPacketQueue) {
        delete mAPacketQueue;
        mAPacketQueue = NULL;
    }
    //释放音频PCM包队列
    if (mPCMQueue) {
        delete mPCMQueue;
        mPCMQueue = NULL;
    }
    //#endif
    //释放视频YUV包队列
    if (mYUVQueue) {
        delete mYUVQueue;
        mYUVQueue = NULL;
    }
    //检测音频渲染器回调函数中保存的最后一个没有被释放的PCM包，并将它归还PCM空槽队列
    if (mLastPCM && mLastPCM->item) {
        mPCMSlotQueue->put(mLastPCM);
        mLastPCM = NULL;
    }
    //释放视频数据包空槽队列
    if (mVSlotQueue) {
        delete mVSlotQueue;
        mVSlotQueue = NULL;
    }
    //释放音频数据包空槽队列
    if (mASlotQueue) {
        delete mASlotQueue;
        mASlotQueue = NULL;
    }
    //释放音频PCM包空槽队列
    if (mPCMSlotQueue) {
        delete mPCMSlotQueue;
        mPCMSlotQueue = NULL;
    }
    //释放视频YUV包空槽队列
    if (mYUVSlotQueue) {
        delete mYUVSlotQueue;
        mYUVSlotQueue = NULL;
    }
    //释放视频颜色空间转换和缩放对象
    if (mConvertCtx) {
        sws_freeContext(mConvertCtx);
        mConvertCtx = NULL;
    }
    //释放颜色空间转换和缩放后视频帧的缓冲区
    if (mPixels) {
        av_free(mPixels);
        mPixels = NULL;
    }
    //释放颜色空间转换和缩放后的视频帧的外壳
    if (mFrame) {
        av_free(mFrame);
        mFrame = NULL;
    }
    //释放视频解码完毕后的视频帧的外壳
    if (mDecFrame) {
        av_free(mDecFrame);
        mDecFrame = NULL;
    }
    //释放音频解码完毕后的音频帧的外壳
    if (mAudioDecFrame) {
        avcodec_free_frame(&mAudioDecFrame);
    }
    //释放音频重采样后的音频帧外壳
    if(mAudioFrame){
        av_free(mAudioFrame);
        mAudioFrame = NULL;
    }
    //释放音频解码后PCM缓冲区
    if (mSamples) {
        av_free(mSamples);
        mSamples = NULL;
    }
    //判断媒体文件是否已经打开
    if (mMediaFile) {
        //判断视频解码器是否已经打开，打开则关闭它并释放资源
        if (mVideoCodecCtx) {
            avcodec_close(mMediaFile->streams[mVideoStreamIndex]->codec);
            mVideoCodecCtx = NULL;
        }
        //判断音频解码器是否已经打开，打开则关闭它并释放资源
        if (mAudioCodecCtx) {
            avcodec_close(mMediaFile->streams[mAudioStreamIndex]->codec);
            mAudioCodecCtx = NULL;
        }
    }
    //关闭媒体文件
    if (mMediaFile) {
        avformat_close_input(&mMediaFile);
    }
    //释放音频渲染器资源
    if (mRendererAudio) {
        delete mRendererAudio;
        mRendererAudio = NULL;
    }
    
    
    // added by bruce
    if (mTimeBase){
        delete[] mTimeBase;
        mTimeBase = NULL;
    }
    // add by yujunfeng
    //释放音频重采样对象
    if (mResampleEngine){
        swr_free(&mResampleEngine);
    }
    
    this->init(false);
    
    //设置播放器状态为停止状态
    mState = UPLAYER_STOPPED;
    ulog_info("UPlayer::stop 4");
    //解锁
    unlock();
    ulog_info("UPlayer::stop exit");
}
status_t UPlayer::prepareAudio() {
    
    //获得音频流的索引号
    for (int i = 0; i < mMediaFile->nb_streams; i++) {
        if (AVMEDIA_TYPE_AUDIO == mMediaFile->streams[i]->codec->codec_type) {
            mAudioStreamIndex = i;
            break;
        }
    }
#if !DEBUG_ENABLE_AUDIO_STREAM
    mAudioStreamIndex = -1;
#endif
    //错误处理
    if (-1 == mAudioStreamIndex) {
        ulog_err("UPlayer::prepareAudio -1 == mAudioStreamIndex");
        return NO_ERROR;
    }
    
    //设置流类型
    mStreamType |= UPLAYER_STREAM_AUDIO;
    
    //取得音频流
    AVStream* stream = mMediaFile->streams[mAudioStreamIndex];
    
    //错误处理
    if (!stream) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareAudio NULL == stream");
        set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
        return FAILED;
    }

#if PLATFORM_DEF == IOS_PLATFORM
    if (AV_NOPTS_VALUE != mMediaFile->streams[mAudioStreamIndex]->start_time) {
        mAStartTime = mMediaFile->streams[mAudioStreamIndex]->start_time * av_q2d(mTimeBase[mAudioStreamIndex]) * 1000;
    }
    
#endif
    
    //错误处理
    if (!stream->codec) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareAudio NULL == stream->codec");
        set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
        return FAILED;
    }
    
    //获得声道数
    mChannels = stream->codec->channels;
    //获得采样率
    mSampleRate = stream->codec->sample_rate;
    
    //获得采样精度
    mSampleFmt = stream->codec->sample_fmt;
    
    ulog_info(
              "UPlayer::prepareAudio mChannels=%d,mSampleRate=%d,mSampleFmt=%d", mChannels, mSampleRate, mSampleFmt);
    //对声道数和采样率进行判断，防止网络处理时获得的数据非法
    if (mChannels < 1) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareAudio mChannels error:mChannels < 1");
        set_player_error_code(this,ERROR_SOFT_PLAYER_UNSUPPORTED);
        return FAILED;
    }
    if (mSampleRate <= 0) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareAudio mSampleRate error");
        set_player_error_code(this,ERROR_SOFT_PLAYER_UNSUPPORTED);
        return FAILED;
    }
    
    //计算音频解码后PCM缓冲区的大小
    mSamplesSize = UPLAYER_MAX_AUDIO_BUFFER_LEN;
    //分配音频解码后PCM缓冲区
    mSamples = (uint8_t *) av_malloc(mSamplesSize);
    //分配重采样音频PCM缓冲区
    //yujunfeng
    //错误处理
    if (!mSamples) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareAudio NULL = mSamples");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        return FAILED;
    }
    //清空缓冲区
    bzero(mSamples, UPLAYER_MAX_AUDIO_BUFFER_LEN);
    
    //分配音频解码完毕后的音频帧的外壳
    mAudioDecFrame = avcodec_alloc_frame();
    if (!mAudioDecFrame) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareAudio mAudioDecFrame == NULL");
        //TODO
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        return FAILED;
    }
    //分配音频重采样完毕后的音频帧的外壳
    mAudioFrame = avcodec_alloc_frame();
    if (!mAudioFrame) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareAudio mAudioFrame == NULL");
        //TODO
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        return FAILED;
    }
    mAudioFrame->data[0] = mSamples;
    //查找音频的解码器，如aac，aac2，amr and so on
    AVCodec* mAudioCodec;
    mAudioCodec = avcodec_find_decoder(stream->codec->codec_id);
    
    //错误处理
    if (!mAudioCodec) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareAudio NULL == mAudioCodec");
        set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
        return FAILED;
    }
    ulog_info("mAudioCodec = %s", mAudioCodec->name);
    //打开音频解码器
    if (avcodec_open2(stream->codec, mAudioCodec,NULL)) {
        
        ulog_err("UPlayer::prepareAudio avcodec_open2 failed audio");
        mState = UPLAYER_STATE_ERROR;
        set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
        return FAILED;
    }
    //保存音频解码器指针，用于判断解码器是否曾经打开过
    mAudioCodecCtx = stream->codec;
    ulog_info("Audio Codec ID=%d", mAudioCodecCtx->codec->id);
    
    //返回成功状态
    return NO_ERROR;
    
}
status_t UPlayer::prepareVideo() {
    
    //获得视频流的索引号
    for (int i = 0; i < mMediaFile->nb_streams; i++) {
        if (AVMEDIA_TYPE_VIDEO == mMediaFile->streams[i]->codec->codec_type) {
            mVideoStreamIndex = i;
            break;
        }
    }
#if !DEBUG_ENABLE_VIDEO_STREAM
    mVideoStreamIndex = -1;
#endif
    //错误处理
    if (-1 == mVideoStreamIndex) {
        ulog_err("UPlayer::prepareVideo -1 == mVideoStreamIndex");
        return NO_ERROR;
    }
    //设置流类型
    mStreamType |= UPLAYER_STREAM_VIDEO;
    //获得视频流
    AVStream* stream = mMediaFile->streams[mVideoStreamIndex];
    stream_tmp = stream;
    //错误处理
    if (!stream) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo NULL == stream");
        set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
        return FAILED;
    }
    
#if PLATFORM_DEF == IOS_PLATFORM
    if (AV_NOPTS_VALUE != mMediaFile->streams[mVideoStreamIndex]->start_time) {
        mVStartTime = mMediaFile->streams[mVideoStreamIndex]->start_time * av_q2d(mTimeBase[mVideoStreamIndex]) * 1000;
    }
    
#endif
    
    //错误处理
    if (!stream->codec) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo NULL == stream->codec");
        set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
        return FAILED;
    }
    //获得视频宽度和高度
    mVideoWidth = stream->codec->width;
    mVideoHeight = stream->codec->height;
    
    //进行数据判断，防止网络获取数据时出错
    if (mVideoWidth <= 0 || mVideoHeight <= 0) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo mVideoWidth <= 0 || mVideoHeight <= 0");
        set_player_error_code(this,ERROR_SOFT_PLAYER_UNSUPPORTED);
        return FAILED;
    }
    //获得视频长度，单位毫秒
    //错误处理
    if (mMediaFile->duration <= 0) {
        ulog_err("UPlayer::prepareVideo mMediaFile->duration <= 0");
        /*		mState = UPLAYER_STATE_ERROR;
         set_player_error_code(this,ERROR_GET_VIDEO_DURATION_FAILED);
         return FAILED;*/
    }
    AVRational avg_frame_rate = av_guess_frame_rate(mMediaFile,stream,NULL);
    mFrameRate = av_q2d(avg_frame_rate);
    if (mFrameRate < 1.0) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo mFrameRate <= 1.0:stream->r_frame_rate.num=%d,stream->r_frame_rate.den;=%d",stream->r_frame_rate.num,stream->r_frame_rate.den);
        set_player_error_code(this,ERROR_SOFT_PLAYER_UNSUPPORTED);
        return FAILED;
    }
    
    ulog_info(
              "UPlayer::prepareVideo mVideoWidth=%d,mVideoHeight=%d,mMediaFile->duration=%f,fps=%f", mVideoWidth, mVideoHeight, (double)mMediaFile->duration, (double)mFrameRate);
    
    if(AV_PIX_FMT_NONE == stream->codec->pix_fmt){
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo AV_PIX_FMT_NONE == stream->codec->pix_fmt");
        set_player_error_code(this,ERROR_SOFT_PLAYER_UNSUPPORTED);
        return FAILED;
    }
    
    //创建颜色空间转换和缩放对象
    mConvertCtx = sws_getContext(mVideoWidth, mVideoHeight,
                                 stream->codec->pix_fmt, mVideoWidth, mVideoHeight,
                                 //stream->codec->pix_fmt,
                                 PIX_FMT_YUV420P,
                                 //PIX_FMT_RGB565,
                                 //PIX_FMT_YUV444P,   //modify1
                                 SWS_POINT, NULL, NULL, NULL);
    //错误处理
    if (!mConvertCtx) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo mConvertCtx == NULL");
        set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
        return FAILED;
    }
    //获得颜色空间转换和缩放后视频帧缓冲区的大小
    mPixelsPerImage = avpicture_get_size(stream->codec->pix_fmt, mVideoWidth,
                                         mVideoHeight);
    
    //分配缓冲区
    mPixels = (uint8_t *) av_malloc(mPixelsPerImage * sizeof(uint8_t));
    
    //错误处理
    if (!mPixels) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo mPixels == NULL");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        return FAILED;
    }
    
    //分配颜色空间转换和缩放后的视频帧的外壳
    mFrame = avcodec_alloc_frame();
    
    //错误处理
    if (!mFrame) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo mFrame == NULL");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        return FAILED;
    }
    //分配视频解码完毕后的视频帧的外壳
    mDecFrame = avcodec_alloc_frame();
    if (!mDecFrame) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo mDecFrame == NULL");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        return FAILED;
    }
    
    //将YUV缓冲区填充到外壳，以便进行颜色空间转换
    if (avpicture_fill((AVPicture *) mFrame, mPixels,
                       //PIX_FMT_YUV444P,
                       stream->codec->pix_fmt,
                       //PIX_FMT_RGB565,
                       mVideoWidth, mVideoHeight) <= 0) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo avpicture_fill failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
        return FAILED;
    }
    
    //查找视频解码器
    AVCodec* mVideoCodec;
    mVideoCodec = avcodec_find_decoder(stream->codec->codec_id);
    
    //错误处理
    if (!mVideoCodec) {
        mState = UPLAYER_STATE_ERROR;
        ulog_err("UPlayer::prepareVideo NULL == mVideoCodec");
        set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
        return FAILED;
    }
    //打开视频解码器
    if (avcodec_open2(stream->codec, mVideoCodec,NULL)) {
        
        ulog_err("UPlayer::prepareVideo avcodec_open failed video");
        mState = UPLAYER_STATE_ERROR;
        set_player_error_code(this,ERROR_SOFT_PLAYER_FFMPEG);
        return FAILED;
    }
    //保存视频解码器指针，用于判断解码器是否曾经打开过
    mVideoCodecCtx = stream->codec;
    ulog_info("Video Codec ID=%d", mVideoCodecCtx->codec->id);
    //返回成功状态
    return NO_ERROR;
}
status_t UPlayer::prepareAsync() {
    //异步线程启动perpare过程，为播放器初始化资源
    if (pthread_create(&mThread, NULL, prepareThread, this)) {
        ulog_err("pthread_create failed:prepare");
        //通知界面出错状态
        this->sendMsg(MEDIA_INFO_PLAYERROR);
        mState = UPLAYER_STATE_ERROR;
        return FAILED;
    }
    //返回成功状态
    return NO_ERROR;

ios_prepare_err:
    
    mState = UPLAYER_STATE_ERROR;
    int err = get_player_error_code(this);
    unlock();
    
    this->notifyMsg(MEDIA_INFO_PLAYERROR,err);
    return FAILED;
}
void* UPlayer::prepareThread(void* ptr) {
    
    UPlayer* player = (UPlayer*) ptr;

    //确保player有值
    assert(player);

#if PLATFORM_DEF == ANDROID_PLATFORM
    //注册用于崩溃捕获的线程id
    player->mCrashHandler.registerTid();
#endif

    //执行perare调用
    player->_prepare();
#if PLATFORM_DEF == ANDROID_PLATFORM
	//取消注册用于崩溃捕获的线程id
	player->mCrashHandler.unRegisterTid();
#endif
    return (void *)0;
}
status_t UPlayer::prepare(){
    
    this->prepareAsync();
    
    return OK;
}
status_t UPlayer::_prepare() {
    int ret = 0,err = ERROR_NO;
    //上锁
    lock();
    
    ulog_info("UPlayer::prepare enter");
    
    mIsDataSourceError = true;
    //判断播放器状态
    if (UPLAYER_INITIALIZED != mState && UPLAYER_STOPPED != mState) {
        ulog_err("UPlayer::prepare UPLAYER_INITIALIZED != mState || UPLAYER_STOPPED != mState");
        set_player_error_code(this,ERROR_SOFT_PLAYER_BAD_INVOKE);
        goto prepare_err;
    }
    
	//初始化timer

	if(!mPlayerPreparedTimer){
		mPlayerPreparedTimer = new UTimer(this->playerPreparedTimer, this, mTimeout[UPLAYER_PREPARE_TIMEOUT_TYPE]);
		if(!mPlayerPreparedTimer){
			ulog_err("UPlayer::UPlayer new mPlayerPreparedTimer failed");
		}
	}
	if(!mNetWorkCheckTimer){
		mNetWorkCheckTimer = new UTimer(this->networkCheckTimer, this, UPLAYER_NETWORK_CHECK_TIME);
		if(!mNetWorkCheckTimer){
			ulog_err("UPlayer::UPlayer new mNetWorkCheckTimer failed");
		}
	}

    mPlayerPreparedTimer->start();
    
    //设置播放器状态为准备状态
    mState = UPLAYER_PREPARING;
    
#if DEBUG_ENABLE_SET_DATA_SOURCE
   
#if PLATFORM_DEF == ANDROID_PLATFORM
    //注册ucache协议
    ffurl_register_protocol(&ff_ucache_protocol);
#endif
    
    //错误处理
    if (mMediaFile) {
        ulog_err("UPlayer::prepare mMediaFile != NULL");
        goto prepare_err;
    }
    mMediaFile = avformat_alloc_context();
    if (!mMediaFile) {
        ulog_err("avformat_alloc_context failed");
        goto prepare_err;
    }
    //设置中断回调
    mMediaFile->interrupt_callback = interrupt_cb_;
    
    //打开媒体文件
    if(ret = avformat_open_input(&mMediaFile, mUrl, NULL, NULL)) {
        ulog_err("UPlayer::prepare avformat_open_input failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_UNSUPPORTED);
        goto datasource_err;
    }
    if(interrupt_var_){
    	goto datasource_err;
    }
    ulog_info("UPlayer::prepare avformat_open_input done");
    //查找流信息
    if (avformat_find_stream_info(mMediaFile, NULL) < 0) {
        ulog_err("UPlayer::prepare avformat_find_stream_info failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_UNSUPPORTED);
        goto datasource_err;
    }
    if(interrupt_var_){
    	goto datasource_err;
    }
    ulog_info("UPlayer::prepare avformat_find_stream_info done");
    // added by bruce
    // check if streams are valid
    if (!mMediaFile || mMediaFile->nb_streams <= 0) {
        ulog_err("UPlayer::prepare failed, for streams are invalid");
        goto prepare_err;
    }
    // 保存时间戳
    mTimeBase = new AVRational[mMediaFile->nb_streams];
    if (!mTimeBase) {
        ulog_err("UPlayer::prepare failed, for no memory is available");
        goto prepare_err;
    }
    for (int i = 0; i < mMediaFile->nb_streams; ++i)
        mTimeBase[i] = mMediaFile->streams[i]->time_base;
#endif
#if DEBUG_ENABLE_PREPARE
    //准备音频资源
    if (prepareAudio()) {
        ulog_err("UPlayer::prepare prepareAudio failed");
        goto prepare_err;
    }
    ulog_info("UPlayer::prepare prepareAudio done");
    //准备视频资源
    if (prepareVideo()) {
        ulog_err("UPlayer::prepare prepareVideo failed");
        goto prepare_err;
    }
    ulog_info("UPlayer::prepare prepareVideo done");
    // added by bruce
    this->notify(MEDIA_INFO_SET_VIDEO_SIZE, mVideoWidth, mVideoHeight);
    
    if(!mStreamType){
        ulog_err("UPlayer::prepare cannot find any stream info");
        set_player_error_code(this,ERROR_SOFT_PLAYER_UNSUPPORTED);
        goto prepare_err;
    }
    //设置播放时间流索引
    if(mStreamType & UPLAYER_STREAM_AUDIO){
        mPtsStreamIndex = mAudioStreamIndex;
    }else{
        mPtsStreamIndex = mVideoStreamIndex;
    }
    //设置seek流索引
    if(mStreamType &UPLAYER_STREAM_VIDEO ){
        mSeekStreamIndex = mVideoStreamIndex;
    }else{
        mSeekStreamIndex = mAudioStreamIndex;
    }

#if PLATFORM_DEF == IOS_PLATFORM
    if (mSampleRate >= 8000 || mFrameRate >= 12.0) {
        int sampleRate = mSampleRate / 1000;
        int frameRate = mFrameRate;
        float maxBufferingDur = 20;
        float minBufferingDur = 2;
        mAudioSlotQueueNum = sampleRate * maxBufferingDur + 100;
        mVideoSlotQueueNum = frameRate * maxBufferingDur + 100;
        mMaxBufferingQueueNum = (frameRate > sampleRate ?  sampleRate : frameRate) * maxBufferingDur;
        mMinBufferingQueueNum = (frameRate > sampleRate ? frameRate : sampleRate) * minBufferingDur;
//        mMinBufferingQueueNum = mMinBufferingQueueNum < UPLAYER_VIDEO_PACKET_BUFFERRING_MIN_NUM ? UPLAYER_VIDEO_PACKET_BUFFERRING_MIN_NUM : mMinBufferingQueueNum;
    }
#endif
    
#if PLATFORM_DEF == IOS_PLATFORM
    mASlotQueue = new UQueue(UQUEUE_TYPE_SLOT, mAudioSlotQueueNum, UQUEUE_TYPE_PACKET);
#else
    mASlotQueue = new UQueue(UQUEUE_TYPE_SLOT, UPLAYER_MAX_PACKET_SLOT_NUM,
                             UQUEUE_TYPE_PACKET);
#endif
    //创建音频数据包空槽队列
    if (!mASlotQueue) {
        ulog_err("UPlayer::prepare new UQueue mASlotQueue failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
    
    //创建音频数据包队列
    mAPacketQueue = new UQueue(UQUEUE_TYPE_PACKET, 0);
    if (!mAPacketQueue) {
        ulog_err("UPlayer::prepare new UQueue mAPacketQueue failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
#if PLATFORM_DEF == IOS_PLATFORM
    mVSlotQueue = new UQueue(UQUEUE_TYPE_SLOT, mVideoSlotQueueNum, UQUEUE_TYPE_PACKET);
#else
    mVSlotQueue = new UQueue(UQUEUE_TYPE_SLOT,
                             UPLAYER_MAX_VIDEO_PACKET_SLOT_NUM, UQUEUE_TYPE_PACKET);
#endif
    //创建视频数据包空槽队列
    
    if (!mVSlotQueue) {
        ulog_err("UPlayer::prepare new UQueue mVSlotQueue failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
    //创建视频数据包队列
    mVPacketQueue = new UQueue(UQUEUE_TYPE_PACKET, 0);
    if (!mVPacketQueue) {
        ulog_err("UPlayer::prepare new UQueue mVPacketQueue failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
    //创建音频PCM空槽队列
    mPCMSlotQueue = new UQueue(UQUEUE_TYPE_SLOT, UPLAYER_MAX_PCM_SLOT_NUM,
                               UQUQUE_TYPE_AUDIO_DATA);
    if (!mPCMSlotQueue) {
        ulog_err("UPlayer::prepare new UQueue mPCMSlotQueue failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
    //创建音频PCM播放队列
    mPCMQueue = new UQueue(UQUQUE_TYPE_AUDIO_DATA, 0);
    if (!mPCMQueue) {
        ulog_err("UPlayer::prepare new UQueue mPCMQueue failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
    //创建视频YUV空槽队列
    mYUVSlotQueue = new UQueue(UQUEUE_TYPE_SLOT, UPLAYER_MAX_YUV_SLOT_NUM,
                               UQUQUE_TYPE_VIDEO_DATA);
    if (!mYUVSlotQueue) {
        ulog_err("UPlayer::prepare new UQueue mYUVSlotQueue failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
    //创建视频YUV播放队列
    mYUVQueue = new UQueue(UQUQUE_TYPE_VIDEO_DATA, 0);
    if (!mYUVQueue) {
        ulog_err("UPlayer::prepare new UQueue mYUVQueue failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
    ulog_info("UPlayer::prepare pipe line queue done");

    //创建拆包线程对象
    mParser = new UParser("uparser", this);
    if (!mParser) {
        ulog_err("UPlayer::prepare new UParser failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
    //创建音频解码线程对象
    mDecoderAudio = new UDecoderAudio("audio decoder", this);
    if (!mDecoderAudio) {
        ulog_err("UPlayer::prepare new UDecoderAudio failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
    if(mStreamType & UPLAYER_STREAM_AUDIO){
#if PLATFORM_DEF == ANDROID_PLATFORM
        //创建音频渲染器
        mRendererAudio = new URendererAudioOpenSLES(this, UPLAYER_AUDIO_CHANNELS,
                                                    mSampleRate, mSampleFmt);
#elif PLATFORM_DEF == IOS_PLATFORM
        mRendererAudio = new URendererAudioQueue(this, mChannels, mSampleRate, mSampleFmt);
#else
        mRendererAudio = new URendererAudioLinux(this, mChannels, mSampleRate, mSampleFmt);
#endif
        //查看音频渲染器状态，进行错误处理
        if (!mRendererAudio) {
            ulog_err("UPlayer::prepareAudio new URendererAudio failed");
            mState = UPLAYER_STATE_ERROR;
            set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
            goto prepare_err;
        }
    }
    
    //创建视频解码线程对象
    mDecoderVideo = new UDecoderVideo("video decoder", this);
    if (!mDecoderVideo) {
        ulog_err("UPlayer::prepare new UDecoderVideo failed");
        set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
        goto prepare_err;
    }
    ulog_info("UPlayer::prepare pipe line resource done");
    
    if(mStreamType & UPLAYER_STREAM_VIDEO){
        
        //创建视频渲染线程对象
        mRendererVideo = new URendererVideo(this);
        if (!mRendererVideo) {
            ulog_err("UPlayer::prepare new URendererVideo failed");
            set_player_error_code(this,ERROR_SOFT_PLAYER_NO_MEMORY);
            goto prepare_err;
        }
    }
#endif
    
#if PLATFORM_DEF != IOS_PLATFORM
    //设置播放器状态为准备就绪
    mState = UPLAYER_PREPARED;
    //启动拆包线程
    if (mParser && mParser->start()) {
        goto prepare_err;
    }
    ulog_info("UPlayer::prepare done");
    
#else
    mState = UPLAYER_PAUSED;
    if(mRendererAudio) mRendererAudio->start();
    
    //启动视频渲染线程
    if (mRendererVideo && mRendererVideo->start()) goto prepare_err;

    //启动音频解码线程
    if (mDecoderAudio && mDecoderAudio->start()) goto prepare_err;

    //启动视频解码线程
    if (mDecoderVideo && mDecoderVideo->start()) goto prepare_err;
    
    //启动拆包线程
    if (mParser && mParser->start()) goto prepare_err;
#endif
    //解锁
    unlock();
#if PLATFORM_DEF == IOS_PLATFORM
    mNetWorkCheckTimer->start();
    if (!(mStreamType & UPLAYER_STREAM_VIDEO)) {
        this->notifyMsg(MEDIA_INFO_PREPARED);
    }
#else
    //通知界面播放器准备就绪
    this->notifyMsg(MEDIA_INFO_PREPARED);
#endif
    
    //返回成功状态
    return NO_ERROR;
    
datasource_err:
    //设置播放器状态为出错状态
    mState = UPLAYER_STATE_ERROR;
    err = get_player_error_code(this);
    //解锁
    unlock();
    if(mIsDataSourceError)this->notifyMsg(MEDIA_INFO_DATA_SOURCE_ERROR,err);
    
    return FAILED;
prepare_err:
    //设置播放器状态为出错状态
    mState = UPLAYER_STATE_ERROR;
    err = get_player_error_code(this);
    //解锁
    unlock();
    this->notifyMsg(MEDIA_INFO_PREPARE_ERROR,err);
    return FAILED;
}

UPLAYER_STATE UPlayer::start() {

#if PLATFORM_DEF == IOS_PLATFORM
    if (mNeedBufferring) {
        return mState;
    }
#endif
    
    if (UPLAYER_STARTED == mState)
        return UPLAYER_STARTED;
    
    ulog_info("UPlayer::start enter=%d", mState);
    
    //判断播放器状态
    if (UPLAYER_PAUSED == mState) {
        //设置播放器状态为启动状态
        #if PLATFORM_DEF == IOS_PLATFORM
            if (mRendererAudio) {
                URendererAudioQueue *rendererAudio = (URendererAudioQueue *)mRendererAudio;
                rendererAudio->restart();
            }
        #endif
        mState = UPLAYER_STARTED;
        return UPLAYER_PAUSED;
    }
    
#if PLATFORM_DEF != IOS_PLATFORM
    //判断播放器状态，错误处理
    if (UPLAYER_PREPARED != mState) {
        set_player_error_code(this,ERROR_SOFT_PLAYER_BAD_INVOKE);
        ulog_err("UPlayer::start UPLAYER_PREPARED != mState");
        goto start_err1;;
    }

    //上锁
    lock();
    //判断播放器状态，错误处理
    if (UPLAYER_PREPARED != mState) {
        set_player_error_code(this,ERROR_SOFT_PLAYER_BAD_INVOKE);
        ulog_err("UPlayer::start UPLAYER_PREPARED != mState");
        goto start_err;
    }
    //设置播放器为暂停状态
    mState = UPLAYER_PAUSED;
    
    //by qingpengchen notify set video size
    //this->notify(MEDIA_INFO_SET_VIDEO_SIZE, mVideoWidth, mVideoHeight);
    
    //启动视频渲染线程
    if (mRendererVideo && mRendererVideo->start()) {
        mState = UPLAYER_STATE_ERROR;
    }
    
    //启动音频解码线程
    if (mStreamType & UPLAYER_STREAM_AUDIO && mDecoderAudio && mDecoderAudio->start()) {
        mState = UPLAYER_STATE_ERROR;
    }
    
    //启动视频解码线程
    if (mStreamType & UPLAYER_STREAM_VIDEO && mDecoderVideo && mDecoderVideo->start()) {
        mState = UPLAYER_STATE_ERROR;
    }
    
    //错误处理
    if (UPLAYER_STATE_ERROR == mState) {
        ulog_err("UPlayer::start failed");
        goto start_err;
    }
    
    //设置播放器为启动状态
    mState = UPLAYER_STARTED;
    //解锁
    unlock();
    //启动音频渲染线程
    if(mRendererAudio)	mRendererAudio->start();
    mNetWorkCheckTimer->start();
    ulog_info("UPlayer::start ok");
    this->notifyMsg(MEDIA_INFO_VIDEO_START);
    return UPLAYER_PREPARED;
start_err:
    
    //解锁
    unlock();
#else
    
    if (UPLAYER_PAUSED != mState) {
        set_player_error_code(this,ERROR_SOFT_PLAYER_BAD_INVOKE);
        ulog_err("UPlayer::start UPLAYER_PREPARED != mState");
        goto start_err1;;
    }
#endif
start_err1:
	//通知界面出错状态
    this->notifyMsg(MEDIA_INFO_PLAYERROR);
    return UPLAYER_STATE_ERROR;
}
void UPlayer::pause() {
    ulog_info("UPlayer::pause enter");
    
    //判断播放器状态，暂停状态的前置状态只能是暂停或播放
    if (UPLAYER_STARTED == mState) {
        //设置播放器状态为暂停状态
        mState = UPLAYER_PAUSED;
        
    }
    
}

void UPlayer::getDuration(int* msec) {
    
    //ulog_info("UPlayer::getDuration enter");
    //判断播放器状态是否已经准备就绪
    if (mState < UPLAYER_PREPARED) {
        *msec = 0;
    } else {
        *msec = (int)(mMediaFile->duration / 1000);
    }
    
}
void UPlayer::registerAVcodec() {
    ulog_info("UPlayer::registerall enter");
    avcodec_register_all();
    av_register_all();
#if UPLAYER_ENABLE_LIBDE265
    libde265dec_register();
#endif
}
void UPlayer::getCurrentPosition(int *msec) {
    
    //判断播放器状态是否已经准备就绪
    if (mState < UPLAYER_PREPARED) {
        *msec = 0;
    } else {
        
        if(mIsSeeking){
            *msec = mSeekPosition;
        }else{
            *msec = mCurrentPosition;
        }
        //ulog_info("UPlayer::getCurrentPosition:mCurrentPosition=%d",*msec);
    }
    
}
void UPlayer::setCurrentPosition(double pts) {
#if PLATFORM_DEF != IOS_PLATFORM
    //参数判断
    if (pts > 0) {
        //设置当前播放位置，单位是毫秒
        mCurrentPosition = (int64_t) pts;
    }
#else
    if (pts > 0) {
        //设置当前播放位置，单位是毫秒
        mCurrentPosition = (int64_t) pts;
        
        if (mStreamType & UPLAYER_STREAM_AUDIO) {
            if (0 != mAStartTime)
                mCurrentPosition -= mAStartTime;
        }else{
            if (0 != mVStartTime) {
                mCurrentPosition -= mVStartTime;
            }
        }
        
    }
#endif
}
void UPlayer::release(){
    this->sendMsg(MEDIA_INFO_RELEASE);
}
void UPlayer::_release() {
    
    ulog_info("UPlayer::release enter");
    
    //重置状态
    reset();
    
    ulog_info("UPlayer::release done");
}
void UPlayer::flush() {
    
    ulog_info("UPlayer::flush enter");
    
    //如果播放器已经推出则返回
    if (mStop)
        return;
    
    //清空音频PCM播放队列及其空槽队列
    UQueue::flush(mPCMQueue, mPCMSlotQueue);
    
    //清空视频YUV播放队列及其空槽队列
    UQueue::flush(mYUVQueue, mYUVSlotQueue);
    
    //清空视频数据包解码队列及其空槽队列
    UQueue::flush(mVPacketQueue, mVSlotQueue);
    
    //清空音频数据包解码队列及其空槽队列
    UQueue::flush(mAPacketQueue, mASlotQueue);
    
    /******************通知解码器清空内部缓冲区*****************************/
    /*
     加宏的目的，是为了防止如果播放个的电影里面如果只有音频或则视频的时候防止往队列里面塞
     空包，假设说当前电影里面只有视频，你往音频队列里面塞空包，可是你的音频线程有没有启动，
     这样在影片播放结束的时候，判断是否结束，就会有问题，因为始终有一帧音频一直未处理
     Add by HuangWeiqing
     */
#if PLATFORM_DEF == IOS_PLATFORM
    if (mStreamType & UPLAYER_STREAM_VIDEO) {
#endif
    //获取数据包空槽
    av_link 	pkt = (av_link)mVSlotQueue->get();
    
    if(!pkt){
        ulog_err("UPlayer::flush mVSlotQueue->get() == NULL");
        return;
    }
    //装载flush包
    pkt->item = &flush_pkt;
    pkt->flag = UPLAYER_DECODER_FLUSH_FLAG;
    
    //将数据包放到队列，等待解码线程获取后flush缓冲区
    mVPacketQueue->put(pkt);
#if PLATFORM_DEF == IOS_PLATFORM
    }
#endif
    
    
#if PLATFORM_DEF == IOS_PLATFORM
    if (mStreamType & UPLAYER_STREAM_AUDIO) {
    av_link pkt = (av_link)mASlotQueue->get();
#else
    //获取数据包空槽
    pkt = (av_link)mASlotQueue->get();
#endif
    
    if(!pkt){
        ulog_err("UPlayer::flush mASlotQueue->get() == NULL");
        return;
    }
    //装载flush包
    pkt->item = &flush_pkt;
    pkt->flag = UPLAYER_DECODER_FLUSH_FLAG;
    //将数据包放到队列，等待解码线程获取后flush缓冲区
    mAPacketQueue->put(pkt);
    
#if PLATFORM_DEF == IOS_PLATFORM
    }
#endif
    ulog_info("UPlayer::flush exit");
    
}
void UPlayer::resetSeeking() {
    
    //重置seek状态
    mLastVideoPts = 0;
    //mNeedYUVBufferFull = true;
    mIsSeeking = false;
    
}
void UPlayer::seekTo(int msec) {
    
    if(msec <0)return;
    
    ulog_info("UPlayer::seekTo enter:msec=%d",msec);
    
    
    
    if( mState >= UPLAYER_PREPARED &&  mState <= UPLAYER_PAUSED){
        //设置seek状态
        mIsSeeking = true;
        
        //设置seek到的位置,单位毫秒
#if PLATFORM_DEF != IOS_PLATFORM
        mSeekPosition = msec;
#else
        int64_t startTime = (mStreamType & UPLAYER_STREAM_VIDEO) ? mVStartTime : mAStartTime;
        mSeekPosition = (0 != startTime) ? (startTime + msec) : msec;
#endif
        mCurrentPosition = msec;
    }
    
}
void UPlayer::setListener(UPlayerListener* listener) {
    
    ulog_info("UPlayer::setListener enter");
    //上锁
    lock();
    //设置Listener对象
    mListener = listener;
    //解锁
    unlock();
}

void UPlayer::setEGL(IEGL* egl) {
    
    ulog_info("UPlayer::setEGL enter");
    //上锁
    lock();
    //设置EGL对象
    mEGL = egl;
    //解锁
    unlock();
}

void UPlayer::getVideoHeight(int *height) {
    
    
    //判断播放器是否准备就绪
    if (mState < UPLAYER_PREPARED) {
        *height = 0;
    } else {
        *height = mVideoHeight;
    }
    
}
void UPlayer::getVideoWidth(int *width) {
    
    
    //判断播放器是否准备就绪
    if (mState < UPLAYER_PREPARED) {
        *width = 0;
    } else {
        *width = mVideoWidth;
    }
    
}
status_t UPlayer::resume() {
    
    //TODO:暂时没用到
    return NO_ERROR;
}
status_t UPlayer::suspend() {
    
    //TODO:暂时没用到
    return NO_ERROR;
}
void UPlayer::reset() {
    
    ulog_info("UPlayer::reset enter");
    
    mIsResetting = true;
    
    //等待所有线程退出
    stop();
    
    //上锁
    lock();
    
    //释放文件缓冲区
    if (mUrl) {
        free(mUrl);
        mUrl = NULL;
    }
    if(mLocalCachePath){
        free(mLocalCachePath);
        mLocalCachePath = NULL;
    }
    mState = UPLAYER_IDLE;
    //解锁
    unlock();
}
void UPlayer::notifyCacheMsg(int ok){
    
    if(ok){
        ulog_info("MEDIA_INFO_CACHE_OK");
        
        this->notify(MEDIA_INFO_CACHE_OK);
    }else{
        ulog_info("MEDIA_INFO_CACHE_FAILED");
        this->notify(MEDIA_INFO_CACHE_FAILED);
    }
}
void UPlayer::notify(int msg, int arg1, int arg2) {
    
    //通知界面消息
    if (mListener)
        mListener->notify(msg, arg1, arg2);
}
void UPlayer::notifyMsg(int msg, int arg1, int arg2) {
    
    //消息线程
    this->sendMsg(msg);
}

void UPlayer::init(bool flag) {
    
    //初始化播放器成员变量
    
    if(flag){
        mState = UPLAYER_IDLE;
        mUrl = NULL;
        mLocalCachePath = NULL;
    }
#if DEBUG_ENABLE_H265_DECODER_TEST
	mTotalDecodeTime = 0;
	mDecodeCount = 0;
#endif
    mUCacheFile = NULL;
    
    mThread = 0;
    mErrorCode = ERROR_NO;
    mStop = false;
    mIsSeeking = false;
    mIsResetting = false;
    mNeedBufferring = false;
    
    mStreamType = UPLAYER_STREAM_NONE;
    
    mMediaFile = NULL;
    
    mVSlotQueue = NULL;
    mVPacketQueue = NULL;
    
    mASlotQueue = NULL;
    mAPacketQueue = NULL;
    
    mPCMQueue = NULL;
    mPCMSlotQueue = NULL;
    
    mYUVQueue = NULL;
    mYUVSlotQueue = NULL;
    
    mParser = NULL;
    mDecoderAudio = NULL;
    mRendererAudio = NULL;
    mDecoderVideo = NULL;
    mRendererVideo = NULL;
    
    mConvertCtx = NULL;
    mPixelsPerImage = 0;
    mPixels = NULL;
    mFrame = NULL;
    mDecFrame = NULL;
    
    mAudioFrame = NULL;
    mAudioDecFrame = NULL;
    
    mAudioStreamIndex = -1;
    mVideoStreamIndex = -1;
    mPtsStreamIndex = -1;
    mSeekStreamIndex = -1;
    
    mVideoCodecCtx = NULL;
    mAudioCodecCtx = NULL;
    
    mChannels = 0;
    mSampleRate = 0;
    mSampleFmt = AV_SAMPLE_FMT_S16;
    mSamples = NULL;
    mSamplesSize = 0;
    mLastPCM = NULL;
    
    mVideoWidth = 0;
    mVideoHeight = 0;
    mDuration = -1;
    mCurrentPosition = -1;
    mSeekPosition = -1;
    mLastVideoPts = 0;
    mNeedYUVBufferFull = true;
    
    //yujunfeng
    
    mNewVideoWidth = 0;
    mNewVideoHeight = 0;
    mRealVideoWidth = 0;
    mRealVideoHeight = 0;
    mRestSize = false;
    
    mDecodeTime = 0;
    mFrameRate = 0.0;
    
    mNeedInitSurface = false;
    // added by bruce
    mTimeBase = NULL;
    // added by yujunfeng
    mResampleEngine = NULL;
    
    //判断是否开启log功能
    if (ulog_enable()) {
        //设置ffmpeg的log回调
#if UPLAYER_ENABLE_FFMPEG_LOG == 1
        av_log_set_callback(ulog_callback);
#else
        av_log_set_callback(NULL);
#endif
    } else {
        av_log_set_callback(NULL);
    }
    
#if PLATFORM_DEF == IOS_PLATFORM
    
    mRepeatMode = false;
    mEndPlaybackTime = 0;
    mLastPacketPts = 0;
    mIsFlush = false;
    mFirstAudioPacketDecoded = false;
    mAudioDecodedPts = 0;
    mPreparedDone = 0;
    mFirstVideoFrameDecoded = false;
    mVStartTime = 0;
    mAStartTime = 0;
    mAudioOrVideo = false;
    mEof = false;
    
    mVideoSlotQueueNum = UPLAYER_MAX_VIDEO_PACKET_SLOT_NUM;
    mAudioSlotQueueNum = UPLAYER_MAX_PACKET_SLOT_NUM;
    mMinBufferingQueueNum = UPLAYER_VIDEO_PACKET_BUFFERRING_MIN_NUM;
    mMaxBufferingQueueNum = UPLAYER_VIDEO_PACKET_MAX_BUFFERRING_NUM;
    
#endif
    
    
    //设置默认网路中断，不中断
    //avio_set_interrupt_cb(default_interrupt_cb);
    interrupt_cb_.opaque = (void*) this;
    interrupt_cb_.callback = check_interrupt;
}
void UPlayer::lock() {
    mLock.lock();
}
void UPlayer::unlock() {
    mLock.unlock();
}


bool UPlayer::playOver(int64_t cur_pos)
{
#if PLATFORM_DEF == IOS_PLATFORM
    int64_t startTime = (mStreamType & UPLAYER_STREAM_AUDIO) ? mAStartTime : mVStartTime;
    int64_t duration = (mMediaFile->duration / MS_TIME_BASE);
    int64_t endTime = (mEndPlaybackTime != 0) ? (mEndPlaybackTime <= duration ? mEndPlaybackTime : duration) : duration;
    return (cur_pos + (MS_TIME_BASE >> 2)) >= endTime;
#else
    return (cur_pos + (MS_TIME_BASE >> 2)) >= (mMediaFile->duration / MS_TIME_BASE);
#endif
}

#if PLATFORM_DEF == IOS_PLATFORM
bool UPlayer::playOver2(int64_t cur_pos, bool audioOrVieo){
    int64_t startTime = audioOrVieo ? mVStartTime : mAStartTime;
    int64_t duration = mMediaFile->duration / MS_TIME_BASE;
    int64_t endTime = (mEndPlaybackTime != 0) ? (mEndPlaybackTime > duration ? duration : mEndPlaybackTime) : duration;
    endTime += startTime;
    return ((cur_pos + (MS_TIME_BASE >> 1)) >= endTime || mEof);
}
#endif

bool UPlayer::playOver2(int64_t cur_pos){
    return (cur_pos + (MS_TIME_BASE >> 1)) >= (mMediaFile->duration / MS_TIME_BASE);
}

void UPlayer::playerPreparedTimer(void* ptr){
    
    UPlayer *player = (UPlayer*)ptr;
    
    if(player)player->sendMsg(MEDIA_INFO_PREPARED_CHECK);
}
void UPlayer::networkCheckTimer(void* ptr){
    UPlayer *player = (UPlayer*)ptr;
    
    if(player)player->sendMsg(MEDIA_INFO_NETWORK_CHECK);
}


void* UPlayer::msgThread(void* ptr) {
    
    UPlayer *player = (UPlayer*)ptr;
    
    bool loop = true;
    int msg;
    
    int last_pos = 0;
    int64_t disconnected_time = 0;
    
    int errcode = ERROR_NO;
    
    
    ulog_info("msgThread enter");
#if PLATFORM_DEF == ANDROID_PLATFORM
    player->mCrashHandler.registerTid();
#endif

    while(loop){
        
        msg = player->waitMsg();
        
        
        switch(msg){
            case MEDIA_INFO_EXIT:
                loop = false;
                break;
            case MEDIA_INFO_RELEASE:
                player->_release();
                break;
            case MEDIA_INFO_PREPARED:
                
                player->notify(MEDIA_INFO_PREPARED);
                ulog_info("mPlayerPreparedTimer->stop()");
                player->mPlayerPreparedTimer->stop();
                
                break;
            case MEDIA_INFO_COMPLETED:
                player->notify(MEDIA_INFO_COMPLETED);
                player->mNetWorkCheckTimer->stop();
                ulog_info("media vedio completed & end");
                break;
            case MEDIA_SEEK_COMPLETE:
                ulog_info("media seek completed");
                player->notify(MEDIA_SEEK_COMPLETE);
                break;
            case MEDIA_INFO_BUFFERING_START:
                ulog_info("media bufferring start");
                player->notify(MEDIA_INFO_BUFFERING_START);
                break;
            case MEDIA_INFO_BUFFERING_END:
                ulog_info("media bufferring end");
                player->notify(MEDIA_INFO_BUFFERING_END);
                break;
            case MEDIA_INFO_PLAYERROR:
                errcode = get_player_error_code(player);
                ulog_err("MEDIA_INFO_PLAYERROR error code %d", errcode);
                player->notify(MEDIA_INFO_PLAYERROR, errcode);
                break;
            case MEDIA_INFO_DATA_SOURCE_ERROR:
            {
                errcode = get_player_error_code(player);

                ulog_err("MEDIA_INFO_DATA_SOURCE_ERROR  error code %d", errcode);
                
                if(errcode)player->notify(MEDIA_INFO_DATA_SOURCE_ERROR, errcode);
                
            }
                break;
            case MEDIA_INFO_PREPARE_ERROR:
                errcode = get_player_error_code(player);
                ulog_err("MEDIA_INFO_PREPARE_ERROR error code %d", errcode);
                player->notify(MEDIA_INFO_PREPARE_ERROR, errcode);
                break;
            case MEDIA_INFO_NETWORK_ERROR:
            	errcode = get_player_error_code(player);

                player->notify(MEDIA_INFO_NETWORK_ERROR,errcode);
                break;
            case MEDIA_INFO_SEEK_ERROR:
            	errcode = get_player_error_code(player);
                player->notify(MEDIA_INFO_SEEK_ERROR,errcode);
                break;
            case MEDIA_INFO_NETWORK_DISCONNECTED:
                errcode = get_player_error_code(player);
                player->notify(MEDIA_INFO_NETWORK_DISCONNECTED, errcode);
                ulog_err("MEDIA_INFO_NETWORK_DISCONNECTED  error code %d", errcode);
                break;
            case MEDIA_INFO_NETWORK_CHECK:
            {
                //40秒播放点没有移动，网速过慢超时，暂时通过播放点来判断弱网
                
#if PLATFORM_DEF == IOS_PLATFORM
                if(player->isPlaying() || player->isSeeking() || player->mNeedBufferring){
#else
                if(player->isPlaying() || player->isSeeking()){
#endif
                    int cur_pos = player->getCurrentPosition();
                    
                    if(0 == last_pos){
                        last_pos = cur_pos;
                    }else if(last_pos != cur_pos){
                        last_pos = cur_pos;
                        disconnected_time = 0;
                    }else{
                        disconnected_time += UPLAYER_NETWORK_CHECK_TIME;
                    }
                    if(disconnected_time >= player->mTimeout[UPLAYER_NETWORK_DISCONNECTED_TIMEOUT_TYPE]){
                        //errcode = get_player_error_code(player);
                        player->notify(MEDIA_INFO_NETWORK_DISCONNECTED_CHECK, ERROR_SOFT_PLAYER_NETWORK_DISCONNECTED);
                        player->mNetWorkCheckTimer->stop();
                    }
                }
                break;
            }
            case MEDIA_INFO_PREPARED_CHECK:
            {
                errcode = get_player_error_code(player);
                ulog_err("MEDIA_INFO_PREPARE_TIMEOUT_ERROR  error code %d", errcode);
                player->mPlayerPreparedTimer->stop();
                player->mIsDataSourceError = false;
                player->notify(MEDIA_INFO_PREPARE_TIMEOUT_ERROR, ERROR_SOFT_PLAYER_PREPARE_TIMEOUT);
                break;
            }
            case MEDIA_INFO_VIDEO_START:
                player->notify(MEDIA_INFO_VIDEO_START);
                break;
                
#if PLATFORM_DEF == IOS_PLATFORM
            case MEDIA_INFO_PLAY_TO_END:
                ulog_info("MEDIA_INFO_PLAY_TO_END");
                player->notify(MEDIA_INFO_PLAY_TO_END);
                break;
#endif
            default:
                ulog_err("waitMsg default msg=%d",msg);
                break;
                
        }
        
    }
#if PLATFORM_DEF == ANDROID_PLATFORM
    player->mCrashHandler.unRegisterTid();
#endif

    ulog_info("msgThread exit");
    return NULL;
}
void UPlayer::setTimeout(int type,int timeout){

	ulog_info("UPlayer::setTimeout:type=%d,timeout=%d",type,timeout);

	if(type < UPLAYER_PREPARE_TIMEOUT_TYPE || type >= UPLAYER_PREPARE_TIMEOUT_TYPE_MAX){
		ulog_err("UPlayer::setTimeout: type = %d is out of range",type);
		return;
	}

	lock();

	if( mState <= UPLAYER_INITIALIZED ){
		mTimeout[type] = timeout * 1000000;
	}
	unlock();

}

void UPlayer::enableHEVC(int flag){

	ulog_info("UPlayer::enableHEVC = %d",flag);

	lock();
	ff_hevc_optimize_open(flag);
	unlock();

}


#if PLATFORM_DEF == IOS_PLATFORM
void UPlayer::setVolume(float volume)
{
    ((URendererAudioQueue*)mRendererAudio)->setVolume(volume);
}
float UPlayer::getVolume()
{
    return ((URendererAudioQueue*)mRendererAudio)->getVolume();
}

void UPlayer::setRenderVideo(bool shown){
    if(mRendererVideo){
        ulog_info("UPlayer::setRenderVideo");
        mRendererVideo->setRenderVideo(shown);
    }else{
        ulog_info("UPlayer::setRenderVideo mRendererVideo = NULL");
    }
    
}

#endif
