/*
 * HEVC video decoder
 *
 * Copyright (C) 2012 - 2013 Guillaume Martres
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/pixdesc.h"

#include "bit_depth_template.c"
#include "hevcpred.h"
#include "hevc_log.h"


#define POS(x, y) src[(x) + stride * (y)]
#define PU(x) \
    ((x) >> s->sps->log2_min_pu_size)
#define MVF(x, y) \
    (s->ref->tab_mvf[(x) + (y) * min_pu_width])
#define MVF_PU(x, y) \
    MVF(PU(x0 + ((x) << hshift)), PU(y0 + ((y) << vshift)))
#define IS_INTRA(x, y) \
    (MVF_PU(x, y).pred_flag == PF_INTRA)
#define MIN_TB_ADDR_ZS(x, y) \
    s->pps->min_tb_addr_zs[(y) * (s->sps->tb_mask+2) + (x)]\

//#define OPT_EXTEND
#ifndef OPT_EXTEND    
#define EXTEND(ptr, val, len)         \
do {                                  \
    pixel4 pix = PIXEL_SPLAT_X4(val); \
    for (i = 0; i < (len); i += 4)    \
        AV_WN4P(ptr + i, pix);        \
} while (0)
#else
#define EXTEND(ptr, val, len) memset(ptr, val, len)
#endif

#if HAVE_NEON_INLINE
 #include "test_neon.h"
 #if BIT_DEPTH == 8
  //#define STRONG_SMOOTH_OPT This option can not be opened
  
#ifdef HEVC_OPT_OPEN
  #define NORM_SMOOTH_OPT
  #define ANGULAR_OPT
  #define ANGULAR_1_OPT  
  #define ANGULAR_2_OPT
  #define ANGULAR_3_OPT
  #define REMOVE_COPY
  #define DC_OPT
  #define EXTEND_SIMD_OPT
#endif  

  #define EXTEND_4(ptr, val)         \
  do {								  \
	pixel4 pix = PIXEL_SPLAT_X4(val); \
	AV_WN4P(ptr , pix);		  \
  } while (0);

  #define EXTEND_8(ptr, val)         \
  do {                               \
  	uint8x8_t vec_load;            \
    vec_load = vdup_n_u8(val);     \
	vst1_u8(ptr,vec_load);                   \
  }while (0);                         \

  #define EXTEND_16(ptr, val)        \
  do {                               \
	uint8x16_t vec_load;           \
    vec_load = vdupq_n_u8(val);    \
	vst1q_u8(ptr,vec_load);                 \
  }while (0);                        \

  #define EXTEND_32(ptr, val)        \
  do {                               \
	uint8x16_t vec_load;           \
    vec_load = vdupq_n_u8(val);    \
	vst1q_u8(ptr,vec_load);                 \
	vst1q_u8(ptr + 16,vec_load);            \
  }while (0);                         \

  #define EXTEND_64(ptr, val)        \
  do {                               \
	uint8x16_t vec_load;           \
    vec_load = vdupq_n_u8(val);    \
	vst1q_u8(ptr,vec_load);               \
	vst1q_u8(ptr + 16,vec_load);            \
	vst1q_u8(ptr + 32,vec_load);            \
	vst1q_u8(ptr + 48,vec_load);            \
 }while (0); 
 #endif
#endif 

#define EXTEND_RIGHT_CIP(ptr, start, length)                                   \
        for (i = start; i < (start) + (length); i += 4)                        \
            if (!IS_INTRA(i, -1))                                              \
                AV_WN4P(&ptr[i], a);                                           \
            else                                                               \
                a = PIXEL_SPLAT_X4(ptr[i+3])
#define EXTEND_LEFT_CIP(ptr, start, length) \
        for (i = start; i > (start) - (length); i--) \
            if (!IS_INTRA(i - 1, -1)) \
                ptr[i - 1] = ptr[i]
#define EXTEND_UP_CIP(ptr, start, length)                                      \
        for (i = (start); i > (start) - (length); i -= 4)                      \
            if (!IS_INTRA(-1, i - 3))                                          \
                AV_WN4P(&ptr[i - 3], a);                                       \
            else                                                               \
                a = PIXEL_SPLAT_X4(ptr[i - 3])
#define EXTEND_DOWN_CIP(ptr, start, length)                                    \
        for (i = start; i < (start) + (length); i += 4)                        \
            if (!IS_INTRA(-1, i))                                              \
                AV_WN4P(&ptr[i], a);                                           \
            else                                                               \
                a = PIXEL_SPLAT_X4(ptr[i + 3])
#if 0

 //#define TL_ARR_DBG
 #ifdef TL_ARR_DBG
   //pixel  left_array##BIT_DEPTH##_dbg[2 * MAX_TB_SIZE + 1];
   //pixel  top_array##BIT_DEPTH##_dbg[2 * MAX_TB_SIZE + 1];
   //pixel  left_arra[BIT_DEPTH];
   //pixel  top_arra_[BIT_DEPTH];
   //#define LEFT_ARRDBG(DEPTH) left_array##DEPTH##_dbg[2 * MAX_TB_SIZE + 1]
   //#define TOP_ARRDBG(DEPTH)  top_array##DEPTH##_dbg[2 * MAX_TB_SIZE + 1]   
   #if BIT_DEPTH == 8
     #define TOP_ARRDBG top_array_dbg_8
     #define LEFT_ARRDBG left_array_dbg_8
     pixel TOP_ARRDBG[2 * MAX_TB_SIZE + 1];
     pixel LEFT_ARRDBG[2 * MAX_TB_SIZE + 1];	 
   #elif BIT_DEPTH == 9
     #define TOP_ARRDBG top_array_dbg_9
     #define LEFT_ARRDBG left_array_dbg_9
     pixel TOP_ARRDBG[2 * MAX_TB_SIZE + 1];
     pixel LEFT_ARRDBG[2 * MAX_TB_SIZE + 1];	 
   #elif BIT_DEPTH == 10
     #define TOP_ARRDBG top_array_dbg_10
     #define LEFT_ARRDBG left_array_dbg_10
     pixel TOP_ARRDBG[2 * MAX_TB_SIZE + 1];
     pixel LEFT_ARRDBG[2 * MAX_TB_SIZE + 1];	 
   #elif BIT_DEPTH == 12
     #define TOP_ARRDBG top_array_dbg_12
     #define LEFT_ARRDBG left_array_dbg_12  
     pixel TOP_ARRDBG[2 * MAX_TB_SIZE + 1];
     pixel LEFT_ARRDBG[2 * MAX_TB_SIZE + 1];	 
   #endif
 #endif

static av_always_inline void FUNC(intra_pred_top)(HEVCContext *s, int x0, int y0,
                                              int log2_size, int c_idx)
{

    HEVCLocalContext *lc = s->HEVClc;
    int i;
    int hshift = s->sps->hshift[c_idx];
    int vshift = s->sps->vshift[c_idx];
    int size = (1 << log2_size);
    int size_in_luma_h = size << hshift;
    int size_in_tbs_h  = size_in_luma_h >> s->sps->log2_min_tb_size;
    int size_in_luma_v = size << vshift;
    int size_in_tbs_v  = size_in_luma_v >> s->sps->log2_min_tb_size;
    int x = x0 >> hshift;
    int y = y0 >> vshift;
    int x_tb = (x0 >> s->sps->log2_min_tb_size) & s->sps->tb_mask;
    int y_tb = (y0 >> s->sps->log2_min_tb_size) & s->sps->tb_mask;

    int cur_tb_addr = MIN_TB_ADDR_ZS(x_tb, y_tb);

    ptrdiff_t stride = s->frame->linesize[c_idx] / sizeof(pixel);
    pixel *src = (pixel*)s->frame->data[c_idx] + x + y * stride;

    int min_pu_width = s->sps->min_pu_width;

    enum IntraPredMode mode = c_idx ? lc->tu.intra_pred_mode_c :
                              lc->tu.intra_pred_mode;
    pixel4 a;
	
    pixel  left_array[2 * MAX_TB_SIZE + 1];
    pixel  filtered_left_array[2 * MAX_TB_SIZE + 1];
	#ifdef TL_ARR_DBG
	pixel  *top_array = TOP_ARRDBG;
	#else
    pixel  top_array[2 * MAX_TB_SIZE + 1];
	#endif
    pixel  filtered_top_array[2 * MAX_TB_SIZE + 1];

    pixel  *left          = left_array + 1;
    pixel  *top           = top_array  + 1;
    pixel  *filtered_left = filtered_left_array + 1;
    pixel  *filtered_top  = filtered_top_array  + 1;
    int cand_bottom_left = lc->na.cand_bottom_left && cur_tb_addr > MIN_TB_ADDR_ZS( x_tb - 1, (y_tb + size_in_tbs_v) & s->sps->tb_mask);
    int cand_left        = lc->na.cand_left;
    int cand_up_left     = lc->na.cand_up_left;
    int cand_up          = lc->na.cand_up;
    int cand_up_right    = lc->na.cand_up_right    && cur_tb_addr > MIN_TB_ADDR_ZS((x_tb + size_in_tbs_h) & s->sps->tb_mask, y_tb - 1);

    int bottom_left_size = (FFMIN(y0 + 2 * size_in_luma_v, s->sps->height) -
                           (y0 + size_in_luma_v)) >> vshift;
    int top_right_size   = (FFMIN(x0 + 2 * size_in_luma_h, s->sps->width) -
                           (x0 + size_in_luma_h)) >> hshift;

	//we only need left[0] and left[-1] and top
    if (cand_up_left) {
        left[-1] = POS(-1, -1);
        top[-1]  = left[-1];
    }
    if (cand_up)
        memcpy(top, src - stride, size * sizeof(pixel));
    if (cand_up_right) {
        memcpy(top + size, src - stride + size, size * sizeof(pixel));
        EXTEND(top + size + top_right_size, POS(size + top_right_size - 1, -1),
               size - top_right_size);
    }
	
    if (cand_left)
    {
    /*
        for (i = 0; i < size; i++)
            left[i] = POS(-1, i);
    */
        left[0] = POS(-1, 0);    
    }
	/*	
    if (cand_bottom_left) {
        for (i = size; i < size + bottom_left_size; i++)
            left[i] = POS(-1, i);
        EXTEND(left + size + bottom_left_size, POS(-1, size + bottom_left_size - 1),
               size - bottom_left_size);
    }
    */

    // Infer the unavailable samples
    if (!cand_bottom_left) {
		
        if (cand_left) {
            //EXTEND(left + size, left[size - 1], size);
        } else if (cand_up_left) {
            //EXTEND(left, left[-1], 2 * size);
			left[0] = left[-1];
            cand_left = 1;
        } else if (cand_up) {
            left[-1] = top[0];
            //EXTEND(left, left[-1], 2 * size);
			left[0] = left[-1];			
            cand_up_left = 1;
            cand_left    = 1;
        } else if (cand_up_right) {
            EXTEND(top, top[size], size);
            left[-1] = top[size];
            //EXTEND(left, left[-1], 2 * size);
            left[0] = left[-1];	
            cand_up      = 1;
            cand_up_left = 1;
            cand_left    = 1;
        } else { // No samples available
            left[-1] = (1 << (BIT_DEPTH - 1));
            EXTEND(top,  left[-1], 2 * size);
            //EXTEND(left, left[-1], 2 * size);
            left[0] = (1 << (BIT_DEPTH - 1));
        }
    }

    /*
    if (!cand_left)
        EXTEND(left, left[size], size);
    */    
    if (!cand_up_left) {
        left[-1] = left[0];
    }
    if (!cand_up)
        EXTEND(top, left[-1], size);
    if (!cand_up_right)
        EXTEND(top + size, top[size - 1], size);

    top[-1] = left[-1];

}

