#include "libavutil/arm/asm.S"
#include "neon.S"
     .macro        epel_h_load @At this macro, the d0~d5 are occupied 
        pld       [r2]
        vld1.8    {d0,d1}, [r2], r3
        vext.8    d2, d0, d1,  #2        
        vext.8    d3, d0, d1,  #3
        vext.8    d1, d0, d1,  #1              
     .endm
     
     .macro        epel_filter @At this macro, the d6~d9 are occupied, the q3[d6-d7] are the result      
        vmull.u8  q3, d1, d17
        vmull.u8  q4, d2, d18           
     	vmlsl.u8  q3, d0, d16
     	vmlsl.u8  q4, d3, d19    	
     	vadd.i16  q3,q3,q4
     .endm	   
     
     .macro        load_coeff_epel mv_reg ,lidx
         
        @d16, d17, d18 ,d19 are the coefficients
        sub    \mv_reg, \mv_reg, #1
        @suppose the mx is less than 1, we do not load any coeffs and return
        blt    load_end\lidx
        cmp    r8, #5
        addls  pc, pc , r8, lsl #2
        b    load_coeff7\lidx
        b    load_coeff1\lidx
        b    load_coeff2\lidx             
        b    load_coeff3\lidx        
        b    load_coeff4\lidx
        b    load_coeff5\lidx             
        b    load_coeff6\lidx         
        
load_coeff1\lidx: 
 	    vmov.i8 d16, #2
	    vmov.i8 d17, #58
	    vmov.i8 d18, #10
	    vmov.i8 d19, #2  
	    b load_end\lidx    
load_coeff2\lidx: 
 	    vmov.i8 d16, #4
	    vmov.i8 d17, #54
	    vmov.i8 d18, #16
	    vmov.i8 d19, #2  
	    b load_end\lidx   	              
load_coeff3\lidx: 
 	    vmov.i8 d16, #6
	    vmov.i8 d17, #46
	    vmov.i8 d18, #28
	    vmov.i8 d19, #4  
	    b load_end\lidx    
load_coeff4\lidx: 
 	    vmov.i8 d16, #4
	    vmov.i8 d17, #36
	    vmov.i8 d18, #36
	    vmov.i8 d19, #4  
	    b load_end\lidx   	 
load_coeff5\lidx: 
 	    vmov.i8 d16, #4
	    vmov.i8 d17, #28
	    vmov.i8 d18, #46
	    vmov.i8 d19, #6  
	    b load_end\lidx    
load_coeff6\lidx: 
 	    vmov.i8 d16, #2
	    vmov.i8 d17, #16
	    vmov.i8 d18, #54
	    vmov.i8 d19, #4  
	    b load_end\lidx   	 
load_coeff7\lidx: 
 	    vmov.i8 d16, #2
	    vmov.i8 d17, #10
	    vmov.i8 d18, #58
	    vmov.i8 d19, #2 
load_end\lidx:
	 .endm
	 
	 .macro	    store_unalingned_byte width, result, pdst, pdststride, lidx 
	    cmp \width , #4
	    bne store_unalingnedb_flag0\lidx 
	    vst1.i32 \result[0], [\pdst]!
	    b store_unalingnedbend\lidx	   
store_unalingnedb_flag0\lidx:	     
	    vst1.i16 \result[0], [\pdst]! 
	    cmp \width, #2
	    beq  store_unalingnedbend\lidx	    
	    vst1.i16 \result[1], [\pdst]! 
	    vst1.i16 \result[2], [\pdst]!     		    
store_unalingnedbend\lidx:
	    add \pdst, \pdststride
	    sub \pdst, \width
	    	     
	 .endm
	 
	 .macro	    store_unalingned_word width, result0, result1, pdst, pdststride, lidx 
	    cmp \width , #4
	    bne store_unalingnedw_flag0\lidx 
	    vst1.s16 \result0, [\pdst]! 
	    b store_unalingnedwend\lidx	   
store_unalingnedw_flag0\lidx:	     
	    vst1.s32 \result0[0], [\pdst]! 
	    cmp \width, #2
	    beq  store_unalingnedwend\lidx	    
	    vst1.s32 \result0[1], [\pdst]! 
	    vst1.s32 \result1[0], [\pdst]!     		    
store_unalingnedwend\lidx:
	    add \pdst, \pdststride
	    sub \pdst, \pdst, \width, lsl #1
	    	     
	 .endm	 
     
     //.global  neon_put_hevc_epel_uni_h_opt_8        
     //.global  neon_put_hevc_epel_h_opt_8 
     //.global  neon_put_hevc_epel_bi_h_opt_8            
        
