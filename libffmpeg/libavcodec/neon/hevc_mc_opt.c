/*
 *
 * Author Zhangli FunTV
 * SIMD OPTIMIZED FUNCTION FOR Motion Compensation in HEVC
 * 2015/02/02
 *
*/

#include "libavcodec/hevc_mc_opt.h"
#include <arm_neon.h>
#include "libavcodec/hevc_epel_neon_wrapper.h"


//-1,  4, -10, 58, 17, -5,  1
static void put_hevc_qpel_uni_h_opt8_mv1(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
	uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86;
	int16x8_t  v160, v161, v162, v163, v164, v165, v166;
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
	uint8x8_t  v1;
	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;


	int x, y;
	uint8_t		*src	  = _src - 3;
	ptrdiff_t	srcstride = _srcstride;
	uint8_t		*dst	  = _dst;
	ptrdiff_t	dststride = _dststride;

	v1 =  vld1_u8(f1);
	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);


	for (y = 0; y < height ; y++)
	{
	   for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
	   {
		 v80 = vld1_u8(src );
		 v81 = vld1_u8(src + 1);
		 v82 = vld1_u8(src + 2);
		 v83 = vld1_u8(src + 3);
		 v84 = vld1_u8(src + 4);
		 v85 = vld1_u8(src + 5);
		 v86 = vld1_u8(src + 6);

		 v161 = vshll_n_u8(v81,2);
		 v166 = vmovl_u8(v86);
         v161 = vmlsl_u8(v161, v80,v1);
		 v166 = vmlsl_u8(v166, v85,v5);
         v161 = vmlsl_u8(v161, v82, v10);
		 v166 = vmlal_u8(v166, v84, v17);
         v161 = vmlal_u8(v161, v83, v58);

		 v160 = vaddq_s16(v161 , v166);

		 v162 = vrshrq_n_s16(v160 , 6);
		 v80 = vqmovun_s16( v162 );

		 vst1_u8(dst, v80);

	   }

	   src += srcstride - x;
	   dst += dststride - x;

	}

}


static void put_hevc_qpel_uni_h_opt8_mv1_ns(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
	uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86;
	int16x8_t  v160, v161, v162, v163, v164, v165, v166;
	uint8_t    work_idx , work_arr[8];
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
	uint8x8_t  v1;
	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;


	int x, y;
	uint8_t		*src	  = _src - 3;
	ptrdiff_t	srcstride = _srcstride;
	uint8_t		*dst	  = _dst;
	ptrdiff_t	dststride = _dststride;

	v1 =  vld1_u8(f1);
	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);


	for (y = 0; y < height ; y++)
	{
	   for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
	   {
		 v80 = vld1_u8(src );
		 v81 = vld1_u8(src + 1);
		 v82 = vld1_u8(src + 2);
		 v83 = vld1_u8(src + 3);
		 v84 = vld1_u8(src + 4);
		 v85 = vld1_u8(src + 5);
		 v86 = vld1_u8(src + 6);

		 v161 = vshll_n_u8(v81,2);
		 v166 = vmovl_u8(v86);
         v161 = vmlsl_u8(v161, v80,v1);
		 v166 = vmlsl_u8(v166, v85,v5);
         v161 = vmlsl_u8(v161, v82, v10);
		 v166 = vmlal_u8(v166, v84, v17);
         v161 = vmlal_u8(v161, v83, v58);

		 v160 = vaddq_s16(v161 , v166);

		 v162 = vrshrq_n_s16(v160 , 6);
		 v80 = vqmovun_s16( v162 );


		 if ( (x + 8 ) <= width )
		 {
			 vst1_u8(dst, v80);
		 }
		 else
		 {
			 vst1_u8(work_arr, v80);
			 for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			 {
			   dst[work_idx] = work_arr[work_idx];
			 }
		 }

	   }
	   src += srcstride - x;
	   dst += dststride - x;

	}
}

// -1,  4,-11, 40, 40,-11,  4, -1
static void put_hevc_qpel_uni_h_opt8_mv2(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v160, v161, v162, v163, v164, v165, v166, v167;
   uint8_t    f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t    f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
   uint8x8_t  v11;
   uint8x8_t  v40;
   uint8x8_t  v1;

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   uint8_t     *dst      = _dst;
   ptrdiff_t   dststride = _dststride;

   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);
   v1 = vld1_u8(f1);

   for (y = 0; y < height ;y++)
   {

      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {
	    v80 = vld1_u8(src);
	    v81 = vld1_u8(src + 1);
	    v82 = vld1_u8(src + 2);
	    v83 = vld1_u8(src + 3);
	    v84 = vld1_u8(src + 4);
	    v85 = vld1_u8(src + 5);
	    v86 = vld1_u8(src + 6);
	    v87 = vld1_u8(src + 7);

 	    v161 = vshll_n_u8 (v81,2);
		v166 = vshll_n_u8 (v86,2);
		v161 = vmlsl_u8(v161, v80, v1);
		v166 = vmlsl_u8(v166, v87, v1);
	    v161 = vmlsl_u8(v161, v82, v11);
		v166 = vmlsl_u8(v166, v85, v11);
	    v161 = vmlal_u8(v161, v83, v40);
		v166 = vmlal_u8(v166, v84, v40);

 		v160 = vaddq_s16(v161 , v166);

		v162 = vrshrq_n_s16(v160 , 6);
		v80 = vqmovun_s16( v162 );

		vst1_u8(dst, v80);

      }
	  src += srcstride - x;
      dst += dststride - x;

   }
}

static void put_hevc_qpel_uni_h_opt8_mv2_ns(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v160, v161, v162, v163, v164, v165, v166, v167;
   uint8_t    work_idx , work_arr[8];
   uint8_t    f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t    f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
   uint8x8_t  v11;
   uint8x8_t  v40;
   uint8x8_t  v1;

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   uint8_t     *dst      = _dst;
   ptrdiff_t   dststride = _dststride;

   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);
   v1 = vld1_u8(f1);

   for (y = 0; y < height ;y++)
   {

      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {
	    v80 = vld1_u8(src);
	    v81 = vld1_u8(src + 1);
	    v82 = vld1_u8(src + 2);
	    v83 = vld1_u8(src + 3);
	    v84 = vld1_u8(src + 4);
	    v85 = vld1_u8(src + 5);
	    v86 = vld1_u8(src + 6);
	    v87 = vld1_u8(src + 7);

 	    v161 = vshll_n_u8 (v81,2);
		v166 = vshll_n_u8 (v86,2);
		v161 = vmlsl_u8(v161, v80, v1);
		v166 = vmlsl_u8(v166, v87, v1);
	    v161 = vmlsl_u8(v161, v82, v11);
		v166 = vmlsl_u8(v166, v85, v11);
	    v161 = vmlal_u8(v161, v83, v40);
		v166 = vmlal_u8(v166, v84, v40);

 		v160 = vaddq_s16(v161 , v166);

		v162 = vrshrq_n_s16(v160 , 6);
		v80 = vqmovun_s16( v162 );


		if ( (x + 8 ) <= width )
		{
		    vst1_u8(dst, v80);
		}
		else
		{
			vst1_u8(work_arr, v80);
			for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			{
			  dst[work_idx] = work_arr[work_idx];
			}
		}
      }
	  src += srcstride - x;
      dst += dststride - x;

   }
}

static void put_hevc_qpel_uni_h_opt8_mv3(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v161, v162, v163, v164, v165, v166, v167;
   uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t    f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   uint8_t     *dst      = _dst;
   ptrdiff_t   dststride = _dststride;

   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);


   for (y = 0; y < height ;y++)
   {
      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {

		v81 = vld1_u8(src + 1);
		v82 = vld1_u8(src + 2);
		v83 = vld1_u8(src + 3);
		v84 = vld1_u8(src + 4);
		v85 = vld1_u8(src + 5);
		v86 = vld1_u8(src + 6);
		v87 = vld1_u8(src + 7);

	    v161 = vmovl_u8(v81);
	    v166 = vshll_n_u8(v86 , 2);
	    v161 = vmlsl_u8(v161, v82,v5);
	    v166 = vmlsl_u8(v166, v87, v1);
	    v161 = vmlal_u8(v161, v83, v17);
	    v166 = vmlsl_u8(v166,v85,v10);
	    v161 = vmlal_u8(v161, v84, v58);

 		v161 = vaddq_s16(v161 , v166);

		v162 = vrshrq_n_s16(v161 , 6);
		v81 = vqmovun_s16( v162 );

		vst1_u8(dst, v81);
      }

	  src += srcstride - x;
      dst += dststride - x;

   }
}


//1, -5, 17, 58, -10,  4, -1,
static void put_hevc_qpel_uni_h_opt8_mv3_ns(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v161, v162, v163, v164, v165, v166, v167;
   uint8_t    work_idx , work_arr[8];
   uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t    f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   uint8_t     *dst      = _dst;
   ptrdiff_t   dststride = _dststride;

   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);


   for (y = 0; y < height ;y++)
   {
      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {

		v81 = vld1_u8(src + 1);
		v82 = vld1_u8(src + 2);
		v83 = vld1_u8(src + 3);
		v84 = vld1_u8(src + 4);
		v85 = vld1_u8(src + 5);
		v86 = vld1_u8(src + 6);
		v87 = vld1_u8(src + 7);

	    v161 = vmovl_u8(v81);
	    v166 = vshll_n_u8(v86 , 2);
	    v161 = vmlsl_u8(v161, v82,v5);
	    v166 = vmlsl_u8(v166, v87, v1);
	    v161 = vmlal_u8(v161, v83, v17);
	    v166 = vmlsl_u8(v166,v85,v10);
	    v161 = vmlal_u8(v161, v84, v58);

 		v161 = vaddq_s16(v161 , v166);

		v162 = vrshrq_n_s16(v161 , 6);
		v81 = vqmovun_s16( v162 );


		if ( (x + 8 ) <= width )
		{
		    vst1_u8(dst, v81);
		}
		else
		{
			vst1_u8(work_arr, v81);
			for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			{
			  dst[work_idx] = work_arr[work_idx];
			}
		}

      }

	  src += srcstride - x;
      dst += dststride - x;

   }
}


//1, -5, 17, 58, -10,  4, -1,
void put_hevc_qpel_uni_h_opt_8(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
   //CLK_DEF(o_tput_hevc_qpel_uni_h);
   //CLK_T0(o_tput_hevc_qpel_uni_h);

   if ((width & 7 ) == 0 )
   {
	if (mx == 1)
	{
	 put_hevc_qpel_uni_h_opt8_mv1(_dst, _dststride,_src, _srcstride,
									  height, width);	   
	}
	else if (mx == 2)
	{	   
	 put_hevc_qpel_uni_h_opt8_mv2(_dst, _dststride,_src, _srcstride,
									 height, width);	
	}
	else
	{
	 put_hevc_qpel_uni_h_opt8_mv3(_dst, _dststride,_src, _srcstride,
									  height, width);	 
	}	  
   }
   else
   {
   
	if (mx == 1)
	{  
	 put_hevc_qpel_uni_h_opt8_mv1_ns(_dst, _dststride,_src, _srcstride,
									  height, width);	   
	}
	else if (mx == 2)
	{  
	 put_hevc_qpel_uni_h_opt8_mv2_ns(_dst, _dststride,_src, _srcstride,
									 height, width);	
	}
	else
	{  
	 put_hevc_qpel_uni_h_opt8_mv3_ns(_dst, _dststride,_src, _srcstride,
									  height, width);	 
	}	  
   }
   //CLK_T1(o_tput_hevc_qpel_uni_h);
}


//-1,  4,-10, 58, 17, -5,  1,  0,
static void put_hevc_qpel_uni_v_opt8_mv1(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v8_int;
   int16x8_t  v160, v161, v162, v163, v164, v165, v166;
   uint8_t	  f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t	  f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t	  f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t	  f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t	  f1[8] = {1,1,1,1,1,1,1,1};

   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;


   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);

   int        x, y;
   uint8_t     *src;
   uint8_t     *dst;

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;

		   v161 = vshll_n_u8 (v81,2);
		   v166 = vmovl_u8 (v86);
		   v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v85,v5);
		   v161 = vmlsl_u8(v161,v82,v10);
		   v166 = vmlal_u8(v166,v84,v17);
		   v161 = vmlal_u8(v161,v83,v58);

 		   v166 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v82  = v83;
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;
		   v86  = vld1_u8(src);

		   v162 = vrshrq_n_s16(v166 , 6);
		   v8_int = vqmovun_s16( v162 );
		   vst1_u8(dst, v8_int);

	  }
   }
}

static void put_hevc_qpel_uni_v_opt8_mv1_ns(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v8_int;
   int16x8_t  v160, v161, v162, v163, v164, v165, v166;
   uint8_t    work_idx , work_arr[8];
   uint8_t	  f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t	  f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t	  f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t	  f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t	  f1[8] = {1,1,1,1,1,1,1,1};

   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;


   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);

   int        x, y;
   uint8_t    *src;
   uint8_t    *dst;

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;

		   v161 = vshll_n_u8 (v81,2);
		   v166 = vmovl_u8 (v86);
		   v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v85,v5);
		   v161 = vmlsl_u8(v161,v82,v10);
		   v166 = vmlal_u8(v166,v84,v17);
		   v161 = vmlal_u8(v161,v83,v58);

 		   v166 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v82  = v83;
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;
		   v86  = vld1_u8(src);

		   v162 = vrshrq_n_s16(v166 , 6);
		   v8_int = vqmovun_s16( v162 );

		   if ( (x + 8 ) <= width )
		   {
		      vst1_u8(dst, v8_int);
		   }
		   else
		   {
		      vst1_u8(work_arr, v8_int);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
			      dst[work_idx] = work_arr[work_idx];
			  }
		   }
	  }
   }
}

static void put_hevc_qpel_uni_v_opt8_mv2(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t   v80, v81, v82, v83 ,v84, v85, v86, v87, v8_int;
   int16x8_t   v160, v161, v162, v163, v164, v165, v166, v167;
   uint8_t     f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t     f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t     f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t   v11;
   uint8x8_t   v40;
   uint8x8_t   v1;

   int         x, y;
   uint8_t     *src;
   uint8_t     *dst;

   v1  = vld1_u8(f1);
   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;

	       v161 = vshll_n_u8 (v81,2);
		   v166 = vshll_n_u8 (v86,2);
	       v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v87,v1);
		   v161 = vmlsl_u8(v161,v82,v11);
	       v166 = vmlsl_u8(v166,v85,v11);
	       v161 = vmlal_u8(v161,v83,v40);
	       v166 = vmlal_u8(v166,v84,v40);

 		   v161 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v82  = v83;
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;
 		   v86  = v87;
		   v87  = vld1_u8(src);

		   v162 = vrshrq_n_s16(v161 , 6);
		   v8_int = vqmovun_s16( v162 );

		   vst1_u8(dst, v8_int);

	  }
   }
}

