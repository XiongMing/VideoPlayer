#include "libavutil/arm/asm.S"
#include "neon.S"


.macro  CMPUTE_O_K  regdst
        vld1.16  {d0, d1, d2, d3}, [r12]!
        vmull.s16 \regdst, d4, d0[0]
        vmlal.s16 \regdst, d5, d0[1]
        vmlal.s16 \regdst, d6, d0[2]
        vmlal.s16 \regdst, d7, d0[3]
        vmlal.s16 \regdst, d8,  d1[0]
        vmlal.s16 \regdst, d9,  d1[1]
        vmlal.s16 \regdst, d10, d1[2]
        vmlal.s16 \regdst, d11, d1[3]
        vmlal.s16 \regdst, d12, d2[0]
        vmlal.s16 \regdst, d13, d2[1]
        vmlal.s16 \regdst, d14, d2[2]
        vmlal.s16 \regdst, d15, d2[3]
        vmlal.s16 \regdst, d16, d3[0]
        vmlal.s16 \regdst, d17, d3[1]
        vmlal.s16 \regdst, d18, d3[2]
        vmlal.s16 \regdst, d19, d3[3]

.endm



function ff_hevc_transform_32x32_add_neon_8, export=1
       stmfd sp!, {r4, r5, r6, r7,r8,r9, r10, r11, lr}
        // 需要2048B放pTmpBlock[32*32]的值，但是存储顺序是使用vld4.16的，
        // 再保存E[0]~E[15](0~3),需要4B*16*4=256
        // 并保留空间为pTmpBlock是4字节对齐,故至少需要2308
		mov r5, #256*9
        sub  sp, sp,r5
        mov r5,sp
        sub  sp, sp, #256 			
        
        //add  r4, sp, r5     						// E[0]~E[15]在sp中的地址
        mov  r4, sp     						// E[0]~E[15]在sp中的地址
        //mov  r4, r0     						// E[0]~E[15]在sp中的地址
        //add  r5, r4, #256       				// E[0]~E[15](0~3)之后存pTmpBlock
        mov   r9, #8 										// j=8
        
        mov  r11, r0
        ldr   r10, =-1664	 								// pSrc[28*32]的地址到pSrc[2*32]的跨度  -256*7+128=-1664
        mov   r6, #256   			 				 	  // pSrc[0]到pSrc[4*32]的跨度1 
