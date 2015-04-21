//
//  IOSUPlayerController.m
//  IOSUPlayerDemo
//
//  Created by Nick on 14-12-29.
//  Copyright (c) 2014年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#import "IOSUPlayerController.h"
#import <MediaPlayer/MPVolumeView.h>
#import "uerror_code.h"

@interface  IOSUPlayerController ()
{
    NSURL *_url;
    UITextView *_urlText;
    UIButton *_doneButton;
    UITapGestureRecognizer *_tap;
    
    UIToolbar           * _bottomBar;
    UIToolbar           * _topBar;
    UIBarButtonItem     *_playBtn;
    UIBarButtonItem     *_pauseBtn;
    UIBarButtonItem     *_rewindBtn;
    UIBarButtonItem     *_forwardBtn;
    UIBarButtonItem     *_spaceItem;
    UIBarButtonItem     *_fixedSpaceItem;
    UIBarButtonItem     *_contentModeBtn;
    UIBarButtonItem     *_repeatModeBtn;
    UIBarButtonItem     *_fullscreenBtn;
    UIBarButtonItem     *_destroyBtn;
    
    UIView              *_airPlayeBtn;
    MPVolumeView        *_volumeView;
    
    UIView              *_topHUD;
    
    UILabel             *_leftLabel;
    UISlider            *_progressSlider;
    NSTimer             *_progressTimer;
    NSTimeInterval      _previousPlaybackTime;
    UILabel             *_rightLabel;
    UIActivityIndicatorView *_activityIndicator;
    
    BOOL                _fspPlayerLoadFinished;
    BOOL                _hidden;
    
    FSIOSPlayer *   _player;
    BOOL        _enableHevc;
    
    
}

//@property(nonatomic) FSIOSPlayer *player;

@end

@implementation IOSUPlayerController

//+(void)initialize
//{
//    if (!gHistory)
//        gHistory = [NSMutableDictionary dictionary];
//}

-(id)initWithContentURL:(NSString *)url withEnableHevc:(BOOL)enableHevc
{
    _enableHevc = enableHevc;
    self = [super init];
    if (!self) {
        return nil;
    }
    _url = [NSURL URLWithString:[url stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    
//    NSLog(@"%@", [_url.absoluteString stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding]);
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(appWillEnterBackGroundAction)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(appWillEnterForeGroundAction)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
    
    
    

    //---------------------------------------------
    //add player observer
    
    
    NSLog(@"IOSPlayer: %@", _player);
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(loadStateChanged:)
                                                 name:MPMoviePlayerLoadStateDidChangeNotification
                                               object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(durationAvailable:)
                                                 name:MPMovieDurationAvailableNotification
                                               object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(naturalSizeAvailable:)
                                                 name:MPMovieNaturalSizeAvailableNotification
                                               object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(mediaTypesAvailable:)
                                                 name:MPMovieMediaTypesAvailableNotification
                                               object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sourceTypeAvailable:)
                                                 name:MPMovieSourceTypeAvailableNotification
                                               object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playbackDidFinish:)
                                                 name:MPMoviePlayerPlaybackDidFinishNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playbackStateChange:)
                                                 name:MPMoviePlayerPlaybackStateDidChangeNotification
                                               object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(scalingModeChange:)
                                                 name:MPMoviePlayerScalingModeDidChangeNotification
                                               object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(willEnterFullScreen:)
                                                 name:MPMoviePlayerWillEnterFullscreenNotification object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didEnterFullScreen:)
                                                 name:MPMoviePlayerDidEnterFullscreenNotification object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(willExitFullScreen:)
                                                 name:MPMoviePlayerWillExitFullscreenNotification object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didExitFullScreen:)
                                                 name:MPMoviePlayerDidExitFullscreenNotification object:_player];
    
    return self;
}

- (void)appWillEnterBackGroundAction
{
    [_player pause];
    //防止视频显示的view被controller引用析构不了
//        [self appWillEnterBackGroundActionOrDoneDidTouch];
    //     NSArray * subViews = [self.view subviews];
    //    if (subViews.count > 0) {
    //        [[subViews objectAtIndex:0] removeFromSuperview];
    //    }
    
}

