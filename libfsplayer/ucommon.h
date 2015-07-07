/** @file  ucommon.h
  *	@brief 用于包含Uplayer的各类参数定义
  * @author  YanJia
  * @date  2012-5-7
  * @version	1.0
  * @note 播放器参数，调试宏定义，消息定义，类型定义以及常用头文件
*/

#ifndef UCOMMON_H_
#define UCOMMON_H_

#define ANDROID_PLATFORM 1
#define IOS_PLATFORM 2
#define LINUX_PLATFORM 3
#define WINDOWS_PLATFORM 4
#define PLATFORM_DEF IOS_PLATFORM

#if PLATFORM_DEF == ANDROID_PLATFORM
#include <android/Errors.h>
#include "jniUtils.h"
#endif

#if PLATFORM_DEF ==  IOS_PLATFORM
#include <assert.h>
#endif

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "ulog.h"
#include "uthread.h"
#include "umsg_queue.h"
#include "utimer.h"
#include "ulock.h"

#include "uerror_code.h"

#ifndef UINT64_MAX
#define UINT64_MAX	0xffffffffffffffffU
#endif

#ifndef UINT64_C
#define UINT64_C(x)		((x) + (UINT64_MAX - UINT64_MAX))
#endif

#ifndef INT64_MAX
#define INT64_MAX   0x7fffffffffffffff
#endif

#ifndef INT64_C
#define INT64_C(x)      ((x) + (INT64_MAX - INT64_MAX))
#endif

/**
* @brief  是否启用libde265库
*/
#define UPLAYER_ENABLE_LIBDE265 false

/**
* @brief  毫秒base  1000ms = 1s
*/
#define MS_TIME_BASE 1000

/**
* @brief  微秒base  1000000us = 1s
*/
#define US_TIME_BASE 1000000
/**
* @brief  播放器暂停时的轮询时间，目前是0.03秒
*/
#define	UPLAYER_PAUSE_TIME		30000

/**
* @brief  播放器播放点更新时间间隔，200毫秒，单位微秒
*/
#define	UPLAYER_CUR_POS_UPDATE_TIME	400000

/**
* @brief  播放器预加载检查时间间隔，1000毫秒，单位微秒
*/
#define	UPLAYER_PRELOAD_CHECK_TIME	1000000

/**
* @brief  播放器加载检查时间间隔，200毫秒，单位微秒
*/
#define	UPLAYER_LOAD_CHECK_TIME	200000

/**
* @brief  网络情况检测间隔时间，1.5秒，单位微秒
*/
#define UPLAYER_NETWORK_CHECK_TIME      1500000

/**
* @brief  播放器网络错误睡眠时长，0.5秒，单位微秒
*/
#define	UPLAYER_NETWORK_SLEEP_TIME	500000

/**
* @brief  音视频数据包AVPacket的队列长度
*/
#if PLATFORM_DEF != IOS_PLATFORM
    #define UPLAYER_MAX_PACKET_SLOT_NUM	400
#else
    #define UPLAYER_MAX_PACKET_SLOT_NUM 750
#endif

/**
* @brief  视频数据包AVPacket的队列长度
*/
#if PLATFORM_DEF != IOS_PLATFORM
    #define UPLAYER_MAX_VIDEO_PACKET_SLOT_NUM	500
#else
    #define UPLAYER_MAX_VIDEO_PACKET_SLOT_NUM	250
#endif
/**
* @brief  视频数据包AVPacket的Bufferring长度
*/
#define UPLAYER_VIDEO_PACKET_BUFFERRING_NUM	50

#if PLATFORM_DEF == IOS_PLATFORM
    #define IOS_PLAYER_ENABLE_WRITE_PCM_FILE_TO_LOCAL_DEBUG  false
    #define UPLAYER_VIDEO_PACKET_BUFFERRING_MIN_NUM 25
    #define UPLAYER_VIDEO_PACKET_MAX_BUFFERRING_NUM 200
#endif
/**
* @brief  视频丢帧阈值
*/
#define UPLAYER_VIDEO_PACKET_DROPPING_THRESHOLD	(UPLAYER_MAX_VIDEO_PACKET_SLOT_NUM - 2)
/**
* @brief  音频PCM播放队列长度
*/
#if PLATFORM_DEF != IOS_PLATFORM
#define UPLAYER_MAX_PCM_SLOT_NUM	2
#else
#define UPLAYER_MAX_PCM_SLOT_NUM    4
#endif
/**
* @brief  视频YUV播放队列长度，缓冲1分钟的视频数据
*/
#define UPLAYER_MAX_YUV_SLOT_NUM	6
/**
 * @brief  解码器flush内部缓冲区flag
 */
