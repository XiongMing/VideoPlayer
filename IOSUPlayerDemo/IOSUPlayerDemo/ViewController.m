//
//  ViewController.m
//  IOSUPlayerDemo
//
//  Created by Nick on 14-12-29.
//  Copyright (c) 2014年 Funshion Online Technologies,.Ltd. All rights reserved.
//

#import "ViewController.h"
#import "IOSUPlayerController.h"

bool APPFORITUNES = true;


@interface BackGroundView : UIView

@end

@implementation BackGroundView

+(Class)layerClass{
    return [CAGradientLayer class];
}

@end


@interface ViewController ()
{
    
    UIView      *_closeHardDecoderView;
    UISwitch    *_closeHardDecoderSwith;
    UILabel     *_closeHardDecoerLabel;
    
    UILabel     *_optimizationLabel;
    UISwitch    *_optimizationSwitch;
    UIView      *_optimizationView;
    CAGradientLayer *_gradient;
    BackGroundView *_backgroundView1;
    
    BOOL    _enableHevc;
    
    UITableView *_tableView;
    NSMutableArray *_pathNames;
    NSMutableArray *_paths;
    
    UITableViewCellEditingStyle _editingStyle;
}
@end

@implementation ViewController


- (void)viewDidLoad
{
    [super viewDidLoad];
    _enableHevc = YES;
    CGRect bounds = [[UIScreen mainScreen] applicationFrame];
    bounds.size.width = [self getFullScreenSize].width;
    bounds.size.height = [self getFullScreenSize].height;
    
    [self initialization];
    
    [self setSelfViewProperty:bounds];
    
    [self createHardDecoder:bounds];
    [self createOptimization:bounds];
    [self setTableProperty:bounds];
    [self createSubView:bounds];
//    [self createSubLayer:bounds];
    
    
    [self.view addSubview:_tableView];
    [self.view addSubview:_closeHardDecoderView];

    if (!APPFORITUNES) {
        [self.view addSubview:_optimizationView];
    }
    
//    [self.view.layer insertSublayer:_gradient atIndex:0];
    [self.view insertSubview:_backgroundView1 atIndex:0];
    
    [self initalTableData];
    NSLog(@"viewDidLoad called");
}


-(void)viewDidUnload{
    [super viewDidUnload];
    NSLog(@"viewDidUnload called");
}

-(void)loadView{
    [super loadView];
    NSLog(@"loadView called");
}

-(void)viewWillAppear:(BOOL)animated{
    [super viewWillAppear:animated];
    NSLog(@"viewWillAppear called");
}

-(void)viewDidAppear:(BOOL)animated{
    [super viewDidAppear:animated];
    NSLog(@"viewDidAppear called");
}

-(void)viewWillDisappear:(BOOL)animated{
    [super viewWillDisappear:animated];
    NSLog(@"viewWillDisappear called");
}

-(void)viewDidDisappear:(BOOL)animated{
    [super viewDidDisappear:animated];
    NSLog(@"viewDidDisappear called");
}

//-(void)viewDidLayoutSubviews{
//    [super viewDidLayoutSubviews];
//    CGRect bounds = [[UIScreen mainScreen] applicationFrame];
//    bounds.size.width = [self getFullScreenSize].width;
//    bounds.size.height = [self getFullScreenSize].height;
//    _gradient.frame = CGRectMake(0, 0, bounds.size.width, bounds.size.height);
//}

#pragma mark - creation ui
-(void)setSelfViewProperty:(CGRect)bounds{
    self.view = [[UIView alloc] initWithFrame:bounds];
    self.view.backgroundColor = [UIColor brownColor];
    self.view.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
}

-(void)createHardDecoder:(CGRect)bounds{
    CGFloat closeHarderLableWidth = 80;
    CGFloat closeHarderSwitchWidth = 40;
    CGFloat closeHarderHeight = 50;
    _closeHardDecoerLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, closeHarderLableWidth, 50)];
    _closeHardDecoerLabel.text = @"关闭硬解";
    _closeHardDecoerLabel.textColor = [UIColor whiteColor];
    _closeHardDecoderSwith = [[UISwitch alloc] initWithFrame:CGRectMake(closeHarderLableWidth, 10, closeHarderSwitchWidth, closeHarderHeight)];
    [_closeHardDecoderSwith addTarget:self action:@selector(switchChanged:) forControlEvents:UIControlEventValueChanged];
    [_closeHardDecoderSwith setOn:NO];
    _closeHardDecoderSwith.onTintColor = [UIColor lightGrayColor];
    CGFloat closeHarderWidth = closeHarderLableWidth + closeHarderSwitchWidth + 20;
    CGFloat cloaseHardDecoderViewX = bounds.size.width / 2 - 20 - closeHarderWidth - 20;
    if (APPFORITUNES) {
        cloaseHardDecoderViewX = bounds.size.width/2 - closeHarderWidth/2;
    }
    _closeHardDecoderView = [[UIView alloc] initWithFrame:CGRectMake(cloaseHardDecoderViewX, 30, closeHarderWidth, closeHarderHeight)];
    [_closeHardDecoderView addSubview:_closeHardDecoerLabel];
    [_closeHardDecoderView addSubview:_closeHardDecoderSwith];
    _closeHardDecoderView.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleLeftMargin;
}

