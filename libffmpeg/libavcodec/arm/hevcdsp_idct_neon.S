#include "libavutil/arm/asm.S"
#include "neon.S"

.macro  transpose_16b_8x8   r0, r1, r2, r3, r4, r5, r6, r7
        vtrn.64         \r0, \r4
        vtrn.64         \r1, \r5
        vtrn.64         \r2, \r6
        vtrn.64         \r3, \r7
        vtrn.32         \r0, \r2
        vtrn.32         \r1, \r3
        vtrn.32         \r4, \r6
        vtrn.32         \r5, \r7
        vtrn.16         \r0, \r1
        vtrn.16         \r2, \r3
        vtrn.16         \r4, \r5
        vtrn.16         \r6, \r7
.endm

// in 4 q regs
// output 8 d regs
.macro transpose_16b_4x4    r0, r1, r2, r3
        vtrn.32         \r0, \r2
        vtrn.32         \r1, \r3
        vtrn.16         \r0, \r1
        vtrn.16         \r2, \r3
.endm

/* uses registers q2 - q6 for temp values */
.macro tr4 r0, r1, r2, r3
        vmull.s16  q4, \r1, d0[0]   // 83 * src1
        vmull.s16  q6, \r1, d0[1]   // 36 * src1
        vshll.s16  q2, \r0, #6   // 64 * src0
        vshll.s16  q3, \r2, #6   // 64 * src2
        vadd.s32   q5, q2, q3    // 64 * (src0 + src2)     e0
        vsub.s32   q2, q2, q3    // 64 * (src0 - src2)     e1
        vmlal.s16  q4, \r3, d0[1]   // 83 * src1 + 36 * src3  o0
        vmlsl.s16  q6, \r3, d0[0]   // 36 * src1 - 83 * src3  o1

        vsub.s32   q3, q5, q4    // e0 - o0
        vadd.s32   q4, q5, q4    // e0 + o0
        vadd.s32   q5, q2, q6    // e1 + o1
        vsub.s32   q6, q2, q6    // e1 - o1
.endm

.macro tr4_shift r0, r1, r2, r3, shift
        vmull.s16  q4, \r1, d0[0]   // 83 * src1
        vmull.s16  q6, \r1, d0[1]   // 36 * src1
        vshll.s16  q2, \r0, #6   // 64 * src0
        vshll.s16  q3, \r2, #6   // 64 * src2
        vadd.s32   q5, q2, q3    // 64 * (src0 + src2)     e0
        vsub.s32   q2, q2, q3    // 64 * (src0 - src2)     e1
        vmlal.s16  q4, \r3, d0[1]   // 83 * src1 + 36 * src3  o0
        vmlsl.s16  q6, \r3, d0[0]   // 36 * src1 - 83 * src3  o1

        vsub.s32   q3, q5, q4    // e0 - o0
        vadd.s32   q4, q5, q4    // e0 + o0
        vadd.s32   q5, q2, q6    // e1 + o1
        vsub.s32   q6, q2, q6    // e1 - o1

        vqrshrn.s32   \r0, q4, \shift
        vqrshrn.s32   \r1, q5, \shift
        vqrshrn.s32   \r2, q6, \shift
        vqrshrn.s32   \r3, q3, \shift
.endm


function ff_hevc_transform_4x4_add_neon_8, export=1

        mov r1,r0
        vld1.16     {q14, q15}, [r0]
        ldr         r3, tr4f
        vmov.32     d0[0], r3

        tr4_shift d28, d29, d30, d31, #7

        vtrn.16     d28, d29
        vtrn.16     d30, d31
        vtrn.32     q14, q15

        tr4_shift d28, d29, d30, d31, #12

        vtrn.16     d28, d29
        vtrn.16     d30, d31
        vtrn.32     q14, q15

        vst1.32     {d28}, [r1]!
        vst1.32     {d29}, [r1]!
        vst1.32     {d30}, [r1]!
        vst1.32     {d31}, [r1]!

        bx lr
endfunc

.macro tr8_begin in0, in1, in2, in3
        vmull.s16  q7, \in0, d1[1]   // 89 * src1
        vmull.s16  q8, \in0, d1[0]   // 75 * src1
        vmull.s16  q9, \in0, d1[3]   // 50 * src1
        vmull.s16  q10, \in0, d1[2]  // 18 * src1

        vmlal.s16  q7, \in1, d1[0]   // 75 * src3
        vmlsl.s16  q8, \in1, d1[2]   //-18 * src3
        vmlsl.s16  q9, \in1, d1[1]   //-89 * src3
        vmlsl.s16  q10, \in1, d1[3]  //-50 * src3

        vmlal.s16  q7, \in2, d1[3]   // 50 * src5
        vmlsl.s16  q8, \in2, d1[1]   //-89 * src5
        vmlal.s16  q9, \in2, d1[2]   // 18 * src5
        vmlal.s16  q10, \in2, d1[0]  // 75 * src5

        vmlal.s16  q7, \in3, d1[2]   // 18 * src7
        vmlsl.s16  q8, \in3, d1[3]   //-50 * src7
        vmlal.s16  q9, \in3, d1[0]   // 75 * src7
        vmlsl.s16  q10, \in3, d1[1]  //-89 * src7
