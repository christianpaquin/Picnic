/*
 *  This file is part of the optimized implementation of the Picnic signature scheme.
 *  See the accompanying documentation for complete details.
 *
 *  The code is provided under the MIT license, see LICENSE for
 *  more details.
 *  SPDX-License-Identifier: MIT
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "io.h"
#include "lowmc.h"
#include "mzd_additional.h"
#if defined(WITH_KKW)
#include "picnic2_impl.h"
#include "picnic2_simulate_mul.h"
#endif
#if defined(WITH_OPT)
#include "simd.h"
#endif

#if !defined(_MSC_VER)
#include <stdalign.h>
#endif
#include <string.h>
static const mzd_local_t mask_126_126_42_a[1] = {
    {{UINT64_C(0x4924924924924924), UINT64_C(0x2492492492492492), UINT64_C(0x0), UINT64_C(0x0)}}};
static const mzd_local_t mask_126_126_42_b[1] = {
    {{UINT64_C(0x9249249249249248), UINT64_C(0x4924924924924924), UINT64_C(0x0), UINT64_C(0x0)}}};
static const mzd_local_t mask_126_126_42_c[1] = {
    {{UINT64_C(0x2492492492492490), UINT64_C(0x9249249249249249), UINT64_C(0x0), UINT64_C(0x0)}}};

static const mzd_local_t mask_192_192_64_a[1] = {
    {{UINT64_C(0x9249249249249249), UINT64_C(0x4924924924924924), UINT64_C(0x2492492492492492),
      UINT64_C(0x0)}}};
static const mzd_local_t mask_192_192_64_b[1] = {
    {{UINT64_C(0x2492492492492492), UINT64_C(0x9249249249249249), UINT64_C(0x4924924924924924),
      UINT64_C(0x0)}}};
static const mzd_local_t mask_192_192_64_c[1] = {
    {{UINT64_C(0x4924924924924924), UINT64_C(0x2492492492492492), UINT64_C(0x9249249249249249),
      UINT64_C(0x0)}}};

static const mzd_local_t mask_255_255_85_a[1] = {
    {{UINT64_C(0x2492492492492492), UINT64_C(0x9249249249249249), UINT64_C(0x4924924924924924),
      UINT64_C(0x2492492492492492)}}};
static const mzd_local_t mask_255_255_85_b[1] = {
    {{UINT64_C(0x4924924924924924), UINT64_C(0x2492492492492492), UINT64_C(0x9249249249249249),
      UINT64_C(0x4924924924924924)}}};
static const mzd_local_t mask_255_255_85_c[1] = {
    {{UINT64_C(0x9249249249249248), UINT64_C(0x4924924924924924), UINT64_C(0x2492492492492492),
      UINT64_C(0x9249249249249249)}}};

/**
 * S-box for m = 42
 */
static void sbox_layer_42(mzd_local_t* in) {
  mzd_local_t x0m[1], x1m[1], x2m[1];
  // a
  mzd_and_uint64_128(x0m, mask_126_126_42_a, in);
  // b
  mzd_and_uint64_128(x1m, mask_126_126_42_b, in);
  // c
  mzd_and_uint64_128(x2m, mask_126_126_42_c, in);

  mzd_shift_left_uint64_128(x0m, x0m, 2);
  mzd_shift_left_uint64_128(x1m, x1m, 1);

  mzd_local_t t0[1], t1[1], t2[1];
  // b & c
  mzd_and_uint64_128(t0, x1m, x2m);
  // c & a
  mzd_and_uint64_128(t1, x0m, x2m);
  // a & b
  mzd_and_uint64_128(t2, x0m, x1m);

  // (b & c) ^ a
  mzd_xor_uint64_128(t0, t0, x0m);

  // (c & a) ^ a ^ b
  mzd_xor_uint64_128(t1, t1, x0m);
  mzd_xor_uint64_128(t1, t1, x1m);

  // (a & b) ^ a ^ b ^c
  mzd_xor_uint64_128(t2, t2, x0m);
  mzd_xor_uint64_128(t2, t2, x1m);
  mzd_xor_uint64_128(t2, t2, x2m);

  mzd_shift_right_uint64_128(t0, t0, 2);
  mzd_shift_right_uint64_128(t1, t1, 1);

  mzd_xor_uint64_128(in, t2, t1);
  mzd_xor_uint64_128(in, in, t0);
}

