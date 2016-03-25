#include "arm_arch.h"

#if __ARM_ARCH__>=7

.text
.arch   armv8-a+crypto

# AES assembly implementation for ARMv8 AArch64
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
    ld1       {v1.16b}, [x2], #16
    ld1       {v0.16b}, [x0]
#ifdef __ARMEL__
    rev32     v1.16b, v1.16b
#endif
    ldr       w3, [x2, #240-16]
    ld1       {v2.16b}, [x2], #16
    aese      v0.16b, v1.16b
    aesmc     v0.16b, v0.16b
    sub       w3, w3, #4
    ld1       {v3.16b}, [x2], #16
.LPrivateEncLoop:
#ifdef __ARMEL__
    rev32     v2.16b, v2.16b
#endif
    subs      w3, w3, #2
#ifdef __ARMEL__
    rev32     v3.16b, v3.16b
#endif
    aese      v0.16b, v2.16b
    aesmc     v0.16b, v0.16b
    ld1       {v2.16b}, [x2], #16
    aese      v0.16b, v3.16b
    aesmc     v0.16b, v0.16b
    ld1       {v3.16b}, [x2], #16
    b.pl      .LPrivateEncLoop
#ifdef __ARMEL__
    rev32     v2.16b, v2.16b
    rev32     v3.16b, v3.16b
#endif
    aese      v0.16b, v2.16b
    eor       v0.16b, v0.16b, v3.16b
    st1       {v0.16b}, [x1]
    ret
.size AES_encrypt,.-AES_encrypt


# void AES_decrypt(const unsigned char *in, unsigned char *out,
#      const AES_KEY *key) {
.global AES_decrypt
.type   AES_decrypt,%function
.align  5
AES_decrypt:
    ld1       {v1.16b}, [x2], #16
    ld1       {v0.16b}, [x0]
#ifdef __ARMEL__
    rev32     v1.16b, v1.16b
#endif
    ldr       w3, [x2, #240-16]
    ld1       {v2.16b}, [x2], #16
    aesd      v0.16b, v1.16b
    aesimc    v0.16b, v0.16b
    sub       w3, w3, #4
    ld1       {v3.16b}, [x2], #16
.LPrivateDecLoop:
#ifdef __ARMEL__
    rev32     v2.16b, v2.16b
#endif
    subs      w3, w3, #2
#ifdef __ARMEL__
    rev32     v3.16b, v3.16b
#endif
    aesd      v0.16b, v2.16b
    aesimc    v0.16b, v0.16b
    ld1       {v2.16b}, [x2], #16
    aesd      v0.16b, v3.16b
    aesimc    v0.16b, v0.16b
    ld1       {v3.16b}, [x2], #16
    b.pl      .LPrivateDecLoop
#ifdef __ARMEL__
    rev32     v2.16b, v2.16b
    rev32     v3.16b, v3.16b
#endif
    aesd      v0.16b, v2.16b
    eor       v0.16b, v0.16b, v3.16b
    st1       {v0.16b}, [x1]
    ret
.size AES_decrypt,.-AES_decrypt


.align 5
private_rcon:
.long         0x00000001,0x00000001,0x00000001,0x00000001
.long         0x0c0f0e0d,0x0c0f0e0d,0x0c0f0e0d,0x0c0f0e0d
.long         0x0000001b,0x0000001b,0x0000001b,0x0000001b


# int private_AES_set_encrypt_key(const unsigned char *userKey, const int bits,
#      AES_KEY *key) {
.global private_AES_set_encrypt_key
.type   private_AES_set_encrypt_key,%function
.align  5
private_AES_set_encrypt_key:
    adr       x3, private_rcon
    cmp       w1, #192
    eor       v0.16b, v0.16b, v0.16b
    ld1       {v4.16b}, [x0], #16
    mov       w1,  #8
    ld1       {v1.4s, v2.4s}, [x3], #32
    b.eq      .LPrivateExpandEnc192
    b.gt      .LPrivateExpandEnc256
.LPrivateExpandEnc128:
.LPrivateLoopEnc128:
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v4.16b}, [x2], #16
#endif
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b 
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    subs      w1,  w1,  #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    b.ne      .LPrivateLoopEnc128
    ld1       {v1.4s}, [x3]
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v4.16b}, [x2], #16
#endif
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v4.16b}, [x2], #16
#endif
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v4.16b}, [x2]
#endif
    add       x2, x2, #0x50
    mov       w12, #10
    str       w12, [x2]
    eor       x0, x0, x0
    ret

.align  4
.LPrivateExpandEnc192:
    ld1       {v5.8b}, [x0], #8
    movi      v3.16b, #8
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v4.16b}, [x2], #16
#endif
    sub       v2.16b, v2.16b, v3.16b
#ifdef __ARMEL__
    rev32     v17.16b, v5.16b
    st1       {v17.8b}, [x2], #8
#else
    st1       {v5.8b}, [x2], #8
#endif
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    tbl       v3.16b, {v5.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    mov       w1, #3
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    eor       v19.16b, v19.16b, v5.16b
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v19.16b, v18.16b
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v4.16b}, [x2], #16
#endif
    eor       v5.16b, v3.16b, v19.16b
#ifdef __ARMEL__
    rev32     v17.16b, v5.16b
    st1       {v17.8b}, [x2], #8
#else
    st1       {v5.8b}, [x2], #8
#endif
.LPrivateLoopEnc192:
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    tbl       v3.16b, {v5.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    subs      w1, w1, #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v6.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    eor       v19.16b, v19.16b, v5.16b
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v19.16b, v18.16b
#ifdef __ARMEL__
    rev32     v16.16b, v6.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v6.16b}, [x2], #16
