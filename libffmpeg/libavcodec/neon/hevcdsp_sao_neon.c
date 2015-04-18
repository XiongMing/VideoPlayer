#include "config.h"
#include "libavutil/avassert.h"
#include "libavutil/pixdesc.h"
#include "libavcodec/get_bits.h"
#include "libavcodec/hevc.h"

#include <arm_neon.h>

void hevc_sao_band_filter_0_8_neon( uint8_t *_dst, uint8_t *_src, ptrdiff_t _stride_dst, ptrdiff_t _stride_src,struct SAOParams *sao,
		int *borders, int width, int height, int c_idx);

void hevc_sao_edge_filter_0_8_neon(uint8_t *_dst, uint8_t *_src,                                           
        ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,    
        int *borders, int _width, int _height, int c_idx, uint8_t *vert_edge,   
        uint8_t *horiz_edge, uint8_t *diag_edge);
		
void hevc_sao_edge_filter_1_8_neon(uint8_t *_dst, uint8_t *_src,                                           
        ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,    
        int *borders, int _width, int _height, int c_idx, uint8_t *vert_edge,   
        uint8_t *horiz_edge, uint8_t *diag_edge);

#define SAO_INIT_8()                                                           \
    uint8_t  *dst       = _dst;                                                \
    uint8_t  *src       = _src;                                                \
    ptrdiff_t stride_dst= _stride_dst;                                         \
    ptrdiff_t stride_src= _stride_src

#define SAO_BAND_FILTER_INIT()                                                 \
    r0   = vdupq_n_u16 ((sao_left_class    ) & 31);							   \
    r1   = vdupq_n_u16 ((sao_left_class+1  ) & 31);							   \
    r2   = vdupq_n_u16 ((sao_left_class+2  ) & 31);							   \
    r3   = vdupq_n_u16 ((sao_left_class+3  ) & 31);							   \
    sao1 = vdupq_n_u16 (sao_offset_val[1]);                                  \
    sao2 = vdupq_n_u16 (sao_offset_val[2]);                                  \
    sao3 = vdupq_n_u16 (sao_offset_val[3]);                                  \
    sao4 = vdupq_n_u16 (sao_offset_val[4])


#define SAO_BAND_FILTER_LOAD_8(x)                                              \
	src0 = vmovl_u8 (vld1_u8 (&src[x]));									   \
	src2 = vshrq_n_u16 (src0, 3); 


#define SAO_BAND_FILTER_COMPUTE() \
    x0   = vceqq_u16 (src2, r0);                                          \
    x1   = vceqq_u16 (src2, r1);                                          \
    x2   = vceqq_u16 (src2, r2);                                          \
    x3   = vceqq_u16 (src2, r3);                                          \
    x0   = vandq_u16 (x0, sao1);                                            \
    x1   = vandq_u16 (x1, sao2);                                            \
    x2   = vandq_u16 (x2, sao3);                                            \
    x3   = vandq_u16 (x3, sao4);                                            \
    x0   = vorrq_u16 (x0, x1);                                               \
    x2   = vorrq_u16 (x2, x3);                                               \
    x0   = vorrq_u16 (x0, x2);                                               \
    src0 = vreinterpretq_u16_s16(vaddq_s16 (vreinterpretq_s16_u16(src0), vreinterpretq_s16_u16(x0)))


#define SAO_BAND_FILTER_STORE_8()                                              \
    src1 =  vqmovun_s16(vreinterpretq_s16_u16(src0));                                       \
    vst1_u8(&dst[x], src1)

void hevc_sao_band_filter_0_8_neon( uint8_t *_dst, uint8_t *_src, ptrdiff_t _stride_dst, ptrdiff_t _stride_src,struct SAOParams *sao,
		int *borders, int width, int height, int c_idx) {                      
    int y, x;                                                                                                              
    int16_t *sao_offset_val = sao->offset_val[c_idx];                          
    uint8_t  sao_left_class = sao->band_position[c_idx];                       
    uint16x8_t r0, r1, r2, r3, x0, x1, x2, x3, sao1, sao2, sao3, sao4;            
    uint16x8_t src0, src2;
    uint8x8_t src1;
    SAO_INIT_8();                                                          
    SAO_BAND_FILTER_INIT();                                                    
    for (y = 0; y < height; y++) {                                             
        for (x = 0; x < width; x += 8) {                                       
            SAO_BAND_FILTER_LOAD_8(x);                                      
            SAO_BAND_FILTER_COMPUTE();                                         
            SAO_BAND_FILTER_STORE_8();                                      
        }                                                                      
        dst += stride_dst;                                                     
        src += stride_src;                                                     
    }                                                                          
}

