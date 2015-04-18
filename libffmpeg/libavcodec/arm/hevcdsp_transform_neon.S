#include "libavutil/arm/asm.S"
#include "neon.S"

.macro transform_add4x4_line
	vld1.32 d0,[r0]
	vld1.16 {d2,d3},[r1],r3
	vaddw.u8 q1,q1,d0
	vqmovun.s16 d0,q1
	vst1.32 D0[0],[r0],r2	
.endm

function transform_add4x4_neon , export=1
	mov r3,#8
	transform_add4x4_line
	transform_add4x4_line
	transform_add4x4_line
	transform_add4x4_line
	bx lr
endfunc

.macro transform_add16x16_line
	vld1.u8 D0,[r0]   				
	vld1.s16 {D2,D3},[r1]! 			
	vaddw.u8 q1,q1,d0
	vqmovun.s16 d0,q1
	vst1.u8 {d0},[r0]!
	
	vld1.u8 D0,[r0]
	vld1.s16 {D2,D3},[r1]!
	vaddw.u8 q1,q1,d0
	vqmovun.s16 d0,q1
	vst1.u8 {d0},[r0],r2
.endm

function transform_add16x16_neon , export=1
	sub r2,r2,#8
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	transform_add16x16_line
	bx lr
endfunc


.macro transform_add32x32_line
	vld1.u8 D0,[r0]
	vld1.s16 {D2,D3},[r1]!
	vaddw.u8 q1,q1,d0
	vqmovun.s16 d0,q1
	vst1.u8 {d0},[r0]!
	
	vld1.u8 D0,[r0]
	vld1.s16 {D2,D3},[r1]!
	vaddw.u8 q1,q1,d0
	vqmovun.s16 d0,q1
	vst1.u8 {d0},[r0]!
	
	vld1.u8 D0,[r0]
	vld1.s16 {D2,D3},[r1]!
	vaddw.u8 q1,q1,d0
	vqmovun.s16 d0,q1
	vst1.u8 {d0},[r0]!
	vld1.u8 D0,[r0] 
	vld1.s16 {D2,D3},[r1]!
	vaddw.u8 q1,q1,d0
	vqmovun.s16 d0,q1
	vst1.u8 {d0},[r0],r2
.endm

function transform_add32x32_neon , export=1
	sub r2,r2,#24
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	transform_add32x32_line
	bx lr
endfunc


function idct_4x4_dc_neon , export=1
	ldrsh r1,[r0]
	add r1,r1,#65
	asr r1,r1,#7
	vdup.s32 q0,r1
	vmovn.s32 d0,q0
	vmov.32 d1,d0
	vmov.32 q1,q0
	vst1.32 {d0-d3},[r0]! //16*1
	bx lr
endfunc

function idct_8x8_dc_neon , export=1
	ldrsh r1,[r0]
	add r1,r1,#65
	asr r1,r1,#7
	vdup.s32 q0,r1
	vmovn.s32 d0,q0
	vmov.32 d1,d0
	vmov.32 q1,q0
	vst1.32 {d0-d3},[r0]! //16*4
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	bx lr
endfunc


function idct_16x16_dc_neon , export=1
	ldrsh r1,[r0]
	add r1,r1,#65
	asr r1,r1,#7
	vdup.s32 q0,r1
	vmovn.s32 d0,q0
	vmov.32 d1,d0
	vmov.32 q1,q0	//16*16
	vst1.32 {d0-d3},[r0]! //16*4
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]! //16*4
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]! //16*4
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]! //16*4
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	bx lr
endfunc

function idct_32x32_dc_neon , export=1
	ldrsh r1,[r0]
	add r1,r1,#65
	asr r1,r1,#7
	vdup.s32 q0,r1
	vmovn.s32 d0,q0
	vmov.32 d1,d0
	vmov.32 q1,q0
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
		vst1.32 {d0-d3},[r0]!
	vst1.32 {d0-d3},[r0]!
	bx lr
endfunc

.macro my_transpose_16_8x8
    vswp			D1,D8
	vswp 			D3,D10
	vswp 			D5,D12
	vswp 			D7,D14
    vtrn.32         Q0, Q2
    vtrn.32         Q1, Q3
    vtrn.32         Q4, Q6
    vtrn.32         Q5, Q7
    vtrn.16         Q0, Q1
    vtrn.16         Q2, Q3
    vtrn.16         Q4, Q5
    vtrn.16         Q6, Q7