IDCT32X32ASMV7_4row_loop:     
        ldr  r12, = 0x240040
        mov  r14, #83
        
        vmov  d0, r12, r14      					// d0[0~63]:64,36,83,0
                
        vld1.16  {d4},  [r0], r6 							//pSrc[0*32]...[3]
        vld1.16  {d5},  [r0], r6 							//pSrc[4*32]...[3]
        vld1.16  {d6},  [r0], r6 							//pSrc[8*32]...[3]
        vld1.16  {d7},  [r0], r6 							//pSrc[12*32]...[3]
        vld1.16  {d8},  [r0], r6 							//pSrc[16*32]...[3]
        vld1.16  {d9},  [r0], r6 							//pSrc[20*32]...[3]
        vld1.16  {d10}, [r0], r6 						  //pSrc[24*32]...[3]
        vld1.16  {d11}, [r0], r10 						//pSrc[28*32]...[3] 
        
        vmull.s16 q6, d4, d0[0] 							// 64 * pSrc[0]
        vmlal.s16 q6, d8, d0[0] 							// +64 * pSrc[16*32]=EEEE[0]
        
        vmull.s16 q7, d4, d0[0] 							// 64 * pSrc[0]
        vmlsl.s16 q7, d8, d0[0] 							// -64 * pSrc[16*32]=EEEE[1]
        
        vmull.s16 q8, d6, d0[2] 							// 83 * pSrc[8*32]
        vmlal.s16 q8, d10, d0[1] 							// +36 * pSrc[24*32]=EEE0[0]
        
        vmull.s16 q9, d6, d0[1] 							// 36 * pSrc[8*32]
        vmlsl.s16 q9, d10, d0[2] 							// -83 * Src[24*32]=EEE0[1]
                                                                 
        vadd.s32  q12, q6, q8 								// EEE[0] = EEEE[0] + EEEO[0]//
        vadd.s32  q13, q7, q9 								// EEE[1] = EEEE[1] + EEEO[1]//
        vsub.s32  q14, q7, q9 								// EEE[2] = EEEE[1] - EEEO[1]//
        vsub.s32  q15, q6, q8 								// EEE[3] = EEEE[0] - EEEO[0]//
        
        ldr   r12, = kg_IDCT_coef_for_t32_asm_EEO_asm
        vld1.16  {d0, d1, d2, d3}, [r12]     // kg_IDCT_coef_for_t32_asm_EEO[0~15]
        
        vmull.s16 q6, d5, d0[0]
        vmlal.s16 q6, d7, d0[1]
        vmlal.s16 q6, d9, d0[2]
        vmlal.s16 q6, d11,d0[3] 							// EEO[0]
        
        vmull.s16 q7, d5, d1[0]
        vmlal.s16 q7, d7, d1[1]
        vmlal.s16 q7, d9, d1[2]
        vmlal.s16 q7, d11,d1[3] 							// EEO[1]
        
        vmull.s16 q8, d5, d2[0]
        vmlal.s16 q8, d7, d2[1]
        vmlal.s16 q8, d9, d2[2]
        vmlal.s16 q8, d11,d2[3] 							// EEO[2]
        
        vmull.s16 q9, d5, d3[0]
        vmlal.s16 q9, d7, d3[1]
        vmlal.s16 q9, d9, d3[2]
        vmlal.s16 q9, d11,d3[3] 							// EEO[3]
        
        vadd.s32  q10,q12, q6 								// EE[0]
        vsub.s32  q6, q12, q6 								// EE[7]
        vadd.s32  q11,q13, q7 								// EE[1]
        vsub.s32  q7, q13, q7 								// EE[6]        
        vadd.s32  q12,q14, q8 								// EE[2]
        vsub.s32  q8, q14, q8 								// EE[5]
        vadd.s32  q13,q15, q9 								// EE[3] 
        vsub.s32  q9, q15, q9 								// EE[4]
        
         
        ldr     r14,  = -1856                   // pSrc[30*32]的地址到pSrc[1*32]的跨度 -256*7-32*2=-1856
        vld1.16  {d4},  [r0], r6 							//pSrc[2*32]...[3]
        vld1.16  {d5},  [r0], r6 							//pSrc[6*32]...[3]
        vld1.16  {d6},  [r0], r6 							//pSrc[10*32]...[3]
        vld1.16  {d7},  [r0], r6 							//pSrc[14*32]...[3]
        vld1.16  {d8},  [r0], r6 							//pSrc[18*32]...[3]
        vld1.16  {d9},  [r0], r6 							//pSrc[22*32]...[3]
        vld1.16  {d10}, [r0], r6 						  //pSrc[26*32]...[3]
        vld1.16  {d11}, [r0], r14 						//pSrc[30*32]...[3] 
        
        ldr  r12,  = kg_IDCT_coef_for_t32_asm_EO_asm   
        
        add  r14, r4, #224 									
        //mov  r6, #-32
        mov  r8, #-32
        //=========== E[0], E[1], E[15],E[14]=============                         
        vld1.16  {d0, d1, d2, d3}, [r12]!    // kg_IDCT_coef_for_t32_asm_EO_asm[0~15]
        
        vmull.s16 q15, d4, d0[0]
        vmlal.s16 q15, d5, d0[1]
        vmlal.s16 q15, d6, d0[2]
        vmlal.s16 q15, d7, d0[3]
        vmlal.s16 q15, d8,  d1[0]
        vmlal.s16 q15, d9,  d1[1]
        vmlal.s16 q15, d10, d1[2]
        vmlal.s16 q15, d11, d1[3] 							// EO[0]
        
        vadd.s32  q14, q10, q15  								// E[0]=EE[0] + EO[0]
        vsub.s32  q10, q10, q15  								// E[15]= EE[0] - EO[0]
        
        vmull.s16 q0, d4, d2[0]
        vmlal.s16 q0, d5, d2[1]
        vmlal.s16 q0, d6, d2[2]
        vmlal.s16 q0, d7, d2[3]
        vmlal.s16 q0, d8,  d3[0]
        vmlal.s16 q0, d9,  d3[1]
        vmlal.s16 q0, d10, d3[2]
        vmlal.s16 q0, d11, d3[3] 							// EO[1]
        
        vadd.s32  q15, q11, q0  								// E[1]=EE[1] + EO[1]
        vsub.s32  q11, q11, q0  								// E[14]= EE[1] - EO[1]
        vswp   q10, q11
        vst1.32 {d28,d29,d30,d31}, [r4]!  			// E[0], E[1]
        vst1.32 {d20,d21,d22,d23}, [r14], r8		// E[14],E[15]
        
        //=========== E[2], E[3], E[13],E[12]=============
        vld1.16  {d0, d1, d2, d3}, [r12]!    // kg_IDCT_coef_for_t32_asm_EO_asm[16~31]
        
        vmull.s16 q15, d4, d0[0]
        vmlal.s16 q15, d5, d0[1]
        vmlal.s16 q15, d6, d0[2]
        vmlal.s16 q15, d7, d0[3]
        vmlal.s16 q15, d8,  d1[0]
        vmlal.s16 q15, d9,  d1[1]
        vmlal.s16 q15, d10, d1[2]
        vmlal.s16 q15, d11, d1[3] 							// EO[2]
        
        vadd.s32  q14, q12, q15  								// E[2]=EE[2] + EO[2]
        vsub.s32  q11, q12, q15  								// E[13]= EE[2] - EO[2]
        
        vmull.s16 q0, d4, d2[0]
        vmlal.s16 q0, d5, d2[1]
        vmlal.s16 q0, d6, d2[2]
        vmlal.s16 q0, d7, d2[3]
        vmlal.s16 q0, d8,  d3[0]
        vmlal.s16 q0, d9,  d3[1]
        vmlal.s16 q0, d10, d3[2]
        vmlal.s16 q0, d11, d3[3] 							// EO[3]
        
        vadd.s32  q15, q13, q0  								// E[3]=EE[3] + EO[3]
        vsub.s32  q10, q13, q0  								// E[12]= EE[3] - EO[3]
        
        vst1.32 {d28,d29,d30,d31}, [r4]!  			// E[2], E[3]
        vst1.32 {d20,d21,d22,d23}, [r14], r8			// E[12],E[13]
         
        //=========== E[4], E[5], E[11],E[10]=============
        vld1.16  {d0, d1, d2, d3}, [r12]!    // kg_IDCT_coef_for_t32_asm_EO_asm[32~47]
        
        vmull.s16 q15, d4, d0[0]
        vmlal.s16 q15, d5, d0[1]
        vmlal.s16 q15, d6, d0[2]
        vmlal.s16 q15, d7, d0[3]
        vmlal.s16 q15, d8,  d1[0]
        vmlal.s16 q15, d9,  d1[1]
        vmlal.s16 q15, d10, d1[2]
        vmlal.s16 q15, d11, d1[3] 							// EO[4]
        
        vadd.s32  q14, q9, q15  								// E[4]=EE[4] + EO[4]
        vsub.s32  q11, q9, q15  								// E[11]= EE[4] - EO[4]
        
        vmull.s16 q0, d4, d2[0]
        vmlal.s16 q0, d5, d2[1]
        vmlal.s16 q0, d6, d2[2]
        vmlal.s16 q0, d7, d2[3]
        vmlal.s16 q0, d8,  d3[0]
        vmlal.s16 q0, d9,  d3[1]
        vmlal.s16 q0, d10, d3[2]
        vmlal.s16 q0, d11, d3[3] 							// EO[5]
        
        vadd.s32  q15, q8, q0  								// E[5]=EE[5] + EO[5]
        vsub.s32  q10, q8, q0  								// E[10]= EE[5] - EO[5]
        
        vst1.32 {d28,d29,d30,d31}, [r4]!  			// E[4], E[5]
        vst1.32 {d20,d21,d22,d23}, [r14], r8		// E[10],E[11],
        
        //=========== E[6], E[7], E[9],E[8]=============
        vld1.16  {d0, d1, d2, d3}, [r12]!    // kg_IDCT_coef_for_t32_asm_EO_asm[48~63]
        
        vmull.s16 q15, d4, d0[0]
        vmlal.s16 q15, d5, d0[1]
        vmlal.s16 q15, d6, d0[2]
        vmlal.s16 q15, d7, d0[3]
        vmlal.s16 q15, d8,  d1[0]
        vmlal.s16 q15, d9,  d1[1]
        vmlal.s16 q15, d10, d1[2]
        vmlal.s16 q15, d11, d1[3] 							// EO[6]
        
        vadd.s32  q14, q7, q15  								// E[6]=EE[6] + EO[6]
        vsub.s32  q11, q7, q15  								// E[9]= EE[6] - EO[6]
        
        vmull.s16 q0, d4, d2[0]
        vmlal.s16 q0, d5, d2[1]
        vmlal.s16 q0, d6, d2[2]
        vmlal.s16 q0, d7, d2[3]
        vmlal.s16 q0, d8,  d3[0]
        vmlal.s16 q0, d9,  d3[1]
        vmlal.s16 q0, d10, d3[2]
        vmlal.s16 q0, d11, d3[3] 							// EO[7]
        
        vadd.s32  q15, q6, q0  								// E[7]=EE[7] + EO[7]
        vsub.s32  q10, q6, q0  								// E[8]= EE[7] - EO[7]
        
        vst1.32 {d28,d29,d30,d31}, [r4]!  			// E[6], E[7]
        vst1.32 {d20,d21,d22,d23}, [r4] 			// E[8],E[9],
        sub   r4, r4, #128
        // =======pSrc[k*32],k=1,3,5...,31==========
        ldr   r12, = kg_IDCT_coef_for_t32_asm_O_asm
        mov  r7, #128
        ldr  r8, = -1976      // 从pSrc[31*32]到pSrc[0*32][4]的距离：-128*15-32*2+4*2=-1976
        
        vld1.16  {d4},  [r0], r7 							//pSrc[1	*	32]...[3]
        vld1.16  {d5},  [r0], r7 							//pSrc[3	*	32]...[3]
        vld1.16  {d6},  [r0], r7 							//pSrc[5	*	32]...[3]
        vld1.16  {d7},  [r0], r7 							//pSrc[7	*	32]...[3]
        vld1.16  {d8},  [r0], r7 							//pSrc[9	*	32]...[3]
        vld1.16  {d9},  [r0], r7 							//pSrc[11	*	32]...[3]
        vld1.16  {d10}, [r0], r7 						  //pSrc[13	*	32]...[3]
        vld1.16  {d11}, [r0], r7 					  	//pSrc[15	*	32]...[3]
        vld1.16  {d12}, [r0], r7 							//pSrc[17	*	32]...[3]
        vld1.16  {d13}, [r0], r7 							//pSrc[19	*	32]...[3]
        vld1.16  {d14}, [r0], r7 							//pSrc[21	*	32]...[3]
        vld1.16  {d15}, [r0], r7 							//pSrc[23	*	32]...[3]
        vld1.16  {d16}, [r0], r7 							//pSrc[25	*	32]...[3]
        vld1.16  {d17}, [r0], r7 							//pSrc[27	*	32]...[3]
        vld1.16  {d18}, [r0], r7 						  //pSrc[29	*	32]...[3]
        vld1.16  {d19}, [r0], r8  						//pSrc[31	*	32]...[3]
        // 此时r0指向pSrc[0*32][4]
        // =======O[k]==========
        add   r7, r5, #224 										// pTmpBlock+31的地址
        mov   r8, #-32
        mov   r14, #4 												// 控制计算O[k]
        //r4为E[0]的地址
