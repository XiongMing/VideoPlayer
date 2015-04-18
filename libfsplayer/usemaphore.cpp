#include "usemaphore.h"
#include "ulog.h"
USemaphore::USemaphore(unsigned int value):mValue(value),mLock(ULOCK_TYPE_NORMAL){

	//初始化条件变量
	if(pthread_cond_init(&mCond,NULL)){
		ulog_err("USemaphore::USemaphore:pthread_cond_init failed");
	}

}

USemaphore::~USemaphore(){

	//销毁USemaphore
	if(pthread_cond_destroy(&mCond)){
		ulog_err("USemaphore::~USemaphore:pthread_cond_destroy failed");
	}
}

void USemaphore::wait(){

	AutoULock lock(mLock);

	while( 0 == mValue ){
		//等待
		if(pthread_cond_wait(&mCond,mLock.getLock())){
			ulog_err("USemaphore::wait:pthread_cond_wait failed");
		}
	}
	mValue--;
}
void USemaphore::post(){

	AutoULock lock(mLock);

	mValue++;

	if(pthread_cond_signal(&mCond)){
		ulog_err("USemaphore::posts:pthread_cond_signal failed");
	}
}

int USemaphore::trywait(){

	AutoULock lock(mLock);

	if( 0 == mValue )return -1;

	mValue--;

	return 0;
}
