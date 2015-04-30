//
//  IOSUPlayerController.h
//  IOSUPlayerDemo
//
//  Created by Nick on 14-12-29.
//  Copyright (c) 2014年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FSIOSPlayer.h"

@interface IOSUPlayerController : UIViewController/*(<IOSUPlayerDelegate>*/

-(id)initWithContentURL:(NSString *)url withEnableHevc:(BOOL)enableHevc withHardDecoder:(BOOL)sign;

@end
