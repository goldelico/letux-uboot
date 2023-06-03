#ifndef __BIGNUM_H__
#define __BIGNUM_H__

#include <asm/types.h>
typedef unsigned long long dbn_t;
typedef u32 bn_t;

#define BN_DIGIT_BITS               32      // For u32
#define BN_MAX_DIGITS               65      // RSA_MAX_MODULUS_LEN + 1

#define BN_MAX_DIGIT                0xFFFFFFFF

#define DIGIT_2MSB(x)               (u32)(((x) >> (BN_DIGIT_BITS - 2)) & 0x03)


void bn_decode(bn_t *bn, u32 digits, u8 *hexarr, u32 size);
void bn_encode(u8 *hexarr, u32 size, bn_t *bn, u32 digits);

void bn_assign(bn_t *a, bn_t *b, u32 digits);                                          // a = b
void bn_assign_zero(bn_t *a, u32 digits);                                              // a = 0

bn_t bn_add(bn_t *a, bn_t *b, bn_t *c, u32 digits);                                    // a = b + c, return carry
bn_t bn_sub(bn_t *a, bn_t *b, bn_t *c, u32 digits);                                    // a = b - c, return borrow
void bn_mul(bn_t *a, bn_t *b, bn_t *c, u32 digits);                                    // a = b * c
void bn_div(bn_t *a, bn_t *b, bn_t *c, u32 cdigits, bn_t *d, u32 ddigits);        // a = b / c, d = b % c
bn_t bn_shift_l(bn_t *a, bn_t *b, u32 c, u32 digits);                             // a = b << c (a = b * 2^c)
bn_t bn_shift_r(bn_t *a, bn_t *b, u32 c, u32 digits);                             // a = b >> c (a = b / 2^c)

void bn_mod(bn_t *a, bn_t *b, u32 bdigits, bn_t *c, u32 cdigits);                 // a = b mod c
void bn_mod_mul(bn_t *a, bn_t *b, bn_t *c, bn_t *d, u32 digits);                       // a = b * c mod d
void bn_mod_exp(bn_t *a, bn_t *b, bn_t *c, u32 cdigits, bn_t *d, u32 ddigits);    // a = b ^ c mod d

int bn_cmp(bn_t *a, bn_t *b, u32 digits);                                              // returns sign of a - b

u32 bn_digits(bn_t *a, u32 digits);                                               // returns significant length of a in digits

#define BN_ASSIGN_DIGIT(a, b, digits)   {bn_assign_zero(a, digits); a[0] = b;}

void *rsa_memset(u32 *p, u32 c, u32 nword);
#endif  // __BIGNUM_H__