/**
 * S-box for m = 64
 */
static void sbox_layer_64(mzd_local_t* in) {
  mzd_local_t x0m[1], x1m[1], x2m[1];
  // a
  mzd_and_uint64_192(x0m, mask_192_192_64_a, in);
  // b
  mzd_and_uint64_192(x1m, mask_192_192_64_b, in);
  // c
  mzd_and_uint64_192(x2m, mask_192_192_64_c, in);

  mzd_rotate_left_uint64_192(x0m, x0m, 2);
  mzd_rotate_left_uint64_192(x0m, x1m, 1);

  mzd_local_t t0[1], t1[1], t2[1];
  // b & c
  mzd_and_uint64_192(t0, x1m, x2m);
  // c & a
  mzd_and_uint64_192(t1, x0m, x2m);
  // a & b
  mzd_and_uint64_192(t2, x0m, x1m);

  // (b & c) ^ a
  mzd_xor_uint64_192(t0, t0, x0m);

  // (c & a) ^ a ^ b
  mzd_xor_uint64_192(t1, t1, x0m);
  mzd_xor_uint64_192(t1, t1, x1m);

  // (a & b) ^ a ^ b ^c
  mzd_xor_uint64_192(t2, t2, x0m);
  mzd_xor_uint64_192(t2, t2, x1m);
  mzd_xor_uint64_192(t2, t2, x2m);

  mzd_rotate_right_uint64_192(t0, t0, 2);
  mzd_rotate_right_uint64_192(t1, t1, 1);

  mzd_xor_uint64_192(in, t2, t1);
  mzd_xor_uint64_192(in, in, t0);
}

/**
 * S-box for m = 85
 */
static void sbox_layer_85(mzd_local_t* in) {
  mzd_local_t x0m[1], x1m[1], x2m[1];
  // a
  mzd_and_uint64_256(x0m, mask_255_255_85_a, in);
  // b
  mzd_and_uint64_256(x1m, mask_255_255_85_b, in);
  // c
  mzd_and_uint64_256(x2m, mask_255_255_85_c, in);

  mzd_rotate_left_uint64_256(x0m, x0m, 2);
  mzd_rotate_left_uint64_256(x1m, x1m, 1);

  mzd_local_t t0[1], t1[1], t2[1];
  // b & c
  mzd_and_uint64_256(t0, x1m, x2m);
  // c & a
  mzd_and_uint64_256(t1, x0m, x2m);
  // a & b
  mzd_and_uint64_256(t2, x0m, x1m);

  // (b & c) ^ a
  mzd_xor_uint64_256(t0, t0, x0m);

  // (c & a) ^ a ^ b
  mzd_xor_uint64_256(t1, t1, x0m);
  mzd_xor_uint64_256(t1, t1, x1m);

  // (a & b) ^ a ^ b ^c
  mzd_xor_uint64_256(t2, t2, x0m);
  mzd_xor_uint64_256(t2, t2, x1m);
  mzd_xor_uint64_256(t2, t2, x2m);

  mzd_rotate_right_uint64_256(t0, t0, 2);
  mzd_rotate_right_uint64_256(t1, t1, 1);

  mzd_xor_uint64_256(in, t2, t1);
  mzd_xor_uint64_256(in, in, t0);
}

#if defined(WITH_LOWMC_126_126_4)
#include "lowmc_126_126_4.h"
#endif
#if defined(WITH_LOWMC_192_192_4)
#include "lowmc_192_192_4.h"
#endif
#if defined(WITH_LOWMC_255_255_4)
#include "lowmc_255_255_4.h"
#endif

#if !defined(NO_UINT64_FALLBACK)
// uint64 based implementation
#include "lowmc_fns_uint64_L1.h"
#define LOWMC lowmc_uint64_126
#include "lowmc.c.i"

#include "lowmc_fns_uint64_L3.h"
#undef LOWMC
#define LOWMC lowmc_uint64_192
#include "lowmc.c.i"

#include "lowmc_fns_uint64_L5.h"
#undef LOWMC
#define LOWMC lowmc_uint64_255
#include "lowmc.c.i"
#endif

