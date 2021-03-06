/*
    Copyright (C) 2017 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include "flint.h"
#include "fmpz.h"
#include "fmpz_mpoly.h"
#include "ulong_extras.h"
#include "profiler.h"

int
main(void)
{
    int i, j, result;

    FLINT_TEST_INIT(state);


    flint_printf("quasidivrem_heap....");
    fflush(stdout);

    /* Check f*g/g = f */
    for (i = 0; i < 10 * flint_test_multiplier(); i++)
    {
        fmpz_mpoly_ctx_t ctx;
        fmpz_mpoly_t f, g, h, r1, q1;
        fmpz_t s1;
        ordering_t ord;
        slong nvars, len, len1, len2, exp_bound, exp_bound1, exp_bound2;
        slong coeff_bits, exp_bits, exp_bits1, exp_bits2;

        ord = mpoly_ordering_randtest(state);
        nvars = n_randint(state, 10) + 1;

        fmpz_mpoly_ctx_init(ctx, nvars, ord);

        fmpz_mpoly_init(f, ctx);
        fmpz_mpoly_init(g, ctx);
        fmpz_mpoly_init(h, ctx);
        fmpz_mpoly_init(q1, ctx);
        fmpz_mpoly_init(r1, ctx);
        fmpz_init(s1);

        len = n_randint(state, 100);
        len1 = n_randint(state, 100) + 50;
        len2 = n_randint(state, 100) + 50;

        exp_bits = n_randint(state, FLINT_BITS - 1 - 
                  mpoly_ordering_isdeg(ord)*FLINT_BIT_COUNT(nvars)) + 1;
        exp_bits1 = n_randint(state, FLINT_BITS - 2 - 
                  mpoly_ordering_isdeg(ord)*FLINT_BIT_COUNT(nvars)) + 1;
        exp_bits2 = n_randint(state, FLINT_BITS - 2 - 
                  mpoly_ordering_isdeg(ord)*FLINT_BIT_COUNT(nvars)) + 1;

        exp_bound = n_randbits(state, exp_bits);
        exp_bound1 = n_randbits(state, exp_bits1);
        exp_bound2 = n_randbits(state, exp_bits2);

        coeff_bits = n_randint(state, 200);

        for (j = 0; j < 4; j++)
        {
            fmpz_mpoly_randtest(q1, state, len, exp_bound, coeff_bits, ctx);
            fmpz_mpoly_randtest(r1, state, len, exp_bound, coeff_bits, ctx);
            fmpz_mpoly_randtest(f, state, len1, exp_bound1, coeff_bits, ctx);
            do {
                fmpz_mpoly_randtest(g, state, len2, exp_bound2, coeff_bits + 1, ctx);
            } while (g->length == 0);

            fmpz_mpoly_mul_johnson(h, f, g, ctx);
            fmpz_mpoly_quasidivrem_heap(s1, q1, r1, h, g, ctx);
            fmpz_mpoly_test(q1, ctx);
            fmpz_mpoly_test(r1, ctx);
            fmpz_mpoly_remainder_strongtest(r1, g, ctx);

            result = fmpz_equal_ui(s1, WORD(1)) && fmpz_mpoly_equal(q1, f, ctx);

            if (!result)
            {
                printf("FAIL1\n");
                printf("Check f*g/g = f\n");
                flint_printf("i=%wd j=%wd\n",i,j);
                flint_abort();
            }
        }

        fmpz_mpoly_clear(f, ctx);
        fmpz_mpoly_clear(g, ctx);
        fmpz_mpoly_clear(h, ctx);
        fmpz_mpoly_clear(q1, ctx);
        fmpz_mpoly_clear(r1, ctx);  
        fmpz_clear(s1);
    }

    /* Check aliasing of quotient with first argument */
    for (i = 0; i < 5 * flint_test_multiplier(); i++)
    {
        fmpz_mpoly_ctx_t ctx;
        fmpz_mpoly_t f, g, h, k, r1, t1, t2;
        fmpz_t s1, s2;
        ordering_t ord;
        slong nvars, len, len1, len2, exp_bound, exp_bound1, exp_bound2;
        slong coeff_bits, exp_bits, exp_bits1, exp_bits2;

        ord = mpoly_ordering_randtest(state);
        nvars = n_randint(state, 10) + 1;

        fmpz_mpoly_ctx_init(ctx, nvars, ord);

        fmpz_init(s1);
        fmpz_init(s2);

        fmpz_mpoly_init(f, ctx);
        fmpz_mpoly_init(g, ctx);
        fmpz_mpoly_init(h, ctx);
        fmpz_mpoly_init(k, ctx);
        fmpz_mpoly_init(r1, ctx);
        fmpz_mpoly_init(t1, ctx);
        fmpz_mpoly_init(t2, ctx);

        len = n_randint(state, 10);
        len1 = n_randint(state, 20);
        len2 = n_randint(state, 10) + 1;

        exp_bits = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;
        exp_bits1 = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;
        exp_bits2 = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;

        exp_bound = n_randbits(state, exp_bits);
        exp_bound1 = n_randbits(state, exp_bits1);
        exp_bound2 = n_randbits(state, exp_bits2);

        coeff_bits = n_randint(state, 50);

        for (j = 0; j < 4; j++)
        {
            fmpz_mpoly_randtest(f, state, len1, exp_bound1, coeff_bits, ctx);
            do {
                fmpz_mpoly_randtest(g, state, len2, exp_bound2, coeff_bits + 1, ctx);
            } while (g->length == 0);
            fmpz_mpoly_randtest(h, state, len, exp_bound, coeff_bits, ctx);
            fmpz_mpoly_randtest(k, state, len, exp_bound, coeff_bits, ctx);
            fmpz_mpoly_randtest(r1, state, len, exp_bound, coeff_bits, ctx);

            fmpz_mpoly_mul_johnson(h, f, g, ctx);

            fmpz_mpoly_quasidivrem_heap(s1, h, r1, f, g, ctx);
            fmpz_mpoly_test(h, ctx);
            fmpz_mpoly_test(r1, ctx);
            fmpz_mpoly_remainder_strongtest(r1, g, ctx);

            fmpz_mpoly_mul_johnson(t1, h, g, ctx);
            fmpz_mpoly_add(t1, t1, r1, ctx);
            fmpz_mpoly_scalar_mul_fmpz(t2, f, s1, ctx);

            fmpz_mpoly_quasidivrem_heap(s2, f, k, f, g, ctx);
            fmpz_mpoly_test(k, ctx);
            fmpz_mpoly_test(f, ctx);

            result = fmpz_mpoly_equal(t1, t2, ctx)
                    && fmpz_mpoly_equal(h, f, ctx)
                    && fmpz_mpoly_equal(r1, k, ctx);

            if (!result)
            {
                printf("FAIL\n");
                printf("Check aliasing of quotient with first argument\n");
                flint_printf("i = %wd  j = %wd\n");
                flint_abort();
            }
        }

        fmpz_clear(s1);
        fmpz_clear(s2);
        fmpz_mpoly_clear(f, ctx);  
        fmpz_mpoly_clear(g, ctx);  
        fmpz_mpoly_clear(h, ctx);  
        fmpz_mpoly_clear(k, ctx);  
        fmpz_mpoly_clear(r1, ctx);  
        fmpz_mpoly_clear(t1, ctx);
        fmpz_mpoly_clear(t2, ctx);
    }

    /* Check aliasing of quotient with second argument */
    for (i = 0; i < 5 * flint_test_multiplier(); i++)
    {
        fmpz_mpoly_ctx_t ctx;
        fmpz_mpoly_t f, g, h, k, r1, t1, t2;
        fmpz_t s1, s2;
        ordering_t ord;
        slong nvars, len, len1, len2, exp_bound, exp_bound1, exp_bound2;
        slong coeff_bits, exp_bits, exp_bits1, exp_bits2;

        ord = mpoly_ordering_randtest(state);
        nvars = n_randint(state, 10) + 1;

        fmpz_mpoly_ctx_init(ctx, nvars, ord);

        fmpz_init(s1);
        fmpz_init(s2);

        fmpz_mpoly_init(f, ctx);
        fmpz_mpoly_init(g, ctx);
        fmpz_mpoly_init(h, ctx);
        fmpz_mpoly_init(k, ctx);
        fmpz_mpoly_init(r1, ctx);
        fmpz_mpoly_init(t1, ctx);
        fmpz_mpoly_init(t2, ctx);

        len = n_randint(state, 10);
        len1 = n_randint(state, 20);
        len2 = n_randint(state, 10) + 1;

        exp_bits = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;
        exp_bits1 = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;
        exp_bits2 = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;

        exp_bound = n_randbits(state, exp_bits);
        exp_bound1 = n_randbits(state, exp_bits1);
        exp_bound2 = n_randbits(state, exp_bits2);

        coeff_bits = n_randint(state, 50);

        for (j = 0; j < 4; j++)
        {
            fmpz_mpoly_randtest(f, state, len1, exp_bound1, coeff_bits, ctx);
            do {
                fmpz_mpoly_randtest(g, state, len2, exp_bound2, coeff_bits + 1, ctx);
            } while (g->length == 0);
            fmpz_mpoly_randtest(h, state, len, exp_bound, coeff_bits, ctx);
            fmpz_mpoly_randtest(k, state, len, exp_bound, coeff_bits, ctx);
            fmpz_mpoly_randtest(r1, state, len, exp_bound, coeff_bits, ctx);

            fmpz_mpoly_mul_johnson(h, f, g, ctx);

            fmpz_mpoly_quasidivrem_heap(s1, h, r1, f, g, ctx);
            fmpz_mpoly_test(h, ctx);
            fmpz_mpoly_test(r1, ctx);
            fmpz_mpoly_remainder_strongtest(r1, g, ctx);

            fmpz_mpoly_mul_johnson(t1, h, g, ctx);
            fmpz_mpoly_add(t1, t1, r1, ctx);
            fmpz_mpoly_scalar_mul_fmpz(t2, f, s1, ctx);

            fmpz_mpoly_quasidivrem_heap(s2, g, k, f, g, ctx);
            fmpz_mpoly_test(k, ctx);
            fmpz_mpoly_test(f, ctx);

            result = fmpz_mpoly_equal(t1, t2, ctx)
                    && fmpz_mpoly_equal(h, g, ctx)
                    && fmpz_mpoly_equal(r1, k, ctx);

            if (!result)
            {
                printf("FAIL\n");
                printf("Check aliasing of quotient with second argument\n");
                flint_printf("i = %wd  j = %wd\n");
                flint_abort();
            }
        }

        fmpz_clear(s1);
        fmpz_clear(s2);
        fmpz_mpoly_clear(f, ctx);  
        fmpz_mpoly_clear(g, ctx);  
        fmpz_mpoly_clear(h, ctx);  
        fmpz_mpoly_clear(k, ctx);  
        fmpz_mpoly_clear(r1, ctx);  
        fmpz_mpoly_clear(t1, ctx);
        fmpz_mpoly_clear(t2, ctx);
    }
	
    /* Check aliasing of remainder with first argument */
    for (i = 0; i < 5 * flint_test_multiplier(); i++)
    {
        fmpz_mpoly_ctx_t ctx;
        fmpz_mpoly_t f, g, h, k, r1, t1, t2;
        fmpz_t s1, s2;
        ordering_t ord;
        slong nvars, len, len1, len2, exp_bound, exp_bound1, exp_bound2;
        slong coeff_bits, exp_bits, exp_bits1, exp_bits2;

        ord = mpoly_ordering_randtest(state);
        nvars = n_randint(state, 10) + 1;

        fmpz_mpoly_ctx_init(ctx, nvars, ord);

        fmpz_init(s1);
        fmpz_init(s2);

        fmpz_mpoly_init(f, ctx);
        fmpz_mpoly_init(g, ctx);
        fmpz_mpoly_init(h, ctx);
        fmpz_mpoly_init(k, ctx);
        fmpz_mpoly_init(r1, ctx);
        fmpz_mpoly_init(t1, ctx);
        fmpz_mpoly_init(t2, ctx);

        len = n_randint(state, 10);
        len1 = n_randint(state, 20);
        len2 = n_randint(state, 10) + 1;

        exp_bits = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;
        exp_bits1 = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;
        exp_bits2 = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;

        exp_bound = n_randbits(state, exp_bits);
        exp_bound1 = n_randbits(state, exp_bits1);
        exp_bound2 = n_randbits(state, exp_bits2);

        coeff_bits = n_randint(state, 50);

        for (j = 0; j < 4; j++)
        {
            fmpz_mpoly_randtest(f, state, len1, exp_bound1, coeff_bits, ctx);
            do {
                fmpz_mpoly_randtest(g, state, len2, exp_bound2, coeff_bits + 1, ctx);
            } while (g->length == 0);
            fmpz_mpoly_randtest(h, state, len, exp_bound, coeff_bits, ctx);
            fmpz_mpoly_randtest(k, state, len, exp_bound, coeff_bits, ctx);
            fmpz_mpoly_randtest(r1, state, len, exp_bound, coeff_bits, ctx);

            fmpz_mpoly_mul_johnson(h, f, g, ctx);

            fmpz_mpoly_quasidivrem_heap(s1, h, r1, f, g, ctx);
            fmpz_mpoly_test(h, ctx);
            fmpz_mpoly_test(r1, ctx);
            fmpz_mpoly_remainder_strongtest(r1, g, ctx);

            fmpz_mpoly_mul_johnson(t1, h, g, ctx);
            fmpz_mpoly_add(t1, t1, r1, ctx);
            fmpz_mpoly_scalar_mul_fmpz(t2, f, s1, ctx);

            fmpz_mpoly_quasidivrem_heap(s2, k, f, f, g, ctx);
            fmpz_mpoly_test(k, ctx);
            fmpz_mpoly_test(f, ctx);

            result = fmpz_mpoly_equal(t1, t2, ctx)
                    && fmpz_mpoly_equal(h, k, ctx)
                    && fmpz_mpoly_equal(r1, f, ctx);

            if (!result)
            {
                printf("FAIL\n");
                printf("Check aliasing of remainder with first argument\n");
                flint_printf("i = %wd  j = %wd\n");
                flint_abort();
            }
        }

        fmpz_clear(s1);
        fmpz_clear(s2);
        fmpz_mpoly_clear(f, ctx);  
        fmpz_mpoly_clear(g, ctx);  
        fmpz_mpoly_clear(h, ctx);  
        fmpz_mpoly_clear(k, ctx);  
        fmpz_mpoly_clear(r1, ctx);  
        fmpz_mpoly_clear(t1, ctx);
        fmpz_mpoly_clear(t2, ctx);
    }

    /* Check aliasing of remainder with second argument */
    for (i = 0; i < 5 * flint_test_multiplier(); i++)
    {
        fmpz_mpoly_ctx_t ctx;
        fmpz_mpoly_t f, g, h, k, r1, t1, t2;
        fmpz_t s1, s2;
        ordering_t ord;
        slong nvars, len, len1, len2, exp_bound, exp_bound1, exp_bound2;
        slong coeff_bits, exp_bits, exp_bits1, exp_bits2;

        ord = mpoly_ordering_randtest(state);
        nvars = n_randint(state, 10) + 1;

        fmpz_mpoly_ctx_init(ctx, nvars, ord);

        fmpz_init(s1);
        fmpz_init(s2);

        fmpz_mpoly_init(f, ctx);
        fmpz_mpoly_init(g, ctx);
        fmpz_mpoly_init(h, ctx);
        fmpz_mpoly_init(k, ctx);
        fmpz_mpoly_init(r1, ctx);
        fmpz_mpoly_init(t1, ctx);
        fmpz_mpoly_init(t2, ctx);

        len = n_randint(state, 10);
        len1 = n_randint(state, 20);
        len2 = n_randint(state, 10) + 1;

        exp_bits = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;
        exp_bits1 = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;
        exp_bits2 = n_randint(state, 14/(nvars + 
                            mpoly_ordering_isdeg(ord) + (nvars == 1)) + 1) + 1;

        exp_bound = n_randbits(state, exp_bits);
        exp_bound1 = n_randbits(state, exp_bits1);
        exp_bound2 = n_randbits(state, exp_bits2);

        coeff_bits = n_randint(state, 50);

        for (j = 0; j < 4; j++)
        {
            fmpz_mpoly_randtest(f, state, len1, exp_bound1, coeff_bits, ctx);
            do {
                fmpz_mpoly_randtest(g, state, len2, exp_bound2, coeff_bits + 1, ctx);
            } while (g->length == 0);
            fmpz_mpoly_randtest(h, state, len, exp_bound, coeff_bits, ctx);
            fmpz_mpoly_randtest(k, state, len, exp_bound, coeff_bits, ctx);
            fmpz_mpoly_randtest(r1, state, len, exp_bound, coeff_bits, ctx);

            fmpz_mpoly_mul_johnson(h, f, g, ctx);

            fmpz_mpoly_quasidivrem_heap(s1, h, r1, f, g, ctx);
            fmpz_mpoly_test(h, ctx);
            fmpz_mpoly_test(r1, ctx);
            fmpz_mpoly_remainder_strongtest(r1, g, ctx);

            fmpz_mpoly_mul_johnson(t1, h, g, ctx);
            fmpz_mpoly_add(t1, t1, r1, ctx);
            fmpz_mpoly_scalar_mul_fmpz(t2, f, s1, ctx);

            fmpz_mpoly_quasidivrem_heap(s2, k, g, f, g, ctx);
            fmpz_mpoly_test(k, ctx);
            fmpz_mpoly_test(g, ctx);

            result = fmpz_mpoly_equal(t1, t2, ctx)
                    && fmpz_mpoly_equal(h, k, ctx)
                    && fmpz_mpoly_equal(r1, g, ctx);

            if (!result)
            {
                printf("FAIL\n");
                printf("Check aliasing of remainder with second argument\n");
                flint_printf("i = %wd  j = %wd\n");
                flint_abort();
            }
        }

        fmpz_clear(s1);
        fmpz_clear(s2);
        fmpz_mpoly_clear(f, ctx);  
        fmpz_mpoly_clear(g, ctx);  
        fmpz_mpoly_clear(h, ctx);  
        fmpz_mpoly_clear(k, ctx);  
        fmpz_mpoly_clear(r1, ctx);  
        fmpz_mpoly_clear(t1, ctx);
        fmpz_mpoly_clear(t2, ctx);
    }


    FLINT_TEST_CLEANUP(state);
    
    flint_printf("PASS\n");
    return 0;
}