static av_always_inline void FUNC(intra_pred_left)(HEVCContext *s, int x0, int y0,
                                              int log2_size, int c_idx)
{

	
    HEVCLocalContext *lc = s->HEVClc;
    int i;
    int hshift = s->sps->hshift[c_idx];
    int vshift = s->sps->vshift[c_idx];
    int size = (1 << log2_size);
    int size_in_luma_h = size << hshift;
    int size_in_tbs_h  = size_in_luma_h >> s->sps->log2_min_tb_size;
    int size_in_luma_v = size << vshift;
    int size_in_tbs_v  = size_in_luma_v >> s->sps->log2_min_tb_size;
    int x = x0 >> hshift;
    int y = y0 >> vshift;
    int x_tb = (x0 >> s->sps->log2_min_tb_size) & s->sps->tb_mask;
    int y_tb = (y0 >> s->sps->log2_min_tb_size) & s->sps->tb_mask;

    int cur_tb_addr = MIN_TB_ADDR_ZS(x_tb, y_tb);

    ptrdiff_t stride = s->frame->linesize[c_idx] / sizeof(pixel);
    pixel *src = (pixel*)s->frame->data[c_idx] + x + y * stride;

    int min_pu_width = s->sps->min_pu_width;

    enum IntraPredMode mode = c_idx ? lc->tu.intra_pred_mode_c :
                              lc->tu.intra_pred_mode;
    pixel4 a;
	#ifdef TL_ARR_DBG	
    pixel  *left_array = LEFT_ARRDBG;
	#else
    pixel  left_array[2 * MAX_TB_SIZE + 1];	
	#endif
    pixel  filtered_left_array[2 * MAX_TB_SIZE + 1];
    pixel  top_array[2 * MAX_TB_SIZE + 1];
    pixel  filtered_top_array[2 * MAX_TB_SIZE + 1];

    pixel  *left          = left_array + 1;
    pixel  *top           = top_array  + 1;
    pixel  *filtered_left = filtered_left_array + 1;
    pixel  *filtered_top  = filtered_top_array  + 1;
    int cand_bottom_left = lc->na.cand_bottom_left && cur_tb_addr > MIN_TB_ADDR_ZS( x_tb - 1, (y_tb + size_in_tbs_v) & s->sps->tb_mask);
    int cand_left        = lc->na.cand_left;
    int cand_up_left     = lc->na.cand_up_left;
    int cand_up          = lc->na.cand_up;
    int cand_up_right    = lc->na.cand_up_right    && cur_tb_addr > MIN_TB_ADDR_ZS((x_tb + size_in_tbs_h) & s->sps->tb_mask, y_tb - 1);

    int bottom_left_size = (FFMIN(y0 + 2 * size_in_luma_v, s->sps->height) -
                           (y0 + size_in_luma_v)) >> vshift;
    int top_right_size   = (FFMIN(x0 + 2 * size_in_luma_h, s->sps->width) -
                           (x0 + size_in_luma_h)) >> hshift;


	//we do not need top
    if (cand_up_left) {
        left[-1] = POS(-1, -1);
        top[-1]  = left[-1];
    }
	
    if (cand_up)
         //memcpy(top, src - stride, size * sizeof(pixel));
         top[0] = *(src - stride);
    	
    if (cand_up_right) {
         //memcpy(top + size, src - stride + size, size * sizeof(pixel));
         //EXTEND(top + size + top_right_size, POS(size + top_right_size - 1, -1),
         //       size - top_right_size);

		 top[size] = *(src - stride + size);
    }
    
    
    if (cand_left)
        for (i = 0; i < size; i++)
            left[i] = POS(-1, i);
    if (cand_bottom_left) {
        for (i = size; i < size + bottom_left_size; i++)
            left[i] = POS(-1, i);
        EXTEND(left + size + bottom_left_size, POS(-1, size + bottom_left_size - 1),
               size - bottom_left_size);
    }

    // Infer the unavailable samples
    if (!cand_bottom_left) {
        if (cand_left) {
            EXTEND(left + size, left[size - 1], size);
        } else if (cand_up_left) {
            EXTEND(left, left[-1], 2 * size);
            cand_left = 1;
        } else if (cand_up) {
            left[-1] = top[0];
            EXTEND(left, left[-1], 2 * size);
            cand_up_left = 1;
            cand_left    = 1;
        } else if (cand_up_right) {
            //EXTEND(top, top[size], size);
            left[-1] = top[size];
            EXTEND(left, left[-1], 2 * size);
            cand_up      = 1;
            cand_up_left = 1;
            cand_left    = 1;
        } else { // No samples available
            left[-1] = (1 << (BIT_DEPTH - 1));
            //EXTEND(top,  left[-1], 2 * size);
            EXTEND(left, left[-1], 2 * size);
        }
    }

    if (!cand_left)
        EXTEND(left, left[size], size);
    if (!cand_up_left) {
        left[-1] = left[0];
    }
	
	/*
    if (!cand_up)
        EXTEND(top, left[-1], size);
    if (!cand_up_right)
        EXTEND(top + size, top[size - 1], size);

    top[-1] = left[-1];
    */
}

#ifdef TL_ARR_DBG
  FUNC(intra_pred_top)(s, x0, y0, log2_size, c_idx);
  FUNC(intra_pred_left)(s, x0, y0, log2_size, c_idx);  
  #if BIT_DEPTH == 8
	if (cmp_uint8_arr(top_array  ,	TOP_ARRDBG , 2 * size + 1, 0, "top_array") == -1)
	{
	   LOGI_ERR("top block %d %d %d %d", x0, y0, log2_size, c_idx);
	}
  
	if (cmp_uint8_arr(left_array ,	LEFT_ARRDBG ,2 * size + 1, 0, "left_array") == -1)
	{
	   LOGI_ERR("left block %d %d %d %d", x0, y0, log2_size, c_idx);
	}	  
  #endif
#endif

#endif

