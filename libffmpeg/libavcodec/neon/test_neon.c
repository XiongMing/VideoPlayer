#include "libavcodec/test_neon.h"
#include "libavcodec/hevc_log.h"
#include <time.h>
#include <stdlib.h>
#include "libavcodec/hevcdsp.h"
#include "libavcodec/hevc_mc_opt.h"

static const int8_t transform[32][32] = {
    { 64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,
      64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64 },
    { 90,  90,  88,  85,  82,  78,  73,  67,  61,  54,  46,  38,  31,  22,  13,   4,
      -4, -13, -22, -31, -38, -46, -54, -61, -67, -73, -78, -82, -85, -88, -90, -90 },
    { 90,  87,  80,  70,  57,  43,  25,   9,  -9, -25, -43, -57, -70, -80, -87, -90,
     -90, -87, -80, -70, -57, -43, -25,  -9,   9,  25,  43,  57,  70,  80,  87,  90 },
    { 90,  82,  67,  46,  22,  -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13,
      13,  38,  61,  78,  88,  90,  85,  73,  54,  31,   4, -22, -46, -67, -82, -90 },
    { 89,  75,  50,  18, -18, -50, -75, -89, -89, -75, -50, -18,  18,  50,  75,  89,
      89,  75,  50,  18, -18, -50, -75, -89, -89, -75, -50, -18,  18,  50,  75,  89 },
    { 88,  67,  31, -13, -54, -82, -90, -78, -46, -4,   38,  73,  90,  85,  61,  22,
     -22, -61, -85, -90, -73, -38,   4,  46,  78,  90,  82,  54,  13, -31, -67, -88 },
    { 87,  57,   9, -43, -80, -90, -70, -25,  25,  70,  90,  80,  43,  -9, -57, -87,
     -87, -57,  -9,  43,  80,  90,  70,  25, -25, -70, -90, -80, -43,   9,  57,  87 },
    { 85,  46, -13, -67, -90, -73, -22,  38,  82,  88,  54,  -4, -61, -90, -78, -31,
      31,  78,  90,  61,   4, -54, -88, -82, -38,  22,  73,  90,  67,  13, -46, -85 },
    { 83,  36, -36, -83, -83, -36,  36,  83,  83,  36, -36, -83, -83, -36,  36,  83,
      83,  36, -36, -83, -83, -36,  36,  83,  83,  36, -36, -83, -83, -36,  36,  83 },
    { 82,  22, -54, -90, -61,  13,  78,  85,  31, -46, -90, -67,   4,  73,  88,  38,
     -38, -88, -73,  -4,  67,  90,  46, -31, -85, -78, -13,  61,  90,  54, -22, -82 },
    { 80,   9, -70, -87, -25,  57,  90,  43, -43, -90, -57,  25,  87,  70,  -9, -80,
     -80,  -9,  70,  87,  25, -57, -90, -43,  43,  90,  57, -25, -87, -70,   9,  80 },
    { 78,  -4, -82, -73,  13,  85,  67, -22, -88, -61,  31,  90,  54, -38, -90, -46,
      46,  90,  38, -54, -90, -31,  61,  88,  22, -67, -85, -13,  73,  82,   4, -78 },
    { 75, -18, -89, -50,  50,  89,  18, -75, -75,  18,  89,  50, -50, -89, -18,  75,
      75, -18, -89, -50,  50,  89,  18, -75, -75,  18,  89,  50, -50, -89, -18,  75 },
    { 73, -31, -90, -22,  78,  67, -38, -90, -13,  82,  61, -46, -88,  -4,  85,  54,
     -54, -85,   4,  88,  46, -61, -82,  13,  90,  38, -67, -78,  22,  90,  31, -73 },
    { 70, -43, -87,   9,  90,  25, -80, -57,  57,  80, -25, -90,  -9,  87,  43, -70,
     -70,  43,  87,  -9, -90, -25,  80,  57, -57, -80,  25,  90,   9, -87, -43,  70 },
    { 67, -54, -78,  38,  85, -22, -90,   4,  90,  13, -88, -31,  82,  46, -73, -61,
      61,  73, -46, -82,  31,  88, -13, -90,  -4,  90,  22, -85, -38,  78,  54, -67 },
    { 64, -64, -64,  64,  64, -64, -64,  64,  64, -64, -64,  64,  64, -64, -64,  64,
      64, -64, -64,  64,  64, -64, -64,  64,  64, -64, -64,  64,  64, -64, -64,  64 },
    { 61, -73, -46,  82,  31, -88, -13,  90,  -4, -90,  22,  85, -38, -78,  54,  67,
     -67, -54,  78,  38, -85, -22,  90,   4, -90,  13,  88, -31, -82,  46,  73, -61 },
    { 57, -80, -25,  90,  -9, -87,  43,  70, -70, -43,  87,   9, -90,  25,  80, -57,
     -57,  80,  25, -90,   9,  87, -43, -70,  70,  43, -87,  -9,  90, -25, -80,  57 },
    { 54, -85,  -4,  88, -46, -61,  82,  13, -90,  38,  67, -78, -22,  90, -31, -73,
      73,  31, -90,  22,  78, -67, -38,  90, -13, -82,  61,  46, -88,   4,  85, -54 },
    { 50, -89,  18,  75, -75, -18,  89, -50, -50,  89, -18, -75,  75,  18, -89,  50,
      50, -89,  18,  75, -75, -18,  89, -50, -50,  89, -18, -75,  75,  18, -89,  50 },
    { 46, -90,  38,  54, -90,  31,  61, -88,  22,  67, -85,  13,  73, -82,   4,  78,
     -78,  -4,  82, -73, -13,  85, -67, -22,  88, -61, -31,  90, -54, -38,  90, -46 },
    { 43, -90,  57,  25, -87,  70,   9, -80,  80,  -9, -70,  87, -25, -57,  90, -43,
     -43,  90, -57, -25,  87, -70,  -9,  80, -80,   9,  70, -87,  25,  57, -90,  43 },
    { 38, -88,  73,  -4, -67,  90, -46, -31,  85, -78,  13,  61, -90,  54,  22, -82,
      82, -22, -54,  90, -61, -13,  78, -85,  31,  46, -90,  67,   4, -73,  88, -38 },
    { 36, -83,  83, -36, -36,  83, -83,  36,  36, -83,  83, -36, -36,  83, -83,  36,
      36, -83,  83, -36, -36,  83, -83,  36,  36, -83,  83, -36, -36,  83, -83,  36 },
    { 31, -78,  90, -61,   4,  54, -88,  82, -38, -22,  73, -90,  67, -13, -46,  85,
     -85,  46,  13, -67,  90, -73,  22,  38, -82,  88, -54,  -4,  61, -90,  78, -31 },
    { 25, -70,  90, -80,  43,   9, -57,  87, -87,  57,  -9, -43,  80, -90,  70, -25,
     -25,  70, -90,  80, -43,  -9,  57, -87,  87, -57,   9,  43, -80,  90, -70,  25 },
    { 22, -61,  85, -90,  73, -38,  -4,  46, -78,  90, -82,  54, -13, -31,  67, -88,
      88, -67,  31,  13, -54,  82, -90,  78, -46,   4,  38, -73,  90, -85,  61, -22 },
    { 18, -50,  75, -89,  89, -75,  50, -18, -18,  50, -75,  89, -89,  75, -50,  18,
      18, -50,  75, -89,  89, -75,  50, -18, -18,  50, -75,  89, -89,  75, -50,  18 },
    { 13, -38,  61, -78,  88, -90,  85, -73,  54, -31,   4,  22, -46,  67, -82,  90,
     -90,  82, -67,  46, -22,  -4,  31, -54,  73, -85,  90, -88,  78, -61,  38, -13 },
    {  9, -25,  43, -57,  70, -80,  87, -90,  90, -87,  80, -70,  57, -43,  25, -9,
      -9,  25, -43,  57, -70,  80, -87,  90, -90,  87, -80,  70, -57,  43, -25,   9 },
    {  4, -13,  22, -31,  38, -46,  54, -61,  67, -73,  78, -82,  85, -88,  90, -90,
      90, -90,  88, -85,  82, -78,  73, -67,  61, -54,  46, -38,  31, -22,  13,  -4 },
};
/*
DECLARE_ALIGNED(16, const int8_t, ff_hevc_epel_filters[7][4]) = {
    { -2, 58, 10, -2},
    { -4, 54, 16, -2},
    { -6, 46, 28, -4},
    { -4, 36, 36, -4},
    { -4, 28, 46, -6},
    { -2, 16, 54, -4},
    { -2, 10, 58, -2},
};

DECLARE_ALIGNED(16, const int8_t, ff_hevc_qpel_filters[3][16]) = {
    { -1,  4,-10, 58, 17, -5,  1,  0, -1,  4,-10, 58, 17, -5,  1,  0},
    { -1,  4,-11, 40, 40,-11,  4, -1, -1,  4,-11, 40, 40,-11,  4, -1},
    {  0,  1, -5, 17, 58,-10,  4, -1,  0,  1, -5, 17, 58,-10,  4, -1}
};
*/


