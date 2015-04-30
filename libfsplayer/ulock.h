/** @file  ulock.h
  *	@brief 采用pthread库的mutex实现的线程锁
  * @author  YanJia
  * @date  2013-8-14
  * @version	1.0
  * @note
*/

#ifndef ULOCK_H_
#define ULOCK_H_

#include <pthread.h>

typedef enum{
	ULOCK_TYPE_NORMAL = PTHREAD_MUTEX_NORMAL,
	ULOCK_TYPE_RECURSIVE = PTHREAD_MUTEX_RECURSIVE,
	ULOCK_TYPE_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK,
}ULOCK_TYPE;

/**
  * @class  ULock ulock.h
  * @brief  线程锁类
  * @author YanJia
*/
class ULock{
public:
	/**
	  * @brief  构造函数
	  * @author  YanJia
	  * @param[in]  type 锁类型
	*/
	ULock(ULOCK_TYPE type);
	/**
	  * @brief  析构函数
	  * @author  YanJia
	*/
	~ULock();
	/**
	  * @brief  上锁
	  * @author  YanJia
	  * @return void
	*/
	void lock();
	/**
	  * @brief  lock的非阻塞版本
	  * @author  YanJia
	  * @return void
	*/
	void trylock();
	/**
	  * @brief  解锁
	  * @author  YanJia
	  * @return void
	*/
	void unlock();

	/**
	  * @brief  获得所对象
	  * @author  YanJia
	  * @return void*	mutex所对象
	*/
	pthread_mutex_t* getLock(){return &mLock;}


private:
	/**
	  * @brief  锁的类型
	*/
	ULOCK_TYPE	mType;
	/**
	  * @brief  mutex锁
	*/
	pthread_mutex_t	mLock;
};

class AutoULock{
public:
	AutoULock(ULock &lock):mLock(lock){
		mLock.lock();
	}
	~AutoULock(){
		mLock.unlock();
	}
private:
	ULock	&mLock;
};

#endif /* ULOCK_H_ */