IDCT32X32ASMV7_4row_4O:       
        CMPUTE_O_K q10 												// O[0]
        vld1.32   {d28,d29,d30,d31}, [r4]! 		// E[0],E[1]
        vadd.s32  q12, q14, q10 							// E[0] + O[0]
        vsub.s32  q10, q14, q10 							// E[0] - O[0]
        vqrshrn.s32 d24, q12, #7 								// pTmpBlock[00~03]
        vqrshrn.s32 d20, q10, #7 								// pTmpBlock[310~313]
         
        CMPUTE_O_K q11 												// O[1]
        vadd.s32  q13, q15, q11 							// E[1] + O[1]
        vsub.s32  q11, q15, q11 							// E[1] - O[1]
        vqrshrn.s32 d25, q13, #7 								// pTmpBlock[10~13]
        vqrshrn.s32 d21, q11, #7 								// pTmpBlock[300~303]
        
        CMPUTE_O_K q11 												// O[2]
        vld1.32   {d28,d29,d30,d31}, [r4]! 		// E[2],E[3]
        vadd.s32  q13, q14, q11 							// E[2] + O[2]
        vsub.s32  q11, q14, q11 							// E[2] - O[2]
        vqrshrn.s32 d26, q13, #7 								// pTmpBlock[20~23]
        vqrshrn.s32 d22, q11, #7 								// pTmpBlock[290~293]
         
        CMPUTE_O_K q14 												// O[1]
        vadd.s32  q0, q15, q14 							// E[1] + O[1]
        vsub.s32  q15, q15, q14 							// E[1] - O[1]
        vqrshrn.s32 d27, q0, #7 								// pTmpBlock[10~13]
        vqrshrn.s32 d23, q15, #7 								// pTmpBlock[300~303]
        // 为了转置存储0~3行和28~31行的4个元素,需要将31~28的寄存器顺序排列
        vswp   d20, d21
        vswp   d22, d23
        vswp   q10, q11
        // d24~d27存放的是00~03,10~13,20~23,30~33
        // d20~d23存放的是280~283,290~293,300~303,310~313        
        vst4.16 {d24,d25,d26,d27}, [r5]!
        vst4.16 {d20,d21,d22,d23}, [r7], r8
        
        subs    r14, r14, #1
        bgt    IDCT32X32ASMV7_4row_4O
        sub     r4, r4, #256 								// 回到存放E[0]的sp地址
        add     r5, r5, #128 								// 下一次循环需要使用sp存放pTmpBlock的地址
        subs    r9, r9, #1 						// j--
        bgt     IDCT32X32ASMV7_4row_loop
        
        
        // ===============第二次变换==================
        //sub    r5, r5, #2048  			// 回到pTmpBlock[0]的位置--与下语句的r0是一致的
        //add    r0, r4, #256 				// 下一次循环是数据源地址，即上一次pTmpBlock的地址
        sub    r0, r5, #2048 				// 下一次循环是数据源地址，即上一次pTmpBlock的地址
        mov    r5, r11 							// 存储在原始pSrc的内存中
        mov    r9, #8 							// j=8
        ldr   r10, =-1776	 								// pSrc[28*32]的地址到pSrc[2*32]的跨度  -256*7+8*2=-1776
        mov   r6, #256   			 				 	  // pSrc[0]到pSrc[4*32]的跨度1
