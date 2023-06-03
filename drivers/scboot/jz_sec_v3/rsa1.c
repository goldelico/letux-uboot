#include "rsa1.h"
#include "bignum.h"

void rsa_public_decrypt(bn_t *out, bn_t *in, bn_t in_len, bn_t * n, bn_t * e, bn_t key_len)
{
	bn_t c[BN_MAX_DIGITS];

	bn_mod_exp(c, in, e, 1, n, key_len);

	bn_encode((unsigned char *)out, 256, c, in_len);
}
void (*f_rsa_public_decrypt)(bn_t*, bn_t*, bn_t, bn_t*, bn_t*, bn_t) = rsa_public_decrypt;