-(void) appWillEnterForeGroundAction
{
    [_player play];
//        [self loadPlayer];
//
}


#pragma mark - rewrite
-(void)loadView
{
    
    [[UIApplication sharedApplication] setStatusBarHidden:YES];
    
    CGRect bounds = [[UIScreen mainScreen] applicationFrame];
    
    self.view = [[UIView alloc] initWithFrame:bounds];
    self.view.backgroundColor = [UIColor lightGrayColor];
    self.view.autoresizingMask = UIViewAutoresizingFlexibleTopMargin |
    UIViewAutoresizingFlexibleBottomMargin |
    UIViewAutoresizingFlexibleLeftMargin |
    UIViewAutoresizingFlexibleRightMargin;
    
    
    CGFloat topH = 50;
    CGFloat botH = 50;
    
    
    
    _doneButton = [UIButton buttonWithType:UIButtonTypeCustom];
    _doneButton.frame = CGRectMake(4, 14, 40, 25);
    [_doneButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [_doneButton setTitle:NSLocalizedString(@"退出", nil) forState:UIControlStateNormal];
    _doneButton.titleLabel.font = [UIFont systemFontOfSize:12];
    _doneButton.showsTouchWhenHighlighted = YES;
    [_doneButton addTarget:self action:@selector(doneDidTouch:) forControlEvents:UIControlEventTouchUpInside];
    
    
    _topHUD    = [[UIView alloc] initWithFrame:CGRectMake(0,0,0,0)];
    _topHUD.frame = CGRectMake(0, 0, bounds.size.width, topH);
    _topHUD.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    _topHUD.backgroundColor = [UIColor clearColor];
    _topHUD.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    
    
    _leftLabel = [[UILabel alloc] initWithFrame:CGRectMake(45, 1, 50, topH)];
    _leftLabel.backgroundColor = [UIColor clearColor];
    _leftLabel.opaque = NO;
    _leftLabel.adjustsFontSizeToFitWidth = NO;
    _leftLabel.textAlignment = NSTextAlignmentLeft;
    _leftLabel.textColor = [UIColor blackColor];
    _leftLabel.text = @"";
    _leftLabel.font = [UIFont systemFontOfSize:12];
    _leftLabel.autoresizingMask = UIViewAutoresizingFlexibleRightMargin;
    
    
    _rightLabel = [[UILabel alloc] initWithFrame:CGRectMake(bounds.size.width - 55, 1, 50, topH)];
    _rightLabel.backgroundColor = [UIColor clearColor];
    _rightLabel.opaque = NO;
    _rightLabel.adjustsFontSizeToFitWidth = NO;
    _rightLabel.textAlignment = NSTextAlignmentLeft;//NSTextAlignmentLeft;
    _rightLabel.textColor = [UIColor blackColor];
    _rightLabel.text = @"";
    _rightLabel.font = [UIFont systemFontOfSize:12];
    _rightLabel.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin;
    
    
    _progressSlider = [[UISlider alloc] initWithFrame:CGRectMake(100, 2, bounds.size.width - 160, topH)];
    _progressSlider.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    _progressSlider.continuous = NO;
    _progressSlider.value = 0;
    [_progressSlider addTarget:self action:@selector(progressDidChange:) forControlEvents:UIControlEventValueChanged];
    
    _topBar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, bounds.size.width, topH)];
    _topBar.backgroundColor = [UIColor darkGrayColor];
    _topBar.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    
    /************************************************************************************************************/
    
    _activityIndicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle: UIActivityIndicatorViewStyleWhiteLarge];
    _activityIndicator.center = self.view.center;
    _activityIndicator.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
    
    [self.view addSubview:_activityIndicator];
    
    
    /************************************************************************************************************/
    CGFloat width = bounds.size.width;
    CGFloat height = bounds.size.height;
    _bottomBar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, height-botH, width, botH)];
    _bottomBar.backgroundColor = [UIColor darkGrayColor];
    _bottomBar.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleWidth;
    
    _rewindBtn = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRewind
                                                               target:self
                                                               action:@selector(rewindTouch:)];
    _rewindBtn.width = 25;
    
    _playBtn = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemPlay
                                                             target:self
                                                             action:@selector(playTouch:)];
    _playBtn.width = 25;
    
    
    _pauseBtn = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemPause
                                                              target:self
                                                              action:@selector(pauseTouch:)];
    _playBtn.width = 25;
    
    
    _forwardBtn = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFastForward
                                                                target:self
                                                                action:@selector(forwardTouch:)];
    _forwardBtn.width = 25;
    
    
    _contentModeBtn = [[UIBarButtonItem alloc] initWithTitle:@"裁边"
                                                       style:UIBarButtonItemStyleDone
                                                      target:self
                                                      action:@selector(contentModeTouch:)];
    
    _contentModeBtn.width = 25;
    
    
    _fullscreenBtn = [[UIBarButtonItem alloc] initWithTitle:@"全屏"
                                                      style:UIBarButtonItemStyleDone
                                                     target:self
                                                     action:@selector(fullscreenTouch:)];
    _fullscreenBtn.width = 25;
    
    _repeatModeBtn = [[UIBarButtonItem alloc] initWithTitle:@"循环"
                                                      style:UIBarButtonItemStyleDone
                                                     target:self
                                                     action:@selector(repeatModeTouch:)];
    _repeatModeBtn.width = 25;
    
    
    _destroyBtn = [[UIBarButtonItem alloc] initWithTitle:@"销毁"
                                                      style:UIBarButtonItemStyleDone
                                                     target:self
                                                     action:@selector(destroyTouch:)];
    _destroyBtn.width = 25;
    
    
    
    _spaceItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                               target:nil
                                                               action:nil];
    
    _fixedSpaceItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace
                                                                    target:nil
                                                                    action:nil];
    
    _spaceItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                               target:nil
                                                               action:nil];
    
    _fixedSpaceItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace
                                                                    target:nil
                                                                    action:nil];
    
    
    _fixedSpaceItem.width = 25;
    
    _airPlayeBtn = [self createAirplayBtn];
    
    [self.view addSubview:_airPlayeBtn];
    
    [_topHUD addSubview:_doneButton];
    [_topHUD addSubview:_progressSlider];
    [_topHUD addSubview:_leftLabel];
    [_topHUD addSubview:_rightLabel];
    
    [_topBar addSubview:_topHUD];
    [self.view addSubview:_bottomBar];
    [self.view addSubview:_topBar];
    [_bottomBar setItems:@[_spaceItem, _rewindBtn, _spaceItem, _playBtn,
                           _spaceItem, _forwardBtn, _spaceItem,
                           _contentModeBtn, _spaceItem, _repeatModeBtn,
                           _spaceItem, _fullscreenBtn, _spaceItem, _destroyBtn,
                           _spaceItem,] animated:NO];
    
    
    [self setUIViewShow];
    
    _tap =[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(tapOperation)];
    _tap.numberOfTapsRequired = 2;
    _tap.numberOfTouchesRequired = 1;
    
    [self loadPlayer];
    
    
}