IDCT32X32ASMV7_4row_loop_2:        
        ldr  r12, = 0x240040
        mov  r14, #83
       
        vmov  d0, r12, r14      					// d0[0~63]:64,36,83,0

        vld1.16  {d4},  [r0], r6 							//pSrc[0*32]...[3]
        vld1.16  {d5},  [r0], r6 							//pSrc[4*32]...[3]
        vld1.16  {d6},  [r0], r6 							//pSrc[8*32]...[3]
        vld1.16  {d7},  [r0], r6 							//pSrc[12*32]...[3]
        vld1.16  {d8},  [r0], r6 							//pSrc[16*32]...[3]
        vld1.16  {d9},  [r0], r6 							//pSrc[20*32]...[3]
        vld1.16  {d10}, [r0], r6 						  //pSrc[24*32]...[3]
        vld1.16  {d11}, [r0], r10 						//pSrc[28*32]...[3] 
        
        vmull.s16 q6, d4, d0[0] 							// 64 * pSrc[0]
        vmlal.s16 q6, d8, d0[0] 							// +64 * pSrc[16*32]=EEEE[0]
        
        vmull.s16 q7, d4, d0[0] 							// 64 * pSrc[0]
        vmlsl.s16 q7, d8, d0[0] 							// -64 * pSrc[16*32]=EEEE[1]
        
        vmull.s16 q8, d6, d0[2] 							// 83 * pSrc[8*32]
        vmlal.s16 q8, d10, d0[1] 							// +36 * pSrc[24*32]=EEE0[0]
        
        vmull.s16 q9, d6, d0[1] 							// 36 * pSrc[8*32]
        vmlsl.s16 q9, d10, d0[2] 							// -83 * Src[24*32]=EEE0[1]
                                                                 
        vadd.s32  q12, q6, q8 								// EEE[0] = EEEE[0] + EEEO[0]//
        vadd.s32  q13, q7, q9 								// EEE[1] = EEEE[1] + EEEO[1]//
        vsub.s32  q14, q7, q9 								// EEE[2] = EEEE[1] - EEEO[1]//
        vsub.s32  q15, q6, q8 								// EEE[3] = EEEE[0] - EEEO[0]//
        
        ldr   r12, = kg_IDCT_coef_for_t32_asm_EEO_asm
        vld1.16  {d0, d1, d2, d3}, [r12]     // kg_IDCT_coef_for_t32_asm_EEO[0~15]
        
        vmull.s16 q6, d5, d0[0]
        vmlal.s16 q6, d7, d0[1]
        vmlal.s16 q6, d9, d0[2]
        vmlal.s16 q6, d11,d0[3] 							// EEO[0]
        
        vmull.s16 q7, d5, d1[0]
        vmlal.s16 q7, d7, d1[1]
        vmlal.s16 q7, d9, d1[2]
        vmlal.s16 q7, d11,d1[3] 							// EEO[1]
        
        vmull.s16 q8, d5, d2[0]
        vmlal.s16 q8, d7, d2[1]
        vmlal.s16 q8, d9, d2[2]
        vmlal.s16 q8, d11,d2[3] 							// EEO[2]
        
        vmull.s16 q9, d5, d3[0]
        vmlal.s16 q9, d7, d3[1]
        vmlal.s16 q9, d9, d3[2]
        vmlal.s16 q9, d11,d3[3] 							// EEO[3]
        
        vadd.s32  q10,q12, q6 								// EE[0]
        vsub.s32  q6, q12, q6 								// EE[7]
        vadd.s32  q11,q13, q7 								// EE[1]
        vsub.s32  q7, q13, q7 								// EE[6]        
        vadd.s32  q12,q14, q8 								// EE[2]
        vsub.s32  q8, q14, q8 								// EE[5]
        vadd.s32  q13,q15, q9 								// EE[3] 
        vsub.s32  q9, q15, q9 								// EE[4]

        ldr     r7, = -1800                  // pSrc[30*32]的地址到pSrc[1*32]的跨度 -256*7-4*2=-1800
        vld1.16  {d4},  [r0], r6 							//pSrc[2*32]...[3]
        vld1.16  {d5},  [r0], r6 							//pSrc[6*32]...[3]
        vld1.16  {d6},  [r0], r6 							//pSrc[10*32]...[3]
        vld1.16  {d7},  [r0], r6 							//pSrc[14*32]...[3]
        vld1.16  {d8},  [r0], r6 							//pSrc[18*32]...[3]
        vld1.16  {d9},  [r0], r6 							//pSrc[22*32]...[3]
        vld1.16  {d10}, [r0], r6 						  //pSrc[26*32]...[3]
        vld1.16  {d11}, [r0], r7 						//pSrc[30*32]...[3] 
        
        ldr  r12,  = kg_IDCT_coef_for_t32_asm_EO_asm   
        
        add  r14, r4, #224   									
        mov  r7, #-32
        //=========== E[0], E[1], E[15],E[14]=============                         
        vld1.16  {d0, d1, d2, d3}, [r12]!    // kg_IDCT_coef_for_t32_asm_EO_asm[0~15]
        
        vmull.s16 q15, d4, d0[0]
        vmlal.s16 q15, d5, d0[1]
        vmlal.s16 q15, d6, d0[2]
        vmlal.s16 q15, d7, d0[3]
        vmlal.s16 q15, d8,  d1[0]
        vmlal.s16 q15, d9,  d1[1]
        vmlal.s16 q15, d10, d1[2]
        vmlal.s16 q15, d11, d1[3] 							// EO[0]
        
        vadd.s32  q14, q10, q15  								// E[0]=EE[0] + EO[0]
        vsub.s32  q10, q10, q15  								// E[15]= EE[0] - EO[0]
        
        vmull.s16 q0, d4, d2[0]
        vmlal.s16 q0, d5, d2[1]
        vmlal.s16 q0, d6, d2[2]
        vmlal.s16 q0, d7, d2[3]
        vmlal.s16 q0, d8,  d3[0]
        vmlal.s16 q0, d9,  d3[1]
        vmlal.s16 q0, d10, d3[2]
        vmlal.s16 q0, d11, d3[3] 							// EO[1]
        
        vadd.s32  q15, q11, q0  								// E[1]=EE[1] + EO[1]
        vsub.s32  q11, q11, q0  								// E[14]= EE[1] - EO[1]
        vswp   q10, q11
        vst1.32 {d28,d29,d30,d31}, [r4]!  			// E[0], E[1]
        vst1.32 {d20,d21,d22,d23}, [r14], r7		// E[14],E[15]
        
        //=========== E[2], E[3], E[13],E[12]=============
        vld1.16  {d0, d1, d2, d3}, [r12]!    // kg_IDCT_coef_for_t32_asm_EO_asm[16~31]
        
        vmull.s16 q15, d4, d0[0]
        vmlal.s16 q15, d5, d0[1]
        vmlal.s16 q15, d6, d0[2]
        vmlal.s16 q15, d7, d0[3]
        vmlal.s16 q15, d8,  d1[0]
        vmlal.s16 q15, d9,  d1[1]
        vmlal.s16 q15, d10, d1[2]
        vmlal.s16 q15, d11, d1[3] 							// EO[2]
        
        vadd.s32  q14, q12, q15  								// E[2]=EE[2] + EO[2]
        vsub.s32  q11, q12, q15  								// E[13]= EE[2] - EO[2]
        
        vmull.s16 q0, d4, d2[0]
        vmlal.s16 q0, d5, d2[1]
        vmlal.s16 q0, d6, d2[2]
        vmlal.s16 q0, d7, d2[3]
        vmlal.s16 q0, d8,  d3[0]
        vmlal.s16 q0, d9,  d3[1]
        vmlal.s16 q0, d10, d3[2]
        vmlal.s16 q0, d11, d3[3] 							// EO[3]
        
        
        vadd.s32  q15, q13, q0  								// E[3]=EE[3] + EO[3]
        vsub.s32  q10, q13, q0  								// E[12]= EE[3] - EO[3]
        
        vst1.32 {d28,d29,d30,d31}, [r4]!  			// E[2], E[3]
        vst1.32 {d20,d21,d22,d23}, [r14], r7			// E[12],E[13]
         
        //=========== E[4], E[5], E[11],E[10]=============
        vld1.16  {d0, d1, d2, d3}, [r12]!    // kg_IDCT_coef_for_t32_asm_EO_asm[32~47]
        
        vmull.s16 q15, d4, d0[0]
        vmlal.s16 q15, d5, d0[1]
        vmlal.s16 q15, d6, d0[2]
        vmlal.s16 q15, d7, d0[3]
        vmlal.s16 q15, d8,  d1[0]
        vmlal.s16 q15, d9,  d1[1]
        vmlal.s16 q15, d10, d1[2]
        vmlal.s16 q15, d11, d1[3] 							// EO[4]
        
        vadd.s32  q14, q9, q15  								// E[4]=EE[4] + EO[4]
        vsub.s32  q11, q9, q15  								// E[11]= EE[4] - EO[4]
        
        vmull.s16 q0, d4, d2[0]
        vmlal.s16 q0, d5, d2[1]
        vmlal.s16 q0, d6, d2[2]
        vmlal.s16 q0, d7, d2[3]
        vmlal.s16 q0, d8,  d3[0]
        vmlal.s16 q0, d9,  d3[1]
        vmlal.s16 q0, d10, d3[2]
        vmlal.s16 q0, d11, d3[3] 							// EO[5]
        
        vadd.s32  q15, q8, q0  								// E[5]=EE[5] + EO[5]
        vsub.s32  q10, q8, q0  								// E[10]= EE[5] - EO[5]
        
        vst1.32 {d28,d29,d30,d31}, [r4]!  			// E[4], E[5]
        vst1.32 {d20,d21,d22,d23}, [r14], r7		// E[10],E[11],
        
        //=========== E[6], E[7], E[9],E[8]=============
        vld1.16  {d0, d1, d2, d3}, [r12]!    // kg_IDCT_coef_for_t32_asm_EO_asm[48~63]
        
        vmull.s16 q15, d4, d0[0]
        vmlal.s16 q15, d5, d0[1]
        vmlal.s16 q15, d6, d0[2]
        vmlal.s16 q15, d7, d0[3]
        vmlal.s16 q15, d8,  d1[0]
        vmlal.s16 q15, d9,  d1[1]
        vmlal.s16 q15, d10, d1[2]
        vmlal.s16 q15, d11, d1[3] 							// EO[6]
        
        vadd.s32  q14, q7, q15  								// E[6]=EE[6] + EO[6]
        vsub.s32  q11, q7, q15  								// E[9]= EE[6] - EO[6]
        
        vmull.s16 q0, d4, d2[0]
        vmlal.s16 q0, d5, d2[1]
        vmlal.s16 q0, d6, d2[2]
        vmlal.s16 q0, d7, d2[3]
        vmlal.s16 q0, d8,  d3[0]
        vmlal.s16 q0, d9,  d3[1]
        vmlal.s16 q0, d10, d3[2]
        vmlal.s16 q0, d11, d3[3] 							// EO[7]
        
        vadd.s32  q15, q6, q0  								// E[7]=EE[7] + EO[7]
        vsub.s32  q10, q6, q0  								// E[8]= EE[7] - EO[7]
        
        vst1.32 {d28,d29,d30,d31}, [r4]!  			// E[6], E[7]
        vst1.32 {d20,d21,d22,d23}, [r4] 			// E[8],E[9],
        sub   r4, r4, #128
        // =======pSrc[k*32],k=1,3,5...,31==========
        ldr   r12, = kg_IDCT_coef_for_t32_asm_O_asm
        mov  r7,  #16
        mov  r14, #240
        ldr  r8, = -1784      // 从pSrc[31*32]到pSrc[0*32][4]的距离：-16*8-240*7+12*2B=
        
        vld1.16  {d4},  [r0], r7 							//pSrc[1	*	32]...[3]
        vld1.16  {d5},  [r0], r14 						//pSrc[3	*	32]...[3]
        vld1.16  {d6},  [r0], r7 							//pSrc[5	*	32]...[3]
        vld1.16  {d7},  [r0], r14 						//pSrc[7	*	32]...[3]
        vld1.16  {d8},  [r0], r7 							//pSrc[9	*	32]...[3]
        vld1.16  {d9},  [r0], r14 						//pSrc[11	*	32]...[3]
        vld1.16  {d10}, [r0], r7 						  //pSrc[13	*	32]...[3]
        vld1.16  {d11}, [r0], r14 				  	//pSrc[15	*	32]...[3]
        vld1.16  {d12}, [r0], r7 							//pSrc[17	*	32]...[3]
        vld1.16  {d13}, [r0], r14 						//pSrc[19	*	32]...[3]
        vld1.16  {d14}, [r0], r7 							//pSrc[21	*	32]...[3]
        vld1.16  {d15}, [r0], r14 						//pSrc[23	*	32]...[3]
        vld1.16  {d16}, [r0], r7 							//pSrc[25	*	32]...[3]
        vld1.16  {d17}, [r0], r14 						//pSrc[27	*	32]...[3]
        vld1.16  {d18}, [r0], r7 						  //pSrc[29	*	32]...[3]
        vld1.16  {d19}, [r0], r8  						//pSrc[31	*	32]...[3]
        // 此时r0指向pSrc[0*32][4]
        // =======O[k]==========
        add   r7, r5, #56//#224 										// pTmpBlock+31的地址
        mov   r8, #64
        mov   r14, #4 												// 控制计算O[k]
        //r4为E[0]的地址
