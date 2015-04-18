#include "ucommon.h"
#include "uplayer.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/time.h"
}

pthread_mutex_t     Lock = PTHREAD_MUTEX_INITIALIZER;//锁
utimer_link UTimer::timer_head = NULL;
pthread_t UTimer::thread;
bool UTimer::timer_thread_started = false;

void UTimer::lock(){
    if(pthread_mutex_lock(&Lock)){
    	ulog_err("UTimer::lock failed");
    }
}
void UTimer::unlock(){
    if(pthread_mutex_unlock(&Lock)){
    	ulog_err("UTimer::unlock failed");
    }
}

UTimer::UTimer(){

	mFNTimer = NULL;
	mData = NULL;
	mInterval = 0;
	mIsRunning = false;
	mLastEntryTime = 0;
}

UTimer::UTimer(FNTimer timer,void* data,unsigned int interval){
	setTimer(timer,data,interval);
}

void UTimer::setTimer(FNTimer timer,void* data,unsigned int interval){

	mFNTimer = timer;
	mData = data;
	if(interval  > UTIMER_MIN_PRECISION)	mInterval = interval;
	else mFNTimer = NULL;
}
UTimer::~UTimer(){

	utimer_link pre,cur;

	//add by qingpengchen
	/*
	if (!cur)
		return;
	*/
	//ulog_info("UTimer::~UTimer 1");
	lock();
	//ulog_info("UTimer::~UTimer 1.1");

	cur = timer_head;
	pre = NULL;

	while(cur){
		if(cur->timer == this){
				if(pre){
					pre->next = cur->next;
				}else{
					timer_head = cur->next;
				}
				cur->timer->mIsRunning = false;

				free(cur);

				break;
		}else{
			pre = cur;
			cur = cur->next;
		}
	}
	/*
	if(!timer_head){
		timer_thread_started = false;
	}
	*/
	//ulog_info("UTimer::~UTimer 2");
	unlock();
	//ulog_info("UTimer::~UTimer 3");

	//等待timer线程退出

	/*
	if(!timer_head){
			pthread_join(thread,NULL);
	}
	*/

	ulog_info("UTimer::~UTimer 4");
}
bool UTimer::isSettled(){

	utimer_link cur = timer_head;

	while(cur){
		if(cur->timer == this)return true;
		cur = cur->next;
	}
	return false;
}
void UTimer::stop(){
	lock();
	mIsRunning = false;
	unlock();
}
void UTimer::start(){

	if(!mFNTimer)return;

	if(isSettled()){
		if(!mIsRunning){
			mLastEntryTime = av_gettime();
			mIsRunning = true;
		}
		return;
	}

	utimer_link timer = (utimer_link)malloc(sizeof(struct utimer_node));

	timer->timer = this;
	timer->next = NULL;


	lock();

	if(!timer_head)timer_head = timer;
	else{
		timer->next = timer_head;
		timer_head = timer;
	}
	mIsRunning = true;
	mLastEntryTime = av_gettime();

	if(!timer_thread_started){
		//modify by eric
		timer_thread_started = true;
		//启动Timer线程
		if (pthread_create(&thread, NULL, timerThread, this->mData)) {
			ulog_err("pthread_create timerThread failed:start");
		}
	}
	unlock();
}
void*	UTimer::timerThread(void* data){

		utimer_link cur;
		int64_t	now;

		ulog_info("timerThread enter");

		UPlayer* player = (UPlayer*)data;

		assert(player);
#if PLATFORM_DEF == ANDROID_PLATFORM
		player->mCrashHandler.registerTid();
#endif

		while(timer_thread_started){
			usleep(UTIMER_MIN_PRECISION);
			lock();
			cur = timer_head;

			while(cur){
				if(cur->timer->mIsRunning){
					if(( (now=av_gettime()) - cur->timer->mLastEntryTime ) >= cur->timer->mInterval){
						cur->timer->mLastEntryTime = now;
						cur->timer->mFNTimer(cur->timer->mData);
					}
				}
				cur = cur->next;
			}
			unlock();
		}
#if PLATFORM_DEF == ANDROID_PLATFORM
		player->mCrashHandler.unRegisterTid();
#endif

		ulog_info("timerThread exit");

		return (void *)0;
}
