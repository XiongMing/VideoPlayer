#include "ugraphics.h"
#include <stdlib.h>
#include "ulog.h"


//yanjia 调试用途
#define YANJIA_ENABLE_GRAPHICS 1
#define YANJIA_ENABLE_FREE_GRAPHICS 1
#define YANJIA_ENABLE_UPDATE_WINDOW 1
#define YANJIA_ENABLE_LOADING_SHADER_DATA 1
#define YANJIA_ENABLE_PROGRAM 1
#define YANJIA_SHOW_SAME_FRAME 0

#if YANJIA_SHOW_SAME_FRAME
//yanjia 测试
FILE* yanjia_file = NULL;
yanjia_file = fopen("/sdcard/Movies/video.dat","r");
if(yanjia_file) {
	if(fread(mPixelsYUV444Packetd,mPixelsPerImage,1,yanjia_file)) {
		LOG_PRINTF(ANDROID_LOG_ERROR, YANJIA_TAG, "mPixelsYUV444Packetd loaded");
	} else {
		LOG_PRINTF(ANDROID_LOG_ERROR, YANJIA_TAG, "mPixelsYUV444Packetd load failed");
	}
	if(yanjia_file)fclose(yanjia_file);
	yanjia_file = NULL;
} else {
	LOG_PRINTF(ANDROID_LOG_ERROR, YANJIA_TAG, "open file failed");
}
#endif
static const char vshader_src[] =
///******** triangle shader vertex *********/
		"attribute vec4 a_position; \n"
				"attribute vec2 a_tex_coord_in; \n"
				"varying vec2 v_tex_coord_out; \n"
				"varying vec4 v_color; \n"
				"void main() {"
				" gl_Position = a_position; \n"
				" v_tex_coord_out = a_tex_coord_in; \n"
				"} \n";

static const char fshader_src[] =

"precision mediump float; \n"

		"varying lowp vec2 v_tex_coord_out; \n"

		"uniform sampler2D u_texture_x; \n"

		"void main() { \n"
		"mat3 test = mat3(1, 1.4075, 0, 1, -0.3455, -0.7169, 1, 1.779, 0); \n"
		"vec4 texX = texture2D(u_texture_x, v_tex_coord_out); \n"
		"vec3 change = vec3(texX.x, texX.y-0.5, texX.z-0.5); \n"

		"vec3 temp =  change * test; \n"

		"gl_FragColor = vec4(temp, 1.0); \n"

		"} \n";

static const char fshader_yuv420planar_src[] =

		"precision mediump float; \n"

				"varying lowp vec2 v_tex_coord_out; \n"

				"uniform sampler2D u_texture_y; \n"
				"uniform sampler2D u_texture_u; \n"
				"uniform sampler2D u_texture_v; \n"

				"void main() { \n"
				"mat3 yuv2rgb = mat3(1, 0, 1.5958, 1, -0.39173, -0.81290, 1, 2.017, 0); \n"
				//"mat3 yuv2rgb = mat3(1, 0, 1.2802, 1, -0.214821, -0.380589, 1, 2.127982, 0); \n"

				"vec3 yuv = vec3(1.1643 * (texture2D(u_texture_y, v_tex_coord_out).r - 0.0625),"
				"texture2D(u_texture_u,v_tex_coord_out).r - 0.5,"
				"texture2D(u_texture_v,v_tex_coord_out).r - 0.5); \n"

				"vec3 rgb = yuv * yuv2rgb; \n"

				"gl_FragColor = vec4(rgb, 1.0); \n"

				"} \n";
