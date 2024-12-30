/* Juels, Jakobsson, Shriver, and Hillyer integer unbiaser version 1.4
 
   Convert a biased uncorrelated stream of integers (eg, click
   intervals from a Geiger counter) into an unbiased uncorrelated
   bitstream.  See
   
   http://www.ciphergoth.org/software/unbiasing

   Copyright notice and license at end. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#if TIME_EXTRACTION
#include <sys/time.h>
#endif

#include <gmp.h>

#define MAX_ENTRIES (1<<16)
#define MAXVAL (520)

int all_data[MAX_ENTRIES];

int freq_table[MAXVAL];
int key_rank[MAXVAL];

mpz_t r, s;

unsigned int simple_gcd(
    unsigned int a,
    unsigned int b
)
{
    int two_pow;

    if (a == 0)
        return b;
    if (b == 0)
        return a;
    two_pow = 0;
    while (((a | b) & 1) == 0) {
        a >>= 1;
        b >>= 1;
        two_pow++;
    }
    while ((a & 1) == 0)
        a >>= 1;
    while ((b & 1) == 0)
        b >>= 1;
    for (;;) {
        if (a == b)
            return a << two_pow;
        if (a > b) {
            a -= b;
            while ((a & 1) == 0)
                a >>= 1;
        } else {
            b -= a;
            while ((b & 1) == 0)
                b >>= 1;
        }
    }
}

void q_one(
    mpz_t big_r,
    mpz_t big_s,
    int *data,
    int *data_end
)
{
    int rank;
    int *dp, *fp;
    unsigned int little_l, little_v, little_f, gcd;

    for (dp = key_rank; dp < (key_rank + MAXVAL); dp++)
        *dp = 0;
    for (dp = data; dp < data_end; dp++)
        key_rank[*dp] = 1;
    rank = 0;
    for (dp = key_rank; dp < (key_rank + MAXVAL); dp++) {
        if (*dp != 0) {
            *dp = rank;
            freq_table[rank++] = 0;
        }
    }
    mpz_set_ui(big_r, 0);
    mpz_set_ui(big_s, 1);
    for (dp = data; dp < data_end; dp++) {
        rank = key_rank[*dp];
        little_v = ++freq_table[rank];
        little_l = 0;
        for (fp = freq_table; fp < (freq_table + rank); fp++)
            little_l += *fp;
        little_f = dp - data +1;
        gcd = simple_gcd(little_v, simple_gcd(little_l, little_f));
        if (gcd > 1) {
            little_f /= gcd;
            little_l /= gcd;
            little_v /= gcd;
        }
            /* We know v | Sl, v | Sf
               Let g = gcd(l, f), then v | Sg
               Let h = gcd(v, g), v = v'h, g = g'h
               then v'h | Sg'h, so v' | Sg', gcd(v', g') = 1
               so v' | S so we can do this divexact: */
        if (little_v > 1)
            mpz_divexact_ui(big_s, big_s, little_v);
        if (little_l > 0)
            mpz_addmul_ui(big_r, big_s, little_l);
        mpz_mul_ui(big_s, big_s, little_f);
    }
}

/* Don't bother to extract the bits - just count them! */
int q_two(
    mpz_t r,
    mpz_t s
)
{
    int i, j;
    mp_limb_t s_limb, r_limb;

    for (i = mpz_size(s) -1; i >= 0; i--) {
        s_limb = mpz_getlimbn(s, i);
        r_limb = mpz_getlimbn(r, i);
        s_limb &= ~r_limb;
        if (s_limb != 0) {
            for (j = mp_bits_per_limb -1; j >= 0; j--) {
                if (s_limb & (1 << j))
                    return i * mp_bits_per_limb + j;
            }
        }
    }
    return 0;
}

int extract_entropy(
    int *data,
    int *data_end
)
{
    q_one(r, s, data, data_end);
    return  q_two(r, s);
}

void initialise(
    int samples
)
{
    mpz_init(r);
    mpz_init(s);
}

int itrunc(
    int low,
    int high,
    int val
)
{
    if (val < low)
        return low;
    if (val > high)
        return high;
    return val;
}

void gen_intervals(
    double mean,
    double sd,
    int *a,
    int n  /* Must be even */
)
{
    int i;
    double x1, x2, w;

    for (i = 0; i < n; i += 2) {
            /* Algorithm for generating normally distributed numbers
               from http://www.taygeta.com/random/gaussian.html */
        do {
            x1 = 2.0 * random() / RAND_MAX - 1.0;
            x2 = 2.0 * random() / RAND_MAX - 1.0;
            w = x1 * x1 + x2 * x2;
        } while (w >= 1.0);
        w = sqrt(-2 * log(w) / w);
            /* Convert them to nearest integer;
               assume mean is +ve and much bigger than sd */
        a[i] = itrunc(0, MAXVAL -1, (int)(x1 * w * sd + mean + 0.5));
        a[i + 1] = itrunc(0, MAXVAL -1, (int)(x2 * w * sd + mean + 0.5));
    }
}

int main(
    int argc,
    char *argv[]
)
{
    int samples, i, bits, total_bits = 0;
#if TIME_EXTRACTION
    struct timeval t_start, t_end;
    double total_worktime = 0, worktime;
#endif

    samples = atoi(argv[1]);
    initialise(samples);
    for (i = 0; i < ((samples <= 1024) ? 100 : 20); i++) {
        printf("Run %d: Generating %d samples...\n", i, samples);
            /* SD from HotBits data: hbhist.gif */
        gen_intervals(MAXVAL / 2, 26.4, all_data, samples);
        printf("Extracting bits...\n");
#if TIME_EXTRACTION
        gettimeofday(&t_start, NULL);
#endif
        bits = extract_entropy(all_data, all_data + samples);
#if TIME_EXTRACTION
        gettimeofday(&t_end, NULL);
#endif
        printf("Extracted %d bits, %f bits/click\n",
               bits, bits * 1.0 / samples);
        total_bits += bits;
#if TIME_EXTRACTION
        worktime = t_end.tv_sec - t_start.tv_sec +
            0.000001 * (t_end.tv_usec - t_start.tv_usec);
        printf("Time: %g seconds, %g / click\n",
               worktime, worktime / samples);
        total_worktime += worktime;
#endif
    }
    printf("Totals:\n");
    printf("Extracted total %d bits, %f bits/click\n",
           total_bits, total_bits * 1.0 / (samples * i));
#if TIME_EXTRACTION
        printf("Time: %g seconds, %g / click, %g / bit\n",
               total_worktime,
               total_worktime / (samples * i),
               total_worktime / total_bits);
        printf("PARSEME: engine=jjsh blocksize=%d runs=%d bits=%d seconds=%g\n",
               samples, i, total_bits, total_worktime);
#endif
    return 0;
}


/* Copyright (c) 2002 Paul Crowley <paul@ciphergoth.org>

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE. */