.endm
	
.macro transform_rdpcm_8x8_mode0_first	
	vld1.16		{D0-D1},[r0],r1
	vld1.16		{D2-D3},[r0],r1
	vld1.16		{D4-D5},[r0],r1
	vld1.16		{D6-D7},[r0],r1
	vld1.16		{D8-D9},[r0],r1
	vld1.16		{D10-D11},[r0],r1
	vld1.16		{D12-D13},[r0],r1
	vld1.16		{D14-D15},[r0],r2
	
	my_transpose_16_8x8
	
	vadd.s16	Q1,Q0,Q1
	vadd.s16	Q2,Q1,Q2
	vadd.s16	Q3,Q2,Q3
	vadd.s16	Q4,Q3,Q4
	vadd.s16	Q5,Q4,Q5
	vadd.s16	Q6,Q5,Q6
	vadd.s16	Q7,Q6,Q7
	
	vmov Q8,Q7
	my_transpose_16_8x8
	
	vst1.16		{D0-D1},[r0],r1
	vst1.16		{D2-D3},[r0],r1
	vst1.16		{D4-D5},[r0],r1
	vst1.16		{D6-D7},[r0],r1
	vst1.16		{D8-D9},[r0],r1
	vst1.16		{D10-D11},[r0],r1
	vst1.16		{D12-D13},[r0],r1
	vst1.16		{D14-D15},[r0],r3
.endm

.macro transform_rdpcm_8x8_mode0_not_first

	vld1.16		{D0-D1},[r0],r1
	vld1.16		{D2-D3},[r0],r1
	vld1.16		{D4-D5},[r0],r1
	vld1.16		{D6-D7},[r0],r1
	vld1.16		{D8-D9},[r0],r1
	vld1.16		{D10-D11},[r0],r1
	vld1.16		{D12-D13},[r0],r1
	vld1.16		{D14-D15},[r0],r2
	
	my_transpose_16_8x8
	
	vadd.s16	Q0,Q8,Q0
	vadd.s16	Q1,Q0,Q1
	vadd.s16	Q2,Q1,Q2
	vadd.s16	Q3,Q2,Q3
	vadd.s16	Q4,Q3,Q4
	vadd.s16	Q5,Q4,Q5
	vadd.s16	Q6,Q5,Q6
	vadd.s16	Q7,Q6,Q7
	vmov.s16    Q8,Q7
	
	my_transpose_16_8x8
	
	vst1.16		{D0-D1},[r0],r1
	vst1.16		{D2-D3},[r0],r1
	vst1.16		{D4-D5},[r0],r1
	vst1.16		{D6-D7},[r0],r1
	vst1.16		{D8-D9},[r0],r1
	vst1.16		{D10-D11},[r0],r1
	vst1.16		{D12-D13},[r0],r1
	vst1.16		{D14-D15},[r0],r3
.endm

.macro	transform_rdpcm_8x8 start,stride,back	
	vld1.16		{D0-D1},[\start],\stride
	vld1.16		{D2-D3},[\start],\stride
	vld1.16		{D4-D5},[\start],\stride
	vld1.16		{D6-D7},[\start],\stride
	vld1.16		{D8-D9},[\start],\stride
	vld1.16		{D10-D11},[\start],\stride
	vld1.16		{D12-D13},[\start],\stride
	vld1.16		{D14-D15},[\start],\back
	
	vadd.s16	Q1,Q0,Q1
	vadd.s16	Q2,Q1,Q2
	vadd.s16	Q3,Q2,Q3
	vadd.s16	Q4,Q3,Q4
	vadd.s16	Q5,Q4,Q5
	vadd.s16	Q6,Q5,Q6
	vadd.s16	Q7,Q6,Q7
	
	vst1.16		{D2-D3},[\start],\stride
	vst1.16		{D4-D5},[\start],\stride
	vst1.16		{D6-D7},[\start],\stride
	vst1.16		{D8-D9},[\start],\stride
	vst1.16		{D10-D11},[\start],\stride
	vst1.16		{D12-D13},[\start],\stride
	vst1.16		{D14-D15},[\start],\stride	
