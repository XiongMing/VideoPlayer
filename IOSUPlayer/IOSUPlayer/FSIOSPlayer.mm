//  FSIOSPlayer.m
//  IOSUPlayerDemo
//
//  Created by Huangwq on 15/1/4.
//  Copyright (c) 2015年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#import "FSIOSPlayer.h"
#import <Foundation/Foundation.h>
#import "IOSEGLSurface.h"
#include "uplayer.h"
//#import "sys/utsname.h"
#import <sys/sysctl.h>
#import <stdlib.h>

#pragma mark - FSIOSPlayer Notification

//These callbacks below is only called by FSIOSPlayer.
//Users don't care. Just posted between Listner and FSIOSPlayer.
//posted when uplayer posts message MEDIA_INFO_PREPARED
NSString *const FSIOSPlayerPreparedToPlayedDidFinishedNotification = @"FSIOSPlayerPreparedToPlayedDidFinishedNotification";

//posted when uplayer posts message MEDIA_INFO_BUFFERING_START
NSString *const FSIOSPlayerMediaInfoBufferingStartNotification = @"FSIOSPlayerMediaInfoBufferingStartNotification";

//posted when uplayer posts message MEDIA_INFO_BUFFERING_END
NSString *const FSIOSPlayerMediaInfoBufferingEndNotification = @"FSIOSPlayerMediaInfoBufferingEndNotification";

//posted when uplayer posts message MEDIA_SEEK_COMPLETE
NSString *const FSIOSPlayerMediaSeekCompleteNotification = @"FSIOSPlayerMediaSeekCompleteNotification";

//posted when uplayer posts message MEDIA_INFO_COMPLETED,MEDIA_INFO_PLAYERROR,MEDIA_INFO_DATA_SOURCE_ERROR
//and MEDIA_INFO_PREPARE_ERROR
NSString *const FSIOSPlayerMediaPlaybackDidFinishNotification = @"FSIOSPlayerMediaPlaybackDidFinishNotification";
    NSString *const FSIOSPlayerMediaPlaybackDidFinishReason = @"FSIOSPlayerMediaPlaybackDidFinishReason";
    NSString *const FSIOSPlayerMediaPlaybackDidFinishExtraReason = @"FSIOSPlayerMediaPlaybackDidFinishExtraReason";
    NSString *const MPMoviePlayerPlaybackErrorCodeInfoKey = @"MPMoviePlayerPlaybackErrorCodeInfoKey";
/*
 when FSIOSPlayerMediaPlaybackDidFinishNotification sends MPMovieFinishReasonPlaybackError,
FSIOSPlayerMediaPlaybackDidFinishExtraReason message should follow which shows details of MPMovieFinishReasonPlaybackError
 */


NSString *const FSIOSPlayerMediaPlaybackToTheEndNotification = @"FSIOSPlayerMediaPlaybackToTheEndNotification";

#pragma mark - UPlayer listner implementation
class Listener : public UPlayerListener
{
public:
    Listener();
    ~Listener();
    void notify(int msg, int ext1=0, int ext2=0);
    void setFSIOSPlayer(FSIOSPlayer *player);
private:
    __weak FSIOSPlayer *_player;
    
};

void Listener::setFSIOSPlayer(FSIOSPlayer *player){
    _player = player;
}

Listener::Listener()
{
    ulog_info("Listener constructor");
}

Listener::~Listener()
{
    ulog_info("Listener destructor");
}

void Listener::notify(int msg, int ext1, int ext2)
{
    switch (msg) {
        case MEDIA_INFO_PREPARED:
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                
                [[NSNotificationCenter defaultCenter] postNotificationName:
                 FSIOSPlayerPreparedToPlayedDidFinishedNotification object:_player];
                
            });
            break;
        }
            
            
