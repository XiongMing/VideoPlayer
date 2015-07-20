//
//  IOSEgl.m
//  UPlayerNew
//
//  Created by Huang Weqing on 14-12-5.
//  Copyright (c) 2014年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#import "IOSEGLSurface.h"
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
@class IOSEglView;

#pragma mark - EGLSurface
EGLSurface::EGLSurface()
{
    ulog_info("EGLSurface constructor");
    glView = nil;
    panelView = nil;
    uPlayer = NULL;
}

EGLSurface::~EGLSurface()
{
    [glView removeFromSuperview];
    panelView = nil;
    glView = nil;
    uPlayer = NULL;
    ulog_info("EGLSurface destructor");
}

bool EGLSurface::InitEGL(int width, int height)
{
    /*
     Why should I add autoreleasepool in this place?
     Fist you should know function InitEGL is called in the way of pthread.
     It isn't called in Object-c thread.
     In Object-c thread, unlike pthread, it contains runloop which manages garbage collection.
     (Runloop is automaticlly started in Object main thread. In the fact, when object main thread exists,
     it notifys runloop to collect garbage. Finally runloop calls realease function of object.)
     Thus, if you use Object-c object in pthread, you should maitain garbage collection by yourself.
     That means you need to create one autoreleasepool once you create thread.
     */
    @autoreleasepool {
        ulog_info("EGLSurface::InitEGL\n");
        
        if(!panelView)
            return false;
        
        if (!uPlayer)
            return false;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            glView = [[IOSEglView alloc] initEGLWithDecoderWidth:width
                                        WithDecoderHeight:height
                                        WithBounds:panelView.bounds
                                        WithIosPlayer:iosPlayer
                                        WithParentView:panelView];
            
            
        });
        
        if (![glView setCurrentText]) {
            glView = nil;
            return false;
        }
        glView.player = uPlayer;
        __weak IOSEglView * weakGlView = glView;
        dispatch_sync(dispatch_get_main_queue(), ^{
            if(panelView && weakGlView){
                [panelView addSubview:weakGlView];
                [panelView setFrame:CGRectMake(0, 0, ((Panel *)panelView).playerBounds.size.width,
                                                ((Panel *)panelView).playerBounds.size.height)];
                weakGlView.contentMode = panelView.contentMode;
            }
        });
    }
    
    
    return true;
}

void EGLSurface::ReleaseEGL()
{
    @autoreleasepool {
        [glView releaseEGL];
    }
    
}

void EGLSurface::UpdateSurface()
{
    @autoreleasepool {
        [glView updateSurface];
    }
}

void EGLSurface::UpdateSize(int *width, int *height)
{
    @autoreleasepool {
        [glView updateSize];
    }
    *width = glView.backingWidth;
    *height = glView.backingHeight;
}

void EGLSurface::setMainView(Panel *mainView)
{
    panelView = mainView;
}

void EGLSurface::setIOSPlaye(FSIOSPlayer *player)
{
    iosPlayer = player;
}

void EGLSurface::setUPlayer(UPlayer *player)
{
    uPlayer = player;
}

#pragma mark - IOSEglView

@implementation IOSEglView
{
    EAGLContext     *_context;
    GLuint          _framebuffer;
    GLuint          _renderbuffer;
    int _decoderWidth;
    int _decoderHeight;
    int _glViewPortX;
    int _glViewPortY;
    CGRect _srcFrame;
    CGRect _dstFrame;
    BOOL _setEglViewShown;
    NSCondition *_lock;
    BOOL _firstSetFrame;
    BOOL _contentModeAnimation;
}

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}


