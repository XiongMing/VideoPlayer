//
//  UPlayCodecInfo.cpp
//  IOSUPlayerDemo
//
//  Created by Nick on 15/7/22.
//  Copyright (c) 2015年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#include "uplayer_codec_info.h"

UPlayCodecInfo::UPlayCodecInfo(){
    mMediaFileContext = NULL;
}


UPlayCodecInfo::~UPlayCodecInfo(){
    if (mMediaFileContext) {
        avformat_close_input(&mMediaFileContext);
        mMediaFileContext = NULL;
    }
}

bool UPlayCodecInfo::getPlayerCodecInfo(const char * url){
    
    av_register_all();
    
    if (avformat_open_input(&mMediaFileContext, url, NULL, NULL) != 0)
        return false;
    
    if(avformat_find_stream_info(mMediaFileContext, NULL) < 0)
        return false;
    
    int streamsCount = mMediaFileContext->nb_streams;
    if (streamsCount != 2)  return false;
    bool h264Sign = false, aacSign = false;
    for(int i = 0; i < streamsCount; i++){
        
        AVCodecID type = mMediaFileContext->streams[i]->codec->codec_id;
        
        if (AV_CODEC_ID_AAC == type)
            aacSign = true;
        
        if (AV_CODEC_ID_H264 == type) {
            h264Sign = true;
        }
    }
    
    if (aacSign && h264Sign) {
        return true;
    }
    
    return false;
    
}