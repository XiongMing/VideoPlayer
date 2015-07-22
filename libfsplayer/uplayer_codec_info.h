//
//  UPlayCodecInfo.h
//  IOSUPlayerDemo
//
//  Created by Nick on 15/7/22.
//  Copyright (c) 2015年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#ifndef UPLAYER_CODEC_INFO_
#define UPLAYER_CODEC_INFO_


extern "C" {
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
}




class UPlayCodecInfo{
    
public:
    
    UPlayCodecInfo();
    
    virtual ~UPlayCodecInfo();
    
    bool getPlayerCodecInfo(const char *url);
    
private:
    
    AVFormatContext  *mMediaFileContext;
};


#endif /* defined(__IOSUPlayerDemo__UPlayCodecInfo__) */