static av_always_inline void FUNC(intra_pred)(HEVCContext *s, int x0, int y0,
                                              int log2_size, int c_idx)
{

    CLK_DEF(tintra_pred0);	
    HEVCLocalContext *lc = s->HEVClc;
    int i;
    int hshift = s->sps->hshift[c_idx];
    int vshift = s->sps->vshift[c_idx];
    int size = (1 << log2_size);
    int size_in_luma_h = size << hshift;
    int size_in_tbs_h  = size_in_luma_h >> s->sps->log2_min_tb_size;
    int size_in_luma_v = size << vshift;
    int size_in_tbs_v  = size_in_luma_v >> s->sps->log2_min_tb_size;
    int x = x0 >> hshift;
    int y = y0 >> vshift;
    int x_tb = (x0 >> s->sps->log2_min_tb_size) & s->sps->tb_mask;
    int y_tb = (y0 >> s->sps->log2_min_tb_size) & s->sps->tb_mask;

    int cur_tb_addr = MIN_TB_ADDR_ZS(x_tb, y_tb);

    ptrdiff_t stride = s->frame->linesize[c_idx] / sizeof(pixel);
    pixel *src = (pixel*)s->frame->data[c_idx] + x + y * stride;

    int min_pu_width = s->sps->min_pu_width;

    enum IntraPredMode mode = c_idx ? lc->tu.intra_pred_mode_c :
                              lc->tu.intra_pred_mode;
    pixel4 a;

    #ifdef REMOVE_COPY
	 #if (defined NORM_SMOOTH_OPT) || (defined STRONG_SMOOTH_OPT)	
	
       pixel  left_array[3 * MAX_TB_SIZE + 2];
       pixel  filtered_left_array[3 * MAX_TB_SIZE + 2];
       pixel  top_array[3 * MAX_TB_SIZE + 2];
       pixel  filtered_top_array[3 * MAX_TB_SIZE + 2];	
	   uint8x8_t v0l, v1l, v2l, v4l, v0t, v1t, v2t, v4t, v_two, v_eight;
	   uint16x8_t v3l,v5l,v3t,v5t;
	   						//63~56
	   uint8_t coeff0[8] = {63,62,61,60,59,58,57,56};
	   uint8_t coeff1[8] = {1,2,3,4,5,6,7,8};
	   pixel  *left          = left_array + 1 + MAX_TB_SIZE;
       pixel  *top           = top_array  + 1 + MAX_TB_SIZE;
       pixel  *filtered_left = filtered_left_array + 1  + MAX_TB_SIZE;
       pixel  *filtered_top  = filtered_top_array  + 1  + MAX_TB_SIZE;	   
	 #else
       pixel  left_array[3 * MAX_TB_SIZE + 1];
       pixel  filtered_left_array[3 * MAX_TB_SIZE + 1];
       pixel  top_array[3 * MAX_TB_SIZE + 1];
       pixel  filtered_top_array[3 * MAX_TB_SIZE + 1];
	   pixel  *left          = left_array + 1 + MAX_TB_SIZE;
       pixel  *top           = top_array  + 1 + MAX_TB_SIZE;
       pixel  *filtered_left = filtered_left_array + 1  + MAX_TB_SIZE;
       pixel  *filtered_top  = filtered_top_array  + 1  + MAX_TB_SIZE;	   
	 #endif	
	#else
	 #if (defined NORM_SMOOTH_OPT) || (defined STRONG_SMOOTH_OPT)	
	
       pixel  left_array[2 * MAX_TB_SIZE + 2];
       pixel  filtered_left_array[2 * MAX_TB_SIZE + 2];
       pixel  top_array[2 * MAX_TB_SIZE + 2];
       pixel  filtered_top_array[2 * MAX_TB_SIZE + 2];	
	   uint8x8_t v0l, v1l, v2l, v4l, v0t, v1t, v2t, v4t, v_two, v_eight;
	   uint16x8_t v3l,v5l,v3t,v5t;
	   						//63~56
	   uint8_t coeff0[8] = {63,62,61,60,59,58,57,56};
	   uint8_t coeff1[8] = {1,2,3,4,5,6,7,8};	
	   pixel  *left          = left_array + 1;
       pixel  *top           = top_array  + 1;
       pixel  *filtered_left = filtered_left_array + 1;
       pixel  *filtered_top  = filtered_top_array  + 1;
	 #else
       pixel  left_array[2 * MAX_TB_SIZE + 1];
       pixel  filtered_left_array[2 * MAX_TB_SIZE + 1];
       pixel  top_array[2 * MAX_TB_SIZE + 1];
       pixel  filtered_top_array[2 * MAX_TB_SIZE + 1];
	   pixel  *left          = left_array + 1;
       pixel  *top           = top_array  + 1;
       pixel  *filtered_left = filtered_left_array + 1;
       pixel  *filtered_top  = filtered_top_array  + 1;
	 #endif  
	#endif 


    int cand_bottom_left = lc->na.cand_bottom_left && cur_tb_addr > MIN_TB_ADDR_ZS( x_tb - 1, (y_tb + size_in_tbs_v) & s->sps->tb_mask);
    int cand_left        = lc->na.cand_left;
    int cand_up_left     = lc->na.cand_up_left;
    int cand_up          = lc->na.cand_up;
    int cand_up_right    = lc->na.cand_up_right    && cur_tb_addr > MIN_TB_ADDR_ZS((x_tb + size_in_tbs_h) & s->sps->tb_mask, y_tb - 1);

    int bottom_left_size = (FFMIN(y0 + 2 * size_in_luma_v, s->sps->height) -
                           (y0 + size_in_luma_v)) >> vshift;
    int top_right_size   = (FFMIN(x0 + 2 * size_in_luma_h, s->sps->width) -
                           (x0 + size_in_luma_h)) >> hshift;

#if (defined NORM_SMOOTH_OPT) || (defined STRONG_SMOOTH_OPT)	
	   v_two = vdup_n_u8 (2);
       v_eight = vdup_n_u8 (8);
	   v1t = vld1_u8(coeff0);
	   v2t = vld1_u8(coeff1);	   
#endif

	CLK_T0(tintra_pred0);

    if (s->pps->constrained_intra_pred_flag == 1) {
        int size_in_luma_pu_v = PU(size_in_luma_v);
        int size_in_luma_pu_h = PU(size_in_luma_h);
        int on_pu_edge_x    = !(x0 & ((1 << s->sps->log2_min_pu_size) - 1));
        int on_pu_edge_y    = !(y0 & ((1 << s->sps->log2_min_pu_size) - 1));
        if (!size_in_luma_pu_h)
            size_in_luma_pu_h++;
        if (cand_bottom_left == 1 && on_pu_edge_x) {
            int x_left_pu   = PU(x0 - 1);
            int y_bottom_pu = PU(y0 + size_in_luma_v);
            int max = FFMIN(size_in_luma_pu_v, s->sps->min_pu_height - y_bottom_pu);
            cand_bottom_left = 0;
            for (i = 0; i < max; i += 2)
                cand_bottom_left |= (MVF(x_left_pu, y_bottom_pu + i).pred_flag == PF_INTRA);
        }
        if (cand_left == 1 && on_pu_edge_x) {
            int x_left_pu   = PU(x0 - 1);
            int y_left_pu   = PU(y0);
            int max = FFMIN(size_in_luma_pu_v, s->sps->min_pu_height - y_left_pu);
            cand_left = 0;
            for (i = 0; i < max; i += 2)
                cand_left |= (MVF(x_left_pu, y_left_pu + i).pred_flag == PF_INTRA);
        }
        if (cand_up_left == 1) {
            int x_left_pu   = PU(x0 - 1);
            int y_top_pu    = PU(y0 - 1);
            cand_up_left = MVF(x_left_pu, y_top_pu).pred_flag == PF_INTRA;
        }
        if (cand_up == 1 && on_pu_edge_y) {
            int x_top_pu    = PU(x0);
            int y_top_pu    = PU(y0 - 1);
            int max = FFMIN(size_in_luma_pu_h, s->sps->min_pu_width - x_top_pu);
            cand_up = 0;
            for (i = 0; i < max; i += 2)
                cand_up |= (MVF(x_top_pu + i, y_top_pu).pred_flag == PF_INTRA);
        }
        if (cand_up_right == 1 && on_pu_edge_y) {
            int y_top_pu    = PU(y0 - 1);
            int x_right_pu  = PU(x0 + size_in_luma_h);
            int max = FFMIN(size_in_luma_pu_h, s->sps->min_pu_width - x_right_pu);
            cand_up_right = 0;
            for (i = 0; i < max; i += 2)
                cand_up_right |= (MVF(x_right_pu + i, y_top_pu).pred_flag == PF_INTRA);
        }
        memset(left, 128, 2 * MAX_TB_SIZE*sizeof(pixel));
        memset(top , 128, 2 * MAX_TB_SIZE*sizeof(pixel));
        top[-1] = 128;
    }

    if (cand_up_left) {
        left[-1] = POS(-1, -1);
        top[-1]  = left[-1];
    }
	
    if (cand_up)
        memcpy(top, src - stride, size * sizeof(pixel));
    if (cand_up_right) {
        memcpy(top + size, src - stride + size, size * sizeof(pixel));
        EXTEND(top + size + top_right_size, POS(size + top_right_size - 1, -1),
               size - top_right_size);
    }
    if (cand_left)
        for (i = 0; i < size; i++)
            left[i] = POS(-1, i);
    if (cand_bottom_left) {
        for (i = size; i < size + bottom_left_size; i++)
            left[i] = POS(-1, i);
        EXTEND(left + size + bottom_left_size, POS(-1, size + bottom_left_size - 1),
               size - bottom_left_size);
    }

    if (s->pps->constrained_intra_pred_flag == 1) {
        if (cand_bottom_left || cand_left || cand_up_left || cand_up || cand_up_right) {
            int size_max_x = x0 + ((2 * size) << hshift) < s->sps->width ?
                                    2 * size : (s->sps->width - x0) >> hshift;
            int size_max_y = y0 + ((2 * size) << vshift) < s->sps->height ?
                                    2 * size : (s->sps->height - y0) >> vshift;
            int j = size + (cand_bottom_left? bottom_left_size: 0) -1;
            if (!cand_up_right) {
                size_max_x = x0 + ((size) << hshift) < s->sps->width ?
                                                    size : (s->sps->width - x0) >> hshift;
            }
            if (!cand_bottom_left) {
                size_max_y = y0 + (( size) << vshift) < s->sps->height ?
                                                     size : (s->sps->height - y0) >> vshift;
            }
            if (cand_bottom_left || cand_left || cand_up_left) {
                while (j > -1 && !IS_INTRA(-1, j))
                    j--;
                if (!IS_INTRA(-1, j)) {
                    j = 0;
                    while (j < size_max_x && !IS_INTRA(j, -1))
                        j++;
                    EXTEND_LEFT_CIP(top, j, j + 1);
                    left[-1] = top[-1];
                }
            } else {
                j = 0;
                while (j < size_max_x && !IS_INTRA(j, -1))
                    j++;
                if (j > 0)
                    if (x0 > 0) {
                        EXTEND_LEFT_CIP(top, j, j + 1);
                    } else {
                        EXTEND_LEFT_CIP(top, j, j);
                        top[-1] = top[0];
                    }
                left[-1] = top[-1];
            }
            left[-1] = top[-1];
            if (cand_bottom_left || cand_left) {
                a = PIXEL_SPLAT_X4(left[-1]);
                EXTEND_DOWN_CIP(left, 0, size_max_y);
            }
            if (!cand_left)
                EXTEND(left, left[-1], size);
            if (!cand_bottom_left)
                EXTEND(left + size, left[size - 1], size);
            if (x0 != 0 && y0 != 0) {
                a = PIXEL_SPLAT_X4(left[size_max_y - 1]);
                EXTEND_UP_CIP(left, size_max_y - 1, size_max_y);
                if (!IS_INTRA(-1, - 1))
                    left[-1] = left[0];
            } else if (x0 == 0) {
                EXTEND(left, 0, size_max_y);
            } else {
                a = PIXEL_SPLAT_X4(left[size_max_y - 1]);
                EXTEND_UP_CIP(left, size_max_y - 1, size_max_y);
            }
            top[-1] = left[-1];
            if (y0 != 0) {
                a = PIXEL_SPLAT_X4(left[-1]);
                EXTEND_RIGHT_CIP(top, 0, size_max_x);
            }
        }
    }
	
	
    // Infer the unavailable samples
#ifndef EXTEND_SIMD_OPT  
    if (!cand_bottom_left) {
        if (cand_left) {
            EXTEND(left + size, left[size - 1], size);
        } else if (cand_up_left) {
            EXTEND(left, left[-1], 2 * size);
            cand_left = 1;
        } else if (cand_up) {
            left[-1] = top[0];
            EXTEND(left, left[-1], 2 * size);
            cand_up_left = 1;
            cand_left    = 1;
        } else if (cand_up_right) {
            EXTEND(top, top[size], size);
            left[-1] = top[size];
            EXTEND(left, left[-1], 2 * size);
            cand_up      = 1;
            cand_up_left = 1;
            cand_left    = 1;
        } else { // No samples available
            left[-1] = (1 << (BIT_DEPTH - 1));
            EXTEND(top,  left[-1], 2 * size);
            EXTEND(left, left[-1], 2 * size);
        }
    }

    if (!cand_left)
        EXTEND(left, left[size], size);
    if (!cand_up_left) {
        left[-1] = left[0];
    }
    if (!cand_up)
        EXTEND(top, left[-1], size);
    if (!cand_up_right)
        EXTEND(top + size, top[size - 1], size);
#else  
    
	switch(log2_size)
	{
	   
	   case 2 :
         if (!cand_bottom_left) {
             if (cand_left) {
                 EXTEND_4(left + size, left[size - 1]);
             } else if (cand_up_left) {
                 EXTEND_8(left, left[-1]);
                 cand_left = 1;
             } else if (cand_up) {
                 left[-1] = top[0];
                 EXTEND_8(left, left[-1]);
                 cand_up_left = 1;
                 cand_left    = 1;
             } else if (cand_up_right) {
                 EXTEND_4(top, top[size]);
                 left[-1] = top[size];
                 EXTEND_8(left, left[-1]);
                 cand_up      = 1;
                 cand_up_left = 1;
                 cand_left    = 1;
             } else { // No samples available
                 left[-1] = (1 << (BIT_DEPTH - 1));
                 EXTEND_8(top,  left[-1]);
                 EXTEND_8(left, left[-1]);
             }
         }
         if (!cand_left)
             EXTEND_4(left, left[size]);
         if (!cand_up_left) {
             left[-1] = left[0];
         }
         if (!cand_up)
             EXTEND_4(top, left[-1]);
         if (!cand_up_right)
             EXTEND_4(top + size, top[size - 1]);	   	
	   break;
	   case 3 :
         if (!cand_bottom_left) {
             if (cand_left) {
                 EXTEND_8(left + size, left[size - 1]);
             } else if (cand_up_left) {
                 EXTEND_16(left, left[-1]);
                 cand_left = 1;
             } else if (cand_up) {
                 left[-1] = top[0];
                 EXTEND_16(left, left[-1]);
                 cand_up_left = 1;
                 cand_left    = 1;
             } else if (cand_up_right) {
                 EXTEND_8(top, top[size]);
                 left[-1] = top[size];
                 EXTEND_16(left, left[-1]);
                 cand_up      = 1;
                 cand_up_left = 1;
                 cand_left    = 1;
             } else { // No samples available
                 left[-1] = (1 << (BIT_DEPTH - 1));
                 EXTEND_16(top,  left[-1]);
                 EXTEND_16(left, left[-1]);
             }
         }
         if (!cand_left)
             EXTEND_8(left, left[size]);
         if (!cand_up_left) {
             left[-1] = left[0];
         }
         if (!cand_up)
             EXTEND_8(top, left[-1]);
         if (!cand_up_right)
             EXTEND_8(top + size, top[size - 1]);		   	
	   break;
	   case 4 :
         if (!cand_bottom_left) {
             if (cand_left) {
                 EXTEND_16(left + size, left[size - 1]);
             } else if (cand_up_left) {
                 EXTEND_32(left, left[-1]);
                 cand_left = 1;
             } else if (cand_up) {
                 left[-1] = top[0];
                 EXTEND_32(left, left[-1]);
                 cand_up_left = 1;
                 cand_left    = 1;
             } else if (cand_up_right) {
                 EXTEND_16(top, top[size]);
                 left[-1] = top[size];
                 EXTEND_32(left, left[-1]);
                 cand_up      = 1;
                 cand_up_left = 1;
                 cand_left    = 1;
             } else { // No samples available
                 left[-1] = (1 << (BIT_DEPTH - 1));
                 EXTEND_32(top,  left[-1]);
                 EXTEND_32(left, left[-1]);
             }
         }
         if (!cand_left)
             EXTEND_16(left, left[size]);
         if (!cand_up_left) {
             left[-1] = left[0];
         }
         if (!cand_up)
             EXTEND_16(top, left[-1]);
         if (!cand_up_right)
             EXTEND_16(top + size, top[size - 1]);		   	
	   break;
	   
	   default :
         if (!cand_bottom_left) {
             if (cand_left) {
                 EXTEND_32(left + size, left[size - 1]);
             } else if (cand_up_left) {
                 EXTEND_64(left, left[-1]);
                 cand_left = 1;
             } else if (cand_up) {
                 left[-1] = top[0];
                 EXTEND_64(left, left[-1]);
                 cand_up_left = 1;
                 cand_left    = 1;
             } else if (cand_up_right) {
                 EXTEND_32(top, top[size]);
                 left[-1] = top[size];
                 EXTEND_64(left, left[-1]);
                 cand_up      = 1;
                 cand_up_left = 1;
                 cand_left    = 1;
             } else { // No samples available
                 left[-1] = (1 << (BIT_DEPTH - 1));
                 EXTEND_64(top,  left[-1]);
                 EXTEND_64(left, left[-1]);
             }
         }
         if (!cand_left)
             EXTEND_32(left, left[size]);
         if (!cand_up_left) {
             left[-1] = left[0];
         }
         if (!cand_up)
             EXTEND_32(top, left[-1]);
         if (!cand_up_right)
             EXTEND_32(top + size, top[size - 1]);		   
	   break;
	}
	
    
#endif 
    top[-1] = left[-1];
	//CLK_T0(tintra_interpolation);

    // Filtering process
    if (!s->sps->intra_smoothing_disabled_flag && (c_idx == 0  || s->sps->chroma_format_idc == 3)) {
        if (mode != INTRA_DC && size != 4){
            int intra_hor_ver_dist_thresh[] = { 7, 1, 0 };
            int min_dist_vert_hor = FFMIN(FFABS((int)(mode - 26U)),
                                          FFABS((int)(mode - 10U)));
			//CLK_T0(tintra_pred1);
            if (min_dist_vert_hor > intra_hor_ver_dist_thresh[log2_size - 3]) {
                int threshold = 1 << (BIT_DEPTH - 5);
                if (s->sps->sps_strong_intra_smoothing_enable_flag && c_idx == 0 &&
                    log2_size == 5 &&
                    FFABS(top[-1]  + top[63]  - 2 * top[31])  < threshold &&
                    FFABS(left[-1] + left[63] - 2 * left[31]) < threshold) {
                    // We can't just overwrite values in top because it could be
                    // a pointer into src
                    #ifndef STRONG_SMOOTH_OPT
                     filtered_top[-1] = top[-1];
                     filtered_top[63] = top[63];
                     for (i = 0; i < 63; i++)
                        filtered_top[i] = ((64 - (i + 1)) * top[-1] +
                                           (i + 1)  * top[63] + 32) >> 6;
                     for (i = 0; i < 63; i++)
                        left[i] = ((64 - (i + 1)) * left[-1] +
                                   (i + 1)  * left[63] + 32) >> 6;                            
                     top = filtered_top;
                     
					#else
                     filtered_top[-1] = top[-1];
                     //for (i = 0; i < 63; i++)
                     //   filtered_top[i] = ((64 - (i + 1)) * top[-1] +
                     //                      (i + 1)  * top[63] + 32) >> 6;
                     //for (i = 0; i < 63; i++)
                     //   left[i] = ((64 - (i + 1)) * left[-1] +
                     //              (i + 1)  * left[63] + 32) >> 6;
                     v0l = vdup_n_u8 (top[-1]);
                     v1l = vdup_n_u8 (top[63]);
					 v2l = vdup_n_u8 (left[-1]);
					 v0t = vdup_n_u8 (left[63]);
					 for (i = 0; i < 64; i += 8)
					 {
						v3l = vmull_u8(v1t, v0l);
						v3l = vmlal_u8(v3l, v2t, v1l);
						v3t = vmull_u8(v1t, v2l);
						v3t = vmlal_u8(v3t, v2t, v0t);
						v5l = vrshrq_n_u16(v3l,6);
						v5t = vrshrq_n_u16(v3t,6);						
                        v4l = vmovn_u16(v5l);
                        v4t = vmovn_u16(v5t);						
						vst1_u8(filtered_top + i, v4l);
						vst1_u8(left + i, v4t);
						//63~56
						v1t = vsub_u8(v1t ,v_eight);
						//1~8
						v2t = vadd_u8(v2t ,v_eight);
					 }
					 filtered_top[63] = top[63];
                     top = filtered_top;					
					#endif

                } else {
                   #ifndef NORM_SMOOTH_OPT
                    filtered_left[2 * size - 1] = left[2 * size - 1];
                    filtered_top[2 * size - 1]  = top[2 * size - 1];
                    for (i = 2 * size - 2; i >= 0; i--)
                        filtered_left[i] = (left[i + 1] + 2 * left[i] +
                                            left[i - 1] + 2) >> 2;
                    filtered_top[-1]  =
                    filtered_left[-1] = (left[0] + 2 * left[-1] + top[0] + 2) >> 2;
                    for (i = 2 * size - 2; i >= 0; i--)
                        filtered_top[i] = (top[i + 1] + 2 * top[i] +
                                           top[i - 1] + 2) >> 2;                      
                    left = filtered_left;
                    top  = filtered_top;
                    
				   #else
					 //2*size should be 8/16/32/64
					 for (i = 0; i < 2*size; i += 8)
					 {
					    v0l = vld1_u8(left + i - 1);
					    v0t = vld1_u8(top + i - 1);						
					    v2l = vld1_u8(left + i + 1);
					    v2t = vld1_u8(top + i + 1);							
					    v1l = vld1_u8(left + i);
					    v1t = vld1_u8(top + i);						
						
						v3l = vaddl_u8(v0l, v2l);
						v3t = vaddl_u8(v0t, v2t);					
						v3l = vmlal_u8 (v3l, v_two, v1l);
						v3t = vmlal_u8 (v3t, v_two, v1t);						
						v5l = vrshrq_n_u16(v3l , 2);
						v5t = vrshrq_n_u16(v3t , 2);						
		                v0l = vmovn_u16( v5l );
		                v0t = vmovn_u16( v5t );						
						vst1_u8(filtered_left + i ,v0l);	
						vst1_u8(filtered_top + i ,v0t);							
						
					 }
                     //for (i = 2 * size - 2; i >= 0; i--)
                     //   filtered_left[i] = (left[i + 1] + 2 * left[i] +
                     //                       left[i - 1] + 2) >> 2;

                     //for (i = 2 * size - 2; i >= 0; i--)
                     //   filtered_top[i] = (top[i + 1] + 2 * top[i] +
                     //                      top[i - 1] + 2) >> 2;
					
                     filtered_left[2 * size - 1] = left[2 * size - 1];
                     filtered_top[2 * size - 1]  = top[2 * size - 1];
					 filtered_top[-1]  =
                     filtered_left[-1] = (left[0] + 2 * left[-1] + top[0] + 2) >> 2;					
                     left = filtered_left;
                     top  = filtered_top;				   					   
				   #endif
                }		
            }
			//CLK_T1(tintra_pred1);			
        }
    }
    //CLK_T1(tintra_interpolation);

    switch (mode) {
    case INTRA_PLANAR:
        s->hpc.pred_planar[log2_size - 2]((uint8_t *)src, (uint8_t *)top,
                                          (uint8_t *)left, stride);
        break;
    case INTRA_DC:
        s->hpc.pred_dc((uint8_t *)src, (uint8_t *)top,
                       (uint8_t *)left, stride, log2_size, c_idx);
        break;
    default:
        s->hpc.pred_angular[log2_size - 2]((uint8_t *)src, (uint8_t *)top,
                                           (uint8_t *)left, stride, c_idx,
                                           mode);
        break;
    }


	CLK_T1(tintra_pred0);	
}