-(void)createOptimization:(CGRect)bounds{
    CGFloat optimizationLableWidth = 100;
    CGFloat optimizationSwitchWidth = 40;
    CGFloat optimizationHeght = 50;
    _optimizationLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, optimizationLableWidth, 50)];
    _optimizationLabel.text = @"启用265优化";
    _optimizationSwitch = [[UISwitch alloc] initWithFrame:CGRectMake(optimizationLableWidth, 10,  optimizationSwitchWidth, optimizationHeght)];
    [_optimizationSwitch addTarget:self action:@selector(optimizationChanged:) forControlEvents:UIControlEventValueChanged];
    _optimizationSwitch.on = YES;
    _optimizationView = [[UIView alloc] initWithFrame:CGRectMake(bounds.size.width / 2 + 20 + 20, 30, optimizationLableWidth + optimizationSwitchWidth + 20, optimizationHeght)];
    [_optimizationView addSubview:_optimizationLabel];
    [_optimizationView addSubview:_optimizationSwitch];
    _optimizationView.autoresizingMask = UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleLeftMargin;
}

- (void)initalTableData
{
    NSArray *externsion = [NSArray arrayWithObjects:@"avi", @"rmvb", @"mkv", @"mp4", @"ts", @"mts", @"tp", @"mpg", @"rm", @"mov", @"flv", @"swf", @"wmv", nil];
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
    
//    [_pathNames addObject:@"http://huaqiangu.mp4"];
//    [_paths addObject:@"http://k.youku.com/player/getFlvPath/sid/8434446948428123f1446_00/st/mp4/fileid/031020010055791BF4C5EA2E10BECFC92EEE48-AF34-794F-A01F-4DC5A9E20B24?K=354598c9b601d0a024123d6c&hd=1&myp=0&ts=2363&ypp=0&ymovie=1&ep=eiaXHk2JUMcE7SfdgD8bZHrjcHMJXP4J9x%2BHgdJjALshTe%2B2mEynw%2B61O45AY%2FgZAFd1Zpj4qaWSH0QcYYE1qmwQ2EesT%2FqR%2FvGV5aknt5Vxb2w0cr%2FVsFSXRTD1&ctype=12&ev=1&token=9604&oip=3702892418"];
    
//    [_pathNames addObject:@"tsststs"];
//    [_paths addObject:@"http://221.229.164.134:5050/livestream/ebdea3f463e21b649f455ff3f7fd1fa811c51e55.m3u8?codec=ts&seq=4057919&len=6"];
    
//    [_pathNames addObject:@"iqiyi adv"];
//    [_paths addObject:@"http://data.video.qiyi.com/videos/other/20150601/fa/9a/70061d3e80f4dd257e857cc598d6ebcd.mp4?pv=0.2"];
//
//    [_pathNames addObject:@"http://192.168.16.128:81/8D5EBA72E161BDC63B5E61EA0264F791D3D47220.mp4"];
//    [_paths addObject:@"http://192.168.16.128:81/8D5EBA72E161BDC63B5E61EA0264F791D3D47220.mp4"];
    
    
//    [_pathNames addObject:@"http://192.168.160.203:44401/B421F1690EDF4115D39A0531770FF1D46B57F9FC.mp4"];
//    [_paths addObject:@"http://192.168.160.203:44401/B421F1690EDF4115D39A0531770FF1D46B57F9FC.mp4"];
    
//    [_pathNames addObject:@"http://192.168.160.203:8000/B421F1690EDF4115D39A0531770FF1D46B57F9FC.mp4"];
//    [_paths addObject:@"http://192.168.160.203:8000/B421F1690EDF4115D39A0531770FF1D46B57F9FC.mp4"];
//    
//    [_pathNames addObject:@"m3u8"];
//    [_paths addObject:@"http://cache.m.iqiyi.com/dc/amdt/b4d3c0f0x3deea9a6/ee36e508b4ebdedf60cc8f48e10d9627/310986900/088ef05abf17255701bcdfe9bbb51239.m3u8?qd_asc=fe6c9b0f0544c3c22889251638b80a96&qd_asrc=f45bc84a7ea643209b29a72b0c1e385f&qd_at=1435029204392&qypid=310986900_21"];

    
}