IDCT32X32ASMV7_4row_4O_2:      
        CMPUTE_O_K q10 												// O[0]
        vld1.32   {d28,d29,d30,d31}, [r4]! 		// E[0],E[1]
        vadd.s32  q12, q14, q10 							// E[0] + O[0]
        vsub.s32  q10, q14, q10 							// E[0] - O[0]
        vqrshrn.s32 d24, q12, #12 								// pTmpBlock[00~03]
        vqrshrn.s32 d20, q10, #12 								// pTmpBlock[310~313]
         
        CMPUTE_O_K q11 												// O[1]
        vadd.s32  q13, q15, q11 							// E[1] + O[1]
        vsub.s32  q11, q15, q11 							// E[1] - O[1]
        vqrshrn.s32 d25, q13, #12 								// pTmpBlock[10~13]
        vqrshrn.s32 d21, q11, #12 								// pTmpBlock[300~303]
        
        CMPUTE_O_K q11 												// O[2]
        vld1.32   {d28,d29,d30,d31}, [r4]! 		// E[2],E[3]
        vadd.s32  q13, q14, q11 							// E[2] + O[2]
        vsub.s32  q11, q14, q11 							// E[2] - O[2]
        vqrshrn.s32 d26, q13, #12 								// pTmpBlock[20~23]
        vqrshrn.s32 d22, q11, #12 								// pTmpBlock[290~293]
         
        CMPUTE_O_K q14 												// O[1]
        vadd.s32  q0, q15, q14 							// E[1] + O[1]
        vsub.s32  q15, q15, q14 							// E[1] - O[1]
        vqrshrn.s32 d27, q0, #12 								// pTmpBlock[10~13]
        vqrshrn.s32 d23, q15, #12 								// pTmpBlock[300~303]
        // 为了转置存储0~3行和28~31行的4个元素,需要将31~28的寄存器顺序排列
        vswp   d20, d21
        vswp   d22, d23
        vswp   q10, q11
        // d24~d27存放的是00~03,10~13,20~23,30~33
        // d20~d23存放的是280~283,290~293,300~303,310~313        
