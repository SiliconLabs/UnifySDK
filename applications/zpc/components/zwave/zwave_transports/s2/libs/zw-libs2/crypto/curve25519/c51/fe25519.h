/* © 2017 Silicon Laboratories Inc.
 */
#ifndef FE25519_H
#define FE25519_H

#include <stdint.h>

#ifdef NOT_USED
#define fe25519_freeze avrnacl_fe25519_freeze
#define fe25519_unpack avrnacl_fe25519_unpack
#define fe25519_pack avrnacl_fe25519_pack
#define fe25519_iszero avrnacl_fe25519_iszero
#define fe25519_iseq_vartime avrnacl_fe25519_iseq_vartime
#define fe25519_cmov avrnacl_fe25519_cmov
#define fe25519_setone avrnacl_fe25519_setone
#define fe25519_setzero avrnacl_fe25519_setzero
#define fe25519_neg avrnacl_fe25519_neg
#define fe25519_getparity avrnacl_fe25519_getparity
#define fe25519_add avrnacl_fe25519_add
#define fe25519_sub avrnacl_fe25519_sub
#define fe25519_mul avrnacl_fe25519_mul
#define fe25519_square avrnacl_fe25519_square
#define fe25519_invert avrnacl_fe25519_invert
#define fe25519_pow2523 avrnacl_fe25519_pow2523
#endif

typedef struct
{
  uint8_t value[32];
}
fe25519_t;

void fe25519_freeze(fe25519_t  *r);

void fe25519_unpack(fe25519_t  *r, const uint8_t  *x);

void fe25519_pack(uint8_t  *r, const fe25519_t  *x);

#if 1//def NOT_USED
int fe25519_iszero(const fe25519_t  *x);

void fe25519_neg(fe25519_t  *r, const fe25519_t  *x);

int fe25519_iseq_vartime(const fe25519_t  *x, const fe25519_t  *y);

unsigned char fe25519_getparity(const fe25519_t  *x);

void fe25519_pow2523(fe25519_t  *r, const fe25519_t  *x);
#endif

void fe25519_cmov(fe25519_t  *r, const fe25519_t  *x, uint8_t b);

void fe25519_setone(fe25519_t  *r);

void fe25519_setzero(fe25519_t  *r);

void fe25519_add(fe25519_t  *r, const fe25519_t  *x, const fe25519_t  *y);

void fe25519_sub(fe25519_t  *r, const fe25519_t  *x, const fe25519_t  *y);

void fe25519_mul(fe25519_t  *r, const fe25519_t  *x, const fe25519_t  *y);

void fe25519_square(fe25519_t  *r, const fe25519_t  *x);

void fe25519_invert(fe25519_t  *r, const fe25519_t  *x);

#endif