static void put_hevc_qpel_uni_v_opt8_mv2_ns(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t   v80, v81, v82, v83 ,v84, v85, v86, v87, v8_int;
   int16x8_t   v160, v161, v162, v163, v164, v165, v166, v167;
   uint8_t     work_idx , work_arr[8];
   uint8_t     f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t     f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t     f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t   v11;
   uint8x8_t   v40;
   uint8x8_t   v1;

   int         x, y;
   uint8_t     *src;
   uint8_t     *dst;

   v1  = vld1_u8(f1);
   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;

	       v161 = vshll_n_u8 (v81,2);
		   v166 = vshll_n_u8 (v86,2);
	       v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v87,v1);
		   v161 = vmlsl_u8(v161,v82,v11);
	       v166 = vmlsl_u8(v166,v85,v11);
	       v161 = vmlal_u8(v161,v83,v40);
	       v166 = vmlal_u8(v166,v84,v40);

 		   v161 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v82  = v83;
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;
 		   v86  = v87;
		   v87  = vld1_u8(src);

		   v162 = vrshrq_n_s16(v161 , 6);
		   v8_int = vqmovun_s16( v162 );

		   if ( (x + 8 ) <= width )
		   {
		      vst1_u8(dst, v8_int);
		   }
		   else
		   {
		      vst1_u8(work_arr, v8_int);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
			      dst[work_idx] = work_arr[work_idx];
			  }
		   }
	  }
   }
}


//0,  1, -5, 17, 58,-10,  4, -1,
static void put_hevc_qpel_uni_v_opt8_mv3(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
	uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87, v8_int;
	int16x8_t  v161, v162, v163, v164, v165, v166, v167;
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8] = {1,1,1,1,1,1,1,1};

	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;
	uint8x8_t  v1;

	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);
	v1  = vld1_u8(f1);

	int 	   x, y;
	uint8_t	   *src;
	uint8_t	   *dst;


   _src = _src - 2*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);
	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;
		   //0,  1, -5, 17, 58, -10,  4, -1,
	       v161 = vmovl_u8(v81);
		   v166 = vshll_n_u8 (v86,2);
		   v161 = vmlsl_u8(v161, v82, v5);
		   v166 = vmlsl_u8(v166, v87, v1);
	       v161 = vmlal_u8(v161, v83, v17);
		   v166 = vmlsl_u8(v166, v85, v10);
	       v161 = vmlal_u8(v161, v84, v58);

 		   v161 = vaddq_s16(v161 , v166);

 		   v81  = v82;
 		   v82  = v83;
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;
 		   v86  = v87;
		   v87  = vld1_u8(src);

		   v162 = vrshrq_n_s16(v161 , 6);
		   v8_int = vqmovun_s16( v162 );

		   vst1_u8(dst, v8_int);
	  }
   }
}

//0,  1, -5, 17, 58,-10,  4, -1,
static void put_hevc_qpel_uni_v_opt8_mv3_ns(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
	uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87, v8_int;
	int16x8_t  v161, v162, v163, v164, v165, v166, v167;
	uint8_t    work_idx , work_arr[8];
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8] = {1,1,1,1,1,1,1,1};

	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;
	uint8x8_t  v1;

	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);
	v1  = vld1_u8(f1);

	int 	   x, y;
	uint8_t	   *src;
	uint8_t	   *dst;


   _src = _src - 2*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);
	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;
		   //0,  1, -5, 17, 58, -10,  4, -1,
	       v161 = vmovl_u8(v81);
		   v166 = vshll_n_u8 (v86,2);
		   v161 = vmlsl_u8(v161, v82, v5);
		   v166 = vmlsl_u8(v166, v87, v1);
	       v161 = vmlal_u8(v161, v83, v17);
		   v166 = vmlsl_u8(v166, v85, v10);
	       v161 = vmlal_u8(v161, v84, v58);

 		   v161 = vaddq_s16(v161 , v166);

 		   v81  = v82;
 		   v82  = v83;
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;
 		   v86  = v87;
		   v87  = vld1_u8(src);

		   v162 = vrshrq_n_s16(v161 , 6);
		   v8_int = vqmovun_s16( v162 );

		   if ( (x + 8 ) <= width )
		   {
		      vst1_u8(dst, v8_int);
		   }
		   else
		   {
		      vst1_u8(work_arr, v8_int);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
			      dst[work_idx] = work_arr[work_idx];
			  }
		   }
	  }
   }
}

void put_hevc_qpel_uni_v_opt_8(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
	//CLK_DEF(o_tput_hevc_qpel_uni_v);
	//CLK_T0(o_tput_hevc_qpel_uni_v);

	
	if ((width & 7 ) == 0 )
	{
	 if (my == 1)
	 {
	  put_hevc_qpel_uni_v_opt8_mv1(_dst, _dststride,_src, _srcstride,
									   height, width);		
	 }
	 else if (my == 2)
	 {		
	  put_hevc_qpel_uni_v_opt8_mv2(_dst, _dststride,_src, _srcstride,
									  height, width);	 
	 }
	 else
	 {
	  put_hevc_qpel_uni_v_opt8_mv3(_dst, _dststride,_src, _srcstride,
									   height, width);	  
	 }	   
	}
	else
	{

	 if (my == 1)
	 {	
	  put_hevc_qpel_uni_v_opt8_mv1_ns(_dst, _dststride,_src, _srcstride,
									   height, width);		
	 }
	 else if (my == 2)
	 {	
	  put_hevc_qpel_uni_v_opt8_mv2_ns(_dst, _dststride,_src, _srcstride,
									  height, width);	 
	 }
	 else
	 {	
	  put_hevc_qpel_uni_v_opt8_mv3_ns(_dst, _dststride,_src, _srcstride,
									   height, width);	  
	 }	   
	}		
	
	//CLK_T1(o_tput_hevc_qpel_uni_v);   


}

void put_hevc_qpel_uni_hv_opt_8(uint8_t *_dst,  ptrdiff_t _dststride,
                          uint8_t *_src, ptrdiff_t _srcstride,
                          int height, intptr_t mx, intptr_t my, int width)
{
    uint8_t tmp_array[(MAX_PB_SIZE + QPEL_EXTRA) * MAX_PB_SIZE];
	uint8_t * src    = _src - QPEL_EXTRA_BEFORE * _srcstride;
    uint8_t * tmp    = tmp_array + QPEL_EXTRA_BEFORE * MAX_PB_SIZE;	
	//CLK_DEF(o_tput_hevc_qpel_uni_hv);
	//CLK_T0(o_tput_hevc_qpel_uni_hv);
	put_hevc_qpel_uni_h_opt_8(tmp_array,  MAX_PB_SIZE,
                              src, _srcstride,
                              height + QPEL_EXTRA , mx, 0, width);
	put_hevc_qpel_uni_v_opt_8(_dst,  _dststride,
                              tmp, MAX_PB_SIZE,
                              height, 0, my, width);
	//CLK_T1(o_tput_hevc_qpel_uni_hv);	
	
}

static void put_hevc_epel_uni_h_opt_8_s(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83;
	int16x8_t  v160, v161, v162, v163;

    int x, y;
    uint8_t *src          = _src - 1;
    ptrdiff_t srcstride   = _srcstride;
    uint8_t *dst          = _dst;
    ptrdiff_t dststride   = _dststride;

    uint8x8_t  vf0;
    uint8x8_t  vf1;
    uint8x8_t  vf2;
    uint8x8_t  vf3;
    int shift = 14 - BIT_DEPTH;

	switch( mx )
	{
	  case   1:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(58);
	    vf2 = vdup_n_u8(10);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  2:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(54);
	    vf2 = vdup_n_u8(16);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  3:
	    vf0 = vdup_n_u8(6);
	    vf1 = vdup_n_u8(46);
	    vf2 = vdup_n_u8(28);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  4:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(36);
	    vf2 = vdup_n_u8(36);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  5:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(28);
	    vf2 = vdup_n_u8(46);
	    vf3 = vdup_n_u8(6);
	  	break;
	  case  6:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(16);
	    vf2 = vdup_n_u8(54);
	    vf3 = vdup_n_u8(4);
	  	break;
	  default :
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(10);
	    vf2 = vdup_n_u8(58);
	    vf3 = vdup_n_u8(2);
	}


	for (y = 0; y < height ;y++)
	{
	   for (x = 0 ; x < width ; dst += 8)
	   {
		v80 = vld1_u8(src);
		v81 = vld1_u8(src + 1);
		v82 = vld1_u8(src + 2);
		v83 = vld1_u8(src + 3);

		v161 = vmull_u8(v81,vf1);
		v162 = vmull_u8(v82,vf2);
		v161 = vmlsl_u8(v161,v80,vf0);
		v162 = vmlsl_u8(v162,v83,vf3);

		 v161 = vaddq_s16(v161 , v162);

		 v163 = vrshrq_n_s16(v161 , 6);
		 src += 8;
		 v80 = vqmovun_s16( v163 );
		 x += 8;
		 vst1_u8(dst, v80);

	   }
	   src += srcstride - x;
	   dst += dststride - x;

	}
}


static void put_hevc_epel_uni_h_opt_8_ns(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83;
	int16x8_t  v160, v161, v162, v163;
	uint8_t    work_idx , work_arr[8];

    int x, y;
    uint8_t *src          = _src - 1;
    ptrdiff_t srcstride   = _srcstride;
    uint8_t *dst          = _dst;
    ptrdiff_t dststride   = _dststride;


    uint8x8_t  vf0;
    uint8x8_t  vf1;
    uint8x8_t  vf2;
    uint8x8_t  vf3;
    int shift = 14 - BIT_DEPTH;

	switch( mx )
	{
	  case   1:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(58);
	    vf2 = vdup_n_u8(10);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  2:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(54);
	    vf2 = vdup_n_u8(16);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  3:
	    vf0 = vdup_n_u8(6);
	    vf1 = vdup_n_u8(46);
	    vf2 = vdup_n_u8(28);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  4:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(36);
	    vf2 = vdup_n_u8(36);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  5:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(28);
	    vf2 = vdup_n_u8(46);
	    vf3 = vdup_n_u8(6);
	  	break;
	  case  6:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(16);
	    vf2 = vdup_n_u8(54);
	    vf3 = vdup_n_u8(4);
	  	break;
	  default :
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(10);
	    vf2 = vdup_n_u8(58);
	    vf3 = vdup_n_u8(2);
	}

	for (y = 0; y < height ;y++)
	{
	   for (x = 0 ; x < width ;x += 8, dst += 8)
	   {
		 v80 = vld1_u8(src);
		 v81 = vld1_u8(src + 1);
		 v82 = vld1_u8(src + 2);
		 v83 = vld1_u8(src + 3);

		 v161 = vmull_u8(v81,vf1);
		 v162 = vmull_u8(v82,vf2);
		 v161 = vmlsl_u8(v161,v80,vf0);
		 v162 = vmlsl_u8(v162,v83,vf3);

		 v161 = vaddq_s16(v161 , v162);

		 v163 = vrshrq_n_s16(v161 , 6);
		 src += 8;
		 v80 = vqmovun_s16( v163 );

		  if ( (x + 8 ) <= width )
		  {
			  vst1_u8(dst, v80);
		  }
		  else
		  {
			  vst1_u8(work_arr, v80);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
				dst[work_idx] = work_arr[work_idx];
			  }
		  }
	   }
	   src += srcstride - x;
	   dst += dststride - x;
	}

}

void put_hevc_epel_uni_h_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{

#ifndef ASM_OPT
   //CLK_DEF(o_tput_hevc_epel_uni_h);
   //CLK_T0(o_tput_hevc_epel_uni_h);	
   if ( (width & 7 ) == 0 )
        put_hevc_epel_uni_h_opt_8_s(_dst, _dststride, _src, _srcstride,
                                  height, mx, my, width);   	   
   else	
        put_hevc_epel_uni_h_opt_8_ns(_dst, _dststride, _src, _srcstride,
                                  height, mx, my, width);    	
   //CLK_T1(o_tput_hevc_epel_uni_h);
#else
   neon_put_hevc_epel_uni_h_opt_8(_dst,  _dststride, _src, _srcstride,
                                      height, mx, my, width);

#endif
}

static void put_hevc_epel_uni_v_opt_8_s(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83, v8_int;
	int16x8_t  v160, v161, v162, v163;

    int x, y;
    uint8_t *src          ;
    ptrdiff_t srcstride   = _srcstride;
    uint8_t *dst          = _dst;
    ptrdiff_t dststride   = _dststride;

    uint8x8_t  vf0;
    uint8x8_t  vf1;
    uint8x8_t  vf2;
    uint8x8_t  vf3;
    int shift = 14 - BIT_DEPTH;

    
	switch( my )
	{
	  case   1:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(58);
	    vf2 = vdup_n_u8(10);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  2:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(54);
	    vf2 = vdup_n_u8(16);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  3:
	    vf0 = vdup_n_u8(6);
	    vf1 = vdup_n_u8(46);
	    vf2 = vdup_n_u8(28);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  4:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(36);
	    vf2 = vdup_n_u8(36);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  5:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(28);
	    vf2 = vdup_n_u8(46);
	    vf3 = vdup_n_u8(6);
	  	break;
	  case  6:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(16);
	    vf2 = vdup_n_u8(54);
	    vf3 = vdup_n_u8(4);
	  	break;
	  default :
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(10);
	    vf2 = vdup_n_u8(58);
	    vf3 = vdup_n_u8(2);
	}


    _src = _src - _srcstride;
	for (x = 0 ; x < width; x += 8)
	{
	   src = _src + x;
	   dst = _dst + x;
	   v80 = vld1_u8(src);
	   src += _srcstride;
	   v81 = vld1_u8(src);
	   src += _srcstride;
	   v82 = vld1_u8(src);
	   src += _srcstride;
	   v83 = vld1_u8(src);

	   for (y = 0; y < height ; y++, dst += _dststride)
	   {
			src += _srcstride;
            v161 = vmull_u8(v81,vf1);
            v162 = vmull_u8(v82,vf2);
            v161 = vmlsl_u8(v161,v80,vf0);
            v162 = vmlsl_u8(v162,v83,vf3);
            v80  = v81;
			v161 = vaddq_s16(v161 , v162);
            v81  = v82;
            v82  = v83;
			v8_int = vqrshrun_n_s16 ( v161, 6 );
            v83  = vld1_u8(src);
			vst1_u8(dst, v8_int);
	   }
	}

}


