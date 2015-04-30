#ifndef __UERROR_CODE_H_
#define __UERROR_CODE_H_

typedef enum{

		//播放正常
		ERROR_NO = 0,

		/*下面是ffmpeg软解播放器相关错误，范围从ERROR_SOFT_BEGIN到ERROR_SOFT_PLAYER_MAX*/

		ERROR_SOFT_PLAYER_BEGIN = -11000,
		//软解播放器未知错误
		ERROR_SOFT_PLAYER_UNKNOWN = -11001,

		/*下面是数据源相关错误*/

		//dns解析超时
		ERROR_SOFT_PLAYER_DNS_TIMEOUT = -11002,
		//dns解析出错
		ERROR_SOFT_PLAYER_DNS_FAILED = -11003,
		//HTTP 3XX系列错误,或与p2p有关
		ERROR_SOFT_PLAYER_HTTP_3XX = -11004,
		//HTTP 4XX系列错误,或与p2p有关
		ERROR_SOFT_PLAYER_HTTP_4XX = -11005,
		//HTTP prepare超时,或与p2p有关
		ERROR_SOFT_PLAYER_PREPARE_TIMEOUT = -11006,
		//url地址出错，协议不支持，文件格式不支持或损坏,或与p2p有关
		ERROR_SOFT_PLAYER_UNSUPPORTED = -11007,
		//播放地址为空
		ERROR_SOFT_PLAYER_ADDRESS_NULL = -11008,
		//ffmpeg seek出错
		ERROR_SOFT_PLAYER_SEEK_FAILED = -11009,
		//播放器准备错误(总类错误)
		ERROR_SOFT_PLAYER_PREPARE_ERROR = -11010,
		//播放器准备错误(总类错误)
		ERROR_SOFT_PLAYER_SOCKET_CONNECT_TIMEOUT = -11011,


		/*下面是网络超时相关错误*/

		//正常播放状态40秒没有从网络读到足够数据
		ERROR_SOFT_PLAYER_NETWORK_DISCONNECTED = -11100,
		//ffmpeg av_read_frame 读取数据包长时间出错
		ERROR_SOFT_PLAYER_AV_READ_FRAME_FAILED = -11101,

		/*下面是Android适配性错误*/

		//egl初始化失败
		ERROR_SOFT_PLAYER_INITI_EGL_FAILED = -11200,
		//加载libfsplayer库失败
		ERROR_SOFT_PLAYER_LOAD_LIBFSPLAYER_FAILED = -11201,

		/*下面是内存分配失败错误*/

		//内存分配失败
		ERROR_SOFT_PLAYER_NO_MEMORY = -11300,

		/*下面是ffmpeg调用相关错误*/

		//ffmpeg相关错误汇总
		ERROR_SOFT_PLAYER_FFMPEG = -11400,

		//openslel相关错误汇总
		ERROR_SOFT_PLAYER_OPENSLES = -11500,

		//opengles相关错误汇总
		ERROR_SOFT_PLAYER_OPENGLES = -11600,

		//软解播放器调用状态错误
		ERROR_SOFT_PLAYER_BAD_INVOKE = -11700,


		ERROR_SOFT_PLAYER_END = -11999,

}PLAYER_ERROR_CODE;
extern void (*pfun_set_player_error_code)(void *,int);
#endif
