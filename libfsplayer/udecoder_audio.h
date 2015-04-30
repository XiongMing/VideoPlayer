/** @file  udecoder_audio.h
  *	@brief 用于实现音频解码线程
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/
#ifndef UDECODER_AUDIO_H_
#define UDECODER_AUDIO_H_

#include "udecoder.h"
#include "uplayer.h"

extern "C" {

#include "libavcodec/avcodec.h"

}
/**
  * @class  UDecoderAudio udecoder_audio.h
  * @brief  音频解码线程类
  * @author YanJia
  * @note
  * 继承自UDecoder类，提供更新播放进度的功能
*/
class UDecoderAudio:public UDecoder{

public:
	/**
	  * @brief  音频解码线程对象构造函数
	  * @author  YanJia
	  * @param[in]  name 线程名
	  * @param[in]  player 播放器对象指针
	*/
	UDecoderAudio(char* name,UPlayer* player):UDecoder(name,player){}
	/**
	  * @brief  音频解码线程对象析构函数
	  * @author  YanJia
	*/
	~UDecoderAudio(){}
	/**
	  * @brief  停止音频线程
	  * @author  YanJia
	*/
	void stop();
	/**
	  * @brief  更新播放进度
	  * @author  YanJia
	*/
    void				updateCurrentPosition(av_link pkt);
private:


	/**
	  * @brief  解码线程函数
	  * @author  YanJia
	*/
    void				decode();
	/**
	  * @brief  解码函数
	  * @author  YanJia
	*/
    void				process(av_link pkt);

private:
	/**
	  * @brief  初始化音频重采样context
	  * @author  YanJia
	*/
    void				init_swr(AVFrame* frame);

};

#endif /* UDECODER_AUDIO_H_ */
