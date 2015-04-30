#include "umsg_queue.h"
#include "ucommon.h"

UMsgQueue::UMsgQueue():mLock(ULOCK_TYPE_NORMAL),mSem(0){

	//初始化内部成员变量

	mAbort = false;

	mHead = mTail = NULL;

	//创建资源信号量
	return;

UMsgQueue_err:

	this->~UMsgQueue();
}

UMsgQueue::~UMsgQueue(){

	umsg_link link;

	while( link = mHead){
		mHead = mHead->next;
		free(link);
	}
}
void UMsgQueue::exit(){
	//设置终止标志
    mAbort = true;
    //释放等待控制线程
    post();
}
int UMsgQueue::recv(){

	umsg_link	temp;

	int msg;

	wait();

	if(mAbort){
		ulog_err("UMsgQueue::recv 1");
		goto recv_err;
	}
	//上锁
	lock();

	if(!mHead){
		ulog_err("UMsgQueue::recv 2");
		goto recv_err;
	}

	msg = mHead->msg;

	temp = mHead;

	if(!mHead->next){
		mHead = mTail = NULL;
	}else{
		mHead = mHead->next;
	}

	free(temp);

	//解锁
	unlock();

	return msg;

recv_err:

	//解锁
	unlock();
	return -1;
}
int UMsgQueue::tryRecv(){

	umsg_link	temp;

	int msg;

	tryWait();

	if(mAbort){
		ulog_err("UMsgQueue::recv 1");
		goto recv_err;
	}
	//上锁
	lock();

	if(!mHead){
		goto recv_err;
	}

	msg = mHead->msg;

	temp = mHead;

	if(!mHead->next){
		mHead = mTail = NULL;
	}else{
		mHead = mHead->next;
	}

	free(temp);

	//解锁
	unlock();

	return msg;

recv_err:

	//解锁
	unlock();
	return -1;
}
void UMsgQueue::send(int msg){

	umsg_link link = NULL;

	//上锁
	lock();

	link = (umsg_link) malloc(sizeof(struct umsg_node));

	if(!link){
		ulog_err("UMsgQueue::send:malloc failed");
	}
	bzero(link,sizeof(struct umsg_node));

	link->msg = msg;

	if(!mHead){
		mHead = mTail =link;
	}else{
		mTail->next = link;
		mTail = link;
	}

	//解锁
	unlock();

	post();
}
int UMsgQueue::flush(){
	while(-1 != tryRecv());
    return 1;
}
void UMsgQueue::lock(){
	mLock.lock();
}
void UMsgQueue::unlock(){
	mLock.unlock();
}
void UMsgQueue::wait(){
	//Down信号量
    mSem.wait();
}
void UMsgQueue::tryWait(){
	//Down信号量
    mSem.trywait();
}
void UMsgQueue::post(){
	//Up信号量
    mSem.post();
}
