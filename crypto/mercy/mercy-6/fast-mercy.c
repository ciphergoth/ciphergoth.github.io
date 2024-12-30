/* -*- mode: c -*- */
/* Mercy-6 - a 4096-bit block cipher for disk block encryption
 * 
 * Paul Crowley, 1996 - 2000
 * This software is placed in the public domain.
 *
 * See http://www.cluefactory.org.uk/paul/mercy/ or mail
 * mercy@paul.cluefactory.org.uk */

#include <stdint.h>
#include "mercy.h"

static uint32_t t_table[256]; /* The T-box, implemented as a 1k
                                 lookup table */

static uint32_t whitening[2][HALF_SIZE]; /* Partial pre- and
                                            post-whitening */
static uint32_t input_spice[SPICE_SIZE]; /* Input to the F-function
                                            which generates the round
                                            spices */

/* Apply some whitening -  a big unrolled XOR loop. */

static inline void whiten_half(
    uint32_t *b,
    uint32_t *w
)
{
    uint32_t *end = b + HALF_SIZE;

   for (; b < end; b += 8, w += 8) {
        b[0] ^= w[0];        
        b[1] ^= w[1];        
        b[2] ^= w[2];        
        b[3] ^= w[3];        
        b[4] ^= w[4];        
        b[5] ^= w[5];        
        b[6] ^= w[6];        
        b[7] ^= w[7];        
    }
}

/* Operation M - a bijective map from words to words */

static inline uint32_t
M_OP(
    uint32_t x
) {
    return (x << 8) ^ t_table[x >> 24];
}

/* All of these macros assume that you make a variable "tmp"
   available in the local scope for them to use */

/* The basic step of the Q state machine.  Of course we don't really
   rotate the registers; we just use them differently. */
#define STEP(v0,v1,v2,v3,in) \
   (tmp = v2 + M_OP(v0 + (in)), v3 ^= tmp)
/* Step based on drawing input from a particular array */
#define ISTEP(v0,v1,v2,v3,ain,x) \
   STEP(v0,v1,v2,v3,(ain)[x]) 
/* Step based on drawing input from a particular array, and write the
   output to a corresponding position in another one */
#define IOSTEP(v0,v1,v2,v3,ain,aout,x) \
   (STEP(v0,v1,v2,v3,(ain)[x]),(aout)[x] = tmp)
/* As above, but XOR the output with the other array */
#define IXSTEP(v0,v1,v2,v3,ain,aout,x) \
   (STEP(v0,v1,v2,v3,(ain)[x]),(aout)[x] ^= tmp)

/* Some of the steps above repeated four times.  These macros assume
   that the state is stored in variables c0 ... c3 in the local
   scope. These rotate the registers properly. */
#define ISTEP4(ain,x) \
   (ISTEP(c0,c1,c2,c3,ain,x), ISTEP(c3,c0,c1,c2,ain,x+1), \
    ISTEP(c2,c3,c0,c1,ain,x+2), ISTEP(c1,c2,c3,c0,ain,x+3))
#define IOSTEP4(ain,aout,x) \
   (IOSTEP(c0,c1,c2,c3,ain,aout,x), IOSTEP(c3,c0,c1,c2,ain,aout,x+1), \
    IOSTEP(c2,c3,c0,c1,ain,aout,x+2), IOSTEP(c1,c2,c3,c0,ain,aout,x+3))
#define IXSTEP4(ain,aout,x) \
   (IXSTEP(c0,c1,c2,c3,ain,aout,x), IXSTEP(c3,c0,c1,c2,ain,aout,x+1), \
    IXSTEP(c2,c3,c0,c1,ain,aout,x+2), IXSTEP(c1,c2,c3,c0,ain,aout,x+3))

/* Expand the input spice into round spices */