#define BIT_DEPTH 8
#include "libavcodec/hevcdsp_template.c"
#undef BIT_DEPTH

//#define MC_CHK_CMP
#define TEST_CNT_OPT 1000
#define _SSTRIDE0 160
#define SHEIGHT 160

#define TEST_CNT_OPT 2000
#define _SSTRIDE0 160
#define SHEIGHT 160

int cmp_uint8_arr(uint8_t * exp, uint8_t * got, unsigned  int len, int interval, const char * point)
{
     int cmp_idx;
	 for ( cmp_idx = 0 ; cmp_idx < len ; cmp_idx++ )
	 {
          if ( exp[cmp_idx] != got[cmp_idx] )
          {
              LOGI_ERR("%s exp %d got %d idx %d", point, exp[cmp_idx] , got[cmp_idx] ,cmp_idx);
			  return -1;
          }
		  if (interval != 0)
		  {
		    if (cmp_idx % interval == 0)
		    {
		      LOGI_OK("%s exp %d got %d idx %d", point, exp[cmp_idx] , got[cmp_idx] ,cmp_idx)
		    }
		  }
	 }
	  return 1;
}

int cmp_int16_arr(int16_t * exp, int16_t * got, unsigned  int len, int interval, const char * point)
{
     int cmp_idx;
	 for ( cmp_idx = 0 ; cmp_idx < len ; cmp_idx++ )
	 {
          if ( exp[cmp_idx] != got[cmp_idx] )
          {
              LOGI_ERR("%s exp %d got %d idx %d", point, exp[cmp_idx] , got[cmp_idx] ,cmp_idx)
			  return -1;
          }
		  if (interval != 0)
		  {
		    if (cmp_idx % interval == 0)
		    {
		      LOGI_OK("%s exp %d got %d idx %d", point, exp[cmp_idx] , got[cmp_idx] ,cmp_idx)
		    }
		  }
	 }
	 return 1;
}