static const char fshader_yuv420planar_src2[] =

		"precision mediump float; \n"

				"varying lowp vec2 v_tex_coord_out; \n"

				"uniform sampler2D u_texture_y; \n"
				"uniform sampler2D u_texture_u; \n"
				"uniform sampler2D u_texture_v; \n"

				"void main() { \n"
				"mat3 yuv2rgb = mat3(1, 0, 1.5958, 1, -0.39173, -0.81290, 1, 2.017, 0); \n"
				//"mat3 yuv2rgb = mat3(1, 0, 1.2802, 1, -0.214821, -0.380589, 1, 2.127982, 0); \n"

				"float y = texture2D(u_texture_y, v_tex_coord_out).r; \n"
				"float u = texture2D(u_texture_u, v_tex_coord_out).r; \n"
				"float v = texture2D(u_texture_v, v_tex_coord_out).r; \n"

				"vec3 rgb = vec3(y + 1.4075*(v-0.5),y - 0.3455 * (u -0.5) - 0.7169 * (v -0.5), y + 1.779 * (u -0.5)); \n"

				"gl_FragColor = vec4(rgb, 1.0); \n"

				"} \n";
static const char fshader_rgb565_src[] =
/***********triangle shader fragment********/

"precision mediump float; \n"
		"varying lowp vec2 v_tex_coord_out; \n"
		"uniform sampler2D u_texture_x; \n"
		"void main() { \n"
		"gl_FragColor = vec4(texture2D(u_texture_x, v_tex_coord_out)); \n"
		"} \n";

typedef struct {
	float Position[3];
	float TexCoord[2];
} Vertex;

