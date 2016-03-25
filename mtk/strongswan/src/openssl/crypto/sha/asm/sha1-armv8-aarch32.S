#include "arm_arch.h"

#if __ARM_ARCH__>=7

.text
.code   32

# SHA1 assembly implementation for ARMv8 AArch32

.global	sha1_block_data_order
.type	sha1_block_data_order,%function
.align	2
sha1_block_data_order:

.Lsha1prolog:

    adr         r3, .LKConstant
    add         r2, r1, r2, lsl #6
    vld1.32     {d16[],d17[]}, [r3]!
    vpush       {q4, q5, q6}
    vld1.32     {d18[],d19[]}, [r3]!
    vldmia      r0, {s0, s1, s2, s3, s4}
    vld1.32     {d20[],d21[]}, [r3]!
    vld1.32     {d22[],d23[]}, [r3]!

.Lsha1loop:

    vld1.32     {d24-d27}, [r1]!
    vmov        q2, q0
    vmov        q3, q1
    vld1.32     {d28-d31}, [r1]!

#ifdef __ARMEL__
    vrev32.8    q12, q12
#endif
#ifdef __ARMEL__
    vrev32.8    q13, q13
#endif
    vadd.i32    q5, q8, q12
#ifdef __ARMEL__
    vrev32.8    q14, q14
#endif
    vadd.i32    q6, q8, q13
    sha1h.32    q4, q2
    sha1c.32    q2, q3, q5
    sha1su0.32  q12, q13, q14
#ifdef __ARMEL__
    vrev32.8    q15, q15
#endif
    vadd.i32    q5, q8, q14
    sha1h.32    q3, q2
    sha1c.32    q2, q4, q6
    sha1su0.32  q13, q14, q15
    sha1su1.32  q12, q15
    vadd.i32    q6, q8, q15
    sha1h.32    q4, q2
    sha1c.32    q2, q3, q5
    sha1su0.32  q14, q15, q12
    sha1su1.32  q13, q12
    vadd.i32    q5, q8, q12
    sha1h.32    q3, q2
    sha1c.32    q2, q4, q6
    sha1su0.32  q15, q12, q13
    sha1su1.32  q14, q13
    vadd.i32    q6, q9, q13
    sha1h.32    q4, q2
    sha1c.32    q2, q3, q5
    sha1su0.32  q12, q13, q14
    sha1su1.32  q15, q14
    vadd.i32    q5, q9, q14
    sha1h.32    q3, q2
    sha1p.32    q2, q4, q6
    sha1su0.32  q13, q14, q15
    sha1su1.32  q12, q15
    vadd.i32    q6, q9, q15
    sha1h.32    q4, q2
    sha1p.32    q2, q3, q5
    sha1su0.32  q14, q15, q12
    sha1su1.32  q13, q12
    vadd.i32    q5, q9, q12
    sha1h.32    q3, q2
    sha1p.32    q2, q4, q6
    sha1su0.32  q15, q12, q13
    sha1su1.32  q14, q13
    vadd.i32    q6, q9, q13
    sha1h.32    q4, q2
    sha1p.32    q2, q3, q5
    sha1su0.32  q12, q13, q14
    sha1su1.32  q15, q14
    vadd.i32    q5, q10, q14
    sha1h.32    q3, q2
    sha1p.32    q2, q4, q6
    sha1su0.32  q13, q14, q15
    sha1su1.32  q12, q15
    vadd.i32    q6, q10, q15
    sha1h.32    q4, q2
    sha1m.32    q2, q3, q5
    sha1su0.32  q14, q15, q12  
    sha1su1.32  q13, q12
    vadd.i32    q5, q10, q12
    sha1h.32    q3, q2
    sha1m.32    q2, q4, q6
    sha1su0.32  q15, q12, q13
    sha1su1.32  q14, q13
    vadd.i32    q6, q10, q13
    sha1h.32    q4, q2
    sha1m.32    q2, q3, q5
    sha1su0.32  q12, q13, q14
    sha1su1.32  q15, q14
    vadd.i32    q5, q10, q14
    sha1h.32    q3, q2
    sha1m.32    q2, q4, q6
    sha1su0.32  q13, q14, q15
    sha1su1.32  q12, q15
    vadd.i32    q6, q11, q15
    sha1h.32    q4, q2
    sha1m.32    q2, q3, q5
    sha1su0.32  q14, q15, q12
    sha1su1.32  q13, q12
    vadd.i32    q5, q11, q12
    sha1h.32    q3, q2
    sha1p.32    q2, q4, q6
    sha1su0.32  q15, q12, q13
    sha1su1.32  q14, q13
    vadd.i32    q6, q11, q13
    sha1h.32    q4, q2
    sha1p.32    q2, q3, q5
    sha1su1.32  q15, q14
    vadd.i32    q5, q11, q14
    sha1h.32    q3, q2
    sha1p.32    q2, q4, q6
    vadd.i32    q6, q11, q15
    sha1h.32    q4, q2
    sha1p.32    q2, q3, q5
    sha1h.32    q3, q2
    sha1p.32    q2, q4, q6
    teq         r1, r2
    vadd.i32    q1, q1, q3
    vadd.i32    q0, q0, q2
    bne         .Lsha1loop

.Lsha1epilog:

    vstmia      r0, {s0, s1, s2, s3, s4}
    vpop        {q4, q5, q6}
    bx          lr

.align  2
.LKConstant:
.word   0x5a827999
.word   0x6ed9eba1
.word   0x8f1bbcdc
.word   0xca62c1d6

.size	  sha1_block_data_order,.-sha1_block_data_order
.align	2

#endif
