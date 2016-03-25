#include "arm_arch.h"

#if __ARM_ARCH__>=7

.text
.code   32

# AES assembly implementation for ARMv8 AArch32
#   - AES_encrypt
#   - AES_decrypt
#   - private_AES_set_encrypt_key
#   - private_AES_set_decrypt_key
#   - aes_v8_set_encrypt_key
#   - aes_v8_set_decrypt_key
#   - aes_v8_encrypt
#   - aes_v8_decrypt
#   - aes_v8_cbc_encrypt
#   - aes_v8_ctr32_encrypt_blocks

# void AES_encrypt(const unsigned char *in, unsigned char *out,
#      const AES_KEY *key) {
.global AES_encrypt
.type   AES_encrypt,%function
.align  5
AES_encrypt:
    vld1.8      {d24-d25}, [r2]!
    vld1.8      {d0-d1}, [r0]
#ifdef __ARMEL__
    vrev32.8    q12, q12
#endif
    ldr         r3, [r2, #240-16]
    vld1.8      {d26-d27}, [r2]!
    aese.8      q0, q12
    aesmc.8     q0, q0
    sub         r3, r3, #4
    vld1.8      {d28-d29}, [r2]!
.LPrivateEncLoop:
#ifdef __ARMEL__
    vrev32.8    q13, q13
#endif
    subs        r3, r3, #2
#ifdef __ARMEL__
    vrev32.8    q14, q14
#endif
    aese.8      q0, q13
    aesmc.8     q0, q0
    vld1.8      {d26-d27}, [r2]!
    aese.8      q0, q14
    aesmc.8     q0, q0
    vld1.8      {d28-d29}, [r2]!
    bpl         .LPrivateEncLoop
#ifdef __ARMEL__
    vrev32.8    q13, q13
    vrev32.8    q14, q14
#endif
    aese.8      q0, q13
    veor.8      q0, q0, q14
    vst1.8      {d0-d1}, [r1]
    bx          lr
.size AES_encrypt,.-AES_encrypt


# void AES_decrypt(const unsigned char *in, unsigned char *out,
#      const AES_KEY *key) {
.global AES_decrypt
.type   AES_decrypt,%function
.align  5
AES_decrypt:
    vld1.8      {d24-d25}, [r2]!
    vld1.8      {d0-d1}, [r0]
#ifdef __ARMEL__
    vrev32.8    q12, q12
#endif
    ldr         r3, [r2, #240-16]
    vld1.8      {d26-d27}, [r2]!
    aesd.8      q0, q12
    aesimc.8    q0, q0
    sub         r3, r3, #4
    vld1.8      {d28-d29}, [r2]!
.LPrivateDecLoop:
#ifdef __ARMEL__
    vrev32.8    q13, q13
#endif
    subs        r3, r3, #2
#ifdef __ARMEL__
    vrev32.8    q14, q14
#endif
    aesd.8      q0, q13
    aesimc.8    q0, q0
    vld1.8      {d26-d27}, [r2]!
    aesd.8      q0, q14
    aesimc.8    q0, q0
    vld1.8      {d28-d29}, [r2]!
    bpl         .LPrivateDecLoop
#ifdef __ARMEL__
    vrev32.8    q13, q13
    vrev32.8    q14, q14
#endif
    aesd.8      q0, q13
    veor.8      q0, q0, q14
    vst1.8      {d0-d1}, [r1]
    bx lr
.size AES_decrypt,.-AES_decrypt


.align 5
private_rcon:
.long   0x00000001,0x00000001,0x00000001,0x00000001
.long   0x0c0f0e0d,0x0c0f0e0d,0x0c0f0e0d,0x0c0f0e0d
.long   0x0000001b,0x0000001b,0x0000001b,0x0000001b


# int private_AES_set_encrypt_key(const unsigned char *userKey, const int bits,
#      AES_KEY *key) {
.global private_AES_set_encrypt_key
.type   private_AES_set_encrypt_key,%function
.align  5
private_AES_set_encrypt_key:
    adr       r3,  private_rcon
    cmp       r1,  #192
    veor      q0,  q0,  q0
    vld1.8    {q8},[r0]!
    mov       r1,  #8
    vld1.32   {q1,q2},[r3]!
    beq       .LPrivateExpandEnc192
    bgt       .LPrivateExpandEnc256
.LPrivateExpandEnc128:
.LPrivateLoopEnc128:
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q8},[r2]!
#endif
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    bne       .LPrivateLoopEnc128
    vld1.32   {q1},[r3]
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q8},[r2]!
#endif
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q8},[r2]!
#endif
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]
#else
    vst1.32   {q8},[r2]
#endif
    add       r2,  r2,  #0x50
    mov       r12, #10
    str       r12, [r2]
    eor       r0,  r0,  r0
    bx        lr

.align  4
.LPrivateExpandEnc192:
    vld1.8    {d18},[r0]
    vmov.i8   q3,  #8
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q8},[r2]!
#endif
    vsub.i8   q2,  q2,  q3
#ifdef __ARMEL__
    vrev32.8  q13, q9
    vst1.32   {d26},[r2]!
#else
    vst1.32   {d18},[r2]!
#endif
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    mov       r1,  #3
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vdup.32   q15, d31[1]
    veor      q15, q15, q9
    vext.8    q14, q0,  q9,  #12
    veor      q15, q15, q14
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q8},[r2]!
#endif
    veor      q9,  q3,  q15
#ifdef __ARMEL__
    vrev32.8  q13, q9
    vst1.32   {d26},[r2]!
#else
    vst1.32   {d18},[r2]!
#endif
.LPrivateLoopEnc192:
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q10, q3,  q15
    vdup.32   q15, d31[1]
    veor      q15, q15, q9
    vext.8    q14, q0,  q9,  #12
    veor      q15, q15, q14