#endif
    eor       v7.16b, v3.16b, v19.16b
#ifdef __ARMEL__
    rev32     v17.16b, v7.16b
    st1       {v17.8b}, [x2], #8
#else
    st1       {v7.8b}, [x2], #8
#endif
    ext       v18.16b, v0.16b, v6.16b, #12
    eor       v19.16b, v6.16b, v18.16b
    tbl       v3.16b, {v7.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    eor       v19.16b, v19.16b, v7.16b
    ext       v18.16b, v0.16b, v7.16b, #12
    eor       v19.16b, v19.16b, v18.16b
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v4.16b}, [x2], #16
#endif
    eor       v5.16b, v3.16b, v19.16b
#ifdef __ARMEL__
    rev32     v17.16b, v5.16b
    st1       {v17.8b}, [x2], #8
#else
    st1       {v5.8b}, [x2], #8
#endif
    b.ne      .LPrivateLoopEnc192
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    tbl       v3.16b, {v5.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v4.16b}, [x2], #16
#endif
    mov       w12, #12
    add       x2, x2, #0x20
    str       w12, [x2]
    eor       x0, x0, x0  
    ret

.align 4
.LPrivateExpandEnc256:
    ld1       {v5.16b}, [x0]
    mov       w1, #6
.LPrivateLoopEnc256:
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v4.16b}, [x2], #16
#endif
    tbl       v3.16b, {v5.16b}, v2.16b
#ifdef __ARMEL__
    rev32     v17.16b, v5.16b
    st1       {v17.16b}, [x2], #16
#else
    st1       {v5.16b}, [x2], #16
#endif
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    subs      w1, w1, #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v5.16b, v18.16b
    dup       v3.4s, v4.s[3]
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v5.16b, v3.16b, v19.16b
    b.ne      .LPrivateLoopEnc256
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v4.16b}, [x2], #16
#endif
    tbl       v3.16b, {v5.16b}, v2.16b
#ifdef __ARMEL__
    rev32     v17.16b, v5.16b
    st1       {v17.16b}, [x2], #16
#else
    st1       {v5.16b}, [x2], #16
#endif
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2], #16
#else
    st1       {v4.16b}, [x2], #16
#endif
    mov       w12, #14
    str       w12, [x2]
    eor       x0, x0, x0
    ret
.size private_AES_set_encrypt_key,.-private_AES_set_encrypt_key


# int private_AES_set_decrypt_key(const unsigned char *userKey, const int bits,
#       AES_KEY *key) {
.global private_AES_set_decrypt_key
.type   private_AES_set_decrypt_key,%function
.align  5
private_AES_set_decrypt_key:
    adr       x3, private_rcon
    cmp       w1, #192
    eor       v0.16b, v0.16b, v0.16b
    ld1       {v4.16b}, [x0], #16
    add       x2, x2, #160
    ld1       {v1.4s, v2.4s}, [x3], #32
    b.eq      .LPrivateExpandDec192
    b.gt      .LPrivateExpandDec256
.LPrivateExpandDec128:
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v4.16b}, [x2]
#endif
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    sub       x2, x2, #16
    aese      v3.16b, v0.16b
    mov       w1, #7
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    aesimc    v16.16b, v4.16b
.LPrivateLoopDec128:
#ifdef __ARMEL__
    rev32     v16.16b, v16.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v16.16b}, [x2]
#endif
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    sub       x2, x2, #16
    aese      v3.16b, v0.16b
    subs      w1, w1, #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    aesimc    v16.16b, v4.16b
    b.ne      .LPrivateLoopDec128
    ld1       {v1.4s}, [x3]
#ifdef __ARMEL__
    rev32     v16.16b, v16.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v16.16b}, [x2]
#endif
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    sub       x2, x2, #16
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    aesimc    v16.16b, v4.16b
#ifdef __ARMEL__
    rev32     v16.16b, v16.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v16.16b}, [x2]
#endif
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    sub       x2, x2, #16
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v4.16b}, [x2]
#endif
    add       x2, x2, #240
    mov       w12, #10
    str       w12, [x2]
    eor       x0, x0, x0
    ret

.align  4
.LPrivateExpandDec192:
    ld1       {v5.8b}, [x0], #8
    add       x2, x2, #32
    movi      v3.16b, #8
    mov       w1, #3 
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v4.16b}, [x2]
#endif
    sub       x2, x2, #16
    sub       v2.16b, v2.16b, v3.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    tbl       v3.16b, {v5.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    mov       v17.d[0], v5.d[0]
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    mov       v17.d[1], v4.d[0]
    eor       v19.16b, v19.16b, v5.16b
    aesimc    v17.16b, v17.16b
    mov       v16.d[0], v4.d[1]
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v19.16b, v18.16b
#ifdef __ARMEL__
    rev32     v17.16b, v17.16b
    st1       {v17.16b}, [x2]
#else
    st1       {v17.16b}, [x2]
#endif
    sub       x2, x2, #16
    eor       v5.16b, v3.16b, v19.16b
    mov       v16.d[1], v5.d[0]
.LPrivateLoopDec192:
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    aesimc    v16.16b, v16.16b
    tbl       v3.16b, {v5.16b}, v2.16b
#ifdef __ARMEL__
    rev32     v16.16b, v16.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v16.16b}, [x2]
#endif
    sub       x2, x2, #16
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    subs      w1, w1, #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v6.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    eor       v19.16b, v19.16b, v5.16b
    aesimc    v16.16b, v6.16b
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v19.16b, v18.16b
#ifdef __ARMEL__
    rev32     v16.16b, v16.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v16.16b}, [x2]