.endm

/* 90,  87,  80,  70,  57,  43,  25,   9,
 87,  57,   9, -43, -80, -90, -70, -25,
 80,   9, -70, -87, -25,  57,  90,  43,
 70, -43, -87,   9,  90,  25, -80, -57,
57, -80, -25,  90,  -9, -87,  43,  70,
43, -90,  57,  25, -87,  70,   9, -80,
 25, -70,  90, -80,  43,   9, -57,  87,
  9, -25,  43, -57,  70, -80,  87, -90,
*/


.macro tr16_begin in0, in1, in2, in3, in4, in5, in6, in7
//TODO: reorder
        vmull.s16  q2, \in0, d2[1]   // 90 * src1
        vmlal.s16  q2, \in1, d2[0]   // 87 * src3
        vmlal.s16  q2, \in2, d2[3]   // 80 * src5
        vmlal.s16  q2, \in3, d2[2]   // 70 * src7
        vmlal.s16  q2, \in4, d3[1]   // 57 * src9
        vmlal.s16  q2, \in5, d3[0]   // 43 * src11
        vmlal.s16  q2, \in6, d3[3]   // 25 * src13
        vmlal.s16  q2, \in7, d3[2]   //  9 * src15

        vmull.s16  q3, \in0, d2[0]   // 87 * src1
        vmlal.s16  q3, \in1, d3[1]   // 57 * src3
        vmlal.s16  q3, \in2, d3[2]   // 9 * src5
        vmlsl.s16  q3, \in3, d3[0]   //-43 * src7
        vmlsl.s16  q3, \in4, d2[3]   //-80 * src9
        vmlsl.s16  q3, \in5, d2[1]   //-90 * src11
        vmlsl.s16  q3, \in6, d2[2]   //-70 * src13
        vmlsl.s16  q3, \in7, d3[3]   //-25 * src15

        vmull.s16  q4, \in0, d2[3]   // 80 * src1
        vmlal.s16  q4, \in1, d3[2]   //  9 * src3
        vmlsl.s16  q4, \in2, d2[2]   //-70 * src5
        vmlsl.s16  q4, \in3, d2[0]   //-87 * src7
        vmlsl.s16  q4, \in4, d3[3]   //-25 * src9
        vmlal.s16  q4, \in5, d3[1]   // 57 * src11
        vmlal.s16  q4, \in6, d2[1]   // 90 * src13
        vmlal.s16  q4, \in7, d3[0]   // 43 * src15

        vmull.s16  q5, \in0, d2[2]   // 70 * src1
        vmlsl.s16  q5, \in1, d3[0]   //-43 * src3
        vmlsl.s16  q5, \in2, d2[0]   //-87 * src5
        vmlal.s16  q5, \in3, d3[2]   //  9 * src7
        vmlal.s16  q5, \in4, d2[1]   // 90 * src9
        vmlal.s16  q5, \in5, d3[3]   // 25 * src11
        vmlsl.s16  q5, \in6, d2[3]   //-80 * src13
        vmlsl.s16  q5, \in7, d3[1]   //-57 * src15

        vmull.s16  q6, \in0, d3[1]   // 57 * src1
        vmlsl.s16  q6, \in1, d2[3]   //-80 * src3
        vmlsl.s16  q6, \in2, d3[3]   //-25 * src5
        vmlal.s16  q6, \in3, d2[1]   // 90 * src7
        vmlsl.s16  q6, \in4, d3[2]   // -9 * src9
        vmlsl.s16  q6, \in5, d2[0]   //-87 * src11
        vmlal.s16  q6, \in6, d3[0]   // 43 * src13
        vmlal.s16  q6, \in7, d2[2]   // 70 * src15

        vmull.s16  q7, \in0, d3[0]   // 43 * src1
        vmlsl.s16  q7, \in1, d2[1]   //-90 * src3
        vmlal.s16  q7, \in2, d3[1]   // 57 * src5
        vmlal.s16  q7, \in3, d3[3]   // 25 * src7
        vmlsl.s16  q7, \in4, d2[0]   //-87 * src9
        vmlal.s16  q7, \in5, d2[2]   // 70 * src11
        vmlal.s16  q7, \in6, d3[2]   //  9 * src13
        vmlsl.s16  q7, \in7, d2[3]   //-80 * src15

        vmull.s16  q8, \in0, d3[3]   // 25 * src1
        vmlsl.s16  q8, \in1, d2[2]   //-70 * src3
        vmlal.s16  q8, \in2, d2[1]   // 90 * src5
        vmlsl.s16  q8, \in3, d2[3]   //-80 * src7
        vmlal.s16  q8, \in4, d3[0]   // 43 * src9
        vmlal.s16  q8, \in5, d3[2]   //  9 * src11
        vmlsl.s16  q8, \in6, d3[1]   //-57 * src13
        vmlal.s16  q8, \in7, d2[0]   // 87 * src15

        vmull.s16  q9, \in0, d3[2]   //  9 * src1
        vmlsl.s16  q9, \in1, d3[3]   //-25 * src3
        vmlal.s16  q9, \in2, d3[0]   // 43 * src5
        vmlsl.s16  q9, \in3, d3[1]   //-57 * src7
        vmlal.s16  q9, \in4, d2[2]   // 70 * src9
        vmlsl.s16  q9, \in5, d2[3]   //-80 * src11
        vmlal.s16  q9, \in6, d2[0]   // 87 * src13
        vmlsl.s16  q9, \in7, d2[1]   //-90 * src15
