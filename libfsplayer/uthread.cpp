#include "uthread.h"
#include "ucommon.h"

UThread::UThread(const char* name){
	//设置线程名字
	mThreadName = name;
	//设置线程状态
	mRunning = false;
}
UThread::~UThread(){

}
int UThread::start(){
	//启动线程函数
    if(pthread_create(&mThread, NULL, startThread, this)){
    	ulog_err("pthread_create %s error",mThreadName?mThreadName:"");
    	return FAILED;
    }
    //add by eric
    mRunning = true;
    return NO_ERROR;
}
void UThread::wait(){

	if(mRunning){
		//等待线程结束
		if(pthread_join(mThread, NULL)){
			ulog_err("pthread_join %s error",mThreadName?mThreadName:"");

		}
	}
}

void* UThread::startThread(void* ptr)
{
	UThread* thread = (UThread *) ptr;

	ulog(ULOG_INFO,"starting thread %s",thread->mThreadName?thread->mThreadName:"");
	//进行子类的向下虚拟调用
    thread->handleRun();
    ulog(ULOG_INFO,"thread ended %s",thread->mThreadName?thread->mThreadName:"");
    return (void *)1;
}
