/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Compat <crypto/chacha.h> for kernels without lib/crypto chacha (< 4.20).
 * Provides chacha_init() and chacha20_crypt() with the same semantics as
 * the 5.x kernel API, backed by the zinc chacha20 implementation that the
 * module already builds for old kernels.
 */
#ifndef _COMPAT_CRYPTO_CHACHA_H
#define _COMPAT_CRYPTO_CHACHA_H

#include <zinc/chacha20.h>

#define CHACHA_IV_SIZE		16
#define CHACHA_KEY_SIZE		32
#define CHACHA_BLOCK_SIZE	64
#define CHACHA_STATE_WORDS	(CHACHA_BLOCK_SIZE / sizeof(u32))

static inline void chacha_init(u32 *state, const u32 *key, const u8 *iv)
{
	state[0]  = CHACHA20_CONSTANT_EXPA;
	state[1]  = CHACHA20_CONSTANT_ND_3;
	state[2]  = CHACHA20_CONSTANT_2_BY;
	state[3]  = CHACHA20_CONSTANT_TE_K;
	state[4]  = key[0];
	state[5]  = key[1];
	state[6]  = key[2];
	state[7]  = key[3];
	state[8]  = key[4];
	state[9]  = key[5];
	state[10] = key[6];
	state[11] = key[7];
	state[12] = get_unaligned_le32(iv +  0);
	state[13] = get_unaligned_le32(iv +  4);
	state[14] = get_unaligned_le32(iv +  8);
	state[15] = get_unaligned_le32(iv + 12);
}

static inline void chacha20_crypt(u32 *state, u8 *dst, const u8 *src,
				  unsigned int bytes)
{
	/* Used for short header protection, also with BH disabled: scalar only. */
	chacha20((struct chacha20_ctx *)state, dst, src, bytes, DONT_USE_SIMD);
}

#endif /* _COMPAT_CRYPTO_CHACHA_H */