-(void)setUIViewShow
{
    _topBar.hidden = NO;
    _doneButton.hidden = NO;
    _progressSlider.hidden = YES;
    _bottomBar.hidden = YES;
    _hidden = YES;
}

-(void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
//    if (_fspPlayerLoadFinished) {
//        
//        UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
//        
//        if (_player.isFullscreen) {
//            [_player setFullscreen:YES animated:NO];
//        }else{
//            [_player setFullscreen:NO animated:NO];
//        }
//        /*
//         UIDeviceOrientationPortrait
//         UIDeviceOrientationPortraitUpsideDown
//         */
//        
//        int width = [[UIScreen mainScreen] bounds].size.width;
//        int height = [[UIScreen mainScreen] bounds].size.height;
//        NSLog(@"after rorating: current width: %d, height: %d", width, height);
//        //[_player.view setFrame:CGRectMake(0, 0, width, height)];
////        _player.view.backgroundColor = [UIColor redColor];
//        
//        
//    }
    
    
}

- (BOOL)prefersStatusBarHidden { return YES; }

-(void)viewWillDisappear:(BOOL)animated
{
    NSLog(@"viewWillDisappear");
}


-(void)viewDidDisappear:(BOOL)animated
{
    NSLog(@"viewDidDisappear");
}

-(void)viewDidAppear:(BOOL)animated
{
    NSLog(@"viewDidAppear");
}

