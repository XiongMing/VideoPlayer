/** @file  uthread.h
  *	@brief 采用Pthread库实现的线程类
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/


#ifndef UTHREAD_H_
#define UTHREAD_H_

#include <pthread.h>

/**
  * @class  UThread uthread.h
  * @brief  实现线程的通用封装
  * @author YanJia
*/
class UThread
{
public:
	/**
	  * @brief  线程类构造函数
	  * @author  YanJia
	  * @param[in]  name 线程名称
	*/
	UThread(char* name);
	/**
	  * @brief  线程类析构函数
	  * @author  YanJia
	*/
	~UThread();

	/**
	  * @brief  启动线程
	  * @author  YanJia
	  * @return 成功返回NO_ERROR,失败返回FAILED
	*/
	int							start();

	/**
	  * @brief  停止线程接口函数
	  * @author  YanJia
	  * @return void
	*/
    virtual void				stop()=0;
	/**
	  * @brief  判断线程是否在运行
	  * @author  YanJia
	  * @return 在运行返回true，否则返回false
	*/
    bool						isRunning(){return mRunning;}

protected:

	/**
	  * @brief  启动线程函数接口
	  * @author  YanJia
	  * @return void
	*/
    virtual void                handleRun() = 0;

	/**
	  * @brief  阻塞等待线程运行结束
	  * @author  YanJia
	  * @return void
	*/
    void						wait();
	/**
	  * @brief  获得线程名称
	  * @author  YanJia
	  * @return 返回线程的名称
	*/
    char*						getName(){return mThreadName;}


private:
	/**
	  * @brief  线程名称
	*/
    char*						mThreadName;
	/**
	  * @brief  线程ID
	*/
    pthread_t                   mThread;
	/**
	  * @brief  线程是否在运行
	*/
    bool						mRunning;
private:

	/**
	  * @brief  线程入口函数
	  * @author  YanJia
	  * @param[in] ptr 线程运行参数
	  * @return void* 返回相关运行状态
	*/
	static void*				startThread(void* ptr);
};


#endif /* UTHREAD_H_ */