#endif
    sub       x2, x2, #16
    eor       v7.16b, v3.16b, v19.16b
    ext       v18.16b, v0.16b, v6.16b, #12
    eor       v19.16b, v6.16b, v18.16b
    tbl       v3.16b, {v7.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    mov       v17.d[0], v7.d[0]
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    mov       v17.d[1], v4.d[0]
    eor       v19.16b, v19.16b, v7.16b
    aesimc    v17.16b, v17.16b
    mov       v16.d[0], v4.d[1]
    ext       v18.16b, v0.16b, v7.16b, #12
    eor       v19.16b, v19.16b, v18.16b
#ifdef __ARMEL__
    rev32     v17.16b, v17.16b
    st1       {v17.16b}, [x2]
#else
    st1       {v17.16b}, [x2]
#endif
    sub       x2, x2, #16
    eor       v5.16b, v3.16b, v19.16b
    mov       v16.d[1], v5.d[0]
    b.ne      .LPrivateLoopDec192
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    aesimc    v16.16b, v16.16b
    tbl       v3.16b, {v5.16b}, v2.16b
#ifdef __ARMEL__
    rev32     v16.16b, v16.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v16.16b}, [x2]
#endif
    sub       x2, x2, #16
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v4.16b}, [x2]
#endif
    add       x2, x2, #240
    mov       w12, #12
    str       w12, [x2]
    eor       x0, x0, x0
    ret

.align 4
.LPrivateExpandDec256:
    ld1       {v5.16b}, [x0]
    add       x2, x2, #80
    mov       v16.16b, v4.16b
    mov       w1, #14
    str       w1, [x2]
    sub       x2, x2, #16
    mov       w1, #6
.LPrivateLoopDec256:
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    aesimc    v17.16b, v5.16b
#ifdef __ARMEL__
    rev32     v16.16b, v16.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v16.16b}, [x2]
#endif
    sub       x2, x2, #16
    tbl       v3.16b, {v5.16b}, v2.16b
#ifdef __ARMEL__
    rev32     v17.16b, v17.16b
    st1       {v17.16b}, [x2]
#else
    st1       {v17.16b}, [x2]
#endif
    sub       x2, x2, #16
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    subs      w1, w1, #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v5.16b, v18.16b
    aesimc    v16.16b, v4.16b
    dup       v3.4s, v4.s[3]
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v5.16b, v3.16b, v19.16b
    b.ne      .LPrivateLoopDec256
    aesimc    v17.16b, v5.16b
#ifdef __ARMEL__
    rev32     v16.16b, v16.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v16.16b}, [x2]
#endif
    sub       x2, x2, #16
    tbl       v3.16b, {v5.16b}, v2.16b
#ifdef __ARMEL__
    rev32     v17.16b, v17.16b
    st1       {v17.16b}, [x2]
#else
    st1       {v17.16b}, [x2]
#endif
    sub       x2, x2, #16
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
#ifdef __ARMEL__
    rev32     v16.16b, v4.16b
    st1       {v16.16b}, [x2]
#else
    st1       {v4.16b}, [x2]
#endif
    eor       x0, x0, x0
    ret
.size private_AES_set_decrypt_key,.-private_AES_set_decrypt_key


.align 5
rcon:
.long         0x00000001,0x00000001,0x00000001,0x00000001
.long         0x0c0f0e0d,0x0c0f0e0d,0x0c0f0e0d,0x0c0f0e0d
.long         0x0000001b,0x0000001b,0x0000001b,0x0000001b


# int aes_v8_set_encrypt_key(const unsigned char *userKey, const int bits,
#     AES_KEY *key);
.global aes_v8_set_encrypt_key
.type   aes_v8_set_encrypt_key,%function
.align  5
aes_v8_set_encrypt_key:
    adr       x3, rcon
    cmp       w1, #192
    eor       v0.16b, v0.16b, v0.16b
    ld1       {v4.16b}, [x0], #16
    mov       w1,  #8
    ld1       {v1.4s, v2.4s}, [x3], #32
    b.eq      .LExpandEnc192
    b.gt      .LExpandEnc256
.LExpandEnc128:
.LLoopEnc128:
    st1       {v4.16b}, [x2], #16
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b 
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    subs      w1,  w1,  #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    b.ne      .LLoopEnc128
    ld1       {v1.4s}, [x3]
    st1       {v4.16b}, [x2], #16
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    st1       {v4.16b}, [x2], #16
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
    st1       {v4.16b}, [x2]
    add       x2, x2, #0x50
    mov       w12, #10
    str       w12, [x2]
    eor       x0, x0, x0
    ret

