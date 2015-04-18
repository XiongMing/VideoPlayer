/** @file  uplayer.h
  *	@brief UPlayer播放器的实现
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/

#ifndef UPLAYER_H_
#define UPLAYER_H_

#include "ucommon.h"

extern "C" {

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavformat/url.h"
}

#if PLATFORM_DEF == ANDROID_PLATFORM
#include "ucrash_handler.h"
#endif

class UParser;
class UDecoder;
class UDecoderAudio;
class IRendererAudio;
class UDecoderVideo;
class URendererVideo;
class UQueue;
class UFilter;
class FSPlayer;
class IEGL;
class UCacheFile;
class UTimer;


/**
 * @class  UPlayerListener uplayer.h
 * @brief  用于向界面发送消息的Listener
 * @author YanJia
 */
class UPlayerListener {
public:
	/**
	  * @brief  消息通知函数
	  * @author  YanJia
	  * @param[in]  msg 消息类型
	  * @return void
	*/
    virtual void notify(int msg, int ext1=0, int ext2=0) = 0;
};
/**
  * @class  UPlayer uplayer.h
  * @brief  播放器类，实现播放器逻辑
  * @author YanJia
*/
class UPlayer{
public:
	friend class UParser;
    friend class UDecoder;
	friend class UDecoderAudio;
	friend class UDecoderVideo;
	friend class URendererAudioOpenSLES;
#if PLATFORM_DEF == ANDROID_PLATFORM
	friend class URendererAudioTrack;
#elif PLATFORM_DEF == IOS_PLATFORM
    friend class URendererAudioQueue;
#else
	friend class URendererAudioLinux;
#endif
	friend class URendererVideo;
	friend class UGraphics;
	friend class UCacheFile;
    friend class UTimer;
	friend void set_player_error_code(void *obj,int err);
	friend int get_player_error_code(void *obj);
public:
	/**
	  * @brief  播放器构造函数
	  * @author  YanJia
	*/
	UPlayer(const char* dumpPath = NULL);
	/**
	  * @brief  播放器析构函数
	  * @author  YanJia
	*/
	~UPlayer();
	/**
	  * @brief  设置超时时间
	  * @author  YanJia
	  * @param[in]  type	超时类型
	  * @param[in]  timeout	超时时间,秒为单位
	  * @return void
	*/
	void setTimeout(int type,int timeout);
	void enableHEVC(int flag);
public:
	/**
	  * @brief  设置数据源（播放文件地址）
	  * @author  YanJia
	  * @param[in]  url	播放文件地址
	  * @return 成功返回 NO_ERROR,失败返回FAILED
	*/
	//status_t setDataSource(const char *url);
	/**
	  * @brief  设置数据源（播放文件地址）
	  * @author  YanJia
	  * @param[in]  url	播放文件地址
	  * @param[in]  localCachePath	缓存的文件地址
	  * @return 成功返回 NO_ERROR,失败返回FAILED
	*/
	status_t setDataSource(const char *url,const char *localCachePath = NULL);

	/**
	  * @brief  播放器准备资源
	  * @author  YanJia
	  * @return 成功返回 NO_ERROR,失败返回FAILED
	*/
	status_t prepareAsync();
	/**
	  * @brief  启动播放器
	  * @author  YanJia
	  * @return last state
	*/
	UPLAYER_STATE start();
	/**
	  * @brief  暂停播放器
	  * @author  YanJia
	  * @return void
	*/
	void pause();
	/**
	  * @brief  释放播放器资源，与prepare相对应
	  * @author  YanJia
	  * @return void
	*/
	void release();
	/**
	 * @brief  是否正在播放
	 * @author  YanJia
	 * @return void
	 */
	bool isPlaying() {
			return UPLAYER_STARTED == mState;
	}
	/**
	  * @brief  seek到某个时间点
	  * @author  YanJia
	  * @param[in] msec	seek到的播放点，单位毫秒
	  * @return void
	*/
	void seekTo(int msec);
	/**
	  * @brief  获得当前播放进度
	  * @author  YanJia
	  * @param[out] msec 输出参数，当前的播放时间点，单位毫秒
	  * @return void
	*/
	void getCurrentPosition(int *msec);
	/**
	  * @brief  获得当前播放文件的总长度
	  * @author  YanJia
	  * @param[out] msec 输出参数，当前播放文件的总时长，单位毫秒
	  * @return void
	*/
	void getDuration(int *msec);
	/**
	  * @brief  获得当前播放视频的高度
	  * @author  YanJia
	  * @param[out] height 输出参数，当前播放视频高，单位像素
	  * @return void
	*/
	void getVideoHeight(int *height);
	/**
	  * @brief  获得当前播放视频的宽度
	  * @author  YanJia
	  * @param[out] width 输出参数，当前播放视频宽，单位像素
	  * @return void
	*/
	void getVideoWidth(int *width);
	/**
	  * @brief  设置上层界面的Listener消息通知对象
	  * @author  YanJia
	  * @param[in] listener 消息通知对象
	  * @return void
	*/
    void setListener(UPlayerListener* listener);