.endm



.macro tr8_end shift
        vadd.s32   q1, q4, q7   //  e_8[0] + o_8[0], dst[0]
        vsub.s32   q4, q4, q7   //  e_8[0] - o_8[0], dst[7]

        vadd.s32   q2, q5, q8   // e_8[1] + o_8[1], dst[1]
        vsub.s32   q5, q5, q8   // e_8[1] - o_8[1], dst[6]

        vadd.s32   q11, q6, q9  // e_8[2] + o_8[2], dst[2]
        vsub.s32    q6, q6, q9  // e_8[2] - o_8[2], dst[5]

        vadd.s32   q12, q3, q10 // e_8[3] + o_8[3], dst[3]
        vsub.s32   q3, q3, q10  // e_8[3] - o_8[3], dst[4]
        vqrshrn.s32   d2, q1, \shift
        vqrshrn.s32   d3, q2, \shift
        vqrshrn.s32   d4, q11, \shift
        vqrshrn.s32   d5, q12, \shift
        vqrshrn.s32   d6, q3, \shift
        vqrshrn.s32   d7, q6, \shift
        vqrshrn.s32   d9, q4, \shift
        vqrshrn.s32   d8, q5, \shift
.endm

.macro tr8_end2
        vsub.s32   q15, q4, q7   // e_8[0] - o_8[0], dst[7]
        vsub.s32   q14, q5, q8   // e_8[1] - o_8[1], dst[6]
        vsub.s32   q13, q6, q9   // e_8[2] - o_8[2], dst[5]
        vsub.s32   q12, q3, q10  // e_8[3] - o_8[3], dst[4]
        vadd.s32   q11, q3, q10  // e_8[3] + o_8[3], dst[3]
        vadd.s32   q10, q6, q9   // e_8[2] + o_8[2], dst[2]
        vadd.s32   q9, q5, q8    // e_8[1] + o_8[1], dst[1]
        vadd.s32   q8, q4, q7    // e_8[0] + o_8[0], dst[0]
.endm


.macro tr8_add r0
        vld1.8      {d24}, [r0]
        vaddw.u8    q13, \r0, d24
        vqmovun.s16 d26, q13
        vst1.8      {d26}, [r0], r2
.endm

.macro tr16_add src, dtmp
        vld1.8      {\dtmp}, [r0]
        vaddw.u8    \src, \src, \dtmp
        vqmovun.s16 \dtmp, \src
        vst1.8      {\dtmp}, [r0], r2
.endm

function ff_hevc_transform_8x8_add_neon_8, export=1

		mov r1,r0
        push   {r4-r8}
        vpush {d8-d15}
        mov    r5, #16

        //adr       r3, tr4f
	ldr r3,=tr4f
        vld1.16   {d0, d1}, [r3]


        vld1.16 {d24}, [r1], r5
        vld1.16 {d25}, [r1], r5
        vld1.16 {d26}, [r1], r5
        vld1.16 {d27}, [r1], r5
        vld1.16 {d28}, [r1], r5
        vld1.16 {d29}, [r1], r5
        vld1.16 {d30}, [r1], r5
        vld1.16 {d31}, [r1], r5
        sub      r1, #128
        tr8_begin d25, d27, d29, d31
        tr4       d24, d26, d28, d30
        tr8_end   #7
        vst1.16 {d2}, [r1], r5
        vst1.16 {d3}, [r1], r5
        vst1.16 {d4}, [r1], r5
        vst1.16 {d5}, [r1], r5
        vst1.16 {d6}, [r1], r5
        vst1.16 {d7}, [r1], r5
        vst1.16 {d8}, [r1], r5
        vst1.16 {d9}, [r1], r5

  
        sub      r1, #120
        vld1.16 {d24}, [r1], r5
        vld1.16 {d25}, [r1], r5
        vld1.16 {d26}, [r1], r5
        vld1.16 {d27}, [r1], r5
        vld1.16 {d28}, [r1], r5
        vld1.16 {d29}, [r1], r5
        vld1.16 {d30}, [r1], r5
        vld1.16 {d31}, [r1], r5
        sub      r1, #128
        tr8_begin d25, d27, d29, d31
        tr4       d24, d26, d28, d30
        tr8_end   #7
        vst1.16 {d2}, [r1], r5
        vst1.16 {d3}, [r1], r5
        vst1.16 {d4}, [r1], r5
        vst1.16 {d5}, [r1], r5
        vst1.16 {d6}, [r1], r5
        vst1.16 {d7}, [r1], r5
        vst1.16 {d8}, [r1], r5
        vst1.16 {d9}, [r1], r5
        sub      r1, #136

        vldm r1!, {q12-q15}
        transpose_16b_4x4 d24, d26, d28, d30
        transpose_16b_4x4 d25, d27, d29, d31
        tr8_begin d26, d30, d27, d31
        tr4 d24, d28, d25, d29
        tr8_end #12
        transpose_16b_4x4 d2, d3, d4, d5
        transpose_16b_4x4 d6, d7, d8, d9
        vswp     d7, d5
        vswp     d7, d8
        vswp     d3, d6
        vswp     d6, d4

	vst1.16 {d2,d3},[r0]!
	vst1.16 {d4,d5},[r0]!
	vst1.16 {d6,d7},[r0]!
	vst1.16 {d8,d9},[r0]!

 
        vldm r1, {q12-q15} 
        transpose_16b_4x4 d24, d26, d28, d30
        transpose_16b_4x4 d25, d27, d29, d31
        tr8_begin d26, d30, d27, d31
        tr4 d24, d28, d25, d29
        tr8_end #12
        transpose_16b_4x4 d2, d3, d4, d5
        transpose_16b_4x4 d6, d7, d8, d9
        vswp     d7, d5
        vswp     d7, d8
        vswp     d3, d6
        vswp     d6, d4

	
	vst1.16 {d2,d3},[r0]!
	vst1.16 {d4,d5},[r0]!
	vst1.16 {d6,d7},[r0]!
	vst1.16 {d8,d9},[r0]!
		
        vpop {d8-d15}
        pop {r4-r8}
        bx lr
