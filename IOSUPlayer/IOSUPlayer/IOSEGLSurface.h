//
//  IOSEgl.h
//  UPlayerNew
//
//  Created by Huang Weiqing on 14-12-5.
//  Copyright (c) 2014年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "ugraphics.h"
#include "FSIOSPlayer.h"

@class IOSEglView;

class EGLSurface : public IEGL
{
public:
    EGLSurface();
     ~EGLSurface();
    bool InitEGL(int width, int height);
    void ReleaseEGL();
    void UpdateSurface();
    void UpdateSize(int *width, int *height);
    
    void setMainView(Panel *mainView);
    void setUPlayer(UPlayer *player);
    void setIOSPlaye(FSIOSPlayer *player);

private:
    __weak Panel *panelView;
    __weak FSIOSPlayer *iosPlayer;
    IOSEglView* glView;
    UPlayer *uPlayer;
    
};


@interface IOSEglView : UIView
-(id) initEGLWithDecoderWidth:(int)width
            WithDecoderHeight:(int)height
                   WithBounds:(CGRect)bounds
                WithIosPlayer:(FSIOSPlayer *)player
                  WithParentView:(UIView *)parentView;

-(void) releaseEGL;
-(void) updateSurface;
-(void) updateSize;

@property(nonatomic, readonly)int backingWidth;
@property(nonatomic, readonly)int backingHeight;
@property(nonatomic) UPlayer * player;
@property(nonatomic, weak) FSIOSPlayer *iosPlayer;

@end
