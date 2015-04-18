/** @file  uthread.h
  *	@brief 采用条件变量实现的简单信号量机制，以便于兼容iOS平台（iOS没有匿名信号量），减少从Android到iOS的移植工作量
  * @author  YanJia
  * @date  2013-8-13
  * @version	1.0
  * @note
*/

#ifndef USEMAPHORE_H_
#define USEMAPHORE_H_

#include <pthread.h>
#include "ulock.h"

/**
  * @class  UQueue uplayer.h
  * @brief  队列类
  * @author YanJia
*/
class USemaphore{

public:
	/**
	  * @brief  信号量构造函数
	  * @author  YanJia
	  * @param[in]  value 信号量的初始值，大于等于0
	*/
	USemaphore(unsigned int value = 0);

	/**
	  * @brief  信号量析构函数
	  * @author  YanJia
	*/
	~USemaphore();

	/**
	  * @brief  信号量UP操作
	  * @author  YanJia
	  * @return void
	*/
	void post();
	/**
	  * @brief  信号量Down操作,阻塞
	  * @author  YanJia
	  * @return void
	*/
	void wait();
	/**
	  * @brief  信号量Down操作，非阻塞
	  * @author  YanJia
	  * @return decrement成功返回0，否则返回-1
	*/
	int trywait();

private:

	/**
	  * @brief	锁
	*/
	ULock	mLock;
	/**
	  * @brief	条件变量
	*/
	pthread_cond_t	mCond;
	/**
	  * @brief	信号量当前的值
	*/
	unsigned int	mValue;
};
#endif /*USEMAPHORE_H_*/
