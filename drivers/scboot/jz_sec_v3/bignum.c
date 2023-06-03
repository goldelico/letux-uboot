#include <div64.h>
#include <malloc.h>
#include "bignum.h"

static bn_t bn_sub_digit_mul(bn_t *a, bn_t *b, bn_t c, bn_t *d, u32 digits);
static bn_t bn_add_digit_mul(bn_t *a, bn_t *b, bn_t c, bn_t *d, u32 digits);
static u32 bn_digit_bits(bn_t a);

static u32 cache_space[1024];
static int cache_space_pos = 0;
void * rsa_malloc(int size){
	u32 *d = (u32 *)&cache_space[cache_space_pos];
	d[0] = size;
	cache_space_pos += (size + 3) / 4 + 1;
	return (void*)&d[1];

}
void rsa_free(void *d)
{
	u32 *v = (u32*)d;
	u32 size = v[-1];
	cache_space_pos -= (size + 3) / 4 + 1;
}

void bn_encode(u8 *hexarr, u32 size, bn_t *bn, u32 digits)
{
	bn_t t;
	int j;
	u32 i, u;

	for(i=0,j=size-1; i<digits && j>=0; i++) {
		t = bn[i];
		for(u=0; j>=0 && u<BN_DIGIT_BITS; j--, u+=8) {
			hexarr[j] = (u8)(t >> u);
		}
	}

	for(; j>=0; j--) {
		hexarr[j] = 0;
	}
}

void bn_assign(bn_t *a, bn_t *b, u32 digits)
{
	u32 i;
	for(i=0; i<digits; i++) {
		a[i] = b[i];
	}
}

void bn_assign_zero(bn_t *a, u32 digits)
{
	u32 i;
	for(i=0; i<digits; i++) {
		a[i] = 0;
	}
}

bn_t bn_add(bn_t *a, bn_t *b, bn_t *c, u32 digits)
{
	bn_t ai, carry;
	u32 i;

	carry = 0;
	for(i=0; i<digits; i++) {
		if((ai = b[i] + carry) < carry) {
			ai = c[i];
		} else if((ai += c[i]) < c[i]) {
			carry = 1;
		} else {
			carry = 0;
		}
		a[i] = ai;
	}

	return carry;
}

bn_t bn_sub(bn_t *a, bn_t *b, bn_t *c, u32 digits)
{
	bn_t ai, borrow;
	u32 i;

	borrow = 0;
	for(i=0; i<digits; i++) {
		if((ai = b[i] - borrow) > (BN_MAX_DIGIT - borrow)) {
			ai = BN_MAX_DIGIT - c[i];
		} else if((ai -= c[i]) > (BN_MAX_DIGIT - c[i])) {
			borrow = 1;
		} else {
			borrow = 0;
		}
		a[i] = ai;
	}

	return borrow;
}

void bn_mul(bn_t *a, bn_t *b, bn_t *c, u32 digits)
{
	bn_t *t;
	u32 bdigits, cdigits, i;

	t = (bn_t *)rsa_malloc(BN_MAX_DIGITS * sizeof(bn_t) * 2);

	bn_assign_zero(t, 2*digits);
	bdigits = bn_digits(b, digits);
	cdigits = bn_digits(c, digits);

	for(i=0; i<bdigits; i++) {
		t[i+cdigits] += bn_add_digit_mul(&t[i], &t[i], b[i], c, cdigits);
	}

	bn_assign(a, t, 2*digits);

	rsa_free(t);

	// Clear potentially sensitive information
	/* rsa_memset(t, 0, sizeof(t) / sizeof(unsigned int)); */
}
#if 0
bn_t __div64_32(dbn_t *n, bn_t base)
{
	dbn_t rem = *n;
	dbn_t b = base;
	dbn_t res, d = 1;
	bn_t high = rem >> 32;

	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (dbn_t) high << 32;
		rem -= (dbn_t) (high*base) << 32;
	}

	while ((long long)b > 0 && b < rem) {
		b = b+b;
		d = d+d;
	}

	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while (d);

	*n = res;
	return rem;
}

/* The unnecessary pointer compare is there
 * to check for type safety (n must be 64bit)
 */
# define do_div(n,base) ({								\
		bn_t __base = (base);						\
		bn_t __rem;									\
		(void)(((typeof((n)) *)0) == ((dbn_t *)0));	\
		if (((n) >> 32) == 0) {						\
		__rem = (bn_t)(n) % __base;				\
		(n) = (bn_t)(n) / __base;				\
		} else										\
		__rem = __div64_32(&(n), __base);		\
		__rem;										\
		})