//        case MEDIA_INFO_COMPLETED:
        case MEDIA_INFO_PLAY_TO_END:
        case MEDIA_INFO_PLAYERROR:
        case MEDIA_INFO_DATA_SOURCE_ERROR:
        case MEDIA_INFO_PREPARE_ERROR:
        case MEDIA_INFO_NETWORK_ERROR:
        case MEDIA_INFO_SEEK_ERROR:
        case MEDIA_INFO_NETWORK_DISCONNECTED:
        case MEDIA_INFO_NETWORK_DISCONNECTED_CHECK:
        case MEDIA_INFO_PREPARE_TIMEOUT_ERROR:
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                
                MPMovieFinishReason reason =
                MEDIA_INFO_PLAY_TO_END == msg ? MPMovieFinishReasonPlaybackEnded:MPMovieFinishReasonPlaybackError;
                NSNumber *number1 = [NSNumber numberWithInteger:reason];
                NSNumber *number2 = [NSNumber numberWithInteger:ext1];
                NSDictionary *userInfo = [NSDictionary dictionaryWithObjectsAndKeys: number1, FSIOSPlayerMediaPlaybackDidFinishReason,
                                          number2, FSIOSPlayerMediaPlaybackDidFinishExtraReason, nil];
                [[NSNotificationCenter defaultCenter] postNotificationName:
                                                    FSIOSPlayerMediaPlaybackDidFinishNotification
                                                    object:_player userInfo:userInfo];
                
            });
            break;
        }
            
        case MEDIA_INFO_BUFFERING_START:
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [[NSNotificationCenter defaultCenter] postNotificationName:
                 FSIOSPlayerMediaInfoBufferingStartNotification object:_player];
            });
            break;
        }
            
            
        case MEDIA_INFO_BUFFERING_END:
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [[NSNotificationCenter defaultCenter] postNotificationName:
                 FSIOSPlayerMediaInfoBufferingEndNotification object:_player];
            });
            break;
        }
            
        
        case MEDIA_SEEK_COMPLETE:
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [[NSNotificationCenter defaultCenter] postNotificationName:
                 FSIOSPlayerMediaSeekCompleteNotification object:_player];
            });
            break;
        }
            
//        case MEDIA_INFO_PLAY_TO_END:
//        {
//            dispatch_async(dispatch_get_main_queue(), ^{
//                [[NSNotificationCenter defaultCenter] postNotificationName:
//                 FSIOSPlayerMediaPlaybackToTheEndNotification object:_player];
//            });
//            break;
//        }
            
        default:
        {
            ulog_info("%d", msg);
            break;
        }
            
    }
    
}

#pragma mark - Panel implementation

@interface Panel(){
    
    BOOL _fullScreenAnimation;
    BOOL _preFullScreen;
    
    // (_preFullScreen: NO,  _iosPlayer.fullScreen: NO, _enterFullScreen: 0)
    // (_preFullScreen: NO,  _iosPlayer.fullScreen: YES, _enterFullScreen: 1)
    // (_preFullScreen: YES,  _iosPlayer.fullScreen: NO, _enterFullScreen: 2)
    // (_preFullScreen: YES,  _iosPlayer.fullScreen: YES, _enterFullScreen: 3)
    int _enterFullScreen;
}
@property(nonatomic, weak) FSIOSPlayer *iosPlayer;

-(id)init;
@end

@implementation Panel{

}

-(id)init{
    self = [super init];
    if (!self) {
        return nil;
    }
    _fullScreenAnimation = NO;
    _preFullScreen = NO;
    _enterFullScreen = 0;
    return self;
}

-(void) setFrame:(CGRect)bounds
{
    /*
     if @equalBetweenTwoRects is YES,
     Even if you define setAnimationWillStartSelector,
     the delegate can't be called.
     */
    BOOL equalBetweenTwoRects = (self.frame.origin.x == bounds.origin.x \
                        && self.frame.origin.y == bounds.origin.y \
                        && self.frame.size.width == bounds.size.width \
                        && self.frame.size.height == bounds.size.height);
                        
    if (_preFullScreen && equalBetweenTwoRects) {
        return;
    }
    
    if (!_preFullScreen && !_iosPlayer.isFullscreen) {
        _enterFullScreen = 0;
    }else if(!_preFullScreen && _iosPlayer.isFullscreen){
        _enterFullScreen = 1;
    }else if(_preFullScreen && !_iosPlayer.isFullscreen){
        _enterFullScreen = 2;
    }else{
        _enterFullScreen = 3;
    }
    
    _preFullScreen = _iosPlayer.isFullscreen;
    _fullScreenAnimation = _iosPlayer.fullScreenAnimated;
    
    if (_fullScreenAnimation && !equalBetweenTwoRects) {
        [UIView beginAnimations:@"ChangeParentSize" context:nil];
        //[UIView setAnimationDuration:2];
        [UIView setAnimationDelegate:self];
        [UIView setAnimationWillStartSelector:@selector(start)];
        [UIView setAnimationDidStopSelector:@selector(stop)];
    }else{
        [self postWillEnterOrExitFullScreen];
    }
    
    NSArray * subViews = [self subviews];
    if (subViews.count > 0) {
        
        UIView * frameView = [subViews objectAtIndex:0];
        frameView.contentMode = UIViewContentModeScaleToFill;
    }
    
    [super setFrame:bounds];
    
    if (subViews.count > 0){
        
        UIView * frameView = [subViews objectAtIndex:0];
        _playerBounds = bounds;
        [frameView setFrame:CGRectMake(0, 0, bounds.size.width, bounds.size.height)];
        frameView.contentMode = self.contentMode;
        
    }else{
        
        self.playerBounds = bounds;
    }
    
    if (_fullScreenAnimation && !equalBetweenTwoRects)
        [UIView commitAnimations];
    else
        [self postDidEnterOrExitFullScreen];
}

