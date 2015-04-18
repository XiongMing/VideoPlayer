/** @file  urenderer_audiotrack.h
  *	@brief 音频渲染器类，通过AudioTrack包装的音频渲染器,支持Android 2.0以上平台
  * @author  YanJia
  * @date  2012-6-25
  * @version	1.0
  * @note
*/


#ifndef URENDERER_AUDIO_LINUX_H_
#define URENDERER_AUDIO_LINUX_H_

#include "ucommon.h"
#include "uqueue.h"
#include "uplayer.h"
#include "urenderer_audio.h"




/**
  * @class  URendererAudioTrack urenderer_audiotrack.h
  * @brief  通过AudioTrack现的音频渲染功能
  * @author YanJia
*/
class URendererAudioLinux:public IRendererAudio,UThread{
private:

public:
	/**
	  * @brief  音频渲染器构造函数
	  * @author  YanJia
	  * @param[in]  player 播放器对象指针
	  * @param[in]	channes 音频声道数
	  * @param[in] samplerate 采样率
	*/
	URendererAudioLinux(UPlayer* player,int channels, int samplerate,int samplefmt = AV_SAMPLE_FMT_S16);
	/**
	  * @brief  音频渲染器析构函数
	  * @author  YanJia
	*/
	~URendererAudioLinux();

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
	/**
	  * @brief  初始化
	  * @author  YanJia
	  * @return void
	*/
    void init();
};


#endif /* URENDER_AUDIO_LINUX_H_ */
