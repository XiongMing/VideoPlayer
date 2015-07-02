/** @file  urenderer_video.h
  *	@brief 视频渲染器类，通过OpenGL ES包装的视频渲染器
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/


#ifndef UVIDEO_RENDERER_H_
#define UVIDEO_RENDERER_H_


#include "ucommon.h"
#include "uplayer.h"

#if PLATFORM_DEF != LINUX_PLATFORM
	#include "ugraphics.h"
#endif

#if PLATFORM_DEF == IOS_PLATFORM
#include "ulock.h"
#endif

/**
  * @class  URendererVideo urenderer_video.h
  * @brief  实现视频渲染线程
  * @author YanJia
*/
class URendererVideo : public UThread{

public:
	/**
	  * @brief  视频渲染线程类构造函数
	  * @author  YanJia
	  * @param[in]  player 播放器对象指针
	*/
	URendererVideo(UPlayer* player);
	/**
	  * @brief  视频渲染线程类析构函数
	*/
	~URendererVideo();
	/**
	  * @brief  停止视频播放
	  * @author  YanJia
	  * @return void
	*/
	void						stop();
private:
	/**
	  * @brief  渲染线程
	  * @author  YanJia
	  * @return void
	*/
    void						render();
	/**
	  * @brief  绘图操作
	  * @author  YanJia
	  * @param[in]  pkt YUV数据包
	  * @return void
	*/
    void						process(av_link pkt);
	/**
	  * @brief  音视频同步
	  * @author  YanJia
	  * @param[in]  pkt 下一个播放的视频包
	  * @return 需要seek视频，true；否则，false
	*/
    bool					synchronize(av_link pkt);
	/**
	  * @brief  获取视频显示间隔时间
	  * @author  YanJia
	  * @param[in]  diff 音视频差值
	  * @return 返回间隔时间
	*/
    int					getRestTime(int diff =0);
	/**
	  * @brief  更新播放进度
	  * @author  YanJia
	*/
    void				skipFrame(UPLAYER_SKIP_FRAME level = UPLAYER_SKIP_NONE);

	/**
	  * @brief  备份最后一帧数据
	  * @author  YanJia
	*/
    void				backupLastFrame(av_link frame);
	/**
	  * @brief  刷新EGL
	  * @author  YanJia
	*/
    void				refreshEGL();
	/**
	  * @brief  显示最后一帧
	  * @author  YanJia
	  * @return void
	*/
    void				showLastFrame();

private:
	/**
	  * @brief  启动线程函数
	  * @author  YanJia
	  * @return void
	*/
    void           				handleRun();

protected:
	/**
	  * @brief  播放器对象指针
	*/
    UPlayer*					mPlayer;
	/**
	  * @brief  绘图对象指针
	*/

	#if PLATFORM_DEF != LINUX_PLATFORM
    	IGraphics*				mGraphics;
    #endif
	/**
	  * @brief  视频帧间隔，单位微秒
	*/
    int							mFrameInterval;

    av_link						mLastFrame;
    
#if PLATFORM_DEF == IOS_PLATFORM
public:
    void setRenderVideo(bool shown);
    
private:
    bool mRenderVideo;
    ULock *mLockRenderVideo;
#endif
};


#endif /* UVIDEO_RENDERER_H_ */