-(void)setContentMode:(UIViewContentMode)contentMode
{
    [super setContentMode:contentMode];
    if (UIViewContentModeScaleAspectFit != self.contentMode) {
        NSLog(@"not equal");
    }
    
    NSArray * subViews = [self subviews];
    if(subViews.count > 0){
        UIView * frameView = [subViews objectAtIndex:0];
        frameView.contentMode = contentMode;
    }
}

-(void)setFSIOSPlayer:(FSIOSPlayer *)player{
    _iosPlayer = player;
}

-(void)start{
    
    [self postWillEnterOrExitFullScreen];
}

-(void)stop{
    
    [self postDidEnterOrExitFullScreen];
}

-(void)postWillEnterOrExitFullScreen{
    if (1 == _enterFullScreen || 3 == _enterFullScreen) {
        NSNotification *notification = [NSNotification
                                        notificationWithName:MPMoviePlayerWillEnterFullscreenNotification
                                        object:_iosPlayer];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification postingStyle:NSPostNow];
    }else if(2 == _enterFullScreen){
        NSNotification *notification = [NSNotification
                                        notificationWithName:MPMoviePlayerWillExitFullscreenNotification
                                        object:_iosPlayer];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification postingStyle:NSPostNow];
    }
}

-(void)postDidEnterOrExitFullScreen{
    if (1 == _enterFullScreen || 3 == _enterFullScreen) {
        NSNotification *notification = [NSNotification
                                        notificationWithName:MPMoviePlayerDidEnterFullscreenNotification
                                        object:_iosPlayer];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification postingStyle:NSPostNow];
    }else if(2 == _enterFullScreen){
        NSNotification *notification = [NSNotification
                                        notificationWithName:MPMoviePlayerDidExitFullscreenNotification
                                        object:_iosPlayer];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification postingStyle:NSPostNow];
    }
}

-(void)dealloc
{
    NSLog(@"panelView dealloc");
}

@end


#pragma mark - FSIOSPlayer extented
@interface FSIOSPlayer()
{
    
    UPlayer *_player;
    EGLSurface *_egl;
    Listener *_listener;
    BOOL    _firstPlay;
    CGRect  _viewFrame;
    CGRect  _currentFullscreenFrame;
    BOOL    _innerStop;
    NSTimeInterval _currentPlaybackTime;
    NSInteger _playbackDidFinishedCode;
    BOOL   _playerStopped;
};

@property (nonatomic, readwrite) MPMoviePlaybackState playbackState;
@property (nonatomic, readwrite) NSTimeInterval duration;
@property (nonatomic, readwrite) CGSize naturalSize;
@property (nonatomic, readwrite) MPMovieSourceType movieSourceType;
@property (nonatomic, readwrite) MPMovieMediaTypeMask movieMediaTypes;
@property (nonatomic, readwrite) NSTimeInterval playableDuration;
@property (nonatomic, readwrite) MPMovieLoadState loadState;
@property (nonatomic, readwrite) Panel *view;
//@property (nonatomic, readwrite) NSInteger playbackErrorCode;

@end

#pragma mark - FSIOsPlayer implementation
@implementation FSIOSPlayer

+(void)initialize{
    
    UPlayer::registerAVcodec();
    
}

#pragma mark - -public

-(id)init{
    self = [super init];
    if (!self)
        self =  nil;
    
    if (![self realInit]) {
        self = nil;
    }
    return self;
}

- (id)initWithContentURL:(NSURL *)url{
    _contentURL = url;
    
    self = [super init];
    if (!self)
        self = nil;
    
    if (![self realInit]) {
        self = nil;
    }
    return self;
}


-(void)prepareToPlay{
    
    if(_playerStopped || !_player) return;
    _player->setDataSource([[[_contentURL absoluteString]
                             stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding] UTF8String]);
                            
    //when you are preparing, loadState should be MPMovieLoadStatePlaythroughOK
    //also means it is buffering.
    self.loadState = MPMovieLoadStatePlaythroughOK;
    NSNotification * notification =  [NSNotification notificationWithName:
                                      MPMoviePlayerLoadStateDidChangeNotification object:self];
    [[NSNotificationQueue defaultQueue] enqueueNotification:notification postingStyle:NSPostNow];
    _player->prepare();
}

