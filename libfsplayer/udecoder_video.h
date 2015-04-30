/** @file  udecoder_video.h
  *	@brief 用于实现视频解码线程
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/
#ifndef UDECODER_VIDEO_H_
#define UDECODER_VIDEO_H_

#include "udecoder.h"
#include "udecoder_audio.h"
#include "uplayer.h"

 #if PLATFORM_DEF != LINUX_PLATFORM
	#include "ugraphics.h"
 #endif
  
extern "C" {

#include "libavcodec/avcodec.h"

}

/**
  * @class  UDecoderVideo udecoder_video.h
  * @brief  视频解码线程类
  * @author YanJia
  * @note
  * 继承自UDecoder类，提供音视频同步功能
*/
class UDecoderVideo:public UDecoder{

public:
	/**
	  * @brief  视频解码线程对象构造函数
	  * @author  YanJia
	  * @param[in]  name 线程名
	  * @param[in]  player 播放器对象指针
	*/
	UDecoderVideo(char* name,UPlayer* player):UDecoder(name,player){}
	/**
	  * @brief  视频解码线程对象析构函数
	  * @author  YanJia
	*/
	~UDecoderVideo(){}
	/**
	  * @brief  停止视频线程
	  * @author  YanJia
	*/
	void				stop();
private:

	/**
	  * @brief  解码线程函数
	  * @author  YanJia
	*/
    void				decode();
	/**
	  * @brief  解码函数，颜色空间转换，缩放
	  * @author  YanJia
	*/
    void				process(av_link pkt);

	/**
	  * @brief  计算数据包的时间戳
	  * @author  YanJia
	 * @return 返回正确的时间戳
	*/
    double			getPacketPts(AVFrame* frame);
public:
		bool 				mNeedInitSws;
};

#endif /* UDECODER_VIDEO_H_ */
