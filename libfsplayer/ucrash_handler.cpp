#include "ucrash_handler.h"
#include "exception_handler.h"
#include <strings.h>
#include <assert.h>
#include "ulog.h"

UCrashHandler::UCrashHandler(const char * dumpPath):mRegisterTidsNum(0),mDumpPath(dumpPath),mLock(ULOCK_TYPE_RECURSIVE),mExceptionHandler(NULL){

		//初始化线程结构
		bzero((void*)mTids,sizeof(mTids));

		//如果用户已传递dump路径，则初始化breakpad
		if(dumpPath){
			setupBreakpad();
		}
}

UCrashHandler::~UCrashHandler(){

		//清除google breakpad实例
		delete mExceptionHandler;
		mExceptionHandler = NULL;
}
void UCrashHandler::setDumpPath(const char *path){

		//确保path不为空
		assert(path);
		//保持dump路径
		mDumpPath = path;
}
void UCrashHandler::registerTid(pthread_t & tid){

		//加入自动锁保护下面mTids
		AutoULock lock(mLock);

		//如果已经注册过就返回
		for( int i = 0; i < UPLAYER_MAX_THREAD; i++ ){
			if( mTids[i] == tid ){
				return;
			}
		}

		//找到一个空槽，用于保存tid
		for( int i = 0; i < UPLAYER_MAX_THREAD; i++ ){
			if( mTids[i].empty()){
				mRegisterTidsNum++;
				mTids[i] = tid;
				return;
			}
		}
		//没有找到空槽,说明空槽已满，代码不应该执行到这里
		ulog_err("UCrashHandler::registerTid,mTids is full");
}
void UCrashHandler::registerTid(UThreadID & tid){

		this->registerTid(tid.getTid());
}
void UCrashHandler::registerTid(){

		//获取当前调用线程的线程id
		//pthread_t tid = pthread_self();
		pthread_t tid = gettid();

		registerTid(tid);
}
void UCrashHandler::unRegisterTid(){

		//获取当前调用线程的线程id
		//pthread_t tid = pthread_self();
		pthread_t tid = gettid();

		unRegisterTid(tid);
}
void UCrashHandler::unRegisterTid(UThreadID & tid){

		unRegisterTid(tid.getTid());
}
void UCrashHandler::unRegisterTid(pthread_t & tid){

		//加入自动锁保护下面mTids
		AutoULock lock(mLock);

		//在mTids中查找当前调用线程的线程id，找到的话清除标志
		for( int i = 0; i < UPLAYER_MAX_THREAD; i++ ){

			if(mTids[i] == tid){
				//清除标志位
				mTids[i].clear();
				mRegisterTidsNum--;
				return;
			}
		}
		//代码不应当执行到这里，输出没有被注册过的线程id
		ulog_err("UCrashHandler::unRegisterTid,there is no tid = %d found in mTids",(int)tid);
}
void UCrashHandler::setupBreakpad(){

		//确保储存路径不为空
		assert(!mDumpPath.empty());

		//初始化路径描述符
		google_breakpad::MinidumpDescriptor descriptor(mDumpPath);

		//创建google breakpad client实例
		mExceptionHandler = new google_breakpad::ExceptionHandler(descriptor, NULL, NULL, this, true, -1);

		//确保new成功
		assert(mExceptionHandler);

		//设置crash过滤器，只转储uplayer自身的崩溃
		mExceptionHandler->set_crash_handler(CrashHandlerCallback);

}
bool UCrashHandler::CrashHandlerCallback(const void* crash_context,size_t crash_context_size,void* context){

		//获得传递过来的crash context
		google_breakpad::ExceptionHandler::CrashContext *crash = (google_breakpad::ExceptionHandler::CrashContext*)crash_context;

		UCrashHandler* handler = (UCrashHandler*)context;

		//确保参数不为空
		assert(crash && handler);

		//加入自动锁保护下面的mTids操作
		AutoULock lock(handler->mLock);

		//从crash context获取crash线程的线程id
		pthread_t crash_tid = crash->tid;

		ulog_info("CrashHandlerCallback crash id = %d,total registered tid num = %d",(int)crash_tid,handler->mRegisterTidsNum);

		//如果crash线程已经注册过则转储
		for( int i = 0; i < UPLAYER_MAX_THREAD; i++ ){

			ulog_info("CrashHandlerCallback cur id = %d",(int)handler->mTids[i].getTid());
			if( handler->mTids[i] == crash_tid ){
				ulog_info("CrashHandlerCallback return false,save dump file!");
				return false;
			}
		}
		ulog_info("CrashHandlerCallback return true,don't save dump file!");
		//如果不是uplayer的崩溃则不转储
		return true;
}