-(void)play{
    
    if(_playerStopped || !_player) return;
    
    if(self.loadState != (MPMovieLoadStatePlayable | MPMovieLoadStatePlaythroughOK))
       return;
    
    if (MPMoviePlaybackStatePlaying != _playbackState) {
        _playbackState = MPMoviePlaybackStatePlaying;
        
        NSNotification * notification = [NSNotification notificationWithName:
                                         MPMoviePlayerPlaybackStateDidChangeNotification object:self];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification
                                                   postingStyle:NSPostNow];
    }
    _player->start();
    
}

-(void)pause{
    if(_playerStopped || !_player) return;
    
    if(self.loadState != (MPMovieLoadStatePlayable | MPMovieLoadStatePlaythroughOK))
        return;
    
    if (MPMoviePlaybackStatePaused != _playbackState) {
        _playbackState = MPMoviePlaybackStatePaused;
        NSNotification * notification = [NSNotification notificationWithName:
                                         MPMoviePlayerPlaybackStateDidChangeNotification object:self];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification
                                                   postingStyle:NSPostNow];
    }
    _player->pause();
    
}

-(void)stop{
    
    if (_playerStopped) return;
    
    _playerStopped = YES;
    
    if (_playbackDidFinishedCode == MPMovieFinishReasonUserExited) {
        
        NSNumber *reason = [NSNumber numberWithInteger:MPMovieFinishReasonUserExited];
        NSNumber *errorCode = [NSNumber numberWithInteger:0];
        NSDictionary *dic = [NSDictionary dictionaryWithObjectsAndKeys:reason, MPMoviePlayerPlaybackDidFinishReasonUserInfoKey,
                             errorCode, MPMoviePlayerPlaybackErrorCodeInfoKey, nil];
        [self postPlaybackDidFinish:dic];
    }
    
    
    if (MPMoviePlaybackStateStopped != _playbackState) {
        _playbackState = MPMoviePlaybackStateStopped;
        NSNotification * notification = [NSNotification notificationWithName:
                                         MPMoviePlayerPlaybackStateDidChangeNotification object:self];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification
                                                   postingStyle:NSPostNow];
    }
    
    if (!_player)  return;
    
    _player->setRenderVideo(false);
    _player->release();
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        if(_player){
            delete _player;
            _player = NULL;
        }
        
        
        if (_listener) {
            delete _listener;
            _listener = NULL;
        }
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            
            if (_egl) {
                delete _egl;
                _egl = NULL;
            }
            if (_view) {
                [_view removeFromSuperview];
            }
            
            _view = nil;
        });
    
        
        [self resetState];
        
    });
}

-(NSTimeInterval)duration{
    
    if(_playerStopped || !_player) return 0;
    int duration;
    _player->getDuration(&duration);
    duration /= 1000;
    return duration;
}

-(CGSize)naturalSize{
    if(_playerStopped || !_player) return CGSizeMake(0, 0);
    int width = 0;
    int height = 0;
    _player->getVideoWidth(&width);
    _player->getVideoHeight(&height);
    return CGSizeMake(width, height);
}

-(MPMovieMediaTypeMask)movieMediaTypes{
    
    if(_playerStopped || !_player) return MPMovieMediaTypeMaskNone;
    
    int mediaTypes = _player->getMediaTypes();
    
    MPMovieMediaTypeMask type = MPMovieMediaTypeMaskNone;
    
    if (UPLAYER_STREAM_VIDEO & mediaTypes)
        type |= MPMovieMediaTypeMaskVideo;
   
    if(UPLAYER_STREAM_AUDIO & mediaTypes)
        type |= MPMovieMediaTypeMaskAudio;
   
    return type;
    
    
}

-(MPMovieSourceType)movieSourceType{

    if(_playerStopped || !_player) return MPMovieSourceTypeUnknown;
    
    NSString *path = [_contentURL absoluteString];
    NSRange r = [path rangeOfString:@":"];
    if (r.location != NSNotFound && (self.loadState & MPMovieLoadStatePlayable))
        return MPMovieSourceTypeStreaming;
    else if(r.location == NSNotFound && (self.loadState & MPMovieLoadStatePlayable))
        return MPMovieSourceTypeFile;
    else
        return MPMovieSourceTypeUnknown;
}

-(NSTimeInterval)currentPlaybackTime{
    
    if(_playerStopped || !_player) return 0;
    
    int currentPlaybackTime = 0;
    _player->getCurrentPosition(&currentPlaybackTime);
    currentPlaybackTime /= 1000;
    _currentPlaybackTime = currentPlaybackTime;
    return currentPlaybackTime;
}