#define SAO_EDGE_FILTER_INIT()                                                 \
    int y_stride_src = init_y * stride_src;                                    \
    int y_stride_dst = init_y * stride_dst;                                    \
    int pos_0_0 = pos[sao_eo_class][0][0];                                     \
    int pos_0_1 = pos[sao_eo_class][0][1];                                     \
    int pos_1_0 = pos[sao_eo_class][1][0];                                     \
    int pos_1_1 = pos[sao_eo_class][1][1];                                     \
    int y_stride_0_1 = (init_y + pos_0_1) * stride_src + pos_0_0;              \
    int y_stride_1_1 = (init_y + pos_1_1) * stride_src + pos_1_0;              \
    offset0 = vdupq_n_s16(sao_offset_val[edge_idx[0]]);                     \
    offset1 = vdupq_n_s16(sao_offset_val[edge_idx[1]]);                     \
    offset2 = vdupq_n_s16(sao_offset_val[edge_idx[2]]);                     \
    offset3 = vdupq_n_s16(sao_offset_val[edge_idx[3]]);                     \
    offset4 = vdupq_n_s16(sao_offset_val[edge_idx[4]])

#define SAO_EDGE_FILTER_LOAD_8()                                             	\
	x0   = vreinterpretq_s16_u16(vmovl_u8(vreinterpret_u8_s8(vld1_s8(src + x + y_stride_src))));		\
	cmp0 = vreinterpretq_s16_u16(vmovl_u8(vreinterpret_u8_s8(vld1_s8(src + x + y_stride_0_1))));		\
	cmp1 = vreinterpretq_s16_u16(vmovl_u8(vreinterpret_u8_s8(vld1_s8(src + x + y_stride_1_1))))


#define SAO_EDGE_FILTER_COMPUTE()												\
    r2 = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(x0), vreinterpretq_u16_s16(cmp0)));		\
    x1 = vreinterpretq_s16_u16(vceqq_s16 (cmp0, r2));                                            \
    x2 = vreinterpretq_s16_u16(vceqq_s16 (x0, r2));                                              \
    x1 = vsubq_s16(x2, x1);                                                \
    r2 = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(x0), vreinterpretq_u16_s16(cmp1)));                                              \
    x3 = vreinterpretq_s16_u16(vceqq_s16 (cmp1, r2));                                            \
    x2 = vreinterpretq_s16_u16(vceqq_s16 (x0, r2));                                              \
    x3 = vsubq_s16(x2, x3);                                                \
    x1 = vaddq_s16(x1, x3);                                                \
    r0 = vreinterpretq_s16_u16(vceqq_s16 (x1, vdupq_n_s16(-2)));                              \
    r1 = vreinterpretq_s16_u16(vceqq_s16 (x1, vdupq_n_s16(-1)));                              \
    r2 = vreinterpretq_s16_u16(vceqq_s16 (x1, vdupq_n_s16(0)));                               \
    r3 = vreinterpretq_s16_u16(vceqq_s16 (x1, vdupq_n_s16(1)));                               \
    r4 = vreinterpretq_s16_u16(vceqq_s16 (x1, vdupq_n_s16(2)));                               \
    r0 = vandq_s16 (r0, offset0);                                           \
    r1 = vandq_s16 (r1, offset1);                                           \
    r2 = vandq_s16 (r2, offset2);                                           \
    r3 = vandq_s16 (r3, offset3);                                           \
    r4 = vandq_s16 (r4, offset4);                                           \
    r0 = vaddq_s16(r0, r1);                                                \
    r2 = vaddq_s16(r2, r3);                                                \
    r0 = vaddq_s16(r0, r4);                                                \
    r0 = vaddq_s16(r0, r2);                                                \
    r0 = vaddq_s16(r0, x0)

