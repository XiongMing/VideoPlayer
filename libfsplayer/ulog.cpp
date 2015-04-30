#ifndef ULOG_CPP_
#define ULOG_CPP_

#include "ulog.h"

extern "C"{
#include "libavutil/log.h"
}


int ulog_enable(){
	//是否启用C/C++语言层的log功能
	return UPLAYER_ENABLE_LOG;
}


void ulog_callback(void* ptr, int level, const char* fmt, va_list vl){

//#if PLATFORM_DEF == ANDROID_PLATFORM
    char line[1024];
    AVClass* avc= ptr ? *(AVClass**)ptr : NULL;
    line[0]=0;

    //获得log的模块信息
    if(avc) {
           if (avc->parent_log_context_offset) {
               AVClass** parent= *(AVClass***)(((uint8_t*)ptr) + avc->parent_log_context_offset);
               if(parent && *parent){
                   snprintf(line, sizeof(line), "[%s @ %p] ", (*parent)->item_name(parent), parent);
               }
           }
           //构造模块信息
           snprintf(line + strlen(line), sizeof(line) - strlen(line), "[%s @ %p] ", avc->item_name(ptr), ptr);
    }
    //构造log内容
    vsnprintf(line + strlen(line), sizeof(line) - strlen(line), fmt, vl);

#if PLATFORM_DEF == ANDROID_PLATFORM
    //log的级别
    switch(level){
    case AV_LOG_PANIC:
    	level = ANDROID_LOG_FATAL;
    	break;
    case AV_LOG_FATAL:
    	level = ANDROID_LOG_FATAL;
    	break;
    case AV_LOG_ERROR:
    	level = ANDROID_LOG_ERROR;
    	break;
    case AV_LOG_WARNING:
    	level = ANDROID_LOG_WARN;
    	break;
    case AV_LOG_INFO:
    	level = ANDROID_LOG_INFO;
    	break;
    case AV_LOG_VERBOSE:
    	level = ANDROID_LOG_VERBOSE;
    	break;
    case AV_LOG_DEBUG:
    	level = ANDROID_LOG_DEBUG;
    	break;
    case AV_LOG_QUIET:
    	level = ANDROID_LOG_SILENT;
    	break;
    default:
    	level = ANDROID_LOG_DEFAULT;
    	break;
    }
    //调用logcat功能
    avlog(level,line);
#else

    fprintf(stderr, line);
//    fprintf(stderr, fmt, vl);
#endif

}


#endif /* ULOG_CPP_ */