#endif

void bn_div(bn_t *a, bn_t *b, bn_t *c, u32 cdigits, bn_t *d, u32 ddigits)
{
	dbn_t tmp;
	bn_t ai, t;
	bn_t *cc, *dd;
	int i;
	u32 dddigits, shift;
	cc = (bn_t *)rsa_malloc((BN_MAX_DIGITS + 1) * sizeof(bn_t) * 2);
	dd = (bn_t *)rsa_malloc(BN_MAX_DIGITS * sizeof(bn_t));

	dddigits = bn_digits(d, ddigits);
	if(dddigits == 0)
		return;

	shift = BN_DIGIT_BITS - bn_digit_bits(d[dddigits-1]);
	bn_assign_zero(cc, dddigits);
	cc[cdigits] = bn_shift_l(cc, c, shift, cdigits);
	bn_shift_l(dd, d, shift, dddigits);
	t = dd[dddigits-1];

	bn_assign_zero(a, cdigits);
	i = cdigits - dddigits;
	for(; i>=0; i--) {
		if(t == BN_MAX_DIGIT) {
			ai = cc[i+dddigits];
		} else {
			tmp = cc[i+dddigits-1];
			tmp += (dbn_t)cc[i+dddigits] << BN_DIGIT_BITS;
			{
				// ai = tmp / (t + 1);
				dbn_t tmp_u64;
				tmp_u64 = tmp;
				do_div(tmp_u64,(t + 1));
				ai = tmp_u64;
			}
		}

		cc[i+dddigits] -= bn_sub_digit_mul(&cc[i], &cc[i], ai, dd, dddigits);
		// printf("cc[%d]: %08X\n", i, cc[i+dddigits]);
		while(cc[i+dddigits] || (bn_cmp(&cc[i], dd, dddigits) >= 0)) {
			ai++;
			cc[i+dddigits] -= bn_sub(&cc[i], &cc[i], dd, dddigits);
		}
		a[i] = ai;
		// printf("ai[%d]: %08X\n", i, ai);
	}

	bn_assign_zero(b, ddigits);
	bn_shift_r(b, cc, shift, dddigits);

	rsa_free(dd);
	rsa_free(cc);
	/* // Clear potentially sensitive information */
	/* rsa_memset(cc, 0, sizeof(cc) / sizeof(unsigned int)); */
	/* rsa_memset(dd, 0, sizeof(dd) / sizeof(unsigned int)); */
}
bn_t bn_shift_l(bn_t *a, bn_t *b, u32 c, u32 digits)
{
	bn_t bi, carry;
	u32 i, t;

	if(c >= BN_DIGIT_BITS)
		return 0;

	t = BN_DIGIT_BITS - c;
	carry = 0;
	for(i=0; i<digits; i++) {
		bi = b[i];
		a[i] = (bi << c) | carry;
		carry = c ? (bi >> t) : 0;
	}

	return carry;
}

bn_t bn_shift_r(bn_t *a, bn_t *b, u32 c, u32 digits)
{
	bn_t bi, carry;
	int i;
	u32 t;

	if(c >= BN_DIGIT_BITS)
		return 0;

	t = BN_DIGIT_BITS - c;
	carry = 0;
	i = digits - 1;
	for(; i>=0; i--) {
		bi = b[i];
		a[i] = (bi >> c) | carry;
		carry = c ? (bi << t) : 0;
	}

	return carry;
}

void bn_mod(bn_t *a, bn_t *b, u32 bdigits, bn_t *c, u32 cdigits)
{
	bn_t *t;
	t = (bn_t *)rsa_malloc(BN_MAX_DIGITS * sizeof(bn_t) * 2);

	//	rsa_memset(t, 0, sizeof(t) / sizeof(unsigned int));

	bn_div(t, a, b, bdigits, c, cdigits);

	rsa_free(t);
	// Clear potentially sensitive information
	/* rsa_memset(t, 0, sizeof(t) / sizeof(unsigned int)); */
}

void bn_mod_mul(bn_t *a, bn_t *b, bn_t *c, bn_t *d, u32 digits)
{
	bn_t *t;
	t = (bn_t *)rsa_malloc(BN_MAX_DIGITS * sizeof(bn_t) * 2);

	bn_mul(t, b, c, digits);
	bn_mod(a, t, 2*digits, d, digits);

	rsa_free(t);
	/* // Clear potentially sensitive information */
	/* rsa_memset(t, 0, sizeof(t) / sizeof(unsigned int)); */
}

