/*
 *  This file is part of the optimized implementation of the Picnic signature scheme.
 *  See the accompanying documentation for complete details.
 *
 *  The code is provided under the MIT license, see LICENSE for
 *  more details.
 *  SPDX-License-Identifier: MIT
 */

#if defined(LOWMC_INSTANCE)
#define lowmc LOWMC_INSTANCE
#else
#define lowmc lowmc_instance
#endif

static mzd_local_t* N_LOWMC(lowmc_t const* lowmc_instance, lowmc_key_t const* lowmc_key,
                            mzd_local_t const* p) {
#if defined(LOWMC_INSTANCE)
  (void)lowmc_instance;
#endif
#if defined(REDUCED_LINEAR_LAYER)
  mzd_local_t* x       = mzd_local_init_ex(1, LOWMC_N, false);
  mzd_local_t* y       = mzd_local_init_ex(1, LOWMC_N, false);
  mzd_local_t* nl_part = mzd_local_init_ex(1, LOWMC_R * 32, false);

  XOR(x, p, lowmc->precomputed_constant_linear);
  ADDMUL(x, lowmc_key, CONCAT(lowmc->k0, matrix_postfix));
  MUL_MC(nl_part, lowmc_key, CONCAT(lowmc->precomputed_non_linear_part, matrix_postfix));
  XOR_MC(nl_part, nl_part, lowmc->precomputed_constant_non_linear);

  lowmc_round_t const* round = lowmc->rounds;
  for (unsigned i = 0; i < LOWMC_R; ++i, ++round) {
    SBOX(x, &lowmc->mask);

    const word nl = CONST_FIRST_ROW(nl_part)[i >> 1];
    FIRST_ROW(x)
    [(LOWMC_N) / (sizeof(word) * 8) - 1] ^=
        (i & 1) ? (nl & WORD_C(0xFFFFFFFF00000000)) : (nl << 32);
    MUL(y, x, CONCAT(round->l, matrix_postfix));

    // swap x and y
    mzd_local_t* t = x;
    x              = y;
    y              = t;
  }

  mzd_local_free(y);
  mzd_local_free(nl_part);
  return x;
#else
  mzd_local_t* x = mzd_local_init_ex(1, LOWMC_N, false);
  mzd_local_t* y = mzd_local_init_ex(1, LOWMC_N, false);

  mzd_local_copy(x, p);
  ADDMUL(x, lowmc_key, CONCAT(lowmc->k0, matrix_postfix));

  lowmc_round_t const* round = lowmc->rounds;
  for (unsigned int i = LOWMC_R; i; --i, ++round) {
    SBOX(x, &lowmc->mask);

    MUL(y, x, CONCAT(round->l, matrix_postfix));
    XOR(x, y, round->constant);
    ADDMUL(x, lowmc_key, CONCAT(round->k, matrix_postix));
  }

  mzd_local_free(y);
  return x;
#endif
}

#undef lowmc

// vim: ft=c