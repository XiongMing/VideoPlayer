/** @file  utimer.h
  *	@brief 低精度的timer
  * @author  YanJia
  * @date  2013-2-25
  * @version	1.0
  * @note
*/

#ifndef UTIMER_H_
#define UTIMER_H_

#define UTIMER_MIN_PRECISION	50000

typedef void (*FNTimer)(void*);
class UTimer;
typedef UTimer utimer_t;

typedef struct utimer_node{
	utimer_t*		timer;
	utimer_node*	next;
}*utimer_link;

class UTimer{
public:
	UTimer();
	UTimer(FNTimer timer,void* data,unsigned int interval);
	~UTimer();
	void setTimer(FNTimer timer,void* data,unsigned int interval);
	void start();
	void stop();

private:

	bool isSettled();

private:
	static void*	timerThread(void* data);
	static void lock();
	static void unlock();
	static utimer_link timer_head;
	static pthread_t	thread;
	static bool			timer_thread_started;
private:

	FNTimer		mFNTimer;
	void*			mData;
	unsigned int	mInterval;
	bool			mIsRunning;
	int64_t		mLastEntryTime;

};


#endif /* UTIMER_H_ */