static int cmp_int16_arr_stride(int16_t * exp, int16_t * got, int height, int width, int stride, int interval, const char * point)
{
     int cmp_idx,x ,y;
	 for ( y = 0 ; y < height ; y++ )
	 {
	     for ( x = 0 ; x < width ; x++ )
		  {
           if ( exp[y*stride + x] != got[y*stride + x] )
           {
          
              LOGI_ERR("%s exp %d got %d idx %d", point, exp[y*stride + x] , got[y*stride + x] ,cmp_idx)
			  return -1;
           }
		   
		   if (interval != 0)
		   {
		    if ( cmp_idx % interval == 0 )
		    {
			  LOGI_OK("%s exp %d got %d idx %d", point, exp[y*stride + x] , got[y*stride + x] ,cmp_idx) 	
		    }
		   }
		   cmp_idx++;
		  }

	 }
	 return 1;
}


static void chk_mc_opt_s16(void)
{
	int16_t * _dst_exp, *_dst_got;
	ptrdiff_t _dststride;
	uint8_t * _src;
	ptrdiff_t _srcstride;
	int       height;
	intptr_t  mx;
	intptr_t  my;
	int       width;
	int dim_arr[10] = {2, 4, 6, 8, 12, 16, 24, 32, 48 ,64};

	int test_num, pidx;
	//160 is enough for any prediction unit
	_srcstride = _SSTRIDE0;
	_dststride = SHEIGHT;
	_src = (uint8_t *)malloc(sizeof(uint8_t)*_dststride*_srcstride);
	if ( !_src )
	{
	   LOGI_ERR("_src allocated failed\n");
	   return ;
	}

    for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
    {
 	   srand(clock());
	   height = dim_arr[(abs(rand())%10)];
       width =  dim_arr[(abs(rand())%10)];
       //width = 16;
	   _dststride = MAX_PB_SIZE;
	   _dst_exp = (int16_t *)malloc(sizeof(int16_t)*height*_dststride);
	   _dst_got = (int16_t *)malloc(sizeof(int16_t)*height*_dststride);
	   if ( !_dst_exp || !_dst_got )
	   {
	     LOGI_ERR("_dst allocated failed\n");
	     return ;
	   }
	   my = (abs(rand())%3) + 1;
       mx = (abs(rand())%3) + 1;
	   //my = 2;
	   //my = 3;
	   for (pidx = 0; pidx < _SSTRIDE0*SHEIGHT ; pidx++)
	   {
	        _src[pidx] = abs(rand())%255;
	        //_src[pidx] = 255;
	   }
	   LOGI1("%d width %d height %d mvy %d mvx %d s16", test_num, width, height,my, mx);

/*
	   put_hevc_qpel_bi_h_8(_dst_exp, _src + _SSTRIDE0*4, _srcstride,
                                      height, mx, my, width);
*/
	   put_hevc_qpel_v_opt_8(_dst_got, _src + _SSTRIDE0*4, _srcstride,
                                      height, mx, my, width);

 	   if ( cmp_int16_arr_stride(_dst_exp, _dst_got,  height, width, _dststride, 0, "mc_opt_test") < 0 )
	   {
	       LOGI_point(mc_opt_test, "width %d height %d mvy %d mvx %d", width, height,my, mx);
	   }

	   //LOGI1("%d %d %d %d %d %d %d %d \n", _dst_exp[0],_dst_exp[1],_dst_exp[2],_dst_exp[3],_dst_exp[4],_dst_exp[5],_dst_exp[6],_dst_exp[7]);
	   free(_dst_exp);
	   free(_dst_got);
	   //LOGI1("buffer free\n");

    }
	free(_src);
}

