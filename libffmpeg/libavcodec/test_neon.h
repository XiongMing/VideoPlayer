#ifndef  TEST_NEON_H
#define  TEST_NEON_H

#include "hevc_log.h"
#include <arm_neon.h>
#include <time.h>

void test_neon(void);
int cmp_uint8_arr(uint8_t * exp, uint8_t * got, unsigned  int len, int interval, const char * point);
int cmp_int16_arr(int16_t * exp, int16_t * got, unsigned  int len, int interval, const char * point);
#endif


