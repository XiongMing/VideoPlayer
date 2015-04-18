/** @file  urenderer_audiotrack.h
  *	@brief 音频渲染器类，通过AudioTrack包装的音频渲染器,支持Android 2.0以上平台
  * @author  YanJia
  * @date  2012-6-25
  * @version	1.0
  * @note
*/


#ifndef UAUDIO_RENDER_AUDIOTRACK_H_
#define UAUDIO_RENDER_AUDIOTRACK_H_

#include "ucommon.h"
#include "uqueue.h"
#include "uplayer.h"
#include "urenderer_audio.h"

#include <dlfcn.h>



/**
  * @class  URendererAudioTrack urenderer_audiotrack.h
  * @brief  通过AudioTrack现的音频渲染功能
  * @author YanJia
*/
class URendererAudioTrack:public IRendererAudio,UThread{
private:
	// _ZN7android11AudioSystem19getOutputFrameCountEPii
	typedef int (*AudioSystem_getOutputFrameCount)(int *, int);
	// _ZN7android11AudioSystem16getOutputLatencyEPji
	typedef int (*AudioSystem_getOutputLatency)(unsigned int *, int);
	// _ZN7android11AudioSystem21getOutputSamplingRateEPii
	typedef int (*AudioSystem_getOutputSamplingRate)(int *, int);

	// _ZN7android10AudioTrack16getMinFrameCountEPiij
	typedef int (*AudioTrack_getMinFrameCount)(int *, int, unsigned int);

	// _ZN7android10AudioTrackC1EijiiijPFviPvS1_ES1_ii
	typedef void (*AudioTrack_ctor)(void *, int, unsigned int, int, int, int, unsigned int, void (*)(int, void *, void *), void *, int, int);
	// _ZN7android10AudioTrackC1EijiiijPFviPvS1_ES1_i
	typedef void (*AudioTrack_ctor_legacy)(void *, int, unsigned int, int, int, int, unsigned int, void (*)(int, void *, void *), void *, int);
	// _ZN7android10AudioTrackD1Ev
	typedef void (*AudioTrack_dtor)(void *);
	// _ZNK7android10AudioTrack9initCheckEv
	typedef int (*AudioTrack_initCheck)(void *);
	// _ZN7android10AudioTrack5startEv
	typedef int (*AudioTrack_start)(void *);
	// _ZN7android10AudioTrack4stopEv
	typedef int (*AudioTrack_stop)(void *);
	// _ZN7android10AudioTrack5writeEPKvj
	typedef int (*AudioTrack_write)(void *, void  const*, unsigned int);
	// _ZN7android10AudioTrack5flushEv
	typedef int (*AudioTrack_flush)(void *);
public:
	/**
	  * @brief  音频渲染器构造函数
	  * @author  YanJia
	  * @param[in]  player 播放器对象指针
	  * @param[in]	channes 音频声道数
	  * @param[in] samplerate 采样率
	*/
	URendererAudioTrack(UPlayer* player,int channels, int samplerate,int samplefmt = AV_SAMPLE_FMT_S16);
	/**
	  * @brief  音频渲染器析构函数
	  * @author  YanJia
	*/
	~URendererAudioTrack();

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
private:

	void*	mLibMedia;
	void*	mAudioTrack;

	AudioSystem_getOutputFrameCount mFpGetOutputFrameCount;
	AudioSystem_getOutputLatency mFpGetOutputLatency;
	AudioSystem_getOutputSamplingRate mFpGetOutputSamplingRate;
	AudioTrack_getMinFrameCount mFpGetMinFrameCount;
	AudioTrack_ctor mFpCtor;
	AudioTrack_ctor_legacy mFpCtorLegacy;
	AudioTrack_dtor mFpDtor;
	AudioTrack_initCheck mFpInitCheck;
	AudioTrack_start mFpStart;
	AudioTrack_stop mFpStop;
	AudioTrack_write mFpWrite;
	AudioTrack_flush mFpFlush;
};


#endif /* UAUDIO_RENDER_H_ */
