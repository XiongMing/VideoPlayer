/** @file  udecoder.h
  *	@brief 用于实现解码线程框架
  * @author  YanJia
  * @date  2012-5-8
  * @version	1.0
  * @note
*/
#ifndef UDECODER_H_
#define UDECODER_H_

#include "uthread.h"
#include "uqueue.h"
#include "ucommon.h"
#include "uplayer.h"

extern "C" {

#include "libavcodec/avcodec.h"

}
/**
  * @class  UDecoder udecoder.h
  * @brief  解码线程基类
  * @author YanJia
  * @note
*/
class UDecoder : public UThread{

public:
	/**
	  * @brief  解码线程对象构造函数
	  * @author  YanJia
	  * @param[in]  name 线程名
	  * @param[in]  player 播放器对象指针
	*/
	UDecoder(char* name,UPlayer* player);
	/**
	  * @brief  解码线程对象析构函数
	  * @author  YanJia
	*/
	~UDecoder(){}


protected:

	/**
	  * @brief  解码线程函数接口
	  * @author  YanJia
	*/
    virtual void				decode()=0;


private:
	/**
	  * @brief  线程处理函数
	  * @author  YanJia
	*/
    void           				handleRun();

protected:
	/**
	  * @brief  播放器对象指针
	  * @author  YanJia
	*/
    UPlayer*					mPlayer;
};

#endif /* UDECODER_H_ */
