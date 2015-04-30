/** @file  uplayer.h
  *	@brief 基于google breakpad的崩溃日志类，主要解决只捕获uplayer相关的崩溃，不捕获进程中其它模块的崩溃
  * @author  YanJia
  * @date  2015-1-1
  * @version	1.0
  * @note
*/
#ifndef UCRASH_HANDLER_H_
#define UCRASH_HANDLER_H_

//播放器最大线程数
#define UPLAYER_MAX_THREAD 10

#include <strings.h>
#include <string>
#include <pthread.h>
#include "ulock.h"
#include "exception_handler.h"

/**
  * @class  UCrashHandler ucrash_handler.h
  * @brief  崩溃捕获封装类
  * @author YanJia
*/
class UThreadID{
public:
	/**
	  * @brief  线程ID类构造函数
	  * @author  YanJia
	*/
	UThreadID():flag_(0){}
	/**
	  * @brief  线程ID类构造函数
	  * @param[in]  tid	采用pthread_t构造UThreadID
	  * @author  YanJia
	*/
	UThreadID(pthread_t tid):tid_(tid),flag_(1){}
	/**
	  * @brief  析构函数
	  * @author  YanJia
	*/
	~UThreadID(){flag_ = 0;}
	/**
	  * @brief  判断是否为空
	  * @author  YanJia
	  * @bool true：为空，false：不为空
	*/
	bool empty(){return 0 == flag_;}
	/**
	  * @brief  清除已经设定的值
	  * @author  YanJia
	  * @void
	*/
	void clear(){flag_ = 0;}
	/**
	  * @brief  获得线程id
	  * @author  YanJia
	  * @void
	*/
	pthread_t & getTid(){return tid_;}

	/**
	  * @brief  返回UThreadID的string表示
	  * @author  YanJia
	  * @void
	*/
	std::string toString(){

		char str[16];

		if(!flag_)return "-1";

		sprintf(str,"%d",(int)tid_);

		return str;
	}
	/**
	  * @brief  赋值操作
	  * @param[in]  tid	需要赋值的线程id
	  * @author  YanJia
	  * @UThread 自身引用
	*/
	UThreadID & operator = (pthread_t & tid){
		this->tid_ = tid;
		this->flag_ = 1;

		return *this;
	}
	/**
	  * @brief  比较操作
	  * @param[in]  tid	需要比较的UTheadID 对象
	  * @author  YanJia
	  * @bool true：相等，false：不相等
	*/
	bool operator == (UThreadID & tid){
		return !this->empty() && !tid.empty() && pthread_equal(this->tid_,tid.tid_);
	}
	/**
	  * @brief  比较操作
	  * @param[in]  tid	需要比较的UTheadID 对象
	  * @author  YanJia
	  * @bool true：不相等，false：相等
	*/
	bool operator != (UThreadID & tid){
		return this->empty() || tid.empty() || !pthread_equal(this->tid_,tid.tid_);
	}
	/**
	  * @brief  比较操作
	  * @param[in]  tid	需要比较的线程id
	  * @param[in]  utid	UTheadID 对象
	  * @author  YanJia
	  * @bool true：相等，false：不相等
	*/
	bool operator == (pthread_t & tid){
		return !this->empty() && pthread_equal(this->tid_,tid);
	}
	/**
	  * @brief  比较操作
	  * @param[in]  tid	需要比较的线程id
	  * @param[in]  utid	UTheadID 对象
	  * @author  YanJia
	  * @bool true：不相等，false：相等
	*/
	bool operator != (pthread_t & tid){
		return this->empty() || !pthread_equal(this->tid_,tid);
	}
private:
	/**
	  * @brief  线程id
	*/
	pthread_t	tid_;
	/**
	  * @brief  内部标志，用于标记tid是否已经赋值
	*/
	int		flag_;
};
/**
  * @class  UCrashHandler ucrash_handler.h
  * @brief  崩溃捕获封装类
  * @author YanJia
*/
class UCrashHandler{

public:
	/**
	  * @brief  google breakpad崩溃捕获类构造函数
	  * @author  YanJia
	*/
	UCrashHandler(const char * dumpPath = NULL);
	/**
	  * @brief  析构函数
	  * @author  YanJia
	*/
	~UCrashHandler();

public:
	/**
	  * @brief  设置崩溃转储路径
	  * @param[in]  path	崩溃日志储存路径
	  * @author  YanJia
	  * @void
	*/
	void setDumpPath(const char *path);
	/**
	  * @brief  安装Breakpad
	  * @param[in]  path	崩溃日志储存路径
	  * @author  YanJia
	  * @void
	*/
	void setupBreakpad();
public:
	/**
	  * @brief  注册调用线程的线程id
	  * @author  YanJia
	  * @void
	*/
	void registerTid();
	/**
	  * @brief  注册指定的线程id
	  * @param[in]  tid	要注册的线程id
	  * @author  YanJia
	  * @void
	*/
	void registerTid(UThreadID & tid);
	void registerTid(pthread_t & tid);
	/**
	  * @brief  反注册调用线程的线程id
	  * @author  YanJia
	  * @void
	*/
	void unRegisterTid();
	/**
	  * @brief  反注册调用线程的线程id
	  * @param[in]  tid	需要反注册的线程id
	  * @author  YanJia
	  * @void
	*/
	void unRegisterTid(UThreadID & tid);
	void unRegisterTid(pthread_t & tid);

private:
	/**
	 * @brief  崩溃过滤，用于过滤多余的崩溃，只捕获属于uplayer的崩溃
	 * @author  YanJia
	 * @return true:不捕获,false:捕获，生成dump日志
	 */
	static bool CrashHandlerCallback(const void* crash_context,size_t crash_context_size,void* context);
private:


	/**
	 * @brief  播放器内的线程ids
	 */
	UThreadID	mTids[UPLAYER_MAX_THREAD];
	/**
	 * @brief  已注册线程数
	 */
	int	mRegisterTidsNum;
	/**
	 * @brief  保护锁
	 */
	ULock			mLock;
	/**
	 * @brief  dump文件路径
	 */
	std::string		mDumpPath;
	/**
	 * @brief  崩溃处理器
	 */
	google_breakpad::ExceptionHandler* mExceptionHandler;
};


#endif /* UCRASH_HANDLE_H_ */
