#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/file.h"
#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

typedef struct Context {
    int fd;
    int64_t end;
    int64_t pos;
    URLContext *inner;
    const char* local_buffer_path;
} Context;
const char* getLocalCachePath(URLContext *h);
static int cache_open(URLContext *h, const char *arg, int flags)
{
	int ret;

    Context *c= (Context*)h->priv_data;

    c->local_buffer_path = getLocalCachePath(h);

    if(!c->local_buffer_path){
    	c->fd = -1;
    	return -1;
    }

    av_strstart(arg, "ucache:", &arg);

    av_log(h, AV_LOG_INFO, "cache_open:local_buffer_path:%s\n",c->local_buffer_path);

    c->fd = open(c->local_buffer_path, O_RDWR |  O_CREAT | O_TRUNC, 0666);
    if (c->fd < 0){
        av_log(h, AV_LOG_ERROR, "ucache_protocol.cache_open:Failed to create tempfile\n");
        //return c->fd;
    }

    ret = ffurl_open(&c->inner, arg, flags, &h->interrupt_callback, NULL);

    if(ret && c->fd >= 0){
    	//删除文件
		close(c->fd);
		c->fd = -1;
		unlink(c->local_buffer_path);
    }
    return ret;

}

static int cache_read(URLContext *h, unsigned char *buf, int size)
{
    	Context *c= (Context*)h->priv_data;

    	int r = 0;

    	//av_log(h, AV_LOG_INFO, "cache_read enter\n",r);
        r = ffurl_read(c->inner, buf, size);
        if( r > 0 && c->fd >= 0 ){
        	int64_t size;
        	//av_log(h, AV_LOG_INFO, "cache_read = %d bytes\n",r);
            int r2= write(c->fd, buf, r);
            av_assert0(r2==r); // FIXME handle cache failure
            c->pos += r;
            c->end += r;

            //判断是否缓存完毕
            size = ffurl_seek(c->inner, 0, AVSEEK_SIZE);

            if( size == c->pos && c->end >= size ){
    		    if(c->local_buffer_path)notifyCacheEnd(h,1);
    			av_log(h, AV_LOG_INFO, "ucache: ok1\n");
    			close(c->fd);
    			c->fd = -1;
            }

        }else{
        	if(c->fd >=0)av_log(h, AV_LOG_ERROR, "cache_read:ffurl_read failed\n",r);
        }
        return r;

}

static int64_t cache_seek(URLContext *h, int64_t pos, int whence)
{
    Context *c= (Context*)h->priv_data;

    int64_t ret;

    if(AVSEEK_SIZE != whence){
    	av_log(h, AV_LOG_ERROR, "c->inner->filename:%s,ucache seek!!!:pos=%lld,c->pos=%lld,whence=%d\n",c->inner->filename,pos,c->pos,whence);
    }
    ret = ffurl_seek(c->inner, pos, whence);

    if( ret >=0 ) {

    	if(AVSEEK_SIZE != whence){
    		av_log(h, AV_LOG_ERROR, "ffurl_seek = %lld\n",ret);
    		pos= lseek(c->fd, pos, whence);
    		if(pos != ret){
    			av_log(h, AV_LOG_ERROR, "lseek pos != ret pos=%lld",pos);
    		}else{
    			c->pos = ret;
    		}
    	}
    }

    return ret;

}
static int cache_close(URLContext *h)
{
	int64_t size;

    Context *c= (Context*)h->priv_data;

    if(c->fd >= 0)close(c->fd);

    //判断是否需要保存
    if(c->local_buffer_path && c->fd >= 0){
		size = ffurl_seek(c->inner, 0, AVSEEK_SIZE);

		if( size != c->pos || c->end < size ){
			//删除文件
			unlink(c->local_buffer_path);
			av_log(h, AV_LOG_INFO, "ucache: failed size=%lld,c->pos=%lld,c->end=%lld\n",size,c->pos,c->end);
		    //通知上层缓存失败
		    if(c->local_buffer_path)notifyCacheEnd(h,0);
		}else{
		    //通知上层缓存成功
		    if(c->local_buffer_path)notifyCacheEnd(h,1);
			av_log(h, AV_LOG_INFO, "ucache: ok2\n");
		}
    }

    ffurl_close(c->inner);

    return 0;
}

URLProtocol ff_ucache_protocol = {
    .name                = "ucache",
    .url_open            = cache_open,
    .url_read            = cache_read,
    .url_seek            = cache_seek,
    .url_close           = cache_close,
    .priv_data_size      = sizeof(Context),
};