	/**
	 * @brief  判断是否正处于seek状态
	 * @author  YanJia
	 * @return seek没有结束时返回true，否则返回false
	 */
	bool isSeeking() {
		return mIsSeeking;
	}

	/**
	  * @brief  设置EGL对象
	  * @author  YanJia
	  * @param[in] egl EGL对象
	  * @return void
	*/
    void setEGL(IEGL* egl);

	/**
	 * @brief  设置新的视频宽高
	 * @author  yujunfeng
	 * @param[in]  width, height
	 * @return void
	 */
	void changeVideoSize(const int width, const int height);
	/**
	 * @brief  设置视频显示surface
	 * @author  YanJia
	 * @param[in] env JEI环境对象
	 * @param[in] jsurface 界面上传递过来的surface对象
	 * @return void
	 */
#if PLATFORM_DEF == ANDROID_PLATFORM
	void setVideoSurface(JNIEnv* env, jobject jsurface);
#else
	void setVideoSurface();
#endif
public:
	/**
	  * @brief  继续恢复运行，目前没有用到
	  * @author  YanJia
	  * @return NO_ERROR
	*/
    status_t resume();
	/**
	  * @brief  暂停运行，目前没有用到
	  * @author  YanJia
	  * @return NO_ERROR
	*/
    status_t suspend();
	/**
	  * @brief  重置播放器
	  * @author  YanJia
	*/
    void reset();
	/**
	  * @brief  准备播放器资源,目前prepare的实现与prepareAsync语义一致，避免界面anr
	  * @author  YanJia
	  * @return 成功返回NO_ERROR，失败返回FAILED
	*/
	status_t prepare();
	/**
	  * @brief  退出播放器
	  * @author  YanJia
	  * @return void
	*/
	void stop();
public:
    /**
     * @brief 注册codec
     * @author songxl
     */
    static void registerAVcodec();
    
#if PLATFORM_DEF == IOS_PLATFORM
    void setVolume(float volume);
    float getVolume();
    int getMediaTypes() const { return mStreamType; }
    void setRepeatMode(bool repeatMode) { mRepeatMode = repeatMode; }
    bool isRepeatMode() const { return mRepeatMode; }
    
    /*
     *unit: ms
     */
    void setEndPlaybackTime(int64_t endPlayBackTime) { mEndPlaybackTime = endPlayBackTime;}
    void setRenderVideo(bool shown);
#endif
private:
	/**
	  * @brief  准备播放器资源
	  * @author  YanJia
	  * @return 成功返回NO_ERROR，失败返回FAILED
	*/
	status_t _prepare();
	/**
	  * @brief  释放播放器资源，与prepare相对应
	  * @author  YanJia
	  * @return void
	*/
	void _release();
	/**
	 * @brief  获得当前播放进度
	 * @author  YanJia
	 * @return 以毫秒为单位的当前播放时间点
	 */
	int64_t getCurrentPosition() {
		return mCurrentPosition;
	}
private:
	/**
	  * @brief  准备音频播放资源
	  * @author  YanJia
	  * @return 成功返回NO_ERROR，失败返回FAILED
	*/
	status_t prepareAudio();
	/**
	  * @brief  准备视频播放资源
	  * @author  YanJia
	  * @return 成功返回NO_ERROR，失败返回FAILED
	*/
	status_t prepareVideo();
	/**
	  * @brief  初始化成员变量
	  * @author  YanJia
	  * @param[in] flag,true 为UPLAYER_IDLE 初始化,false 为UPLAYER_INITIALIZED初始化
	  * @return void
	*/
	void init(bool flag = true);
	/**
	  * @brief  设置播放进度
	  * @author  YanJia
	  * @param[in] pts 当前解码时间戳，目前采用音频的显示时间戳
	  * @return void
	*/
	void setCurrentPosition(double pts);
	/**
	  * @brief  重置seek状态
	  * @author  YanJia
	  * @return void
	*/
	void resetSeeking();
	/**
	  * @brief  异步prepare线程函数
	  * @author  YanJia
	  * @param[in] player 播放器对象指针
	  * @return void* 回传状态
	*/
	static void* prepareThread(void* player);

