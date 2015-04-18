#include "get_bits.h"
#include "hevc.h"

#include "bit_depth_template.c"
#include "hevcdsp.h"
#include "hevc_log.h"

#if HAVE_NEON_INLINE
 #define MC_OPT
 #ifdef MC_OPT
    #define LUMA_UNI_HOR_OPT
 #endif

 #ifdef MC_OPT
    #define LUMA_UNI_VER_OPT
 #endif

 #ifdef MC_OPT
    #define LUMA_UNI_HV_OPT
 #endif 

 #ifdef MC_OPT
    #define CHROMA_UNI_HOR_OPT
 #endif

 #ifdef MC_OPT
    #define CHROMA_UNI_VER_OPT
 #endif

 #ifdef MC_OPT
    #define CHROMA_UNI_HV_OPT
 #endif

 #ifdef MC_OPT
    #define PUT_PEL_BI_OPT
 #endif   

 #ifdef MC_OPT
    #define PUT_PEL_OPT
 #endif

 #ifdef MC_OPT
    #define LUMA_HOR_OPT
 #endif

 #ifdef MC_OPT
    #define LUMA_VER_OPT
 #endif 

 #ifdef MC_OPT
    #define LUMA_HV_OPT
 #endif 

 #ifdef MC_OPT
    #define PUT_QPEL_HOR_BI_OPT
 #endif   

 #ifdef MC_OPT
    #define PUT_QPEL_VER_BI_OPT
 #endif   

 #ifdef MC_OPT
    #define PUT_QPEL_HV_BI_OPT
 #endif

 #ifdef MC_OPT
    #define PUT_EPEL_HOR_OPT
 #endif   

 #ifdef MC_OPT
    #define PUT_EPEL_VER_OPT
 #endif   

 #ifdef MC_OPT
    #define PUT_EPEL_HV_OPT
 #endif  

 #ifdef MC_OPT
    #define PUT_EPEL_HOR_BI_OPT
 #endif   

 #ifdef MC_OPT
    #define PUT_EPEL_VER_BI_OPT
 #endif   

 #ifdef MC_OPT
    #define PUT_EPEL_HV_BI_OPT
 #endif 
 
#endif


void put_hevc_qpel_uni_h_opt_8(uint8_t *_dst,  ptrdiff_t _dststride,
                                      uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width);

void put_hevc_qpel_uni_v_opt_8(uint8_t *_dst,  ptrdiff_t _dststride,
							          uint8_t *_src, ptrdiff_t _srcstride,
							          int height, intptr_t mx, intptr_t my, int width);

void put_hevc_qpel_uni_hv_opt_8(uint8_t *_dst,  ptrdiff_t _dststride,
									  uint8_t *_src, ptrdiff_t _srcstride,
									  int height, intptr_t mx, intptr_t my, int width);

void put_hevc_epel_uni_h_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
									 int height, intptr_t mx, intptr_t my, int width);

void put_hevc_epel_uni_v_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
								     int height, intptr_t mx, intptr_t my, int width);

void put_hevc_epel_uni_hv_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
									 int height, intptr_t mx, intptr_t my, int width);

void put_hevc_pel_bi_pixels_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
									 int16_t *src2, int height, intptr_t mx, intptr_t my, int width);

void init_put_hevc_pel_bi_pixels_opt_8(void);

void put_hevc_pel_pixels_opt_8(int16_t *dst,
                                    uint8_t *_src, ptrdiff_t _srcstride,
                                    int height, intptr_t mx, intptr_t my, int width);

void put_hevc_qpel_h_opt_8(int16_t *_dst,
						   uint8_t *_src, ptrdiff_t _srcstride,
						   int height, intptr_t mx, intptr_t my, int width);

void put_hevc_qpel_v_opt_8(int16_t *_dst,
							uint8_t *_src, ptrdiff_t _srcstride,
							int height, intptr_t mx, intptr_t my, int width);

void put_hevc_qpel_hv_opt_8(int16_t *dst,
							uint8_t *_src,
							ptrdiff_t _srcstride,
							int height, intptr_t mx,
							intptr_t my, int width);

void put_hevc_qpel_bi_h_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
							  int16_t *src2,
							  int height, intptr_t mx, intptr_t my, int width);

void put_hevc_qpel_bi_v_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
							  int16_t *src2,
							  int height, intptr_t mx, intptr_t my, int width);

void put_hevc_qpel_bi_hv_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
							   int16_t *src2,
							   int height, intptr_t mx, intptr_t my, int width);

void put_hevc_epel_h_opt_8(int16_t *_dst, uint8_t *_src, ptrdiff_t _srcstride,
                                      int height, intptr_t mx, intptr_t my, int width);

void put_hevc_epel_v_opt_8(int16_t *_dst, uint8_t *_src, ptrdiff_t _srcstride,
								int height, intptr_t mx, intptr_t my, int width);

void put_hevc_epel_hv_opt_8(int16_t *_dst, uint8_t *_src, ptrdiff_t _srcstride,
							int height, intptr_t mx, intptr_t my, int width);


void put_hevc_epel_bi_h_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int16_t *src2,
                              int height, intptr_t mx, intptr_t my, int width);


void put_hevc_epel_bi_v_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
							  int16_t *_src2,
							  int height, intptr_t mx, intptr_t my, int width);

void put_hevc_epel_bi_hv_opt_8(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
							   int16_t *src2,
							   int height, intptr_t mx, intptr_t my, int width);

