static void put_hevc_epel_uni_v_opt_8_ns(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
		uint8x8_t  v80, v81, v82, v83, v8_int;
		int16x8_t  v160, v161, v162, v163;
	    uint8_t    work_idx , work_arr[8];
		int x, y;
		uint8_t *src		  ;
		ptrdiff_t srcstride   = _srcstride;
		uint8_t *dst		  = _dst;
		ptrdiff_t dststride   = _dststride;

		uint8x8_t  vf0;
		uint8x8_t  vf1;
		uint8x8_t  vf2;
		uint8x8_t  vf3;
		int shift = 14 - BIT_DEPTH;

		switch( my )
		{
		  case   1:
		    vf0 = vdup_n_u8(2);
		    vf1 = vdup_n_u8(58);
		    vf2 = vdup_n_u8(10);
		    vf3 = vdup_n_u8(2);
		  	break;
		  case  2:
		    vf0 = vdup_n_u8(4);
		    vf1 = vdup_n_u8(54);
		    vf2 = vdup_n_u8(16);
		    vf3 = vdup_n_u8(2);
		  	break;
		  case  3:
		    vf0 = vdup_n_u8(6);
		    vf1 = vdup_n_u8(46);
		    vf2 = vdup_n_u8(28);
		    vf3 = vdup_n_u8(4);
		  	break;
		  case  4:
		    vf0 = vdup_n_u8(4);
		    vf1 = vdup_n_u8(36);
		    vf2 = vdup_n_u8(36);
		    vf3 = vdup_n_u8(4);
		  	break;
		  case  5:
		    vf0 = vdup_n_u8(4);
		    vf1 = vdup_n_u8(28);
		    vf2 = vdup_n_u8(46);
		    vf3 = vdup_n_u8(6);
		  	break;
		  case  6:
		    vf0 = vdup_n_u8(2);
		    vf1 = vdup_n_u8(16);
		    vf2 = vdup_n_u8(54);
		    vf3 = vdup_n_u8(4);
		  	break;
		  default :
		    vf0 = vdup_n_u8(2);
		    vf1 = vdup_n_u8(10);
		    vf2 = vdup_n_u8(58);
		    vf3 = vdup_n_u8(2);
		}

        _src = _src - _srcstride;
		for (x = 0 ; x < width; x += 8)
		{
		   src = _src + x;
		   dst = _dst + x;
		   v80 = vld1_u8(src);
		   src += _srcstride;
		   v81 = vld1_u8(src);
		   src += _srcstride;
		   v82 = vld1_u8(src);
		   src += _srcstride;
		   v83 = vld1_u8(src);

           if (x + 8 <= width) 
           {
		     for (y = 0; y < height ; y++, dst += _dststride)
		     {
				src += _srcstride;
	            v161 = vmull_u8(v81,vf1);
	            v162 = vmull_u8(v82,vf2);
	            v161 = vmlsl_u8(v161,v80,vf0);
	            v162 = vmlsl_u8(v162,v83,vf3);
	            v80  = v81;
				v161 = vaddq_s16(v161 , v162);
	            v81  = v82;
	            v82  = v83;
				v8_int = vqrshrun_n_s16 ( v161 , 6 );
	            v83  = vld1_u8(src);

		        vst1_u8(dst, v8_int);
					        
		     }           
           }
		   else
		   {
		     for (y = 0; y < height ; y++, dst += _dststride)
		     {
				src += _srcstride;
	            v161 = vmull_u8(v81,vf1);
	            v162 = vmull_u8(v82,vf2);
	            v161 = vmlsl_u8(v161,v80,vf0);
	            v162 = vmlsl_u8(v162,v83,vf3);
	            v80  = v81;
				v161 = vaddq_s16(v161 , v162);
	            v81  = v82;
	            v82  = v83;
				v8_int = vqrshrun_n_s16 ( v161 , 6 );
	            v83  = vld1_u8(src);

                
                if ( (width & 7) == 4 )
                {
                  vst1_lane_u32 ((uint32_t*)dst, v8_int, 0); 
                }
				else			    
				{
		          vst1_u8(work_arr, v8_int);
			      for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			      {
			        dst[work_idx] = work_arr[work_idx];
			      }					
				}
				
	        			    
		      }		   
		   }

		}
}

void put_hevc_epel_uni_v_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
   //CLK_DEF(o_tput_hevc_epel_uni_v);
   //CLK_T0(o_tput_hevc_epel_uni_v);	
   if ( (width & 7 ) == 0 )
        put_hevc_epel_uni_v_opt_8_s(_dst, _dststride, _src, _srcstride,
                                  height, mx, my, width);   	   
   else	
        put_hevc_epel_uni_v_opt_8_ns(_dst, _dststride, _src, _srcstride,
                                  height, mx, my, width);    	
   //CLK_T1(o_tput_hevc_epel_uni_v);
}


void put_hevc_epel_uni_hv_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
   uint8_t tmp_array[(MAX_PB_SIZE + EPEL_EXTRA) * MAX_PB_SIZE];
   uint8_t * src	 = _src - EPEL_EXTRA_BEFORE * _srcstride;
   uint8_t * tmp	 = tmp_array + EPEL_EXTRA_BEFORE * MAX_PB_SIZE; 
   //CLK_DEF(o_tput_hevc_epel_uni_hv);
   //CLK_T0(o_tput_hevc_epel_uni_hv);
   put_hevc_epel_uni_h_opt_8(tmp_array,  MAX_PB_SIZE,
						  src, _srcstride,
						  height + EPEL_EXTRA , mx, 0, width);
   put_hevc_epel_uni_v_opt_8(_dst,  _dststride,
						  tmp, MAX_PB_SIZE,
						  height, 0, my, width);
   //CLK_T1(o_tput_hevc_epel_uni_hv);
}


static void put_hevc_pel_bi_pixels_opt_8_w2(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{ 
    int x, y;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};
	uint8_t    work_idx , work_arr[8];	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {
        v0 = vld1_u8(_src);
		v2 = vld1q_s16(src2);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );		   
		vst1_u8(work_arr, v1);
		
		_dst[0] = work_arr[0];
		_dst[1] = work_arr[1];		
        _src += _srcstride;
        _dst += _dststride;
        src2 += MAX_PB_SIZE;
    }
	
}

static void put_hevc_pel_bi_pixels_opt_8_w4(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{ 
    int y;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};
	uint8_t    work_idx , work_arr[8];	
	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {
        v0 = vld1_u8(_src);
		v2 = vld1q_s16(src2);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );			   
		vst1_u8(work_arr, v1);	
		
		_dst[0] = work_arr[0];
		_dst[1] = work_arr[1];
		_dst[2] = work_arr[2];
		_dst[3] = work_arr[3];		
        _src += _srcstride;
        _dst += _dststride;
        src2 += MAX_PB_SIZE;
    }	
}

static void put_hevc_pel_bi_pixels_opt_8_w6(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{ 
    int y;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};
	uint8_t    work_idx , work_arr[8];	
	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {
        v0 = vld1_u8(_src);
		v2 = vld1q_s16(src2);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	 
		vst1_u8(work_arr, v1);	
		
		_dst[0] = work_arr[0];
		_dst[1] = work_arr[1];
		_dst[2] = work_arr[2];
		_dst[3] = work_arr[3];
		_dst[4] = work_arr[4];
		_dst[5] = work_arr[5];		
        _src += _srcstride;
        _dst += _dststride;
        src2 += MAX_PB_SIZE;
    }	
}


static void put_hevc_pel_bi_pixels_opt_8_w8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{ 
    int y;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};
	uint8_t    work_idx , work_arr[8];	
	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {
        v0 = vld1_u8(_src);
		v2 = vld1q_s16(src2);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );			   
		vst1_u8(_dst, v1);	
		
        _src += _srcstride;
        _dst += _dststride;
        src2 += MAX_PB_SIZE;
    }	
}

static void put_hevc_pel_bi_pixels_opt_8_w12(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{ 
    int y;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};
	uint8_t    work_idx , work_arr[8];	
	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {
        v0 = vld1_u8(_src);
		v2 = vld1q_s16(src2);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst, v1);	

        v0 = vld1_u8(_src + 8);
		v2 = vld1q_s16(src2 + 8);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(work_arr, v1);	
		
		_dst[8] = work_arr[0];
		_dst[9] = work_arr[1];
		_dst[10] = work_arr[2];
		_dst[11] = work_arr[3];		
        _src += _srcstride;
        _dst += _dststride;
        src2 += MAX_PB_SIZE;
    }	
}

static void put_hevc_pel_bi_pixels_opt_8_w16(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{ 
    int y;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};	
	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {
        v0 = vld1_u8(_src);
		v2 = vld1q_s16(src2);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst, v1);	

        v0 = vld1_u8(_src + 8);
		v2 = vld1q_s16(src2 + 8);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 8, v1);	
			
        _src += _srcstride;
        _dst += _dststride;
        src2 += MAX_PB_SIZE;
    }	
}

static void put_hevc_pel_bi_pixels_opt_8_w24(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{ 
    int y;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};	
	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {
        v0 = vld1_u8(_src);
		v2 = vld1q_s16(src2);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst, v1);	

        v0 = vld1_u8(_src + 8);
		v2 = vld1q_s16(src2 + 8);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 8, v1);	
		
        v0 = vld1_u8(_src + 16);
		v2 = vld1q_s16(src2 + 16);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 16, v1);	

        _src += _srcstride;
        _dst += _dststride;
        src2 += MAX_PB_SIZE;
    }	
}

static void put_hevc_pel_bi_pixels_opt_8_w32(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{ 
    int y;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};
	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {

        v0 = vld1_u8(_src);
		v2 = vld1q_s16(src2);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst, v1);	

        v0 = vld1_u8(_src + 8);
		v2 = vld1q_s16(src2 + 8);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 8, v1);	
		
        v0 = vld1_u8(_src + 16);
		v2 = vld1q_s16(src2 + 16);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 16, v1);	

        v0 = vld1_u8(_src + 24);
		v2 = vld1q_s16(src2 + 24);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 24, v1);			
        
        _src += _srcstride;
        _dst += _dststride;
        src2 += MAX_PB_SIZE;
    }	
}

static void put_hevc_pel_bi_pixels_opt_8_w48(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{ 
    int y;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};	
	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {
        v0 = vld1_u8(_src);
		v2 = vld1q_s16(src2);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst, v1);	

        v0 = vld1_u8(_src + 8);
		v2 = vld1q_s16(src2 + 8);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 8, v1);	
		
        v0 = vld1_u8(_src + 16);
		v2 = vld1q_s16(src2 + 16);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 16, v1);	

        v0 = vld1_u8(_src + 24);
		v2 = vld1q_s16(src2 + 24);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 24, v1);	

        v0 = vld1_u8(_src + 32);
		v2 = vld1q_s16(src2 + 32);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 32, v1);	

        v0 = vld1_u8(_src + 40);
		v2 = vld1q_s16(src2 + 40);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 40, v1);			

        _src += _srcstride;
        _dst += _dststride;
        src2 += MAX_PB_SIZE;
    }	
}

static void put_hevc_pel_bi_pixels_opt_8_w64(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{ 
    int y;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};	
	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {

        v0 = vld1_u8(_src);
		v2 = vld1q_s16(src2);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst, v1);	

        v0 = vld1_u8(_src + 8);
		v2 = vld1q_s16(src2 + 8);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 8, v1);	
		
        v0 = vld1_u8(_src + 16);
		v2 = vld1q_s16(src2 + 16);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 16, v1);	

        v0 = vld1_u8(_src + 24);
		v2 = vld1q_s16(src2 + 24);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 24, v1);	

        v0 = vld1_u8(_src + 32);
		v2 = vld1q_s16(src2 + 32);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 32, v1);	

        v0 = vld1_u8(_src + 40);
		v2 = vld1q_s16(src2 + 40);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 40, v1);	

        v0 = vld1_u8(_src + 48);
		v2 = vld1q_s16(src2 + 48);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 48, v1);	


        v0 = vld1_u8(_src + 56);
		v2 = vld1q_s16(src2 + 56);
		v3 = vmull_u8(v0, vf0);
		v4 = vqaddq_s16(v3, v2); 
		v5 = vrshrq_n_s16(v4 , 7);
		v1 = vqmovun_s16( v5 );	   
		vst1_u8(_dst + 56, v1);		
        
        _src += _srcstride;
        _dst += _dststride;
        src2 += MAX_PB_SIZE;
    }	
}


static void (*put_hevc_pel_bi_opt[10])(uint8_t *dst, ptrdiff_t dststride, uint8_t *_src, ptrdiff_t _srcstride,
								   int16_t *src2,
								   int height, intptr_t mx, intptr_t my, int width);

const uint8_t hevc_pel_weight[65] = { [2] = 0, [4] = 1, [6] = 2, [8] = 3, [12] = 4, [16] = 5, [24] = 6, [32] = 7, [48] = 8, [64] = 9 };


void init_put_hevc_pel_bi_pixels_opt_8()
{

    put_hevc_pel_bi_opt[0] = put_hevc_pel_bi_pixels_opt_8_w2;
    put_hevc_pel_bi_opt[1] = put_hevc_pel_bi_pixels_opt_8_w4;	
    put_hevc_pel_bi_opt[2] = put_hevc_pel_bi_pixels_opt_8_w6;	
    put_hevc_pel_bi_opt[3] = put_hevc_pel_bi_pixels_opt_8_w8;
    put_hevc_pel_bi_opt[4] = put_hevc_pel_bi_pixels_opt_8_w12;	
    put_hevc_pel_bi_opt[5] = put_hevc_pel_bi_pixels_opt_8_w16;	
    put_hevc_pel_bi_opt[6] = put_hevc_pel_bi_pixels_opt_8_w24;	
    put_hevc_pel_bi_opt[7] = put_hevc_pel_bi_pixels_opt_8_w32;	
    put_hevc_pel_bi_opt[8] = put_hevc_pel_bi_pixels_opt_8_w48;	
    put_hevc_pel_bi_opt[9] = put_hevc_pel_bi_pixels_opt_8_w64;		
}

void put_hevc_pel_bi_pixels_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2, int height, intptr_t mx, intptr_t my, int width)
{
    //CLK_DEF(o_tput_hevc_pel_bi_pixels);
    //CLK_T0(o_tput_hevc_pel_bi_pixels);   
	
    int idx = hevc_pel_weight[width];
	
    (*put_hevc_pel_bi_opt[idx])(_dst, _dststride, _src, _srcstride,
                              src2, height, mx, my, width);		
                              

#if 0	
    int x, y;
    int shift = 7;
	uint8x8_t v0, v1, vf0;
	int16x8_t v2, v3, v4, v5;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};
	uint8_t    work_idx , work_arr[8];	
    CLK_T0(o_tput_hevc_pel_bi_pixels); 	
	vf0 = vld1_u8(arr64);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x += 8, _dst += 8, _src += 8, src2 += 8)		
        {
             v0 = vld1_u8(_src);
			 v2 = vld1q_s16(src2);
			 v3 = vmull_u8(v0, vf0);
			 v4 = vqaddq_s16(v3, v2); 
		     v5 = vrshrq_n_s16(v4 , 7);
		     v1 = vqmovun_s16( v5 );	
		   
		     if ( (x + 8 ) <= width )	
		     {	
		       vst1_u8(_dst, v1);		
		     }	
		     else
		     {
		       vst1_u8(work_arr, v1);		
			   for (work_idx = 0; work_idx < (width & 7) ; work_idx++)	
			   {	 
			      _dst[work_idx] = work_arr[work_idx];		
			   }	
		     }				 			
        }
        _src  += _srcstride - x;
        _dst  += _dststride - x;
        src2 += MAX_PB_SIZE - x;
    }
#endif    
    //CLK_T1(o_tput_hevc_pel_bi_pixels);  	
}