#if defined(WITH_OPT)
#if defined(WITH_SSE2) || defined(WITH_NEON)
#if defined(WITH_SSE2)
#define FN_ATTR ATTR_TARGET_SSE2
#endif

// L1 using SSE2/NEON
#include "lowmc_fns_s128_L1.h"
#undef LOWMC
#define LOWMC lowmc_s128_126
#include "lowmc.c.i"

// L3 using SSE2/NEON
#include "lowmc_fns_s128_L3.h"
#undef LOWMC
#define LOWMC lowmc_s128_192
#include "lowmc.c.i"

// L5 using SSE2/NEON
#include "lowmc_fns_s128_L5.h"
#undef LOWMC
#define LOWMC lowmc_s128_255
#include "lowmc.c.i"

#undef FN_ATTR
#endif

#if defined(WITH_AVX2)
#define FN_ATTR ATTR_TARGET_AVX2

// L1 using AVX2
#include "lowmc_fns_s256_L1.h"
#undef LOWMC
#define LOWMC lowmc_s256_126
#include "lowmc.c.i"

// L3 using AVX2
#include "lowmc_fns_s256_L3.h"
#undef LOWMC
#define LOWMC lowmc_s256_192
#include "lowmc.c.i"

// L5 using AVX2
#include "lowmc_fns_s256_L5.h"
#undef LOWMC
#define LOWMC lowmc_s256_255
#include "lowmc.c.i"

#undef FN_ATTR

#endif
#endif

lowmc_implementation_f lowmc_get_implementation(const lowmc_t* lowmc) {
  ASSUME(lowmc->m == 42 || lowmc->m == 64 || lowmc->m == 85);
  ASSUME(lowmc->n == 126 || lowmc->n == 192 || lowmc->n == 255);

#if defined(WITH_OPT)
#if defined(WITH_AVX2)
  if (CPU_SUPPORTS_AVX2) {
#if defined(WITH_LOWMC_126_126_4)
    if (lowmc->n == 126 && lowmc->m == 42)
      return lowmc_s256_126_42;
#endif
#if defined(WITH_LOWMC_192_192_4)
    if (lowmc->n == 192 && lowmc->m == 64)
      return lowmc_s256_192_64;
#endif
#if defined(WITH_LOWMC_255_255_4)
    if (lowmc->n == 255 && lowmc->m == 85)
      return lowmc_s256_255_85;
#endif
  }
#endif
#if defined(WITH_SSE2) || defined(WITH_NEON)
  if (CPU_SUPPORTS_SSE2 || CPU_SUPPORTS_NEON) {
#if defined(WITH_LOWMC_126_126_4)
    if (lowmc->n == 126 && lowmc->m == 42)
      return lowmc_s128_126_42;
#endif
#if defined(WITH_LOWMC_192_192_4)
    if (lowmc->n == 192 && lowmc->m == 64)
      return lowmc_s128_192_64;
#endif
#if defined(WITH_LOWMC_255_255_4)
    if (lowmc->n == 255 && lowmc->m == 85)
      return lowmc_s128_255_85;
#endif
  }
#endif
#endif

#if !defined(NO_UINT64_FALLBACK)
#if defined(WITH_LOWMC_126_126_4)
  if (lowmc->n == 126 && lowmc->m == 42)
    return lowmc_uint64_126_42;
#endif
#if defined(WITH_LOWMC_192_192_4)
  if (lowmc->n == 192 && lowmc->m == 64)
    return lowmc_uint64_192_64;
#endif
#if defined(WITH_LOWMC_255_255_4)
  if (lowmc->n == 255 && lowmc->m == 85)
    return lowmc_uint64_255_85;
#endif
#endif

  return NULL;
}