#define UPLAYER_DECODER_FLUSH_FLAG	0x01
/**
 * @brief  过期的视频包，需要丢弃
 */
#define UPLAYER_VPACKET_FLUSH_FLAG	0x02
/**
 * @brief  最后一个过期的视频包，需要清空解码器缓冲区, by weixinghua
 */
#define UPLAYER_VPACKET_FLUSH_LAST  0x04
/**
 * @brief  重绘最后一帧yuv数据
 */
#define UPLAYER_REDRAW_LAST_YUV  0x08
/**
 * @brief  标记该packet为非关键帧的普通视频包, by weixinghua
 */
#define UPLAYER_VPACKET_NORMAL_FLAG  0
/**
 * @brief  标记该packet包含的帧为关键帧
 */
#define UPLAYER_PKT_KEY_FRAME_FLAG	0x10
/**
 * @brief	同步逻辑间隔,单位微秒
 */
#define	UPLAYER_SYNCHRONIZE_INTEVAL	5000
/**
* @brief  同步时间阈值,单位毫秒
*/
//视频同步范围,同步差1帧
#define	UPLAYER_SYNCHRONIZE_THRESHOLD	53
//视频同步差2帧
#define	UPLAYER_SYNCHRONIZE_THRESHOLD_LOW	106
//视频同步差5帧
#define	UPLAYER_SYNCHRONIZE_THRESHOLD_HIGH	265
//视频同步差8帧
#define	UPLAYER_SYNCHRONIZE_THRESHOLD_MAX	424

/**
* @brief  视频同步最大时间间隔，150毫秒，单位微秒
*/
#define	UPLAYER_SYNCHRONIZE_MAX_INTERVAL	150000
/**
* @brief  跳帧
*/
typedef enum{
    UPLAYER_SKIP_NONE       	= 0,			/**不跳帧*/
    UPLAYER_SKIP_LEVEL1  		,				/**跳过filter*/
    UPLAYER_SKIP_LEVEL2 		,				/**跳过非参考帧idct阶段 */
    UPLAYER_SKIP_LEVEL3		 	,				/**跳过非双向帧idct阶段 */
    UPLAYER_SKIP_LEVEL4  		,				/**跳过所有帧的idct阶段 */
    UPLAYER_SKIP_LEVEL5	 		,				/**跳过非参考帧*/
    UPLAYER_SKIP_MAX	   		,

}UPLAYER_SKIP_FRAME;

/**
* @brief  丢帧时间阈值,单位毫秒
*/
#define	UPLAYER_SKIPFRAME_THRESHOLD_LOW		500
#define	UPLAYER_SKIPFRAME_THRESHOLD_MID1	600
#define	UPLAYER_SKIPFRAME_THRESHOLD_MID2	800
#define	UPLAYER_SKIPFRAME_THRESHOLD_MID3	1000
#define	UPLAYER_SKIPFRAME_THRESHOLD_HIGH1	1200
#define	UPLAYER_SKIPFRAME_THRESHOLD_HIGH2	1500

/**
* @brief 最大视频渲染时间，单位微秒
*/
#define	UPLAYER_MAX_VIDEO_RENDER_TIME		5000
/**
* @brief 同步时间,单位微秒
*/
#define	UPLAYER_SYNCHRONIZE_TIME	800
/**
* @brief  上层界面传递的最大文件名长度,设置为最大url长度的两倍
*/
#define UPLAYER_MAX_STRING_LEN		1024

#define UPLAYER_MAX_AUDIO_BUFFER_LEN	(192000)


/**
* @brief  是否启用H265 decoder性能测试
*/
#define DEBUG_ENABLE_H265_DECODER_TEST	false

/**
* @brief  是否启用视频流
*/
#define DEBUG_ENABLE_VIDEO_STREAM	true
/**
* @brief  是否启用音频流
*/
#define DEBUG_ENABLE_AUDIO_STREAM	true
/**
* @brief  是否启用prepare
*/
#define DEBUG_ENABLE_PREPARE	true
/**
* @brief  是否启用setDataSource
*/
#define DEBUG_ENABLE_SET_DATA_SOURCE	true
/**
* @brief  使用EGL方法获取surfaceview的宽高
*/
#if PLATFORM_DEF == IOS_PLATFORM
    #define EGL_GET_WIDTH_HEIGHT  true
#else
    #define EGL_GET_WIDTH_HEIGHT false
#endif