//        vst4.16 {d24,d25,d26,d27}, [r5]!
//        vst4.16 {d20,d21,d22,d23}, [r7], r8
        vtrn.32	d24,d26
        vtrn.32	d25,d27
        vtrn.16	d24,d25
        vtrn.16	d26,d27
        
        vst1.16 d24,[r5],r8		//a[0]
        vst1.16 d25,[r5],r8		//a[32]
        vst1.16 d26,[r5],r8		//a[64]
        vst1.16 d27,[r5]		//a[96]
        sub r5,r5,#184
        
        vtrn.32	d20,d22
        vtrn.32	d21,d23
        vtrn.16	d20,d21
        vtrn.16	d22,d23
        
        
        vst1.16 d20,[r7],r8		//a[0]
        vst1.16 d21,[r7],r8		//a[32]
        vst1.16 d22,[r7],r8		//a[64]
        vst1.16 d23,[r7]
        sub r7,r7,#200
        
        
        subs    r14, r14, #1
        bgt    IDCT32X32ASMV7_4row_4O_2
        sub     r4, r4, #256 								// 回到存放E[0]的sp地址
        add     r5, r5, #112*2 								// 下一次循环需要使用sp存放pTmpBlock的地址
        subs    r9, r9, #1 						// j--
        bgt     IDCT32X32ASMV7_4row_loop_2

        add   sp, sp, #256*10									

        
        ldmfd sp!, {r4, r5, r6, r7,r8,r9, r10, r11, pc}

