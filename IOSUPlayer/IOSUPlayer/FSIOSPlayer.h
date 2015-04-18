//
//  FSIOSPlayer.h
//  IOSUPlayerDemo
//
//  Created by Huang Weiqing on 15/1/4.
//  Copyright (c) 2015年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <MediaPlayer/MPMoviePlayerController.h>

//It should be fallowed when player posts message "MPMoviePlayerPlaybackDidFinishNotification"
//You can get value from dictionary which keys include "MPMoviePlayerPlaybackErrorCodeInfoKey"
//You can use it for more dealing with.
MP_EXTERN NSString * const MPMoviePlayerPlaybackErrorCodeInfoKey;

@class FSIOSPlayer;

typedef NS_ENUM(NSInteger, FSIOSPlayerDefinitionMode) {
    FSIOSPlayerDefinitionModeNone,
    FSIOSPlayerDefinitionModeSmooth,
    FSIOSPlayerDefinitionModeStandard,
    FSIOSPlayerDefinitionModeHigh,
    FSIOSPlayerDefinitionModeSuper
};

#pragma mark - Panel
@interface Panel : UIView
@property(nonatomic) CGRect playerBounds;
-(void)setFrame:(CGRect)bounds;
-(void)setFSIOSPlayer:(FSIOSPlayer *)player;
@end

#pragma mark - FSIOSPlayer
@interface FSIOSPlayer : NSObject

#pragma mark - -Method

-(id)init;

- (id)initWithContentURL:(NSURL *)url;

// Prepares the current queue for playback, interrupting any active (non-mixible) audio sessions.
// Automatically invoked when -play is called if the player is not already prepared.
- (void)prepareToPlay;

// Plays items from the current queue, resuming paused playback if possible.
- (void)play;

// Pauses playback if playing.
- (void)pause;

// Ends playback. Calling -play again will start from the beginnning of the queue.
- (void)stop;

// The current playback time of the now playing item in seconds.
// Include two methods
@property (nonatomic, readwrite) NSTimeInterval currentPlaybackTime;

/*
// The seeking rate will increase the longer scanning is active.
- (void)beginSeekingForward;
- (void)beginSeekingBackward;
- (void)endSeeking;
 */

//- (void) seek:(NSTimeInterval)time;

// Returns a thumbnail at the given time.
// Deprecated.  Use -requestThumbnailImagesAtTimes:timeOption: / MPMoviePlayerThumbnailImageRequestDidFinishNotification instead.
//- (UIImage *)thumbnailImageAtTime:(NSTimeInterval)playbackTime timeOption:(MPMovieTimeOption)option;

// Asynchronously request thumbnails for one or more times, provided as an array of NSNumbers (double).
// Posts MPMoviePlayerThumbnailImageRequestDidFinishNotification on completion.
//- (void)requestThumbnailImagesAtTimes:(NSArray *)playbackTimes timeOption:(MPMovieTimeOption)option;
//
// Cancels all pending asynchronous thumbnail requests.
//- (void)cancelAllThumbnailImageRequests;


#pragma mark - -Properties
// -----------------------------------------------------------------------------
// Movie Property Types


@property (nonatomic, readwrite) NSURL *contentURL;

// Returns YES if prepared for playback.
@property(nonatomic, readonly) BOOL isPreparedToPlay;

// The natural size of the movie, or CGSizeZero if not known/applicable.
@property (nonatomic, readonly) CGSize naturalSize;

// The duration of the movie, or 0.0 if not known.
@property (nonatomic, readonly) NSTimeInterval duration;

// The types of media in the movie, or MPMovieMediaTypeNone if not known.
//shows whether it is audio, video or the their combination
@property (nonatomic, readonly) MPMovieMediaTypeMask movieMediaTypes;

// The playback type of the movie. Defaults to MPMovieSourceTypeUnknown.
// Specifying a playback type before playing the movie can result in faster load times.
//shows if is local file or live streaming
@property (nonatomic, readonly) MPMovieSourceType movieSourceType;

// Returns the network load state of the movie player.
@property (nonatomic, readonly) MPMovieLoadState loadState;

// Indicates if a movie should automatically start playback when it is likely to finish uninterrupted based on e.g.
//network conditions. Defaults to YES.
@property (nonatomic, readwrite) BOOL shouldAutoplay;

// The currently playable duration of the movie, for progressively downloaded network content.
@property (nonatomic, readonly) NSTimeInterval playableDuration;

// Returns the current playback state of the movie player.
@property (nonatomic, readonly) MPMoviePlaybackState playbackState;

// Determines how the content scales to fit the view. Defaults to MPMovieScalingModeAspectFit.
@property (nonatomic, readwrite) MPMovieScalingMode scalingMode;

// The view in which the media and playback controls are displayed.
@property (nonatomic, readonly) Panel *view;

// Determines how the movie player repeats when reaching the end of playback. Defaults to MPMovieRepeatModeNone.
@property (nonatomic, readwrite) MPMovieRepeatMode repeatMode;

// The start time of movie playback. Defaults to NaN, indicating the natural start time of the movie.
@property (nonatomic, readwrite) NSTimeInterval initialPlaybackTime;

// The end time of movie playback. Defaults to NaN, which indicates natural end time of the movie.
@property (nonatomic, readwrite) NSTimeInterval endPlaybackTime;


// The current playback rate of the now playing item. Default is 1.0 (normal speed).
// Pausing will set the rate to 0.0. Setting the rate to non-zero implies playing.
//@property(nonatomic) float currentPlaybackRate;

// Determines if the movie is presented in the entire screen (obscuring all other application content). Default is NO.
// Setting this property to YES before the movie player's view is visible will have no effect.
@property (nonatomic, getter=isFullscreen) BOOL fullscreen;

- (void)setFullscreen:(BOOL)fullscreen animated:(BOOL)animated;

// setFrame animation is going to be opened if fullScreenAnimated is YES
@property (nonatomic, readwrite) BOOL fullScreenAnimated;

/********************************************************************/
// A view for customization which is always displayed behind movie content.
@property (nonatomic, readonly) UIView *backgroundView;

// The style of the playback controls. Defaults to MPMovieControlStyleDefault.
@property (nonatomic) MPMovieControlStyle controlStyle;

// Returns YES if the first video frame has been made ready for display for the current item.
// Will remain NO for items that do not have video tracks associated.
@property (nonatomic, readonly) BOOL readyForDisplay;

@property (nonatomic, readwrite) BOOL enableHevcOptimization;

@property (nonatomic, readonly) FSIOSPlayerDefinitionMode definitionMode;

@end