.endm

function transform_skip_neon , export=1
	rsb r2,r1,#6
	mov r3,#1
	lsl r2,r3,r2
	vdup.16 D1,r2
	lsl r2,r3,r1
	rsb r3,r1,#7
	vdup.16 d3,r3
	vneg.s16 d3,d3
transform_skip_neon_loop:
	vld1.16		d0,[r0]
	vadd.s16	d2,d0,d1
	vshl.s16	d2,d2,d3
	vst1.16		d2,[r0]!
	subs r2,r2,#1
	bgt transform_skip_neon_loop
	bx lr
endfunc

function transform_rdpcm_neon , export=1
	mov r3,#1
	lsl r3,r3,r1
	lsl r1,r3,#1
	cmp r2,#0
	bne transform_rdpcm_neon_mode1
	cmp r3,#4
	beq transform_rdpcm_mode0_case4
	cmp r3,#8
	beq transform_rdpcm_mode0_case8
	cmp r3,#16
	beq transform_rdpcm_mode0_case16
	b transform_rdpcm_mode0_case32	

transform_rdpcm_mode0_case4:
	mov r2,#-24
	vld1.16		D0,[r0]!
	vld1.16		D1,[r0]!
	vld1.16		D2,[r0]!
	vld1.16		D3,[r0],r2
		
	vtrn.32		D0,D2
	vtrn.32		D1,D3
	vtrn.16		D0,D1
	vtrn.16		D2,D3
	
	vadd.s16	D1,D0,D1
	vadd.s16	D2,D1,D2
	vadd.s16	D3,D2,D3
	
	vtrn.32		D0,D2
	vtrn.32		D1,D3
	vtrn.16		D0,D1
	vtrn.16		D2,D3
	
	vst1.16		D0,[r0]!
	vst1.16		D1,[r0]!
	vst1.16		D2,[r0]!
	vst1.16		D3,[r0]
	bx lr
transform_rdpcm_mode0_case8:
	mov  r2,    #-16*7
	vld1.16		{D0-D1},[r0]!
	vld1.16		{D2-D3},[r0]!
	vld1.16		{D4-D5},[r0]!
	vld1.16		{D6-D7},[r0]!
	vld1.16		{D8-D9},[r0]!
	vld1.16		{D10-D11},[r0]!
	vld1.16		{D12-D13},[r0]!
	vld1.16		{D14-D15},[r0],r2
	
	my_transpose_16_8x8
	
	vadd.s16	Q1,Q0,Q1
	vadd.s16	Q2,Q1,Q2
	vadd.s16	Q3,Q2,Q3
	vadd.s16	Q4,Q3,Q4
	vadd.s16	Q5,Q4,Q5
	vadd.s16	Q6,Q5,Q6
	vadd.s16	Q7,Q6,Q7
	
	my_transpose_16_8x8
	
	vst1.16		{D0-D1},[r0]!
	vst1.16		{D2-D3},[r0]!
	vst1.16		{D4-D5},[r0]!
	vst1.16		{D6-D7},[r0]!
	vst1.16		{D8-D9},[r0]!
	vst1.16		{D10-D11},[r0]!
	vst1.16		{D12-D13},[r0]!
	vst1.16		{D14-D15},[r0]
	bx  lr
transform_rdpcm_mode0_case16:
	mov r1,		#16*2
	mov r2,		#-16*7*2
	mov r3,		#-16*7*2+8*2
	transform_rdpcm_8x8_mode0_first
	mov r3,#8*2
	transform_rdpcm_8x8_mode0_not_first
	mov r3,#-16*7*2+8*2
	transform_rdpcm_8x8_mode0_first
	mov r3,#8*2
	transform_rdpcm_8x8_mode0_not_first
	
	
	bx lr