.align  4
.LExpandEnc192:
    ld1       {v5.8b}, [x0], #8
    movi      v3.16b, #8
    st1       {v4.16b}, [x2], #16
    sub       v2.16b, v2.16b, v3.16b
    st1       {v5.8b}, [x2], #8
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    tbl       v3.16b, {v5.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    mov       w1, #3
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    eor       v19.16b, v19.16b, v5.16b
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    st1       {v4.16b}, [x2], #16
    eor       v5.16b, v3.16b, v19.16b
    st1       {v5.8b}, [x2], #8
.LLoopEnc192:
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    tbl       v3.16b, {v5.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    subs      w1, w1, #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v6.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    eor       v19.16b, v19.16b, v5.16b
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    st1       {v6.16b}, [x2], #16
    eor       v7.16b, v3.16b, v19.16b
    st1       {v7.8b}, [x2], #8
    ext       v18.16b, v0.16b, v6.16b, #12
    eor       v19.16b, v6.16b, v18.16b
    tbl       v3.16b, {v7.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    eor       v19.16b, v19.16b, v7.16b
    ext       v18.16b, v0.16b, v7.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    st1       {v4.16b}, [x2], #16
    eor       v5.16b, v3.16b, v19.16b
    st1       {v5.8b}, [x2], #8
    b.ne      .LLoopEnc192
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    tbl       v3.16b, {v5.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
    st1       {v4.16b}, [x2], #16
    mov       w12, #12
    add       x2, x2, #0x20
    str       w12, [x2]
    eor       x0, x0, x0  
    ret

.align 4
.LExpandEnc256:
    ld1       {v5.16b}, [x0]
    mov       w1, #6
.LLoopEnc256:
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    st1       {v4.16b}, [x2], #16
    tbl       v3.16b, {v5.16b}, v2.16b
    st1       {v5.16b}, [x2], #16
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    subs      w1, w1, #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v5.16b, v18.16b
    dup       v3.4s, v4.s[3]
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v5.16b, v3.16b, v19.16b
    b.ne      .LLoopEnc256
    st1       {v4.16b}, [x2], #16
    tbl       v3.16b, {v5.16b}, v2.16b
    st1       {v5.16b}, [x2], #16
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
    st1       {v4.16b}, [x2], #16
    mov       w12, #14
    str       w12, [x2]
    eor       x0, x0, x0
    ret
.size   aes_v8_set_encrypt_key,.-aes_v8_set_encrypt_key


# int aes_v8_set_decrypt_key(const unsigned char *userKey, const int bits,
#     AES_KEY *key);
.global aes_v8_set_decrypt_key
.type   aes_v8_set_decrypt_key,%function
.align  5
aes_v8_set_decrypt_key:
    adr       x3, rcon
    cmp       w1, #192
    eor       v0.16b, v0.16b, v0.16b
    ld1       {v4.16b}, [x0], #16
    add       x2, x2, #160
    ld1       {v1.4s, v2.4s}, [x3], #32
    b.eq      .LExpandDec192
    b.gt      .LExpandDec256
.LExpandDec128:
    st1       {v4.16b}, [x2]
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    sub       x2, x2, #16
    aese      v3.16b, v0.16b
    mov       w1, #7
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    aesimc    v16.16b, v4.16b
.LLoopDec128:
    st1       {v16.16b}, [x2]
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    sub       x2, x2, #16
    aese      v3.16b, v0.16b
    subs      w1, w1, #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    aesimc    v16.16b, v4.16b
    b.ne      .LLoopDec128
    ld1       {v1.4s}, [x3]
    st1       {v16.16b}, [x2]
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    sub       x2, x2, #16
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    aesimc    v16.16b, v4.16b
    st1       {v16.16b}, [x2]
    tbl       v3.16b, {v4.16b}, v2.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    sub       x2, x2, #16
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
    st1       {v4.16b}, [x2]
    add       x2, x2, #240
    mov       w12, #10
    str       w12, [x2]
    eor       x0, x0, x0
    ret

.align  4
.LExpandDec192:
    ld1       {v5.8b}, [x0], #8
    add       x2, x2, #32
    movi      v3.16b, #8
    mov       w1, #3 
    st1       {v4.16b}, [x2]
    sub       x2, x2, #16
    sub       v2.16b, v2.16b, v3.16b
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    tbl       v3.16b, {v5.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    mov       v17.d[0], v5.d[0]
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    mov       v17.d[1], v4.d[0]
    eor       v19.16b, v19.16b, v5.16b
    aesimc    v17.16b, v17.16b
    mov       v16.d[0], v4.d[1]
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    st1       {v17.16b}, [x2]
    sub       x2, x2, #16
    eor       v5.16b, v3.16b, v19.16b
    mov       v16.d[1], v5.d[0]
.LLoopDec192:
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    aesimc    v16.16b, v16.16b
    tbl       v3.16b, {v5.16b}, v2.16b
    st1       {v16.16b}, [x2]
    sub       x2, x2, #16
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    subs      w1, w1, #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v6.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    eor       v19.16b, v19.16b, v5.16b
    aesimc    v16.16b, v6.16b
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    st1       {v16.16b}, [x2]
    sub       x2, x2, #16
    eor       v7.16b, v3.16b, v19.16b
    ext       v18.16b, v0.16b, v6.16b, #12
    eor       v19.16b, v6.16b, v18.16b
    tbl       v3.16b, {v7.16b}, v2.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    mov       v17.d[0], v7.d[0]
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    dup       v19.4s, v19.s[3]
    mov       v17.d[1], v4.d[0]
    eor       v19.16b, v19.16b, v7.16b
    aesimc    v17.16b, v17.16b
    mov       v16.d[0], v4.d[1]
    ext       v18.16b, v0.16b, v7.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    st1       {v17.16b}, [x2]
    sub       x2, x2, #16
    eor       v5.16b, v3.16b, v19.16b
    mov       v16.d[1], v5.d[0]
    b.ne      .LLoopDec192
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    aesimc    v16.16b, v16.16b
    tbl       v3.16b, {v5.16b}, v2.16b
    st1       {v16.16b}, [x2]
    sub       x2, x2, #16
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
    st1       {v4.16b}, [x2]
    add       x2, x2, #240
    mov       w12, #12
    str       w12, [x2]
    eor       x0, x0, x0
    ret

.align 4
.LExpandDec256:
    ld1       {v5.16b}, [x0]
    add       x2, x2, #80
    mov       v16.16b, v4.16b
    mov       w1, #14
    str       w1, [x2]
    sub       x2, x2, #16
    mov       w1, #6
.LLoopDec256:
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    aesimc    v17.16b, v5.16b
    st1       {v16.16b}, [x2]
    sub       x2, x2, #16
    tbl       v3.16b, {v5.16b}, v2.16b
    st1       {v17.16b}, [x2]
    sub       x2, x2, #16
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    subs      w1, w1, #1
    eor       v3.16b, v3.16b, v1.16b
    shl       v1.16b, v1.16b, #1
    eor       v4.16b, v3.16b, v19.16b
    ext       v18.16b, v0.16b, v5.16b, #12
    eor       v19.16b, v5.16b, v18.16b
    aesimc    v16.16b, v4.16b
    dup       v3.4s, v4.s[3]
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v5.16b, v3.16b, v19.16b
    b.ne      .LLoopDec256
    aesimc    v17.16b, v5.16b
    st1       {v16.16b}, [x2]
    sub       x2, x2, #16
    tbl       v3.16b, {v5.16b}, v2.16b
    st1       {v17.16b}, [x2]
    sub       x2, x2, #16
    ext       v18.16b, v0.16b, v4.16b, #12
    eor       v19.16b, v4.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    ext       v18.16b, v0.16b, v18.16b, #12
    eor       v19.16b, v19.16b, v18.16b
    aese      v3.16b, v0.16b
    eor       v3.16b, v3.16b, v1.16b
    eor       v4.16b, v3.16b, v19.16b
    st1       {v4.16b}, [x2]
    eor       x0, x0, x0
    ret
.size   aes_v8_set_decrypt_key,.-aes_v8_set_decrypt_key


# void aes_v8_encrypt(const unsigned char *in, unsigned char *out,
#      const AES_KEY *key);
.global aes_v8_encrypt
.type   aes_v8_encrypt,%function
.align  5
aes_v8_encrypt:
    ld1       {v1.16b}, [x2], #16
    ld1       {v0.16b}, [x0]
    ldr       w3, [x2, #240-16]
    ld1       {v2.16b}, [x2], #16
    aese      v0.16b, v1.16b
    aesmc     v0.16b, v0.16b
    sub       w3, w3, #4
    ld1       {v3.16b}, [x2], #16
.LEncLoop:
    subs      w3, w3, #2
    aese      v0.16b, v2.16b
    aesmc     v0.16b, v0.16b
    ld1       {v2.16b}, [x2], #16
    aese      v0.16b, v3.16b
    aesmc     v0.16b, v0.16b
    ld1       {v3.16b}, [x2], #16
    b.pl      .LEncLoop
    aese      v0.16b, v2.16b
    eor       v0.16b, v0.16b, v3.16b
    st1       {v0.16b}, [x1]    
    ret
.size   aes_v8_encrypt,.-aes_v8_encrypt


# void aes_v8_decrypt(const unsigned char *in, unsigned char *out,
#      const AES_KEY *key);
.global aes_v8_decrypt
.type   aes_v8_decrypt,%function
.align  5
aes_v8_decrypt:
    ld1       {v1.16b}, [x2], #16
    ld1       {v0.16b}, [x0]
    ldr       w3, [x2, #240-16]
    ld1       {v2.16b}, [x2], #16
    aesd      v0.16b, v1.16b
    aesimc    v0.16b, v0.16b
    sub       w3, w3, #4
    ld1       {v3.16b}, [x2], #16
.LDecLoop:
    subs      w3, w3, #2
    aesd      v0.16b, v2.16b
    aesimc    v0.16b, v0.16b
    ld1       {v2.16b}, [x2], #16
    aesd      v0.16b, v3.16b
    aesimc    v0.16b, v0.16b
    ld1       {v3.16b}, [x2], #16
    b.pl      .LDecLoop
    aesd      v0.16b, v2.16b
    eor       v0.16b, v0.16b, v3.16b
    st1       {v0.16b}, [x1]
    ret
.size   aes_v8_decrypt,.-aes_v8_decrypt


# void aes_v8_cbc_encrypt(const unsigned char *in, unsigned char *out,
#      size_t length, const AES_KEY *key,
#      unsigned char *ivec, const int enc);
.globl  aes_v8_cbc_encrypt
.type aes_v8_cbc_encrypt,%function
.align  5
aes_v8_cbc_encrypt:
  stp x29,x30,[sp,#-16]!
  add x29,sp,#0
  subs  x2,x2,#16
  mov x8,#16
  b.lo  .Lcbc_abort
  csel  x8,xzr,x8,eq

  cmp w5,#0     // en- or decrypting?
  ldr w5,[x3,#240]
  and x2,x2,#-16
  ld1 {v6.16b},[x4]
  ld1 {v0.16b},[x0],x8

  ld1 {v16.4s-v17.4s},[x3]    // load key schedule...
  sub w5,w5,#6
  add x7,x3,x5,lsl#4  // pointer to last 7 round keys
  sub w5,w5,#2
  ld1 {v18.4s-v19.4s},[x7],#32
  ld1 {v20.4s-v21.4s},[x7],#32
  ld1 {v22.4s-v23.4s},[x7],#32
  ld1 {v7.4s},[x7]

  add x7,x3,#32
  mov w6,w5
  b.eq  .Lcbc_dec

  cmp w5,#2
  eor v0.16b,v0.16b,v6.16b
  eor v5.16b,v16.16b,v7.16b
  b.eq  .Lcbc_enc128

.Loop_cbc_enc:
  aese  v0.16b,v16.16b
  ld1 {v16.4s},[x7],#16
  aesmc v0.16b,v0.16b
  subs  w6,w6,#2
  aese  v0.16b,v17.16b
  ld1 {v17.4s},[x7],#16
  aesmc v0.16b,v0.16b
  b.gt  .Loop_cbc_enc

  aese  v0.16b,v16.16b
  aesmc v0.16b,v0.16b
   subs x2,x2,#16
  aese  v0.16b,v17.16b
  aesmc v0.16b,v0.16b
   csel x8,xzr,x8,eq
  aese  v0.16b,v18.16b
  aesmc v0.16b,v0.16b
   add  x7,x3,#16
  aese  v0.16b,v19.16b
  aesmc v0.16b,v0.16b
   ld1  {v16.16b},[x0],x8
  aese  v0.16b,v20.16b
  aesmc v0.16b,v0.16b
   eor  v16.16b,v16.16b,v5.16b
  aese  v0.16b,v21.16b
  aesmc v0.16b,v0.16b
   ld1 {v17.4s},[x7],#16  // re-pre-load rndkey[1]
  aese  v0.16b,v22.16b
  aesmc v0.16b,v0.16b
  aese  v0.16b,v23.16b

   mov  w6,w5
  eor v6.16b,v0.16b,v7.16b
  st1 {v6.16b},[x1],#16
  b.hs  .Loop_cbc_enc

  b .Lcbc_done

.align  5
.Lcbc_enc128:
  ld1 {v2.4s-v3.4s},[x7]
  aese  v0.16b,v16.16b
  aesmc v0.16b,v0.16b
  b .Lenter_cbc_enc128
.Loop_cbc_enc128:
  aese  v0.16b,v16.16b
  aesmc v0.16b,v0.16b
   st1  {v6.16b},[x1],#16
.Lenter_cbc_enc128:
  aese  v0.16b,v17.16b
  aesmc v0.16b,v0.16b
   subs x2,x2,#16
  aese  v0.16b,v2.16b
  aesmc v0.16b,v0.16b
   csel x8,xzr,x8,eq
  aese  v0.16b,v3.16b
  aesmc v0.16b,v0.16b
  aese  v0.16b,v18.16b
  aesmc v0.16b,v0.16b
  aese  v0.16b,v19.16b
  aesmc v0.16b,v0.16b
   ld1  {v16.16b},[x0],x8
  aese  v0.16b,v20.16b
  aesmc v0.16b,v0.16b
  aese  v0.16b,v21.16b
  aesmc v0.16b,v0.16b
  aese  v0.16b,v22.16b
  aesmc v0.16b,v0.16b
   eor  v16.16b,v16.16b,v5.16b
  aese  v0.16b,v23.16b
  eor v6.16b,v0.16b,v7.16b
  b.hs  .Loop_cbc_enc128

  st1 {v6.16b},[x1],#16
  b .Lcbc_done

.align  5
.Lcbc_dec128:
  ld1 {v4.4s-v5.4s},[x7]
  eor v6.16b,v6.16b,v7.16b
  eor v2.16b,v0.16b,v7.16b
  mov x12,x8

.Loop2x_cbc_dec128:
  aesd  v0.16b,v16.16b
  aesd  v1.16b,v16.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
   subs x2,x2,#32
  aesd  v0.16b,v17.16b
  aesd  v1.16b,v17.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
   csel x8,xzr,x8,lo
  aesd  v0.16b,v4.16b
  aesd  v1.16b,v4.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
   csel x12,xzr,x12,ls
  aesd  v0.16b,v5.16b
  aesd  v1.16b,v5.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
  aesd  v0.16b,v18.16b
  aesd  v1.16b,v18.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
  aesd  v0.16b,v19.16b
  aesd  v1.16b,v19.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
  aesd  v0.16b,v20.16b
  aesd  v1.16b,v20.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
  aesd  v0.16b,v21.16b
  aesd  v1.16b,v21.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
  aesd  v0.16b,v22.16b
  aesd  v1.16b,v22.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
  aesd  v0.16b,v23.16b
  aesd  v1.16b,v23.16b

  eor v6.16b,v6.16b,v0.16b
  ld1 {v0.16b},[x0],x8
  eor v2.16b,v2.16b,v1.16b
  ld1 {v1.16b},[x0],x12
  st1 {v6.16b},[x1],#16
  eor v6.16b,v3.16b,v7.16b
  st1 {v2.16b},[x1],#16
  eor v2.16b,v0.16b,v7.16b
  orr v3.16b,v1.16b,v1.16b
  b.hs  .Loop2x_cbc_dec128

  adds  x2,x2,#32
  eor v6.16b,v6.16b,v7.16b
  b.eq  .Lcbc_done
  eor v2.16b,v2.16b,v7.16b
  b .Lcbc_dec_tail

.align  5
.Lcbc_dec:
  subs  x2,x2,#16
  orr v2.16b,v0.16b,v0.16b
  b.lo  .Lcbc_dec_tail

  csel  x8,xzr,x8,eq
  cmp w5,#2
  ld1 {v1.16b},[x0],x8
  orr v3.16b,v1.16b,v1.16b
  b.eq  .Lcbc_dec128

.Loop2x_cbc_dec:
  aesd  v0.16b,v16.16b
  aesd  v1.16b,v16.16b
  ld1 {v16.4s},[x7],#16
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
  subs  w6,w6,#2
  aesd  v0.16b,v17.16b
  aesd  v1.16b,v17.16b
  ld1 {v17.4s},[x7],#16
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
  b.gt  .Loop2x_cbc_dec

  aesd  v0.16b,v16.16b
  aesd  v1.16b,v16.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
   eor  v4.16b,v6.16b,v7.16b
   eor  v5.16b,v2.16b,v7.16b
  aesd  v0.16b,v17.16b
  aesd  v1.16b,v17.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
   orr  v6.16b,v3.16b,v3.16b
   subs x2,x2,#32
  aesd  v0.16b,v18.16b
  aesd  v1.16b,v18.16b
  aesimc  v0.16b,v0.16b
   csel x8,xzr,x8,lo
  aesimc  v1.16b,v1.16b
   mov  x7,x3
  aesd  v0.16b,v19.16b
  aesd  v1.16b,v19.16b
  aesimc  v0.16b,v0.16b
   ld1  {v2.16b},[x0],x8
  aesimc  v1.16b,v1.16b
   csel x8,xzr,x8,ls
  aesd  v0.16b,v20.16b
  aesd  v1.16b,v20.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
   ld1  {v3.16b},[x0],x8
  aesd  v0.16b,v21.16b
  aesd  v1.16b,v21.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
   ld1 {v16.4s},[x7],#16  // re-pre-load rndkey[0]
  aesd  v0.16b,v22.16b
  aesd  v1.16b,v22.16b
  aesimc  v0.16b,v0.16b
  aesimc  v1.16b,v1.16b
   ld1 {v17.4s},[x7],#16  // re-pre-load rndkey[1]
  aesd  v0.16b,v23.16b
  aesd  v1.16b,v23.16b

   mov  w6,w5
  eor v4.16b,v4.16b,v0.16b
  eor v5.16b,v5.16b,v1.16b
   orr  v0.16b,v2.16b,v2.16b
  st1 {v4.16b},[x1],#16
   orr  v1.16b,v3.16b,v3.16b
  st1 {v5.16b},[x1],#16
  b.hs  .Loop2x_cbc_dec

  adds  x2,x2,#32
  b.eq  .Lcbc_done

.Lcbc_dec_tail:
  aesd  v0.16b,v16.16b
  ld1 {v16.4s},[x7],#16
  aesimc  v0.16b,v0.16b
  subs  w6,w6,#2
  aesd  v0.16b,v17.16b
  ld1 {v17.4s},[x7],#16
  aesimc  v0.16b,v0.16b
  b.gt  .Lcbc_dec_tail

  aesd  v0.16b,v16.16b
  aesimc  v0.16b,v0.16b
  aesd  v0.16b,v17.16b
  aesimc  v0.16b,v0.16b
   eor  v4.16b,v6.16b,v7.16b
  aesd  v0.16b,v18.16b
  aesimc  v0.16b,v0.16b
   orr  v6.16b,v2.16b,v2.16b
  aesd  v0.16b,v19.16b
  aesimc  v0.16b,v0.16b
  aesd  v0.16b,v20.16b
  aesimc  v0.16b,v0.16b
  aesd  v0.16b,v21.16b
  aesimc  v0.16b,v0.16b
  aesd  v0.16b,v22.16b
  aesimc  v0.16b,v0.16b
  aesd  v0.16b,v23.16b

  eor v4.16b,v4.16b,v0.16b
  st1 {v4.16b},[x1],#16

.Lcbc_done:
  st1 {v6.16b},[x4]
.Lcbc_abort:
  ldr x29,[sp],#16
  ret
.size aes_v8_cbc_encrypt,.-aes_v8_cbc_encrypt


# void aes_v8_ctr32_encrypt_blocks(const unsigned char *in, unsigned char *out,
#      size_t len, const AES_KEY *key, const unsigned char ivec[16]);
.globl  aes_v8_ctr32_encrypt_blocks
.type aes_v8_ctr32_encrypt_blocks,%function
.align  5
aes_v8_ctr32_encrypt_blocks:
  stp   x29,x30,[sp,#-16]!
  add   x29,sp,#0
  ldr   w5,[x3,#240]

  ldr   w8, [x4, #12]
  ld1   {v0.4s},[x4]

  ld1   {v16.4s-v17.4s},[x3]    // load key schedule...
  sub   w5,w5,#6
  add   x7,x3,x5,lsl#4  // pointer to last 7 round keys
  sub   w5,w5,#2
  ld1   {v18.4s-v19.4s},[x7],#32
  ld1   {v20.4s-v21.4s},[x7],#32
  ld1   {v22.4s-v23.4s},[x7],#32
  ld1   {v7.4s},[x7]

  add   x7,x3,#32
  mov   w6,w5

  subs    x2,x2,#2
  b.lo    .Lctr32_tail

#ifndef __ARMEB__
  rev   w8, w8
#endif
  orr   v1.16b,v0.16b,v0.16b
  add   w8, w8, #1
  orr   v6.16b,v0.16b,v0.16b
  rev   w10, w8
  cmp   w5,#2
  mov   v1.s[3],w10
  b.eq    .Lctr32_128

.Loop2x_ctr32:
  aese    v0.16b,v16.16b
  aese    v1.16b,v16.16b
  ld1   {v16.4s},[x7],#16
  aesmc   v0.16b,v0.16b
  aesmc   v1.16b,v1.16b
  subs    w6,w6,#2
  aese    v0.16b,v17.16b
  aese    v1.16b,v17.16b
  ld1   {v17.4s},[x7],#16
  aesmc   v0.16b,v0.16b
  aesmc   v1.16b,v1.16b
  b.gt    .Loop2x_ctr32

  aese    v0.16b,v16.16b
  aese    v1.16b,v16.16b
  aesmc   v4.16b,v0.16b
   orr    v0.16b,v6.16b,v6.16b
  aesmc   v5.16b,v1.16b
   orr    v1.16b,v6.16b,v6.16b
  aese    v4.16b,v17.16b
  aese    v5.16b,v17.16b
   ld1    {v2.16b},[x0],#16
  aesmc   v4.16b,v4.16b
   ld1    {v3.16b},[x0],#16
  aesmc   v5.16b,v5.16b
   add    w8,w8,#1
  aese    v4.16b,v18.16b
  aese    v5.16b,v18.16b
   rev    w9,w8
  aesmc   v4.16b,v4.16b
  aesmc   v5.16b,v5.16b
   add    w8,w8,#1
  aese    v4.16b,v19.16b
  aese    v5.16b,v19.16b
   eor    v2.16b,v2.16b,v7.16b
   rev    w10,w8
  aesmc   v4.16b,v4.16b
  aesmc   v5.16b,v5.16b
   eor    v3.16b,v3.16b,v7.16b
   mov    x7,x3
  aese    v4.16b,v20.16b
  aese    v5.16b,v20.16b
   subs   x2,x2,#2
  aesmc   v4.16b,v4.16b
  aesmc   v5.16b,v5.16b
   ld1   {v16.4s-v17.4s},[x7],#32 // re-pre-load rndkey[0-1]
  aese    v4.16b,v21.16b
  aese    v5.16b,v21.16b
  aesmc   v4.16b,v4.16b
  aesmc   v5.16b,v5.16b
  aese    v4.16b,v22.16b
  aese    v5.16b,v22.16b
   mov  v0.s[3], w9
  aesmc   v4.16b,v4.16b
   mov  v1.s[3], w10
  aesmc   v5.16b,v5.16b
  aese    v4.16b,v23.16b
  aese    v5.16b,v23.16b

   mov    w6,w5
  eor   v2.16b,v2.16b,v4.16b
  eor   v3.16b,v3.16b,v5.16b
  st1   {v2.16b},[x1],#16
  st1   {v3.16b},[x1],#16
  b.hs    .Loop2x_ctr32

  adds    x2,x2,#2
  b.eq    .Lctr32_done
  b   .Lctr32_tail

.Lctr32_128:
  ld1   {v4.4s-v5.4s},[x7]

.Loop2x_ctr32_128:
  aese    v0.16b,v16.16b
  aese    v1.16b,v16.16b
  aesmc   v0.16b,v0.16b
   ld1    {v2.16b},[x0],#16
  aesmc   v1.16b,v1.16b
   ld1    {v3.16b},[x0],#16
  aese    v0.16b,v17.16b
  aese    v1.16b,v17.16b
   add    w8,w8,#1
  aesmc   v0.16b,v0.16b
  aesmc   v1.16b,v1.16b
   rev    w9,w8
  aese    v0.16b,v4.16b
  aese    v1.16b,v4.16b
   add    w8,w8,#1
  aesmc   v0.16b,v0.16b
  aesmc   v1.16b,v1.16b
   rev    w10,w8
  aese    v0.16b,v5.16b
  aese    v1.16b,v5.16b
   subs   x2,x2,#2
  aesmc   v0.16b,v0.16b
  aesmc   v1.16b,v1.16b
  aese    v0.16b,v18.16b
  aese    v1.16b,v18.16b
  aesmc   v0.16b,v0.16b
  aesmc   v1.16b,v1.16b
  aese    v0.16b,v19.16b
  aese    v1.16b,v19.16b
  aesmc   v0.16b,v0.16b
  aesmc   v1.16b,v1.16b
  aese    v0.16b,v20.16b
  aese    v1.16b,v20.16b
  aesmc   v0.16b,v0.16b
  aesmc   v1.16b,v1.16b
  aese    v0.16b,v21.16b
  aese    v1.16b,v21.16b
  aesmc   v0.16b,v0.16b
  aesmc   v1.16b,v1.16b
  aese    v0.16b,v22.16b
  aese    v1.16b,v22.16b
  aesmc   v0.16b,v0.16b
  aesmc   v1.16b,v1.16b
   eor    v2.16b,v2.16b,v7.16b
  aese    v0.16b,v23.16b
   eor    v3.16b,v3.16b,v7.16b
  aese    v1.16b,v23.16b

  eor   v2.16b,v2.16b,v0.16b
  orr   v0.16b,v6.16b,v6.16b
  eor   v3.16b,v3.16b,v1.16b
  orr   v1.16b,v6.16b,v6.16b
  st1   {v2.16b},[x1],#16
  mov   v0.s[3], w9
  st1   {v3.16b},[x1],#16
  mov   v1.s[3], w10
  b.hs    .Loop2x_ctr32_128

  adds    x2,x2,#2
  b.eq    .Lctr32_done

.Lctr32_tail:
  aese    v0.16b,v16.16b
  ld1   {v16.4s},[x7],#16
  aesmc   v0.16b,v0.16b
  subs    w6,w6,#2
  aese    v0.16b,v17.16b
  ld1   {v17.4s},[x7],#16
  aesmc   v0.16b,v0.16b
  b.gt    .Lctr32_tail

  aese    v0.16b,v16.16b
  aesmc   v0.16b,v0.16b
  aese    v0.16b,v17.16b
  aesmc   v0.16b,v0.16b
   ld1    {v2.16b},[x0]
  aese    v0.16b,v18.16b
  aesmc   v0.16b,v0.16b
  aese    v0.16b,v19.16b
  aesmc   v0.16b,v0.16b
  aese    v0.16b,v20.16b
  aesmc   v0.16b,v0.16b
  aese    v0.16b,v21.16b
  aesmc   v0.16b,v0.16b
  aese    v0.16b,v22.16b
  aesmc   v0.16b,v0.16b
   eor    v2.16b,v2.16b,v7.16b
  aese    v0.16b,v23.16b

  eor   v2.16b,v2.16b,v0.16b
  st1   {v2.16b},[x1]

.Lctr32_done:
  ldr   x29,[sp],#16
  ret
.size aes_v8_ctr32_encrypt_blocks,.-aes_v8_ctr32_encrypt_blocks


#endif
