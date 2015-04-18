/** @file  uallocator.h
  *	@brief 用于实现双Queue机制中三种类型的数据分配器
  * @author  YanJia
  * @date  2012-5-7
  * @version	1.0
  * @note
  * 工具类，需要配合UQueue对象使用
*/

#ifndef UALLOCATOR_H_
#define UALLOCATOR_H_

#include "ucommon.h"

/** @brief  用于标识分配器类型 */
typedef enum{
	UALLOCATOR_TYPE_UNKNOWN = 0,
	UALLOCATOR_TYPE_PACKET,			/**音视频数据包类型分配器*/
	UALLOCATOR_TYPE_PCM,			/**音频PCM数据包类型分配器*/
	UALLOCATOR_TYPE_YUV,			/**视频YUV数据包类型分配器*/
	UALLOCATOR_TYPE_MAX,
}UALLOCATOR_TYPE;


/**
  * @class  IAllocator uallocator.h
  * @brief  数据分配器接口
  * @author YanJia
  * @note
  * 包含线程安全机制，可以被多线程同时使用
*/
class IAllocator{
public:
	friend class UAllocatorFactory;
public:
	/**
	  * @brief  分配器构造函数
	  * @author  YanJia
	*/
	IAllocator();
	/**
	  * @brief  多态析构函数
	  * @author  YanJia
	*/
	virtual ~IAllocator();
	/**
	  * @brief  用于数据分配
	  * @author  YanJia
	  * @param[in]  item 需要复制的数据对象
	  * @return  成功返回已分配对象指针，失败返回NULL
	*/
	virtual void*	alloc(void*	item) = 0;
	/**
	  * @brief  用于释放已分配数据对象
	  * @author  YanJia
	  * @param[in]  item 需要释放的数据对象
	  * @return  void
	*/
	virtual void	release(void* item) = 0;

protected:
	/**
	  * @brief  进入公共区域时上锁
	  * @author  YanJia
	  * @return  void
	*/
	void lock();
	/**
	  * @brief  退出公共区域时解锁
	  * @author  YanJia
	  * @return  void
	*/
	void unlock();
	/**
	* @brief  分配器类型
	*/
	UALLOCATOR_TYPE		mType;
private:
	/**
	* @brief  mutex锁
	*/
	ULock     mLock;
	/**
	* @brief  资源引用计数
	*/
	int 				mRefCount;

};

/**
  * @class  UAVPacketAllocator uallocator.h
  * @brief  音视频数据包分配器
  * @author YanJia
  * @note
  * 采用ffmpeg的AVPacket数据机制实现解音视频数据包的分配和释放
*/
class UAVPacketAllocator:public IAllocator{
public:
	friend class UAllocatorFactory;
private:
	/**
	  * @brief  音视频数据包分配器私有构造函数
	  * @author  YanJia
	*/
	UAVPacketAllocator(){mType = UALLOCATOR_TYPE_PACKET;}
public:
	/**
	  * @brief  音视频数据包分配器析构函数
	  * @author  YanJia
	*/
	~UAVPacketAllocator(){}
private:
	/**
	  * @brief  用于数据分配
	  * @author  YanJia
	  * @param[in]  item 需要复制的数据对象
	  * @return  成功返回已分配对象指针，失败返回NULL
	*/
	void*	alloc(void* item);
	/**
	  * @brief  用于释放已分配数据对象
	  * @author  YanJia
	  * @param[in]  item 需要释放的数据对象
	  * @return  void
	*/
	void 	release(void* item);
private:
	/**
	* @brief  分配器对象指针
	*/
	static IAllocator* 	Allocator;
};

