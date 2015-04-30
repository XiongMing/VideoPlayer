/** @file  umsg_queue.h
  *	@brief 消息队列机制
  * @author  YanJia
  * @date  2013-2-25
  * @version	1.0
  * @note
*/

#ifndef UMSG_QUEUE_H_
#define UMSG_QUEUE_H_

#include <pthread.h>
#include <semaphore.h>
#include "ucommon.h"
#include "ulock.h"
#include "usemaphore.h"

/**
* @brief  消息节点，链表数据结构
*/
typedef struct umsg_node{
	int	msg;				//消息报
	struct umsg_node*	next;	//下一个节点
}*umsg_link;


/** @file  umsg_queue.h
  *	@brief 消息队列类
  * @author  YanJia
*/
class UMsgQueue{
public:
	/**
	  * @brief  消息队列构造函数
	  * @author  YanJia
	*/
	UMsgQueue();
	/**
	  * @brief  消息队列析构函数
	  * @author  YanJia
	*/
	~UMsgQueue();
	/**
	  * @brief  client发送消息
	  * @author  YanJia
	  * @param[in]  msg 消息值
	  * @return void
	*/
	void send(int msg);
	/**
	  * @brief  server阻塞接收client发送的消息
	  * @author  YanJia
	  * @return 返回的消息报
	*/
	int recv();
	/**
	  * @brief  server非阻塞接收client发送的消息
	  * @author  YanJia
	  * @return 返回的消息报
	*/
	int tryRecv();
	/**
	  * @brief  清空消息
	  * @author  YanJia
	  * @return 返回的消息报
	*/
	int flush();
	/**
	  * @brief  退出消息循环
	  * @author  YanJia
	  * @return void
	*/
	void exit();

private:

	/**
	  * @brief  上锁
	  * @author  YanJia
	  * @return void
	*/
	void lock();
	/**
	  * @brief  解锁
	  * @author  YanJia
	  * @return void
	*/
	void unlock();
	/**
	  * @brief  阻塞获取资源到来
	  * @author  YanJia
	  * @return void
	*/
	void wait();
	/**
	  * @brief  非阻塞获取资源到来
	  * @author  YanJia
	  * @return void
	*/
	void tryWait();
	/**
	  * @brief  增加资源计数，释放等待线程
	  * @author  YanJia
	  * @return void
	*/
	void post();


private:

	/**
	  * @brief  互斥锁
	*/
	ULock     mLock;
	/**
	  * @brief  信号量
	*/
	USemaphore	mSem;
	/**
	  * @brief  消息队列头
	*/
	umsg_link			mHead;

	/**
	  * @brief  消息队尾
	*/
	umsg_link			mTail;
	/**
	  * @brief  中断标记
	*/
	bool				mAbort;
};


#endif /* UMSG_QUEUE_H_ */
