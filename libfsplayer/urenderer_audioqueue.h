/** @file  urenderer_audioqueue.h
  *	@brief 音频渲染器类，通过AudioQueue包装的音频渲染器
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/


#ifndef UAUDIO_RENDER_AUDIOQUEUE_H_
#define UAUDIO_RENDER_AUDIOQUEUE_H_

#include "ucommon.h"
#include "uqueue.h"
#include "uplayer.h"
#include "urenderer_audio.h"
#include "ulock.h"

#include <AudioToolBox/AudioToolBox.h>

/**
  * @class  URendererAudioQueue urenderer_audioqueue.h
  * @brief  通过AudioQueue实现的音频渲染功能
  * @author YanJia
*/
class URendererAudioQueue:public IRendererAudio,UThread{

	// engine 接口
public:
	/**
	  * @brief  音频渲染器构造函数
	  * @author  YanJia
	  * @param[in]  player 播放器对象指针
	  * @param[in]	channes 音频声道数
	  * @param[in] samplerate 采样率
	*/
	URendererAudioQueue(UPlayer* player,int channels, int samplerate,int samplefmt = AV_SAMPLE_FMT_S16);
	/**
	  * @brief  音频渲染器析构函数
	  * @author  YanJia
	*/
	~URendererAudioQueue();
	/**
	  * @brief  启动播放回调逻辑
	  * @author  YanJia
	  * @return void
	*/
	void start();
	/**
	  * @brief  停止音频播放
	  * @author  YanJia
	  * @return void
	*/
	void stop();
    /**
     * @brief  暂停或被中断后重新打开音频播放
     * @author  weixinghua
     * @return void
     */
	void restart();
    
#if PLATFORM_DEF == IOS_PLATFORM
    /**
     +     * @brief   flush缓存队列里的所有数据
     +     * @author  weixinghua
     +     * @return void
     +     */
    void flush();
    /**
     +     * @brief  设置音频增益, 即音量大小
     +     * @author  weixinghua
     +     * @return void
     +     */
    void setVolume(float gain);
    /**
     +     * @brief  获取当前音频增益, 即音量大小
     +     * @author  weixinghua
     +     * @return 当前增益大小 float
     +     */
    float getVolume();
#endif
    
private:
	/**
	  * @brief  渲染线程
	  * @author  YanJia
	  * @return void
	*/
    void						render();
	/**
	  * @brief  启动线程函数
	  * @author  YanJia
	  * @return void
	*/
    void handleRun();
private:
	/**
	  * @brief  创建AudioQueuePlayer
	  * @author  YanJia
	  * @param[in]	channes 音频声道数
	  * @param[in] samplerate 采样率
	  * @return 成功返回NO_ERROR,失败返回FAILED
	*/
	int createAudioQueuePlayer(int channels, int samplerate);

	/**
	  * @brief  AudioQueue音频播放回调函数
	  * @author  YanJia
	  * @param[in]	inUserData调用者私有数据
	  * @param[in] inAQ AudioQueue实例
	  * @param[in] inBuffer 音频buffer
	  * @return 成功返回NO_ERROR,失败返回FAILED
	*/
	static void AudioQueueCallback(void * inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);
private:
    ULock mLock;
	/**
	  * @brief  AudioQueue音频播放对象
	*/
    AudioQueueRef mAudioQueue;
    

};


#endif /* UAUDIO_RENDER_AUDIOQUEUE_H_ */
