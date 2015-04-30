#include "uplayer.h"
#include "ulog.h"
extern "C"{
#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavutil/file.h"
}
class UCacheFile{

public:
	static const char* getLocalCachePath(UPlayer* obj){
			UPlayer* player = (UPlayer*)obj;

			if(!player)return NULL;

			return player->getLocalCachePath();
	}
	static void notifyCacheMsg(UPlayer* obj,int ok){

		UPlayer* player = (UPlayer*)obj;

		if(!player)return;

		player->notifyCacheMsg(ok);
	}
};
extern "C" const char* getLocalCachePath(URLContext *h){

		if(!h)return NULL;

		UPlayer* player = (UPlayer*)h->interrupt_callback.opaque;

		if(!player)return NULL;

		return UCacheFile::getLocalCachePath(player);

}
extern "C" void notifyCacheEnd(URLContext *h,int ok){

	if(!h)return;

	UPlayer* player = (UPlayer*)h->interrupt_callback.opaque;

	if(!player)return;

	UCacheFile::notifyCacheMsg(player,ok);
}