int UGraphics::LoadShaderDataRGB565() {

	if (!mGlProgram)
		return 0;

	//顶点和纹理坐标
	Vertex vertices[] = { { { 1.0f, -1.0f, 0.0f }, { 1, 1 } }, { { 1.0f, 1.0f,
			0.0f }, { 1, 0 } }, { { -1.0f, 1.0f, 0.0f }, { 0, 0 } }, { { -1.0f,
			-1.0f, 0.0f }, { 0, 1 } } };
	//绘图区域
	GLubyte indices[] = { 0, 1, 2, 2, 3, 0 };
	memcpy(mIndices, indices, sizeof(indices));

	vertices[0].TexCoord[0] = vertices[1].TexCoord[0] = mVideoWidth * 1.0
			/ mRenderWidth;
	vertices[0].TexCoord[1] = vertices[3].TexCoord[1] = mVideoHeight * 1.0
			/ mRenderHeight;

	//缓存顶点数据
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//获得着色器属性
	GLuint a_position = glGetAttribLocation(mGlProgram, "a_position");
	GLuint a_tex_coord_in = glGetAttribLocation(mGlProgram, "a_tex_coord_in");

	//激活
	glEnableVertexAttribArray(a_position);
	glEnableVertexAttribArray(a_tex_coord_in);

	//加载数据
	glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), 0);
	glVertexAttribPointer(a_tex_coord_in, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex),
			(GLvoid*) (sizeof(float) * 3));

	//缓存索引
	glGenBuffers(1, &mIndicesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
			GL_STATIC_DRAW);

	//创建材质
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &mTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	//获得采样器
	GLuint u_texture_x = glGetUniformLocation(mGlProgram, "u_texture_x");

	//关联采样器到材质
	glUniform1i(u_texture_x, 0);

	//设置材质缩放和重复模式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mRenderWidth, mRenderHeight, 0,
			GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

	return 1;
}
int UGraphics::LoadShaderData() {

	if (!mGlProgram)
		return 0;

	//顶点和纹理坐标
	Vertex vertices[] = { { { 1.0f, -1.0f, 0.0f }, { 1, 1 } }, { { 1.0f, 1.0f,
			0.0f }, { 1, 0 } }, { { -1.0f, 1.0f, 0.0f }, { 0, 0 } }, { { -1.0f,
			-1.0f, 0.0f }, { 0, 1 } } };
	//绘图区域
	GLubyte indices[] = { 0, 1, 2, 2, 3, 0 };
	memcpy(mIndices, indices, sizeof(indices));

	vertices[0].TexCoord[0] = vertices[1].TexCoord[0] = mVideoWidth * 1.0
			/ mRenderWidth;
	vertices[0].TexCoord[1] = vertices[3].TexCoord[1] = mVideoHeight * 1.0
			/ mRenderHeight;

	//缓存顶点数据
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//获得着色器属性
	GLuint a_position = glGetAttribLocation(mGlProgram, "a_position");
	GLuint a_tex_coord_in = glGetAttribLocation(mGlProgram, "a_tex_coord_in");

	//激活
	glEnableVertexAttribArray(a_position);
	glEnableVertexAttribArray(a_tex_coord_in);

	//加载数据
	glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), 0);
	glVertexAttribPointer(a_tex_coord_in, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex),
			(GLvoid*) (sizeof(float) * 3));

	//缓存索引
	glGenBuffers(1, &mIndicesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
			GL_STATIC_DRAW);

	//创建材质
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &mTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	//获得采样器
	GLuint u_texture_x = glGetUniformLocation(mGlProgram, "u_texture_x");

	//关联采样器到材质
	glUniform1i(u_texture_x, 0);

	//设置材质缩放和重复模式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mRenderWidth, mRenderHeight, 0,
			GL_RGB, GL_UNSIGNED_BYTE, NULL);

	return 1;

}
int UGraphics::LoadShaderDataYUV420(int videoOrientation) {

#if YANJIA_ENABLE_LOADING_SHADER_DATA

	if (!mGlProgram)
		return 0;

	//顶点和纹理坐标
	Vertex vertices[] = { { { 1.0f, -1.0f, 0.0f }, { 1, 1 } }, { { 1.0f, 1.0f,
			0.0f }, { 1, 0 } }, { { -1.0f, 1.0f, 0.0f }, { 0, 0 } }, { { -1.0f,
			-1.0f, 0.0f }, { 0, 1 } } };

	//yujunfeng
	//根据设置的视频播放方向调整视频朝向，默认是正向

	switch (videoOrientation) {
	case VIDEO_ORIENTARION_NORM:
		/*	vertices = {
		 //正向
		 {{1.0f, -1.0f, 0.0f},{1,1}},
		 {{1.0f, 1.0f, 0.0f}, {1,0}},
		 {{-1.0f, 1.0f, 0.0f}, {0,0}},
		 {{-1.0f, -1.0f, 0.0f},{0,1}}
		 };*/
		vertices[0] = { {1.0f, -1.0f, 0.0f}, {1,1}};
		vertices[1] = { {1.0f, 1.0f, 0.0f}, {1,0}};
		vertices[2] = { {-1.0f, 1.0f, 0.0f}, {0,0}};
		vertices[3] = { {-1.0f, -1.0f, 0.0f}, {0,1}};
		break;
		case VIDEO_ORIENTARION_LEFT90:
		/*	vertices = {
		 //逆90°
		 {{1.0f, -1.0f, 0.0f},{0,1}},
		 {{1.0f, 1.0f, 0.0f}, {1,1}},
		 {{-1.0f, 1.0f, 0.0f}, {1,0}},
		 {{-1.0f, -1.0f, 0.0f},{0,0}}
		 };*/
		vertices[0] = { {1.0f, -1.0f, 0.0f}, {0,1}};
		vertices[1] = { {1.0f, 1.0f, 0.0f}, {1,1}};
		vertices[2] = { {-1.0f, 1.0f, 0.0f}, {1,0}};
		vertices[3] = { {-1.0f, -1.0f, 0.0f}, {0,0}};
		break;
		case VIDEO_ORIENTARION_RIGHT90:
		/*	vertices = {
		 //顺90°
		 {{1.0f, -1.0f, 0.0f},{1,0}},
		 {{1.0f, 1.0f, 0.0f}, {0,0}},
		 {{-1.0f, 1.0f, 0.0f}, {0,1}},
		 {{-1.0f, -1.0f, 0.0f},{1,1}}
		 };*/
		vertices[0] = { {1.0f, -1.0f, 0.0f}, {1,0}};
		vertices[1] = { {1.0f, 1.0f, 0.0f}, {0,0}};
		vertices[2] = { {-1.0f, 1.0f, 0.0f}, {0,1}};
		vertices[3] = { {-1.0f, -1.0f, 0.0f}, {1,1}};
		break;
		case VIDEO_ORIENTARION_LEFT180:
		/*	vertices = {
		 //反180°
		 {{1.0f, -1.0f, 0.0f},{0,0}},
		 {{1.0f, 1.0f, 0.0f}, {0,1}},
		 {{-1.0f, 1.0f, 0.0f}, {1,1}},
		 {{-1.0f, -1.0f, 0.0f},{1,0}}
		 };*/
		vertices[0] = { {1.0f, -1.0f, 0.0f}, {0,0}};
		vertices[1] = { {1.0f, 1.0f, 0.0f}, {0,1}};
		vertices[2] = { {-1.0f, 1.0f, 0.0f}, {1,1}};
		vertices[3] = { {-1.0f, -1.0f, 0.0f}, {1,0}};
		break;
		default:
		break;
	}

	//绘图区域
	GLubyte indices[] = { 0, 1, 2, 2, 3, 0 };
	memcpy(mIndices, indices, sizeof(indices));

	switch (videoOrientation) {
	case VIDEO_ORIENTARION_NORM:
		//正向
		vertices[0].TexCoord[0] = vertices[1].TexCoord[0] = mVideoWidth * 1.0
				/ mRenderWidth;
		vertices[0].TexCoord[1] = vertices[3].TexCoord[1] = mVideoHeight * 1.0
				/ mRenderHeight;
		break;
	case VIDEO_ORIENTARION_LEFT90:
		//逆90°
		vertices[1].TexCoord[0] = vertices[2].TexCoord[0] = mVideoWidth * 1.0
				/ mRenderWidth;
		vertices[0].TexCoord[1] = vertices[1].TexCoord[1] = mVideoHeight * 1.0
				/ mRenderHeight;
		break;
	case VIDEO_ORIENTARION_RIGHT90:
		//顺90°
		vertices[0].TexCoord[0] = vertices[3].TexCoord[0] = mVideoWidth * 1.0
				/ mRenderWidth;
		vertices[2].TexCoord[1] = vertices[3].TexCoord[1] = mVideoHeight * 1.0
				/ mRenderHeight;
		break;
	case VIDEO_ORIENTARION_LEFT180:
		//反180°
		vertices[2].TexCoord[0] = vertices[3].TexCoord[0] = mVideoWidth * 1.0
				/ mRenderWidth;
		vertices[1].TexCoord[1] = vertices[2].TexCoord[1] = mVideoHeight * 1.0
				/ mRenderHeight;
		break;
	default:
		break;
	}

	//缓存顶点数据
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//获得着色器属性
	GLuint a_position = glGetAttribLocation(mGlProgram, "a_position");
	GLuint a_tex_coord_in = glGetAttribLocation(mGlProgram, "a_tex_coord_in");

	//激活
	glEnableVertexAttribArray(a_position);
	glEnableVertexAttribArray(a_tex_coord_in);

	//加载数据
	glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), 0);
	glVertexAttribPointer(a_tex_coord_in, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex),
			(GLvoid*) (sizeof(float) * 3));

	//缓存索引
	glGenBuffers(1, &mIndicesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
			GL_STATIC_DRAW);

	//创建材质Y分量
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &mTextureY);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTextureY);

	//获得采样器
	GLuint u_texture_y = glGetUniformLocation(mGlProgram, "u_texture_y");

	//关联采样器到材质
	glUniform1i(u_texture_y, 0);

	//设置材质缩放和重复模式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mRenderWidth, mRenderHeight, 0,
			GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

	//创建材质U分量
	glGenTextures(1, &mTextureU);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mTextureU);

	//获得采样器
	GLuint u_texture_u = glGetUniformLocation(mGlProgram, "u_texture_u");

	//关联采样器到材质
	glUniform1i(u_texture_u, 1);

	//设置材质缩放和重复模式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mRenderWidth >> 1,
			mRenderHeight >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

	//创建材质V分量
	glGenTextures(1, &mTextureV);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mTextureV);

	//获得采样器
	GLuint u_texture_v = glGetUniformLocation(mGlProgram, "u_texture_v");

	//关联采样器到材质
	glUniform1i(u_texture_v, 2);

	//设置材质缩放和重复模式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mRenderWidth >> 1,
			mRenderHeight >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

	glActiveTexture(GL_TEXTURE0);

