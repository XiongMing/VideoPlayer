/** @file  uqueue.h
  *	@brief 音视频双Q机制的实现
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/
#ifndef UQUEUE_H_
#define UQUEUE_H_

#include "ucommon.h"
#include "uallocator.h"
#include "ulock.h"
#include "usemaphore.h"

/**
* @brief  队列类型
*/
typedef enum{
	UQUEUE_TYPE_UNKNOWN = 0,
	UQUEUE_TYPE_SLOT,					/**空槽类型*/
	UQUEUE_TYPE_PACKET,					/**音视频包类型*/
	UQUQUE_TYPE_AUDIO_DATA,				/**音频PCM包类型*/
	UQUQUE_TYPE_VIDEO_DATA,				/**视频YUV包类型*/
	UQUEUE_TYPE_MAX,
}UQUEUE_TYPE;

/**
* @brief  最大队列长度限制
*/
#define UQUEUE_MAX_ITEM_NUM	1000


/**
  * @class  UQueue uplayer.h
  * @brief  队列类
  * @author YanJia
*/
class UQueue{

public:
	/**
	  * @brief  队列构造函数
	  * @author  YanJia
	  * @param[in]  type 队列类型
	  * @param[in]	init_items 初始队列长度
	  * @param[in]	data_type	空槽队列所对应的数据类型
	*/
	UQueue(UQUEUE_TYPE type,int init_items,UQUEUE_TYPE data_type = UQUEUE_TYPE_PACKET);
	/**
	  * @brief  队列析构函数
	  * @author  YanJia
	*/
	~UQueue();
	/**
	  * @brief  入队
	  * @author  YanJia
	  * @param[in]  item 数据项
	  * @return void
	*/
	void put(void* item);
	/**
	  * @brief  出对
	  * @author  YanJia
	  * @return void* 出对的数据项
	*/
	void* get(bool flag = true);
	/**
	  * @brief  获得队列中数据项的个数
	  * @author  YanJia
	  * @return 返回队列中的数据项个数
	*/
	int size()const{return mSize;}

	/**
	  * @brief  获得初始化空槽数（只有空槽队列初始化空槽数不为0）
	  * @author  YanJia
	  * @return 返回初始化空槽数
	*/
	int getInitItems()const{return mIinitItems;}
	/**
	  * @brief  中断队列阻塞，释放等待线程
	  * @author  YanJia
	  * @return void
	*/
	void abort();
private:
	/**
	  * @brief  flush出对
	  * @author  YanJia
	  * @return void* 出对的数据项
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
	  * @brief  增加资源计数，释放等待线程
	  * @author  YanJia
	  * @return void
	*/
	void post();

public:
	/**
	  * @brief  清空队列，把packet_queue中的空槽还回slot_queue
	  * @author  YanJia
	  * @return void
	*/
	static void flush(UQueue *packet_queue,UQueue *slot_queue);

	/**
	  * @brief  在队列中标记需要flush的数据包
	  * @author  YanJia
	  * @return void
	*/
	void flush();
private:
	/**
	  * @brief  对首指针
	*/
	av_link				mHead;
	/**
	  * @brief  对尾指针
	*/
	av_link				mTail;
	/**
	  * @brief  资源个数
	*/
	int					mSize;
	/**
	  * @brief  互斥锁
	*/
	ULock     mLock;
	/**
	  * @brief  信号量
	*/
	USemaphore		mSem;
	/**
	  * @brief  中断标记
	*/
	bool				mAbort;
	/**
	  * @brief  队列类型
	*/
	UQUEUE_TYPE			mType;
	/**
	  * @brief  空槽队列的数据类型
	*/
	UQUEUE_TYPE			mDataType;
	/**
	  * @brief  队列锁对应的数据项分配器
	*/
	IAllocator*			mAllocator;
	/**
	  * @brief  初始化的空槽个数
	*/
	int					mIinitItems;
	/**
	  * @brief  空槽
	*/
	av_link				mSlots;


};
#endif /* UQUEUE_H_ */