/**
  * @class  UAudioPCMAllocator uallocator.h
  * @brief  音频PCM数据包分配器
  * @author YanJia
  * @note
  * 采用首次分配机制实现数据内存管理，PCM包内存在第一次使用时分配好，
  * 以后每次使用第一次分配的内存，直到播放结束后释放首次分配内存，
  * 分配的内存多少取决于PCM队列的长度，默认长度为2
*/
class UAudioPCMAllocator:public IAllocator{
public:
	friend class UAllocatorFactory;
private:
	/**
	  * @brief  音频PCM数据包分配器私有构造函数
	  * @author  YanJia
	*/
	UAudioPCMAllocator(){mType = UALLOCATOR_TYPE_PCM;}
public:
	/**
	  * @brief  音频PCM数据包分配器析构函数
	  * @author  YanJia
	*/
	~UAudioPCMAllocator(){}
private:
	/**
	  * @brief  用于数据分配
	  * @author  YanJia
	  * @param[in]  item 需要复制的数据对象
	  * @return  成功返回已分配对象指针，失败返回NULL
	*/
	void*	alloc(void* item);
	/**
	  * @brief  用于释放已分配数据对象
	  * @author  YanJia
	  * @param[in]  item 需要释放的数据对象
	  * @return  void
	*/
	void 	release(void* item);

private:
	/**
	* @brief  分配器对象指针
	*/
	static IAllocator* 	Allocator;
};
/**
  * @class  UVideoYUVAllocator uallocator.h
  * @brief  视频YUV数据包分配器
  * @author YanJia
  * @note
  * 采用静态分配机制实现数据内存管理，YUV数据包内存在播放器prepare时分配，
  * 以后每次使用同一块内存，直到播放结束后释放该内存，
  * 目前只分配一块内存，由于目前视频解码效率低，耗时大概是视频渲染时间的10倍以上，
  * 故即便采用同一块内存，但视频解码和视频渲染在独立线程里同时并发进行时，
  * 永远不会出现解码后的数据overwrite正在渲染数据的情况，示意图如下：
  *
  * 			解码线程与渲染线程并发执行
  *
  * 	解码时间：|	------------------------------ |		10倍于渲染时间
  * 	渲染时间：|			---					   |
  *
  * 			在下一次解码完毕前，渲染肯定已经执行完毕
  *
  *
  * 等视频优化到解码时间与渲染时间一致时， 分配器策略将更改为与音频PCM策略一致
*/
class UVideoYUVAllocator:public IAllocator{
public:
	friend class UAllocatorFactory;
private:
	/**
	  * @brief  视频YUV数据包分配器私有构造函数
	  * @author  YanJia
	*/
	UVideoYUVAllocator(){mType = UALLOCATOR_TYPE_YUV;}
public:
	/**
	  * @brief  视频YUV数据包分配器析构函数
	  * @author  YanJia
	*/
	~UVideoYUVAllocator(){}
private:
	/**
	  * @brief  用于数据分配
	  * @author  YanJia
	  * @param[in]  item 需要复制的数据对象
	  * @return  返回item
	*/
	void*	alloc(void* item);
	/**
	  * @brief  用于释放已分配数据对象
	  * @author  YanJia
	  * @param[in]  item 需要释放的数据对象
	  * @return  void
	*/
	void 	release(void* item);

private:
	/**
	* @brief  分配器对象指针
	*/
	static IAllocator* 	Allocator;
};

/**
  * @class  UAllocatorFactory uallocator.h
  * @brief  数据分配器工厂
  * @author YanJia
  * @note
  * 用于创建和销毁分配器对象，并支持线程安全
*/
class UAllocatorFactory{
public:
	/**
	  * @brief  获得指定类型分配器
	  * @author  YanJia
	  * @param[in]  type 分配器类型
	  * @return  返回分配器对象指针
	*/
	static IAllocator* GetAllocator(UALLOCATOR_TYPE type);
	/**
	  * @brief  释放分配器
	  * @author  YanJia
	  * @param[in]  allocator 分配器对象指针
	  * @return  void
	*/
	static void		DestoryAllocator(IAllocator* allocator);
};

#endif /* UALLOCATOR_H_ */