void put_hevc_pel_pixels_opt_8(int16_t *dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
    //CLK_DEF(o_tput_hevc_pel_pixels);
    int x, y;
	uint8x8_t  v0,vf0;
	int16x8_t  v1;
	uint8_t arr64[8] = {64, 64, 64, 64, 64, 64, 64, 64};
	int16_t    work_idx , work_arr[8];
    //CLK_T0(o_tput_hevc_pel_pixels);	
	vf0 = vld1_u8(arr64);	

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x += 8, dst += 8, _src += 8)		
        {	
		     v0 = vld1_u8(_src);
			 v1 = vmull_u8(v0, vf0);
		     if ( (x + 8 ) <= width )	
		     {	
		       vst1q_s16(dst, v1);		
		     }	
		     else
		     {
		       vst1q_s16(work_arr, v1);		
			   for (work_idx = 0; work_idx < (width & 7) ; work_idx++)	
			   {	 
			      dst[work_idx] = work_arr[work_idx];		
			   }	
		     }				 			
        }
        _src  += _srcstride - x;
        dst  += MAX_PB_SIZE - x;	
    }
	//CLK_T1(o_tput_hevc_pel_pixels);  
}



// -1,  4,-10, 58, 17, -5,  1,
static void put_hevc_qpel_h_opt8_mv1(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
	uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86;
	int16x8_t  v160, v161, v166;
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
	uint8x8_t  v1;
	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;


	int x, y;
	uint8_t		*src	  = _src - 3;
	ptrdiff_t	srcstride = _srcstride;
	int16_t		*dst	  = _dst;

	v1 =  vld1_u8(f1);
	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);

	for (y = 0; y < height ; y++)
	{
	   for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
	   {
		 v80 = vld1_u8(src );
		 v81 = vld1_u8(src + 1);
		 v82 = vld1_u8(src + 2);
		 v83 = vld1_u8(src + 3);
		 v84 = vld1_u8(src + 4);
		 v85 = vld1_u8(src + 5);
		 v86 = vld1_u8(src + 6);

		 v161 = vshll_n_u8(v81,2);
		 v166 = vmovl_u8(v86);
         v161 = vmlsl_u8(v161, v80,v1);
		 v166 = vmlsl_u8(v166, v85,v5);
         v161 = vmlsl_u8(v161, v82, v10);
		 v166 = vmlal_u8(v166, v84, v17);
         v161 = vmlal_u8(v161, v83, v58);

		 v160 = vaddq_s16(v161 , v166);

		 vst1q_s16(dst, v160);

	   }

	   src += srcstride - x;
	   dst += MAX_PB_SIZE - x;

	}

}


static void put_hevc_qpel_h_opt8_mv1_ns(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
	uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86;
	int16x8_t  v160, v161, v166;
	int16_t    work_idx , work_arr[8];
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
	uint8x8_t  v1;
	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;

	int x, y;
	uint8_t		*src	  = _src - 3;
	ptrdiff_t	srcstride = _srcstride;
	int16_t		*dst	  = _dst;

	v1 =  vld1_u8(f1);
	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);


	for (y = 0; y < height ; y++)
	{
	   for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
	   {
		 v80 = vld1_u8(src );
		 v81 = vld1_u8(src + 1);
		 v82 = vld1_u8(src + 2);
		 v83 = vld1_u8(src + 3);
		 v84 = vld1_u8(src + 4);
		 v85 = vld1_u8(src + 5);
		 v86 = vld1_u8(src + 6);

		 v161 = vshll_n_u8(v81,2);
		 v166 = vmovl_u8(v86);
         v161 = vmlsl_u8(v161, v80,v1);
		 v166 = vmlsl_u8(v166, v85,v5);
         v161 = vmlsl_u8(v161, v82, v10);
		 v166 = vmlal_u8(v166, v84, v17);
         v161 = vmlal_u8(v161, v83, v58);

		 v160 = vaddq_s16(v161 , v166);

		if ( (x + 8 ) <= width )
		{
			vst1q_s16(dst, v160);
		}
		else
		{
			vst1q_s16(work_arr, v160);
			for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			{
			  dst[work_idx] = work_arr[work_idx];
			}
		}

	   }
	   src += srcstride - x;
	   dst += MAX_PB_SIZE - x;

	}
}


// -1,  4,-11, 40, 40,-11,  4, -1
static void put_hevc_qpel_h_opt8_mv2(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v160, v161, v166;
   uint8_t    f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t    f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
   uint8x8_t  v11;
   uint8x8_t  v40;
   uint8x8_t  v1;

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   int16_t     *dst      = _dst;

   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);
   v1 = vld1_u8(f1);

   for (y = 0; y < height ;y++)
   {

      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {
	    v80 = vld1_u8(src);
	    v81 = vld1_u8(src + 1);
	    v82 = vld1_u8(src + 2);
	    v83 = vld1_u8(src + 3);
	    v84 = vld1_u8(src + 4);
	    v85 = vld1_u8(src + 5);
	    v86 = vld1_u8(src + 6);
	    v87 = vld1_u8(src + 7);

	    v161 = vshll_n_u8 (v81,2);
		v166 = vshll_n_u8 (v86,2);
		v161 = vmlsl_u8(v161, v80, v1);
		v166 = vmlsl_u8(v166, v87, v1);
	    v161 = vmlsl_u8(v161, v82, v11);
		v166 = vmlsl_u8(v166, v85, v11);
	    v161 = vmlal_u8(v161, v83, v40);
		v166 = vmlal_u8(v166, v84, v40);

 		v160 = vaddq_s16(v161 , v166);

		vst1q_s16(dst, v160);

      }
	  src += srcstride - x;
      dst += MAX_PB_SIZE - x;

   }
}

static void put_hevc_qpel_h_opt8_mv2_ns(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v160, v161,v166;
   int16_t    work_idx , work_arr[8];
   uint8_t    f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t    f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
   uint8_t    f4[8]  = {4,4,4,4,4,4,4,4};
   uint8x8_t  v11;
   uint8x8_t  v40;
   uint8x8_t  v1;
   uint8x8_t  v4;
   v1 = vld1_u8(f1);

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   int16_t     *dst      = _dst;

   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);
   v4  = vld1_u8(f4);
   v1  = vld1_u8(f1);

   for (y = 0; y < height ;y++)
   {
      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {
  	    v80 = vld1_u8(src);
  	    v81 = vld1_u8(src + 1);
  	    v82 = vld1_u8(src + 2);
  	    v83 = vld1_u8(src + 3);
  	    v84 = vld1_u8(src + 4);
  	    v85 = vld1_u8(src + 5);
  	    v86 = vld1_u8(src + 6);
  	    v87 = vld1_u8(src + 7);

  	    v161 = vshll_n_u8 (v81,2);
  		v166 = vshll_n_u8 (v86,2);
  		v161 = vmlsl_u8(v161, v80, v1);
  		v166 = vmlsl_u8(v166, v87, v1);
  	    v161 = vmlsl_u8(v161, v82, v11);
  		v166 = vmlsl_u8(v166, v85, v11);
  	    v161 = vmlal_u8(v161, v83, v40);
  		v166 = vmlal_u8(v166, v84, v40);

   		v160 = vaddq_s16(v161 , v166);

  		vst1q_s16(dst, v160);

		if ( (x + 8 ) <= width )
		{
			vst1q_s16(dst, v160);
		}
		else
		{
			vst1q_s16(work_arr, v160);
			for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			{
			  dst[work_idx] = work_arr[work_idx];
			}
		}
      }
	  src += srcstride - x;
      dst += MAX_PB_SIZE - x;

   }
}

//0,  1, -5, 17, 58,-10,  4, -1,
static void put_hevc_qpel_h_opt8_mv3(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v161, v166;
   uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t    f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   int16_t     *dst      = _dst;

   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);


   for (y = 0; y < height ;y++)
   {
      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {

		v81 = vld1_u8(src + 1);
		v82 = vld1_u8(src + 2);
		v83 = vld1_u8(src + 3);
		v84 = vld1_u8(src + 4);
		v85 = vld1_u8(src + 5);
		v86 = vld1_u8(src + 6);
		v87 = vld1_u8(src + 7);

	    v161 = vmovl_u8(v81);
	    v166 = vshll_n_u8(v86 , 2);
	    v161 = vmlsl_u8(v161, v82,v5);
	    v166 = vmlsl_u8(v166, v87, v1);
	    v161 = vmlal_u8(v161, v83, v17);
	    v166 = vmlsl_u8(v166,v85,v10);
	    v161 = vmlal_u8(v161, v84, v58);

 		v161 = vaddq_s16(v161 , v166);

		vst1q_s16(dst, v161);
      }

	  src += srcstride - x;
      dst += MAX_PB_SIZE - x;

   }
}


//1, -5, 17, 58, -10,  4, -1,
static void put_hevc_qpel_h_opt8_mv3_ns(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v161, v166;
   int16_t    work_idx , work_arr[8];
   uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t    f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   int16_t     *dst      = _dst;

   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);


   for (y = 0; y < height ;y++)
   {
      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {

		v81 = vld1_u8(src + 1);
		v82 = vld1_u8(src + 2);
		v83 = vld1_u8(src + 3);
		v84 = vld1_u8(src + 4);
		v85 = vld1_u8(src + 5);
		v86 = vld1_u8(src + 6);
		v87 = vld1_u8(src + 7);

	    v161 = vmovl_u8(v81);
	    v166 = vshll_n_u8(v86 , 2);
	    v161 = vmlsl_u8(v161, v82,v5);
	    v166 = vmlsl_u8(v166, v87, v1);
	    v161 = vmlal_u8(v161, v83, v17);
	    v166 = vmlsl_u8(v166,v85,v10);
	    v161 = vmlal_u8(v161, v84, v58);

 		v161 = vaddq_s16(v161 , v166);

		if ( (x + 8 ) <= width )
		{
			vst1q_s16(dst, v161);
		}
		else
		{
			vst1q_s16(work_arr, v161);
			for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			{
			  dst[work_idx] = work_arr[work_idx];
			}
		}

      }

	  src += srcstride - x;
      dst += MAX_PB_SIZE - x;

   }
}

void put_hevc_qpel_h_opt_8(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
   //CLK_DEF(o_tput_hevc_qpel_h)
   //CLK_T0(o_tput_hevc_qpel_h)	

   if ((width & 7 ) == 0 )
   {
	if (mx == 1)
	{
	 put_hevc_qpel_h_opt8_mv1(_dst, _src, _srcstride,
									  height, width);
	}
	else if (mx == 2)
	{
	 put_hevc_qpel_h_opt8_mv2(_dst, _src, _srcstride,
									 height, width);
	}
	else
	{
	 put_hevc_qpel_h_opt8_mv3(_dst, _src, _srcstride,
									  height, width);
	}
   }
   else
   {

	if (mx == 1)
	{
	 put_hevc_qpel_h_opt8_mv1_ns(_dst, _src, _srcstride,
									  height, width);
	}
	else if (mx == 2)
	{
	 put_hevc_qpel_h_opt8_mv2_ns(_dst, _src, _srcstride,
									 height, width);
	}
	else
	{
	 put_hevc_qpel_h_opt8_mv3_ns(_dst, _src, _srcstride,
									  height, width);
	}
   }
   //CLK_T1(o_tput_hevc_qpel_h)	
}

static void put_hevc_qpel_v_opt8_mv1(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v8_int;
   int16x8_t  v161, v166;
   uint8_t	  f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t	  f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t	  f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t	  f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t	  f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;

   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);

   int        x, y;
   uint8_t     *src;
   int16_t     *dst;

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += MAX_PB_SIZE)
	  {
           src += _srcstride;

		   v161 = vshll_n_u8 (v81,2);
		   v166 = vmovl_u8 (v86);
		   v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v85,v5);
		   v161 = vmlsl_u8(v161,v82,v10);
		   v166 = vmlal_u8(v166,v84,v17);
		   v161 = vmlal_u8(v161,v83,v58);

 		   v166 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v82  = v83;
		   vst1q_s16(dst, v166);
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;

		   v86  = vld1_u8(src);
	  }
   }
}

static void put_hevc_qpel_v_opt8_mv1_ns(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86;
   int16x8_t  v161, v166;
   int16_t    work_idx , work_arr[8];
   uint8_t	  f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t	  f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t	  f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t	  f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t	  f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;

   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);


   int        x, y;
   uint8_t    *src;
   int16_t    *dst;

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += MAX_PB_SIZE)
	  {
           src += _srcstride;
		   v161 = vshll_n_u8 (v81,2);
		   v166 = vmovl_u8 (v86);
		   v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v85,v5);
		   v161 = vmlsl_u8(v161,v82,v10);
		   v166 = vmlal_u8(v166,v84,v17);
		   v161 = vmlal_u8(v161,v83,v58);

 		   v166 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v82  = v83;
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;

		   v86  = vld1_u8(src);

		   if ( (x + 8 ) <= width )
		   {
		      vst1q_s16(dst, v166);
		   }
		   else
		   {
		      vst1q_s16(work_arr, v166);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
			      dst[work_idx] = work_arr[work_idx];
			  }
		   }
	  }
   }
}

//-1,  4,-11, 40, 40,-11,  4, -1
static void put_hevc_qpel_v_opt8_mv2(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t   v80, v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t   v161,v166;
   uint8_t     f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t     f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t     f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t   v11;
   uint8x8_t   v40;
   uint8x8_t   v1;

   int         x, y;
   uint8_t     *src;
   int16_t     *dst;

   v1  = vld1_u8(f1);
   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += MAX_PB_SIZE)
	  {
           src += _srcstride;

	       v161 = vshll_n_u8 (v81,2);
		   v166 = vshll_n_u8 (v86,2);
	       v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v87,v1);
		   v161 = vmlsl_u8(v161,v82,v11);
	       v166 = vmlsl_u8(v166,v85,v11);
	       v161 = vmlal_u8(v161,v83,v40);
	       v166 = vmlal_u8(v166,v84,v40);

 		   v161 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v82  = v83;
		   vst1q_s16(dst, v161);
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;
 		   v86  = v87;

		   v87  = vld1_u8(src);
	  }
   }
}

static void put_hevc_qpel_v_opt8_mv2_ns(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
   uint8x8_t   v80, v81, v82, v83 ,v84, v85, v86, v87, v8_int;
   int16x8_t   v161, v166;
   int16_t     work_idx , work_arr[8];
   uint8_t     f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t     f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t     f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t   v11;
   uint8x8_t   v40;
   uint8x8_t   v1;

   int         x, y;
   uint8_t     *src;
   int16_t     *dst;

   v1  = vld1_u8(f1);
   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += MAX_PB_SIZE)
	  {
           src += _srcstride;

	       v161 = vshll_n_u8 (v81,2);
		   v166 = vshll_n_u8 (v86,2);
	       v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v87,v1);
		   v161 = vmlsl_u8(v161,v82,v11);
	       v166 = vmlsl_u8(v166,v85,v11);
	       v161 = vmlal_u8(v161,v83,v40);
	       v166 = vmlal_u8(v166,v84,v40);

 		   v161 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v82  = v83;
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;
 		   v86  = v87;
		   v87  = vld1_u8(src);

		   if ( (x + 8 ) <= width )
		   {
		      vst1q_s16(dst, v161);
		   }
		   else
		   {
		      vst1q_s16(work_arr, v161);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
			      dst[work_idx] = work_arr[work_idx];
			  }
		   }
	  }
   }
}


