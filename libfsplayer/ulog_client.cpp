//
//  ulog_client.cpp
//  UKPlayer
//
//  Created by 韦兴华 on 11/7/13.
//  Copyright (c) 2013 Funshion Inc. All rights reserved.
//
#include <stdarg.h>

#include "ulog_client.h"

UlogClient* UlogClient::_instance = 0;

UlogClient::~UlogClient() {
    _log_sys = NULL;
}

UlogClient::UlogClient () {
    _log_sys = NULL;
    _enable = true;
}

UlogClient* UlogClient::shareClient() {
    if (_instance == 0) {
        _instance = new UlogClient;
    }
    return _instance;
}

void UlogClient::iosLogCat(const char* tag, const char* fmt, ...) {
    if (_enable && (NULL != _log_sys)) {
        va_list args;
        va_start(args, fmt);
        _log_sys->print_log(tag, fmt, args);
        va_end(args);
    }
}

void UlogClient::setDelegate(UPlayerLog *delegate) {
    _log_sys = delegate;
}

void UlogClient::enableLog(bool enable) {
    _enable = enable;
}