#define SAO_EDGE_FILTER_STORE_8()                                              \
	vst1_u8(dst + x + y_stride_dst,vqmovun_s16(r0))

#define SAO_EDGE_FILTER_BORDER_LOOP_8(incr_dst, incr_src)                      \
    tx1 = vdup_n_s8(sao_offset_val[0]);                                     \
    for (x = 0; x < width; x += 8) {                                           \
        tx0 = vld1_s8((src + incr_src));                    \
        tx0 = vadd_s8(tx0, tx1);                                             \
        vst1_s8((dst + incr_dst), tx0);                    \
    }

static void hevc_sao_edge_filter_8_neon(              
        uint8_t *_dst, uint8_t *_src,                                           
        ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,    
        int *borders, int _width, int _height, int c_idx, uint8_t *vert_edge,   
        uint8_t *horiz_edge, uint8_t *diag_edge) {                              
    int x, y;                                                                   
    int16_t *sao_offset_val = sao->offset_val[c_idx];                           
    int  sao_eo_class   = sao->eo_class[c_idx];                                 
    const uint8_t edge_idx[]  = { 1, 2, 0, 3, 4 };                              
    const int8_t pos[4][2][2] = {                                               
        { {-1, 0}, { 1, 0} }, /* horizontal */                                  
        { { 0,-1}, { 0, 1} }, /* vertical   */                                  
        { {-1,-1}, { 1, 1} }, /* 45 degree  */                                  
        { { 1,-1}, {-1, 1} }, /* 135 degree */                                  
    };                                                                          
    int init_y = 0, width = _width, height = _height;                           
    int16x8_t x0, x1, x2, x3, offset0, offset1, offset2, offset3, offset4;        
    int16x8_t cmp0, cmp1, r0, r1, r2, r3, r4;
    int8x8_t tx0,tx1;
    SAO_INIT_8();                                                           
    if (sao_eo_class != SAO_EO_HORIZ) {                                         
        if (borders[1]) {                                                       
            SAO_EDGE_FILTER_BORDER_LOOP_8(x, x);                            
            init_y = 1;                                                         
        }                                                                       
        if (borders[3]) {                                                       
            int y_stride_dst = stride_dst * (_height - 1);                      
            int y_stride_src = stride_src * (_height - 1);                      
            SAO_EDGE_FILTER_BORDER_LOOP_8(x + y_stride_dst, x + y_stride_src); 
            height--;                                                           
        }                                                                       
    }                                                                           
    {                                                                           
        SAO_EDGE_FILTER_INIT();                                                 
        for (y = init_y; y < height; y++) {                                     
            for (x = 0; x < width; x += 8) {                                    
                SAO_EDGE_FILTER_LOAD_8();                                   
                SAO_EDGE_FILTER_COMPUTE();
                SAO_EDGE_FILTER_STORE_8();                                  
            }                                                                   
            y_stride_dst += stride_dst;                                         
            y_stride_src += stride_src;                                         
            y_stride_0_1 += stride_src;                                         
            y_stride_1_1 += stride_src;                                         
        }                                                                       
    }                                                                           
    if (sao_eo_class != SAO_EO_VERT) {                                          
        if (borders[0]) {                                                       
            int idx_src        = 0;                                             
            int idx_dst        = 0;                                             
            int16_t offset_val = sao_offset_val[0];                             
            for (y = 0; y < height; y++) {                                      
                dst[idx_dst] = av_clip_uintp2(src[idx_src] + offset_val, 8);    
                idx_dst     += stride_dst;                                      
                idx_src     += stride_src;                                      
            }                                                                   
        }                                                                       
        if (borders[2]) {                                                       
            int idx_src        = _width - 1;                                    
            int idx_dst        = idx_src;                                       
            int16_t offset_val = sao_offset_val[0];                             
            for (y = 0; y < height; y++) {                                      
                dst[idx_dst] = av_clip_uintp2(src[idx_src] + offset_val, 8);    
                idx_dst     += stride_dst;                                      
                idx_src     += stride_src;                                      
            }                                                                   
        }                                                                       
    }                                                                           
}