static void put_hevc_qpel_v_opt8_mv3(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
	uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87;
	int16x8_t  v161, v166;
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8] = {1,1,1,1,1,1,1,1};

	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;
	uint8x8_t  v1;

	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);
	v1  = vld1_u8(f1);

	int 	   x, y;
	uint8_t	   *src;
	int16_t	   *dst;


   _src = _src - 2*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);
	  for (y = 0; y < height ; y++, dst += MAX_PB_SIZE)
	  {
           src += _srcstride;

		   //0,  1, -5, 17, 58, -10,  4, -1,
	       v161 = vmovl_u8(v81);
		   v166 = vshll_n_u8 (v86,2);
		   v161 = vmlsl_u8(v161, v82, v5);
		   v166 = vmlsl_u8(v166, v87, v1);
	       v161 = vmlal_u8(v161, v83, v17);
		   v166 = vmlsl_u8(v166, v85, v10);
	       v161 = vmlal_u8(v161, v84, v58);

 		   v161 = vaddq_s16(v161 , v166);

 		   v81  = v82;
 		   v82  = v83;
 		   v83  = v84;
		   vst1q_s16(dst, v161);
 		   v84  = v85;
 		   v85  = v86;
 		   v86  = v87;
		   v87  = vld1_u8(src);

	  }
   }
}

//0,  1, -5, 17, 58,-10,  4, -1,
static void put_hevc_qpel_v_opt8_mv3_ns(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, int width)
{
	uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87;
	int16x8_t  v161, v166;
	int16_t    work_idx , work_arr[8];
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8] = {1,1,1,1,1,1,1,1};

	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;
	uint8x8_t  v1;

	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);
	v1  = vld1_u8(f1);

	int 	   x, y;
	uint8_t	   *src;
	int16_t	   *dst;


   _src = _src - 2*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);
	  for (y = 0; y < height ; y++, dst += MAX_PB_SIZE)
	  {
           src += _srcstride;
		   //0,  1, -5, 17, 58, -10,  4, -1,
	       v161 = vmovl_u8(v81);
		   v166 = vshll_n_u8 (v86,2);
		   v161 = vmlsl_u8(v161, v82, v5);
		   v166 = vmlsl_u8(v166, v87, v1);
	       v161 = vmlal_u8(v161, v83, v17);
		   v166 = vmlsl_u8(v166, v85, v10);
	       v161 = vmlal_u8(v161, v84, v58);

 		   v161 = vaddq_s16(v161 , v166);

 		   v81  = v82;
 		   v82  = v83;
 		   v83  = v84;
 		   v84  = v85;
 		   v85  = v86;
 		   v86  = v87;
		   v87  = vld1_u8(src);

		   if ( (x + 8 ) <= width )
		   {
		      vst1q_s16(dst, v161);
		   }
		   else
		   {
		      vst1q_s16(work_arr, v161);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
			      dst[work_idx] = work_arr[work_idx];
			  }
		   }
	  }
   }
}


void put_hevc_qpel_v_opt_8(int16_t *_dst,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
	//CLK_DEF(o_tput_hevc_qpel_v)
	//CLK_T0(o_tput_hevc_qpel_v)	

	if ((width & 7 ) == 0 )
	{
	 if (my == 1)
	 {
	  put_hevc_qpel_v_opt8_mv1(_dst, _src, _srcstride,
									   height, width);
	 }
	 else if (my == 2)
	 {
	  put_hevc_qpel_v_opt8_mv2(_dst, _src, _srcstride,
									  height, width);
	 }
	 else
	 {
	  put_hevc_qpel_v_opt8_mv3(_dst, _src, _srcstride,
									   height, width);
	 }
	}
	else
	{
	 if (my == 1)
	 {
	  put_hevc_qpel_v_opt8_mv1_ns(_dst, _src, _srcstride,
									   height, width);
	 }
	 else if (my == 2)
	 {
	  put_hevc_qpel_v_opt8_mv2_ns(_dst, _src, _srcstride,
									  height, width);
	 }
	 else
	 {
	  put_hevc_qpel_v_opt8_mv3_ns(_dst, _src, _srcstride,
									   height, width);
	 }
	}
	//CLK_T1(o_tput_hevc_qpel_v)	
}


void put_hevc_qpel_hv_opt_8(int16_t *dst,
                            uint8_t *_src,
                            ptrdiff_t _srcstride,
                            int height, intptr_t mx,
                            intptr_t my, int width)
{
    uint8_t tmp_array[(MAX_PB_SIZE + QPEL_EXTRA) * MAX_PB_SIZE];
	uint8_t * src    = _src - QPEL_EXTRA_BEFORE * _srcstride;
    uint8_t * tmp    = tmp_array + QPEL_EXTRA_BEFORE * MAX_PB_SIZE;	
	//CLK_DEF(o_tput_hevc_qpel_hv);
	//CLK_T0(o_tput_hevc_qpel_hv);
	put_hevc_qpel_uni_h_opt_8(tmp_array,  MAX_PB_SIZE,
                              src, _srcstride,
                              height + QPEL_EXTRA , mx, 0, width);
	put_hevc_qpel_v_opt_8(dst,
                              tmp, MAX_PB_SIZE,
                              height, 0, my, width);
	//CLK_T1(o_tput_hevc_qpel_hv);		
}

static void put_hevc_qpel_bi_h_opt8_mv1(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86;
	int16x8_t  v160, v161, v162, v163, v164, v166;
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
	uint8x8_t  v1;
	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;


	int x, y;
	uint8_t		*src	  = _src - 3;
	ptrdiff_t	srcstride = _srcstride;
	uint8_t		*dst	  = _dst;

	v1 =  vld1_u8(f1);
	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);

	for (y = 0; y < height ; y++)
	{
	   for (x = 0 ; x < width ;x += 8, src += 8, dst += 8 )
	   {
		 v80 = vld1_u8(src );
		 v81 = vld1_u8(src + 1);
		 v82 = vld1_u8(src + 2);
		 v83 = vld1_u8(src + 3);
		 v84 = vld1_u8(src + 4);
		 v85 = vld1_u8(src + 5);
		 v86 = vld1_u8(src + 6);

		 v161 = vshll_n_u8(v81,2);
		 v166 = vmovl_u8(v86);
         v161 = vmlsl_u8(v161, v80,v1);
		 v166 = vmlsl_u8(v166, v85,v5);
         v161 = vmlsl_u8(v161, v82, v10);
		 v166 = vmlal_u8(v166, v84, v17);
         v161 = vmlal_u8(v161, v83, v58);
		 v160 = vaddq_s16(v161 , v166);

		 v162 = vld1q_s16(src2 + x);
		 v163 = vqaddq_s16(v160, v162);
		 v164 = vrshrq_n_s16(v163 , 7);
		 v86 = vqmovun_s16( v164 );
		 vst1_u8(dst, v86);

	   }

	   src += srcstride - x;
	   dst += _dststride - x;
	   src2 += MAX_PB_SIZE;

	}

}


static void put_hevc_qpel_bi_h_opt8_mv1_ns(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86;
	int16x8_t  v160, v161, v162, v163, v164, v165, v166;
	uint8_t    work_idx , work_arr[8];
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
	uint8x8_t  v1;
	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;

	int x, y;
	uint8_t		*src	  = _src - 3;
	ptrdiff_t	srcstride = _srcstride;
	uint8_t		*dst	  = _dst;

	v1 =  vld1_u8(f1);
	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);


	for (y = 0; y < height ; y++)
	{
	   for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
	   {
		 v80 = vld1_u8(src );
		 v81 = vld1_u8(src + 1);
		 v82 = vld1_u8(src + 2);
		 v83 = vld1_u8(src + 3);
		 v84 = vld1_u8(src + 4);
		 v85 = vld1_u8(src + 5);
		 v86 = vld1_u8(src + 6);

		 v161 = vshll_n_u8(v81,2);
		 v166 = vmovl_u8(v86);
         v161 = vmlsl_u8(v161, v80,v1);
		 v166 = vmlsl_u8(v166, v85,v5);
         v161 = vmlsl_u8(v161, v82, v10);
		 v166 = vmlal_u8(v166, v84, v17);
         v161 = vmlal_u8(v161, v83, v58);
		 v160 = vaddq_s16(v161 , v166);

		 v162 = vld1q_s16(src2 + x);
		 v163 = vqaddq_s16(v160, v162);
		 v164 = vrshrq_n_s16(v163 , 7);
		 v86 = vqmovun_s16( v164 );

		 if ( (x + 8 ) <= width )
		 {
			vst1_u8(dst, v86);
		 }
		 else
		 {
			vst1_u8(work_arr, v86);
			for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			{
			  dst[work_idx] = work_arr[work_idx];
			}
		 }
	   }
	   src += srcstride - x;
	   dst += _dststride - x;
	   src2 += MAX_PB_SIZE;
	}
}


// -1,  4,-11, 40, 40,-11,  4, -1
static void put_hevc_qpel_bi_h_opt8_mv2(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v160, v161, v162, v163, v164, v165, v166, v167;
   uint8_t    f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t    f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
   uint8x8_t  v11;
   uint8x8_t  v40;
   uint8x8_t  v1;

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   uint8_t     *dst      = _dst;

   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);
   v1 = vld1_u8(f1);

   for (y = 0; y < height ;y++)
   {

      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {
	    v80 = vld1_u8(src);
	    v81 = vld1_u8(src + 1);
	    v82 = vld1_u8(src + 2);
	    v83 = vld1_u8(src + 3);
	    v84 = vld1_u8(src + 4);
	    v85 = vld1_u8(src + 5);
	    v86 = vld1_u8(src + 6);
	    v87 = vld1_u8(src + 7);

	    v161 = vshll_n_u8 (v81,2);
		v166 = vshll_n_u8 (v86,2);
		v161 = vmlsl_u8(v161, v80, v1);
		v166 = vmlsl_u8(v166, v87, v1);
	    v161 = vmlsl_u8(v161, v82, v11);
		v166 = vmlsl_u8(v166, v85, v11);
	    v161 = vmlal_u8(v161, v83, v40);
		v166 = vmlal_u8(v166, v84, v40);

 		v160 = vaddq_s16(v161 , v166);

		v162 = vld1q_s16(src2 + x);
		v163 = vqaddq_s16(v160, v162);
		v164 = vrshrq_n_s16(v163 , 7);
		v86 = vqmovun_s16( v164 );
		vst1_u8(dst, v86);

      }
	  src += srcstride - x;
      dst += _dststride - x;
	  src2 += MAX_PB_SIZE;
   }
}



static void put_hevc_qpel_bi_h_opt8_mv2_ns(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v160, v161, v162, v163, v164, v165, v166, v167;
   uint8_t    work_idx , work_arr[8];
   uint8_t    f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t    f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t    f1[8]  = {1,1,1,1,1,1,1,1};
   uint8_t    f4[8]  = {4,4,4,4,4,4,4,4};
   uint8x8_t  v11;
   uint8x8_t  v40;
   uint8x8_t  v1;
   uint8x8_t  v4;
   v1 = vld1_u8(f1);

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   uint8_t     *dst      = _dst;

   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);
   v4  = vld1_u8(f4);
   v1  = vld1_u8(f1);

   for (y = 0; y < height ;y++)
   {
      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {
  	    v80 = vld1_u8(src);
  	    v81 = vld1_u8(src + 1);
  	    v82 = vld1_u8(src + 2);
  	    v83 = vld1_u8(src + 3);
  	    v84 = vld1_u8(src + 4);
  	    v85 = vld1_u8(src + 5);
  	    v86 = vld1_u8(src + 6);
  	    v87 = vld1_u8(src + 7);

  	    v161 = vshll_n_u8 (v81,2);
  		v166 = vshll_n_u8 (v86,2);
  		v161 = vmlsl_u8(v161, v80, v1);
  		v166 = vmlsl_u8(v166, v87, v1);
  	    v161 = vmlsl_u8(v161, v82, v11);
  		v166 = vmlsl_u8(v166, v85, v11);
  	    v161 = vmlal_u8(v161, v83, v40);
  		v166 = vmlal_u8(v166, v84, v40);

   		v160 = vaddq_s16(v161 , v166);

		v162 = vld1q_s16(src2 + x);
		v163 = vqaddq_s16(v160, v162);
		v164 = vrshrq_n_s16(v163 , 7);
		v86 = vqmovun_s16( v164 );

		if ( (x + 8 ) <= width )
		{
			vst1_u8(dst, v86);
		}
		else
		{
			vst1_u8(work_arr, v86);
			for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			{
			  dst[work_idx] = work_arr[work_idx];
			}
		}
      }
	  src += srcstride - x;
      dst += _dststride - x;
	  src2 += MAX_PB_SIZE;
   }
}

//0,  1, -5, 17, 58,-10,  4, -1,
static void put_hevc_qpel_bi_h_opt8_mv3(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
   uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v161, v162, v163, v164, v165, v166, v167;
   uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t    f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   uint8_t     *dst      = _dst;

   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);


   for (y = 0; y < height ;y++)
   {
      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {

		v81 = vld1_u8(src + 1);
		v82 = vld1_u8(src + 2);
		v83 = vld1_u8(src + 3);
		v84 = vld1_u8(src + 4);
		v85 = vld1_u8(src + 5);
		v86 = vld1_u8(src + 6);
		v87 = vld1_u8(src + 7);

	    v161 = vmovl_u8(v81);
	    v166 = vshll_n_u8(v86 , 2);
	    v161 = vmlsl_u8(v161, v82,v5);
	    v166 = vmlsl_u8(v166, v87, v1);
	    v161 = vmlal_u8(v161, v83, v17);
	    v166 = vmlsl_u8(v166,v85,v10);
	    v161 = vmlal_u8(v161, v84, v58);

 		v161 = vaddq_s16(v161 , v166);

		v162 = vld1q_s16(src2 + x);
		v163 = vqaddq_s16(v161, v162);
		v164 = vrshrq_n_s16(v163 , 7);
		v86 = vqmovun_s16( v164 );
		vst1_u8(dst, v86);
      }

	  src += srcstride - x;
      dst += _dststride - x;
	  src2 += MAX_PB_SIZE;
   }
}