function neon_put_hevc_epel_uni_h_opt_8 export=1
        @r0 dst, r1 dst_stride, r2 src, r3 src_stride
        push   {r4-r10}
        @int height, intptr_t mx, intptr_t my, int width
        ldr    r5, [sp, #40] @ width
        ldr    r4, [sp, #28] @ height
        ldr    r8, [sp, #32] @ mx
    
        load_coeff_epel r8, l1
            	
        sub       r2, #1
        
        cmp       r5 ,#8
        blt       loop1_1
        
    	mov       r9, r4 @r9 is used to store height
        mov       r6, r0 @r6 is used to store _dst
        mov       r7, r2 @r7 is used to store _src               
loop0_1: 
        subs      r4, #1 @inner loop at y direction
        epel_h_load
        epel_filter
        vqrshrun.s16   d10, q3, #6
        vst1.8    d10, [r0], r1
        bne       loop0_1
        sub       r5, #8
        cmp       r5, #8

        mov       r4, r9 @restore height
        add       r6, #8
        mov       r0, r6 @step dst
        add       r7, #8
        mov       r2, r7 @step src
        blt       loop1_1      
        b         loop0_1         

loop1_1: 
        cmp r5, #0
        beq end_1           
        epel_h_load
        epel_filter
        vqrshrun.s16   d10, q3, #6
        @$width, $result, $pdst, $pdststride, $lidx 
        store_unalingned_byte r5, d10, r0, r1, l1
        subs      r4, #1 @inner loop at y direction
        bne       loop1_1

end_1:        
        pop      {r4-r10}
        bx lr
endfunc        
        
function neon_put_hevc_epel_h_opt_8 export=1
        @r0 dst, r1 dst_stride, r2 src, r3 src_stride
        push   {r4-r10}
        @int height, intptr_t mx, intptr_t my, int width
        ldr    r5, [sp, #40] @ width
        ldr    r4, [sp, #28] @ height
        ldr    r8, [sp, #32] @ mx
    
        load_coeff_epel r8, l2
            	
        sub       r2, #1
        
        cmp       r5 ,#8
        blt       loop1_2
        
    	mov       r9, r4 @r9 is used to store height
        mov       r6, r0 @r6 is used to store _dst
        mov       r7, r2 @r7 is used to store _src               
loop0_2: 
        subs      r4, #1 @inner loop at y direction
        epel_h_load
        epel_filter
        vst1.16   {q3}, [r0], r1
        bne       loop0_2
        sub       r5, #8
        cmp       r5, #8

        mov       r4, r9 @restore height
        add       r6, #16
        mov       r0, r6 @step dst
        add       r7, #8
        mov       r2, r7 @step src
        blt       loop1_2      
        b         loop0_2         

loop1_2:    
        cmp r5, #0
        beq end_2         
        epel_h_load
        epel_filter
        @$width, $result, $pdst, $pdststride, $lidx 
        store_unalingned_word r5, d6, d7, r0, r1, l2
        subs      r4, #1 @inner loop at y direction
        bne       loop1_2

end_2:        
        pop      {r4-r10}
        bx lr                           
endfunc        
        
function neon_put_hevc_epel_bi_h_opt_8 export=1
        @r0 dst, r1 dst_stride, r2 src, r3 src_stride
        push   {r4-r12}
        @int height, intptr_t mx, intptr_t my, int width
        ldr    r5, [sp, #52]  @ width
        ldr    r4, [sp, #40]  @ height
        ldr    r8, [sp, #44]  @ mx
        ldr    r10, [sp, #36] @ src2        
    
        @set the r12 to be step of MAX_PB_SIZE words 
        mov    r12,  #128
        load_coeff_epel r8, l3
            	
        sub       r2, #1
        
        cmp       r5 ,#8
        blt       loop1_3
        
    	mov       r9, r4 @r9 is used to store height
        mov       r6, r0 @r6 is used to store _dst
        mov       r7, r2 @r7 is used to store _src    
        mov       r11, r10 @r11 is used to store _src             
loop0_3: 
        subs      r4, #1 @inner loop at y direction
        vld1.16     {q5}, [r10], r12
        epel_h_load
        epel_filter
        vqaddq.s16  q3, q5        
        vqrshrun.s16   d10, q3, #7
        vst1.8    d10, [r0], r1
        bne       loop0_3
        sub       r5, #8
        cmp       r5, #8

        mov       r4, r9 @restore height
        add       r6, #8
        mov       r0, r6 @step dst
        add       r7, #8
        mov       r2, r7 @step src
        add       r11, #16
        mov       r10, r11 @step src       
        blt       loop1_3      
        b         loop0_3         

loop1_3: 
        cmp r5, #0
        beq end_3     
        vld1.16     {q5}, [r10], r12      
        epel_h_load
        epel_filter
        vqaddq.s16  q3, q5
        vqrshrun.s16   d10, q3, #7
        @$width, $result, $pdst, $pdststride, $lidx 
        store_unalingned_byte r5, d10, r0, r1, l3
        subs      r4, #1 @inner loop at y direction
        bne       loop1_3

end_3:        
        pop      {r4-r12}
        bx lr
endfunc        
        