static void chk_mc_opt_bi(void)
{
	uint8_t * _dst_exp, *_dst_got;
	ptrdiff_t _dststride;
	uint8_t * _src;
	ptrdiff_t _srcstride;
	int       height;
	intptr_t  mx;
	intptr_t  my;
	int       width;
	int dim_arr[10] = {2, 4, 6, 8, 12, 16, 24, 32, 48 ,64};
	int16_t tmp[(MAX_PB_SIZE + QPEL_EXTRA) * (MAX_PB_SIZE + QPEL_EXTRA)];

	int test_num, pidx;
	//160 is enough for any prediction unit
	_srcstride = _SSTRIDE0;
	_src = (uint8_t *)malloc(sizeof(uint8_t)*_SSTRIDE0*_SSTRIDE0);
	if ( !_src )
	{
	   LOGI_ERR("_src allocated failed\n");
	   return ;
	}
	//init_put_hevc_pel_bi_pixels_opt_8();
    for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
    {
 	   srand(clock());
	   height = dim_arr[(abs(rand())%10)];
       width = dim_arr[(abs(rand())%10)];
	   switch(test_num/100)
	   {
	      case 0:
		    width = 2;
		  break;
	      case 1:
		    width = 4;
		  break;
	      case 2:
		    width = 6;
		  break;
	      case 3:
		    width = 8;
		  break;
	      case 4:
		    width = 12;
		  break;
	      case 5:
		    width = 16;
		  break;
	      case 6:
		    width = 24;
		  break;
	      case 7:
		    width = 32;
		  break;
	      case 8:
		    width = 48;
		  break;
	      case 9:
		    width = 64;
		  break;
	   }
	   //height = 1;
       //width = 12;
	   _dststride = width;
	   _dst_exp = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	   _dst_got = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	   if ( !_dst_exp || !_dst_got )
	   {
	     LOGI_ERR("_dst allocated failed\n");
	     return ;
	   }
	   my = (abs(rand())%3) + 1;
       mx = (abs(rand())%3) + 1;
	   //my = 1;
	   //mx = 3;
	   for (pidx = 0; pidx < _SSTRIDE0*_SSTRIDE0 ; pidx++)
	   {
	        _src[pidx] = abs(rand())%255;
	        //_src[pidx] = 255;
	   }
	   for (pidx = 0; pidx < (MAX_PB_SIZE + QPEL_EXTRA) * (MAX_PB_SIZE + QPEL_EXTRA) ; pidx++)
	   {
	        tmp[pidx] = abs(rand())%65535 - 32768;
	        //tmp[pidx] = 0;
	        //tmp[pidx] = 32767;
	        //tmp[pidx] = -32768;
		    //tmp[pidx] = 0;
	   }
	   LOGI1("%d width %d height %d mvy %d mvx %d", test_num, width, height,my, mx);
/*
       int line_idx;
	   for (line_idx = 0 ; line_idx < 16 ; line_idx++)
	   {
	        LOGI_DBG("line idx %d %d %d %d %d %d %d %d %d", line_idx,
				   _src[160 + line_idx*160 + 0], _src[160 + line_idx*160 + 1],
				   _src[160 + line_idx*160 + 2], _src[160 + line_idx*160 + 3],
				   _src[160 + line_idx*160 + 4], _src[160 + line_idx*160 + 5],
				   _src[160 + line_idx*160 + 6], _src[160 + line_idx*160 + 7]
				   );
	   }
*/
	   put_hevc_qpel_bi_v_8(_dst_exp, _dststride, _src + _SSTRIDE0*4, _srcstride,
                                      tmp, height, mx, my, width);
	   //LOGI_DBG("exp %d %d %d %d %d %d %d %d %d %d %d %d %d ",width, _dst_exp[0], _dst_exp[1], _dst_exp[2], _dst_exp[3],_dst_exp[4], _dst_exp[5], _dst_exp[6], _dst_exp[7],_dst_exp[8], _dst_exp[9], _dst_exp[10], _dst_exp[11]);

	   put_hevc_qpel_bi_v_opt_8(_dst_got, _dststride, _src + _SSTRIDE0*4, _srcstride,
                                      tmp, height, mx, my , width);
	   //LOGI_DBG("got %d %d %d %d %d %d %d %d %d %d %d %d %d ",width, _dst_got[0], _dst_got[1], _dst_got[2], _dst_got[3],_dst_got[4], _dst_got[5], _dst_got[6], _dst_got[7],_dst_got[8], _dst_got[9], _dst_got[10], _dst_got[11]);

	   if ( cmp_uint8_arr(_dst_exp, _dst_got, width*height, 0, "mc_opt_test") < 0 )
	   {
	       LOGI_point(mc_opt_test, "width %d height %d mvy %d mvx %d", width, height,my, mx);
	   }
	   free(_dst_exp);
	   free(_dst_got);

    }
	free(_src);

}