//1, -5, 17, 58, -10,  4, -1,
static void put_hevc_qpel_bi_h_opt8_mv3_ns(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
   uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87;
   int16x8_t  v161, v162, v163, v164, v165, v166, v167;
   uint8_t    work_idx , work_arr[8];
   uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t    f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;

   int x, y;
   uint8_t     *src      = _src - 3;
   ptrdiff_t   srcstride = _srcstride;
   uint8_t     *dst      = _dst;

   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);


   for (y = 0; y < height ;y++)
   {
      for (x = 0 ; x < width ;x += 8, src += 8, dst += 8)
      {

		v81 = vld1_u8(src + 1);
		v82 = vld1_u8(src + 2);
		v83 = vld1_u8(src + 3);
		v84 = vld1_u8(src + 4);
		v85 = vld1_u8(src + 5);
		v86 = vld1_u8(src + 6);
		v87 = vld1_u8(src + 7);

	    v161 = vmovl_u8(v81);
	    v166 = vshll_n_u8(v86 , 2);
	    v161 = vmlsl_u8(v161, v82,v5);
	    v166 = vmlsl_u8(v166, v87, v1);
	    v161 = vmlal_u8(v161, v83, v17);
	    v166 = vmlsl_u8(v166,v85,v10);
	    v161 = vmlal_u8(v161, v84, v58);

 		v161 = vaddq_s16(v161 , v166);

		v162 = vld1q_s16(src2 + x);
		v163 = vqaddq_s16(v161, v162);
		v164 = vrshrq_n_s16(v163 , 7);
		v86 = vqmovun_s16( v164 );

		if ( (x + 8 ) <= width )
		{
			vst1_u8(dst, v86);
		}
		else
		{
			vst1_u8(work_arr, v86);
			for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			{
			  dst[work_idx] = work_arr[work_idx];
			}
		}
      }

	  src += srcstride - x;
      dst += _dststride - x;
	  src2 += MAX_PB_SIZE;
   }
}

void put_hevc_qpel_bi_h_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
    //CLK_DEF(o_tput_hevc_qpel_bi_h);
    //CLK_T0(o_tput_hevc_qpel_bi_h);	

	if ( (width & 7) == 0 )
	{
	   if (mx == 1)
	   {
		   put_hevc_qpel_bi_h_opt8_mv1(_dst, _dststride, _src, _srcstride,
		                             src2,height, mx, my, width);
	   }
	   else if (mx == 2)
	   {
		   put_hevc_qpel_bi_h_opt8_mv2(_dst, _dststride, _src, _srcstride,
		                             src2,height, mx, my, width);
	   }
	   else
	   {
		   put_hevc_qpel_bi_h_opt8_mv3(_dst, _dststride, _src, _srcstride,
		                             src2,height, mx, my, width);
	   }
	}
	else
	{
		if (mx == 1)
		{
			put_hevc_qpel_bi_h_opt8_mv1_ns(_dst, _dststride, _src, _srcstride,
			                             src2,height, mx, my, width);
		}
		else if (mx == 2)
		{
			put_hevc_qpel_bi_h_opt8_mv2_ns(_dst, _dststride, _src, _srcstride,
			                             src2,height, mx, my, width);
		}
		else
		{
			put_hevc_qpel_bi_h_opt8_mv3_ns(_dst, _dststride, _src, _srcstride,
			                             src2,height, mx, my, width);
		}
	}
	//CLK_T1(o_tput_hevc_qpel_bi_h);	
}


static void put_hevc_qpel_bi_v_opt8_mv1(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v8_int;
   int16x8_t  v160, v161, v162, v163, v164, v165, v166;
   uint8_t	  f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t	  f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t	  f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t	  f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t	  f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;

   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);

   int        x, y;
   uint8_t     *src;
   uint8_t     *dst;
   int16_t     *_src2 = src2;

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  src2 = _src2 + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;

		   v161 = vshll_n_u8 (v81,2);
		   v166 = vmovl_u8 (v86);
		   v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v85,v5);
		   v161 = vmlsl_u8(v161,v82,v10);
		   v166 = vmlal_u8(v166,v84,v17);
		   v161 = vmlal_u8(v161,v83,v58);

 		   v166 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v162 = vld1q_s16(src2);
 		   v82  = v83;
 		   v163 = vqaddq_s16(v166, v162);
 		   v83  = v84;
 		   v164 = vrshrq_n_s16(v163 , 7);
 		   v84  = v85;
 		   v8_int = vqmovun_s16( v164 );
 		   v85  = v86;
		   vst1_u8(dst, v8_int);

		   v86  = vld1_u8(src);
           src2 += MAX_PB_SIZE;
	  }
   }
}

static void put_hevc_qpel_bi_v_opt8_mv1_ns(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
   uint8x8_t  v80, v81, v82, v83 ,v84, v85, v86, v8_int;
   int16x8_t  v160, v161, v162, v163, v164, v165, v166;
   uint8_t    work_idx , work_arr[8];
   uint8_t	  f5[8]  = {5,5,5,5,5,5,5,5};
   uint8_t	  f17[8] = {17,17,17,17,17,17,17,17};
   uint8_t	  f58[8] = {58,58,58,58,58,58,58,58};
   uint8_t	  f10[8] = {10,10,10,10,10,10,10,10};
   uint8_t	  f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t  v1;
   uint8x8_t  v5;
   uint8x8_t  v17;
   uint8x8_t  v58;
   uint8x8_t  v10;

   v1 =  vld1_u8(f1);
   v5 =  vld1_u8(f5);
   v17 = vld1_u8(f17);
   v58 = vld1_u8(f58);
   v10 = vld1_u8(f10);


   int        x, y;
   uint8_t    *src;
   uint8_t    *dst;
   int16_t     *_src2 = src2;

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  src2 = _src2 + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;
		   v161 = vshll_n_u8 (v81,2);
		   v166 = vmovl_u8 (v86);
		   v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v85,v5);
		   v161 = vmlsl_u8(v161,v82,v10);
		   v166 = vmlal_u8(v166,v84,v17);
		   v161 = vmlal_u8(v161,v83,v58);

 		   v166 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v162 = vld1q_s16(src2);
 		   v82  = v83;
 		   v163 = vqaddq_s16(v166, v162);
 		   v83  = v84;
 		   v164 = vrshrq_n_s16(v163 , 7);
 		   v84  = v85;
 		   v8_int = vqmovun_s16( v164 );
 		   v85  = v86;

		   v86  = vld1_u8(src);

		   if ( (x + 8 ) <= width )
		   {
			  vst1_u8(dst, v8_int);
		   }
		   else
		   {
			  vst1_u8(work_arr, v8_int);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
			      dst[work_idx] = work_arr[work_idx];
			  }
		   }

		   v86  = vld1_u8(src);
           src2 += MAX_PB_SIZE;
	  }
   }
}


//-1,  4,-11, 40, 40,-11,  4, -1
static void put_hevc_qpel_bi_v_opt8_mv2(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
   uint8x8_t   v80, v81, v82, v83 ,v84, v85, v86, v87, v8_int;
   int16x8_t   v160, v161, v162, v163, v164, v165, v166, v167;
   uint8_t     f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t     f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t     f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t   v11;
   uint8x8_t   v40;
   uint8x8_t   v1;

   int         x, y;
   uint8_t     *src;
   uint8_t     *dst;
   int16_t     *_src2 = src2;

   v1  = vld1_u8(f1);
   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  src2 = _src2 + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;

	       v161 = vshll_n_u8 (v81,2);
		   v166 = vshll_n_u8 (v86,2);
	       v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v87,v1);
		   v161 = vmlsl_u8(v161,v82,v11);
	       v166 = vmlsl_u8(v166,v85,v11);
	       v161 = vmlal_u8(v161,v83,v40);
	       v166 = vmlal_u8(v166,v84,v40);

 		   v161 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v162 = vld1q_s16(src2);
 		   v82  = v83;
 		   v163 = vqaddq_s16(v161, v162);
 		   v83  = v84;
 		   v164 = vrshrq_n_s16(v163 , 7);
 		   v84  = v85;
 		   v8_int = vqmovun_s16( v164 );
 		   v85  = v86;
		   vst1_u8(dst, v8_int);
 		   v86  = v87;

		   v87  = vld1_u8(src);
           src2 += MAX_PB_SIZE;
	  }
   }
}

static void put_hevc_qpel_bi_v_opt8_mv2_ns(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)

{
   uint8x8_t   v80, v81, v82, v83 ,v84, v85, v86, v87, v8_int;
   int16x8_t   v160, v161, v162, v163, v164, v165, v166, v167;
   uint8_t     work_idx , work_arr[8];
   uint8_t     f11[8] = {11,11,11,11,11,11,11,11};
   uint8_t     f40[8] = {40,40,40,40,40,40,40,40};
   uint8_t     f1[8] = {1,1,1,1,1,1,1,1};
   uint8x8_t   v11;
   uint8x8_t   v40;
   uint8x8_t   v1;

   int         x, y;
   uint8_t     *src;
   uint8_t     *dst;
   int16_t     *_src2 = src2;

   v1  = vld1_u8(f1);
   v11 = vld1_u8(f11);
   v40 = vld1_u8(f40);

   _src = _src - 3*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  src2 = _src2 + x;
	  v80 = vld1_u8(src);
	  src += _srcstride;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);

	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;

	       v161 = vshll_n_u8 (v81,2);
		   v166 = vshll_n_u8 (v86,2);
	       v161 = vmlsl_u8(v161,v80,v1);
		   v166 = vmlsl_u8(v166,v87,v1);
		   v161 = vmlsl_u8(v161,v82,v11);
	       v166 = vmlsl_u8(v166,v85,v11);
	       v161 = vmlal_u8(v161,v83,v40);
	       v166 = vmlal_u8(v166,v84,v40);

 		   v161 = vaddq_s16(v166 , v161);

 		   v80  = v81;
 		   v81  = v82;
 		   v162 = vld1q_s16(src2);
 		   v82  = v83;
 		   v163 = vqaddq_s16(v161, v162);
 		   v83  = v84;
 		   v164 = vrshrq_n_s16(v163 , 7);
 		   v84  = v85;
 		   v8_int = vqmovun_s16( v164 );
 		   v85  = v86;
 		   v86  = v87;

		   if ( (x + 8 ) <= width )
		   {
		      vst1_u8(dst, v8_int);
		   }
		   else
		   {
		      vst1_u8(work_arr, v8_int);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
			      dst[work_idx] = work_arr[work_idx];
			  }
		   }

		   v87  = vld1_u8(src);
           src2 += MAX_PB_SIZE;
	  }
   }
}


static void put_hevc_qpel_bi_v_opt8_mv3(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87, v8_int;
	int16x8_t  v161, v162, v163, v164, v165, v166, v167;
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8] = {1,1,1,1,1,1,1,1};

	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;
	uint8x8_t  v1;

	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);
	v1  = vld1_u8(f1);

	int 	   x, y;
	uint8_t	   *src;
	uint8_t	   *dst;
	int16_t     *_src2 = src2;

   _src = _src - 2*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  src2 = _src2 + x;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);
	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;

		   //0,  1, -5, 17, 58, -10,  4, -1,
	       v161 = vmovl_u8(v81);
		   v166 = vshll_n_u8 (v86,2);
		   v161 = vmlsl_u8(v161, v82, v5);
		   v166 = vmlsl_u8(v166, v87, v1);
	       v161 = vmlal_u8(v161, v83, v17);
		   v166 = vmlsl_u8(v166, v85, v10);
	       v161 = vmlal_u8(v161, v84, v58);

 		   v161 = vaddq_s16(v161 , v166);

 		   v81  = v82;
 		   v82  = v83;
 		   v162 = vld1q_s16(src2);
 		   v83  = v84;
 		   v163 = vqaddq_s16(v161, v162);
 		   v84  = v85;
 		   v164 = vrshrq_n_s16(v163 , 7);
 		   v85  = v86;
 		   v8_int = vqmovun_s16( v164 );
 		   v86  = v87;
		   vst1_u8(dst, v8_int);

		   v87  = vld1_u8(src);
           src2 += MAX_PB_SIZE;

	  }
   }
}

//0,  1, -5, 17, 58,-10,  4, -1,
static void put_hevc_qpel_bi_v_opt8_mv3_ns(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)

{
	uint8x8_t  v81, v82, v83 ,v84, v85, v86, v87, v8_int;
	int16x8_t  v161, v162, v163, v164, v165, v166, v167;
	uint8_t    work_idx , work_arr[8];
	uint8_t    f5[8]  = {5,5,5,5,5,5,5,5};
	uint8_t    f17[8] = {17,17,17,17,17,17,17,17};
	uint8_t    f58[8] = {58,58,58,58,58,58,58,58};
	uint8_t    f10[8] = {10,10,10,10,10,10,10,10};
	uint8_t    f1[8] = {1,1,1,1,1,1,1,1};

	uint8x8_t  v5;
	uint8x8_t  v17;
	uint8x8_t  v58;
	uint8x8_t  v10;
	uint8x8_t  v1;

	v5 =  vld1_u8(f5);
	v17 = vld1_u8(f17);
	v58 = vld1_u8(f58);
	v10 = vld1_u8(f10);
	v1  = vld1_u8(f1);

	int 	   x, y;
	uint8_t	   *src;
	uint8_t	   *dst;
	int16_t     *_src2 = src2;

   _src = _src - 2*_srcstride;
   for (x = 0 ; x < width; x += 8)
   {
      src = _src + x;
	  dst = _dst + x;
	  src2 = _src2 + x;
	  v81 = vld1_u8(src);
	  src += _srcstride;
	  v82 = vld1_u8(src);
	  src += _srcstride;
	  v83 = vld1_u8(src);
	  src += _srcstride;
	  v84 = vld1_u8(src);
	  src += _srcstride;
	  v85 = vld1_u8(src);
	  src += _srcstride;
	  v86 = vld1_u8(src);
	  src += _srcstride;
	  v87 = vld1_u8(src);
	  for (y = 0; y < height ; y++, dst += _dststride)
	  {
           src += _srcstride;

		   //0,  1, -5, 17, 58, -10,  4, -1,
	       v161 = vmovl_u8(v81);
		   v166 = vshll_n_u8 (v86,2);
		   v161 = vmlsl_u8(v161, v82, v5);
		   v166 = vmlsl_u8(v166, v87, v1);
	       v161 = vmlal_u8(v161, v83, v17);
		   v166 = vmlsl_u8(v166, v85, v10);
	       v161 = vmlal_u8(v161, v84, v58);

 		   v161 = vaddq_s16(v161 , v166);

 		   v81  = v82;
 		   v82  = v83;
 		   v162 = vld1q_s16(src2);
 		   v83  = v84;
 		   v163 = vqaddq_s16(v161, v162);
 		   v84  = v85;
 		   v164 = vrshrq_n_s16(v163 , 7);
 		   v85  = v86;
 		   v8_int = vqmovun_s16( v164 );
 		   v86  = v87;

		   if ( (x + 8 ) <= width )
		   {
			  vst1_u8(dst, v8_int);
		   }
		   else
		   {
			  vst1_u8(work_arr, v8_int);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
			      dst[work_idx] = work_arr[work_idx];
			  }
		   }
		   v87  = vld1_u8(src);
           src2 += MAX_PB_SIZE;
	  }
   }
}

