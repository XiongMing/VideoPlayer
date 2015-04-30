/** @file  urenderer_opensl_es.h
  *	@brief 音频渲染器类，通过OpenSL ES包装的音频渲染器
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/


#ifndef UAUDIO_RENDER_OPENSL_ES_H_
#define UAUDIO_RENDER_OPENSL_ES_H_

#include "ucommon.h"
#include "uqueue.h"
#include "uplayer.h"
#include "urenderer_audio.h"
#include "ucrash_handler.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

/**
  * @class  URendererAudioOpenSLES urenderer_audio.h
  * @brief  通过OpenSL ES实现的音频渲染功能
  * @author YanJia
*/
class URendererAudioOpenSLES:public IRendererAudio,UThread{

	// engine 接口
public:
	/**
	  * @brief  音频渲染器构造函数
	  * @author  YanJia
	  * @param[in]  player 播放器对象指针
	  * @param[in]	channes 音频声道数
	  * @param[in] samplerate 采样率
	*/
	URendererAudioOpenSLES(UPlayer* player,int channels, int samplerate,int samplefmt = AV_SAMPLE_FMT_S16);
	/**
	  * @brief  音频渲染器析构函数
	  * @author  YanJia
	*/
	~URendererAudioOpenSLES();
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
	  * @brief  创建音频播放引擎
	  * @author  YanJia
	  * @return 成功返回NO_ERROR,失败返回FAILED
	*/
	int createEngine();
	/**
	  * @brief  创建BufferQueueAudioPlayer
	  * @author  YanJia
	  * @param[in]	channes 音频声道数
	  * @param[in] samplerate 采样率
	  * @return 成功返回NO_ERROR,失败返回FAILED
	*/
	int createBufferQueueAudioPlayer(int channels, int samplerate);
private:
	/**
	  * @brief  音频播放回调函数
	  * @author  YanJia
	  * @param[in]	bq BufferQueue对象接口
	  * @param[in] context 回调所需的数据，PCM数据
	  * @return void
	*/
	static void AudioPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

	/**
	  * @brief  启动线程函数
	  * @author  YanJia
	  * @return void
	*/
    void handleRun();
private:

    /**
	  * @brief  音频播放引擎对象
	*/
	SLObjectItf mEngineObject;
	/**
	  * @brief  音频播放引擎接口
	*/
	SLEngineItf mEngine;

	/**
	  * @brief  音频输出混合器对象
	*/
	SLObjectItf mOutputMixObject;

	/**
	  * @brief  音频播放对象
	*/
	SLObjectItf mPlayerObject;
	/**
	  * @brief  音频播放接口
	*/
	SLPlayItf mPlayerPlay;
	/**
	  * @brief  BufferQueue对象接口
	*/
	SLAndroidSimpleBufferQueueItf mPlayerBufferQueue;

	/**
	  * @brief  音频线程id
	*/
	UThreadID	mTids[2];

};


#endif /* UAUDIO_RENDER_H_ */