static void chk_mc_opt(void)
{
	uint8_t * _dst_exp, *_dst_got;
	ptrdiff_t _dststride;
	uint8_t * _src;
	ptrdiff_t _srcstride;
	int       height;
	intptr_t  mx;
	intptr_t  my;
	int       width;
	int dim_arr[10] = {2, 4, 6, 8, 12, 16, 24, 32, 48 ,64};
	int16_t tmp[(MAX_PB_SIZE + QPEL_EXTRA) * (MAX_PB_SIZE + QPEL_EXTRA)];

	int test_num, pidx;
	//160 is enough for any prediction unit
	_srcstride = _SSTRIDE0;
	_dststride = SHEIGHT;
	_src = (uint8_t *)malloc(sizeof(uint8_t)*_dststride*_srcstride);
	if ( !_src )
	{
	   LOGI_ERR("_src allocated failed\n");
	   return ;
	}

    for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
    {
 	   srand(clock());
	   height = dim_arr[(abs(rand())%10)];
       width = dim_arr[(abs(rand())%10)];
	   //height = 8 ;
	   switch(test_num/100)
	   {
	      case 0:
		    width = 2;
		  break;
	      case 1:
		    width = 4;
		  break;
	      case 2:
		    width = 6;
		  break;
	      case 3:
		    width = 8;
		  break;
	      case 4:
		    width = 12;
		  break;
	      case 5:
		    width = 16;
		  break;
	      case 6:
		    width = 24;
		  break;
	      case 7:
		    width = 32;
		  break;
	      case 8:
		    width = 48;
		  break;
	      case 9:
		    width = 64;
		  break;
	   }
	   _dststride = width;
	   _dst_exp = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	   _dst_got = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	   if ( !_dst_exp || !_dst_got )
	   {
	     LOGI_ERR("_dst allocated failed\n");
	     return ;
	   }
	   my = (abs(rand())%3) + 1;
       mx = (abs(rand())%3) + 1;
	   //my = 3;
	   //mx = 1;
	   for (pidx = 0; pidx < _SSTRIDE0*SHEIGHT ; pidx++)
	   {
	        _src[pidx] = abs(rand())%255;
	        //_src[pidx] = 255;
	   }
	   LOGI1("%d width %d height %d mvy %d mvx %d", test_num, width, height,my, mx);
/*
       int line_idx;
	   for (line_idx = 0 ; line_idx < 16 ; line_idx++)
	   {
	        LOGI_DBG("line idx %d %d %d %d %d %d %d %d %d", line_idx,
				   _src[160 + line_idx*160 + 0], _src[160 + line_idx*160 + 1],
				   _src[160 + line_idx*160 + 2], _src[160 + line_idx*160 + 3],
				   _src[160 + line_idx*160 + 4], _src[160 + line_idx*160 + 5],
				   _src[160 + line_idx*160 + 6], _src[160 + line_idx*160 + 7]
				   );
	   }
*/
/*
	   put_hevc_qpel_bi_h_8(_dst_exp, _dststride, _src + _SSTRIDE0*4, _srcstride,
                                      height, mx, my, width);
	   put_hevc_qpel_bi_h_opt8_mv1(_dst_got, _dststride, _src + _SSTRIDE0*4, _srcstride,
                                      height, mx, my, width);
*/
	   if ( cmp_uint8_arr(_dst_exp, _dst_got, width*height, 0, "mc_opt_test") < 0 )
	   {
	       LOGI_point(mc_opt_test, "width %d height %d mvy %d mvx %d", width, height,my, mx);
	   }
	   free(_dst_exp);
	   free(_dst_got);

    }
	free(_src);

}

static void chk_mc_opt_s16_epel(void)
{
	int16_t * _dst_exp, *_dst_got;
	ptrdiff_t _dststride;
	uint8_t * _src;
	ptrdiff_t _srcstride;
	int       height;
	intptr_t  mx;
	intptr_t  my;
	int       width;
	int dim_arr[10] = {2, 4, 6, 8, 12, 16, 24, 32, 48 ,64};

	int test_num, pidx;
	//160 is enough for any prediction unit
	_srcstride = _SSTRIDE0;
	_dststride = SHEIGHT;
	_src = (uint8_t *)malloc(sizeof(uint8_t)*_dststride*_srcstride);
	if ( !_src )
	{
	   LOGI_ERR("_src allocated failed\n");
	   return ;
	}

    for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
    {
 	   srand(clock());
	   height = dim_arr[(abs(rand())%10)];
       width =  dim_arr[(abs(rand())%10)];
       //width = 16;
	   _dststride = MAX_PB_SIZE;
	   _dst_exp = (int16_t *)malloc(sizeof(int16_t)*height*_dststride);
	   _dst_got = (int16_t *)malloc(sizeof(int16_t)*height*_dststride);
	   if ( !_dst_exp || !_dst_got )
	   {
	     LOGI_ERR("_dst allocated failed\n");
	     return ;
	   }
	   my = (abs(rand())%7) + 1;
       mx = (abs(rand())%7) + 1;
	   //my = 2;
	   //my = 3;
	   for (pidx = 0; pidx < _SSTRIDE0*SHEIGHT ; pidx++)
	   {
	        _src[pidx] = abs(rand())%255;
	        //_src[pidx] = 255;
	   }
	   LOGI1("%d width %d height %d mvy %d mvx %d s16", test_num, width, height,my, mx);


	   put_hevc_epel_hv_8(_dst_exp, _src + _SSTRIDE0*4, _srcstride,
                                      height, mx, my, width);

	   put_hevc_epel_hv_opt_8(_dst_got, _src + _SSTRIDE0*4, _srcstride,
                                      height, mx, my, width);

 	   if ( cmp_int16_arr_stride(_dst_exp, _dst_got,  height, width, _dststride, 0, "mc_opt_test") < 0 )
	   {
	       LOGI_point(mc_opt_test, "width %d height %d mvy %d mvx %d", width, height,my, mx);
	   }

	   //LOGI1("%d %d %d %d %d %d %d %d \n", _dst_exp[0],_dst_exp[1],_dst_exp[2],_dst_exp[3],_dst_exp[4],_dst_exp[5],_dst_exp[6],_dst_exp[7]);
	   free(_dst_exp);
	   free(_dst_got);
	   //LOGI1("buffer free\n");

    }
	free(_src);
}