void put_hevc_qpel_bi_v_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                     int16_t *src2,
                                     int height, intptr_t mx, intptr_t my, int width)
{
    //CLK_DEF(o_tput_hevc_qpel_bi_v);
    //CLK_T0(o_tput_hevc_qpel_bi_v);	

	if ( (width & 7) == 0 )
	{
	   if (my == 1)
	   {
		   put_hevc_qpel_bi_v_opt8_mv1(_dst, _dststride, _src, _srcstride,
		                             src2,height, mx, my, width);
	   }
	   else if (my == 2)
	   {
		   put_hevc_qpel_bi_v_opt8_mv2(_dst, _dststride, _src, _srcstride,
		                             src2,height, mx, my, width);
	   }
	   else
	   {
		   put_hevc_qpel_bi_v_opt8_mv3(_dst, _dststride, _src, _srcstride,
		                             src2,height, mx, my, width);
	   }
	}
	else
	{
		if (my == 1)
		{
			put_hevc_qpel_bi_v_opt8_mv1_ns(_dst, _dststride, _src, _srcstride,
			                             src2,height, mx, my, width);
		}
		else if (my == 2)
		{
			put_hevc_qpel_bi_v_opt8_mv2_ns(_dst, _dststride, _src, _srcstride,
			                             src2,height, mx, my, width);
		}
		else
		{
			put_hevc_qpel_bi_v_opt8_mv3_ns(_dst, _dststride, _src, _srcstride,
			                             src2,height, mx, my, width);
		}
	}
    //CLK_T1(o_tput_hevc_qpel_bi_v);		
}


void put_hevc_qpel_bi_hv_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int16_t *src2,
                                      int height, intptr_t mx, intptr_t my, int width)
{

    uint8_t tmp_array[(MAX_PB_SIZE + QPEL_EXTRA) * MAX_PB_SIZE];
	uint8_t * src    = _src - QPEL_EXTRA_BEFORE * _srcstride;
    uint8_t * tmp    = tmp_array + QPEL_EXTRA_BEFORE * MAX_PB_SIZE;	
	//CLK_DEF(o_tput_hevc_qpel_bi_hv);
	//CLK_T0(o_tput_hevc_qpel_bi_hv);
	put_hevc_qpel_uni_h_opt_8(tmp_array,  MAX_PB_SIZE,
                              src, _srcstride,
                              height + QPEL_EXTRA , mx, 0, width);
	
	put_hevc_qpel_bi_v_opt_8(_dst,  _dststride,
                              tmp, MAX_PB_SIZE,
                              src2, height, 0, my, width);
    //CLK_T1(o_tput_hevc_qpel_bi_hv);
}


static void put_hevc_epel_h_opt_8_s(int16_t *_dst, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83;
	int16x8_t  v160, v161, v162, v163;

    int x, y;
    uint8_t *src          = _src - 1;
    ptrdiff_t srcstride   = _srcstride;
    int16_t *dst          = _dst;

    uint8x8_t  vf0;
    uint8x8_t  vf1;
    uint8x8_t  vf2;
    uint8x8_t  vf3;
    int shift = 14 - BIT_DEPTH;

	switch( mx )
	{
	  case   1:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(58);
	    vf2 = vdup_n_u8(10);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  2:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(54);
	    vf2 = vdup_n_u8(16);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  3:
	    vf0 = vdup_n_u8(6);
	    vf1 = vdup_n_u8(46);
	    vf2 = vdup_n_u8(28);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  4:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(36);
	    vf2 = vdup_n_u8(36);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  5:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(28);
	    vf2 = vdup_n_u8(46);
	    vf3 = vdup_n_u8(6);
	  	break;
	  case  6:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(16);
	    vf2 = vdup_n_u8(54);
	    vf3 = vdup_n_u8(4);
	  	break;
	  default :
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(10);
	    vf2 = vdup_n_u8(58);
	    vf3 = vdup_n_u8(2);
	}


	for (y = 0; y < height ;y++)
	{
	   for (x = 0 ; x < width ; dst += 8)
	   {
		v80 = vld1_u8(src);
		v81 = vld1_u8(src + 1);
		v82 = vld1_u8(src + 2);
		v83 = vld1_u8(src + 3);

		v161 = vmull_u8(v81,vf1);
		v162 = vmull_u8(v82,vf2);
		v161 = vmlsl_u8(v161,v80,vf0);
		v162 = vmlsl_u8(v162,v83,vf3);

		v161 = vaddq_s16(v161 , v162);

		src += 8;
		x += 8;
		vst1q_s16(dst, v161);

	   }
	   src += srcstride - x;
	   dst += MAX_PB_SIZE - x;

	}
}

static void put_hevc_epel_h_opt_8_ns(int16_t *_dst, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83;
	int16x8_t  v160, v161, v162, v163;
	int16_t    work_idx , work_arr[8];

    int x, y;
    uint8_t *src          = _src - 1;
    ptrdiff_t srcstride   = _srcstride;
    int16_t *dst          = _dst;


    uint8x8_t  vf0;
    uint8x8_t  vf1;
    uint8x8_t  vf2;
    uint8x8_t  vf3;
    int shift = 14 - BIT_DEPTH;

	switch( mx )
	{
	  case   1:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(58);
	    vf2 = vdup_n_u8(10);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  2:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(54);
	    vf2 = vdup_n_u8(16);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  3:
	    vf0 = vdup_n_u8(6);
	    vf1 = vdup_n_u8(46);
	    vf2 = vdup_n_u8(28);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  4:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(36);
	    vf2 = vdup_n_u8(36);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  5:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(28);
	    vf2 = vdup_n_u8(46);
	    vf3 = vdup_n_u8(6);
	  	break;
	  case  6:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(16);
	    vf2 = vdup_n_u8(54);
	    vf3 = vdup_n_u8(4);
	  	break;
	  default :
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(10);
	    vf2 = vdup_n_u8(58);
	    vf3 = vdup_n_u8(2);
	}

	for (y = 0; y < height ;y++)
	{
	   for (x = 0 ; x < width ;x += 8, dst += 8)
	   {
		 v80 = vld1_u8(src);
		 v81 = vld1_u8(src + 1);
		 v82 = vld1_u8(src + 2);
		 v83 = vld1_u8(src + 3);

		 v161 = vmull_u8(v81,vf1);
		 v162 = vmull_u8(v82,vf2);
		 v161 = vmlsl_u8(v161,v80,vf0);
		 v162 = vmlsl_u8(v162,v83,vf3);

		 v161 = vaddq_s16(v161 , v162);
		 src += 8;
		 if ( (x + 8 ) <= width )
		 {
			  vst1q_s16(dst, v161);
		 }
		 else
		 {
			 vst1q_s16(work_arr, v161);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
				dst[work_idx] = work_arr[work_idx];
			  }
		 }
	   }
	   src += srcstride - x;
	   dst += MAX_PB_SIZE - x;
	}

}

void put_hevc_epel_h_opt_8(int16_t *_dst, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
#ifndef ASM_OPT
   //CLK_DEF(o_put_hevc_epel_h)
   //CLK_T0(o_put_hevc_epel_h)   	
   if ( (width & 7 ) == 0 )
        put_hevc_epel_h_opt_8_s(_dst,  _src, _srcstride,
                                  height, mx, my, width);
   else
        put_hevc_epel_h_opt_8_ns(_dst,  _src, _srcstride,
                                  height, mx, my, width);
   //CLK_T1(o_put_hevc_epel_h) 
#else
        neon_put_hevc_epel_h_opt_8(_dst, MAX_PB_SIZE*2, _src, _srcstride,
                                      height, mx, my, width);

#endif
}


static void put_hevc_epel_v_opt_8_s(int16_t *_dst, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83, v8_int;
	int16x8_t  v160, v161, v162, v163;

    int x, y;
    uint8_t *src          ;
    ptrdiff_t srcstride   = _srcstride;
    int16_t *dst          = _dst;

    uint8x8_t  vf0;
    uint8x8_t  vf1;
    uint8x8_t  vf2;
    uint8x8_t  vf3;
    int shift = 14 - BIT_DEPTH;

	switch( my )
	{
	  case   1:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(58);
	    vf2 = vdup_n_u8(10);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  2:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(54);
	    vf2 = vdup_n_u8(16);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  3:
	    vf0 = vdup_n_u8(6);
	    vf1 = vdup_n_u8(46);
	    vf2 = vdup_n_u8(28);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  4:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(36);
	    vf2 = vdup_n_u8(36);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  5:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(28);
	    vf2 = vdup_n_u8(46);
	    vf3 = vdup_n_u8(6);
	  	break;
	  case  6:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(16);
	    vf2 = vdup_n_u8(54);
	    vf3 = vdup_n_u8(4);
	  	break;
	  default :
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(10);
	    vf2 = vdup_n_u8(58);
	    vf3 = vdup_n_u8(2);
	}


    _src = _src - _srcstride;
	for (x = 0 ; x < width; x += 8)
	{
	   src = _src + x;
	   dst = _dst + x;
	   v80 = vld1_u8(src);
	   src += _srcstride;
	   v81 = vld1_u8(src);
	   src += _srcstride;
	   v82 = vld1_u8(src);
	   src += _srcstride;
	   v83 = vld1_u8(src);

	   for (y = 0; y < height ; y++, dst += MAX_PB_SIZE)
	   {
			src += _srcstride;
            v161 = vmull_u8(v81,vf1);
            v162 = vmull_u8(v82,vf2);
            v161 = vmlsl_u8(v161,v80,vf0);
            v162 = vmlsl_u8(v162,v83,vf3);
            v80  = v81;
			v161 = vaddq_s16(v161 , v162);
            v81  = v82;
            v82  = v83;
            v83  = vld1_u8(src);
			vst1q_s16(dst, v161);
	   }
	}

}

static void put_hevc_epel_v_opt_8_ns(int16_t *_dst, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
		uint8x8_t  v80, v81, v82, v83, v8_int;
		int16x8_t  v160, v161, v162, v163;
		int16_t    work_idx , work_arr[8];
		int x, y;
		uint8_t *src		  ;
		ptrdiff_t srcstride   = _srcstride;
		int16_t *dst		  = _dst;

		uint8x8_t  vf0;
		uint8x8_t  vf1;
		uint8x8_t  vf2;
		uint8x8_t  vf3;
		int shift = 14 - BIT_DEPTH;

		switch( my )
		{
		  case   1:
		    vf0 = vdup_n_u8(2);
		    vf1 = vdup_n_u8(58);
		    vf2 = vdup_n_u8(10);
		    vf3 = vdup_n_u8(2);
		  	break;
		  case  2:
		    vf0 = vdup_n_u8(4);
		    vf1 = vdup_n_u8(54);
		    vf2 = vdup_n_u8(16);
		    vf3 = vdup_n_u8(2);
		  	break;
		  case  3:
		    vf0 = vdup_n_u8(6);
		    vf1 = vdup_n_u8(46);
		    vf2 = vdup_n_u8(28);
		    vf3 = vdup_n_u8(4);
		  	break;
		  case  4:
		    vf0 = vdup_n_u8(4);
		    vf1 = vdup_n_u8(36);
		    vf2 = vdup_n_u8(36);
		    vf3 = vdup_n_u8(4);
		  	break;
		  case  5:
		    vf0 = vdup_n_u8(4);
		    vf1 = vdup_n_u8(28);
		    vf2 = vdup_n_u8(46);
		    vf3 = vdup_n_u8(6);
		  	break;
		  case  6:
		    vf0 = vdup_n_u8(2);
		    vf1 = vdup_n_u8(16);
		    vf2 = vdup_n_u8(54);
		    vf3 = vdup_n_u8(4);
		  	break;
		  default :
		    vf0 = vdup_n_u8(2);
		    vf1 = vdup_n_u8(10);
		    vf2 = vdup_n_u8(58);
		    vf3 = vdup_n_u8(2);
		}

        _src = _src - _srcstride;
		for (x = 0 ; x < width; x += 8)
		{
		   src = _src + x;
		   dst = _dst + x;
		   v80 = vld1_u8(src);
		   src += _srcstride;
		   v81 = vld1_u8(src);
		   src += _srcstride;
		   v82 = vld1_u8(src);
		   src += _srcstride;
		   v83 = vld1_u8(src);

           if ( (x + 8 ) <= width )
           {
		     for (y = 0; y < height ; y++, dst += MAX_PB_SIZE)
		     {
				src += _srcstride;
	            v161 = vmull_u8(v81,vf1);
	            v162 = vmull_u8(v82,vf2);
	            v161 = vmlsl_u8(v161,v80,vf0);
	            v162 = vmlsl_u8(v162,v83,vf3);
	            v80  = v81;
				v161 = vaddq_s16(v161 , v162);
	            v81  = v82;
	            v82  = v83;
	            v83  = vld1_u8(src);

		        vst1q_s16(dst, v161);
		     }           
           }
		   else 
		   {
		     for (y = 0; y < height ; y++, dst += MAX_PB_SIZE)
		     {
				src += _srcstride;
	            v161 = vmull_u8(v81,vf1);
	            v162 = vmull_u8(v82,vf2);
	            v161 = vmlsl_u8(v161,v80,vf0);
	            v162 = vmlsl_u8(v162,v83,vf3);
	            v80  = v81;
				v161 = vaddq_s16(v161 , v162);
	            v81  = v82;
	            v82  = v83;
	            v83  = vld1_u8(src);

		        if ( (width & 7 ) == 4 )
		        { 
				  vst1q_lane_s64 ((int64_t *)dst, v161, 0); 
		        }
		        else
		        {
		          vst1q_s16(work_arr, v161);
			      for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			      {
			        dst[work_idx] = work_arr[work_idx];
			      }
		        }
		     }		      
		   }

		}
}


void put_hevc_epel_v_opt_8(int16_t *_dst, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
   //CLK_DEF(o_put_hevc_epel_v)
   //CLK_T0(o_put_hevc_epel_v)	

   if ( (width & 7 ) == 0 )
        put_hevc_epel_v_opt_8_s(_dst,  _src, _srcstride,
                                  height, mx, my, width);
   else
        put_hevc_epel_v_opt_8_ns(_dst,  _src, _srcstride,
                                  height, mx, my, width);
   //CLK_T1(o_put_hevc_epel_v)

}

void put_hevc_epel_hv_opt_8(int16_t *_dst, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width)
{
   uint8_t tmp_array[(MAX_PB_SIZE + EPEL_EXTRA) * MAX_PB_SIZE];
   uint8_t * src	 = _src - EPEL_EXTRA_BEFORE * _srcstride;
   uint8_t * tmp	 = tmp_array + EPEL_EXTRA_BEFORE * MAX_PB_SIZE; 
   //CLK_DEF(o_put_hevc_epel_hv);
   //CLK_T0(o_put_hevc_epel_hv);
   put_hevc_epel_uni_h_opt_8(tmp_array,  MAX_PB_SIZE,
						  src, _srcstride,
						  height + EPEL_EXTRA , mx, 0, width);
   put_hevc_epel_v_opt_8(_dst,  
						  tmp, MAX_PB_SIZE,
						  height, 0, my, width);
   //CLK_T1(o_put_hevc_epel_hv);
}