#define INTRA_PRED(size)                                                            \
static void FUNC(intra_pred_ ## size)(HEVCContext *s, int x0, int y0, int c_idx)    \
{                                                                                   \
    FUNC(intra_pred)(s, x0, y0, size, c_idx);                                       \
}

INTRA_PRED(2)
INTRA_PRED(3)
INTRA_PRED(4)
INTRA_PRED(5)

#undef INTRA_PRED

static av_always_inline void FUNC(pred_planar)(uint8_t *_src, const uint8_t *_top,
                                  const uint8_t *_left, ptrdiff_t stride,
                                  int trafo_size)
{
    int x, y;
    pixel *src        = (pixel *)_src;
    const pixel *top  = (const pixel *)_top;
    const pixel *left = (const pixel *)_left;
    int size = 1 << trafo_size;
    for (y = 0; y < size; y++)
        for (x = 0; x < size; x++)
            POS(x, y) = ((size - 1 - x) * left[y] + (x + 1) * top[size]  +
                         (size - 1 - y) * top[x]  + (y + 1) * left[size] + size) >> (trafo_size + 1);
}

#define PRED_PLANAR(size)\
static void FUNC(pred_planar_ ## size)(uint8_t *src, const uint8_t *top,        \
                                       const uint8_t *left, ptrdiff_t stride)   \
{                                                                               \
    FUNC(pred_planar)(src, top, left, stride, size + 2);                        \
}

PRED_PLANAR(0)
PRED_PLANAR(1)
PRED_PLANAR(2)
PRED_PLANAR(3)

#undef PRED_PLANAR

#ifdef BIT_DEPTH == 8
static void FUNC(pred_dc)(uint8_t *_src, const uint8_t *_top,
                          const uint8_t *_left,
                          ptrdiff_t stride, int log2_size, int c_idx)
{
    int i, j, x, y;
    int size          = (1 << log2_size);
    pixel *src        = (pixel *)_src;
    const pixel *top  = (const pixel *)_top;
    const pixel *left = (const pixel *)_left;
    int dc            = size;
    pixel4 a;
    for (i = 0; i < size; i++)
        dc += left[i] + top[i];

    dc >>= log2_size + 1;
    //LOGI_point(DCINTRA, "dc is optimized");
	//return;
	#ifdef DC_OPT
	   uint8_t val = dc;
	   switch(log2_size)
	   {
	     case 2 :
          for (i = 0; i < size; i++)
          {
             EXTEND_4(src + stride * (i),val) 
          }		 	
		 break;
	     case 3 :
          for (i = 0; i < size; i++)
          {
             EXTEND_8(src + stride * (i),val) 
          }			 	
		 break;
	     case 4 :
          for (i = 0; i < size; i++)
          {
             EXTEND_16(src + stride * (i),val) 
          }		 	
		 break;
	     case 5 :
          for (i = 0; i < size; i++)
          {
             EXTEND_32(src + stride * (i),val) 
          }			 	
		 break;

	   }

	#else
	   a = PIXEL_SPLAT_X4(dc);
       for (i = 0; i < size; i++)
       {
         for (j = 0; j < size; j+=4)
            AV_WN4P(&POS(j, i), a);
       }	
	#endif

    if (c_idx == 0 && size < 32) {
        POS(0, 0) = (left[0] + 2 * dc + top[0] + 2) >> 2;
        for (x = 1; x < size; x++)
            POS(x, 0) = (top[x] + 3 * dc + 2) >> 2;
        for (y = 1; y < size; y++)
            POS(0, y) = (left[y] + 3 * dc + 2) >> 2;
    }
}
#else
static void FUNC(pred_dc)(uint8_t *_src, const uint8_t *_top,
                          const uint8_t *_left,
                          ptrdiff_t stride, int log2_size, int c_idx)
{
    int i, j, x, y;
    int size          = (1 << log2_size);
    pixel *src        = (pixel *)_src;
    const pixel *top  = (const pixel *)_top;
    const pixel *left = (const pixel *)_left;
    int dc            = size;
    pixel4 a;
    for (i = 0; i < size; i++)
        dc += left[i] + top[i];

    dc >>= log2_size + 1;

    a = PIXEL_SPLAT_X4(dc);

	
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j+=4)
            AV_WN4P(&POS(j, i), a);
    }

    if (c_idx == 0 && size < 32) {
        POS(0, 0) = (left[0] + 2 * dc + top[0] + 2) >> 2;
        for (x = 1; x < size; x++)
            POS(x, 0) = (top[x] + 3 * dc + 2) >> 2;
        for (y = 1; y < size; y++)
            POS(0, y) = (left[y] + 3 * dc + 2) >> 2;
    }
}

