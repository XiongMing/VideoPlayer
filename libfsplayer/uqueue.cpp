#include "uqueue.h"

UQueue::UQueue(UQUEUE_TYPE type,int init_items,UQUEUE_TYPE data_type):mLock(ULOCK_TYPE_NORMAL),mSem(init_items){

	//初始化成员变量
    
	mHead = NULL;
	mTail = NULL;
	mSlots = NULL;
	mAllocator = NULL;
	mAbort = false;
	mSize = 0;

	//参数判断
	if(type <= UQUEUE_TYPE_UNKNOWN || \
			type >= UQUEUE_TYPE_MAX || \
			init_items < 0 || \
			init_items >UQUEUE_MAX_ITEM_NUM){
		ulog_err("UQueue: param err1");
		goto uqueue_err;
	}
	//参数判断
	if(UQUEUE_TYPE_SLOT != type && init_items){
		ulog_err("UQueue: param err2");
		goto uqueue_err;
	}
	//参数判断
	if(UQUEUE_TYPE_SLOT == type && !init_items){
		ulog_err("UQueue: param err3");
		goto uqueue_err;
	}

	//设置Queue类型和空槽对应的数据类型
	mType = type;
	mDataType = data_type;

	UALLOCATOR_TYPE allocator_type,slot_allocator_type;

	//确定分配器类型
	switch(mType){
	case UQUEUE_TYPE_SLOT:
		allocator_type = UALLOCATOR_TYPE_UNKNOWN;
		break;
	case UQUEUE_TYPE_PACKET:
		allocator_type = UALLOCATOR_TYPE_PACKET;
		break;
	case UQUQUE_TYPE_AUDIO_DATA:
		allocator_type = UALLOCATOR_TYPE_PCM;
		break;
	case UQUQUE_TYPE_VIDEO_DATA:
		allocator_type = UALLOCATOR_TYPE_YUV;
		break;
	default:
		ulog_err("UQueue::UQueue:type UQUEUE_TYPE_UNKNOWN");
		allocator_type = UALLOCATOR_TYPE_UNKNOWN;
		goto uqueue_err;


	}

	if(UQUEUE_TYPE_SLOT == mType){
		//判断空槽的数据类型
		switch(mDataType){
		case UQUEUE_TYPE_PACKET:
			slot_allocator_type = UALLOCATOR_TYPE_PACKET;
			break;
		case UQUQUE_TYPE_AUDIO_DATA:
			slot_allocator_type = UALLOCATOR_TYPE_PCM;
			break;
		case UQUQUE_TYPE_VIDEO_DATA:
			slot_allocator_type = UALLOCATOR_TYPE_YUV;
			break;
		default:
			ulog_err("UQueue::UQueue:data_type UQUEUE_TYPE_UNKNOWN");
			slot_allocator_type = UALLOCATOR_TYPE_UNKNOWN;
			goto uqueue_err;
		}
		//获得空槽对应的数据分配器
		mAllocator = UAllocatorFactory::GetAllocator(slot_allocator_type);
		if(!mAllocator){
			ulog_err("UQueue::UQueue:UAllocatorFactory::GetAllocator failed");
			goto uqueue_err;
		}
	}else{
		//获得包队列或数据队列的内存分配器
		mAllocator = UAllocatorFactory::GetAllocator(allocator_type);
		if(!mAllocator){
			ulog_err("UQueue::UQueue:UAllocatorFactory::GetAllocator failed2");
			goto uqueue_err;
		}
	}
	//初始化队列资源个数
	mSize = mIinitItems = init_items;

	//初始值不为0则创建空槽资源
	if(mIinitItems){
		//分配空槽内存
		int size = sizeof(struct node) * mIinitItems;
		mSlots = (av_link)malloc(size);
		if(!mSlots){
			ulog_err("UQueue::UQueue:mSlots malloc failed");
			goto uqueue_err;
		}
		//清空内存
		bzero(mSlots,size);

		//将空槽资源串成一个单链表
		mHead = mSlots;
		if(1 == mIinitItems){
			mTail = mHead;
		}else{
			av_link item = mHead;
			mTail = &mHead[mIinitItems-1];
			for(int i=0;i<mIinitItems-1;i++){
				item->next = &item[1];
				item++;
			}
		}
	}

	return;

uqueue_err:

	//出错释放资源
	this->~UQueue();

}