	/**
	  * @brief   判断是否播放完成
	  * @author  eric
	  * @return  true is over
	*/
	bool playOver(int64_t cur_pos);
	/**
	  * @brief   判断是否接近播放完成
	  * @author  eric
	  * @return  true is over
	*/
	bool playOver2(int64_t cur_pos);
private:
	/**
	  * @brief   获取本地缓存路径
	  * @author  YanJia
	  * @return
	*/
	const char* getLocalCachePath(){
		return mLocalCachePath;
	}
	/**
	  * @brief  通知上层界面缓存消息
	  * @param[in] ok :1 成功,0 失败
	  * @author  YanJia
	  * @return void
	*/
	void notifyCacheMsg(int ok);
private:
	/**
	  * @brief  通知上层界面消息
	  * @author  YanJia
	  * @param[in] msg 消息编号
	  * @return void
	*/
    virtual void notify(int msg, int arg1 = 0, int arg2 = 0);
	/**
	  * @brief  通知内部消息
	  * @author  YanJia
	  * @param[in] msg 消息编号
	  * @return void
	*/
    void notifyMsg(int msg, int arg1 = 0, int arg2 = 0);

private:
	/**
	  * @brief  清空解码队列和播放队列
	  * @author  YanJia
	  * @return void
	*/
	void 	flush();

private:
	/**
	 * @brief  播放器是否退出
	 * @author  YanJia
	 * @return 退出返回true，否则返回false
	 */
	bool isStop() const {
		return this->mStop;
	}
	/**
	 * @brief  播放器是否暂停
	 * @author  YanJia
	 * @return 暂停返回true，否则返回false
	 */
	bool isPause() const {
		return UPLAYER_PAUSED == mState;
	}
	/**
	 * @brief  播放器是否准备就绪
	 * @author  YanJia
	 * @return 暂停返回true，否则返回false
	 */
	bool isPrepared() const {
		return UPLAYER_PREPARED == mState;
	}
	/**
	 * @brief  播放器是否重置
	 * @author  YanJia
	 * @return 重置返回true，否则返回false
	 */
	bool isResetting() const {
		return mIsResetting;
	}
private:
	/**
	 * @brief  默认网络中断回调，不中断
	 * @author  YanJia
	 * @return 暂停0
	 */
	static int default_interrupt_cb(void) {
		return 0;
	}
	/**
	 * @brief  网络中断回调，中断
	 * @author  YanJia
	 * @return 暂停1
	 */
	static int interrupt_cb(void) {
		return 1;
	}

private:
	/**
	  * @brief  播放器上锁
	  * @author  YanJia
	  * @return void
	*/
    void lock();
	/**
	  * @brief  播放器解锁
	  * @author  YanJia
	  * @return void
	*/
    void unlock();

private:
	/**
	  * @brief  给播放器消息循环线程发送消息
	  * @author  YanJia
	  * @param[in] msg 消息值
	  * @return void
	*/
	void sendMsg(int msg){if(mMsgQueue)mMsgQueue->send(msg);}
	/**
	  * @brief  阻塞等待消息
	  * @author  YanJia
	  * @return int 消息值,-1 错误
	*/
	int waitMsg(){if(mMsgQueue)return mMsgQueue->recv();return -1;}
private:
	/**
	  * @brief  播放器消息循环线程
	  * @author  YanJia
	  * @param[in] player 播放器对象指针
	  * @return void* 回传状态
	*/
	static void* msgThread(void* player);

	/**
	  * @brief  播放点更新Timer
	  * @author  YanJia
	  * @param[in] player 播放器对象指针
	  * @return void
	*/
	static void curPosUpdateTimer(void* player);

	/**
	  * @brief  Prepare Timer
	  * @author  qingpengchen
	  * @param[in] player 播放器对象指针
	  * @return void
	*/
	static void playerPreparedTimer(void* player);


