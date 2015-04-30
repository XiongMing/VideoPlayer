//
//  ulog_client.h
//  UKPlayer
//
//  Created by 韦兴华 on 11/7/13.
//  Copyright (c) 2013 Funshion Inc. All rights reserved.
//

#ifndef __UKPlayer__ulog_client__
#define __UKPlayer__ulog_client__

#define ios_log_print(tag, fmt, ...)  UlogClient::shareClient()->iosLogCat(tag, fmt, ##__VA_ARGS__)
//	printf(tag ":" fmt, ##__VA_ARGS__);putchar('\n')
#include <iostream>

class UPlayerLog {
public:
    virtual void print_log(const char* tag, const char* fmt, ...) = 0;
};

class UlogClient {
protected:
    UlogClient();
private:
    static UlogClient* _instance;
    UPlayerLog* _log_sys;
    bool		_enable;
    
public:
    static UlogClient* shareClient();
    void iosLogCat(const char* tag, const char* fmt, ...);
    void setDelegate(UPlayerLog *delegate);
    void enableLog(bool enable);
    ~UlogClient();
};

#endif /* defined(__UKPlayer__ulog_client__) */
