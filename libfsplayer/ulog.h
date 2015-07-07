/** @file  ulog.h
  *	@brief 用于log功能
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/

#ifndef ULOG_H_
#define ULOG_H_
#include "ucommon.h"

#if PLATFORM_DEF == ANDROID_PLATFORM
#include <jni.h>
#include <android/log.h>
#elif PLATFORM_DEF == IOS_PLATFORM
#include "ulog_client.h"
#else
#include <stdarg.h>
#endif
#include <stdio.h>

/**
  * @brief  是否启用log功能
*/
#define UPLAYER_ENABLE_LOG		0
#define UPLAYER_DLNA_ENABLE_LOG		0
#define UPLAYER_ENABLE_FFMPEG_LOG		0
/*
	for more log level（NDK内置的log级别）

    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT,
*/
/**
  * @brief  一般信息
*/
#define ULOG_INFO	ANDROID_LOG_INFO
/**
  * @brief  错误信息
*/
#define ULOG_ERR	ANDROID_LOG_ERROR

/**
  * @brief  UPlayer播放器标签
*/
#define UPLAYER	"uplayer"
#define UPLAYER_ERR	"uplayer_err"
/**
  * @brief  ffmpeg标签
*/
#define AVLOG	"av_log"
/**
  * @brief  YanJia的调试过程中的标签
*/
#define YANJIA	"yanjia"
/**
 * @brief	DLNA的调试标签
 */
#define DLNA	"dlna"

/**
  * @brief  是否启用log功能
  * @author  YanJia
*/
int ulog_enable();
/**
  * @brief  ffmpeg的log回调函数
  * @author  YanJia
  * @param[in]  ptr log标签
  * @param[in]  level log级别
  * @param[in]  fmt 格式
  * @param[in]  vl	参数
  * @return	void
*/
void ulog_callback(void* ptr, int level, const char* fmt, va_list vl);

/**
  * @brief  logcat功能
  * @author  YanJia
  * @param[in]  log_level log级别
  * @param[in]  tag log标签
*/


#if PLATFORM_DEF == ANDROID_PLATFORM
#define logcat(log_level, tag, ...)   \
if(UPLAYER_ENABLE_LOG)		\
__android_log_print(log_level, tag, __VA_ARGS__)

#define avlog(log_level, ...)	\
logcat(log_level,AVLOG,__VA_ARGS__)

#define ulog(log_level, ...)	\
logcat(log_level,UPLAYER,__VA_ARGS__)

#define ulog_err(...)	\
logcat(ULOG_ERR,UPLAYER_ERR,__VA_ARGS__)

#define ulog_info(...)	\
ulog(ULOG_INFO,__VA_ARGS__)

//#elif PLATFORM_DEF == IOS_PLATFORM
//
//#define logcat(log_level, tag, fmt, ...)   \
//ios_log_print(tag, fmt, ##__VA_ARGS__)
//
//#define avlog(log_level, fmt, ...)	\
//logcat(log_level,AVLOG,fmt,##__VA_ARGS__)
//
//#define ulog(log_level, fmt, ...)	\
//logcat(log_level,UPLAYER,fmt,##__VA_ARGS__)
//
//#define ulog_err(fmt, ...)	\
//logcat(ULOG_ERR,UPLAYER_ERR,fmt,##__VA_ARGS__)
//
//#define ulog_info(fmt, ...)	\
//ulog(ULOG_INFO,fmt,##__VA_ARGS__)

#elif PLATFORM_DEF == LINUX_PLATFORM
#include <stdio.h>
#define logcat(log_level, tag, fmt, ...) printf(#fmt"\n", ##__VA_ARGS__)
#define avlog(log_level, fmt, ...) printf(#fmt"\n", ##__VA_ARGS__)
#define ulog(log_level, fmt, ...) printf(#fmt"\n", ##__VA_ARGS__)
#define ulog_err(fmt, ...) printf(#fmt"\n", ##__VA_ARGS__)
#define ulog_info(fmt, ...) printf(#fmt"\n", ##__VA_ARGS__)
#else
#define logcat(log_level, tag, fmt, ...) if(UPLAYER_ENABLE_LOG) printf(#fmt"\n", ##__VA_ARGS__)
#define avlog(log_level, fmt, ...) if(UPLAYER_ENABLE_LOG) printf(#fmt"\n", ##__VA_ARGS__)
#define ulog(log_level, fmt, ...) if(UPLAYER_ENABLE_LOG) printf(#fmt"\n", ##__VA_ARGS__)
#define ulog_err(fmt, ...) if(UPLAYER_ENABLE_LOG) printf(#fmt"\n", ##__VA_ARGS__)
#define ulog_info(fmt, ...) if(UPLAYER_ENABLE_LOG) printf(#fmt"\n", ##__VA_ARGS__)
#endif

#define PRINT_LINE
#define AV_LOG_EX(level, ...)
/**
  * @brief  YanJia专用的log
  * @author  YanJia
  * @param[in]  log_level log级别
*/
#define ylog(log_level, ...)	\
		logcat(log_level,YANJIA,__VA_ARGS__)

/**
  * @brief  YanJia专用的错误log
  * @author  YanJia
*/
#define ylog_err(...)	\
		ylog(ULOG_ERR,__VA_ARGS__)
/**
  * @brief  YanJia专用的信息log
  * @author  YanJia
*/
#define ylog_info(...)	\
		ylog(ULOG_INFO,__VA_ARGS__)

/**
 * @brief	DLNA专用的log
 * @author	Baowang
 * @param[in]	log_level  log级别
 */
#define DLNAlog(log_level, ...)		\
		if(UPLAYER_DLNA_ENABLE_LOG)		\
		__android_log_print(log_level, DLNA, __VA_ARGS__)

/**
 * @brief	DLNA专用的错误log
 * @author	Baowang
 */
#define DLNAlog_err(...)	\
		DLNAlog(ULOG_ERR, __VA_ARGS__)

/**
 * @brief	DLNA专用的信息log
 * @author	Baowang
 */
#define DLNAlog_info(...)	\
		DLNAlog(ULOG_INFO, __VA_ARGS__)

#define empty_log(log_info, ...)	\

#endif /* ULOG_H_ */