transform_rdpcm_mode0_case32:
	mov r1,		#32*2
	mov r2,		#32*7*2
	rsb r2,r2,#0
	
	add r3,r2,#16
	transform_rdpcm_8x8_mode0_first
	transform_rdpcm_8x8_mode0_not_first
	transform_rdpcm_8x8_mode0_not_first
	mov r3,#8*2
	transform_rdpcm_8x8_mode0_not_first
	
	add r3,r2,#16
	transform_rdpcm_8x8_mode0_first
	transform_rdpcm_8x8_mode0_not_first
	transform_rdpcm_8x8_mode0_not_first
	mov r3,#8*2
	transform_rdpcm_8x8_mode0_not_first
	
	add r3,r2,#16
	transform_rdpcm_8x8_mode0_first
	transform_rdpcm_8x8_mode0_not_first
	transform_rdpcm_8x8_mode0_not_first
	mov r3,#8*2
	transform_rdpcm_8x8_mode0_not_first
	
	add r3,r2,#16
	transform_rdpcm_8x8_mode0_first
	transform_rdpcm_8x8_mode0_not_first
	transform_rdpcm_8x8_mode0_not_first
	mov r3,#8*2
	transform_rdpcm_8x8_mode0_not_first
	
	bx lr
transform_rdpcm_neon_mode1:
	cmp r3,#4
	beq transform_rdpcm_neon_mode1_case4
	cmp r3,#8
	beq transform_rdpcm_neon_mode1_case8
	cmp r3,#16
	beq transform_rdpcm_neon_mode1_case16
	b transform_rdpcm_neon_mode1_case32
	
transform_rdpcm_neon_mode1_case4:
	mov r2,#-16
	vld1.16		D0,[r0]!
	vld1.16		D1,[r0]!
	vld1.16		D2,[r0]!
	vld1.16		D3,[r0],r2
	vadd.s16	D1,D0,D1
	vadd.s16	D2,D1,D2
	vadd.s16	D3,D2,D3
	vst1.16		D1,[r0]!
	vst1.16		D2,[r0]!
	vst1.16		D3,[r0]
	bx lr
	
transform_rdpcm_neon_mode1_case8:
	mov r2,#-16*6
	transform_rdpcm_8x8 r0,r1,r2
	bx lr
transform_rdpcm_neon_mode1_case16:
	mov r2,#-6*32

	vld1.16		{D0-D3},[r0]!
	vld1.16		{D4-D7},[r0]!
	vld1.16		{D8-D11},[r0]!
	vld1.16		{D12-D15},[r0]!
	vld1.16		{D16-D19},[r0]!
	vld1.16		{D20-D23},[r0]!
	vld1.16		{D24-D27},[r0]!
	vld1.16		{D28-D31},[r0],r2
	
	vadd.s16	Q2,Q0,Q2
	vadd.s16	Q4,Q2,Q4
	vadd.s16	Q6,Q4,Q6
	vadd.s16	Q8,Q6,Q8
	vadd.s16	Q10,Q8,Q10
	vadd.s16	Q12,Q10,Q12
	vadd.s16	Q14,Q12,Q14
	
	vadd.s16	Q3,Q1,Q3
	vadd.s16	Q5,Q3,Q5
	vadd.s16	Q7,Q5,Q7
	vadd.s16	Q9,Q7,Q9
	vadd.s16	Q11,Q9,Q11
	vadd.s16	Q13,Q11,Q13
	vadd.s16	Q15,Q13,Q15
	
	vst1.16		{D4-D7},[r0]!
	vst1.16		{D8-D11},[r0]!
	vst1.16		{D12-D15},[r0]!
	vst1.16		{D16-D19},[r0]!
	vst1.16		{D20-D23},[r0]!
	vst1.16		{D24-D27},[r0]!
	vst1.16		{D28-D31},[r0]!	
	
	sub r2,r2,#32
		
	vld1.16		{D0-D3},[r0]!
	vld1.16		{D4-D7},[r0]!
	vld1.16		{D8-D11},[r0]!
	vld1.16		{D12-D15},[r0]!
	vadd.s16	Q0,Q14,Q0
	vadd.s16	Q1,Q15,Q1
	vld1.16		{D16-D19},[r0]!
	vld1.16		{D20-D23},[r0]!
	vld1.16		{D24-D27},[r0]!
	vld1.16		{D28-D31},[r0],r2
	
	vadd.s16	Q2,Q0,Q2
	vadd.s16	Q4,Q2,Q4
	vadd.s16	Q6,Q4,Q6
	vadd.s16	Q8,Q6,Q8
	vadd.s16	Q10,Q8,Q10
	vadd.s16	Q12,Q10,Q12
	vadd.s16	Q14,Q12,Q14
	
	vadd.s16	Q3,Q1,Q3
	vadd.s16	Q5,Q3,Q5
	vadd.s16	Q7,Q5,Q7
	vadd.s16	Q9,Q7,Q9
	vadd.s16	Q11,Q9,Q11
	vadd.s16	Q13,Q11,Q13
	vadd.s16	Q15,Q13,Q15
	
	vst1.16		{D0-D3},[r0]!
	vst1.16		{D4-D7},[r0]!
	vst1.16		{D8-D11},[r0]!
	vst1.16		{D12-D15},[r0]!
	vst1.16		{D16-D19},[r0]!
	vst1.16		{D20-D23},[r0]!
	vst1.16		{D24-D27},[r0]!
	vst1.16		{D28-D31},[r0]!

	bx lr
	