-(void)setCurrentPlaybackTime:(NSTimeInterval)currentPlaybackTime{

    if(_playerStopped || !_player) return ;
    
    if (currentPlaybackTime == self.currentPlaybackTime) {
        
        return;
        
    }else if(currentPlaybackTime > _currentPlaybackTime){
        
        if(MPMoviePlaybackStateSeekingForward != self.currentPlaybackTime){
            
            self.playbackState = MPMoviePlaybackStateSeekingForward;
            NSNotification * notification = [NSNotification notificationWithName:
                                             MPMoviePlayerPlaybackStateDidChangeNotification object:self];
            [[NSNotificationQueue defaultQueue] enqueueNotification:notification
                                                       postingStyle:NSPostNow];
        }
        
    }else{
        
        if (MPMoviePlaybackStateSeekingBackward != self.playbackState) {
            self.playbackState = MPMoviePlaybackStateSeekingBackward;
            NSNotification * notification = [NSNotification notificationWithName:
                                             MPMoviePlayerPlaybackStateDidChangeNotification object:self];
            [[NSNotificationQueue defaultQueue] enqueueNotification:notification
                                                       postingStyle:NSPostNow];
        }
    }
    
    NSTimeInterval seekTime = currentPlaybackTime;
    
    seekTime = seekTime < self.initialPlaybackTime ? self.initialPlaybackTime : seekTime;
    double tmpDuration = self.endPlaybackTime != 0 ? self.endPlaybackTime : self.duration;
    seekTime = seekTime > tmpDuration ? tmpDuration : seekTime;
//    seekTime = seekTime < self.initialPlaybackTime ? self.initialPlaybackTime : seekTime;
//    seekTime = seekTime < 0.0f ? 0.0f : seekTime;
//    seekTime = seekTime > self.duration ? self.duration : seekTime;
    _player->seekTo(seekTime * 1000);
}

-(NSTimeInterval)playableDuration{
    
    //get from p2p level.
    //Reserved for later modification
    return self.currentPlaybackTime + 1;
}

-(void)setScalingMode:(MPMovieScalingMode)scalingMode{
    
    BOOL notification = NO;
    switch (scalingMode) {
        case MPMovieScalingModeAspectFit:
        {
            if (self.scalingMode != scalingMode) {
                notification = YES;
                _scalingMode = scalingMode;
                self.view.contentMode = UIViewContentModeScaleAspectFit;
            }
        }
            break;
        case MPMovieScalingModeAspectFill:
        {
            if (self.scalingMode != scalingMode) {
                notification = YES;
                _scalingMode = scalingMode;
                self.view.contentMode = UIViewContentModeScaleAspectFill;
            }
        }
            break;
        case MPMovieScalingModeFill:
        {
            if (self.scalingMode != scalingMode) {
                notification = YES;
                _scalingMode = scalingMode;
                self.view.contentMode = UIViewContentModeScaleToFill;
            }
        }
            break;
        default:
            NSLog(@"unknown scaling mode");
            break;
    }
    
    if (notification) {
        NSNotification *notification = [NSNotification notificationWithName:
                                         MPMoviePlayerScalingModeDidChangeNotification object:self];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification postingStyle:NSPostNow];
    }
    
}


-(void)setRepeatMode:(MPMovieRepeatMode)repeatMode{
    if(_playerStopped || !_player) return;
    _repeatMode = repeatMode;
    bool repeat = _repeatMode == MPMovieRepeatModeOne ? true : false;
    _player->setRepeatMode(repeat);
}


-(BOOL)isFullscreen{
    
    return _fullscreen;
}

-(void)setFullscreen:(BOOL)fullscreen animated:(BOOL)animated{
    
    _fullScreenAnimated = animated;
    _fullscreen = fullscreen;
    if (fullscreen) {
        
        CGSize size = [self getFullScreenSize];
        self.view.frame = CGRectMake(0, 0, size.width, size.height);
        
    }
}

-(void)setInitialPlaybackTime:(NSTimeInterval)initialPlaybackTime{
    
    if(initialPlaybackTime > 0.0f)
        _initialPlaybackTime = initialPlaybackTime;
    else
        _initialPlaybackTime = 0.0f;
}

-(void)setEndPlaybackTime:(NSTimeInterval)endPlaybackTime{
    
    if(_playerStopped || !_player) return ;
    if (endPlaybackTime > 0) {
        _endPlaybackTime = endPlaybackTime;
        _player->setEndPlaybackTime(endPlaybackTime * 1000);
    }
}

-(void)setEnableHevcOptimization:(BOOL)enableHevcOptimization{
    
    if(_playerStopped || !_player) return ;
    int flag = enableHevcOptimization ? 1 : 0;
    _player->enableHEVC(flag);
}