#pragma mark - Touch

-(void) doneDidTouch:(id) sender
{
    [self appWillEnterBackGroundActionOrDoneDidTouch];
    
    if (self.presentingViewController || !self.navigationController)
        [self dismissViewControllerAnimated:YES completion:nil];
    else
        [self.navigationController popViewControllerAnimated:YES];
}

-(void)playTouch:(id)sender
{
    NSLog(@"play touch");
    [_player play];
}

-(void)pauseTouch:(id)sender
{
    NSLog(@"pause touch");
    [_player pause];
}

-(void) forwardTouch:(id)sender
{
    NSLog(@"forwardTouch");
//    [_player seek:([_player currentPlaybackTime] + 40)];
    _player.currentPlaybackTime = (_player.currentPlaybackTime + 40);
}

-(void)contentModeTouch:(id)sender
{
    
    NSLog(@"contentModeTouch");
    if (MPMovieScalingModeAspectFit == _player.scalingMode) {
        
        _player.scalingMode = MPMovieScalingModeAspectFill;
        _contentModeBtn.title = @"填充";
        
    }else if( MPMovieScalingModeAspectFill == _player.scalingMode){
        
        _player.scalingMode = MPMovieScalingModeFill;
        _contentModeBtn.title = @"黑边";
        
    }else{
        
        _player.scalingMode = MPMovieScalingModeAspectFit;
        _contentModeBtn.title = @"裁边";
        
    }
    
}

-(void)progressDidChange:(id)sender
{
    NSLog(@"progressDidChange");
    UISlider *slider = sender;
    _player.currentPlaybackTime = (slider.value * _player.duration);
    
}

-(void) rewindTouch:(id)sender
{
    NSLog(@"rewindTouch");
    _player.currentPlaybackTime = (_player.currentPlaybackTime - 40);
    
}

-(void)fullscreenTouch:(id)sender{
    if (_player.isFullscreen == YES) {
        [_player setFullscreen:NO animated:YES];
        [_player.view setFrame:CGRectMake(5, 40, 300, 200)];
        _fullscreenBtn.tintColor = [UIColor blueColor];
        
    }else{
        [_player setFullscreen:YES animated:YES];
        _fullscreenBtn.tintColor = [UIColor redColor];
    }
}

-(void)destroyTouch:(id)sender{
    [self appWillEnterBackGroundActionOrDoneDidTouch];
    _leftLabel.hidden = YES;
    _rightLabel.hidden = YES;
}

-(void)repeatModeTouch:(id)sender{
    
    if(_player.repeatMode == MPMovieRepeatModeNone){
        _player.repeatMode = MPMovieRepeatModeOne;
        _repeatModeBtn.tintColor = [UIColor redColor];
    }else{
        _player.repeatMode = MPMovieRepeatModeNone;
        _repeatModeBtn.tintColor = [UIColor blueColor];
    }
        
}


-(void)tapOperation
{
    NSLog(@"tapOperation");
    if (_fspPlayerLoadFinished) {
        if (_hidden ) {
            _topBar.hidden = NO;
            _bottomBar.hidden = NO;
            _hidden = NO;
        }else{
            _topBar.hidden = YES;
            _bottomBar.hidden = YES;
            _hidden = YES;
        }
    }else{
        _topBar.hidden = NO;
        _doneButton.hidden = NO;
    }
    
}


#pragma mark - observer
-(void)loadStateChanged:(NSNotification *)notification{
    
    if (_player.loadState & MPMovieLoadStatePlayable
        && !_fspPlayerLoadFinished) {
        NSLog(@"================================");
        _fspPlayerLoadFinished = YES;
        _hidden = NO;
        _progressSlider.hidden = NO;
        _bottomBar.hidden = NO;
        _leftLabel.text = formatTimeInterval(0, NO);
//        if (!_player.shouldAutoplay) {
//            [_player play];
//        }
        
        
    }
    
    if (_player.loadState == (MPMovieLoadStatePlayable | MPMovieLoadStateStalled)) {
        _activityIndicator.hidden = NO;
        [_activityIndicator startAnimating];
        NSLog(@"Observer buffer begin");
    }
    
    if (_player.loadState == (MPMovieLoadStatePlayable | MPMovieLoadStatePlaythroughOK)) {
        _activityIndicator.hidden = YES;
        [_activityIndicator stopAnimating];
        NSLog(@"Observer buffer end");
    }
}