-(void)setTableProperty:(CGRect)bounds{
    _tableView = [[UITableView  alloc] initWithFrame:CGRectMake(0, 100, bounds.size.width, bounds.size.height - 100)];
    _tableView.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleWidth |
    UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
    [_tableView setDelegate:self];
    [_tableView setDataSource:self];
    _tableView.tableFooterView = [[UIView alloc] init];
    _tableView.backgroundColor = [UIColor clearColor];
    _tableView.separatorStyle = UITableViewCellSeparatorStyleNone;
}

-(void)createSubView:(CGRect)bounds{
    _backgroundView1 = [[BackGroundView alloc] initWithFrame:CGRectMake(0, 0, bounds.size.width, bounds.size.height)];
    _backgroundView1.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    ((CAGradientLayer *)_backgroundView1.layer).colors = [NSArray arrayWithObjects:
                                                          (id)[UIColor lightGrayColor].CGColor,
                                                          (id)[UIColor blackColor].CGColor,
                                                          (id)[UIColor lightGrayColor].CGColor,
                                                          nil];
//    _backgroundView1.layer.borderColor = [UIColor greenColor].CGColor;
    _backgroundView1.layer.masksToBounds = YES;
    _backgroundView1.backgroundColor = [UIColor blackColor];
//    _backgroundView1.layer.borderWidth = 4.0;
//    self.view.layer = _backgroundView1.layer;
}

-(void)createSubLayer:(CGRect)bounds{
    _gradient = [CAGradientLayer layer];
    CGFloat maxVal = bounds.size.width > bounds.size.height ? bounds.size.width : bounds.size.height;
    _gradient.frame = self.view.layer.frame;
    _gradient.masksToBounds = YES;
//    _gradient.colors = [NSArray arrayWithObjects:(id)[UIColor lightGrayColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor lightGrayColor].CGColor, nil];
    _gradient.backgroundColor = [UIColor blackColor].CGColor;
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
    cell.backgroundColor = [UIColor clearColor];
    cell.textLabel.font = [UIFont systemFontOfSize:15];
    cell.textLabel.textColor = [UIColor whiteColor];
//    cell.imageView.image = [UIImage imageNamed:@"play24x24.png"];
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    cell.selectionStyle = UITableViewCellSelectionStyleBlue;
    
    return cell;
}



#pragma mark - UITableViewDelegate
- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 45;
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
    if (_closeHardDecoderSwith.on) {
        sign = NO;
    }
    IOSUPlayerController *playerController = [[IOSUPlayerController alloc]
                                              initWithContentURL:path withEnableHevc:_enableHevc withHardDecoder:sign];
    [self presentViewController:playerController animated:YES completion:nil];
}


-(void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath{
    if(_editingStyle == UITableViewCellEditingStyleDelete){
        NSFileManager *fileMgr = [NSFileManager defaultManager];
        NSString *path = [_paths objectAtIndex:indexPath.row];
        BOOL ret = [fileMgr fileExistsAtPath:path];
        if (ret) {
            NSError *error;
            [fileMgr removeItemAtPath:path error:&error];
            if (error) {
                NSLog(@"DELETE FILE: %@ error", path);
            }
        }
        [_paths removeObjectAtIndex:indexPath.row];
        [_pathNames removeObjectAtIndex:indexPath.row];
        [_tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

-(UITableViewCellEditingStyle)tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath{
    return _editingStyle;
}

-(NSString *)tableView:(UITableView *)tableView titleForDeleteConfirmationButtonForRowAtIndexPath:(NSIndexPath *)indexPath{
    return @"delete";
}

#pragma mark - operation


-(void) optimizationChanged:(id)sender{
    _enableHevc = _optimizationSwitch.on;
}


-(void)switchChanged:(id)sender{
    //do nothing
}

-(void)dealloc
{
}

#pragma mark - private

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

-(void)initialization{
    _editingStyle = UITableViewCellEditingStyleDelete;
}
@end