static void put_hevc_epel_bi_h_opt_8_s(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
		                               int16_t *src2,
		                               int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83;
	int16x8_t  v160, v161, v162, v163;

    int x, y;
    uint8_t *src          = _src - 1;
    ptrdiff_t srcstride   = _srcstride;
    uint8_t *dst          = _dst;
    ptrdiff_t dststride   = _dststride;

    uint8x8_t  vf0;
    uint8x8_t  vf1;
    uint8x8_t  vf2;
    uint8x8_t  vf3;
    int shift = 14 - BIT_DEPTH;

	switch( mx )
	{
	  case   1:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(58);
	    vf2 = vdup_n_u8(10);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  2:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(54);
	    vf2 = vdup_n_u8(16);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  3:
	    vf0 = vdup_n_u8(6);
	    vf1 = vdup_n_u8(46);
	    vf2 = vdup_n_u8(28);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  4:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(36);
	    vf2 = vdup_n_u8(36);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  5:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(28);
	    vf2 = vdup_n_u8(46);
	    vf3 = vdup_n_u8(6);
	  	break;
	  case  6:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(16);
	    vf2 = vdup_n_u8(54);
	    vf3 = vdup_n_u8(4);
	  	break;
	  default :
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(10);
	    vf2 = vdup_n_u8(58);
	    vf3 = vdup_n_u8(2);
	}


	for (y = 0; y < height ;y++)
	{
	   for (x = 0 ; x < width ; dst += 8)
	   {
		v80 = vld1_u8(src);
		v81 = vld1_u8(src + 1);
		v82 = vld1_u8(src + 2);
		v83 = vld1_u8(src + 3);
		v160 = vld1q_s16(src2);

		v161 = vmull_u8(v81,vf1);
		v162 = vmull_u8(v82,vf2);
		v161 = vmlsl_u8(v161,v80,vf0);
		v162 = vmlsl_u8(v162,v83,vf3);

		v161 = vaddq_s16(v161 , v162);
		src2 += 8;
		v160 = vqaddq_s16(v160, v161);
		v163 = vrshrq_n_s16(v160 , 7);
		src += 8;
		v80 = vqmovun_s16( v163 );
		x += 8;
		vst1_u8(dst, v80);

	   }
	   src += srcstride - x;
	   dst += dststride - x;
	   src2 += MAX_PB_SIZE - x;

	}
}

static void put_hevc_epel_bi_h_opt_8_ns(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                        int16_t *src2,
		                                int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83;
	int16x8_t  v160, v161, v162, v163;
	uint8_t    work_idx , work_arr[8];

    int x, y;
    uint8_t *src          = _src - 1;
    ptrdiff_t srcstride   = _srcstride;
    uint8_t *dst          = _dst;
    ptrdiff_t dststride   = _dststride;



    uint8x8_t  vf0;
    uint8x8_t  vf1;
    uint8x8_t  vf2;
    uint8x8_t  vf3;
    int shift = 14 - BIT_DEPTH;

	switch( mx )
	{
	  case   1:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(58);
	    vf2 = vdup_n_u8(10);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  2:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(54);
	    vf2 = vdup_n_u8(16);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  3:
	    vf0 = vdup_n_u8(6);
	    vf1 = vdup_n_u8(46);
	    vf2 = vdup_n_u8(28);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  4:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(36);
	    vf2 = vdup_n_u8(36);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  5:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(28);
	    vf2 = vdup_n_u8(46);
	    vf3 = vdup_n_u8(6);
	  	break;
	  case  6:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(16);
	    vf2 = vdup_n_u8(54);
	    vf3 = vdup_n_u8(4);
	  	break;
	  default :
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(10);
	    vf2 = vdup_n_u8(58);
	    vf3 = vdup_n_u8(2);
	}

	for (y = 0; y < height ;y++)
	{
	   for (x = 0 ; x < width ;x += 8, dst += 8)
	   {
		 v80 = vld1_u8(src);
		 v81 = vld1_u8(src + 1);
		 v82 = vld1_u8(src + 2);
		 v83 = vld1_u8(src + 3);
		 v160 = vld1q_s16(src2);

		 v161 = vmull_u8(v81,vf1);
		 v162 = vmull_u8(v82,vf2);
		 v161 = vmlsl_u8(v161,v80,vf0);
		 v162 = vmlsl_u8(v162,v83,vf3);

		 v161 = vaddq_s16(v161 , v162);
		 src2 += 8;
		 v160 = vqaddq_s16(v160, v161);
		 v163 = vrshrq_n_s16(v160 , 7);
		 src += 8;
		 v80 = vqmovun_s16( v163 );

		  if ( (x + 8 ) <= width )
		  {
			  vst1_u8(dst, v80);
		  }
		  else
		  {
			  vst1_u8(work_arr, v80);
			  for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			  {
				dst[work_idx] = work_arr[work_idx];
			  }
		  }
	   }
	   src += srcstride - x;
	   dst += dststride - x;
	   src2 += MAX_PB_SIZE - x;
	}

}

void put_hevc_epel_bi_h_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int16_t *src2,
                                      int height, intptr_t mx, intptr_t my, int width)
{
#ifndef ASM_OPT
   //CLK_DEF(o_tput_hevc_epel_bi_h);
   //CLK_T0(o_tput_hevc_epel_bi_h);

   if ( (width & 7 ) == 0 )
        put_hevc_epel_bi_h_opt_8_s(_dst, _dststride, _src, _srcstride,
        		src2,height, mx, my, width);
   else
        put_hevc_epel_bi_h_opt_8_ns(_dst, _dststride, _src, _srcstride,
        		src2, height, mx, my, width);
   //CLK_T1(o_tput_hevc_epel_bi_h);
#else
    neon_put_hevc_epel_bi_h_opt_8(_dst, _dststride, _src, _srcstride,
                                   src2,
		                           height, mx, my, width);

#endif
}


static void put_hevc_epel_bi_v_opt_8_s(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                    int16_t *_src2,
		                            int height, intptr_t mx, intptr_t my, int width)
{
	uint8x8_t  v80, v81, v82, v83, v8_int;
	int16x8_t  v160, v161, v162, v163;

    int x, y;
    uint8_t *src          ;
    ptrdiff_t srcstride   = _srcstride;
    uint8_t *dst          = _dst;
    ptrdiff_t dststride   = _dststride;
	int16_t *src2         ;

    uint8x8_t  vf0;
    uint8x8_t  vf1;
    uint8x8_t  vf2;
    uint8x8_t  vf3;
    int shift = 14 - BIT_DEPTH;

	switch( my )
	{
	  case   1:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(58);
	    vf2 = vdup_n_u8(10);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  2:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(54);
	    vf2 = vdup_n_u8(16);
	    vf3 = vdup_n_u8(2);
	  	break;
	  case  3:
	    vf0 = vdup_n_u8(6);
	    vf1 = vdup_n_u8(46);
	    vf2 = vdup_n_u8(28);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  4:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(36);
	    vf2 = vdup_n_u8(36);
	    vf3 = vdup_n_u8(4);
	  	break;
	  case  5:
	    vf0 = vdup_n_u8(4);
	    vf1 = vdup_n_u8(28);
	    vf2 = vdup_n_u8(46);
	    vf3 = vdup_n_u8(6);
	  	break;
	  case  6:
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(16);
	    vf2 = vdup_n_u8(54);
	    vf3 = vdup_n_u8(4);
	  	break;
	  default :
	    vf0 = vdup_n_u8(2);
	    vf1 = vdup_n_u8(10);
	    vf2 = vdup_n_u8(58);
	    vf3 = vdup_n_u8(2);
	}


    _src = _src - _srcstride;
	for (x = 0 ; x < width; x += 8)
	{
	   src = _src + x;
	   dst = _dst + x;
	   src2 = _src2 + x;
	   v80 = vld1_u8(src);
	   src += _srcstride;
	   v81 = vld1_u8(src);
	   src += _srcstride;
	   v82 = vld1_u8(src);
	   src += _srcstride;
	   v83 = vld1_u8(src);

	   for (y = 0; y < height ; y++, dst += _dststride,src2 += MAX_PB_SIZE)
	   {
		    v160 = vld1q_s16(src2);
            v161 = vmull_u8(v81,vf1);
            v162 = vmull_u8(v82,vf2);
            v161 = vmlsl_u8(v161,v80,vf0);
            v162 = vmlsl_u8(v162,v83,vf3);
			src += _srcstride;
            v80  = v81;
			v161 = vaddq_s16(v161 , v162);
            v81  = v82;
            v160 = vqaddq_s16(v160, v161);
            v82  = v83;
			v8_int = vqrshrun_n_s16(v160 , 7);
            v83  = vld1_u8(src);
			vst1_u8(dst, v8_int);
	   }
	}

}

static void put_hevc_epel_bi_v_opt_8_ns(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int16_t *_src2,
                                      int height, intptr_t mx, intptr_t my, int width)
{
		uint8x8_t  v80, v81, v82, v83, v8_int;
		int16x8_t  v160, v161, v162, v163;
	    uint8_t    work_idx , work_arr[8];
		int x, y;
		uint8_t *src		  ;
		ptrdiff_t srcstride   = _srcstride;
		uint8_t *dst		  = _dst;
		ptrdiff_t dststride   = _dststride;
		int16_t *src2         ;

		uint8x8_t  vf0;
		uint8x8_t  vf1;
		uint8x8_t  vf2;
		uint8x8_t  vf3;
		int shift = 14 - BIT_DEPTH;

		switch( my )
		{
		  case   1:
		    vf0 = vdup_n_u8(2);
		    vf1 = vdup_n_u8(58);
		    vf2 = vdup_n_u8(10);
		    vf3 = vdup_n_u8(2);
		  	break;
		  case  2:
		    vf0 = vdup_n_u8(4);
		    vf1 = vdup_n_u8(54);
		    vf2 = vdup_n_u8(16);
		    vf3 = vdup_n_u8(2);
		  	break;
		  case  3:
		    vf0 = vdup_n_u8(6);
		    vf1 = vdup_n_u8(46);
		    vf2 = vdup_n_u8(28);
		    vf3 = vdup_n_u8(4);
		  	break;
		  case  4:
		    vf0 = vdup_n_u8(4);
		    vf1 = vdup_n_u8(36);
		    vf2 = vdup_n_u8(36);
		    vf3 = vdup_n_u8(4);
		  	break;
		  case  5:
		    vf0 = vdup_n_u8(4);
		    vf1 = vdup_n_u8(28);
		    vf2 = vdup_n_u8(46);
		    vf3 = vdup_n_u8(6);
		  	break;
		  case  6:
		    vf0 = vdup_n_u8(2);
		    vf1 = vdup_n_u8(16);
		    vf2 = vdup_n_u8(54);
		    vf3 = vdup_n_u8(4);
		  	break;
		  default :
		    vf0 = vdup_n_u8(2);
		    vf1 = vdup_n_u8(10);
		    vf2 = vdup_n_u8(58);
		    vf3 = vdup_n_u8(2);
		}

        _src = _src - _srcstride;
		for (x = 0 ; x < width; x += 8)
		{
		   src = _src + x;
		   dst = _dst + x;
		   src2 = _src2 + x;
		   v80 = vld1_u8(src);
		   src += _srcstride;
		   v81 = vld1_u8(src);
		   src += _srcstride;
		   v82 = vld1_u8(src);
		   src += _srcstride;
		   v83 = vld1_u8(src);

           if ((x + 8) <= width)
           {
		     for (y = 0; y < height ; y++, dst += _dststride, src2 += MAX_PB_SIZE)
		     {
			    v160 = vld1q_s16(src2);
	            v161 = vmull_u8(v81,vf1);
	            v162 = vmull_u8(v82,vf2);
	            v161 = vmlsl_u8(v161,v80,vf0);
	            v162 = vmlsl_u8(v162,v83,vf3);
				src += _srcstride;
	            v80  = v81;
				v161 = vaddq_s16(v161 , v162);
	            v81  = v82;
	            v160 = vqaddq_s16(v160, v161);
				v163 = vrshrq_n_s16(v160 , 7);
	            v82  = v83;
				v8_int = vqmovun_s16( v163 );
	            v83  = vld1_u8(src);
		        vst1_u8(dst, v8_int);
		     }           
           }
		   else
		   {
		     for (y = 0; y < height ; y++, dst += _dststride, src2 += MAX_PB_SIZE)
		     {
			    v160 = vld1q_s16(src2);
	            v161 = vmull_u8(v81,vf1);
	            v162 = vmull_u8(v82,vf2);
	            v161 = vmlsl_u8(v161,v80,vf0);
	            v162 = vmlsl_u8(v162,v83,vf3);
				src += _srcstride;
	            v80  = v81;
				v161 = vaddq_s16(v161 , v162);
	            v81  = v82;
	            v160 = vqaddq_s16(v160, v161);
				v163 = vrshrq_n_s16(v160 , 7);
	            v82  = v83;
				v8_int = vqmovun_s16( v163 );
	            v83  = vld1_u8(src);

		        if ( (width & 7 ) == 4 )
		        {
				  vst1_lane_u32((uint32_t *)dst, v8_int, 0);
		        }
		        else
		        {
		          vst1_u8(work_arr, v8_int);
			      for (work_idx = 0; work_idx < (width & 7) ; work_idx++)
			      {
			        dst[work_idx] = work_arr[work_idx];
			      }
		        }
		     }		   
		   }

		}
}


void put_hevc_epel_bi_v_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int16_t *_src2,
                                      int height, intptr_t mx, intptr_t my, int width)
{
   //CLK_DEF(o_tput_hevc_epel_bi_v);
   //CLK_T0(o_tput_hevc_epel_bi_v);

   if ( (width & 7 ) == 0 )
        put_hevc_epel_bi_v_opt_8_s(_dst, _dststride, _src, _srcstride,
        		                   _src2, height, mx, my, width);
   else
        put_hevc_epel_bi_v_opt_8_ns(_dst, _dststride, _src, _srcstride,
        		                   _src2, height, mx, my, width);
   //CLK_T1(o_tput_hevc_epel_bi_v);

}


void put_hevc_epel_bi_hv_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                      int16_t *src2,
                                      int height, intptr_t mx, intptr_t my, int width)
{
    uint8_t tmp_array[(MAX_PB_SIZE + EPEL_EXTRA) * MAX_PB_SIZE];
	uint8_t * src    = _src - EPEL_EXTRA_BEFORE * _srcstride;
    uint8_t * tmp    = tmp_array + EPEL_EXTRA_BEFORE * MAX_PB_SIZE;	
	//CLK_DEF(o_tput_hevc_epel_bi_hv);
	//CLK_T0(o_tput_hevc_epel_bi_hv);
	put_hevc_epel_uni_h_opt_8(tmp_array,  MAX_PB_SIZE,
                              src, _srcstride,
                              height + EPEL_EXTRA , mx, 0, width);
	
	put_hevc_epel_bi_v_opt_8(_dst,  _dststride,
                              tmp, MAX_PB_SIZE,
                              src2, height, 0, my, width);
    //CLK_T1(o_tput_hevc_epel_bi_hv);


}