#ifdef __ARMEL__
    vrev32.8  q12, q10
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q10},[r2]!
#endif
    veor      q11, q3,  q15
#ifdef __ARMEL__
    vrev32.8  q13, q11
    vst1.32   {d26},[r2]!
#else
    vst1.32   {d22},[r2]!
#endif
    vext.8    q14, q0,  q10, #12
    veor      q15, q10, q14
    vtbl.8    d6,  {q11},d4
    vtbl.8    d7,  {q11},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vdup.32   q15, d31[1]
    veor      q15, q15, q11
    vext.8    q14, q0,  q11, #12
    veor      q15, q15, q14
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q8},[r2]!
#endif
    veor      q9,  q3,  q15
#ifdef __ARMEL__
    vrev32.8  q13, q9
    vst1.32   {d26},[r2]!
#else
    vst1.32   {d18},[r2]!
#endif
    bne       .LPrivateLoopEnc192
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q8},[r2]!
#endif
    mov       r12, #12
    add       r2,  r2,  #0x20
    str       r12, [r2]
    eor       r0,  r0,  r0
    bx        lr

.align  4
.LPrivateExpandEnc256:
    vld1.8    {q9},[r0]
    mov       r1,  #6
.LPrivateLoopEnc256:
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q8},[r2]!
#endif
    vtbl.8    d6,  {q9},d4
#ifdef __ARMEL__
    vrev32.8  q13, q9
    vst1.32   {q13},[r2]!
#else
    vst1.32   {q9},[r2]!
#endif
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vext.8    q14, q0,  q9,  #12
    veor      q15, q9,  q14
    vdup.32   q3,  d17[1]
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q9,  q3,  q15
    bne       .LPrivateLoopEnc256
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q8},[r2]!
#endif
    vtbl.8    d6,  {q9},d4
#ifdef __ARMEL__
    vrev32.8  q13, q9
    vst1.32   {q13},[r2]!
#else
    vst1.32   {q9},[r2]!
#endif
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]!
#else
    vst1.32   {q8},[r2]!
#endif
    mov       r12, #14
    str       r12, [r2]
    eor       r0,  r0,  r0
    bx        lr
.size private_AES_set_encrypt_key,.-private_AES_set_encrypt_key


# int private_AES_set_decrypt_key(const unsigned char *userKey, const int bits,
#       AES_KEY *key) {
.global private_AES_set_decrypt_key
.type   private_AES_set_decrypt_key,%function
.align  5
private_AES_set_decrypt_key:
    adr       r3,  private_rcon
    cmp       r1,  #192
    veor      q0,  q0,  q0
    vld1.8    {q8},[r0]!
    add       r2,  r2,  #160
    vld1.32   {q1,q2},[r3]!
    beq       .LPrivateExpandDec192
    bgt       .LPrivateExpandDec256
.LPrivateExpandDec128:
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]
#else
    vst1.32   {q8},[r2]
#endif
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    sub       r2,  r2,  #16
    aese.8    q3,  q0
    mov       r1,  #7
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    aesimc.8  q12, q8
.LPrivateLoopDec128:
#ifdef __ARMEL__
    vrev32.8  q12, q12
    vst1.32   {q12},[r2]
#else
    vst1.32   {q12},[r2]
#endif
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    sub       r2,  r2,  #16
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    aesimc.8  q12, q8
    bne       .LPrivateLoopDec128
    vld1.32   {q1},[r3]
#ifdef __ARMEL__
    vrev32.8  q12, q12
    vst1.32   {q12},[r2]
#else
    vst1.32   {q12},[r2]
#endif
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    sub       r2,  r2,  #16
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    aesimc.8  q12, q8
#ifdef __ARMEL__
    vrev32.8  q12, q12
    vst1.32   {q12},[r2]
#else
    vst1.32   {q12},[r2]
#endif
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    sub       r2,  r2,  #16
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]
#else
    vst1.32   {q8},[r2]
#endif
    add       r2,  r2,  #240
    mov       r12, #10
    str       r12, [r2]
    eor       r0,  r0,  r0
    bx        lr

.align  4
.LPrivateExpandDec192:
    vld1.8    {d18},[r0]
    add       r2,  r2,  #32
    vmov.i8   q3,  #8
    mov       r1,  #3
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]
#else
    vst1.32   {q8},[r2]
#endif
    sub       r2,  r2,  #16
    vsub.i8   q2,  q2,  q3
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    vmov      d26, d18
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vdup.32   q15, d31[1]
    vmov      d27, d16
    veor      q15, q15, q9
    aesimc.8  q13, q13
    vmov      d24, d17
    vext.8    q14, q0,  q9,  #12
    veor      q15, q15, q14
#ifdef __ARMEL__
    vrev32.8  q13, q13
    vst1.32   {q13},[r2]
#else
    vst1.32   {q13},[r2]
#endif
    sub       r2,  r2,  #16
    veor      q9,  q3,  q15
    vmov      d25, d18
.LPrivateLoopDec192:
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    aesimc.8  q12, q12
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
#ifdef __ARMEL__
    vrev32.8  q12, q12
    vst1.32   {q12},[r2]
#else
    vst1.32   {q12},[r2]
#endif
    sub       r2,  r2,  #16
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q10, q3,  q15
    vdup.32   q15, d31[1]
    veor      q15, q15, q9
    aesimc.8  q12, q10
    vext.8    q14, q0,  q9,  #12
    veor      q15, q15, q14
#ifdef __ARMEL__
    vrev32.8  q12, q12
    vst1.32   {q12},[r2]
#else
    vst1.32   {q12},[r2]
#endif
    sub       r2,  r2,  #16
    veor      q11, q3,  q15
    vext.8    q14, q0,  q10, #12
    veor      q15, q10, q14
    vtbl.8    d6,  {q11},d4
    vtbl.8    d7,  {q11},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    vmov      d26, d22
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vdup.32   q15, d31[1]
    vmov      d27, d16
    veor      q15, q15, q11
    aesimc.8  q13, q13
    vmov      d24, d17
    vext.8    q14, q0,  q11, #12
    veor      q15, q15, q14