#endif

	return 1;
}
unsigned UGraphics::clz(unsigned x) {
	unsigned i = sizeof(x) * 8;
	while (x) {
		x = x >> 1;
		i--;
	}
	return i;
}
int UGraphics::GetAlignedSize(unsigned size) {

	/* Return the smallest larger or equal power of 2 */
	unsigned align = 1 << (8 * sizeof(unsigned) - clz(size));
	return ((align >> 1) == size) ? size : align;
}
GLuint UGraphics::LoadShader(GLenum type, const char* shaderSrc) {

	//创建着色器
	GLuint shader = glCreateShader(type);

	if (!shader) {
		empty_log(LOG_ERROR,"glCreateShader failed");
		return 0;
	}
	//加载着色器
	glShaderSource(shader, 1, &shaderSrc, NULL);

	//编译着色器
	glCompileShader(shader);

	glReleaseShaderCompiler();

	//错误处理
	GLint compiled;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled) {
		empty_log(LOG_ERROR,"glCompileShader failed");
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint UGraphics::CreateProgram(GLuint vshader, GLuint fshader) {

	if (!vshader || !fshader) {
		empty_log(LOG_ERROR,"CreateProgram param error");
		return 0;
	}

	//创建程序对象
	GLuint program = glCreateProgram();

	if (!program) {
		empty_log(LOG_ERROR,"glCreateProgram failed");
		return 0;
	}

	//关联着色器
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	//连接程序
	GLint linked;
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if (!linked) {
		empty_log(LOG_ERROR,"glLinkProgram failed");
		glDeleteProgram(program);
		return 0;
	}

	glUseProgram(program);

	return program;
}
void UGraphics::init() {

#if PLATFORM_DEF == ANDROID_PLATFORM
	mJNIEnv = NULL;
#endif

	mGlProgram = 0;
	mGlVShader = 0;
	mGlFShader = 0;
	mVertexBuffer = 0;
	mIndicesBuffer = 0;
	mTexture = 0;
	mTextureY = 0;
	mTextureU = 0;
	mTextureV = 0;

	mVideoWidth = 0;
	mVideoHeight = 0;

	mRenderWidth = 0;
	mRenderHeight = 0;

	mUOffset = 0;
	mVOffset = 0;

	bzero(mIndices, sizeof(mIndices));

	mState = UGRAPHICS_STATE_OK;
	mMode = UGRAPHICS_FMT_NONE;

	mSizePerImage = 0;
	mPixelsYUV444Packetd = NULL;
}

UGraphics::UGraphics(UPlayer* player, IEGL* egl, int videoWidth,
		int videoHeight, int mode) {

#if YANJIA_ENABLE_GRAPHICS
	init();
	empty_log(LOG_ERROR,"UGraphics enter");

	if (!egl) {
		mState = UGRAPHICS_STATE_ERROR;
		empty_log(LOG_ERROR,"UGraphics:egl == NULL");
		return;
	}
	mEGL = egl;

	oldVideoOrientation = VIDEO_ORIENTARION_NORM;
	mPlayer = player;
	//检测图像大小
	if (!videoWidth || !videoHeight) {
		mState = UGRAPHICS_STATE_ERROR;
		empty_log(LOG_ERROR,"!videoWidth || !videoHeight");
		return;
	}
	this->mVideoWidth = videoWidth;
	this->mVideoHeight = videoHeight;
	this->mRenderWidth = GetAlignedSize(videoWidth);
	this->mRenderHeight = GetAlignedSize(videoHeight);

	//检测渲染模式
	if (mode > UGRAPHICS_FMT_NONE && mode < UGRAPHICS_FMT_MAX)
		mMode = mode;
	else {
		mMode = UGRAPHICS_FMT_NONE;
		mState = UGRAPHICS_STATE_ERROR;
		empty_log(LOG_ERROR,"UGRAPHICS_FMT_NONE");
		return;
	}

	if (UGRAPHICS_FMT_YUV420_PLANAR == mMode) {
		mUOffset = this->mVideoWidth * this->mVideoHeight;
		mVOffset = ((this->mVideoWidth * this->mVideoHeight) * 5) >> 2;
	}

	mSizePerImage = avpicture_get_size(PIX_FMT_YUV444P, mVideoWidth,
			mVideoHeight);
	mPixelsYUV444Packetd = (unsigned char*) malloc(
			mSizePerImage * sizeof(unsigned char));
	if (!mPixelsYUV444Packetd) {
		mState = UGRAPHICS_STATE_ERROR;
		empty_log(LOG_ERROR,"malloc mPixelsYUV444Packetd failed");
		return;
	}

	//绑定API版本
	/*
	 if(!eglBindAPI(EGL_OPENGL_ES_API)){
	 mState = UGRAPHICS_STATE_ERROR;
	 empty_log(LOG_ERROR,"eglBindAPI failed");
	 return;
	 }
	 */

    
#if PLATFORM_DEF == IOS_PLATFORM
    if (!mEGL->InitEGL(videoWidth, videoHeight)) {
#else
        if (!mEGL->InitEGL()) {
#endif
		mState = UGRAPHICS_STATE_ERROR;
		ulog(ULOG_ERR, "InitEGL failed");
		set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_INITI_EGL_FAILED);
		mPlayer->notifyMsg(MEDIA_INFO_PREPARE_ERROR,ERROR_SOFT_PLAYER_INITI_EGL_FAILED);
		return;
	}
	mEGL->UpdateSize(&mSurfaceWidth, &mSurfaceHeight);

	//错误检测
	if (0 == mSurfaceWidth || 0 == mSurfaceHeight) {
		mState = UGRAPHICS_STATE_ERROR;
		ulog_err("mSurfaceWidth == 0 || mSurfaceHeight == 0");
		return;
	}

	ulog_info(
			"mSurfaceWidth == %d || mSurfaceHeight == %d", mSurfaceWidth, mSurfaceHeight);


#if YANJIA_ENABLE_PROGRAM
	//加载顶点着色器
	mGlVShader = LoadShader(GL_VERTEX_SHADER, vshader_src);

	if (!mGlVShader) {
		mState = UGRAPHICS_STATE_ERROR;
		set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
		mPlayer->notifyMsg(MEDIA_INFO_PREPARE_ERROR,ERROR_SOFT_PLAYER_OPENSLES);
		empty_log(LOG_ERROR,"LoadShader failed");
		return;
	}
	//加载片段着色器
	if (UGRAPHICS_FMT_YUV444_PACKED == mMode) {
		empty_log(LOG_ERROR,"load fshader444");
		mGlFShader = LoadShader(GL_FRAGMENT_SHADER, fshader_src);
	} else if (UGRAPHICS_FMT_YUV420_PLANAR == mMode) {
		empty_log(LOG_ERROR,"load fshader420");
		mGlFShader = LoadShader(GL_FRAGMENT_SHADER, fshader_yuv420planar_src);
	} else if (UGRAPHICS_FMT_RGB565 == mMode) {
		empty_log(LOG_ERROR,"load fshader565");
		mGlFShader = LoadShader(GL_FRAGMENT_SHADER, fshader_rgb565_src);
	} else {
		mState = UGRAPHICS_STATE_ERROR;
		empty_log(LOG_ERROR,"UGRAPHICS_FMT_NONE == mMode");
		return;
	}

	if (!mGlFShader) {
		mState = UGRAPHICS_STATE_ERROR;
		set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
		mPlayer->notifyMsg(MEDIA_INFO_PREPARE_ERROR,ERROR_SOFT_PLAYER_OPENSLES);
		empty_log(LOG_ERROR,"LoadShader failed");
		return;
	}

	//创建程序对象
	if (!(mGlProgram = CreateProgram(mGlVShader, mGlFShader))) {
		mState = UGRAPHICS_STATE_ERROR;
		set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
		mPlayer->notifyMsg(MEDIA_INFO_PREPARE_ERROR,ERROR_SOFT_PLAYER_OPENSLES);
		empty_log(LOG_ERROR,"CreateProgram failed");
		return;
	}

	//加载顶点着色器属性
	if (UGRAPHICS_FMT_YUV444_PACKED == mMode) {
		if (!LoadShaderData()) {
			mState = UGRAPHICS_STATE_ERROR;
			empty_log(LOG_ERROR,"LoadShaderData failed");
			return;
		}
	} else if (UGRAPHICS_FMT_YUV420_PLANAR == mMode) {

		if (!LoadShaderDataYUV420(VIDEO_ORIENTARION_NORM)) {
			mState = UGRAPHICS_STATE_ERROR;
			set_player_error_code(mPlayer,ERROR_SOFT_PLAYER_OPENSLES);
			mPlayer->notifyMsg(MEDIA_INFO_PREPARE_ERROR,ERROR_SOFT_PLAYER_OPENSLES);
			empty_log(LOG_ERROR,"LoadShaderDataYUV420 failed");
			return;
		}
	} else if (UGRAPHICS_FMT_RGB565 == mMode) {
		if (!LoadShaderDataRGB565()) {
			mState = UGRAPHICS_STATE_ERROR;
			empty_log(LOG_ERROR,"LoadShaderDataRGB565 failed");
			return;
		}
	} else {
		mState = UGRAPHICS_STATE_ERROR;
		empty_log(LOG_ERROR,"UGRAPHICS_FMT_NONE == mMode");
		return;
	}
#endif

	//准备绘图区
//#if PLATFORM_DEF != IOS_PLATFORM
	glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);
//#endif

	//清屏
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//清屏操作

	mEGL->UpdateSurface();

	//设置绘图状态
	mState = UGRAPHICS_STATE_OK;

	empty_log(LOG_ERROR,"Graphics Init OK.");

#endif
}
void UGraphics::onSizeChange() {

	bool need_redraw = false;

#if EGL_GET_WIDTH_HEIGHT
	//获取绘图表面宽和高
	int surface_width,surface_height;

	mEGL->UpdateSize(&surface_width,&surface_height);


	//错误检测
	if(0 == surface_width || 0 == surface_height) {
		mState = UGRAPHICS_STATE_ERROR;
		empty_log(LOG_ERROR,"surface_width == 0 || surface_height == 0");
		return;
	}

	if(surface_width != mSurfaceWidth || surface_height != mSurfaceHeight) {
		mSurfaceWidth = surface_width;
		mSurfaceHeight = surface_height;
		need_redraw = true;
	}
#else !EGL_GET_WIDTH_HEIGHT
	if (mPlayer->mRestSize == true) {
		mSurfaceWidth = mPlayer->mNewVideoWidth;
		mSurfaceHeight = mPlayer->mNewVideoHeight;
		need_redraw = true;
	}
#endif
	if(need_redraw){
		//准备绘图区
		glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);

		//清屏
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		mPlayer->mRestSize = false;
	}
}
UGraphics::~UGraphics() {

	oldVideoOrientation = VIDEO_ORIENTARION_NORM;

#if YANJIA_ENABLE_GRAPHICS

#if YANJIA_ENABLE_FREE_GRAPHICS

	if (mPixelsYUV444Packetd)
		free(mPixelsYUV444Packetd);

	if (mGlVShader && mGlProgram) {
		glDetachShader(mGlProgram, mGlVShader);
		glDeleteShader(mGlVShader);
	}
	if (mGlFShader && mGlProgram) {
		glDetachShader(mGlProgram, mGlFShader);
		glDeleteShader(mGlFShader);
	}

	if (mGlProgram) {
		glDeleteProgram(mGlProgram);
	}

	if (mVertexBuffer) {
		glDeleteBuffers(1, &mVertexBuffer);
	}
	if (mIndicesBuffer) {
		glDeleteBuffers(1, &mIndicesBuffer);
	}

	if (UGRAPHICS_FMT_YUV444_PACKED == mMode) {
		if (mTexture)
			glDeleteTextures(1, &mTexture);
	} else if (UGRAPHICS_FMT_RGB565 == mMode) {
		if (mTexture)
			glDeleteTextures(1, &mTexture);
	} else if (UGRAPHICS_FMT_YUV420_PLANAR == mMode) {
		if (mTextureY) {
			glDeleteTextures(1, &mTextureY);
		}
		if (mTextureU) {
			glDeleteTextures(1, &mTextureU);
		}
		if (mTextureV) {
			glDeleteTextures(1, &mTextureV);
		}
	}

	mEGL->ReleaseEGL();

	empty_log(LOG_ERROR,"UGraphics free done");
#endif

#endif
}

