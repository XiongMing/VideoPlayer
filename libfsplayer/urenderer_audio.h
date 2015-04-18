/** @file  urenderer_audio.h
  *	@brief 音频渲染器接口
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/


#ifndef UAUDIO_RENDER_H_
#define UAUDIO_RENDER_H_

#include "ucommon.h"
#include "uqueue.h"
#include "uplayer.h"

/**
  * @class  IRendererAudio urenderer_audio.h
  * @brief  音频渲染接口
  * @author YanJia
*/
class IRendererAudio{

public:
	/**
	  * @brief  音频渲染器构造函数
	  * @author  YanJia
	  * @param[in]  player 播放器对象指针
	  * @param[in]	channes 音频声道数
	  * @param[in] samplerate 采样率
	*/
	IRendererAudio(UPlayer* player,int channels, int samplerate,int samplefmt = AV_SAMPLE_FMT_S16){
		mChannels = channels;
		mSampleRate = samplerate;
		mSampleFmt = samplefmt;
	}
	/**
	  * @brief  音频渲染器析构函数
	  * @author  YanJia
	*/

	virtual ~IRendererAudio(){};
	/**
	  * @brief  是否准备就绪
	  * @author  YanJia
	  * @return 准备就绪返回true，否则返回false
	*/
	bool prepared(){return mPrepared;}
	/**
	  * @brief  启动播放回调逻辑
	  * @author  YanJia
	  * @return void
	*/
	virtual void start() = 0;
	/**
	  * @brief  停止音频播放
	  * @author  YanJia
	  * @return void
	*/
	virtual void stop() = 0;


protected:

	/**
	  * @brief  是否准备就绪
	*/
	bool		mPrepared;
	/**
	  * @brief  播放器对象指针
	*/
	UPlayer*	mPlayer;


	/**
	  * @brief  采样率
	*/
	int	mSampleRate;

	/**
	  * @brief  声道数
	*/
	int	mChannels;

	/**
	  * @brief  采样精度
	*/
	int	mSampleFmt;

};


#endif /* UAUDIO_RENDER_H_ */