	/**
	  * @brief  network check Timer
	  * @author  qingpengchen
	  * @param[in] player 播放器对象指针
	  * @return void
	*/
	static void networkCheckTimer(void* player);
#if PLATFORM_DEF == ANDROID_PLATFORM
private:
    /**
     * @brief  播放器Prepare是否发生数据源错误
     */
    UCrashHandler		mCrashHandler;
public:
    /**
     * @brief  设置崩溃日志捕获路径
     * @author  YanJia
     * @param[in]  path	本地存放dump日志的路径
     * @return void
     */
    void setDumpPath(const char* path){ mCrashHandler.setDumpPath(path); }
#endif
private:
	/**
	  * @brief  prepare线程ID
	*/
    pthread_t           mThread;
private:
	/**
	  * @brief  以秒计时的超时数据
	*/
	int     mTimeout[UPLAYER_PREPARE_TIMEOUT_TYPE_MAX];
private:
	/**
	  * @brief  播放器锁对象
	*/
	ULock     mLock;//锁
	/**
	  * @brief  播放器消息通知对象
	*/
    UPlayerListener*	mListener;

	/**
	  * @brief  媒体文件
	*/
	AVFormatContext*	mMediaFile;

	IEGL*				mEGL;

	/**
	  * @brief  音频数据包空槽队列
	*/
	UQueue*				mASlotQueue;
	/**
	  * @brief  音频数据包队列
	*/
    UQueue*				mAPacketQueue;
	/**
	  * @brief  视频数据包空槽队列
	*/
    UQueue*				mVSlotQueue;
	/**
	  * @brief  视频数据包队列
	*/
    UQueue*				mVPacketQueue;
	/**
	  * @brief  音频PCM播放队列
	*/
    UQueue*				mPCMQueue;
	/**
	  * @brief  音频PCM空槽队列
	*/
    UQueue*				mPCMSlotQueue;
	/**
	  * @brief  视频YUV播放队列
	*/
    UQueue*				mYUVQueue;
	/**
	  * @brief  播放器YUV空槽队列
	*/
    UQueue*				mYUVSlotQueue;
	/**
	  * @brief  拆包线程对象
	*/
    UParser*			mParser;
	/**
	  * @brief  音频解码线程对象
	*/
    UDecoderAudio*		mDecoderAudio;
	/**
	  * @brief  音频渲染线程对象
	*/
    IRendererAudio*		mRendererAudio;
	/**
	  * @brief  视频解码线程对象
	*/
    UDecoderVideo*		mDecoderVideo;
	/**
	  * @brief  视频渲染线程对象
	*/
    URendererVideo*		mRendererVideo;
	/**
	  * @brief  视频颜色空间转换和缩放对象	*/
    struct SwsContext*	mConvertCtx;
	/**
	  * @brief  颜色空间转换之后的视频buffer size
	*/
    int					mPixelsPerImage;
	/**
	  * @brief  颜色空间转换之后的视频buffer
	*/
    uint8_t*			mPixels;
	/**
	  * @brief  颜色空间转换之后的视频帧的外壳
	*/
	AVFrame*			mFrame;
	/**
	  * @brief  解码之后的视频帧外壳
	*/
	AVFrame*			mDecFrame;
	/**
	  * @brief  重采样之后的音频帧外壳
	*/
	AVFrame*			mAudioFrame;
	/**
	  * @brief  解码之后的音频帧外壳
	*/
	AVFrame*			mAudioDecFrame;
	/**
	  * @brief  音频流索引
	*/
    int					mAudioStreamIndex;
	/**
	  * @brief  视频流索引
	*/
    int					mVideoStreamIndex;
	/**
	  * @brief  用于更新播放时间的流索引
	*/
    int					mPtsStreamIndex;
	/**
	  * @brief  用于seek的流索引
	*/
    int					mSeekStreamIndex;
	/**
	  * @brief  音频解码器指针
	*/
    AVCodecContext*		mAudioCodecCtx;
	/**
	  * @brief  视频解码器指针
	*/
    AVCodecContext*		mVideoCodecCtx;

	/**
	  * @brief  音频声道数
	*/
    int					mChannels;
	/**
	  * @brief  音频采用率
	*/
    int					mSampleRate;
	/**
	  * @brief  采样精度
	*/
    int					mSampleFmt;
	/**
	  * @brief  解码后音频PCM数据
	*/
    uint8_t*			mSamples;
	/**
	  * @brief  分配的音频PCM缓冲区大小
	*/
    int                 mSamplesSize;
	/**
	  * @brief  音频渲染器回调函数中保存的最后一个没有被释放的PCM包
	*/
	av_link				mLastPCM;
	/**
	  * @brief  视频宽度，单位像素
	*/
    int					mVideoWidth;
	/**
	  * @brief  视频高度，单位像素
	*/
    int					mVideoHeight;
	/**
	  * @brief  视频时长，单位毫秒
	*/
    int64_t				mDuration;
	/**
	  * @brief  当前播放时间点，单位毫秒
	*/
    int64_t				mCurrentPosition;
	/**
	  * @brief  当前播放时间点，单位毫秒
	*/
    int64_t				mSeekPosition;
	/**
	  * @brief  上一帧视频的pts
	*/
    int64_t				mLastVideoPts;
	/**
	  * @brief  当前是否需要缓冲视频数据
	*/
    bool				mNeedYUVBufferFull;
	/**
	 * @brief  视频文件地址
	 */
	char* mUrl;
	/**
	 * @brief  本地缓存视频文件路径
	 */
	char* mLocalCachePath;
	/**
	 * @brief  要缓存的视频文件
	 */
	UCacheFile* mUCacheFile;
	/**
	 * @brief 时间戳的缩放比例
	 */
	AVRational *mTimeBase;