#pragma mark - - observer callback
-(void)preparedDidFinished:(NSNotification *)notification{
    
    self.loadState |= MPMovieLoadStatePlayable;
    NSNotification *notification1 = [NSNotification notificationWithName:
                                     MPMoviePlayerLoadStateDidChangeNotification object:self];
    [[NSNotificationQueue defaultQueue] enqueueNotification:notification1 postingStyle:NSPostNow];
    
    if (self.initialPlaybackTime > 0) {
        
        self.currentPlaybackTime = self.initialPlaybackTime;
    }
    
    //have to check whether it spends sevral seconds in play operation.
    if (self.shouldAutoplay) {
        [self play];
    }
    //add callback MPMovieDurationAvailableNotification
    notification1 = [NSNotification notificationWithName:MPMovieDurationAvailableNotification object:self];
    [[NSNotificationQueue defaultQueue] enqueueNotification:notification1 postingStyle:NSPostNow];
    
    //add callback MPMovieNaturalSizeAvailableNotification
    notification1 = [NSNotification notificationWithName:MPMovieNaturalSizeAvailableNotification object:self];
    [[NSNotificationQueue defaultQueue] enqueueNotification:notification1 postingStyle:NSPostNow];
    
    //add callback MPMovieMediaTypesAvailableNotification
    notification1 = [NSNotification notificationWithName:MPMovieMediaTypesAvailableNotification object:self];
    [[NSNotificationQueue defaultQueue] enqueueNotification:notification1 postingStyle:NSPostNow];
    
}

-(void)bufferingStart:(NSNotification *)notification{
    
    [self pause];
    if (self.loadState != (MPMovieLoadStatePlayable | MPMovieLoadStateStalled)) {
        self.loadState = MPMovieLoadStatePlayable | MPMovieLoadStateStalled;
        NSNotification *notification1 = [NSNotification notificationWithName:
                                         MPMoviePlayerLoadStateDidChangeNotification object:self];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification1 postingStyle:NSPostNow];
    }
}

-(void)bufferingEnd:(NSNotification *)notification{
    
    if (self.loadState != (MPMovieLoadStatePlayable | MPMovieLoadStatePlaythroughOK)) {
        self.loadState = MPMovieLoadStatePlayable | MPMovieLoadStatePlaythroughOK;
        NSNotification *notification1 = [NSNotification notificationWithName:
                                         MPMoviePlayerLoadStateDidChangeNotification object:self];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification1 postingStyle:NSPostNow];
    }
    
    if(self.shouldAutoplay)
        [self play];
}

-(void)mediaSeekComplete:(NSNotification *)notification{
    
    if (_playerStopped || !_player) return;
    self.playbackState = _player->isPlaying() ? MPMoviePlaybackStatePlaying : MPMoviePlaybackStatePaused;
    NSNotification * notification1 = [NSNotification notificationWithName:
                                     MPMoviePlayerPlaybackStateDidChangeNotification object:self];
    [[NSNotificationQueue defaultQueue] enqueueNotification:notification1
                                               postingStyle:NSPostNow];
}

-(void)playbackDidFinish:(NSNotification *)notification{

    if(MPMovieFinishReasonPlaybackEnded == [[notification.userInfo objectForKey:FSIOSPlayerMediaPlaybackDidFinishReason] integerValue]){
        if (MPMovieRepeatModeOne == self.repeatMode){
            self.currentPlaybackTime = self.initialPlaybackTime;
        }else{
            [self pause];
        }
    }
    
    NSNumber *reason = [notification.userInfo objectForKey:FSIOSPlayerMediaPlaybackDidFinishReason];
    NSNumber *errorCode = [notification.userInfo objectForKey:FSIOSPlayerMediaPlaybackDidFinishExtraReason];
    NSDictionary *dic = [NSDictionary dictionaryWithObjectsAndKeys:reason, MPMoviePlayerPlaybackDidFinishReasonUserInfoKey,
                         errorCode, MPMoviePlayerPlaybackErrorCodeInfoKey, nil];
    [self postPlaybackDidFinish:dic];
}

-(void)postPlaybackDidFinish:(NSDictionary *)dic{
        NSNotification * notification = [NSNotification notificationWithName:MPMoviePlayerPlaybackDidFinishNotification
                                        object:self userInfo:dic];
        [[NSNotificationQueue defaultQueue] enqueueNotification:notification postingStyle:NSPostNow];
}

//-(void)playToTheEnd:(NSNotification *)notification{
//    
//    self.currentPlaybackTime = self.initialPlaybackTime;
//    //_firstPlay = FALSE;
//}

-(void)statusBarOrientationDidChange:(NSNotification *)notification{
    if(self.isFullscreen)
        [self setFullscreen:YES animated:_fullScreenAnimated];
}

