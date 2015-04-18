/** @file  uparser.h
  *	@brief 用于实现拆包线程
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/

#ifndef UPASSER_H_
#define UPASSER_H_

#include "ucommon.h"
#include "uthread.h"
#include "uqueue.h"
#include "uplayer.h"
#include <list>
#include <iostream>

using namespace std;


extern "C" {

#include "libavformat/avformat.h"

}
/**
  * @class  UParser uparser.h
  * @brief  播放器拆包线程类
  * @author YanJia
  * @note
  * 继承自UThread类
*/
class UParser : public UThread{

public:
	/**
	  * @brief  拆包线程对象构造函数
	  * @author  YanJia
	  * @param[in]  name 线程名
	  * @param[in]  player 播放器对象指针
	*/
	UParser(char* name,UPlayer* player);
	/**
	  * @brief  拆包线程对象析构函数
	  * @author  YanJia
	*/
	~UParser();

public:

	/**
	  * @brief  停止线程
	  * @author  YanJia
	*/
	void						stop();
private:
	/**
	  * @brief  拆包线程
	  * @author  YanJia
	*/
    void						parse();
	/**
	  * @brief  seek功能
	  * @author  YanJia
	*/
    int						seek();

private:
	/**
	  * @brief  线程处理函数
	  * @author  YanJia
	*/
    void           				handleRun();
private:
	/**
	  * @brief  播放器对象指针
	  * @author  YanJia
	*/
    UPlayer*					mPlayer;

    /**
     * @brief  是否需要发送播放结束通知, add by weixinghua
     */
    bool				mNeedSendingPlayEndCode;
};


#endif /* UPASSER_H_ */