/**
* @brief  uplayer播放器统一声道数
*/
#define UPLAYER_AUDIO_CHANNELS 2

/**
 * moved from jniUtils.h
 * update by weixinghua
 */
#if PLATFORM_DEF != ANDROID_PLATFORM
typedef enum{
	FAILED = -1,
	OK		= 0,
	NO_ERROR = 0,
}status_t;
#endif

/**
 * @brief  用于标识UPlayer通知上层界面的消息代码
 */
typedef enum{
    // 0xx
    MEDIA_INFO_UNKNOWN = 1,

    MEDIA_SEEK_COMPLETE = 4,
    // 7xx
    // The video is too complex for the decoder: it can't decode frames fast
    // enough. Possibly only the audio plays fine at this stage.
    MEDIA_INFO_VIDEO_TRACK_LAGGING = 700,

    MEDIA_INFO_BUFFERING_START = 701,
    MEDIA_INFO_BUFFERING_END = 702,

    MEDIA_INFO_CACHE_OK = 751,
    MEDIA_INFO_CACHE_FAILED = 752,
    // 8xx
    // Bad interleaving means that a media has been improperly interleaved or not
    // interleaved at all, e.g has all the video samples first then all the audio
    // ones. Video is playing but a lot of disk seek may be happening.
    MEDIA_INFO_BAD_INTERLEAVING = 800,
    // The media is not seekable (e.g live stream).
    MEDIA_INFO_NOT_SEEKABLE = 801,
    // New media metadata is available.
    MEDIA_INFO_METADATA_UPDATE = 802,

    MEDIA_INFO_FRAMERATE_VIDEO = 900,
    MEDIA_INFO_FRAMERATE_AUDIO,

    //update by madexiang
    //播放器已经初始化就绪
    MEDIA_INFO_PREPARED =1000,
    //播放器播放视频完成
    MEDIA_INFO_COMPLETED =1001,
    //播放器播放错误
    MEDIA_INFO_PLAYERROR =1002,
    //update by madexiang

    // add by bruce
    MEDIA_INFO_START_LOADING = 1003,
    MEDIA_INFO_END_LOADING = 1004,
    MEDIA_INFO_SET_VIDEO_SIZE = 1030,
    MEDIA_INFO_BUFFERING_UPDATE = 1031,

    //add by yanjia
    MEDIA_INFO_DATA_SOURCE_ERROR =1006,
    MEDIA_INFO_PREPARE_ERROR =1007,
    MEDIA_INFO_NETWORK_ERROR =1008,
    MEDIA_INFO_SEEK_ERROR = 1009,
    MEDIA_INFO_PREPARE_TIMEOUT_ERROR = 1010,

    MEDIA_INFO_NETWORK_DISCONNECTED = 1101,
    MEDIA_INFO_NETWORK_DISCONNECTED_CHECK = 1102,

    //add by yanjia
    //向上层通知的播放起止点通知
    MEDIA_INFO_AD_START = 1011,
    MEDIA_INFO_AD_END = 1012,
    MEDIA_INFO_VIDEO_START = 1017,
    MEDIA_INFO_VIDEO_END = 1018,
    MEDIA_INFO_VIDEO_ALL_END = 1019,

    //by qingpengchen
    //广告倒计时通知
    MEDIA_INFO_AD_COUNT_DOWN = 1040,

    //add by yanjia，用于播放器内部的消息

    MEDIA_INFO_PREPAR_ASYNC = 1041,
    MEDIA_INFO_RELEASE = 1042,
    //播放点通知
    MEDIA_INFO_CURRENT_POSITION_UPDATE = 2000,
    //预加载检查通知
    MEDIA_INFO_PRELOAD_CHECK = 2001,

    //视频加载情况通知
    MEDIA_INFO_LOAD_CHECK = 2002,

    //prepared 超时通知
    MEDIA_INFO_PREPARED_CHECK = 2003,

    //网速差通知
    MEDIA_INFO_NETWORK_CHECK = 2004,

    //prepared AD超时通知
    MEDIA_INFO_PREPARED_AD_CHECK = 2005,

    //网速通知
    MEDIA_INFO_NETWORK_SPEED_UPDATE = 2006,
	
	//P2P缓冲
    MEDIA_INFO_P2P_CACHE_RATE = 2007,

    // ios ad load check, by weixinghua
#if PLATFORM_DEF == IOS_PLATFORM
    MEDIA_INFO_AD_LOAD_CHECK = 2100,
    MEDIA_INFO_AD_START_LOADING = 2101,
    MEDIA_INFO_AD_END_LOADING = 2102,
#endif

    //前贴广告消息间隔
    MEDIA_INFO_PRE_AD = 5000,

    //后贴广告消息间隔
    MEDIA_INFO_POST_AD = 8000,

    //中贴广告消息间隔
    MEDIA_INFO_MID_AD = 10000,

    MEDIA_INFO_EXIT = 20000,
    
    
#if PLATFORM_DEF == IOS_PLATFORM
    //posted when movie plays to the end
    MEDIA_INFO_PLAY_TO_END = 999111,
#endif
    
}MEDIA_INFO_TYPE;

