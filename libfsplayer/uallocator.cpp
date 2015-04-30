#include "uallocator.h"

extern "C" {

#include "libavformat/avformat.h"

} // end of extern C

IAllocator *UAVPacketAllocator::Allocator = NULL;
IAllocator *UAudioPCMAllocator::Allocator = NULL;
IAllocator *UVideoYUVAllocator::Allocator = NULL;
IAllocator::IAllocator():mLock(ULOCK_TYPE_NORMAL){

	//初始化引用计数
	mRefCount = -1;
	//分配器类型
	mType = UALLOCATOR_TYPE_UNKNOWN;
}

IAllocator::~IAllocator(){}

void IAllocator::lock(){
	//上锁
	mLock.lock();
}
void IAllocator::unlock(){
	//解锁
	mLock.unlock();
}


void* UAVPacketAllocator::alloc(void* item){

	av_link link = (av_link)item;
	AVPacket*	pkt = (AVPacket*)link->item;

	//复制音视频包，使用堆内存装载数据
    if (av_dup_packet(pkt)){
    	ulog_err("UAVPacketAllocator::alloc:av_dup_packet failed");
    	return NULL;
    }
    //分配数据包外壳
    AVPacket	*packet = (AVPacket*)av_malloc(sizeof(AVPacket));

    if(!packet){
    	ulog_err("UAVPacketAllocator::alloc:av_malloc failed");
    	av_free_packet(pkt);
    	return NULL;
    }

    //浅copy，指向pkt的堆内存
    *packet = *pkt;

    //装载数据
    link->item = packet;
    link->next = NULL;
    link->size = 0;

    return packet;
}
void UAVPacketAllocator::release(void * item){

	if(!item)return;

	av_link link = (av_link)item;
	AVPacket*	pkt = (AVPacket*)link->item;

	if(pkt){
		//释放数据包堆内存
		if(UPLAYER_DECODER_FLUSH_FLAG != link->flag)av_free_packet(pkt);
		//释放数据包外壳
		av_free(pkt);
	}
	//重置成员
	link->item = NULL;
	link->size = 0;
	link->flag = 0;
	link->next = NULL;
}
void* UAudioPCMAllocator::alloc(void* item){

	av_link link = (av_link)item;

	if(!link->item){
		ulog_err("UAudioPCMAllocator::alloc:av_malloc link->item == NULL");
		return NULL;
	}

	if(link->size > 0){
		//如果之前分配过内存，就用原来的，否则分配新的空间
		if(!link->data_buf){
			link->data_buf = av_malloc(link->size);
			if(!link->data_buf){
				ulog_err("UAudioPCMAllocator::alloc:av_malloc failed");
				return NULL;
			}
			link->data_size = link->size;
		}else if(link->size > link->data_size){
			link->data_buf = av_realloc(link->data_buf,link->size);
			if(!link->data_buf){
				ulog_err("UAudioPCMAllocator::alloc:av_realloc failed");
				return NULL;
			}
			link->data_size = link->size;
		}
		//装载PCM数据
		memcpy(link->data_buf,link->item,link->size);
		link->item = link->data_buf;
	}else{
		ulog_err("UAudioPCMAllocator::alloc:link->size <= 0");
		return NULL;
	}
	link->next = NULL;
	return link->item;
}
void UAudioPCMAllocator::release(void * item){

	if(!item)return;

	av_link link = (av_link)item;
	if(link && link->data_buf){
		//释放PCM buffer
		av_free(link->data_buf);
		link->data_buf = NULL;
		link->data_size = 0;
	}
	//重置成员
	link->size = 0;
	link->item = NULL;
	link->next = NULL;
}
void* UVideoYUVAllocator::alloc(void* item){
	av_link link = (av_link)item;

	if(!link->item){
		ulog_err("UVideoYUVAllocator::alloc:av_malloc link->item == NULL");
		return NULL;
	}

	if(link->size > 0){
		//如果之前分配过内存，就用原来的，否则分配新的空间
		if(!link->data_buf){
			link->data_buf = av_malloc(link->size);
			if(!link->data_buf){
				ulog_err("UVideoYUVAllocator::alloc:av_malloc failed");
				return NULL;
			}
			link->data_size = link->size;
		}else if(link->size > link->data_size){
			link->data_buf = av_realloc(link->data_buf,link->size);
			if(!link->data_buf){
				ulog_err("UVideoYUVAllocator::alloc:av_realloc failed");
				return NULL;
			}
			link->data_size = link->size;
		}
		//装载PCM数据
		memcpy(link->data_buf,link->item,link->size);
		link->item = link->data_buf;
	}else if(!link->size && UPLAYER_REDRAW_LAST_YUV == link->flag){
		//ulog_info("UPLAYER_REDRAW_LAST_YUV");
	}else{
		ulog_err("UVideoYUVAllocator::alloc:link->size <= 0");
		return NULL;
	}
	link->next = NULL;
	return link->item;

}
void UVideoYUVAllocator::release(void * item){

	if(!item)return;

	av_link link = (av_link)item;
	if(link && link->data_buf){

		//释放YUV buffer
		av_free(link->data_buf);
		link->data_buf = NULL;
		link->data_size = 0;
	}
	//重置成员
	link->size = 0;
	link->flag = 0;
	link->pts = 0;
	link->item = NULL;
	link->next = NULL;
}