	/**
	 * @brief 视频帧解码时间
	 */
	int64_t mDecodeTime;
	/**
	 * @brief 视频帧率
	 */
	float mFrameRate;

private:
	/**
	  * @brief  处理内部播放器消息的线程对象
	*/
	pthread_t				mMsgThread;
	/**
	  * @brief  消息队列
	*/
	UMsgQueue*		mMsgQueue;

	/**
		  * @brief  播放器Prepare加载Timer
	*/
	UTimer*				mPlayerPreparedTimer;
	/**
		  * @brief  网络情况检测Timer
	*/
	UTimer*				mNetWorkCheckTimer;
	/**
		  * @brief  播放器Prepare是否发生数据源错误
	*/
	bool						mIsDataSourceError;
	/**
	 *    * @brief 是否http文件
	 */
	bool is_http_file_;
private:
	/**
	  * @brief  是否需要resetting
	*/
    bool				mIsResetting;
	/**
	  * @brief  播放器退出标志
	*/
    bool				mStop;
	/**
	  * @brief  播放器seek标志
	*/
    bool				mIsSeeking;
	/**
	  * @brief  播放器状态
	*/
    UPLAYER_STATE		mState;
	/**
	 * @brief  视频流类型
	 */
	int mStreamType;
	/**
	 * @brief  音频重采样对象
	 */
	struct SwrContext* mResampleEngine; //yujunfeng
	/**
	 * @brief  旋转后视频的宽
	 */
	int mNewVideoWidth; //yujunfeng
	/**
	 * @brief  旋转后视频的高
	 */
	int mNewVideoHeight; //yujunfeng
		/**
	 * @brief  解码数据后的真实视频高
	 */
	 int mRealVideoWidth;
	 	/**
	 * @brief  解码数据后的真实视频宽
	 */
	 int mRealVideoHeight;
	/**
	 * @brief  是否重新设置视频宽和高
	 */
	bool mRestSize;
	/**
	 * @brief  是否需要重新初始化surface
	 */
	bool mNeedInitSurface;
	/**
	 * @brief  是否需要Bufferring
	 */
	bool mNeedBufferring;
	AVIOInterruptCB interrupt_cb_;
	char interrupt_var_;
private:
	int mErrorCode;
private:
#if DEBUG_ENABLE_H265_DECODER_TEST
	int64_t	mTotalDecodeTime;
	int	mDecodeCount;
#endif
private:
	//flush包，用于flush解码器缓冲区
	static AVPacket flush_pkt;
	static int check_interrupt(void *obj)
	{
		//ulog_err("UPlayer::check_interrupt is %d", (obj ? ((UPlayer*) obj)->interrupt_var_: 1));
		return obj ? ((UPlayer*) obj)->interrupt_var_: 1;
	}
public:
	AVStream* stream_tmp;
        
        
#if PLATFORM_DEF == IOS_PLATFORM
    private:
        bool mRepeatMode;
    int64_t mEndPlaybackTime;
    int64_t mLastPacketPts;
    bool    mIsFlush;
    double mAudioDecodedPts;
    
    //添加读写锁，用于控制快进的时候，不能对音频解码和视频渲染进行操作，否则再做音视频同步的时候，可能出现问题
    pthread_rwlock_t mRWLock;
    
    bool    mFirstAudioPacketDecoded;
    
    /*
     0: 准备好消息未发送
     1: seek操作
     2: 数据准备好，可播状态
     */
    int    mPreparedDone;
    
    /*
     *用于标示影片开始播放，和seek操作后是否解码第一帧数据
     */
    bool    mIsFirstVideoFramePrepared;
    
#endif
};
void set_player_error_code(void *obj,int err);
#endif /* UPLAYER_H_ */
