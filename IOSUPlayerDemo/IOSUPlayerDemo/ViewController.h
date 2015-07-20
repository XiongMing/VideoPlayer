//
//  ViewController.h
//  IOSUPlayerDemo
//
//  Created by Nick on 14-12-29.
//  Copyright (c) 2014年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController<UITableViewDelegate, UITableViewDataSource>

-(id)initWithDirectory:(NSString *)directory withEnableHardDecoder:(BOOL)on;

@end
