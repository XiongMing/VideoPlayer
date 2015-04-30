#include "ulock.h"
#include "ulog.h"

ULock::ULock(ULOCK_TYPE type):mType(type){

	//初始化锁属性
	pthread_mutexattr_t		attr;

	if(pthread_mutexattr_init(&attr)){
		ulog_err("ULock::ULock pthread_mutexattr_init failed");
	}
	//初始化锁类型
	if(pthread_mutexattr_settype(&attr,mType)){
		ulog_err("ULock::ULock pthread_mutexattr_settype failed");
	}

	//初始化锁
	if(pthread_mutex_init(&mLock,&attr)){
		ulog_err("ULock::ULock pthread_mutex_init failed");
	}

	//释放属性对象
	if(pthread_mutexattr_destroy(&attr)){
		ulog_err("ULock::ULock pthread_mutexattr_destroy failed");
	}

}

ULock::~ULock(){

	//释放属性对象
	if(pthread_mutex_destroy(&mLock)){
		ulog_err("ULock::ULock pthread_mutex_destroy failed");
	}

}
void ULock::lock(){

	//阻塞上锁
	if(pthread_mutex_lock(&mLock)){
		ulog_err("ULock::ULock pthread_mutex_lock failed,mType=%d",mType);
	}
}
void ULock::trylock(){
	//非阻塞上锁
	if(pthread_mutex_trylock(&mLock)){
		ulog_err("ULock::ULock pthread_mutex_trylock failed,mType=%d",mType);
	}
}
void ULock::unlock(){
	//解锁
	if(pthread_mutex_unlock(&mLock)){
		ulog_err("ULock::ULock pthread_mutex_unlock failed,mType=%d",mType);
	}
}