void UGraphics::UpdateWindow(void * pData) {

#if YANJIA_ENABLE_GRAPHICS

#if YANJIA_ENABLE_UPDATE_WINDOW

	unsigned char *pixels = (unsigned char*) pData;

	if (UGRAPHICS_STATE_OK != mState)
		return;
	if(!pixels)return;
    if (mPlayer->mRestSize)
    {
        onSizeChange();
        mPlayer->mRestSize = false;
    }
	if (UGRAPHICS_FMT_YUV444_PACKED == mMode) {

		//empty_log(LOG_ERROR,"Graphics yuv444");
		// 获取YUV序列
		int m;
		int z = mSizePerImage / 3;

		for (int i = 0; i < z; i++) {
			m = i * 3;
			mPixelsYUV444Packetd[m] = pixels[i];
			mPixelsYUV444Packetd[m + 1] = pixels[i + z];
			mPixelsYUV444Packetd[m + 2] = pixels[i + z * 2];
		}

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoWidth, mVideoHeight,
				GL_RGB, GL_UNSIGNED_BYTE, mPixelsYUV444Packetd);

		mEGL->UpdateSurface();

		glFlush();
	} else if (UGRAPHICS_FMT_RGB565 == mMode) {
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoWidth, mVideoHeight,
				GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixels);

		mEGL->UpdateSurface();

		glFlush();
	} else if (UGRAPHICS_FMT_YUV420_PLANAR == mMode) {

		//empty_log(LOG_ERROR,"Drawing yuv420");

        
		glActiveTexture(GL_TEXTURE0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoWidth, mVideoHeight,
				GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);

		glActiveTexture(GL_TEXTURE1);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoWidth >> 1,
				mVideoHeight >> 1, GL_LUMINANCE, GL_UNSIGNED_BYTE,
				pixels + mUOffset);

		glActiveTexture(GL_TEXTURE2);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoWidth >> 1,
				mVideoHeight >> 1, GL_LUMINANCE, GL_UNSIGNED_BYTE,
				pixels + mVOffset);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

		mEGL->UpdateSurface();
        glFlush();

	} else {
		empty_log(LOG_ERROR,"Graphics yuv other");
	}

#endif
#endif

}
