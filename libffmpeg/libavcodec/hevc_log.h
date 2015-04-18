#ifndef AVCODEC_HEVC_LOG_H
#define AVCODEC_HEVC_LOG_H


#define ANDROID_PLATFORM 1
#define IOS_PLATFORM 2
#define LINUX_PLATFORM 3
#define WINDOWS_PLATFORM 4
#define PLATFORM_DEF IOS_PLATFORM

#if PLATFORM_DEF == ANDROID_PLATFORM
  #include <android/log.h>
  #include <time.h>
  //#define HEVC_LOG
#elif PLATFORM_DEF == LINUX_PLATFORM
  #include <stdio.h>
#endif


/*
#include <android/log.h>
#include <time.h>
#define HEVC_LOG
*/

#ifdef HEVC_LOG
  #define TIME_STRUCT
  #ifdef TIME_STRUCT
    //calculating in the time unit of us
    //display in the time unit of us  
	#define CLK_DEF(ENTRY) 					  clock_t t0_##ENTRY, t1_##ENTRY; \
												  static  int total_##ENTRY = 0; \
												  static int current_frame_##ENTRY = 0 ;


    #define CLK_T0(ENTRY)  t0_##ENTRY = clock();
                           
						 
  
    #define CLK_T1(ENTRY)  t1_##ENTRY = clock(); \
	                          total_##ENTRY += t1_##ENTRY - t0_##ENTRY; \
						      if (current_frame_##ENTRY != get_totalframe()) \
						      { \
						       current_frame_##ENTRY = get_totalframe(); \
						       __android_log_print(ANDROID_LOG_INFO, #ENTRY, "%d %d %d ", total_##ENTRY/current_frame_##ENTRY, current_frame_##ENTRY, (t1_##ENTRY - t0_##ENTRY)); \
						      }						   

    #define CLKS_DEF(ENTRY) struct timespec res_##ENTRY; \
	                       double t0_##ENTRY, t1_##ENTRY;
  
    #define CLKS_T0(ENTRY)  clock_gettime(CLOCK_REALTIME, &res_##ENTRY); \
                           t0_##ENTRY = 1e9*res_##ENTRY.tv_sec + (double)res_##ENTRY.tv_nsec;
						 
  
    #define CLKS_T1(ENTRY)  clock_gettime(CLOCK_REALTIME, &res_##ENTRY); \
                            t1_##ENTRY = 1e9*res_##ENTRY.tv_sec + (double)res_##ENTRY.tv_nsec; \
						    __android_log_print(ANDROID_LOG_INFO, #ENTRY, "%f ", (t1_##ENTRY - t0_##ENTRY)/1000);  						   


  #else
    #define CLK_DEF(ENTRY) clock_t t0_##ENTRY, t1_##ENTRY; \
	                       static int total_##ENTRY = 0; \
	                       static int current_frame_##ENTRY = 0 ; 

    #define CLK_T0(ENTRY)  t0_##ENTRY = clock(); \
		                   

    #define CLK_T1(ENTRY)  t1_##ENTRY = clock(); \
	                       total_##ENTRY += t1_##ENTRY - t0_##ENTRY; \
	                       if (current_frame_##ENTRY != get_totalframe()) \
	                       { \
	                         current_frame_##ENTRY = get_totalframe(); \
                             __android_log_print(ANDROID_LOG_INFO, #ENTRY, "%d %d %d ", total_##ENTRY/current_frame_##ENTRY, current_frame_##ENTRY, t1_##ENTRY - t0_##ENTRY); \
	                       } 
	                   
  #endif
#else
  #define CLK_DEF(ENTRY) 
  #define CLK_T0(ENTRY)  
  #define CLK_T1(ENTRY)  
#endif

#ifdef HEVC_LOG
#define LOGI0(...) __android_log_print(ANDROID_LOG_INFO, "LOGI0", __VA_ARGS__);
#else
#define LOGI0(...) 
#endif

#ifdef HEVC_LOG
#define LOGI1(...) __android_log_print(ANDROID_LOG_INFO, "LOGI1", __VA_ARGS__);
#else
#define LOGI1(...) 
#endif

#ifdef HEVC_LOG
#define LOGI_ERR(...) __android_log_print(ANDROID_LOG_INFO, "LOGI_ERR", __VA_ARGS__);
#else
#define LOGI_ERR(...) 
#endif

#ifdef HEVC_LOG
#define LOGI_OK(...) __android_log_print(ANDROID_LOG_INFO, "LOGI_OK", __VA_ARGS__);
#else
#define LOGI_OK(...) 
#endif

#ifdef HEVC_LOG
#define LOGI_DBG(...) __android_log_print(ANDROID_LOG_INFO, "LOGI_DBG", __VA_ARGS__);
#else
#define LOGI_DBG(...) 
#endif

#ifdef HEVC_LOG

#define  PRINT_s16(VEC) LOGI_DBG("%d %d %d %d %d %d %d %d ",  vgetq_lane_s16( VEC, 0),vgetq_lane_s16( VEC, 1),\
vgetq_lane_s16( VEC, 2),vgetq_lane_s16( VEC, 3),\
vgetq_lane_s16( VEC, 4),vgetq_lane_s16( VEC, 5), \
vgetq_lane_s16( VEC, 6),vgetq_lane_s16( VEC, 7));
#else
#define  PRINT_s16(VEC) 
#endif

#ifdef HEVC_LOG
#define LOGI_TIMER(ENTRY) 
#else
#define LOGI_TIMER(ENTRY) 
#endif



#if PLATFORM_DEF == ANDROID_PLATFORM
 #define LOGI_point(ENTRY,...) __android_log_print(ANDROID_LOG_INFO, #ENTRY, __VA_ARGS__);
#elif  PLATFORM_DEF == LINUX_PLATFORM
 #define LOGI_point(ENTRY,fmt, ...) printf(#fmt"\n", ##__VA_ARGS__);
#else
 #define LOGI_point(ENTRY,fmt, ...)  
#endif

//#define LOGI_point(ENTRY,...) __android_log_print(ANDROID_LOG_INFO, #ENTRY, __VA_ARGS__);

void  totalframe_add(void);
int   get_totalframe(void);
void  clear_totalframe(void);


#endif