static inline void schedule_spice(
    uint32_t *spice,
    uint32_t *round_spice
)
{
    register uint32_t c0, c1, c2, c3, tmp;

#if ROUNDS != 6
#error "schedule_spice() must be changed if the number of rounds is changed"
#endif
    c0 = input_spice[SPICE_SIZE - 4];
    c1 = input_spice[SPICE_SIZE - 3];
    c2 = input_spice[SPICE_SIZE - 2];
    c3 = input_spice[SPICE_SIZE - 1];
    ISTEP4(spice, 0);
    ISTEP4(input_spice, SPICE_SIZE -8);
    IOSTEP4(input_spice, round_spice, 0);
    IOSTEP4(input_spice, round_spice, 4);
    IOSTEP4(input_spice, round_spice, 8);
    IOSTEP4(input_spice, round_spice, 12);
    IOSTEP4(input_spice, round_spice, 16);
    round_spice[SPICE_SIZE -4] = c0;
    round_spice[SPICE_SIZE -3] = c1;
    round_spice[SPICE_SIZE -2] = c2;
    round_spice[SPICE_SIZE -1] = c3;
}

/* Put the right half through the F function with the appropriate
   round spice, and XOR the left half with the result. */

static inline void round_function(
    uint32_t *lp,
    uint32_t *rp,
    uint32_t *round_spice
)
{
    register uint32_t c0, c1, c2, c3, tmp;

#if HALF_SIZE != 64
#error "round_function() must be changed if the size of the block is changed"
#endif
    c0 = rp[60]; c1 = rp[61]; c2 = rp[62]; c3 = rp[63];
    ISTEP4(round_spice, 0);
    ISTEP4(rp, 56);
    IXSTEP4(rp, lp, 0);
    IXSTEP4(rp, lp, 4);
    IXSTEP4(rp, lp, 8);
    IXSTEP4(rp, lp, 12);
    IXSTEP4(rp, lp, 16);
    IXSTEP4(rp, lp, 20);
    IXSTEP4(rp, lp, 24);
    IXSTEP4(rp, lp, 28);
    IXSTEP4(rp, lp, 32);
    IXSTEP4(rp, lp, 36);
    IXSTEP4(rp, lp, 40);
    IXSTEP4(rp, lp, 44);
    IXSTEP4(rp, lp, 48);
    IXSTEP4(rp, lp, 52);
    IXSTEP4(rp, lp, 56);
    lp[60] ^= c0; lp[61] ^= c1; lp[62] ^= c2; lp[63] ^= c3;
}

/* Encrypt or decrypt a block.  We go to some lengths to make sure the
   round function is only called exactly once anywhere in the code, so
   it can be expanded inline without incurring much cost in code
   expansion. */

void crypt_block(
    int direction, /* 0 = decrypt, 1 = encrypt */
    uint32_t *spice,
    uint32_t *text
)
{
    uint32_t round_spice[SPICE_SIZE];
    uint32_t *rsp, *rsstop;
    uint32_t *lp, *rp, *tmp;
    int rsstep;

    schedule_spice(spice, round_spice);
    if (direction) {
        lp = text; rp = text + HALF_SIZE;
        rsp = round_spice + SPICE_SIZE -4;
        rsstop = round_spice - 4;
        rsstep = -4;
    } else {
        rp = text; lp = text + HALF_SIZE;
        rsp = round_spice;
        rsstop = round_spice + SPICE_SIZE;
        rsstep = 4;
    }
    whiten_half(rp, whitening[direction]);
    for (; rsp != rsstop; rsp += rsstep) {
        round_function(lp, rp, rsp);
        tmp = rp; rp = lp; lp = tmp;
    }
    whiten_half(lp, whitening[!direction]);
}

/* ============================================================ */

/* Key scheduling CPRNG - ARCFOUR in this instance. */

typedef struct KSCPRNG_t 
{
    uint8_t s[256];
    uint8_t x, y;
} KSCPRNG_t;