/**
 * @brief  前贴广告消息宏
 */
#define UPLAYER_PRE_AD_MSG(x) (x + MEDIA_INFO_PRE_AD)
/**
 * @brief  后贴广告消息宏
 */
#define UPLAYER_POST_AD_MSG(x) (x + MEDIA_INFO_POST_AD)
/**
 * @brief  中贴广告消息宏
 */
#define UPLAYER_MID_AD_MSG(x) (x + MEDIA_INFO_MID_AD)
/**
 * @brief  需要底层播放器处理的视频广告类型定义
 */
typedef enum{
    UPLAYER_AD_NONE = -1,
    UPLAYER_AD_VIDEO = 0,		/**普通视频类型*/
    UPLAYER_AD_PRE,				/**前贴广告视频类型*/
    UPLAYER_AD_POST,				/**后贴广告视频类型*/
    UPLAYER_AD_MID,				/**中贴广告视频类型*/
}UPLAYER_AD_TYPE;

/**
 * @brief  最大联播广告数
 */
#define	UPLAYER_MAX_SEQ_AD_NUM		10


/**
 * @brief	流类型
 */
#define	UPLAYER_STREAM_NONE		0x0
#define	UPLAYER_STREAM_AUDIO		0x01
#define	UPLAYER_STREAM_VIDEO		0x02
#define	UPLAYER_STREAM_AUDIO_VIDEO	(UPLAYER_STREAM_AUDIO|UPLAYER_STREAM_VIDEO)

/**
* @brief  标识播放器状态
*/
typedef enum{
    UPLAYER_STATE_ERROR        = 0,				/**出错状态*/
    UPLAYER_IDLE               = 1 << 0,		/**初始状态*/
    UPLAYER_INITIALIZED        = 1 << 1,		/**已初始化状态*/
    UPLAYER_PREPARING          = 1 << 2,		/**准备中*/
    UPLAYER_PREPARED           = 1 << 3,		/**已就绪*/
    UPLAYER_DECODED            = 1 << 4,		/**播放完成，目前没有用到*/
    UPLAYER_STARTED            = 1 << 5,		/**播放状态*/
    UPLAYER_PAUSED             = 1 << 6,		/**暂停状态*/
    UPLAYER_STOPPED            = 1 << 7,		/**停止状态*/
    UPLAYER_PLAYBACK_COMPLETE  = 1 << 8			/**播放完成状态*/
}UPLAYER_STATE;


/**
* @brief  数据包节点，链表数据结构
*/
typedef struct node{
	void*	item;				//数据项
	void*	data_buf;			//内部储存区，采用首次分配策略时用到
	int		size;				//数据项的大小
	int		data_size;			//内部储存区大小
	int		flag;					//特殊包标志
	double	pts;					//视频时间戳
	struct node* next;			//下一个数据项的首地址
}*av_link;



//播放器prepare超时时间，10秒，单位微秒 风行超时比较长，暂设为60秒
//int UPLAYER_PREPARE_CHECK_TIME = 10000000;
#define UPLAYER_PREPARE_CHECK_TIME	60000000
//播放器网络错误重试时长，60秒，单位微秒,风行超时比较长，暂设为60秒
#define UPLAYER_NETWORK_TRY_TIME 60000000
//load 超时20s 单位微秒,风行超时比较长，暂设为60秒
#define UPLAYER_LOAD_TIME 60000000

//播放器网络缓冲超时时间，0.3秒，单位微秒
#define UPLAYER_BUFFERRING_CHECK_TIME 300000
//缓冲时间5s 单位秒
#define UPLAYER_BUFFER_TIME	5

typedef enum {

	UPLAYER_PREPARE_TIMEOUT_TYPE = 0,
	UPLAYER_NETWORK_DISCONNECTED_TIMEOUT_TYPE,
	UPLAYER_PREPARE_TIMEOUT_TYPE_MAX,

}UPLAYER_TIMEOUT_TYPE;

#endif /* UCOMMON_H_ */