-(void)durationAvailable:(NSNotification *)notification{
    
    _rightLabel.text = formatTimeInterval(_player.duration, NO);
}

-(void)naturalSizeAvailable:(NSNotification *)notification{
    NSLog(@"Observer natural size: (%f, %f)", _player.naturalSize.width, _player.naturalSize.height);
}

-(void)mediaTypesAvailable:(NSNotification *)notification{
    
    if (_player.movieMediaTypes & MPMovieMediaTypeMaskAudio){
        NSLog(@"Observer MPMovieMediaTypeMaskAudio");
    }
    
    if (_player.movieMediaTypes & MPMovieMediaTypeMaskVideo) {
        NSLog(@"Observer MPMovieMediaTypeMaskVideo");
    }
    
    if (!(_player.movieMediaTypes | MPMovieMediaTypeMaskNone)) {
        NSLog(@"Observer MPMovieMediaTypeMaskNone");
    }
}

-(void)sourceTypeAvailable:(NSNotification *)notification{
    if (_player.movieSourceType == MPMovieSourceTypeFile) {
        
        NSLog(@"Observer MPMovieSourceTypeFile");
    }else if(_player.movieSourceType == MPMovieSourceTypeStreaming){
        
        NSLog(@"Observer MPMovieSourceTypeStreaming");
    }else{
        
        NSLog(@"Observer MPMovieSourceTypeUnknown");
    }
}

-(void)playbackDidFinish:(NSNotification *)notification{
    
    
    
    NSInteger reason = [[notification.userInfo valueForKey:MPMoviePlayerPlaybackDidFinishReasonUserInfoKey] integerValue];
    
//    NSString *info = MPMovieFinishReasonPlaybackEnded == reason ? @"正常播放结束" : MPMovieFinishReasonUserExited == reason ? @"用户主动退出" : @"播放错误";
    NSInteger errorCode = [[notification.userInfo valueForKey:MPMoviePlayerPlaybackErrorCodeInfoKey] integerValue];
    NSString *info = nil;
    if (MPMovieFinishReasonPlaybackEnded == reason) {
        info = @"正常播放结束";
    }else if(MPMovieFinishReasonUserExited == reason){
        info = @"用户主动退出";
    }else{
        switch (errorCode) {
            case ERROR_SOFT_PLAYER_DNS_TIMEOUT:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_DNS_TIMEOUT"];
                break;
            case ERROR_SOFT_PLAYER_DNS_FAILED:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_DNS_FAILED"];
                break;
            case ERROR_SOFT_PLAYER_HTTP_3XX:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_HTTP_3XX"];
                break;
            case ERROR_SOFT_PLAYER_HTTP_4XX:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_HTTP_4XX"];
                break;
            case ERROR_SOFT_PLAYER_PREPARE_TIMEOUT:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_PREPARE_TIMEOUT"];
                break;
            case ERROR_SOFT_PLAYER_UNSUPPORTED:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_UNSUPPORTED"];
                break;
            case ERROR_SOFT_PLAYER_ADDRESS_NULL:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_ADDRESS_NULL"];
                break;
            case ERROR_SOFT_PLAYER_SEEK_FAILED:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_SEEK_FAILED"];
                break;
            case ERROR_SOFT_PLAYER_PREPARE_ERROR:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_PREPARE_ERROR"];
                break;
            case ERROR_SOFT_PLAYER_SOCKET_CONNECT_TIMEOUT:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_SOCKET_CONNECT_TIMEOUT"];
                break;
            case ERROR_SOFT_PLAYER_NETWORK_DISCONNECTED:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_NETWORK_DISCONNECTED"];
                break;
            case ERROR_SOFT_PLAYER_AV_READ_FRAME_FAILED:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_AV_READ_FRAME_FAILED"];
                break;
            case ERROR_SOFT_PLAYER_INITI_EGL_FAILED:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_INITI_EGL_FAILED"];
                break;
            case ERROR_SOFT_PLAYER_LOAD_LIBFSPLAYER_FAILED:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_LOAD_LIBFSPLAYER_FAILED"];
                break;
            case ERROR_SOFT_PLAYER_NO_MEMORY:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_NO_MEMORY"];
                break;
            case ERROR_SOFT_PLAYER_FFMPEG:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_FFMPEG"];
                break;
            case ERROR_SOFT_PLAYER_OPENSLES:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_OPENSLES"];
                break;
            case ERROR_SOFT_PLAYER_BAD_INVOKE:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_BAD_INVOKE"];
                break;
            case ERROR_SOFT_PLAYER_END:
                info = [NSString stringWithFormat:@"播放错误: %@", @"ERROR_SOFT_PLAYER_END"];
                break;
            default:
                info = [NSString stringWithFormat:@"播放错误: %d", errorCode];
                break;
        }
    }
    
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:
                              NSLocalizedString(@"play", nil)
                                                        message:info
                                                       delegate:nil
                                              cancelButtonTitle:NSLocalizedString(@"Close", nil)
                                              otherButtonTitles:nil];
    
    [alertView show];
}