static void chk_mc_opt_bi_epel(void)
{
	uint8_t * _dst_exp, *_dst_got;
	ptrdiff_t _dststride;
	uint8_t * _src;
	ptrdiff_t _srcstride;
	int       height;
	intptr_t  mx;
	intptr_t  my;
	int       width;
	int dim_arr[10] = {2, 4, 6, 8, 12, 16, 24, 32, 48 ,64};
	int16_t tmp[(MAX_PB_SIZE + QPEL_EXTRA) * (MAX_PB_SIZE + QPEL_EXTRA)];

	int test_num, pidx;
	//160 is enough for any prediction unit
	_srcstride = _SSTRIDE0;
	_src = (uint8_t *)malloc(sizeof(uint8_t)*_SSTRIDE0*_SSTRIDE0);
	if ( !_src )
	{
	   LOGI_ERR("_src allocated failed\n");
	   return ;
	}
	//init_put_hevc_pel_bi_pixels_opt_8();
    for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
    {
 	   srand(clock());
	   height = dim_arr[(abs(rand())%10)];
       width = dim_arr[(abs(rand())%10)];
	   switch(test_num/100)
	   {
	      case 0:
		    width = 2;
		  break;
	      case 1:
		    width = 4;
		  break;
	      case 2:
		    width = 6;
		  break;
	      case 3:
		    width = 8;
		  break;
	      case 4:
		    width = 12;
		  break;
	      case 5:
		    width = 16;
		  break;
	      case 6:
		    width = 24;
		  break;
	      case 7:
		    width = 32;
		  break;
	      case 8:
		    width = 48;
		  break;
	      case 9:
		    width = 64;
		  break;
	   }
	   //height = 1;
       //width = 12;
	   _dststride = width;
	   _dst_exp = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	   _dst_got = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	   if ( !_dst_exp || !_dst_got )
	   {
	     LOGI_ERR("_dst allocated failed\n");
	     return ;
	   }
	   my = (abs(rand())%7) + 1;
       mx = (abs(rand())%7) + 1;
	   //my = 1;
	   //mx = 3;
	   for (pidx = 0; pidx < _SSTRIDE0*_SSTRIDE0 ; pidx++)
	   {
	        _src[pidx] = abs(rand())%255;
	        //_src[pidx] = 255;
	   }
	   for (pidx = 0; pidx < (MAX_PB_SIZE + QPEL_EXTRA) * (MAX_PB_SIZE + QPEL_EXTRA) ; pidx++)
	   {
	        tmp[pidx] = abs(rand())%65535 - 32768;
	        //tmp[pidx] = 0;
	        //tmp[pidx] = 32767;
	        //tmp[pidx] = -32768;
		    //tmp[pidx] = 0;
	   }
	   LOGI1("%d width %d height %d mvy %d mvx %d", test_num, width, height,my, mx);
/*
       int line_idx;
	   for (line_idx = 0 ; line_idx < 16 ; line_idx++)
	   {
	        LOGI_DBG("line idx %d %d %d %d %d %d %d %d %d", line_idx,
				   _src[160 + line_idx*160 + 0], _src[160 + line_idx*160 + 1],
				   _src[160 + line_idx*160 + 2], _src[160 + line_idx*160 + 3],
				   _src[160 + line_idx*160 + 4], _src[160 + line_idx*160 + 5],
				   _src[160 + line_idx*160 + 6], _src[160 + line_idx*160 + 7]
				   );
	   }
*/
	   put_hevc_epel_bi_hv_8(_dst_exp, _dststride, _src + _SSTRIDE0*4, _srcstride,
                                      tmp, height, mx, my, width);
	   //LOGI_DBG("exp %d %d %d %d %d %d %d %d %d %d %d %d %d ",width, _dst_exp[0], _dst_exp[1], _dst_exp[2], _dst_exp[3],_dst_exp[4], _dst_exp[5], _dst_exp[6], _dst_exp[7],_dst_exp[8], _dst_exp[9], _dst_exp[10], _dst_exp[11]);

	   put_hevc_epel_bi_hv_opt_8(_dst_got, _dststride, _src + _SSTRIDE0*4, _srcstride,
                                      tmp, height, mx, my , width);
	   //LOGI_DBG("got %d %d %d %d %d %d %d %d %d %d %d %d %d ",width, _dst_got[0], _dst_got[1], _dst_got[2], _dst_got[3],_dst_got[4], _dst_got[5], _dst_got[6], _dst_got[7],_dst_got[8], _dst_got[9], _dst_got[10], _dst_got[11]);

	   if ( cmp_uint8_arr(_dst_exp, _dst_got, width*height, 0, "mc_opt_test") < 0 )
	   {
	       LOGI_point(mc_opt_test, "width %d height %d mvy %d mvx %d", width, height,my, mx);
	   }
	   free(_dst_exp);
	   free(_dst_got);

    }
	free(_src);

}