-(void)willResignActive:(NSNotification *)notification{
    
    /*
     Note：
     If "stop" method is called in global queue, you need to add lock to keep
     "_player" safe. Why? Maybe you call "setRenderVideo" as below. The "_player" 
     may be killed in global queue.
     "setContent" is called in main queue in synchronized way.
     So I have to call "stop" in another thread. Thus you need to keep synchronized.
     
     Now I set contentMode in asynchronized way. "stop" can be called in main thread.
     "setRenderVideo" and "stop" are called in main thread, so it is safe in 
     this situation. Or dead-lock may take place.
     */
    if (_playerStopped || !_player)  return;
    _player->setRenderVideo(false);
        ulog_info("willResignActive false");

    
//    [self pause];
}

-(void)becomeActive:(NSNotification *)notification{
    /*
     Has the same situation as above.
     */
    if (_playerStopped || !_player)  return;
    _player->setRenderVideo(true);
    ulog_info("becomeActive true");
//    [self play];
}

-(void)didEnterFullScreen:(NSNotification *)notification{
    NSNotification * notification1 = [NSNotification notificationWithName:
                                      MPMoviePlayerDidEnterFullscreenNotification object:self];
    [[NSNotificationQueue defaultQueue] enqueueNotification:notification1 postingStyle:NSPostNow];
}

-(void)willEnterFullScreen:(NSNotification *)notification{
    NSNotification * notification1 = [NSNotification notificationWithName:
                                      MPMoviePlayerWillEnterFullscreenNotification object:self];
    [[NSNotificationQueue defaultQueue] enqueueNotification:notification1 postingStyle:NSPostNow];
}

#pragma mark - -private

-(BOOL)realInit{
    
    _playerStopped = NO;
    
    [self resetState];
    
    /*
     In my implementation, @_view is not real display layer.
     It is container which contains subview that are real display
     layer created by render thread.
     If you want to set the properties of display layer.
     You can store them in this @_view first.
     The real layer can get them from the parent view[@_view]
     after it is created.
     */
    
    _view = [[Panel alloc] init];
    if(!_view)
        return NO;
    
    [_view setFSIOSPlayer:self];
    _view.backgroundColor = [UIColor blackColor];
    _view.contentMode = UIViewContentModeScaleAspectFit;
//    CGRect bounds = [[UIScreen mainScreen] applicationFrame];
    CGSize size = [self getFullScreenSize];
    [_view setFrame:CGRectMake(0, 0, size.width, size.height)];
    
    
    _player = new UPlayer;
    if(!_player)
        return NO;
    
    _listener = new Listener;
    if(!_listener)
        return NO;
    
    _egl = new EGLSurface;
    if (!_egl)
        return NO;
    
    _egl->setUPlayer(_player);
    _egl->setMainView(_view);
    _egl->setIOSPlaye(self);
    _player->setListener(_listener);
    _player->setEGL(_egl);
    _player->setVideoSurface();
    _listener->setFSIOSPlayer(self);
    
    [self addObservers];
    
    return YES;
}

-(void)resetState{
    
    _innerStop = NO;
    _firstPlay = YES;
    _isPreparedToPlay = NO;
    _repeatMode = MPMovieRepeatModeNone;
    _scalingMode = MPMovieScalingModeAspectFit;
    _duration = 0.0f;
    _initialPlaybackTime = 0.0f;
    _endPlaybackTime = 0.0f;
    _currentPlaybackTime = 0.0f;
    _playableDuration = 0.0f;
    _movieSourceType = MPMovieSourceTypeUnknown;
    _movieMediaTypes = MPMovieMediaTypeMaskNone;
    _loadState = MPMovieLoadStateUnknown;
    _naturalSize = CGSizeMake(0, 0);
    _fullscreen = NO;
    _shouldAutoplay = NO;
    _playbackState = MPMoviePlaybackStatePaused;
    _currentFullscreenFrame = CGRectMake(0, 0, 0, 0);
    _fullScreenAnimated = NO;
    _playbackDidFinishedCode = MPMovieFinishReasonUserExited;
}


-(void) addObservers{
    
    //Add observer posted by myself
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(preparedDidFinished:)
                                                 name:FSIOSPlayerPreparedToPlayedDidFinishedNotification
                                               object:self];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(bufferingStart:)
                                                 name:FSIOSPlayerMediaInfoBufferingStartNotification
                                               object:self];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(bufferingEnd:)
                                                 name:FSIOSPlayerMediaInfoBufferingEndNotification
                                               object:self];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(mediaSeekComplete:)
                                                 name:FSIOSPlayerMediaSeekCompleteNotification
                                               object:self];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(playbackDidFinish:)
                                                 name:FSIOSPlayerMediaPlaybackDidFinishNotification
                                               object:self];
    