transform_rdpcm_neon_mode1_case32:
	
	mov r3,#64
	mov r2,#6*64
	rsb r2,r2,#0
	
	vld1.16		{D0-D3},[r0],r3
	vld1.16		{D4-D7},[r0],r3
	vld1.16		{D8-D11},[r0],r3
	vld1.16		{D12-D15},[r0],r3
	vld1.16		{D16-D19},[r0],r3
	vld1.16		{D20-D23},[r0],r3
	vld1.16		{D24-D27},[r0],r3
	vld1.16		{D28-D31},[r0],r2
	
	vadd.s16	Q2,Q0,Q2
	vadd.s16	Q4,Q2,Q4
	vadd.s16	Q6,Q4,Q6
	vadd.s16	Q8,Q6,Q8
	vadd.s16	Q10,Q8,Q10
	vadd.s16	Q12,Q10,Q12
	vadd.s16	Q14,Q12,Q14
	
	vadd.s16	Q3,Q1,Q3
	vadd.s16	Q5,Q3,Q5
	vadd.s16	Q7,Q5,Q7
	vadd.s16	Q9,Q7,Q9
	vadd.s16	Q11,Q9,Q11
	vadd.s16	Q13,Q11,Q13
	vadd.s16	Q15,Q13,Q15
	
	vst1.16		{D4-D7},[r0],r3
	vst1.16		{D8-D11},[r0],r3
	vst1.16		{D12-D15},[r0],r3
	vst1.16		{D16-D19},[r0],r3
	vst1.16		{D20-D23},[r0],r3
	vst1.16		{D24-D27},[r0],r3
	vst1.16		{D28-D31},[r0],r3
	
	sub r2,r2,#64
		
	vld1.16		{D0-D3},[r0],r3
	vld1.16		{D4-D7},[r0],r3
	vld1.16		{D8-D11},[r0],r3
	vld1.16		{D12-D15},[r0],r3
	vadd.s16	Q0,Q14,Q0
	vadd.s16	Q1,Q15,Q1
	vld1.16		{D16-D19},[r0],r3
	vld1.16		{D20-D23},[r0],r3
	vld1.16		{D24-D27},[r0],r3
	vld1.16		{D28-D31},[r0],r2
	
	vadd.s16	Q2,Q0,Q2
	vadd.s16	Q4,Q2,Q4
	vadd.s16	Q6,Q4,Q6
	vadd.s16	Q8,Q6,Q8
	vadd.s16	Q10,Q8,Q10
	vadd.s16	Q12,Q10,Q12
	vadd.s16	Q14,Q12,Q14
	
	vadd.s16	Q3,Q1,Q3
	vadd.s16	Q5,Q3,Q5
	vadd.s16	Q7,Q5,Q7
	vadd.s16	Q9,Q7,Q9
	vadd.s16	Q11,Q9,Q11
	vadd.s16	Q13,Q11,Q13
	vadd.s16	Q15,Q13,Q15
	
	vst1.16		{D0-D3},[r0],r3
	vst1.16		{D4-D7},[r0],r3
	vst1.16		{D8-D11},[r0],r3
	vst1.16		{D12-D15},[r0],r3
	vst1.16		{D16-D19},[r0],r3
	vst1.16		{D20-D23},[r0],r3
	vst1.16		{D24-D27},[r0],r3
	vst1.16		{D28-D31},[r0],r3 

	vld1.16		{D0-D3},[r0],r3
	vld1.16		{D4-D7},[r0],r3
	vld1.16		{D8-D11},[r0],r3
	vld1.16		{D12-D15},[r0],r3
	vadd.s16	Q0,Q14,Q0
	vadd.s16	Q1,Q15,Q1
	vld1.16		{D16-D19},[r0],r3
	vld1.16		{D20-D23},[r0],r3
	vld1.16		{D24-D27},[r0],r3
	vld1.16		{D28-D31},[r0],r2
	
	vadd.s16	Q2,Q0,Q2
	vadd.s16	Q4,Q2,Q4
	vadd.s16	Q6,Q4,Q6
	vadd.s16	Q8,Q6,Q8
	vadd.s16	Q10,Q8,Q10
	vadd.s16	Q12,Q10,Q12
	vadd.s16	Q14,Q12,Q14
	
	vadd.s16	Q3,Q1,Q3
	vadd.s16	Q5,Q3,Q5
	vadd.s16	Q7,Q5,Q7
	vadd.s16	Q9,Q7,Q9
	vadd.s16	Q11,Q9,Q11
	vadd.s16	Q13,Q11,Q13
	vadd.s16	Q15,Q13,Q15
	
	vst1.16		{D0-D3},[r0],r3
	vst1.16		{D4-D7},[r0],r3
	vst1.16		{D8-D11},[r0],r3
	vst1.16		{D12-D15},[r0],r3
	vst1.16		{D16-D19},[r0],r3
	vst1.16		{D20-D23},[r0],r3
	vst1.16		{D24-D27},[r0],r3
	vst1.16		{D28-D31},[r0],r3
	
	vld1.16		{D0-D3},[r0],r3
	vld1.16		{D4-D7},[r0],r3
	vld1.16		{D8-D11},[r0],r3
	vld1.16		{D12-D15},[r0],r3
	vadd.s16	Q0,Q14,Q0
	vadd.s16	Q1,Q15,Q1
	vld1.16		{D16-D19},[r0],r3
	vld1.16		{D20-D23},[r0],r3
	vld1.16		{D24-D27},[r0],r3
	vld1.16		{D28-D31},[r0],r2
	
	vadd.s16	Q2,Q0,Q2
	vadd.s16	Q4,Q2,Q4
	vadd.s16	Q6,Q4,Q6
	vadd.s16	Q8,Q6,Q8
	vadd.s16	Q10,Q8,Q10
	vadd.s16	Q12,Q10,Q12
	vadd.s16	Q14,Q12,Q14
	
	vadd.s16	Q3,Q1,Q3
	vadd.s16	Q5,Q3,Q5
	vadd.s16	Q7,Q5,Q7
	vadd.s16	Q9,Q7,Q9
	vadd.s16	Q11,Q9,Q11
	vadd.s16	Q13,Q11,Q13
	vadd.s16	Q15,Q13,Q15
	
	vst1.16		{D0-D3},[r0],r3
	vst1.16		{D4-D7},[r0],r3
	vst1.16		{D8-D11},[r0],r3
	vst1.16		{D12-D15},[r0],r3
	vst1.16		{D16-D19},[r0],r3
	vst1.16		{D20-D23},[r0],r3
	vst1.16		{D24-D27},[r0],r3
	vst1.16		{D28-D31},[r0],r3
	
	sub r0,r0,#32*32*2
	add r0,r0,#32
	
	add r2,r2,#64

	vld1.16		{D0-D3},[r0],r3
	vld1.16		{D4-D7},[r0],r3
	vld1.16		{D8-D11},[r0],r3
	vld1.16		{D12-D15},[r0],r3
	vld1.16		{D16-D19},[r0],r3
	vld1.16		{D20-D23},[r0],r3
	vld1.16		{D24-D27},[r0],r3
	vld1.16		{D28-D31},[r0],r2
	
	vadd.s16	Q2,Q0,Q2
	vadd.s16	Q4,Q2,Q4
	vadd.s16	Q6,Q4,Q6
	vadd.s16	Q8,Q6,Q8
	vadd.s16	Q10,Q8,Q10
	vadd.s16	Q12,Q10,Q12
	vadd.s16	Q14,Q12,Q14
	
	vadd.s16	Q3,Q1,Q3
	vadd.s16	Q5,Q3,Q5
	vadd.s16	Q7,Q5,Q7
	vadd.s16	Q9,Q7,Q9
	vadd.s16	Q11,Q9,Q11
	vadd.s16	Q13,Q11,Q13
	vadd.s16	Q15,Q13,Q15
	
	vst1.16		{D4-D7},[r0],r3
	vst1.16		{D8-D11},[r0],r3
	vst1.16		{D12-D15},[r0],r3
	vst1.16		{D16-D19},[r0],r3
	vst1.16		{D20-D23},[r0],r3
	vst1.16		{D24-D27},[r0],r3
	vst1.16		{D28-D31},[r0],r3
	
	sub r2,r2,#64
		
	vld1.16		{D0-D3},[r0],r3
	vld1.16		{D4-D7},[r0],r3
	vld1.16		{D8-D11},[r0],r3
	vld1.16		{D12-D15},[r0],r3
	vadd.s16	Q0,Q14,Q0
	vadd.s16	Q1,Q15,Q1
	vld1.16		{D16-D19},[r0],r3
	vld1.16		{D20-D23},[r0],r3
	vld1.16		{D24-D27},[r0],r3
	vld1.16		{D28-D31},[r0],r2
	
	vadd.s16	Q2,Q0,Q2
	vadd.s16	Q4,Q2,Q4
	vadd.s16	Q6,Q4,Q6
	vadd.s16	Q8,Q6,Q8
	vadd.s16	Q10,Q8,Q10
	vadd.s16	Q12,Q10,Q12
	vadd.s16	Q14,Q12,Q14
	
	vadd.s16	Q3,Q1,Q3
	vadd.s16	Q5,Q3,Q5
	vadd.s16	Q7,Q5,Q7
	vadd.s16	Q9,Q7,Q9
	vadd.s16	Q11,Q9,Q11
	vadd.s16	Q13,Q11,Q13
	vadd.s16	Q15,Q13,Q15
	
	vst1.16		{D0-D3},[r0],r3
	vst1.16		{D4-D7},[r0],r3
	vst1.16		{D8-D11},[r0],r3
	vst1.16		{D12-D15},[r0],r3
	vst1.16		{D16-D19},[r0],r3
	vst1.16		{D20-D23},[r0],r3
	vst1.16		{D24-D27},[r0],r3
	vst1.16		{D28-D31},[r0],r3
	
	vld1.16		{D0-D3},[r0],r3
	vld1.16		{D4-D7},[r0],r3
	vld1.16		{D8-D11},[r0],r3
	vld1.16		{D12-D15},[r0],r3
	vadd.s16	Q0,Q14,Q0
	vadd.s16	Q1,Q15,Q1
	vld1.16		{D16-D19},[r0],r3
	vld1.16		{D20-D23},[r0],r3
	vld1.16		{D24-D27},[r0],r3
	vld1.16		{D28-D31},[r0],r2
	
	vadd.s16	Q2,Q0,Q2
	vadd.s16	Q4,Q2,Q4
	vadd.s16	Q6,Q4,Q6
	vadd.s16	Q8,Q6,Q8
	vadd.s16	Q10,Q8,Q10
	vadd.s16	Q12,Q10,Q12
	vadd.s16	Q14,Q12,Q14
	
	vadd.s16	Q3,Q1,Q3
	vadd.s16	Q5,Q3,Q5
	vadd.s16	Q7,Q5,Q7
	vadd.s16	Q9,Q7,Q9
	vadd.s16	Q11,Q9,Q11
	vadd.s16	Q13,Q11,Q13
	vadd.s16	Q15,Q13,Q15
	
	vst1.16		{D0-D3},[r0],r3
	vst1.16		{D4-D7},[r0],r3
	vst1.16		{D8-D11},[r0],r3
	vst1.16		{D12-D15},[r0],r3
	vst1.16		{D16-D19},[r0],r3
	vst1.16		{D20-D23},[r0],r3
	vst1.16		{D24-D27},[r0],r3
	vst1.16		{D28-D31},[r0],r3	
	
	vld1.16		{D0-D3},[r0],r3
	vld1.16		{D4-D7},[r0],r3
	vld1.16		{D8-D11},[r0],r3
	vld1.16		{D12-D15},[r0],r3
	vadd.s16	Q0,Q14,Q0
	vadd.s16	Q1,Q15,Q1
	vld1.16		{D16-D19},[r0],r3
	vld1.16		{D20-D23},[r0],r3
	vld1.16		{D24-D27},[r0],r3
	vld1.16		{D28-D31},[r0],r2
	
	vadd.s16	Q2,Q0,Q2
	vadd.s16	Q4,Q2,Q4
	vadd.s16	Q6,Q4,Q6
	vadd.s16	Q8,Q6,Q8
	vadd.s16	Q10,Q8,Q10
	vadd.s16	Q12,Q10,Q12
	vadd.s16	Q14,Q12,Q14
	
	vadd.s16	Q3,Q1,Q3
	vadd.s16	Q5,Q3,Q5
	vadd.s16	Q7,Q5,Q7
	vadd.s16	Q9,Q7,Q9
	vadd.s16	Q11,Q9,Q11
	vadd.s16	Q13,Q11,Q13
	vadd.s16	Q15,Q13,Q15
	
	vst1.16		{D0-D3},[r0],r3
	vst1.16		{D4-D7},[r0],r3
	vst1.16		{D8-D11},[r0],r3
	vst1.16		{D12-D15},[r0],r3
	vst1.16		{D16-D19},[r0],r3
	vst1.16		{D20-D23},[r0],r3
	vst1.16		{D24-D27},[r0],r3
	vst1.16		{D28-D31},[r0],r3
	
	bx lr