-(void)playbackStateChange:(NSNotification *)notification{
    switch (_player.playbackState) {
        case MPMoviePlaybackStateInterrupted:
            NSLog(@"Observer MPMoviePlaybackStateInterrupted");
            break;
        case MPMoviePlaybackStatePaused:
            NSLog(@"Observer MPMoviePlaybackStatePaused");
            break;
        case MPMoviePlaybackStatePlaying:
            NSLog(@"Observer MPMoviePlaybackStatePlaying");
            break;
        case MPMoviePlaybackStateSeekingBackward:
            NSLog(@"Observer MPMoviePlaybackStateSeekingBackward");
            break;
        case MPMoviePlaybackStateSeekingForward:
            NSLog(@"Observer MPMoviePlaybackStateSeekingForward");
            break;
        case MPMoviePlaybackStateStopped:
            NSLog(@"Observer MPMoviePlaybackStateStopped");
            break;
        default:
            NSLog(@"Observer playbackState: %d", _player.playbackState);
            break;
    }
}

-(void)scalingModeChange:(NSNotification *)notification{
    switch (_player.scalingMode) {
        case MPMovieScalingModeAspectFill:
            NSLog(@"Observer crop");
            break;
        case MPMovieScalingModeFill:
            NSLog(@"Observer transform");
            break;
        case MPMovieScalingModeAspectFit:
            NSLog(@"Observer common");
            break;
        default:
            NSLog(@"Observer MPMovieScalingModeNone");
            break;
    }
}

#pragma mark - private

- (void) loadPlayer
{
    _progressTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self
                                                    selector:@selector(updateProgressValue) userInfo:nil repeats:YES];
//    _player = [[FSIOSPlayer alloc] initWithContentURL:_url];
    _player = [[FSIOSPlayer alloc] init];
    
    _player.enableHevcOptimization = _enableHevc;
    
    _player.contentURL = _url;
//    _player.view.frame = self.view.bounds;
    [_player setFullscreen:YES animated:YES];
    _player.shouldAutoplay = YES;
    _player.initialPlaybackTime = 0;
    _player.endPlaybackTime = 0;
//    _player.allowsAirPlay = YES;
    [_player prepareToPlay];
    [self.view insertSubview:_player.view atIndex:0];
    [_player.view addGestureRecognizer:_tap];
}