endfunc


function ff_hevc_transform_16x16_add_neon_8, export=1
      stmfd sp!, {r4, r5, r6, r7,r8,r9, r10, r11, lr}
        sub  sp, sp, #528
       // and  r5, sp, #0xF
       mov r5,sp
       and r5,r5,#0xF
        rsb  r5, r5, #0x10
        add  r5, sp, r5     						// pTmpBlock
        mov  r11, r0

        // g_IDCT16X16H_EEE,g_IDCT16X16H_EO,g_IDCT16X16H_O
        ldr   r12, =tr16
        vld1.16  {d0, d1, d2, d3},  [r12]
        // add   r4, r5, #96											// pTmpBlock+12*16
        mov   r6, #64   			 				 	 
        mov   r8, #96 										
        mov   r12, #-64 									

        ldr   r10, =-416	 								// pSrc[14*16]
        ldr   r7, =-472  									// pSrc+16  -64*7-32+8=-472

        mov   r9, #4 										// j=4

IDCT16X16_4Row_ASM_loop:
        vld1.16  {d4},  [r0], r6 							//pSrc[0]...[3]
        vld1.16  {d5},  [r0], r6 							//pSrc[2*16]...[3]
        vld1.16  {d6},  [r0], r6 							//pSrc[4*16]...[3]
        vld1.16  {d7},  [r0], r6 							//pSrc[6*16]...[3]
        vld1.16  {d8},  [r0], r6 							//pSrc[8*16]...[3]
        vld1.16  {d9},  [r0], r6 							//pSrc[10*16]...[3]
        vld1.16  {d10}, [r0], r6 						//pSrc[12*16]...[3]
        vld1.16  {d11}, [r0], r10 						//pSrc[14*16]...[3]

        vmull.s16 q6, d4, d0[0] 							// 64 * pSrc[0]
        vmlal.s16 q6, d8, d0[0] 							// +64 * pSrc[ 8*16  ]=EEE[0]

        vmull.s16 q7, d4, d0[0] 							// 64 * pSrc[0]
        vmlsl.s16 q7, d8, d0[0] 							// -64 * pSrc[ 8*16  ]=EEE[1]

        vmull.s16 q8, d6, d0[1] 							// 83 * pSrc[0]
        vmlal.s16 q8, d10, d0[3] 							// +36 * pSrc[ 8*16  ]=EE0[0]

        vmull.s16 q9, d6, d0[3] 							// 36 * pSrc[0]
        vmlsl.s16 q9, d10, d0[1] 							// -83 * pSrc[ 8*16  ]=EE0[1]

        vadd.s32  q10, q6, q8 								// EE[0] = EEE[0] + EEO[0]//
        vadd.s32  q11, q7, q9 								// EE[1] = EEE[1] + EEO[1]//
        vsub.s32  q13, q6, q8 								// EE[3] = EEE[0] - EEO[0]//
        vsub.s32  q12, q7, q9 								// EE[2] = EEE[1] - EEO[1]//

        vmull.s16 q6, d5, d1[0]
        vmlal.s16 q6, d7, d1[1]
        vmlal.s16 q6, d9, d1[2]
        vmlal.s16 q6, d11,d1[3] 							// EO[0]

        vmull.s16 q7, d5, d1[1]
        vmlsl.s16 q7, d7, d1[3]
        vmlsl.s16 q7, d9, d1[0]
        vmlsl.s16 q7, d11,d1[2] 							// EO[1]

        vmull.s16 q8, d5, d1[2]
        vmlsl.s16 q8, d7, d1[0]
        vmlal.s16 q8, d9, d1[3]
        vmlal.s16 q8, d11,d1[1] 							// EO[2]

        vmull.s16 q9, d5, d1[3]
        vmlsl.s16 q9, d7, d1[2]
        vmlal.s16 q9, d9, d1[1]
        vmlsl.s16 q9, d11,d1[0] 							// EO[3]

        vadd.s32  q2, q10, q6 								// E[0]
        vadd.s32  q3, q11, q7 								// E[1]
        vadd.s32  q4, q12, q8 								// E[2]
        vadd.s32  q5, q13, q9 								// E[3]

        vsub.s32  q6, q10, q6 								// E[7]
        vsub.s32  q7, q11, q7 								// E[6]
        vsub.s32  q8, q12, q8 								// E[5]
        vsub.s32  q9, q13, q9 								// E[4]


        vld1.16  {d20},  [r0], r6 							//pSrc[1*16]...[3]
        vld1.16  {d21},  [r0], r6 							//pSrc[3*16]...[3]
        vld1.16  {d22},  [r0], r6 							//pSrc[5*16]...[3]
        vld1.16  {d23},  [r0], r6 							//pSrc[7*16]...[3]
        vld1.16  {d24},  [r0], r6 							//pSrc[9*16]...[3]
        vld1.16  {d25},  [r0], r6 							//pSrc[11*16]...[3]
        vld1.16  {d26},  [r0], r6 						//pSrc[13*16]...[3]
        vld1.16  {d27},  [r0], r7 						//pSrc[15*16]...[3]

        vmull.s16 q15, d20, d2[0]
        vmlal.s16 q15, d21, d2[1]
        vmlal.s16 q15, d22, d2[2]
        vmlal.s16 q15, d23, d2[3]
        vmlal.s16 q15, d24, d3[0]
        vmlal.s16 q15, d25, d3[1]
        vmlal.s16 q15, d26, d3[2]
        vmlal.s16 q15, d27, d3[3] 							// O[0]

        vadd.s32  q14, q2, q15  								// E[0] + O[0]
        vsub.s32  q2,  q2, q15  								// E[0] - O[0]
        vqrshrn.s32 d28, q14, #7 								// pTmpBlock[00~03]
        vqrshrn.s32 d29, q2,  #7 								// pTmpBlock[150~153]

        vmull.s16 q15, d20, d2[1]
        vmlal.s16 q15, d21, d3[0]
        vmlal.s16 q15, d22, d3[3]
        vmlsl.s16 q15, d23, d3[1]
        vmlsl.s16 q15, d24, d2[2]
        vmlsl.s16 q15, d25, d2[0]
        vmlsl.s16 q15, d26, d2[3]
        vmlsl.s16 q15, d27, d3[2] 							// O[1]

        vadd.s32  q2,  q3, q15  								// E[1] + O[1]
        vsub.s32  q3,  q3, q15  								// E[1] - O[1]
        vqrshrn.s32 d5, q2, #7  								// pTmpBlock[10~13]
        vqrshrn.s32 d4,  q3,  #7 								// pTmpBlock[140~143]

        vmull.s16 q3, d20, d2[2]
        vmlal.s16 q3, d21, d3[3]
        vmlsl.s16 q3, d22, d2[3]
        vmlsl.s16 q3, d23, d2[1]
        vmlsl.s16 q3, d24, d3[2]
        vmlal.s16 q3, d25, d3[0]
        vmlal.s16 q3, d26, d2[0]
        vmlal.s16 q3, d27, d3[1] 							// O[2]

        vadd.s32  q15, q4, q3  								// E[2] + O[2]
        vsub.s32  q3,  q4, q3  								// E[2] - O[2]
        vqrshrn.s32 d30, q15, #7 								// pTmpBlock[20~23]
        vqrshrn.s32 d31,  q3,  #7 								// pTmpBlock[130~133]


        vmull.s16 q4, d20, d2[3]
        vmlsl.s16 q4, d21, d3[1]
        vmlsl.s16 q4, d22, d2[1]
        vmlal.s16 q4, d23, d3[3]
        vmlal.s16 q4, d24, d2[0]
        vmlal.s16 q4, d25, d3[2]
        vmlsl.s16 q4, d26, d2[2]
        vmlsl.s16 q4, d27, d3[0] 							// O[3]

        vadd.s32  q3, q5, q4  								// E[3] + O[3]
        vsub.s32  q5, q5, q4  								// E[3] - O[3]
        vqrshrn.s32 d7, q3, #7 								// pTmpBlock[30~33]
        vqrshrn.s32 d6,  q5,  #7 								// pTmpBlock[120~123]

        vswp    d5, d29
        vswp    d7, d31
        vswp    q2, q3

        vst4.16 {d28, d29, d30, d31}, [r5],r8
        vst4.16 {d4, d5, d6, d7}, [r5],r12

        vmull.s16 q14, d20, d3[0]
        vmlsl.s16 q14, d21, d2[2]
        vmlsl.s16 q14, d22, d3[2]
        vmlal.s16 q14, d23, d2[0]
        vmlsl.s16 q14, d24, d3[3]
        vmlsl.s16 q14, d25, d2[1]
        vmlal.s16 q14, d26, d3[1]
        vmlal.s16 q14, d27, d2[3] 							// O[4]

        vadd.s32  q15, q9, q14  								// E[4] + O[4]
        vsub.s32  q14, q9, q14  								// E[4] - O[4]
        vqrshrn.s32 d4, q15, #7 								// pTmpBlock[40~43]
        vqrshrn.s32 d11,q14,  #7 								// pTmpBlock[110~113]


        vmull.s16 q14, d20, d3[1]
        vmlsl.s16 q14, d21, d2[0]
        vmlal.s16 q14, d22, d3[0]
        vmlal.s16 q14, d23, d3[2]
        vmlsl.s16 q14, d24, d2[1]
        vmlal.s16 q14, d25, d2[3]
        vmlal.s16 q14, d26, d3[3]
        vmlsl.s16 q14, d27, d2[2] 							// O[5]

        vadd.s32  q15, q8, q14  								// E[5] + O[5]
        vsub.s32  q14, q8, q14  								// E[5] - O[5]
        vqrshrn.s32 d5, q15, #7 								// pTmpBlock[50~53]
        vqrshrn.s32 d10,  q14,  #7 								// pTmpBlock[100~103]


        vmull.s16 q14, d20, d3[2]
        vmlsl.s16 q14, d21, d2[3]
        vmlal.s16 q14, d22, d2[0]
        vmlsl.s16 q14, d23, d2[2]
        vmlal.s16 q14, d24, d3[1]
        vmlal.s16 q14, d25, d3[3]
        vmlsl.s16 q14, d26, d3[0]
        vmlal.s16 q14, d27, d2[1] 							// O[6]

        vadd.s32  q15, q7, q14  								// E[6] + O[6]
        vsub.s32  q14,  q7, q14  								// E[6] - O[6]
        vqrshrn.s32 d6, q15, #7 								// pTmpBlock[60~63]
        vqrshrn.s32 d9,  q14,  #7 								// pTmpBlock[90~93]


        vmull.s16 q14, d20, d3[3]
        vmlsl.s16 q14, d21, d3[2]
        vmlal.s16 q14, d22, d3[1]
        vmlsl.s16 q14, d23, d3[0]
        vmlal.s16 q14, d24, d2[3]
        vmlsl.s16 q14, d25, d2[2]
        vmlal.s16 q14, d26, d2[1]
        vmlsl.s16 q14, d27, d2[0] 							// O[7]

        vadd.s32  q15, q6, q14  								// E[7] + O[7]
        vsub.s32  q14,  q6, q14  								// E[7] - O[7]
        vqrshrn.s32 d7, q15, #7 								// pTmpBlock[70~73]
        vqrshrn.s32 d8,  q14,  #7								// pTmpBlock[80~83]
        vst4.16 {d4, d5, d6, d7}, [r5]! 			// 4*8+64
        vst4.16 {d8, d9,d10, d11},[r5],r6         // 32*2  pTmpBlock+15*16

        subs r9, r9, #1 													// j--
        bgt  IDCT16X16_4Row_ASM_loop

        mov  r9, #4
        sub  r0, r5, #512 							
        mov  r5, r11 										
        mov  r4, #16  			 				 	
        mov  r11, #112								
        ldr  r10, =-376 									
        ldr  r7, =-392 									