endfunc

function transform_4x4_luma_neon , export=1
	vld1.16  {d0-d3}, [r0]
        
        ldr       r12, = transform_4x4_luma_table
        vld1.16  {d14-d17}, [r12]
        
        vmull.s16 q4, d14, d0[0] 						
        vmlal.s16 q4, d15, d1[0] 						
        vmlal.s16 q4, d16, d2[0] 						
        vmlal.s16 q4, d17, d3[0] 						
        vqrshrn.s32 d4, q4, #7 						
        
        vmull.s16 q4, d14, d0[1] 						
        vmlal.s16 q4, d15, d1[1] 						
        vmlal.s16 q4, d16, d2[1] 						
        vmlal.s16 q4, d17, d3[1] 						
        vqrshrn.s32 d5, q4, #7 						
        
        vmull.s16 q4, d14, d0[2] 						
        vmlal.s16 q4, d15, d1[2] 						
        vmlal.s16 q4, d16, d2[2] 						
        vmlal.s16 q4, d17, d3[2] 						
        vqrshrn.s32 d6, q4, #7 						
        
        vmull.s16 q4, d14, d0[3] 						
        vmlal.s16 q4, d15, d1[3] 						
        vmlal.s16 q4, d16, d2[3] 						
        vmlal.s16 q4, d17, d3[3] 						
        vqrshrn.s32 d7, q4, #7 						
        
        vmull.s16 q4, d14, d4[0] 						
        vmlal.s16 q4, d15, d5[0] 						
        vmlal.s16 q4, d16, d6[0] 						
        vmlal.s16 q4, d17, d7[0] 						
        vqrshrn.s32 d0, q4, #12 						
        
        vmull.s16 q4, d14, d4[1] 						
        vmlal.s16 q4, d15, d5[1] 						
        vmlal.s16 q4, d16, d6[1] 						
        vmlal.s16 q4, d17, d7[1] 						
        vqrshrn.s32 d1, q4, #12 						
        
        vmull.s16 q4, d14, d4[2] 						
        vmlal.s16 q4, d15, d5[2] 						
        vmlal.s16 q4, d16, d6[2] 						
        vmlal.s16 q4, d17, d7[2] 						
        vqrshrn.s32 d2, q4, #12 						
        
        vmull.s16 q4, d14, d4[3] 						
        vmlal.s16 q4, d15, d5[3] 						
        vmlal.s16 q4, d16, d6[3] 						
        vmlal.s16 q4, d17, d7[3] 						
        vqrshrn.s32 d3, q4, #12 						

        vst1.32      {d0-d3}, [r0]!  
        bx lr
endfunc

transform_4x4_luma_table:
.word 0x0037001d,0x0054004a,0x004a004a,0xffb60000,0xffe30054,0x0037ffb6,0xffac0037,0xffe3004a