endfunc


function transform_add8x8_neon,export=1
	vld1.16     {d0,d1},[r1]!
        vld1.8      {d24}, [r0]
        vaddw.u8    q13, q0, d24
        vqmovun.s16 d26, q13
        vst1.8      {d26}, [r0], r2
       	vld1.16     {d0,d1},[r1]!
        vld1.8      {d24}, [r0]
        vaddw.u8    q13, q0, d24
        vqmovun.s16 d26, q13
        vst1.8      {d26}, [r0], r2
        		vld1.16     {d0,d1},[r1]!
        vld1.8      {d24}, [r0]
        vaddw.u8    q13, q0, d24
        vqmovun.s16 d26, q13
        vst1.8      {d26}, [r0], r2
       	vld1.16     {d0,d1},[r1]!
        vld1.8      {d24}, [r0]
        vaddw.u8    q13, q0, d24
        vqmovun.s16 d26, q13
        vst1.8      {d26}, [r0], r2
        		vld1.16     {d0,d1},[r1]!
        vld1.8      {d24}, [r0]
        vaddw.u8    q13, q0, d24
        vqmovun.s16 d26, q13
        vst1.8      {d26}, [r0], r2
       	vld1.16     {d0,d1},[r1]!
        vld1.8      {d24}, [r0]
        vaddw.u8    q13, q0, d24
        vqmovun.s16 d26, q13
        vst1.8      {d26}, [r0], r2
        		vld1.16     {d0,d1},[r1]!
        vld1.8      {d24}, [r0]
        vaddw.u8    q13, q0, d24
        vqmovun.s16 d26, q13
        vst1.8      {d26}, [r0], r2
       	vld1.16     {d0,d1},[r1]!
        vld1.8      {d24}, [r0]
        vaddw.u8    q13, q0, d24
        vqmovun.s16 d26, q13
        vst1.8      {d26}, [r0], r2
        
        bx lr