IAllocator*	UAllocatorFactory::GetAllocator(UALLOCATOR_TYPE type){

	//参数判断
	if(type <= UALLOCATOR_TYPE_UNKNOWN || \
			type >= UALLOCATOR_TYPE_MAX){
		ulog_err("UAllocatorFactory::GetAllocator type = %d",type);
		return NULL;
	}

	IAllocator *Allocator;
	//获取相应类型的分配器，第一次获取需要创建分配器
	switch(type){
	case UALLOCATOR_TYPE_PACKET:
		if(!UAVPacketAllocator::Allocator)UAVPacketAllocator::Allocator = new UAVPacketAllocator();
		if(!UAVPacketAllocator::Allocator){
				ulog_err("UAllocatorFactory::new UAVPacketAllocator failed");
				return NULL;
		}
		Allocator = UAVPacketAllocator::Allocator;
		break;
	case UALLOCATOR_TYPE_PCM:
		if(!UAudioPCMAllocator::Allocator)UAudioPCMAllocator::Allocator = new UAudioPCMAllocator();
		if(!UAudioPCMAllocator::Allocator){
				ulog_err("UAllocatorFactory::new UAudioPCMAllocator failed");
				return NULL;
		}
		Allocator = UAudioPCMAllocator::Allocator;
		break;
	case UALLOCATOR_TYPE_YUV:
		if(!UVideoYUVAllocator::Allocator)UVideoYUVAllocator::Allocator = new UVideoYUVAllocator();
		if(!UVideoYUVAllocator::Allocator){
				ulog_err("UAllocatorFactory::new UVideoYUVAllocator failed");
				return NULL;
		}
		Allocator = UVideoYUVAllocator::Allocator;
		break;
	default:
		ulog_err("UAllocatorFactory::GetAllocator:type error");
		return NULL;
	}
	//增加引用计数
	Allocator->lock();
	Allocator->mRefCount++;
	Allocator->unlock();

	return Allocator;
}
void UAllocatorFactory::DestoryAllocator(IAllocator* allocator){

	//参数判断
	if(!allocator)return;

	if(allocator->mType <= UALLOCATOR_TYPE_UNKNOWN || \
			allocator->mType >= UALLOCATOR_TYPE_MAX)return;

	//减少引用计数，引用计数为0时需要释放资源

	if(allocator->mRefCount){
		allocator->lock();
		allocator->mRefCount--;
		allocator->unlock();
	}else{

		switch(allocator->mType){
			case UALLOCATOR_TYPE_PACKET:
				if(allocator != UAVPacketAllocator::Allocator){
					ulog_err("UAllocatorFactory::DestoryAllocator error1");
				}else
					UAVPacketAllocator::Allocator = NULL;
				//ulog_info("UAllocatorFactory::DestoryAllocator UALLOCATOR_TYPE_PACKET");
				break;
			case UALLOCATOR_TYPE_PCM:
				if(allocator != UAudioPCMAllocator::Allocator){
					ulog_err("UAllocatorFactory::DestoryAllocator error2");
				}else
					UAudioPCMAllocator::Allocator = NULL;
				//ulog_info("UAllocatorFactory::DestoryAllocator UALLOCATOR_TYPE_PCM");
				break;
			case UALLOCATOR_TYPE_YUV:
				if(allocator != UVideoYUVAllocator::Allocator){
					ulog_err("UAllocatorFactory::DestoryAllocator error3");
				}else
					UVideoYUVAllocator::Allocator = NULL;
				//ulog_info("UAllocatorFactory::DestoryAllocator UALLOCATOR_TYPE_YUV");
				break;
			default:
				ulog_err("UAllocatorFactory::GetAllocator:type error");

		}

		delete allocator;
	}

}
