#ifndef __RSA_H__
#define __RSA_H__

// RSA key lengths
#define RSA_MAX_MODULUS_BITS                2048
#define RSA_MAX_MODULUS_LEN                 ((RSA_MAX_MODULUS_BITS + 7) / 8)
#define RSA_MAX_PRIME_BITS                  ((RSA_MAX_MODULUS_BITS + 1) / 2)
#define RSA_MAX_PRIME_LEN                   ((RSA_MAX_PRIME_BITS + 7) / 8)

// Error codes
#define ERR_WRONG_DATA                      0x1001
#define ERR_WRONG_LEN                       0x1002

typedef unsigned int bn_t;
extern void (*f_rsa_public_decrypt)(bn_t*, bn_t*, bn_t, bn_t*, bn_t*, bn_t);


#endif  // __RSA_H__