endfunc

.align 4
kg_IDCT_coef_for_t32_asm_EEO_asm:
.word 0x004b0059,0x00120032,0xffee004b,0xffceffa7,0xffa70032,0x004b0012,0xffce0012,0xffa7004b
kg_IDCT_coef_for_t32_asm_EO_asm:
.word 0x0057005a,0x00460050,0x002b0039,0x00090019,0x00390057,0xffd50009,0xffa6ffb0,0xffe7ffba,0x00090050,0xffa9ffba,0x0039ffe7,0x002b005a,0xffd50046,0x0009ffa9,0x0019005a,0xffc7ffb0,0xffb00039,0x005affe7,0xffa9fff7,0x0046002b,0xffa6002b,0x00190039,0x0046ffa9,0xffb00009,0xffba0019,0xffb0005a,0x0009002b,0x0057ffc7,0xffe70009,0xffc7002b,0xffb00046,0xffa60057
kg_IDCT_coef_for_t32_asm_O_asm:
.word 0x005a005a,0x00550058,0x004e0052,0x00430049,0x0036003d,0x0026002e,0x0016001f,0x0004000d,0x0052005a,0x002e0043,0xfffc0016,0xffcaffe1,0xffabffb7,0xffa8ffa6,0xffc3ffb2,0xfff3ffda,0x00430058,0xfff3001f,0xffaeffca,0xffb2ffa6,0xfffcffd2,0x00490026,0x0055005a,0x0016003d,0x002e0055,0xffbdfff3,0xffb7ffa6,0x0026ffea,0x00580052,0xfffc0036,0xffa6ffc3,0xffe1ffb2,0x00160052,0xffa6ffca,0x000dffc3,0x0055004e,0xffd2001f,0xffbdffa6,0x00490004,0x00260058,0xfffc004e,0xffb7ffae,0x0055000d,0xffea0043,0xffc3ffa8,0x005a001f,0xffda0036,0xffd2ffa6,0xffe10049,0xffeaffa6,0x0043004e,0xffa6ffda,0x0052fff3,0xffd2003d,0xfffcffa8,0x00360055,0xffca0043,0x0026ffb2,0xffea0055,0x0004ffa6,0x000d005a,0xffe1ffa8,0x002e0052,0xffc3ffb7,0xffb7003d,0x0052ffd2,0xffa8001f,0x005afff3,0xffa6fffc,0x00550016,0xffb2ffda,0x00430036,0xffab0036,0x0058fffc,0xffc3ffd2,0x000d0052,0x0026ffa6,0xffb20043,0x005affea,0xffb7ffe1,0xffa6002e,0x00360026,0x001fffa6,0xffa8003d,0x00430016,0x000dffab,0xffae0049,0x004e0004,0xffa80026,0xfffc0049,0x005affbd,0xffe1ffd2,0xffb20055,0x003d000d,0x0036ffa6,0xffae0016,0xffb2001f,0xffc3005a,0x00360004,0x0052ffa8,0xffeaffda,0xffa60049,0xfff30043,0x0055ffd2,0xffc30016,0xffa60055,0xffda0049,0x002efffc,0x005affb2,0x0036ffae,0xffe1fff3,0xffa80043,0xffda000d,0xffb2003d,0xffa60058,0xffb70055,0xffe10036,0x00160004,0x0043ffd2,0x005affae,0xfff30004,0xffe10016,0xffd20026,0xffc30036,0xffb70043,0xffae004e,0xffa80055,0xffa6005a