#if defined(WITH_ZKBPP)
lowmc_store_implementation_f lowmc_store_get_implementation(const lowmc_t* lowmc) {
  ASSUME(lowmc->m == 42 || lowmc->m == 64 || lowmc->m == 85);
  ASSUME(lowmc->n == 126 || lowmc->n == 192 || lowmc->n == 255);

#if defined(WITH_OPT)
#if defined(WITH_AVX2)
  if (CPU_SUPPORTS_AVX2) {
#if defined(WITH_LOWMC_126_126_4)
  if (lowmc->n == 126 && lowmc->m == 42)
    return lowmc_s256_126_42_store;
#endif
#if defined(WITH_LOWMC_192_192_4)
  if (lowmc->n == 192 && lowmc->m == 64)
    return lowmc_s256_192_64_store;
#endif
#if defined(WITH_LOWMC_255_255_4)
  if (lowmc->n == 255 && lowmc->m == 85)
    return lowmc_s256_255_85_store;
#endif
  }
#endif
#if defined(WITH_SSE2) || defined(WITH_NEON)
  if (CPU_SUPPORTS_SSE2 || CPU_SUPPORTS_NEON) {
#if defined(WITH_LOWMC_126_126_4)
  if (lowmc->n == 126 && lowmc->m == 42)
    return lowmc_s128_126_42_store;
#endif
#if defined(WITH_LOWMC_192_192_4)
  if (lowmc->n == 192 && lowmc->m == 64)
    return lowmc_s128_192_64_store;
#endif
#if defined(WITH_LOWMC_255_255_4)
  if (lowmc->n == 255 && lowmc->m == 85)
    return lowmc_s128_255_85_store;
#endif
  }
#endif
#endif

#if !defined(NO_UINT64_FALLBACK)
#if defined(WITH_LOWMC_126_126_4)
  if (lowmc->n == 126 && lowmc->m == 42)
    return lowmc_uint64_126_42_store;
#endif
#if defined(WITH_LOWMC_192_192_4)
  if (lowmc->n == 192 && lowmc->m == 64)
    return lowmc_uint64_192_64_store;
#endif
#if defined(WITH_LOWMC_255_255_4)
  if (lowmc->n == 255 && lowmc->m == 85)
    return lowmc_uint64_255_85_store;
#endif
#endif

  return NULL;
}
#endif

#if defined(WITH_KKW)
lowmc_compute_aux_implementation_f lowmc_compute_aux_get_implementation(const lowmc_t* lowmc) {
  ASSUME(lowmc->m == 42 || lowmc->m == 64 || lowmc->m == 85);
  ASSUME(lowmc->n == 126 || lowmc->n == 192 || lowmc->n == 255);

#if defined(WITH_OPT)
#if defined(WITH_AVX2)
  if (CPU_SUPPORTS_AVX2) {
#if defined(WITH_LOWMC_126_126_4)
  if (lowmc->n == 126 && lowmc->m == 42)
    return lowmc_s256_126_42_compute_aux;
#endif
#if defined(WITH_LOWMC_192_192_4)
  if (lowmc->n == 192 && lowmc->m == 64)
    return lowmc_s256_192_64_compute_aux;
#endif
#if defined(WITH_LOWMC_255_255_4)
  if (lowmc->n == 255 && lowmc->m == 85)
    return lowmc_s256_255_85_compute_aux;
#endif
  }
#endif
#if defined(WITH_SSE2) || defined(WITH_NEON)
  if (CPU_SUPPORTS_SSE2 && CPU_SUPPORTS_NEON) {
#if defined(WITH_LOWMC_126_126_4)
  if (lowmc->n == 126 && lowmc->m == 42)
    return lowmc_s128_126_42_compute_aux;
#endif
#if defined(WITH_LOWMC_192_192_4)
  if (lowmc->n == 192 && lowmc->m == 64)
    return lowmc_s128_192_64_compute_aux;
#endif
#if defined(WITH_LOWMC_255_255_4)
  if (lowmc->n == 255 && lowmc->m == 85)
    return lowmc_s128_255_85_compute_aux;
#endif
  }
#endif
#endif

#if !defined(NO_UINT64_FALLBACK)
#if defined(WITH_LOWMC_126_126_4)
  if (lowmc->n == 126 && lowmc->m == 42)
    return lowmc_uint64_126_42_compute_aux;
#endif
#if defined(WITH_LOWMC_192_192_4)
  if (lowmc->n == 192 && lowmc->m == 64)
    return lowmc_uint64_192_64_compute_aux;
#endif
#if defined(WITH_LOWMC_255_255_4)
  if (lowmc->n == 255 && lowmc->m == 85)
    return lowmc_uint64_255_85_compute_aux;
#endif
#endif

  return NULL;
}
#endif