#ifdef __ARMEL__
    vrev32.8  q13, q13
    vst1.32   {q13},[r2]
#else
    vst1.32   {q13},[r2]
#endif
    sub       r2,  r2,  #16
    veor      q9,  q3,  q15
    vmov      d25, d18
    bne       .LPrivateLoopDec192
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    aesimc.8  q12, q12
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
#ifdef __ARMEL__
    vrev32.8  q12, q12
    vst1.32   {q12},[r2]
#else
    vst1.32   {q12},[r2]
#endif
    sub       r2,  r2,  #16
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]
#else
    vst1.32   {q8},[r2]
#endif
    add       r2,  r2,  #240
    mov       r12, #12
    str       r12, [r2]
    eor       r0,  r0,  r0
    bx        lr

.align  4
.LPrivateExpandDec256:
    vld1.8    {q9},[r0]
    add       r2,  r2,  #80
    vmov      q12, q8
    mov       r1,  #14
    str       r1,  [r2]
    sub       r2,  r2,  #16
    mov       r1,  #6
.LPrivateLoopDec256:
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    aesimc.8  q13, q9
#ifdef __ARMEL__
    vrev32.8  q12, q12
    vst1.32   {q12},[r2]
#else
    vst1.32   {q12},[r2]
#endif
    sub       r2,  r2,  #16
    vtbl.8    d6,  {q9},d4
#ifdef __ARMEL__
    vrev32.8  q13, q13
    vst1.32   {q13},[r2]
#else
    vst1.32   {q13},[r2]
#endif
    sub       r2,  r2,  #16
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vext.8    q14, q0,  q9,  #12
    veor      q15, q9,  q14
    aesimc.8  q12, q8
    vdup.32   q3,  d17[1]
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q9,  q3,  q15
    bne       .LPrivateLoopDec256
    aesimc.8  q13, q9
#ifdef __ARMEL__
    vrev32.8  q12, q12
    vst1.32   {q12},[r2]
#else
    vst1.32   {q12},[r2]
#endif
    sub       r2,  r2,  #16
    vtbl.8    d6,  {q9},d4
#ifdef __ARMEL__
    vrev32.8  q13, q13
    vst1.32   {q13},[r2]
#else
    vst1.32   {q13},[r2]
#endif
    sub       r2,  r2,  #16
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
#ifdef __ARMEL__
    vrev32.8  q12, q8
    vst1.32   {q12},[r2]
#else
    vst1.32   {q8},[r2]
#endif
    eor       r0,  r0,  r0
    bx        lr
.size private_AES_set_decrypt_key,.-private_AES_set_decrypt_key


.align 5
rcon:
.long   0x00000001,0x00000001,0x00000001,0x00000001
.long   0x0c0f0e0d,0x0c0f0e0d,0x0c0f0e0d,0x0c0f0e0d
.long   0x0000001b,0x0000001b,0x0000001b,0x0000001b


# int aes_v8_set_encrypt_key(const unsigned char *userKey, const int bits,
#     AES_KEY *key);
.global aes_v8_set_encrypt_key
.type   aes_v8_set_encrypt_key,%function
.align  5
aes_v8_set_encrypt_key:
    adr       r3,  rcon
    cmp       r1,  #192
    veor      q0,  q0,  q0
    vld1.8    {q8},[r0]!
    mov       r1,  #8
    vld1.32   {q1,q2},[r3]!
    beq       .LExpandEnc192
    bgt       .LExpandEnc256
.LExpandEnc128:
.LLoopEnc128:
    vst1.32   {q8},[r2]!
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    bne       .LLoopEnc128
    vld1.32   {q1},[r3]
    vst1.32   {q8},[r2]!
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vst1.32   {q8},[r2]!
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
    vst1.32   {q8},[r2]
    add       r2,  r2,  #0x50
    mov       r12, #10
    str       r12, [r2]
    eor       r0,  r0,  r0
    bx        lr

.align  4
.LExpandEnc192:
    vld1.8    {d18},[r0]
    vmov.i8   q3,  #8
    vst1.32   {q8},[r2]!
    vsub.i8   q2,  q2,  q3
    vst1.32   {d18},[r2]!
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    mov       r1,  #3
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vdup.32   q15, d31[1]
    veor      q15, q15, q9
    vext.8    q14, q0,  q9,  #12
    veor      q15, q15, q14
    vst1.32   {q8},[r2]!
    veor      q9,  q3,  q15
    vst1.32   {d18},[r2]!
.LLoopEnc192:
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q10, q3,  q15
    vdup.32   q15, d31[1]
    veor      q15, q15, q9
    vext.8    q14, q0,  q9,  #12
    veor      q15, q15, q14
    vst1.32   {q10},[r2]!
    veor      q11, q3,  q15
    vst1.32   {d22},[r2]!
    vext.8    q14, q0,  q10, #12
    veor      q15, q10, q14
    vtbl.8    d6,  {q11},d4
    vtbl.8    d7,  {q11},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vdup.32   q15, d31[1]
    veor      q15, q15, q11
    vext.8    q14, q0,  q11, #12
    veor      q15, q15, q14
    vst1.32   {q8},[r2]!
    veor      q9,  q3,  q15
    vst1.32   {d18},[r2]!
    bne       .LLoopEnc192
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
    vst1.32   {q8},[r2]!
    mov       r12, #12
    add       r2,  r2,  #0x20
    str       r12, [r2]
    eor       r0,  r0,  r0
    bx        lr

.align  4
.LExpandEnc256:
    vld1.8    {q9},[r0]
    mov       r1,  #6