static void chk_mc_opt_epel(void)
{
	uint8_t * _dst_exp, *_dst_got;
	ptrdiff_t _dststride;
	uint8_t * _src;
	ptrdiff_t _srcstride;
	int       height;
	intptr_t  mx;
	intptr_t  my;
	int       width;
	int dim_arr[10] = {2, 4, 6, 8, 12, 16, 24, 32, 48 ,64};
	int16_t tmp[(MAX_PB_SIZE + QPEL_EXTRA) * (MAX_PB_SIZE + QPEL_EXTRA)];

	int test_num, pidx;
	//160 is enough for any prediction unit
	_srcstride = _SSTRIDE0;
	_dststride = SHEIGHT;
	_src = (uint8_t *)malloc(sizeof(uint8_t)*_dststride*_srcstride);
	if ( !_src )
	{
	   LOGI_ERR("_src allocated failed\n");
	   return ;
	}

    for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
    {
 	   srand(clock());
	   height = dim_arr[(abs(rand())%10)];
       width = dim_arr[(abs(rand())%10)];
	   //height = 8 ;
	   switch(test_num/100)
	   {
	      case 0:
		    width = 2;
		  break;
	      case 1:
		    width = 4;
		  break;
	      case 2:
		    width = 6;
		  break;
	      case 3:
		    width = 8;
		  break;
	      case 4:
		    width = 12;
		  break;
	      case 5:
		    width = 16;
		  break;
	      case 6:
		    width = 24;
		  break;
	      case 7:
		    width = 32;
		  break;
	      case 8:
		    width = 48;
		  break;
	      case 9:
		    width = 64;
		  break;
	   }
	   _dststride = width;
	   _dst_exp = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	   _dst_got = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	   if ( !_dst_exp || !_dst_got )
	   {
	     LOGI_ERR("_dst allocated failed\n");
	     return ;
	   }
	   my = (abs(rand())%7) + 1;
       mx = (abs(rand())%7) + 1;
	   //my = 3;
	   //mx = 1;
	   for (pidx = 0; pidx < _SSTRIDE0*SHEIGHT ; pidx++)
	   {
	        _src[pidx] = abs(rand())%255;
	        //_src[pidx] = 255;
	   }
	   LOGI1("%d width %d height %d mvy %d mvx %d", test_num, width, height,my, mx);
/*
       int line_idx;
	   for (line_idx = 0 ; line_idx < 16 ; line_idx++)
	   {
	        LOGI_DBG("line idx %d %d %d %d %d %d %d %d %d", line_idx,
				   _src[160 + line_idx*160 + 0], _src[160 + line_idx*160 + 1],
				   _src[160 + line_idx*160 + 2], _src[160 + line_idx*160 + 3],
				   _src[160 + line_idx*160 + 4], _src[160 + line_idx*160 + 5],
				   _src[160 + line_idx*160 + 6], _src[160 + line_idx*160 + 7]
				   );
	   }
*/

	   put_hevc_epel_uni_v_8(_dst_exp, _dststride, _src + _SSTRIDE0*4, _srcstride,
                                      height, mx, my, width);
	   put_hevc_epel_uni_v_opt_8(_dst_got, _dststride, _src + _SSTRIDE0*4, _srcstride,
                                      height, mx, my, width);

	   if ( cmp_uint8_arr(_dst_exp, _dst_got, width*height, 0, "mc_opt_test") < 0 )
	   {
	       LOGI_point(mc_opt_test, "width %d height %d mvy %d mvx %d", width, height,my, mx);
	   }
	   free(_dst_exp);
	   free(_dst_got);

    }
	free(_src);

}

#define TEST_CNT_OPT 1000
#define _SSTRIDE1 1600
#define _DSTRIDE1 160

static void cmp_mc_opt(void)
{
	//CLKS_DEF(tmc_orig);
	//CLKS_DEF(tmc_opt);	

	uint8_t * _dst_exp, *_dst_got;
	ptrdiff_t _dststride;
	uint8_t * _src;
	ptrdiff_t _srcstride;
	int       height;
	intptr_t  mx;
	intptr_t  my;
	int       width;
	int dim_arr[10] = {2, 4, 6, 8, 12, 16, 24, 32, 48 ,64};
	int test_num, pidx;
	//80 is enough for any prediction unit
	_srcstride = _SSTRIDE1;
	_dststride = _DSTRIDE1;
	_src = (uint8_t *)malloc(sizeof(uint8_t)*_dststride*_srcstride);
	if ( !_src )
	{
	   LOGI_ERR("_src allocated failed\n");
	   return ;
	}

	height = 16;
    width = 16; 	   
	_dststride = width;
	_dst_exp = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	_dst_got = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	if ( !_dst_exp || !_dst_got )
	{
	   LOGI_ERR("_dst allocated failed\n");
	   return ;
	}	   
	mx = 2;
    my = 0;
	for (pidx = 0; pidx < _SSTRIDE1*_DSTRIDE1 ; pidx++)
	{
	     _src[pidx] = abs(rand())%255;
	}
	
#if 0	
	CLKS_T0(tmc_orig);
	for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
    {
		put_hevc_qpel_uni_h_8(_dst_exp, _dststride,_src + _SSTRIDE1*4, _srcstride,
									   height, mx, my, width);

    }
	CLKS_T1(tmc_orig);
		

	CLKS_T0(tmc_opt);	
    for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
    {
		put_hevc_qpel_uni_h_opt_8(_dst_got, _dststride,_src + _SSTRIDE1*4, _srcstride,
									   height, mx, my, width);   
    }
	CLKS_T1(tmc_opt);
#endif

#if 0
	my = 2;
    mx = 0;	
	CLKS_T0(tmc_orig);
	for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
    {
		put_hevc_qpel_uni_v_8(_dst_exp, _dststride,_src + _SSTRIDE1*4, _srcstride,
									   height, mx, my, width);

    }
	CLKS_T1(tmc_orig);

	CLKS_T0(tmc_opt);	
    for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
    {
		put_hevc_qpel_uni_v_opt_8(_dst_got, _dststride,_src + _SSTRIDE1*4, _srcstride,
									   height, mx, my, width);   
    }
	CLKS_T1(tmc_opt);	
#endif

#if 0
	my = 2;
	mx = 2; 
	CLKS_T0(tmc_orig);
	for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
	{
		put_hevc_qpel_uni_hv_8(_dst_exp, _dststride,_src + _SSTRIDE1*4, _srcstride,
										   height, mx, my, width);
	
	}
	CLKS_T1(tmc_orig);
	
	CLKS_T0(tmc_opt);	
	for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
	{
		put_hevc_qpel_uni_hv_opt_8(_dst_got, _dststride,_src + _SSTRIDE1*4, _srcstride,
										   height, mx, my, width);	 
	}
	CLKS_T1(tmc_opt);	
#endif

	
	free(_dst_exp);
	free(_dst_got);
	free(_src);

#if 0
   #define LEN 32
   uint8_t src_arr[LEN]; 
   uint8_t dst_arr[LEN];
   int     test_num, pidx;
   uint8x8_t v0, v1,v2,v3;
   

   CLKS_T0(tmc_orig);
   for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
   {
	   memcpy(dst_arr, src_arr,32);	  
   }
   CLKS_T1(tmc_orig);
   
   
   CLKS_T0(tmc_opt);   
   for (test_num = 0; test_num < TEST_CNT_OPT; test_num++)
   {
	   v0 = vld1_u8(src_arr);
	   v1 = vld1_u8(src_arr + 8);
	   v2 = vld1_u8(src_arr + 16);
	   v3 = vld1_u8(src_arr + 24);	   
	   
	   vst1_u8(dst_arr ,v0);	 
	   vst1_u8(dst_arr + 8,  v1 );
	   vst1_u8(dst_arr + 16, v2 );	 
	   vst1_u8(dst_arr + 24, v3 );	   
   }
   CLKS_T1(tmc_opt);   
#endif     
   
}