#endif 

static av_always_inline void FUNC(pred_angular)(uint8_t *_src,
                                                const uint8_t *_top,
                                                const uint8_t *_left,
                                                ptrdiff_t stride, int c_idx,
                                                int mode, int size)
{
    int x, y;
    pixel *src        = (pixel *)_src;
    const pixel *top  = (const pixel *)_top;
    const pixel *left = (const pixel *)_left;

    static const int intra_pred_angle[] = {
         32,  26,  21,  17, 13,  9,  5, 2, 0, -2, -5, -9, -13, -17, -21, -26, -32,
        -26, -21, -17, -13, -9, -5, -2, 0, 2,  5,  9, 13,  17,  21,  26,  32
    };
    static const int inv_angle[] = {
        -4096, -1638, -910, -630, -482, -390, -315, -256, -315, -390, -482,
        -630, -910, -1638, -4096
    };

    int angle = intra_pred_angle[mode - 2];
    pixel ref_array[3 * MAX_TB_SIZE + 4];
    pixel *ref_tmp = ref_array + size;
    pixel *ref;
    int last = (size * angle) >> 5;

    if (mode >= 18) {
        ref = top - 1;
        if (angle < 0 && last < -1) {
         #ifndef REMOVE_COPY			
            for (x = 0; x <= size; x += 4)
                AV_WN4P(&ref_tmp[x], AV_RN4P(&top[x - 1]));
            /*
			#if BIT_DEPTH == 8
				memcpy(ref_tmp, top -  1, size + 1);
            #else
				memcpy(ref_tmp, top -  1, 2*size + 2);
			#endif
			*/
            for (x = last; x <= -1; x++)
                ref_tmp[x] = left[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];
            ref = ref_tmp;
		 #else
		    /*
            for (x = 0; x <= size; x += 4)
                AV_WN4P(&ref_tmp[x], AV_RN4P(&top[x - 1]));
            */    
			
            for (x = last; x <= -1; x++)
                ref[x] = left[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];
            //ref = ref_tmp;		 
	  	 #endif	
        }
#ifndef ANGULAR_OPT
        for (y = 0; y < size; y++) {
            int idx  = ((y + 1) * angle) >> 5;
            int fact = ((y + 1) * angle) & 31;
            if (fact) {
                for (x = 0; x < size; x += 4) {
                    POS(x    , y) = ((32 - fact) * ref[x + idx + 1] +
                                           fact  * ref[x + idx + 2] + 16) >> 5;
                    POS(x + 1, y) = ((32 - fact) * ref[x + 1 + idx + 1] +
                                           fact  * ref[x + 1 + idx + 2] + 16) >> 5;
                    POS(x + 2, y) = ((32 - fact) * ref[x + 2 + idx + 1] +
                                           fact  * ref[x + 2 + idx + 2] + 16) >> 5;
                    POS(x + 3, y) = ((32 - fact) * ref[x + 3 + idx + 1] +
                                           fact  * ref[x + 3 + idx + 2] + 16) >> 5;
                }
            } else {
                for (x = 0; x < size; x += 4)
                    AV_WN4P(&POS(x, y), AV_RN4P(&ref[x + idx + 1]));
            }
        }
#else
        for (y = 0; y < size; y++) {
        	int    idx  = ((y + 1) * angle) >> 5;
        	uint8_t fact = ((y + 1) * angle) & 31;
			uint8x8_t v0,v1,v2,v3,v6;
			uint16x8_t v4,v5;
        	if ( fact ) {
		       if( size <= 4 )
		       {
        		 for (x = 0; x < size; x++) {
        			POS(x	 , y) = ((32 - fact) * ref[x + idx + 1] +
        								   fact  * ref[x + idx + 2] + 16) >> 5;
		         } 		       
		       }
			   else
			   {
		         v0 = vdup_n_u8(32 - fact);	
		         v2 = vdup_n_u8(fact);				 
		         for (x = 0; x < size; x += 8) {
                   v1 = vld1_u8(ref + x + idx + 1);
                   v3 = vld1_u8(ref + x + idx + 2);
				   v4 = vmull_u8(v0,v1);
				   v4 = vmlal_u8(v4, v2,v3);
				   v5 = vrshrq_n_u16(v4,5);
				   v6 = vmovn_u16(v5);
				   vst1_u8(&POS(x	 , y), v6);
		         }			   
			   }

        	} else {
        	     #if BIT_DEPTH == 8
        		   	 memcpy(&POS(0, y), ref + idx + 1, size);
				 #else
				     memcpy(&POS(0, y), ref + idx + 1, 2*size);
        		 #endif
        		
        	}
        }

#endif
        if (mode == 26 && c_idx == 0 && size < 32) {
            for (y = 0; y < size; y++)
                POS(0, y) = av_clip_pixel(top[0] + ((left[y] - left[-1]) >> 1));
        }
    } else {
        ref = left - 1;
        if (angle < 0 && last < -1) {
         #ifndef REMOVE_COPY				
            for (x = 0; x <= size; x += 4)
                AV_WN4P(&ref_tmp[x], AV_RN4P(&left[x - 1]));
			/*
 			#if BIT_DEPTH == 8
				memcpy(ref_tmp, left -  1, size + 1);
            #else
				memcpy(ref_tmp, left -  1, 2*size + 2);	
			#endif
			*/			
            for (x = last; x <= -1; x++)
                ref_tmp[x] = top[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];
            ref = ref_tmp;
		 #else
            //for (x = 0; x <= size; x += 4)
            //    AV_WN4P(&ref_tmp[x], AV_RN4P(&left[x - 1]));		
            for (x = last; x <= -1; x++)
                ref[x] = top[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];
			//ref = ref_tmp;
			
		 #endif
        }

        for (x = 0; x < size; x++) {
            int idx  = ((x + 1) * angle) >> 5;
            int fact = ((x + 1) * angle) & 31;
            if (fact) {
                for (y = 0; y < size; y++) {
                    POS(x, y) = ((32 - fact) * ref[y + idx + 1] +
                                       fact  * ref[y + idx + 2] + 16) >> 5;
                }
            } else {
                for (y = 0; y < size; y++)
                    POS(x, y) = ref[y + idx + 1];
            }
        }
        if (mode == 10 && c_idx == 0 && size < 32) {
            for (x = 0; x < size; x += 4) {
                POS(x,     0) = av_clip_pixel(left[0] + ((top[x    ] - top[-1]) >> 1));
                POS(x + 1, 0) = av_clip_pixel(left[0] + ((top[x + 1] - top[-1]) >> 1));
                POS(x + 2, 0) = av_clip_pixel(left[0] + ((top[x + 2] - top[-1]) >> 1));
                POS(x + 3, 0) = av_clip_pixel(left[0] + ((top[x + 3] - top[-1]) >> 1));
            }
        }
    }
}