.LLoopEnc256:
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vst1.32   {q8},[r2]!
    vtbl.8    d6,  {q9},d4
    vst1.32   {q9},[r2]!
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vext.8    q14, q0,  q9,  #12
    veor      q15, q9,  q14
    vdup.32   q3,  d17[1]
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q9,  q3,  q15
    bne       .LLoopEnc256
    vst1.32   {q8},[r2]!
    vtbl.8    d6,  {q9},d4
    vst1.32   {q9},[r2]!
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
    vst1.32   {q8},[r2]!
    mov       r12, #14
    str       r12, [r2]
    eor       r0,  r0,  r0
    bx        lr
.size   aes_v8_set_encrypt_key,.-aes_v8_set_encrypt_key


# int aes_v8_set_decrypt_key(const unsigned char *userKey, const int bits,
#     AES_KEY *key);
.global aes_v8_set_decrypt_key
.type   aes_v8_set_decrypt_key,%function
.align  5
aes_v8_set_decrypt_key:
    adr       r3,  rcon
    cmp       r1,  #192
    veor      q0,  q0,  q0
    vld1.8    {q8},[r0]!
    add       r2,  r2,  #160
    vld1.32   {q1,q2},[r3]!
    beq       .LExpandDec192
    bgt       .LExpandDec256
.LExpandDec128:
    vst1.32   {q8},[r2]
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    sub       r2,  r2,  #16
    aese.8    q3,  q0
    mov       r1,  #7
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    aesimc.8  q12, q8
.LLoopDec128:
    vst1.32   {q12},[r2]
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    sub       r2,  r2,  #16
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    aesimc.8  q12, q8
    bne       .LLoopDec128
    vld1.32   {q1},[r3]
    vst1.32   {q12},[r2]
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    sub       r2,  r2,  #16
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    aesimc.8  q12, q8
    vst1.32   {q12},[r2]
    vtbl.8    d6,  {q8},d4
    vtbl.8    d7,  {q8},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    sub       r2,  r2,  #16
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
    vst1.32   {q8},[r2]
    add       r2,  r2,  #240
    mov       r12, #10
    str       r12, [r2]
    eor       r0,  r0,  r0
    bx        lr

.align  4
.LExpandDec192:
    vld1.8    {d18},[r0]
    add       r2,  r2,  #32
    vmov.i8   q3,  #8
    mov       r1,  #3
    vst1.32   {q8},[r2]
    sub       r2,  r2,  #16
    vsub.i8   q2,  q2,  q3
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    vmov      d26, d18
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vdup.32   q15, d31[1]
    vmov      d27, d16
    veor      q15, q15, q9
    aesimc.8  q13, q13
    vmov      d24, d17
    vext.8    q14, q0,  q9,  #12
    veor      q15, q15, q14
    vst1.32   {q13},[r2]
    sub       r2,  r2,  #16
    veor      q9,  q3,  q15
    vmov      d25, d18
.LLoopDec192:
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    aesimc.8  q12, q12
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
    vst1.32   {q12},[r2]
    sub       r2,  r2,  #16
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q10, q3,  q15
    vdup.32   q15, d31[1]
    veor      q15, q15, q9
    aesimc.8  q12, q10
    vext.8    q14, q0,  q9,  #12
    veor      q15, q15, q14
    vst1.32   {q12},[r2]
    sub       r2,  r2,  #16
    veor      q11, q3,  q15
    vext.8    q14, q0,  q10, #12
    veor      q15, q10, q14
    vtbl.8    d6,  {q11},d4
    vtbl.8    d7,  {q11},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    vmov      d26, d22
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vdup.32   q15, d31[1]
    vmov      d27, d16
    veor      q15, q15, q11
    aesimc.8  q13, q13
    vmov      d24, d17
    vext.8    q14, q0,  q11, #12
    veor      q15, q15, q14
    vst1.32   {q13},[r2]
    sub       r2,  r2,  #16
    veor      q9,  q3,  q15
    vmov      d25, d18
    bne       .LLoopDec192
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    aesimc.8  q12, q12
    vtbl.8    d6,  {q9},d4
    vtbl.8    d7,  {q9},d5
    vst1.32   {q12},[r2]
    sub       r2,  r2,  #16
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
    vst1.32   {q8},[r2]
    add       r2,  r2,  #240
    mov       r12, #12
    str       r12, [r2]
    eor       r0,  r0,  r0
    bx        lr

.align  4
.LExpandDec256:
    vld1.8    {q9},[r0]
    add       r2,  r2,  #80
    vmov      q12, q8
    mov       r1,  #14
    str       r1,  [r2]
    sub       r2,  r2,  #16
    mov       r1,  #6
.LLoopDec256:
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    aesimc.8  q13, q9
    vst1.32   {q12},[r2]
    sub       r2,  r2,  #16
    vtbl.8    d6,  {q9},d4
    vst1.32   {q13},[r2]
    sub       r2,  r2,  #16
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    subs      r1,  r1,  #1
    veor      q3,  q3,  q1
    vshl.u8   q1,  q1,  #1
    veor      q8,  q3,  q15
    vext.8    q14, q0,  q9,  #12
    veor      q15, q9,  q14
    aesimc.8  q12, q8
    vdup.32   q3,  d17[1]
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q9,  q3,  q15
    bne       .LLoopDec256
    aesimc.8  q13, q9
    vst1.32   {q12},[r2]
    sub       r2,  r2,  #16
    vtbl.8    d6,  {q9},d4
    vst1.32   {q13},[r2]
    sub       r2,  r2,  #16
    vtbl.8    d7,  {q9},d5
    vext.8    q14, q0,  q8,  #12
    veor      q15, q8,  q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    vext.8    q14, q0,  q14, #12
    veor      q15, q15, q14
    aese.8    q3,  q0
    veor      q3,  q3,  q1
    veor      q8,  q3,  q15
    vst1.32   {q8},[r2]
    eor       r0,  r0,  r0
    bx        lr
