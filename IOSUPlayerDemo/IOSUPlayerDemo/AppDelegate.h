//
//  AppDelegate.h
//  IOSUPlayerDemo
//
//  Created by Nick on 14-12-29.
//  Copyright (c) 2014年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property(nonatomic,getter=isIdleTimerDisabled)       BOOL idleTimerDisabled;

@end
