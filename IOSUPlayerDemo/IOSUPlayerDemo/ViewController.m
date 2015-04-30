//
//  ViewController.m
//  IOSUPlayerDemo
//
//  Created by Nick on 14-12-29.
//  Copyright (c) 2014年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#import "ViewController.h"
#import "IOSUPlayerController.h"
//#import "sys/utsname.h"
//#import <sys/sysctl.h>
//#import <mach/mach.h>

#define PUBULISH_FOR_INTERNAL (true)

@interface ViewController ()
{
    UITextView *_urlText;
    UIButton *_submitBtn;
    UIButton *_clearBtn;
    UIButton *_optimizeBtn;
    UIButton *_unOptimizeBtn;
    UITapGestureRecognizer * _tap;
    UIView      *_topHUD;
    BOOL    _enableHevc;
    
    UITableView *_tableView;
    NSMutableArray *_pathNames;
    NSMutableArray *_paths;
}
@end

@implementation ViewController


- (void)loadView
{
    _enableHevc = YES;
    CGRect bounds = [[UIScreen mainScreen] applicationFrame];
    
    self.view = [[UIView alloc] initWithFrame:bounds];
    self.view.backgroundColor = [UIColor lightGrayColor];
    //    self.view.tintColor = [UIColor blackColor];
    
    self.view.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
    
    
    CGFloat urlTextWidth = bounds.size.width - 8;
    CGFloat urlTextHeight = 60;
    CGFloat urlTextx = bounds.size.width / 10;
    CGFloat urlTexty = bounds.size.height / 2 - 40;
    _urlText = [[UITextView alloc] initWithFrame:CGRectMake(4,
                                                            urlTexty,
                                                            urlTextWidth,
                                                            urlTextHeight)];
    _urlText.font = [UIFont systemFontOfSize:10];
    _urlText.backgroundColor = [UIColor whiteColor];
    _urlText.textColor = [UIColor blackColor];
    _urlText.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    
    
    _topHUD    = [[UIView alloc] initWithFrame:CGRectMake(0,0,0,0)];
    _topHUD.frame = CGRectMake(0, 0, bounds.size.width, urlTexty);
    _topHUD.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleWidth;
    
    
    CGFloat submitBtnTextWidth = bounds.size.width / 5;
    CGFloat submitBtnTextHeight = 24;
    CGFloat submitBtnTextx = bounds.size.width / 7 + 40;
    CGFloat submitBtnTexty = bounds.size.height / 2  + 40;
    
    
    
    _submitBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    _submitBtn.frame = CGRectMake(submitBtnTextx,
                                  submitBtnTexty,
                                  submitBtnTextWidth,
                                  submitBtnTextHeight);
    
    
    _submitBtn.backgroundColor = [UIColor clearColor];
//        _submitBtn.backgroundColor = [UIColor redColor];
    [_submitBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [_submitBtn setTitle:NSLocalizedString(@"播放", nil) forState:UIControlStateNormal];
    _submitBtn.titleLabel.font = [UIFont systemFontOfSize:18];
    _submitBtn.showsTouchWhenHighlighted = YES;
    [_submitBtn addTarget:self action:@selector(submitDidTouch:) forControlEvents:UIControlEventTouchUpInside];
    
    _submitBtn.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleWidth |
    UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
    
    
    CGFloat clearBtnTextWidth = bounds.size.width / 5;
    CGFloat clearBtnTextHeight = 24;
    CGFloat clearBtnTextx = bounds.size.width / 7 * 3;
    CGFloat clearBtnTexty = bounds.size.height / 2  + 40;
    
    _clearBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    _clearBtn.frame = CGRectMake(clearBtnTextx,
                                 clearBtnTexty,
                                 clearBtnTextWidth,
                                 clearBtnTextHeight);
    
    _clearBtn.backgroundColor = [UIColor clearColor];
//        _clearBtn.backgroundColor = [UIColor redColor];
    [_clearBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [_clearBtn setTitle:NSLocalizedString(@"清除", nil) forState:UIControlStateNormal];
    _clearBtn.titleLabel.font = [UIFont systemFontOfSize:18];
    _clearBtn.showsTouchWhenHighlighted = YES;
    [_clearBtn addTarget:self action:@selector(clearDidTouch:) forControlEvents:UIControlEventTouchUpInside];
    
    _clearBtn.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleWidth |
    UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
    
    CGFloat optimizeTextWidth = 80;
    CGFloat optimizeTextHeight = 50;
    CGFloat optimizeTextx = bounds.size.width / 2 - 100;
    CGFloat optimizeTexty = 10;
    _optimizeBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    _optimizeBtn.frame = CGRectMake(optimizeTextx, optimizeTexty, optimizeTextWidth, optimizeTextHeight);
    [_optimizeBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [_optimizeBtn setTitle:NSLocalizedString(@"优化", nil) forState:UIControlStateNormal];
    _optimizeBtn.titleLabel.font = [UIFont systemFontOfSize:21];
    _optimizeBtn.showsTouchWhenHighlighted = YES;
    [_optimizeBtn addTarget:self action:@selector(optimizeTouch:) forControlEvents:UIControlEventTouchUpInside];
    
    _optimizeBtn.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleWidth |
    UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
    
    
    CGFloat unOptimizeTextWidth = 80;
    CGFloat unOptimizeTextHeight = 50;
    CGFloat unOptimizeTextx = bounds.size.width / 2 + 20;
    CGFloat unOptimizeTexty = 10;
    _unOptimizeBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    _unOptimizeBtn.frame = CGRectMake(unOptimizeTextx, unOptimizeTexty, unOptimizeTextWidth, unOptimizeTextHeight);
    //    _optimizeBtn.backgroundColor = [UIColor clearColor];
    //    _optimizeBtn.backgroundColor = [UIColor redColor];
    [_unOptimizeBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [_unOptimizeBtn setTitle:NSLocalizedString(@"非优化 ", nil) forState:UIControlStateNormal];
    _unOptimizeBtn.titleLabel.font = [UIFont systemFontOfSize:21];
    _unOptimizeBtn.showsTouchWhenHighlighted = YES;
    [_unOptimizeBtn addTarget:self action:@selector(unOptimizeTouch:) forControlEvents:UIControlEventTouchUpInside];
    
    _unOptimizeBtn.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleWidth |
    UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
    
    
    _tap = [[UITapGestureRecognizer alloc] initWithTarget:self
                                                   action:@selector(shutDownKeyboard)];
    [_tap setNumberOfTouchesRequired:1];
    
    _optimizeBtn.titleLabel.backgroundColor = [UIColor redColor];
    _unOptimizeBtn.titleLabel.backgroundColor = [UIColor clearColor];
    
    [self initalTableData];
    
    _tableView = [[UITableView  alloc] initWithFrame:CGRectMake(0, 70, bounds.size.width, bounds.size.height - 70)];
    _tableView.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleWidth |
    UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
    [_tableView setDelegate:self];
    [_tableView setDataSource:self];
    _tableView.tableFooterView = [[UIView alloc] init];
    
    
    [self.view addSubview:_topHUD];
    [_topHUD addGestureRecognizer:_tap];
    [self.view addSubview:_topHUD];
//    [self.view addSubview:_urlText];
//    [self.view addSubview:_submitBtn];
//    [self.view addSubview:_clearBtn];
    [self.view addSubview:_optimizeBtn];
    [self.view addSubview:_unOptimizeBtn];
    // _urlText.text = @"http://222.84.164.5/4236DC48E8E268AA1257EC384EAD253AF09BF800/%E6%BE%B3%E9%97%A8%E9%A3%8E%E4%BA%91.mp4";
    
    //[self getDeviceAndOSInfo];
    
    [self.view addSubview:_tableView];
    
}

- (void)initalTableData
{
    NSArray *externsion = [NSArray arrayWithObjects:@"avi", @"rmvb", @"mkv", @"mp4", @"ts", @"mts", @"mpg", @"rm", @"mov", @"flv", nil];
    NSArray *tmpArray  = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSLog(@"%@", tmpArray);
    NSString *documentPath = [tmpArray objectAtIndex:0];
    NSLog(@"documentPath: %@", documentPath);
//    NSArray *documentPaths = [[NSBundle mainBundle] pathsForResourcesOfType:@"mp4" inDirectory:documentPath];
    NSError *error = nil;
    NSArray *filenames = [[NSFileManager defaultManager] subpathsOfDirectoryAtPath:documentPath error:&error];
    if (!error) {
        _pathNames = [[NSMutableArray alloc] init];
        _paths = [[NSMutableArray alloc] init];
        for(int i = 0; i < filenames.count; i++)
        {
            NSString *filename  = [filenames objectAtIndex:i];
            int j = 0;
            for(; j < externsion.count; j++)
                if ([[filename lowercaseString] rangeOfString:[externsion objectAtIndex:j]].length > 0)
                    break;
            
            if (j == externsion.count)
                continue;
            
            [_pathNames addObject:filename];
            [_paths addObject:[documentPath stringByAppendingPathComponent:filename]];
        }
    }
    
    
    
    
//    NSArray *paths = [[NSBundle mainBundle] pathsForResourcesOfType:@"mp4" inDirectory:nil];
//    NSLog(@"paths: %@", paths);
//    _pathNames = [[NSMutableArray alloc] init];
//    _paths = [[NSMutableArray alloc] init];
//    for(int i = 0; i < paths.count; i++)
//    {
//        NSString *path = [paths objectAtIndex:i];
//        NSRange range = [path rangeOfString:@"/" options:NSBackwardsSearch];
//        NSRange mp4Range = [path rangeOfString:@".mp4" options:NSBackwardsSearch];
//        NSString *pathName = [path substringWithRange:NSMakeRange(range.location + 1, mp4Range.location - range.location - 1)];
//        [_pathNames addObject:pathName];
//        [_paths addObject:path];
//        
//    }
//    
    [_pathNames addObject:@"http://192.168.16.128:81/8D5EBA72E161BDC63B5E61EA0264F791D3D47220.mp4"];
    [_paths addObject:@"http://192.168.16.128:81/8D5EBA72E161BDC63B5E61EA0264F791D3D47220.mp4"];

    
}

#pragma mark - UITableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return _pathNames.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    //只是section里面的数据重复利用，防止出现内存泄露
    static NSString *cellIdentifier = @"UITableViewCell";
    
    UITableViewCell *cell = [_tableView dequeueReusableCellWithIdentifier:cellIdentifier];
    
    if (nil == cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                      reuseIdentifier:cellIdentifier];
    }
    
    /*关联数据*/
    cell.textLabel.text = [_pathNames objectAtIndex:indexPath.row];
//    cell.imageView.image = [UIImage imageNamed:@"play24x24.png"];
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    
    return cell;
}

-(BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath{
    return YES;
}

-(void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath{
    
}

#pragma mark - UITableViewDelegate
- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 50.0;
}

- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath{
    
//    cell.imageView.image = [UIImage imageNamed:@"play24x24.png"];
}

- (void)tableView:(UITableView *)tableView didEndDisplayingCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath*)indexPath NS_AVAILABLE_IOS(6_0){
//    cell.imageView.image = nil;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    BOOL sign = NO;
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
//    NSLog(@"---->>当前行：%d",indexPath.row);
    NSString *path = [_paths objectAtIndex:indexPath.row];
    NSRange range = [[path lowercaseString] rangeOfString:@"mp4"];
    NSRange range1 = [[path lowercaseString] rangeOfString:@"mov"];
    if (range.length > 0 || range1.length > 0)
        sign = YES;
//    NSLog(@"path: %@", path);
    IOSUPlayerController *playerController = [[IOSUPlayerController alloc]
                                              initWithContentURL:path withEnableHevc:_enableHevc withHardDecoder:sign];
    [self presentViewController:playerController animated:YES completion:nil];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - operation

-(void) submitDidTouch:(id) sender
{
    NSString *url = [[_urlText text] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];

    
    //NSString *homePath = NSTemporaryDirectory();
   // NSLog(@"homepath:%@,paths%@",homePath,paths);
    
//    if (url.length > 0) {
        NSString *filePath4 = [[NSBundle mainBundle] pathForResource:@"18_standard" ofType:@"mp4"];
        NSString *file1 = @"http://192.168.16.128:81/8D5EBA72E161BDC63B5E61EA0264F791D3D47220.mp4";
        NSString *file2 = @"http://111.63.135.117/77046AB4F816D76F2648505ED6E8571C1D1DFF6E.mp4";
        NSString *file3 = @"http://120.131.127.36:1186/0000000000000000000000000000000000000411.m3u8";
//        IOSUPlayerController *playerController = [[IOSUPlayerController alloc] initWithContentURL:file1 withEnableHevc:_enableHevc];
//        [self presentViewController:playerController animated:YES completion:nil];
        NSLog(@"child controller : %d", self.childViewControllers.count);
//    }else{
//        NSLog(@"nil path");
//    }
    
}

//- (void)getDeviceAndOSInfo
//{
//    //here use sys/utsname.h
//    struct utsname systemInfo;
//    uname(&systemInfo);
//    //get the device model and the system version
//    Fsp265PlayerLog(@"iphone 4还是iphone 5： %@", [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding]);
//    Fsp265PlayerLog(@"系统类型：%@", [[UIDevice currentDevice] systemVersion]);
//
//    host_basic_info_data_t hostInfo;
//    mach_msg_type_number_t infoCount = HOST_BASIC_INFO_COUNT;
//    kern_return_t ret = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostInfo, &infoCount);
//
//    //CPU_SUBTYPE_ARM_V5TEJ
//    //#define CPU_SUBTYPE_ARM_V5TEJ           ((cpu_subtype_t) 7)
//    //#define CPU_SUBTYPE_ARM_V7F		((cpu_subtype_t) 10) /* Cortex A9 */
//
//    if (ret == KERN_SUCCESS) {
//        Fsp265PlayerLog(@"cpu type: %d", hostInfo.cpu_subtype);
//              //hostInfo.CPU_SUBTYPE_XEON);
//    }
//}

-(void)clearDidTouch:(id)sender
{
    _urlText.text = @"";
}

-(void) shutDownKeyboard
{
    //[self.view resignFirstResponder];
    [_urlText resignFirstResponder];
}

-(void) optimizeTouch:(id)sender
{
//    NSString *documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
//    NSLog(@"ssss: %@", documentPath);
//    NSArray *documentPaths = [[NSBundle mainBundle] pathsForResourcesOfType:@"mp4" inDirectory:documentPath];
//    NSLog(@"tmpPaths: %@", documentPaths);
    
    _optimizeBtn.titleLabel.backgroundColor = [UIColor redColor];
    _unOptimizeBtn.titleLabel.backgroundColor = [UIColor clearColor];
    _enableHevc = YES;
}

-(void) unOptimizeTouch:(id)sender
{
    _unOptimizeBtn.titleLabel.backgroundColor = [UIColor redColor];
    _optimizeBtn.titleLabel.backgroundColor = [UIColor clearColor];
    _enableHevc = NO;
}

-(void)dealloc
{
}


@end