void test_neon(void)
{
  //hello_neon();

  //intstruction test for signed/unsigned operation
  #if 0 
  uint8_t av[8]   = {127,10,0,1,2,128,254,255};
  uint8_t bv[8]   = {255, 253, 252 ,251, 0, 1, 2,3};
  short cv[8];
  //uint8_t bv[8] = {7,6,5,4,3,2,1,0};
  uint8x8_t v0,v1;
  int16x8_t v2, v3, v4;

  v0 = vld1_u8(av);
  v1 = vld1_u8(bv);
  //need to add -flax-vector-conversions in c flag
  v2 = vmovl_u8(v0);
  v3 = vmovl_u8(v1);
  v2 = vmulq_n_s16 (v2, 2);
  v3 = vmulq_n_s16 (v3, 3);
  
  v2 = vsubq_s16 (v2, v3); 
  vst1q_s16 (cv, v2);
  LOGI_point(test_neon,"bv val %d %d %d %d %d %d %d %d", cv[0], cv[1], cv[2], cv[3],
  	cv[4], cv[5], cv[6], cv[7]);
  #endif

  #if 0 
  uint8_t av[8];
  int16_t bv[8] = {-32768, -2, 255, 0, 1, -256, 32767, 256};
  uint8x8_t v0;
  int16x8_t v1;
  v1 = vld1q_s16(bv);
  v0 = vqmovun_s16 (v1);
  vst1_u8 (av, v0); 

  LOGI_point(test_neon,"av val %d %d %d %d %d %d %d %d", av[0], av[1], av[2], av[3],
  	av[4], av[5], av[6], av[7]);  
  #endif 

  #if 0 
  int16_t av1[8];
  int16_t bv1[8] = {-23*64 - 33, -23*64 - 31, -23*64 - 32, -19, -95, -97, 127*64 + 25, 129*64 + 37};
  int16x8_t v11;
  v11 = vld1q_s16(bv1);
  v11 = vrshrq_n_s16 (v11, 6); 
  vst1q_s16 (av1, v11); 

  LOGI_point(test_neon,"%d %d", av1[0],( bv1[0] + 32)>>6);
  LOGI_point(test_neon,"%d %d", av1[1],( bv1[1] + 32)>>6);
  LOGI_point(test_neon,"%d %d", av1[2],( bv1[2] + 32)>>6);
  LOGI_point(test_neon,"%d %d", av1[3],( bv1[3] + 32)>>6);
  LOGI_point(test_neon,"%d %d", av1[4],( bv1[4] + 32)>>6);
  LOGI_point(test_neon,"%d %d", av1[5],( bv1[5] + 32)>>6);
  LOGI_point(test_neon,"%d %d", av1[6],( bv1[6] + 32)>>6);
  LOGI_point(test_neon,"%d %d", av1[7],( bv1[7] + 32)>>6);    
  #endif 

#if HAVE_NEON_INLINE
 #ifdef MC_CHK_CMP
   //chk_mc_opt();
   //chk_mc_opt_bi();
   //chk_mc_opt_s16();
   //chk_mc_opt_s16_epel();
   chk_mc_opt_bi_epel();
 #endif
#endif


  


    
}