-(void)appWillEnterBackGroundActionOrDoneDidTouch
{
    [self setUIViewShow];
    
    if (_progressTimer) {
        [_progressTimer invalidate];
        _progressTimer = nil;
    }
    //    if (_fspPlayerLoadFinished) {
    //        if (_player.currentPlaybackTime == 0 || abs(_player.currentPlaybackTime  - _player.duration) <=10)
    //            [gHistory removeObjectForKey:_player.contentURL];
    //        else
    //            [gHistory setValue:[NSNumber numberWithFloat:[_player currentPlaybackTime]] forKey:_player.contentURL];
    //    }
    //    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, NULL), ^{
    [_player stop];
    _player = nil;
    _fspPlayerLoadFinished = NO;
    printf("appWillEnterBackGroundActionOrDoneDidTouch\n");
    //    });
    
}

-(void)updateProgressValue
{
    /*更新进度条*/
    NSTimeInterval currentPlaybackTime = _player.currentPlaybackTime;
    CGFloat currentProgressValue =  currentPlaybackTime / _player.duration;
    if (currentProgressValue != _progressSlider.value) {
        _progressSlider.value = currentProgressValue;
    }
    
    /*跟新播放时间*/
    if(_previousPlaybackTime != _player.currentPlaybackTime)
    {
        _leftLabel.text = formatTimeInterval(currentPlaybackTime, NO);
    }
    
    //更新按钮操作
    [self updateBottomBar];
}

-(void)updateBottomBar
{
    UIBarButtonItem *playPauseBtn = nil;
    if (_player.playbackState == MPMoviePlaybackStatePaused) {
        playPauseBtn = _playBtn;
    }else{
        playPauseBtn = _pauseBtn;
    }
    
    [_bottomBar setItems:@[_spaceItem, _rewindBtn, _spaceItem, playPauseBtn,
                           _spaceItem, _forwardBtn, _spaceItem,
                           _contentModeBtn, _spaceItem,
                           _repeatModeBtn, _spaceItem,
                           _fullscreenBtn, _spaceItem,
                           _destroyBtn, _spaceItem] animated:YES];
    
}

- (UIView*)createAirplayBtn
{
    int width = 100;
    int height = 100;
    UIView *baseView = [[UIView alloc] initWithFrame:CGRectMake(400, 400, width, height)];
    
    _volumeView = [[MPVolumeView alloc] initWithFrame:CGRectMake(-7, -2, width + 7, height + 2)];
    _volumeView.showsVolumeSlider = NO;
    [baseView addSubview:_volumeView];
    for (int i = 0; i < [[_volumeView subviews] count]; ++i) {
        UIButton *item = [[_volumeView subviews] objectAtIndex:i];
        if ([item isKindOfClass:[UIButton class]]) {
            item.bounds = CGRectMake(0, 0, width, height);
            break;
        }
    }
    
    return baseView;
}

static NSString * formatTimeInterval(CGFloat seconds, BOOL isLeft)
{
    
    seconds = MAX(0, seconds);
    
    NSInteger s = seconds;
    NSInteger m = s / 60;
    NSInteger h = m / 60;
    
    s = s % 60;
    m = m % 60;
    
    NSMutableString *format = [(isLeft && seconds >= 0.5 ? @"-" : @"") mutableCopy];
    [format appendFormat:@"%0.2d:%0.2d:%0.2d", h, m, s];
    return format;
}


-(void)willEnterFullScreen:(NSNotification *)notification{
//    sleep(4);
    NSLog(@"==========willEnterFullScreen");
}

-(void)didEnterFullScreen:(NSNotification *)notification{
    if (notification.userInfo) {
        NSTimeInterval animationGap = [[notification.userInfo
                                        objectForKey:MPMoviePlayerFullscreenAnimationDurationUserInfoKey] doubleValue];
        NSLog(@"animation: %f", animationGap);
    }
    NSLog(@"==========didEnterFullScreen");
}

-(void)willExitFullScreen:(NSNotification *)notification{
//    sleep(4);
    NSLog(@"==========willExitFullScreen");
}

-(void)didExitFullScreen:(NSNotification *)notification{
    if (notification.userInfo) {
        NSTimeInterval animationGap = [[notification.userInfo
                                        objectForKey:MPMoviePlayerFullscreenAnimationDurationUserInfoKey] doubleValue];
        NSLog(@"animation: %f", animationGap);
    }
    NSLog(@"==========didExitFullScreen");
}

-(void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    NSLog(@"%@ dealloc FspPlayerController", self);
    
    [[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
}


@end