static uint8_t kscprng_new_byte(
    KSCPRNG_t *state
)
{
    uint8_t sx = state->s[(++state->x) & 0xff];
    uint8_t sy = state->s[(state->y += state->x) & 0xff];

    state->s[state->x] = sy; state->s[state->y] = sx;
    return state->s[(sx + sy) & 0xff];
}

static void kscprng_schedule_key(
    KSCPRNG_t *state,
    uint8_t *key,
    int keylen
)
{
    int i;
    
    for (i = 0; i < 256; i++)
        state->s[i] = (uint8_t) i;
    state->y = 0;
    for (i = 0; i < 256; i++) {
        uint8_t si = state->s[i];
        
        state->y += si + key[i % keylen];
        state->s[i] = state->s[state->y];
        state->s[state->y] = si;
    }
    state->x = state->y = 0;
        /* Discard 256 bytes of output to avoid weak keys */
    for (i = 0; i < 256; i++)
        kscprng_new_byte(state);
}

/* Key schedule itself */

#define POLY_BASE (0x11B)
#define XTIME(x) ((x << 1) ^ ((x & 0x80) ? POLY_BASE : 0))

/* Generate the N function used for nonlinear substitution, based on
   multiplicative inverses in GF(2^8).  Basically much like
   Rijndael. */

static void make_sbox(
    uint8_t f[256]
)
{
    uint8_t pow_tab[256], log_tab[256];
    int i, xi;

    xi = 1;
    for (i = 0; i < 256; i++) {
        pow_tab[i] = (uint8_t) xi;
        log_tab[xi] = (uint8_t) i;
        xi ^= XTIME(xi);
    }
    log_tab[1] = 0;
    f[0] = 0;
    for(i = 1; i < 256; i++) {   
        f[i] = (uint8_t) pow_tab[255 - log_tab[i]];
    }
}

/* Use the CPRNG to generate a function drawn fairly at random from
   the space of bijective affine mappings on addition in GF(2^8). */

static void make_rand_affinefunc(
    KSCPRNG_t *state,
    uint8_t f[256]
) 
{
    int i, j, t;

    f[0] = kscprng_new_byte(state);
    i = 1;
    while (i < 256) {
        t = kscprng_new_byte(state);
        for (j = i -1; j >= 0 && t != f[j]; j--)
            ; /* empty */
        if (j < 0) {
            t ^= f[0];
            for (j = 0; j < i; j++)
                f[i + j] = t ^ f[j];
            i <<= 1;
        }
    }
}

/* Fill a buffer of words with random values */

static void random_fill(
    KSCPRNG_t *state,
    uint32_t *b,
    int blen
)
{
  uint32_t tmp;
  int i, j;

  for (i = 0; i < blen; i++) {
    tmp = 0;
    for (j = 0; j < 32; j += 8) {
      tmp += kscprng_new_byte(state) << j;
    }
    b[i] = tmp;
  }
}

/* Schedule the key given into the t_table, the input_spice, and the
   whitening. */

void schedule_key(
    uint8_t *key,
    int keylen
)
{
    KSCPRNG_t state;
    uint8_t pre[256], sbox[256], post[256];
    int i, j;

    make_sbox(sbox);
    kscprng_schedule_key(&state, key, keylen);
    for (j = 0; j < 256; j++)
        t_table[j] = 0;
    for (i = 0; i < 32; i += 8) {
        make_rand_affinefunc(&state, pre);
        make_rand_affinefunc(&state, post);
        for (j = 0; j < 256; j++)
            t_table[j] |= ((uint32_t) post[sbox[pre[j]]]) << i;
    }
    random_fill(&state, input_spice, SPICE_SIZE);
    random_fill(&state, whitening[0], HALF_SIZE);
    random_fill(&state, whitening[1], HALF_SIZE);
}

/* Schedule a key which is a string; simple convenience function. */

void schedule_stringkey(
    char *key
)
{
    schedule_key((uint8_t *) key, strlen(key));
}