void bn_mod_exp(bn_t *a, bn_t *b, bn_t *c, u32 cdigits, bn_t *d, u32 ddigits)
{
	bn_t ci;
	bn_t *bpower[3],*t;
	int i;
	u32 ci_bits, j, s;
	bpower[0] = (bn_t *)rsa_malloc(BN_MAX_DIGITS * sizeof(bn_t));
	bpower[1] = (bn_t *)rsa_malloc(BN_MAX_DIGITS * sizeof(bn_t));
	bpower[2] = (bn_t *)rsa_malloc(BN_MAX_DIGITS * sizeof(bn_t));
	t = (bn_t *)rsa_malloc(BN_MAX_DIGITS * sizeof(bn_t));
	bn_assign(bpower[0], b, ddigits);
	bn_mod_mul(bpower[1], bpower[0], b, d, ddigits);
	bn_mod_mul(bpower[2], bpower[1], b, d, ddigits);

	BN_ASSIGN_DIGIT(t, 1, ddigits);

	cdigits = bn_digits(c, cdigits);
	i = cdigits - 1;
	for(; i>=0; i--) {
		ci = c[i];
		ci_bits = BN_DIGIT_BITS;

		if(i == (int)(cdigits - 1)) {
			while(!DIGIT_2MSB(ci)) {
				ci <<= 2;
				ci_bits -= 2;
			}
		}

		for(j=0; j<ci_bits; j+=2) {
			bn_mod_mul(t, t, t, d, ddigits);
			bn_mod_mul(t, t, t, d, ddigits);
			if((s = DIGIT_2MSB(ci)) != 0) {
				bn_mod_mul(t, t, bpower[s-1], d, ddigits);
			}
			ci <<= 2;
		}
	}

	bn_assign(a, t, ddigits);
	rsa_free(t);
	rsa_free(bpower[2]);
	rsa_free(bpower[1]);
	rsa_free(bpower[0]);
	/* // Clear potentially sensitive information */
	/* rsa_memset((u32*)bpower, 0, sizeof(bpower) / sizeof(unsigned int)); */
	/* rsa_memset((u32*)t, 0, sizeof(t) / sizeof(unsigned int)); */
}

int bn_cmp(bn_t *a, bn_t *b, u32 digits)
{
	int i;
	for(i=digits-1; i>=0; i--) {
		if(a[i] > b[i])     return 1;
		if(a[i] < b[i])     return -1;
	}

	return 0;
}

u32 bn_digits(bn_t *a, u32 digits)
{
	int i;
	for(i=digits-1; i>=0; i--) {
		if(a[i])    break;
	}

	return (i + 1);
}

static bn_t bn_add_digit_mul(bn_t *a, bn_t *b, bn_t c, bn_t *d, u32 digits)
{
	dbn_t result;
	bn_t carry, rh, rl;
	u32 i;

	if(c == 0)
		return 0;

	carry = 0;
	for(i=0; i<digits; i++) {
		result = (dbn_t)c * d[i];
		rl = result & BN_MAX_DIGIT;
		rh = (result >> BN_DIGIT_BITS) & BN_MAX_DIGIT;
		if((a[i] = b[i] + carry) < carry) {
			carry = 1;
		} else {
			carry = 0;
		}
		if((a[i] += rl) < rl) {
			carry++;
		}
		carry += rh;
	}

	return carry;
}

static bn_t bn_sub_digit_mul(bn_t *a, bn_t *b, bn_t c, bn_t *d, u32 digits)
{
	dbn_t result;
	bn_t borrow, rh, rl;
	u32 i;

	if(c == 0)
		return 0;

	borrow = 0;
	for(i=0; i<digits; i++) {
		result = (dbn_t)c * d[i];
		rl = result & BN_MAX_DIGIT;
		rh = (result >> BN_DIGIT_BITS) & BN_MAX_DIGIT;
		if((a[i] = b[i] - borrow) > (BN_MAX_DIGIT - borrow)) {
			borrow = 1;
		} else {
			borrow = 0;
		}
		if((a[i] -= rl) > (BN_MAX_DIGIT - rl)) {
			borrow++;
		}
		borrow += rh;
	}

	return borrow;
}
static u32 bn_digit_bits(bn_t a)
{
	u32 i;
	for(i=0; i<BN_DIGIT_BITS; i++) {
		if(a == 0)  break;
		a >>= 1;
	}

	return i;
}