-(void)setContentMode:(UIViewContentMode)contentMode{
    if (self.contentMode == contentMode) {
        return;
    }
    
    _contentModeAnimation = _iosPlayer.fullScreenAnimated;
    
    NSTimeInterval width = 0.0f;
    NSTimeInterval height = 0.0f;
    NSTimeInterval x = 0.0f;
    NSTimeInterval y = 0.0f;
    NSTimeInterval widthScale = 0.0f;
    NSTimeInterval heightScale = 0.0f;
    
    NSTimeInterval superviewFrameScale = self.superview.frame.size.width / self.superview.frame.size.height;
    NSTimeInterval decodeScale = (NSTimeInterval)_decoderWidth / _decoderHeight;
    
    
    if (_contentModeAnimation) {
        [UIView beginAnimations:@"contenMode" context:nil];
        [UIView setAnimationDuration:0.3];
    }
    
    /*Zoom images.
     All operations should reset "contentMode" to that of initializtion.
     "contentMode" is initialized to "UIViewContentModeScaleToFill" by IOS system.
     It is equivalent to that "self.transform" should be set to "CGAffineTransformIdentity".
     Why?
     In my program, I resize images by using "CGAffineTransformScale" function.
     */
    if (self.contentMode ==  UIViewContentModeScaleToFill) {
        
        if (contentMode == UIViewContentModeScaleAspectFit) {
            
            if (superviewFrameScale ==  decodeScale) {
                
                width = self.superview.frame.size.width;
                height = self.superview.frame.size.height;
                x = 0;
                y = 0;
                widthScale = 1.0;
                heightScale = 1.0;
                
            }else if(superviewFrameScale > decodeScale){
                
                //高度不变
                height = self.superview.frame.size.height;
                width = self.superview.frame.size.height * _decoderWidth / _decoderHeight;
                y = 0.0f;
                x = (self.superview.frame.size.width - width) / 2;
                heightScale = 1.0;
                widthScale = width / self.superview.frame.size.width;
                
                
            }else{
                
                //宽度不变
                width = self.superview.frame.size.width;
                height = self.superview.frame.size.width * _decoderHeight / _decoderWidth;
                x = 0;
                y = 0 /*(self.superview.frame.size.height - height) / 2;*/;
                widthScale = 1.0;
                heightScale = height / self.superview.frame.size.height;
                
            }
            
        }
        
        if(contentMode == UIViewContentModeScaleAspectFill){
            
            if (superviewFrameScale == decodeScale) {
                
                width = self.superview.frame.size.width;
                height = self.superview.frame.size.height;
                x = 0;
                y = 0;
                widthScale = 1.0;
                heightScale = 1.0;
                
            }else if(superviewFrameScale > decodeScale){
                
                width = self.superview.frame.size.width;
                height = self.superview.frame.size.width * _decoderHeight / _decoderWidth;
                x = 0;
                y = (self.superview.frame.size.height - height) / 2;
                widthScale = 1.0;
                heightScale = height / self.superview.frame.size.height;
                
            }else{
                
                height = self.superview.frame.size.height;
                width = self.superview.frame.size.height * _decoderWidth / _decoderHeight;
                y = 0;
                x = (self.superview.frame.size.width - width) / 2;
                heightScale = 1.0;
                widthScale = width / self.superview.frame.size.width;
                
            }
            
            
        }
        
        self.transform = CGAffineTransformScale(self.transform, widthScale, heightScale);
    }
    
    else if(self.contentMode == UIViewContentModeScaleAspectFit){
        
        if (contentMode == UIViewContentModeScaleAspectFill) {
            
            self.transform = CGAffineTransformIdentity;
            
            if (superviewFrameScale == decodeScale) {
                
                width = self.superview.frame.size.width;
                height = self.superview.frame.size.height;
                x = 0;
                y = 0;
                widthScale = 1.0;
                heightScale = 1.0;
                
            }else if(superviewFrameScale > decodeScale){
                
                width = self.superview.frame.size.width;
                height = self.superview.frame.size.width * _decoderHeight / _decoderWidth;
                x = 0;
                y = (self.superview.frame.size.height - height) / 2;
                widthScale = 1.0;
                heightScale = height / self.superview.frame.size.height;
                
            }else{
                
                height = self.superview.frame.size.height;
                width = self.superview.frame.size.height * _decoderWidth / _decoderHeight;
                y = 0;
                x = (self.superview.frame.size.width - width) / 2;
                heightScale = 1.0;
                widthScale = width / self.superview.frame.size.width;
                
            }
            
            self.transform = CGAffineTransformScale(self.transform, widthScale, heightScale);
            
        }else if(contentMode == UIViewContentModeScaleToFill){
            
            self.transform = CGAffineTransformIdentity;
        }
        
    }
    else{
        
        //UIViewContentModeScaleAspectFill
        if (contentMode == UIViewContentModeScaleToFill) {
            
           self.transform = CGAffineTransformIdentity;
            
        }else if(contentMode ==  UIViewContentModeScaleAspectFit){
    
            self.transform = CGAffineTransformIdentity;
           
            if (superviewFrameScale ==  decodeScale) {
                
                width = self.superview.frame.size.width;
                height = self.superview.frame.size.height;
                x = 0;
                y = 0;
                widthScale = 1.0;
                heightScale = 1.0;
                
            }else if(superviewFrameScale > decodeScale){
                
                //高度不变
                height = self.superview.frame.size.height;
                width = self.superview.frame.size.height * _decoderWidth / _decoderHeight;
                y = 0.0f;
                x = (self.superview.frame.size.width - width) / 2;
                heightScale = 1.0;
                widthScale = width / self.superview.frame.size.width;
                
                
            }else{
                
                //宽度不变
                width = self.superview.frame.size.width;
                height = self.superview.frame.size.width * _decoderHeight / _decoderWidth;
                x = 0;
                y = 0 /*(self.superview.frame.size.height - height) / 2;*/;
                widthScale = 1.0;
                heightScale = height / self.superview.frame.size.height;
                
            }
            self.transform = CGAffineTransformScale(self.transform, widthScale, heightScale);
        }
        
    }
    [super setContentMode:contentMode];
    
    if (_contentModeAnimation) {
        [UIView commitAnimations];
    }
//    NSLog(@"contentModeChanged: selfFrame:%@ width: %f, height: %f", self, self.frame.size.width, self.frame.size.height );
}