.size   aes_v8_set_decrypt_key,.-aes_v8_set_decrypt_key


# void aes_v8_encrypt(const unsigned char *in, unsigned char *out,
#      const AES_KEY *key);
.global aes_v8_encrypt
.type   aes_v8_encrypt,%function
.align  5
aes_v8_encrypt:
    vld1.8      {d24-d25}, [r2]!
    vld1.8      {d0-d1}, [r0]
    ldr         r3, [r2, #240-16]
    vld1.8      {d26-d27}, [r2]!
    aese.8      q0, q12
    aesmc.8     q0, q0
    sub         r3, r3, #4
    vld1.8      {d28-d29}, [r2]!
.LEncLoop:
    subs        r3, r3, #2
    aese.8      q0, q13
    aesmc.8     q0, q0
    vld1.8      {d26-d27}, [r2]!
    aese.8      q0, q14
    aesmc.8     q0, q0
    vld1.8      {d28-d29}, [r2]!
    bpl         .LEncLoop
    aese.8      q0, q13
    veor.8      q0, q0, q14
    vst1.8      {d0-d1}, [r1]
    bx          lr
.size   aes_v8_encrypt,.-aes_v8_encrypt


# void aes_v8_decrypt(const unsigned char *in, unsigned char *out,
#      const AES_KEY *key);
.global aes_v8_decrypt
.type   aes_v8_decrypt,%function
.align  5
aes_v8_decrypt:
    vld1.8      {d24-d25}, [r2]!
    vld1.8      {d0-d1}, [r0]
    ldr         r3, [r2, #240-16]
    vld1.8      {d26-d27}, [r2]!
    aesd.8      q0, q12
    aesimc.8    q0, q0
    sub         r3, r3, #4
    vld1.8      {d28-d29}, [r2]!
.LDecLoop:
    subs        r3, r3, #2
    aesd.8      q0, q13
    aesimc.8    q0, q0
    vld1.8      {d26-d27}, [r2]!
    aesd.8      q0, q14
    aesimc.8    q0, q0
    vld1.8      {d28-d29}, [r2]!
    bpl         .LDecLoop
    aesd.8      q0, q13
    veor.8      q0, q0, q14
    vst1.8      {d0-d1}, [r1]
    bx lr
.size   aes_v8_decrypt,.-aes_v8_decrypt


# void aes_v8_cbc_encrypt(const unsigned char *in, unsigned char *out,
#      size_t length, const AES_KEY *key,
#      unsigned char *ivec, const int enc);
.global  aes_v8_cbc_encrypt
.type   aes_v8_cbc_encrypt,%function
.align  5
aes_v8_cbc_encrypt:
  mov ip,sp
  stmdb sp!,{r4-r8,lr}
  vstmdb  sp!,{d8-d15}            @ ABI specification says so
  ldmia ip,{r4-r5}    @ load remaining args
  subs  r2,r2,#16
  mov r8,#16
  blo .Lcbc_abort
  moveq r8,#0

  cmp r5,#0     @ en- or decrypting?
  ldr r5,[r3,#240]
  and r2,r2,#-16
  vld1.8  {q6},[r4]
  vld1.8  {q0},[r0],r8

  vld1.32 {q8-q9},[r3]    @ load key schedule...
  sub r5,r5,#6
  add r7,r3,r5,lsl#4  @ pointer to last 7 round keys
  sub r5,r5,#2
  vld1.32 {q10-q11},[r7]!
  vld1.32 {q12-q13},[r7]!
  vld1.32 {q14-q15},[r7]!
  vld1.32 {q7},[r7]

  add r7,r3,#32
  mov r6,r5
  beq .Lcbc_dec

  cmp r5,#2
  veor  q0,q0,q6
  veor  q5,q8,q7
  beq .Lcbc_enc128

.Loop_cbc_enc:
  .byte 0x20,0x03,0xb0,0xf3 @ aese q0,q8
  vld1.32 {q8},[r7]!
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  subs  r6,r6,#2
  .byte 0x22,0x03,0xb0,0xf3 @ aese q0,q9
  vld1.32 {q9},[r7]!
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  bgt .Loop_cbc_enc

  .byte 0x20,0x03,0xb0,0xf3 @ aese q0,q8
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   subs r2,r2,#16
  .byte 0x22,0x03,0xb0,0xf3 @ aese q0,q9
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   moveq  r8,#0
  .byte 0x24,0x03,0xb0,0xf3 @ aese q0,q10
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   add  r7,r3,#16
  .byte 0x26,0x03,0xb0,0xf3 @ aese q0,q11
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   vld1.8 {q8},[r0],r8
  .byte 0x28,0x03,0xb0,0xf3 @ aese q0,q12
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   veor q8,q8,q5
  .byte 0x2a,0x03,0xb0,0xf3 @ aese q0,q13
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   vld1.32 {q9},[r7]! @ re-pre-load rndkey[1]
  .byte 0x2c,0x03,0xb0,0xf3 @ aese q0,q14
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x2e,0x03,0xb0,0xf3 @ aese q0,q15

   mov  r6,r5
  veor  q6,q0,q7
  vst1.8  {q6},[r1]!
  bhs .Loop_cbc_enc

  b .Lcbc_done

.align  5
.Lcbc_enc128:
  vld1.32 {q2-q3},[r7]
  .byte 0x20,0x03,0xb0,0xf3 @ aese q0,q8
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  b .Lenter_cbc_enc128
.Loop_cbc_enc128:
  .byte 0x20,0x03,0xb0,0xf3 @ aese q0,q8
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   vst1.8 {q6},[r1]!
.Lenter_cbc_enc128:
  .byte 0x22,0x03,0xb0,0xf3 @ aese q0,q9
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   subs r2,r2,#16
  .byte 0x04,0x03,0xb0,0xf3 @ aese q0,q2
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   moveq  r8,#0
  .byte 0x06,0x03,0xb0,0xf3 @ aese q0,q3
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x24,0x03,0xb0,0xf3 @ aese q0,q10
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x26,0x03,0xb0,0xf3 @ aese q0,q11
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   vld1.8 {q8},[r0],r8
  .byte 0x28,0x03,0xb0,0xf3 @ aese q0,q12
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x2a,0x03,0xb0,0xf3 @ aese q0,q13
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x2c,0x03,0xb0,0xf3 @ aese q0,q14
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   veor q8,q8,q5
  .byte 0x2e,0x03,0xb0,0xf3 @ aese q0,q15
  veor  q6,q0,q7
  bhs .Loop_cbc_enc128

  vst1.8  {q6},[r1]!
  b .Lcbc_done

.align  5
.Lcbc_dec128:
  vld1.32 {q4-q5},[r7]
  veor  q6,q6,q7
  veor  q2,q0,q7
  mov r12,r8

.Loop2x_cbc_dec128:
  .byte 0x60,0x03,0xb0,0xf3 @ aesd q0,q8
  .byte 0x60,0x23,0xb0,0xf3 @ aesd q1,q8
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
   subs r2,r2,#32
  .byte 0x62,0x03,0xb0,0xf3 @ aesd q0,q9
  .byte 0x62,0x23,0xb0,0xf3 @ aesd q1,q9
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
   movlo  r8,#0
  .byte 0x48,0x03,0xb0,0xf3 @ aesd q0,q4
  .byte 0x48,0x23,0xb0,0xf3 @ aesd q1,q4
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
   movls  r12,#0
  .byte 0x4a,0x03,0xb0,0xf3 @ aesd q0,q5
  .byte 0x4a,0x23,0xb0,0xf3 @ aesd q1,q5
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
  .byte 0x64,0x03,0xb0,0xf3 @ aesd q0,q10
  .byte 0x64,0x23,0xb0,0xf3 @ aesd q1,q10
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
  .byte 0x66,0x03,0xb0,0xf3 @ aesd q0,q11
  .byte 0x66,0x23,0xb0,0xf3 @ aesd q1,q11
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
  .byte 0x68,0x03,0xb0,0xf3 @ aesd q0,q12
  .byte 0x68,0x23,0xb0,0xf3 @ aesd q1,q12
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
  .byte 0x6a,0x03,0xb0,0xf3 @ aesd q0,q13
  .byte 0x6a,0x23,0xb0,0xf3 @ aesd q1,q13
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
  .byte 0x6c,0x03,0xb0,0xf3 @ aesd q0,q14
  .byte 0x6c,0x23,0xb0,0xf3 @ aesd q1,q14
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
  .byte 0x6e,0x03,0xb0,0xf3 @ aesd q0,q15
  .byte 0x6e,0x23,0xb0,0xf3 @ aesd q1,q15

  veor  q6,q6,q0
  vld1.8  {q0},[r0],r8
  veor  q2,q2,q1
  vld1.8  {q1},[r0],r12
  vst1.8  {q6},[r1]!
  veor  q6,q3,q7
  vst1.8  {q2},[r1]!
  veor  q2,q0,q7
  vorr  q3,q1,q1
  bhs .Loop2x_cbc_dec128

  adds  r2,r2,#32
  veor  q6,q6,q7
  beq .Lcbc_done
  veor  q2,q2,q7
  b .Lcbc_dec_tail

.align  5
.Lcbc_dec:
  subs  r2,r2,#16
  vorr  q2,q0,q0
  blo .Lcbc_dec_tail

  moveq r8,#0
  cmp r5,#2
  vld1.8  {q1},[r0],r8
  vorr  q3,q1,q1
  beq .Lcbc_dec128

.Loop2x_cbc_dec:
  .byte 0x60,0x03,0xb0,0xf3 @ aesd q0,q8
  .byte 0x60,0x23,0xb0,0xf3 @ aesd q1,q8
  vld1.32 {q8},[r7]!
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
  subs  r6,r6,#2
  .byte 0x62,0x03,0xb0,0xf3 @ aesd q0,q9
  .byte 0x62,0x23,0xb0,0xf3 @ aesd q1,q9
  vld1.32 {q9},[r7]!
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
  bgt .Loop2x_cbc_dec

  .byte 0x60,0x03,0xb0,0xf3 @ aesd q0,q8
  .byte 0x60,0x23,0xb0,0xf3 @ aesd q1,q8
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
   veor q4,q6,q7
   veor q5,q2,q7
  .byte 0x62,0x03,0xb0,0xf3 @ aesd q0,q9
  .byte 0x62,0x23,0xb0,0xf3 @ aesd q1,q9
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
   vorr q6,q3,q3
   subs r2,r2,#32
  .byte 0x64,0x03,0xb0,0xf3 @ aesd q0,q10
  .byte 0x64,0x23,0xb0,0xf3 @ aesd q1,q10
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
   movlo  r8,#0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
   mov  r7,r3
  .byte 0x66,0x03,0xb0,0xf3 @ aesd q0,q11
  .byte 0x66,0x23,0xb0,0xf3 @ aesd q1,q11
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
   vld1.8 {q2},[r0],r8
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
   movls  r8,#0
  .byte 0x68,0x03,0xb0,0xf3 @ aesd q0,q12
  .byte 0x68,0x23,0xb0,0xf3 @ aesd q1,q12
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
   vld1.8 {q3},[r0],r8
  .byte 0x6a,0x03,0xb0,0xf3 @ aesd q0,q13
  .byte 0x6a,0x23,0xb0,0xf3 @ aesd q1,q13
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
   vld1.32 {q8},[r7]! @ re-pre-load rndkey[0]
  .byte 0x6c,0x03,0xb0,0xf3 @ aesd q0,q14
  .byte 0x6c,0x23,0xb0,0xf3 @ aesd q1,q14
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0xc2,0x23,0xb0,0xf3 @ aesimc q1,q1
   vld1.32 {q9},[r7]! @ re-pre-load rndkey[1]
  .byte 0x6e,0x03,0xb0,0xf3 @ aesd q0,q15
  .byte 0x6e,0x23,0xb0,0xf3 @ aesd q1,q15

   mov  r6,r5
  veor  q4,q4,q0
  veor  q5,q5,q1
   vorr q0,q2,q2
  vst1.8  {q4},[r1]!
   vorr q1,q3,q3
  vst1.8  {q5},[r1]!
  bhs .Loop2x_cbc_dec

  adds  r2,r2,#32
  beq .Lcbc_done

.Lcbc_dec_tail:
  .byte 0x60,0x03,0xb0,0xf3 @ aesd q0,q8
  vld1.32 {q8},[r7]!
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  subs  r6,r6,#2
  .byte 0x62,0x03,0xb0,0xf3 @ aesd q0,q9
  vld1.32 {q9},[r7]!
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  bgt .Lcbc_dec_tail

  .byte 0x60,0x03,0xb0,0xf3 @ aesd q0,q8
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0x62,0x03,0xb0,0xf3 @ aesd q0,q9
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
   veor q4,q6,q7
  .byte 0x64,0x03,0xb0,0xf3 @ aesd q0,q10
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
   vorr q6,q2,q2
  .byte 0x66,0x03,0xb0,0xf3 @ aesd q0,q11
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0x68,0x03,0xb0,0xf3 @ aesd q0,q12
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0x6a,0x03,0xb0,0xf3 @ aesd q0,q13
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0x6c,0x03,0xb0,0xf3 @ aesd q0,q14
  .byte 0xc0,0x03,0xb0,0xf3 @ aesimc q0,q0
  .byte 0x6e,0x03,0xb0,0xf3 @ aesd q0,q15

  veor  q4,q4,q0
  vst1.8  {q4},[r1]!

.Lcbc_done:
  vst1.8  {q6},[r4]
.Lcbc_abort:
  vldmia  sp!,{d8-d15}
  ldmia sp!,{r4-r8,pc}
.size aes_v8_cbc_encrypt,.-aes_v8_cbc_encrypt

# void aes_v8_ctr32_encrypt_blocks(const unsigned char *in, unsigned char *out,
#      size_t len, const AES_KEY *key, const unsigned char ivec[16]);
.global  aes_v8_ctr32_encrypt_blocks
.type aes_v8_ctr32_encrypt_blocks,%function
.align  5
aes_v8_ctr32_encrypt_blocks:
  mov   ip,sp
  stmdb   sp!,{r4-r10,lr}
  vstmdb    sp!,{d8-d15}            @ ABI specification says so
  ldr   r4, [ip]    @ load remaining arg
  ldr   r5,[r3,#240]

  ldr   r8, [r4, #12]
  vld1.32   {q0},[r4]

  vld1.32   {q8-q9},[r3]    @ load key schedule...
  sub   r5,r5,#6
  add   r7,r3,r5,lsl#4  @ pointer to last 7 round keys
  sub   r5,r5,#2
  vld1.32   {q10-q11},[r7]!
  vld1.32   {q12-q13},[r7]!
  vld1.32   {q14-q15},[r7]!
  vld1.32   {q7},[r7]

  add   r7,r3,#32
  mov   r6,r5

  subs    r2,r2,#2
  blo   .Lctr32_tail

#ifndef __ARMEB__
  rev   r8, r8
#endif
  vorr    q1,q0,q0
  add   r8, r8, #1
  vorr    q6,q0,q0
  rev   r10, r8
  cmp   r5,#2
  vmov.32 d3[1],r10
  beq   .Lctr32_128

.Loop2x_ctr32:
  .byte 0x20,0x03,0xb0,0xf3 @ aese q0,q8
  .byte 0x20,0x23,0xb0,0xf3 @ aese q1,q8
  vld1.32   {q8},[r7]!
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
  subs    r6,r6,#2
  .byte 0x22,0x03,0xb0,0xf3 @ aese q0,q9
  .byte 0x22,0x23,0xb0,0xf3 @ aese q1,q9
  vld1.32   {q9},[r7]!
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
  bgt   .Loop2x_ctr32

  .byte 0x20,0x03,0xb0,0xf3 @ aese q0,q8
  .byte 0x20,0x23,0xb0,0xf3 @ aese q1,q8
  .byte 0x80,0x83,0xb0,0xf3 @ aesmc q4,q0
   vorr   q0,q6,q6
  .byte 0x82,0xa3,0xb0,0xf3 @ aesmc q5,q1
   vorr   q1,q6,q6
  .byte 0x22,0x83,0xb0,0xf3 @ aese q4,q9
  .byte 0x22,0xa3,0xb0,0xf3 @ aese q5,q9
   vld1.8   {q2},[r0]!
  .byte 0x88,0x83,0xb0,0xf3 @ aesmc q4,q4
   vld1.8   {q3},[r0]!
  .byte 0x8a,0xa3,0xb0,0xf3 @ aesmc q5,q5
   add    r8,r8,#1
  .byte 0x24,0x83,0xb0,0xf3 @ aese q4,q10
  .byte 0x24,0xa3,0xb0,0xf3 @ aese q5,q10
   rev    r9,r8
  .byte 0x88,0x83,0xb0,0xf3 @ aesmc q4,q4
  .byte 0x8a,0xa3,0xb0,0xf3 @ aesmc q5,q5
   add    r8,r8,#1
  .byte 0x26,0x83,0xb0,0xf3 @ aese q4,q11
  .byte 0x26,0xa3,0xb0,0xf3 @ aese q5,q11
   veor   q2,q2,q7
   rev    r10,r8
  .byte 0x88,0x83,0xb0,0xf3 @ aesmc q4,q4
  .byte 0x8a,0xa3,0xb0,0xf3 @ aesmc q5,q5
   veor   q3,q3,q7
   mov    r7,r3
  .byte 0x28,0x83,0xb0,0xf3 @ aese q4,q12
  .byte 0x28,0xa3,0xb0,0xf3 @ aese q5,q12
   subs   r2,r2,#2
  .byte 0x88,0x83,0xb0,0xf3 @ aesmc q4,q4
  .byte 0x8a,0xa3,0xb0,0xf3 @ aesmc q5,q5
   vld1.32   {q8-q9},[r7]!  @ re-pre-load rndkey[0-1]
  .byte 0x2a,0x83,0xb0,0xf3 @ aese q4,q13
  .byte 0x2a,0xa3,0xb0,0xf3 @ aese q5,q13
  .byte 0x88,0x83,0xb0,0xf3 @ aesmc q4,q4
  .byte 0x8a,0xa3,0xb0,0xf3 @ aesmc q5,q5
  .byte 0x2c,0x83,0xb0,0xf3 @ aese q4,q14
  .byte 0x2c,0xa3,0xb0,0xf3 @ aese q5,q14
   vmov.32  d1[1], r9
  .byte 0x88,0x83,0xb0,0xf3 @ aesmc q4,q4
   vmov.32  d3[1], r10
  .byte 0x8a,0xa3,0xb0,0xf3 @ aesmc q5,q5
  .byte 0x2e,0x83,0xb0,0xf3 @ aese q4,q15
  .byte 0x2e,0xa3,0xb0,0xf3 @ aese q5,q15

   mov    r6,r5
  veor    q2,q2,q4
  veor    q3,q3,q5
  vst1.8    {q2},[r1]!
  vst1.8    {q3},[r1]!
  bhs   .Loop2x_ctr32

  adds    r2,r2,#2
  beq   .Lctr32_done
  b   .Lctr32_tail

.Lctr32_128:
  vld1.32   {q4-q5},[r7]

.Loop2x_ctr32_128:
  .byte 0x20,0x03,0xb0,0xf3 @ aese q0,q8
  .byte 0x20,0x23,0xb0,0xf3 @ aese q1,q8
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   vld1.8   {q2},[r0]!
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
   vld1.8   {q3},[r0]!
  .byte 0x22,0x03,0xb0,0xf3 @ aese q0,q9
  .byte 0x22,0x23,0xb0,0xf3 @ aese q1,q9
   add    r8,r8,#1
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
   rev    r9,r8
  .byte 0x08,0x03,0xb0,0xf3 @ aese q0,q4
  .byte 0x08,0x23,0xb0,0xf3 @ aese q1,q4
   add    r8,r8,#1
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
   rev    r10,r8
  .byte 0x0a,0x03,0xb0,0xf3 @ aese q0,q5
  .byte 0x0a,0x23,0xb0,0xf3 @ aese q1,q5
   subs   r2,r2,#2
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
  .byte 0x24,0x03,0xb0,0xf3 @ aese q0,q10
  .byte 0x24,0x23,0xb0,0xf3 @ aese q1,q10
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
  .byte 0x26,0x03,0xb0,0xf3 @ aese q0,q11
  .byte 0x26,0x23,0xb0,0xf3 @ aese q1,q11
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
  .byte 0x28,0x03,0xb0,0xf3 @ aese q0,q12
  .byte 0x28,0x23,0xb0,0xf3 @ aese q1,q12
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
  .byte 0x2a,0x03,0xb0,0xf3 @ aese q0,q13
  .byte 0x2a,0x23,0xb0,0xf3 @ aese q1,q13
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
  .byte 0x2c,0x03,0xb0,0xf3 @ aese q0,q14
  .byte 0x2c,0x23,0xb0,0xf3 @ aese q1,q14
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x82,0x23,0xb0,0xf3 @ aesmc q1,q1
   veor   q2,q2,q7
  .byte 0x2e,0x03,0xb0,0xf3 @ aese q0,q15
   veor   q3,q3,q7
  .byte 0x2e,0x23,0xb0,0xf3 @ aese q1,q15

  veor    q2,q2,q0
  vorr    q0,q6,q6
  veor    q3,q3,q1
  vorr    q1,q6,q6
  vst1.8    {q2},[r1]!
  vmov.32 d1[1], r9
  vst1.8    {q3},[r1]!
  vmov.32 d3[1], r10
  bhs   .Loop2x_ctr32_128

  adds    r2,r2,#2
  beq   .Lctr32_done

.Lctr32_tail:
  .byte 0x20,0x03,0xb0,0xf3 @ aese q0,q8
  vld1.32   {q8},[r7]!
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  subs    r6,r6,#2
  .byte 0x22,0x03,0xb0,0xf3 @ aese q0,q9
  vld1.32   {q9},[r7]!
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  bgt   .Lctr32_tail

  .byte 0x20,0x03,0xb0,0xf3 @ aese q0,q8
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x22,0x03,0xb0,0xf3 @ aese q0,q9
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   vld1.8   {q2},[r0]
  .byte 0x24,0x03,0xb0,0xf3 @ aese q0,q10
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x26,0x03,0xb0,0xf3 @ aese q0,q11
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x28,0x03,0xb0,0xf3 @ aese q0,q12
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x2a,0x03,0xb0,0xf3 @ aese q0,q13
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
  .byte 0x2c,0x03,0xb0,0xf3 @ aese q0,q14
  .byte 0x80,0x03,0xb0,0xf3 @ aesmc q0,q0
   veor   q2,q2,q7
  .byte 0x2e,0x03,0xb0,0xf3 @ aese q0,q15

  veor    q2,q2,q0
  vst1.8    {q2},[r1]

.Lctr32_done:
  vldmia    sp!,{d8-d15}
  ldmia   sp!,{r4-r10,pc}
.size aes_v8_ctr32_encrypt_blocks,.-aes_v8_ctr32_encrypt_blocks

#endif