//    [[NSNotificationCenter defaultCenter] addObserver:self
//                                             selector:@selector(playToTheEnd:)
//                                                 name:FSIOSPlayerMediaPlaybackToTheEndNotification
//                                               object:self];
    
    
    //Posted by ios system.
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(willResignActive:)
                                                 name:UIApplicationWillResignActiveNotification object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(becomeActive:)
                                                 name:UIApplicationDidBecomeActiveNotification object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(statusBarOrientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
}

-(BOOL)isEqualBetweenRect:(CGRect)rect1 andRect:(CGRect)rect2{
    if(rect1.origin.x == rect2.origin.x &&
       rect1.origin.y ==  rect2.origin.y &&
       rect1.size.width == rect2.size.width &&
       rect1.size.height == rect2.size.height)
        return YES;
    else
        return NO;
}

-(CGSize)getFullScreenSize{
    CGFloat width = [[UIScreen mainScreen] bounds].size.width;
    CGFloat height = [[UIScreen mainScreen] bounds].size.height;
    
    CGFloat bigger = width > height ? width : height;
    CGFloat smaller = width > height ? height : width;
    
    UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
    if(UIInterfaceOrientationPortrait == orientation ||
       UIInterfaceOrientationPortraitUpsideDown == orientation){
        width = smaller;
        height = bigger;
    }else{
        width = bigger;
        height = smaller;
    }
    return CGSizeMake(width, height);
}

-(void)dealloc{
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    NSLog(@"FSIOSPlayer %@ dealloc", self);
}

#pragma mark - global

FSIOSPlayerDefinitionMode getDefinitionMode(){
    size_t size;
    sysctlbyname("hw.machine", NULL, &size, NULL, 0);
    char *machine = (char *)malloc(size);
    sysctlbyname("hw.machine", machine, &size, NULL, 0);
    NSString *platform = [NSString stringWithUTF8String:machine];
    free(machine);
    
    NSString *platformLowerString = [platform lowercaseString];
    if ([platformLowerString isEqualToString:@"ipod1,1"]    // ipod 4,1
        || [platformLowerString isEqualToString:@"ipod2,1"] //ipod touch 2
        || [platformLowerString isEqualToString:@"ipod3,1"] //ipod touch 3
        || [platformLowerString isEqualToString:@"ipod4,1"] //ipod touch 4
        || [platformLowerString isEqualToString:@"ipad1,1"] //ipad 1
        || [platformLowerString isEqualToString:@"iphone1,1"] //iphone 3
        || [platformLowerString isEqualToString:@"iphone1,2"] //iphone 3g
        || [platformLowerString isEqualToString:@"iphone2,1"] //iphone 3gs
        )
    {
        //ipod 1, 2, 3, 4
        //iphone 3, 3g, 3gs
        //ipad 1
        return FSIOSPlayerDefinitionModeNone;
        
    }else if([platformLowerString isEqualToString:@"ipod5,1"]      //ipod touch 5
             || [platformLowerString isEqualToString:@"iphone3,1"] //iphone 4
             || [platformLowerString isEqualToString:@"iphone3,2"] //iphone 4
             || [platformLowerString isEqualToString:@"iphone3,3"] //iphone 4
             || [platformLowerString isEqualToString:@"iphone4,1"] //iphone 4s
             )
    {
        //ipod touch 5
        //iphone 4, 4s
        return FSIOSPlayerDefinitionModeStandard;
        
    }else if([platformLowerString isEqualToString:@"ipad2,1"]    //ipad 2
             || [platformLowerString isEqualToString:@"ipad2,2"] //ipad 2
             || [platformLowerString isEqualToString:@"ipad2,3"] //ipad 2
             || [platformLowerString isEqualToString:@"ipad2,4"] //ipad 2
             || [platformLowerString isEqualToString:@"ipad2,5"] //ipad mini
             || [platformLowerString isEqualToString:@"ipad2,6"] //ipad mini
             || [platformLowerString isEqualToString:@"ipad2,7"] //ipad mini
             || [platformLowerString isEqualToString:@"ipad3,1"] //ipad 3
             || [platformLowerString isEqualToString:@"ipad3,2"] //ipad 3
             || [platformLowerString isEqualToString:@"ipad3,3"] //ipad 3
             )
    {
        //ipad 2,3
        //ipad mini
        return FSIOSPlayerDefinitionModeHigh;
    }
    else{
        //iphone 5,5c,5s,6,6 plus
        //ipad 4, air air2
        //ipad mini 2, mini 3
        //ios simulator
        return FSIOSPlayerDefinitionModeSuper;
    }
}
@end