void hevc_sao_edge_filter_0_8_neon(uint8_t *_dst, uint8_t *_src,                                           
        ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,    
        int *borders, int _width, int _height, int c_idx, uint8_t *vert_edge,   
        uint8_t *horiz_edge, uint8_t *diag_edge)
{
	 hevc_sao_edge_filter_8_neon(_dst, _src, _stride_dst, _stride_src, \
            sao, borders, _width, _height, c_idx,                              \
            vert_edge, horiz_edge, diag_edge);
}

void hevc_sao_edge_filter_1_8_neon(uint8_t *_dst, uint8_t *_src,      
        ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,   
        int *borders, int _width, int _height, int c_idx,                      
        uint8_t *vert_edge, uint8_t *horiz_edge,                               
        uint8_t *diag_edge) {                                                  
    int x, y;                                                                  
    int  sao_eo_class   = sao->eo_class[c_idx];                                
    int init_x = 0, init_y = 0, width = _width, height = _height;              
    int save_upper_left  = !diag_edge[0] && sao_eo_class == SAO_EO_135D && !borders[0] && !borders[1];
    int save_upper_right = !diag_edge[1] && sao_eo_class == SAO_EO_45D  && !borders[1] && !borders[2];
    int save_lower_right = !diag_edge[2] && sao_eo_class == SAO_EO_135D && !borders[2] && !borders[3];
    int save_lower_left  = !diag_edge[3] && sao_eo_class == SAO_EO_45D  && !borders[0] && !borders[3];
    SAO_INIT_8();                                                          
    hevc_sao_edge_filter_8_neon(_dst, _src, _stride_dst, _stride_src, 
            sao, borders, _width, _height, c_idx,                              
            vert_edge, horiz_edge, diag_edge);                                 
    if (sao_eo_class != SAO_EO_HORIZ) {                                        
        if (borders[1])                                                        
            init_y = 1;                                                        
        if (borders[3])                                                        
            height--;                                                          
    }                                                                          
    if (sao_eo_class != SAO_EO_VERT) {                                         
        if (borders[0])                                                        
            init_x = 1;                                                        
        if (borders[2])                                                        
            width--;                                                           
    }                                                                          
    /* Restore pixels that can't be modified */                                
    if(vert_edge[0] && sao_eo_class != SAO_EO_VERT)                            
        for(y = init_y + save_upper_left; y < height - save_lower_left; y++)   
            dst[y * _stride_dst] = src[y * _stride_src];                       
    if(vert_edge[1] && sao_eo_class != SAO_EO_VERT)                            
        for(y = init_y + save_upper_right; y < height - save_lower_right; y++) 
            dst[y*_stride_dst + width - 1] = src[y * _stride_src + width - 1]; 
    if(horiz_edge[0] && sao_eo_class != SAO_EO_HORIZ)                          
        for(x = init_x + save_upper_left; x < width - save_upper_right; x++)   
            dst[x] = src[x];                                                   
    if(horiz_edge[1] && sao_eo_class != SAO_EO_HORIZ)                          
        for(x = init_x + save_lower_left; x < width - save_lower_right; x++)   
            dst[(height - 1) * _stride_dst + x] =                              
                src[(height - 1) * _stride_src + x];                           
    if(diag_edge[0] && sao_eo_class == SAO_EO_135D)                            
        dst[0] = src[0];                                                       
    if(diag_edge[1] && sao_eo_class == SAO_EO_45D)                            
        dst[width - 1] = src[width - 1];                                       
    if(diag_edge[2] && sao_eo_class == SAO_EO_135D)                            
        dst[_stride_dst*(height-1) + width-1] =                                
            src[_stride_src * (height-1) + width-1];                           
    if(diag_edge[3] && sao_eo_class == SAO_EO_45D)                             
        dst[_stride_dst * (height - 1)] = src[_stride_src * (height - 1)];     
}