static void FUNC(pred_angular_0)(uint8_t *src, const uint8_t *top,
                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{
    FUNC(pred_angular)(src, top, left, stride, c_idx, mode, 1 << 2);
}

#ifndef ANGULAR_1_OPT 
static void FUNC(pred_angular_1)(uint8_t *src, const uint8_t *top,
                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{
    FUNC(pred_angular)(src, top, left, stride, c_idx, mode, 1 << 3);
}
#else
static void FUNC(pred_angular_1)(uint8_t *src, const uint8_t *top,
                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{
    int x, y;

    static const int intra_pred_angle[] = {
         32,  26,  21,  17, 13,  9,  5, 2, 0, -2, -5, -9, -13, -17, -21, -26, -32,
        -26, -21, -17, -13, -9, -5, -2, 0, 2,  5,  9, 13,  17,  21,  26,  32
    };
    static const int inv_angle[] = {
        -4096, -1638, -910, -630, -482, -390, -315, -256, -315, -390, -482,
        -630, -910, -1638, -4096
    };

    int angle = intra_pred_angle[mode - 2];
    pixel ref_array[3 * MAX_TB_SIZE + 4];
    pixel *ref_tmp = ref_array + 8;
    pixel *ref;
    int last = (8 * angle) >> 5;

    if (mode >= 18) {
        ref = top - 1;

        if (angle < 0 && last < -1) {
		  #ifndef REMOVE_COPY	
            for (x = 0; x <= 8; x += 4)
                AV_WN4P(&ref_tmp[x], AV_RN4P(&top[x - 1]));
            for (x = last; x <= -1; x++)
                ref_tmp[x] = left[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];
            ref = ref_tmp;
		  #else
            for (x = last; x <= -1; x++)
                ref[x] = left[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];		  
		  #endif	
        }

        if (angle == 0 || angle == 32)
        {
          for (y = 0; y < 8; y++) {
            int idx  = ((y + 1) * angle) >> 5;

	        #if BIT_DEPTH == 8
			  memcpy(&POS(0, y), ref + idx + 1, 8);
	        #else
			  memcpy(&POS(0, y), ref + idx + 1, 16);
	        #endif
          }	           
        }
		else
		{
          for (y = 0; y < 8; y++) {
        	int    idx  = ((y + 1) * angle) >> 5;
        	uint8_t fact = ((y + 1) * angle) & 31;
			uint8x8_t v0,v1,v2,v3,v6;
			uint16x8_t v4,v5;			
            if (fact) {
				  v0 = vdup_n_u8(32 - fact);
				  v1 = vld1_u8(ref + 0 + idx + 1);
				  v2 = vdup_n_u8(fact);
				  v3 = vld1_u8(ref + 0 + idx + 2);
				  v4 = vmull_u8(v0,v1);
				  v4 = vmlal_u8(v4, v2,v3);
				  v5 = vrshrq_n_u16(v4,5);
				  v6 = vmovn_u16(v5);
				  vst1_u8(&POS(0  , y), v6);

            } else {
	 	         #if BIT_DEPTH == 8
					memcpy(&POS(0, y), ref + idx + 1, 8);
		         #else
					memcpy(&POS(0, y), ref + idx + 1, 16);
		         #endif
            }
          }		
		}

        if (mode == 26 && c_idx == 0 ) {
            for (y = 0; y < 8; y++)
                POS(0, y) = av_clip_pixel(top[0] + ((left[y] - left[-1]) >> 1));
        }
    } else {
        ref = left - 1;
        if (angle < 0 && last < -1) {
		  #ifndef REMOVE_COPY		
            for (x = 0; x <= 8; x += 4)
                AV_WN4P(&ref_tmp[x], AV_RN4P(&left[x - 1]));		
            for (x = last; x <= -1; x++)
                ref_tmp[x] = top[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];
            ref = ref_tmp;
		  #else
            for (x = last; x <= -1; x++)
                ref[x] = top[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];		  
		  #endif
        }

        for (x = 0; x < 8; x++) {
            int idx  = ((x + 1) * angle) >> 5;
            int fact = ((x + 1) * angle) & 31;
            if (fact) {
                for (y = 0; y < 8; y++) {
                    POS(x, y) = ((32 - fact) * ref[y + idx + 1] +
                                       fact  * ref[y + idx + 2] + 16) >> 5;
                }
            } else {
                for (y = 0; y < 8; y++)
                    POS(x, y) = ref[y + idx + 1];
            }
        }
        if (mode == 10 && c_idx == 0 ) {
            for (x = 0; x < 8; x += 4) {
                POS(x,     0) = av_clip_pixel(left[0] + ((top[x    ] - top[-1]) >> 1));
                POS(x + 1, 0) = av_clip_pixel(left[0] + ((top[x + 1] - top[-1]) >> 1));
                POS(x + 2, 0) = av_clip_pixel(left[0] + ((top[x + 2] - top[-1]) >> 1));
                POS(x + 3, 0) = av_clip_pixel(left[0] + ((top[x + 3] - top[-1]) >> 1));
            }
        }
    }

}

#endif

#ifndef ANGULAR_2_OPT 
static void FUNC(pred_angular_2)(uint8_t *src, const uint8_t *top,
                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{
    FUNC(pred_angular)(src, top, left, stride, c_idx, mode, 1 << 4);
}
#else
static void FUNC(pred_angular_2)(uint8_t *src, const uint8_t *top,
                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{
    int x, y;

    static const int intra_pred_angle[] = {
         32,  26,  21,  17, 13,  9,  5, 2, 0, -2, -5, -9, -13, -17, -21, -26, -32,
        -26, -21, -17, -13, -9, -5, -2, 0, 2,  5,  9, 13,  17,  21,  26,  32
    };
    static const int inv_angle[] = {
        -4096, -1638, -910, -630, -482, -390, -315, -256, -315, -390, -482,
        -630, -910, -1638, -4096
    };

    int angle = intra_pred_angle[mode - 2];
    pixel ref_array[3 * MAX_TB_SIZE + 4];
    pixel *ref_tmp = ref_array + 16;
    pixel *ref;
    int last = (16 * angle) >> 5;

    if (mode >= 18) {
        ref = top - 1;
        if (angle < 0 && last < -1) {
		  #ifndef REMOVE_COPY
            for (x = 0; x <= 16; x += 4)
                AV_WN4P(&ref_tmp[x], AV_RN4P(&top[x - 1]));
            for (x = last; x <= -1; x++)
                ref_tmp[x] = left[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];
            ref = ref_tmp;
		  #else
            for (x = last; x <= -1; x++)
                ref[x] = left[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];		  
		  #endif
        }

        if (angle == 0 || angle == 32)
        {
          for (y = 0; y < 16; y++) {
            int idx  = ((y + 1) * angle) >> 5;

	        #if BIT_DEPTH == 8
				memcpy(&POS(0, y), ref + idx + 1, 16);
	        #else
				memcpy(&POS(0, y), ref + idx + 1, 32);
	        #endif
          }	           
        }
		else
		{
          for (y = 0; y < 16; y++) {
			int	 idx  = ((y + 1) * angle) >> 5;
			uint8_t fact = ((y + 1) * angle) & 31;
			uint8x8_t v0,v1,v2,v3,v6;
			uint16x8_t v4,v5;

            if (fact) {
				v0 = vdup_n_u8(32 - fact);
				v1 = vld1_u8(ref + 0 + idx + 1);
				v2 = vdup_n_u8(fact);
				v3 = vld1_u8(ref + 0 + idx + 2);
				v4 = vmull_u8(v0,v1);
				v4 = vmlal_u8(v4, v2,v3);
				v5 = vrshrq_n_u16(v4,5);
				v6 = vmovn_u16(v5);
				vst1_u8(&POS(0	, y), v6);


				v1 = vld1_u8(ref + 8 + idx + 1);
				v3 = vld1_u8(ref + 8 + idx + 2);
				v4 = vmull_u8(v0,v1);
				v4 = vmlal_u8(v4, v2,v3);
				v5 = vrshrq_n_u16(v4,5);
				v6 = vmovn_u16(v5);
				vst1_u8(&POS(8	, y), v6);				

            } else {
		           #if BIT_DEPTH == 8
					 memcpy(&POS(0, y), ref + idx + 1, 16);
		           #else
					 memcpy(&POS(0, y), ref + idx + 1, 32);
		           #endif

            }
          }		
		}

        if (mode == 26 && c_idx == 0 ) {
            for (y = 0; y < 16; y++)
                POS(0, y) = av_clip_pixel(top[0] + ((left[y] - left[-1]) >> 1));
        }
    } else {
        ref = left - 1;
        if (angle < 0 && last < -1) {
		  #ifndef REMOVE_COPY	
            for (x = 0; x <= 16; x += 4)
                AV_WN4P(&ref_tmp[x], AV_RN4P(&left[x - 1]));			
            for (x = last; x <= -1; x++)
                ref_tmp[x] = top[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];
            ref = ref_tmp;
		  #else
            for (x = last; x <= -1; x++)
                ref[x] = top[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];		  
		  #endif
        }

        for (x = 0; x < 16; x++) {
            int idx  = ((x + 1) * angle) >> 5;
            int fact = ((x + 1) * angle) & 31;
            if (fact) {
                for (y = 0; y < 16; y++) {
                    POS(x, y) = ((32 - fact) * ref[y + idx + 1] +
                                       fact  * ref[y + idx + 2] + 16) >> 5;
                }
            } else {
                for (y = 0; y < 16; y++)
                    POS(x, y) = ref[y + idx + 1];
            }
        }
        if (mode == 10 && c_idx == 0) {
            for (x = 0; x < 16; x += 4) {
                POS(x,     0) = av_clip_pixel(left[0] + ((top[x    ] - top[-1]) >> 1));
                POS(x + 1, 0) = av_clip_pixel(left[0] + ((top[x + 1] - top[-1]) >> 1));
                POS(x + 2, 0) = av_clip_pixel(left[0] + ((top[x + 2] - top[-1]) >> 1));
                POS(x + 3, 0) = av_clip_pixel(left[0] + ((top[x + 3] - top[-1]) >> 1));
            }
        }
    }

}

#endif

#ifndef ANGULAR_3_OPT
static void FUNC(pred_angular_3)(uint8_t *src, const uint8_t *top,
                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{
    FUNC(pred_angular)(src, top, left, stride, c_idx, mode, 1 << 5);
}
#else
static void FUNC(pred_angular_3)(uint8_t *src, const uint8_t *top,
                                 const uint8_t *left,
                                 ptrdiff_t stride, int c_idx, int mode)
{
    int x, y;
	

    static const int intra_pred_angle[] = {
         32,  26,  21,  17, 13,  9,  5, 2, 0, -2, -5, -9, -13, -17, -21, -26, -32,
        -26, -21, -17, -13, -9, -5, -2, 0, 2,  5,  9, 13,  17,  21,  26,  32
    };
    static const int inv_angle[] = {
        -4096, -1638, -910, -630, -482, -390, -315, -256, -315, -390, -482,
        -630, -910, -1638, -4096
    };

    int angle = intra_pred_angle[mode - 2];
    pixel ref_array[3 * MAX_TB_SIZE + 4];
    pixel *ref_tmp = ref_array + 32;
    pixel *ref;
    int last = (32 * angle) >> 5;

    if (mode >= 18) {
        ref = top - 1;
        if (angle < 0 && last < -1) {
		  #ifndef REMOVE_COPY	
            for (x = 0; x <= 32; x += 4)
                AV_WN4P(&ref_tmp[x], AV_RN4P(&top[x - 1]));           
            for (x = last; x <= -1; x++)
                ref_tmp[x] = left[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];
            ref = ref_tmp;
		  #else
            for (x = last; x <= -1; x++)
                ref[x] = left[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];		  
		  #endif
        }

        if (angle == 0 || angle == 32)
        {
          for (y = 0; y < 32; y++) {
            int idx  = ((y + 1) * angle) >> 5;

            #if BIT_DEPTH == 8
				memcpy(&POS(0, y), ref + idx + 1, 32);
            #else
				memcpy(&POS(0, y), ref + idx + 1, 64);
            #endif
          }	           
        }
		else
		{
          for (y = 0; y < 32; y++) {
			int  idx	= ((y + 1) * angle) >> 5;
			uint8_t fact = ((y + 1) * angle) & 31;
			uint8x8_t v0,v1,v2,v3,v6;
			uint16x8_t v4,v5;

            if (fact) {
				v0 = vdup_n_u8(32 - fact);
				v1 = vld1_u8(ref + 0 + idx + 1);
				v2 = vdup_n_u8(fact);
				v3 = vld1_u8(ref + 0 + idx + 2);
				v4 = vmull_u8(v0,v1);
				v4 = vmlal_u8(v4, v2,v3);
				v5 = vrshrq_n_u16(v4,5);
				v6 = vmovn_u16(v5);
				vst1_u8(&POS(0	, y), v6);


				v1 = vld1_u8(ref + 8 + idx + 1);
				v3 = vld1_u8(ref + 8 + idx + 2);
				v4 = vmull_u8(v0,v1);
				v4 = vmlal_u8(v4, v2,v3);
				v5 = vrshrq_n_u16(v4,5);
				v6 = vmovn_u16(v5);
				vst1_u8(&POS(8	, y), v6);	

				v1 = vld1_u8(ref + 16 + idx + 1);
				v3 = vld1_u8(ref + 16 + idx + 2);
				v4 = vmull_u8(v0,v1);
				v4 = vmlal_u8(v4, v2,v3);
				v5 = vrshrq_n_u16(v4,5);
				v6 = vmovn_u16(v5);
				vst1_u8(&POS(16	, y), v6);
				
				v1 = vld1_u8(ref + 24 + idx + 1);
				v3 = vld1_u8(ref + 24 + idx + 2);
				v4 = vmull_u8(v0,v1);
				v4 = vmlal_u8(v4, v2,v3);
				v5 = vrshrq_n_u16(v4,5);
				v6 = vmovn_u16(v5);
				vst1_u8(&POS(24	, y), v6);

            } else {
                #if BIT_DEPTH == 8
					memcpy(&POS(0, y), ref + idx + 1, 32);
                #else
					memcpy(&POS(0, y), ref + idx + 1, 64);
                #endif

            }
          }		
		}

		/*
        if (mode == 26 && c_idx == 0 && size < 32) {
            for (y = 0; y < size; y++)
                POS(0, y) = av_clip_pixel(top[0] + ((left[y] - left[-1]) >> 1));
        }
        */
    } else {
        ref = left - 1;
        if (angle < 0 && last < -1) {
		  #ifndef REMOVE_COPY	
            for (x = 0; x <= 32; x += 4)
                AV_WN4P(&ref_tmp[x], AV_RN4P(&left[x - 1]));          
            for (x = last; x <= -1; x++)
                ref_tmp[x] = top[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];
            ref = ref_tmp;
		  #else
            for (x = last; x <= -1; x++)
                ref[x] = top[-1 + ((x * inv_angle[mode - 11] + 128) >> 8)];		  
		  #endif
        }

        for (x = 0; x < 32; x++) {
            int idx  = ((x + 1) * angle) >> 5;
            int fact = ((x + 1) * angle) & 31;
            if (fact) {
                for (y = 0; y < 32; y++) {
                    POS(x, y) = ((32 - fact) * ref[y + idx + 1] +
                                       fact  * ref[y + idx + 2] + 16) >> 5;
                }
            } else {
                for (y = 0; y < 32; y++)
                    POS(x, y) = ref[y + idx + 1];
            }
        }
		/*
        if (mode == 10 && c_idx == 0 && size < 32) {
            for (x = 0; x < size; x += 4) {
                POS(x,     0) = av_clip_pixel(left[0] + ((top[x    ] - top[-1]) >> 1));
                POS(x + 1, 0) = av_clip_pixel(left[0] + ((top[x + 1] - top[-1]) >> 1));
                POS(x + 2, 0) = av_clip_pixel(left[0] + ((top[x + 2] - top[-1]) >> 1));
                POS(x + 3, 0) = av_clip_pixel(left[0] + ((top[x + 3] - top[-1]) >> 1));
            }
        }
        */
    }

}

#endif

#undef EXTEND_LEFT_CIP
#undef EXTEND_RIGHT_CIP
#undef EXTEND_UP_CIP
#undef EXTEND_DOWN_CIP
#undef IS_INTRA
#undef MVF_PU
#undef MVF
#undef PU
#undef EXTEND
#undef MIN_TB_ADDR_ZS
#undef POS
#undef SMOOTH_OPT
#undef EXTEND_SIMD_OPT
#undef ANGULAR_OPT
#undef ANGULAR_1_OPT
#undef ANGULAR_2_OPT
#undef ANGULAR_3_OPT
#undef REMOVE_COPY
#undef DC_OPT