UQueue::~UQueue(){


	//对于空槽Queue需要释放器数据资源
	if(UQUEUE_TYPE_SLOT == mType){
		av_link cur;

		if(!mAllocator){
			ulog_err("UQueue::~UQueue:mAllocator == NULL && UQUEUE_TYPE_PACKET == mType");
			goto start_free;
		}

		for(int i = 0;i < mIinitItems;i++){
			cur = &mSlots[i];
			mAllocator->release(cur);
		}
	}
start_free:

	//释放分配器
	UAllocatorFactory::DestoryAllocator(mAllocator);

	//释放空槽
	if(mSlots)free(mSlots);

	mHead = NULL;
	mTail = NULL;
	mSize = 0;

	mSlots = NULL;
	mAllocator = NULL;

	mAbort = true;

	mType = UQUEUE_TYPE_UNKNOWN;
	mIinitItems = 0;

}

void UQueue::put(void* item){

	if(!item)return;

	//上锁
	lock();

    av_link temp = (av_link)item;
    temp->next = NULL;

    //拷贝释放数据
    if(!mAllocator){
    	ulog_err("UQueue::put:mAllocator == NULL");
    	goto put_err;
    }
    if(UQUEUE_TYPE_SLOT == mType){
    	//释放对象
    	if(UQUEUE_TYPE_PACKET == mDataType)mAllocator->release(temp);
    	if(UQUQUE_TYPE_VIDEO_DATA == mDataType){
    		//重置成员
    		temp->size = 0;
    		temp->flag = 0;
    		temp->pts = 0;
    		temp->item = NULL;
    		temp->next = NULL;
    	}
    	//mAllocator->release(temp);

    }else {
    	//复制对象
		if(!mAllocator->alloc(temp)){
			ulog_err("UQueue::put:mAllocator->alloc failed");
			goto put_err;
		}
    }

	//插入队尾
	if(!mTail){
		mHead = mTail = temp;
	}else{
		mTail->next = temp;
		mTail = temp;
	}
	//增加资源个数
	mSize++;

	//解锁
	unlock();

    //增加资源计数，释放等待线程
    post();

    return;

put_err:

	//解锁
	unlock();

}

void* UQueue::get(bool flag){

	av_link temp = NULL;

	if(flag)wait();
	//控制线程需要退出
	if(mAbort)return NULL;

	//上锁
	lock();

	if(!flag){
		if(mSem.trywait()){
			goto get_err;
		}
	}
    //从对首返回资源
	if( 0 == mSize ){
		goto get_err;
	}
    if(!mHead){
    	temp = NULL;
		ulog_err("UQueue::flush_get:mHead == NULL,mSize=%d",mSize);
		goto get_err;
    }else if(mHead == mTail){
    	temp = mHead;
    	mHead = mTail = NULL;
    }else{
    	temp = mHead;
    	mHead = mHead->next;
    	if(!temp){
    		ulog_err("UQueue::flush_get:temp == NULL");
    	}
    }
    //减少资源个数
    mSize--;

    //解锁
    unlock();

    return temp;
get_err:
    //解锁
    unlock();

    return temp;
}

void UQueue::flush(){

	av_link keyframe = NULL,head = NULL;

	//控制线程需要退出
	if(mAbort)return;

	//上锁
	lock();

    if(mHead){

    	head = mHead;
    	while(head != mTail){
    		if(UPLAYER_PKT_KEY_FRAME_FLAG == head->flag){
    			keyframe = head;
    			break;
    		}
    		head = head->next;
    	}
    	if(keyframe){
            // 标记最后一个需要丢弃的包, by weixinghua
            av_link last_flush = NULL;
    		head = mHead;
    		while(head != keyframe){
    			if(!head->flag){
                    head->flag = UPLAYER_VPACKET_FLUSH_FLAG;
                    last_flush = head;
    			}
    			head = head->next;
    		}
            
            if(last_flush)
                last_flush->flag |= UPLAYER_VPACKET_FLUSH_LAST;
    	}
    }

    //解锁
    unlock();

    return;
}
void UQueue::abort(){

	//设置终止标志
    mAbort = true;
    //释放等待控制线程
    post();
}

void UQueue::flush(UQueue *packet_queue,UQueue *slot_queue){
    
    //参数判断
    if(!packet_queue || !slot_queue)return;
    
    //参数判断
    if((UQUEUE_TYPE_PACKET == packet_queue->mType || \
        UQUQUE_TYPE_AUDIO_DATA == packet_queue->mType || \
        UQUQUE_TYPE_VIDEO_DATA == packet_queue->mType ) \
       && UQUEUE_TYPE_SLOT == slot_queue->mType){
        
        //清空包队列或数据队列中的资源，将空槽资源还回空槽队列
        
        while(packet_queue->size() > 0)slot_queue->put(packet_queue->get(false));
    }
    
}
void UQueue::lock(){
	mLock.lock();
}
void UQueue::unlock(){
	mLock.unlock();
}
void UQueue::wait(){
    //Down信号量
	mSem.wait();
}
void UQueue::post(){
    //Up信号量
    mSem.post();
}

