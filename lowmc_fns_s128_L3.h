/*
 *  This file is part of the optimized implementation of the Picnic signature scheme.
 *  See the accompanying documentation for complete details.
 *
 *  The code is provided under the MIT license, see LICENSE for
 *  more details.
 *  SPDX-License-Identifier: MIT
 */

#include "lowmc_fns_undef.h"

#define ADDMUL mzd_addmul_v_s128_192
#define MUL mzd_mul_v_s128_192
#define XOR mzd_xor_s128_256
#define COPY mzd_copy_s128_256
#define MPC_MUL mpc_matrix_mul_s128_192

#if defined(WITH_LOWMC_192_192_4)
#define LOWMC_INSTANCE lowmc_192_192_4
#endif
#define LOWMC_N LOWMC_L3_N
#define LOWMC_R LOWMC_L3_R
#define LOWMC_M LOWMC_L3_M
