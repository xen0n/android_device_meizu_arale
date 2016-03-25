#include "arm_arch.h"

#if __ARM_ARCH__>=7

.text
.arch   armv8-a+crypto

# SHA1 assembly implementation for ARMv8 AArch64

.global	sha1_block_data_order
.type	sha1_block_data_order,%function
.align	2
sha1_block_data_order:

.Lsha1prolog:

    adr     x3, .LKConstant
    add     x2, x1, x2, lsl #6
    ld1     {v0.4s}, [x0], #16
    ld1     {v1.s}[0], [x0]
    sub     x0, x0, #16
    ld1     {v16.4s-v19.4s}, [x3]

.Lsha1loop:

    ld1     {v20.16b-v23.16b}, [x1], #64
    mov     v2.16b, v0.16b
    mov     v3.s[0], v1.s[0]

#ifdef __ARMEL__
    rev32   v20.16b, v20.16b
#endif
#ifdef __ARMEL__
    rev32   v21.16b, v21.16b
#endif
    add     v24.4s, v16.4s, v20.4s
#ifdef __ARMEL__
    rev32   v22.16b, v22.16b
#endif
    add     v25.4s, v16.4s, v21.4s
    sha1h   s4, s2
    sha1c   q2, s3, v24.4s
    sha1su0 v20.4s, v21.4s, v22.4s
#ifdef __ARMEL__
    rev32   v23.16b, v23.16b
#endif
    add     v26.4s, v16.4s, v22.4s
    sha1h   s5, s2
    sha1c   q2, s4, v25.4s
    sha1su0 v21.4s, v22.4s, v23.4s
    sha1su1 v20.4s, v23.4s
    add     v27.4s, v16.4s, v23.4s
    sha1h   s6, s2
    sha1c   q2, s5, v26.4s 
    sha1su0 v22.4s, v23.4s, v20.4s
    sha1su1 v21.4s, v20.4s
    add     v24.4s, v16.4s, v20.4s
    sha1h   s3, s2
    sha1c   q2, s6, v27.4s
    sha1su0 v23.4s, v20.4s, v21.4s
    sha1su1 v22.4s, v21.4s
    add     v25.4s, v17.4s, v21.4s
    sha1h   s4, s2
    sha1c   q2, s3, v24.4s
    sha1su0 v20.4s, v21.4s, v22.4s
    sha1su1 v23.4s, v22.4s
    add     v26.4s, v17.4s, v22.4s
    sha1h   s5, s2
    sha1p   q2, s4, v25.4s
    sha1su0 v21.4s, v22.4s, v23.4s
    sha1su1 v20.4s, v23.4s
    add     v27.4s, v17.4s, v23.4s
    sha1h   s6, s2
    sha1p   q2, s5, v26.4s
    sha1su0 v22.4s, v23.4s, v20.4s
    sha1su1 v21.4s, v20.4s
    add     v24.4s, v17.4s, v20.4s
    sha1h   s3, s2
    sha1p   q2, s6, v27.4s
    sha1su0 v23.4s, v20.4s, v21.4s
    sha1su1 v22.4s, v21.4s
    add     v25.4s, v17.4s, v21.4s
    sha1h   s4, s2
    sha1p   q2, s3, v24.4s
    sha1su0 v20.4s, v21.4s, v22.4s
    sha1su1 v23.4s, v22.4s
    add     v26.4s, v18.4s, v22.4s
    sha1h   s5, s2
    sha1p   q2, s4, v25.4s
    sha1su0 v21.4s, v22.4s, v23.4s
    sha1su1 v20.4s, v23.4s
    add     v27.4s, v18.4s, v23.4s
    sha1h   s6, s2
    sha1m   q2, s5, v26.4s
    sha1su0 v22.4s, v23.4s, v20.4s
    sha1su1 v21.4s, v20.4s
    add     v24.4s, v18.4s, v20.4s
    sha1h   s3, s2
    sha1m   q2, s6, v27.4s
    sha1su0 v23.4s, v20.4s, v21.4s
    sha1su1 v22.4s, v21.4s
    add     v25.4s, v18.4s, v21.4s
    sha1h   s4, s2
    sha1m   q2, s3, v24.4s
    sha1su0 v20.4s, v21.4s, v22.4s
    sha1su1 v23.4s, v22.4s
    add     v26.4s, v18.4s, v22.4s
    sha1h   s5, s2
    sha1m   q2, s4, v25.4s
    sha1su0 v21.4s, v22.4s, v23.4s
    sha1su1 v20.4s, v23.4s
    add     v27.4s, v19.4s, v23.4s
    sha1h   s6, s2
    sha1m   q2, s5, v26.4s
    sha1su0 v22.4s, v23.4s, v20.4s
    sha1su1 v21.4s, v20.4s
    add     v24.4s, v19.4s, v20.4s
    sha1h   s3, s2
    sha1p   q2, s6, v27.4s
    sha1su0 v23.4s, v20.4s, v21.4s
    sha1su1 v22.4s, v21.4s
    add     v25.4s, v19.4s, v21.4s
    sha1h   s4, s2
    sha1p   q2, s3, v24.4s
    sha1su1 v23.4s, v22.4s
    add     v26.4s, v19.4s, v22.4s
    sha1h   s5, s2
    sha1p   q2, s4, v25.4s
    add     v27.4s, v19.4s, v23.4s
    sha1h   s6, s2
    sha1p   q2, s5, v26.4s
    sha1h   s3, s2
    sha1p   q2, s6, v27.4s
    cmp     x1, x2
    add     v1.4s, v1.4s, v3.4s
    add     v0.4s, v0.4s, v2.4s
    b.ne    .Lsha1loop

.LSha1epilog:

    st1     {v0.4s}, [x0], #16
    st1     {v1.s}[0], [x0]
    ret

.align  2
.LKConstant:
.long   0x5a827999,0x5a827999,0x5a827999,0x5a827999
.long   0x6ed9eba1,0x6ed9eba1,0x6ed9eba1,0x6ed9eba1
.long   0x8f1bbcdc,0x8f1bbcdc,0x8f1bbcdc,0x8f1bbcdc
.long   0xca62c1d6,0xca62c1d6,0xca62c1d6,0xca62c1d6

.size	  sha1_block_data_order,.-sha1_block_data_order
.align	2

#endif