-(void)setFrame:(CGRect)frame{
    
    [super setFrame:frame];
    _backingHeight = frame.size.height;
    _backingWidth = frame.size.width;
    
    //Because you resize the "self.frame"
    //You should realloc the render buffer which is called by "UPlayer".
    //It is realized through function "changeVideoSize".
    if(_player){
        _player->changeVideoSize(_backingWidth, _backingHeight);
    }
}

-(id)initEGLWithDecoderWidth:(int)width WithDecoderHeight:(int)height WithBounds:(CGRect)bounds
               WithIosPlayer:(FSIOSPlayer *)player WithParentView:(UIView *)parentView
{
    _firstSetFrame = YES;
    _contentModeAnimation = NO;
    _iosPlayer = player;
    _lock = [[NSCondition alloc] init];
    _setEglViewShown = YES;
    _glViewPortX = 0;
    _glViewPortY = 0;
    parentView.clipsToBounds = YES;
    self = [super init];
    if(!self)
        return nil;
//    [parentView addSubview:self];
//    [self setFrame:CGRectMake(0, 0, ((Panel *)self.superview).playerBounds.size.width,
//                              ((Panel *)self.superview).playerBounds.size.height)];
    [self setFrame:CGRectMake(0, 0, parentView.frame.size.width, parentView.frame.size.height)];
    
    _decoderWidth = width;
    _decoderHeight = height;
//    CAEAGLLayer *eaglLayer = (CAEAGLLayer*) self.layer;
//    eaglLayer.opaque = YES;
//    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
//                                    [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
//                                    kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
//                                    nil];
//    
//    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
//    
//    if (!_context ||
//        ![EAGLContext setCurrentContext:_context]) {
//        
//        NSLog(@"failed to setup EAGLContext");
//        self = nil;
//        return nil;
//    }
//    glGenFramebuffers(1, &_framebuffer);
//    glGenRenderbuffers(1, &_renderbuffer);
//    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
//    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
//    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
//    
//    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
//    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
//    
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderbuffer);
//    
//    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//    if (status != GL_FRAMEBUFFER_COMPLETE) {
//        
//        NSLog(@"failed to make complete framebuffer object %x", status);
//        self = nil;
//        return self;
//    }
//    
//    GLenum glError = glGetError();
//    if (GL_NO_ERROR != glError) {
//        
//        NSLog(@"failed to setup GL %x", glError);
//        self = nil;
//        return self;
//    }
    return self;
}

-(BOOL)setCurrentText{
        CAEAGLLayer *eaglLayer = (CAEAGLLayer*) self.layer;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];
    
        _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
        if (!_context ||
            ![EAGLContext setCurrentContext:_context]) {
    
            NSLog(@"failed to setup EAGLContext");
            return NO;
        }
        glGenFramebuffers(1, &_framebuffer);
        glGenRenderbuffers(1, &_renderbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
        [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
    
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
    
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderbuffer);
    
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
    
            NSLog(@"failed to make complete framebuffer object %x", status);
            return NO;
        }
    
        GLenum glError = glGetError();
        if (GL_NO_ERROR != glError) {
            
            NSLog(@"failed to setup GL %x", glError);
            return NO;
        }
    return YES;

}

-(void)updateSurface
{
    [_context presentRenderbuffer:GL_RENDERBUFFER];
}

-(void)updateSize
{
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
}

-(void)releaseEGL
{
    if (_framebuffer) {
        glDeleteFramebuffers(1, &_framebuffer);
        _framebuffer = 0;
    }
    
    if (_renderbuffer) {
        glDeleteRenderbuffers(1, &_renderbuffer);
        _renderbuffer = 0;
    }
    
	
	if ([EAGLContext currentContext] == _context) {
		[EAGLContext setCurrentContext:nil];
	}
    
	_context = nil;
    _player = NULL;
}

-(void)dealloc
{
    [self releaseEGL];
    NSLog(@"%@ IOSEglView dealloc", self);

}


@end
