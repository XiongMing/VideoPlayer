/** @file  ugraphics.h
  *	@brief 用于实现播放器的绘图接口，调用OpenGL ES实现屏幕绘图
  * @author  YanJia
  * @date  2012-3-31
  * @version	1.0
  * @note
  * 工具类，同一个线程只能拥有一个Graphics实例，同一实例方法的调用必须在同一线程内
*/

#ifndef FFMPEG_GRAPHICS_H
#define FFMPEG_GRAPHICS_H


#include "ucommon.h"
#include "uplayer.h"

#if PLATFORM_DEF != IOS_PLATFORM
#include "jniUtils.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#else
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#include "ulog.h"
extern "C" {
#include "libavcodec/avcodec.h"
}
/**
  * @class  IGraphics ugraphics.h
  * @brief  公开给客户端的绘图接口
  * @author YanJia
  * @note
  * 只公开1个接口UpdateWindow给客户端，方便使用，以后根据需要可以开发更多接口
*/
class IGraphics{

protected:

	IGraphics(){}

public:

	/**
	  * @brief  用户绘图时更新画面显示的接口
	  * @author  YanJia
	  * @param[in]  pData  绘图数据，一般为yuv420 planner格式,支持rgb565,yuv444packed
	  * @return  void
	*/

	virtual void UpdateWindow(void * pData) = 0;
	virtual ~IGraphics(){}
};


class IEGL{
public:
    
#if PLATFORM_DEF != IOS_PLATFORM
	virtual bool InitEGL() = 0;
#else
    virtual bool InitEGL(int with, int height) = 0;
#endif

	virtual void ReleaseEGL() = 0;

	virtual void UpdateSurface() = 0;

	virtual void UpdateSize(int *width,int *height) = 0;
};


/** @brief  用于标识UGraphics的内部状态 */
enum{
	UGRAPHICS_STATE_OK	= 0,	/**状态正常*/
	UGRAPHICS_STATE_ERROR,		/**出错状态*/
};
/** @brief  用于标识解码后传递给OpenGL显示时的数据格式 */
enum{
	UGRAPHICS_FMT_NONE = 0,		/**格式枚举开始*/
	UGRAPHICS_FMT_YUV444_PACKED,/**YUV444packed格式数据*/
	UGRAPHICS_FMT_YUV420_PLANAR,/**YUV420planar格式数据*/
	UGRAPHICS_FMT_RGB565,		/**RGB565格式数据*/
	UGRAPHICS_FMT_MAX,			/**格式枚举最大值*/
};

//add by yujunfeng
/** @brief  初始化时视频源的方向，0为正向，1为左转90°，2为右转90°，3为翻转 */
enum{
	VIDEO_ORIENTARION_NORM = 0,
	VIDEO_ORIENTARION_LEFT90 ,
	VIDEO_ORIENTARION_RIGHT90 ,
	VIDEO_ORIENTARION_LEFT180 ,
};
/**
  * @class  UGraphics ugraphics.h
  * @brief  用OPenGL ES 2.0实现的绘图类
  * @author YanJia
  * @note
  * 用于Uplayer第一期版本的绘图类，可以实现其他的绘图类，但必须继承自IGraphics接口，用于实现封装
*/
class UGraphics : public IGraphics{

public:
	/**
	  * @brief  UGraphics 构造函数
	  * @author  YanJia
	  * @param[in]  pWin  本地窗口对象指针
	  * @param[in]  videoWidth  绘图窗口宽，单位是像素
	  * @param[in]  videoHeight  绘图窗口高，单位是像素
	  * @param[in]  mode  绘图模式，yuv420planar,yuv444packed,rgb565三种
	*/

	UGraphics(UPlayer* player,IEGL* egl,int videoWidth,int videoHeight,int mode = UGRAPHICS_FMT_YUV420_PLANAR);

	~UGraphics();
public:
	/**
	  * @brief  用户绘图时更新画面显示的对象方法
	  * @author  YanJia
	  * @param[in]  pData  绘图数据，一般为yuv420 planner格式,支持rgb565,yuv444packed
	  * @return  void
	*/
	void UpdateWindow(void * pData);
	/**
	  * @brief  设置视频源旋转方向
	  * @author  YuJunfeng
	  * @param[in]  videoOrientation  视频朝向
	  * @return  void
	*/
private:
	/**
	  * @brief  用于初始化内部状态的函数
	  * @author  YanJia
	*/
	void init();	//用于初始化变量
	//内部helper函数
private:

	/**
	  * @brief  创建并编译着色器
	  * @author  YanJia
	  * @param[in]  type  着色器类型GL_VERTEX_SHADER，GL_FRAGMENT_SHADER
	  * @param[in]  shaderSrc  着色器源代码
	  * @return  返回着色器对象句柄
	*/
	static GLuint LoadShader(GLenum type, const char* shaderSrc);

	/**
	  * @brief  创建程序对象，关联着色器,链接并激活程序对象
	  * @author  YanJia
	  * @param[in]  vshader  顶点着色器句柄
	  * @param[in]  fshader  片段着色器句柄
	  * @return  成功返回程序对象句柄，失败返回0
	*/
	static GLuint CreateProgram(GLuint vshader, GLuint fshader);
	static unsigned clz (unsigned x);
	static int GetAlignedSize(unsigned size);


	/**
	  * @brief  加载顶点着色器属性,直接渲染YUV444packed模式
	  * @author  YanJia
	  * @return  成功返回1，失败返回0
	*/
	int LoadShaderData();


	/**
	  * @brief  加载顶点着色器属性,直接渲染YUV420planar模式
	  * @author  YanJia
	  * @return  成功返回1，失败返回0
	*/
	int LoadShaderDataYUV420(int videoOrientation);
	/**
	  * @brief  加载顶点着色器属性,直接渲染rgb565模式
	  * @author  YanJia
	  * @return  成功返回1，失败返回0
	*/
	int LoadShaderDataRGB565();


	/**
	  * @brief  动态调整显示尺寸
	  * @author  YanJia
	  * @return  void
	*/
	void onSizeChange();

	//以下是需要释放的绘图对象
private:

	IEGL		*mEGL;

	//EG对象
	/**
	* @brief  程序对象
	*/
	GLuint		mGlProgram;
	/**
	* @brief  顶点着色器
	*/
	GLuint		mGlVShader;
	/**
	* @brief  片段着色器
	*/
	GLuint		mGlFShader;
	/**
	* @brief  顶点缓冲对象
	*/
	GLuint		mVertexBuffer;
	/**
	* @brief  索引缓冲对象
	*/
	GLuint		mIndicesBuffer;
	/**
	* @brief  材质对象
	*/
	GLuint 		mTexture;

	//直接使用yuv420planar格式的数据渲染
	/**
	* @brief  Y分量
	*/
	GLuint		mTextureY;
	/**
	* @brief  U分量
	*/
	GLuint		mTextureU;
	/**
	* @brief  V分量
	*/
	GLuint		mTextureV;

	//内部参数
private:
#if PLATFORM_DEF != IOS_PLATFORM

	JNIEnv	* 	mJNIEnv;

#endif
	/**
	* @brief  解码出来的宽度
	*/
	int mVideoWidth;
	/**
	* @brief  解码出来的高度
	*/
	int mVideoHeight;
	/**
	* @brief  渲染宽度
	*/
	int	mRenderWidth;
	/**
	* @brief  渲染高度
	*/
	int mRenderHeight;
	/**
	* @brief  渲染宽度
	*/
	int	mSurfaceWidth;
	/**
	* @brief  渲染高度
	*/
	int mSurfaceHeight;
	/**
	* @brief  U分量偏移
	*/
	int mUOffset;
	/**
	* @brief  V分量偏移
	*/
	int mVOffset;
	/**
	* @brief 绘图区索引
	*/
	GLubyte mIndices[6];
	/**
	* @brief  当前内部状态
	*/
	int	mState;
	/**
	* @brief  渲染模式
	*/
	int mMode;

	//解码相关的参数
private:
	/**
	* @brief  YUV444时分配的内部内存大小
	*/
	int	mSizePerImage;
	/**
	* @brief  YUV444时分配的内部内存指针
	*/
	unsigned char*	mPixelsYUV444Packetd;
	/**
	* @brief  播放器对象
	*/
	UPlayer* mPlayer;
	/**
	* @brief  原始的视频源方向
	*/
	int oldVideoOrientation;

};

#endif