IDCT16X16_4Row_ASM_loop_2:
        vld1.16  {d4},  [r0], r4 							//pSrc[0]...[3]
        vld1.16  {d5},  [r0], r11 							//pSrc[2*16]...[3]
        vld1.16  {d6},  [r0], r4 							//pSrc[4*16]...[3]
        vld1.16  {d7},  [r0], r11 							//pSrc[6*16]...[3]
        vld1.16  {d8},  [r0], r4 							//pSrc[8*16]...[3]
        vld1.16  {d9},  [r0], r11 							//pSrc[10*16]...[3]
        vld1.16  {d10}, [r0], r4 						//pSrc[12*16]...[3]
        vld1.16  {d11}, [r0], r7   						//pSrc[14*16]...[3]

        vmull.s16 q6, d4, d0[0] 							// 64 * pSrc[0]
        vmlal.s16 q6, d8, d0[0] 							// +64 * pSrc[ 8*16  ]=EEE[0]

        vmull.s16 q7, d4, d0[0] 							// 64 * pSrc[0]
        vmlsl.s16 q7, d8, d0[0] 							// -64 * pSrc[ 8*16  ]=EEE[1]

        vmull.s16 q8, d6, d0[1] 							// 83 * pSrc[0]
        vmlal.s16 q8, d10, d0[3] 							// +36 * pSrc[ 8*16  ]=EE0[0]

        vmull.s16 q9, d6, d0[3] 							// 36 * pSrc[0]
        vmlsl.s16 q9, d10, d0[1] 							// -83 * pSrc[ 8*16  ]=EE0[1]

        vadd.s32  q10, q6, q8 								// EE[0] = EEE[0] + EEO[0]//
        vadd.s32  q11, q7, q9 								// EE[1] = EEE[1] + EEO[1]//
        vsub.s32  q13, q6, q8 								// EE[3] = EEE[0] - EEO[0]//
        vsub.s32  q12, q7, q9 								// EE[2] = EEE[1] - EEO[1]//

        vmull.s16 q6, d5, d1[0]
        vmlal.s16 q6, d7, d1[1]
        vmlal.s16 q6, d9, d1[2]
        vmlal.s16 q6, d11,d1[3] 							// EO[0]

        vmull.s16 q7, d5, d1[1]
        vmlsl.s16 q7, d7, d1[3]
        vmlsl.s16 q7, d9, d1[0]
        vmlsl.s16 q7, d11,d1[2] 							// EO[1]

        vmull.s16 q8, d5, d1[2]
        vmlsl.s16 q8, d7, d1[0]
        vmlal.s16 q8, d9, d1[3]
        vmlal.s16 q8, d11,d1[1] 							// EO[2]

        vmull.s16 q9, d5, d1[3]
        vmlsl.s16 q9, d7, d1[2]
        vmlal.s16 q9, d9, d1[1]
        vmlsl.s16 q9, d11,d1[0] 							// EO[3]

        vadd.s32  q2, q10, q6 								// E[0]
        vadd.s32  q3, q11, q7 								// E[1]
        vadd.s32  q4, q12, q8 								// E[2]
        vadd.s32  q5, q13, q9 								// E[3]

        vsub.s32  q6, q10, q6 								// E[7]
        vsub.s32  q7, q11, q7 								// E[6]
        vsub.s32  q8, q12, q8 								// E[5]
        vsub.s32  q9, q13, q9 								// E[4]


        vld1.16  {d20},  [r0], r4 							//pSrc[1*16]...[3]
        vld1.16  {d21},  [r0], r11 							//pSrc[3*16]...[3]
        vld1.16  {d22},  [r0], r4  							//pSrc[5*16]...[3]
        vld1.16  {d23},  [r0], r11 							//pSrc[7*16]...[3]
        vld1.16  {d24},  [r0], r4 							//pSrc[9*16]...[3]
        vld1.16  {d25},  [r0], r11 							//pSrc[11*16]...[3]
        vld1.16  {d26},  [r0], r4  						//pSrc[13*16]...[3]
        vld1.16  {d27},  [r0], r10    						//pSrc[15*16]...[3]

        vmull.s16 q15, d20, d2[0]
        vmlal.s16 q15, d21, d2[1]
        vmlal.s16 q15, d22, d2[2]
        vmlal.s16 q15, d23, d2[3]
        vmlal.s16 q15, d24, d3[0]
        vmlal.s16 q15, d25, d3[1]
        vmlal.s16 q15, d26, d3[2]
        vmlal.s16 q15, d27, d3[3] 							// O[0]

        vadd.s32  q14, q2, q15  								// E[0] + O[0]
        vsub.s32  q2,  q2, q15  								// E[0] - O[0]
        vqrshrn.s32 d28, q14, #12 								// pTmpBlock[00~03]
        vqrshrn.s32 d29, q2,  #12 								// pTmpBlock[150~153]

        vmull.s16 q15, d20, d2[1]
        vmlal.s16 q15, d21, d3[0]
        vmlal.s16 q15, d22, d3[3]
        vmlsl.s16 q15, d23, d3[1]
        vmlsl.s16 q15, d24, d2[2]
        vmlsl.s16 q15, d25, d2[0]
        vmlsl.s16 q15, d26, d2[3]
        vmlsl.s16 q15, d27, d3[2] 							// O[1]

        vadd.s32  q2,  q3, q15  								// E[1] + O[1]
        vsub.s32  q3,  q3, q15  								// E[1] - O[1]
        vqrshrn.s32 d5, q2, #12  								// pTmpBlock[10~13]
        vqrshrn.s32 d4,  q3,  #12 								// pTmpBlock[140~143]

        vmull.s16 q3, d20, d2[2]
        vmlal.s16 q3, d21, d3[3]
        vmlsl.s16 q3, d22, d2[3]
        vmlsl.s16 q3, d23, d2[1]
        vmlsl.s16 q3, d24, d3[2]
        vmlal.s16 q3, d25, d3[0]
        vmlal.s16 q3, d26, d2[0]
        vmlal.s16 q3, d27, d3[1] 							// O[2]

        vadd.s32  q15, q4, q3  								// E[2] + O[2]
        vsub.s32  q3,  q4, q3  								// E[2] - O[2]
        vqrshrn.s32 d30, q15, #12 								// pTmpBlock[20~23]
        vqrshrn.s32 d31,  q3,  #12 								// pTmpBlock[130~133]


        vmull.s16 q4, d20, d2[3]
        vmlsl.s16 q4, d21, d3[1]
        vmlsl.s16 q4, d22, d2[1]
        vmlal.s16 q4, d23, d3[3]
        vmlal.s16 q4, d24, d2[0]
        vmlal.s16 q4, d25, d3[2]
        vmlsl.s16 q4, d26, d2[2]
        vmlsl.s16 q4, d27, d3[0] 							// O[3]

        vadd.s32  q3, q5, q4  								// E[3] + O[3]
        vsub.s32  q5, q5, q4  								// E[3] - O[3]
        vqrshrn.s32 d7, q3, #12 								// pTmpBlock[30~33]
        vqrshrn.s32 d6,  q5,  #12 								// pTmpBlock[120~123]

        vswp    d5, d29
        vswp    d7, d31
        vswp    q2, q3
       
        vtrn.32	d28,d30
        vtrn.32	d29,d31
        vtrn.16	d28,d29
        vtrn.16	d30,d31
        
        mov r8 ,#32
        mov r12,#24-96
        vst1.16 d28, [r5],r8 //a[0]
        vst1.16 d29, [r5],r8 //a[16]
        vst1.16 d30, [r5],r8 //a[32]
        vst1.16 d31, [r5],r12 //a[48]
        
        vtrn.32	d4,d6 
        vtrn.32	d5,d7
        vtrn.16	d4,d5
        vtrn.16	d6,d7
        
        vst1.16 d4, [r5],r8 //a[12]
        vst1.16 d5, [r5],r8 //a[28]
        vst1.16 d6, [r5],r8 //a[44]
        vst1.16 d7, [r5]	//a[60]

		sub r5,r5,#96+24-8  
		
        vmull.s16 q14, d20, d3[0]
        vmlsl.s16 q14, d21, d2[2]
        vmlsl.s16 q14, d22, d3[2]
        vmlal.s16 q14, d23, d2[0]
        vmlsl.s16 q14, d24, d3[3]
        vmlsl.s16 q14, d25, d2[1]
        vmlal.s16 q14, d26, d3[1]
        vmlal.s16 q14, d27, d2[3] 							// O[4]

        vadd.s32  q15, q9, q14  								// E[4] + O[4]
        vsub.s32  q14, q9, q14  								// E[4] - O[4]
        vqrshrn.s32 d4, q15, #12 								// pTmpBlock[40~43]
        vqrshrn.s32 d11,q14,  #12 								// pTmpBlock[110~113]


        vmull.s16 q14, d20, d3[1]
        vmlsl.s16 q14, d21, d2[0]
        vmlal.s16 q14, d22, d3[0]
        vmlal.s16 q14, d23, d3[2]
        vmlsl.s16 q14, d24, d2[1]
        vmlal.s16 q14, d25, d2[3]
        vmlal.s16 q14, d26, d3[3]
        vmlsl.s16 q14, d27, d2[2] 							// O[5]

        vadd.s32  q15, q8, q14  								// E[5] + O[5]
        vsub.s32  q14, q8, q14  								// E[5] - O[5]
        vqrshrn.s32 d5, q15, #12 								// pTmpBlock[50~53]
        vqrshrn.s32 d10,  q14,  #12 								// pTmpBlock[100~103]


        vmull.s16 q14, d20, d3[2]
        vmlsl.s16 q14, d21, d2[3]
        vmlal.s16 q14, d22, d2[0]
        vmlsl.s16 q14, d23, d2[2]
        vmlal.s16 q14, d24, d3[1]
        vmlal.s16 q14, d25, d3[3]
        vmlsl.s16 q14, d26, d3[0]
        vmlal.s16 q14, d27, d2[1] 							// O[6]

        vadd.s32  q15, q7, q14  								// E[6] + O[6]
        vsub.s32  q14,  q7, q14  								// E[6] - O[6]
        vqrshrn.s32 d6, q15, #12 								// pTmpBlock[60~63]
        vqrshrn.s32 d9,  q14,  #12 								// pTmpBlock[90~93]


        vmull.s16 q14, d20, d3[3]
        vmlsl.s16 q14, d21, d3[2]
        vmlal.s16 q14, d22, d3[1]
        vmlsl.s16 q14, d23, d3[0]
        vmlal.s16 q14, d24, d2[3]
        vmlsl.s16 q14, d25, d2[2]
        vmlal.s16 q14, d26, d2[1]
        vmlsl.s16 q14, d27, d2[0] 							// O[7]

        vadd.s32  q15, q6, q14  								// E[7] + O[7]
        vsub.s32  q14,  q6, q14  								// E[7] - O[7]
        vqrshrn.s32 d7, q15, #12 								// pTmpBlock[70~73]
        vqrshrn.s32 d8,  q14,  #12								// pTmpBlock[80~83]
        
        vtrn.32	d4,d6
        vtrn.32	d5,d7
        vtrn.16	d4,d5
        vtrn.16	d6,d7
        
        vst1.16 d4, [r5],r8    //a[4]
        vst1.16 d5, [r5],r8		//a[20]
        vst1.16 d6, [r5],r8		//a[36]
        vst1.16 d7, [r5]		//a[52]
        
        sub r5,r5,#96-8
        //vst4.16 {d8, d9,d10, d11},[r5],r6
		vtrn.32	d8,d10
        vtrn.32	d9,d11
        vtrn.16	d8,d9
        vtrn.16	d10,d11
        
        vst1.16 d8, [r5],r8  //a[8]-a[11]
        vst1.16 d9, [r5],r8  //a[24]-a[27]
        vst1.16 d10, [r5],r8 //a[40]-a[43]
        vst1.16 d11, [r5]	 //a[56]-a[59]
        
        add r5,r5,#16

        subs r9, r9, #1 													// j--
        bgt  IDCT16X16_4Row_ASM_loop_2

        add  sp, sp, #528

        ldmfd sp!, {r4, r5, r6, r7,r8,r9, r10,r11, pc}

endfunc

.align 4
tr4f:
.word 0x00240053  // 36 and d1[0] = 83
.word 0x00000000
tr8f:
.word 0x0059004b  // 89, d0[0] = 75
.word 0x00320012  // 50, d0[2] = 18
tr16:
.word 0x00530040,0x00240040,0x004b0059,0x00120032,0x0057005a,0x00460050,0x002b0039,0x00090019
