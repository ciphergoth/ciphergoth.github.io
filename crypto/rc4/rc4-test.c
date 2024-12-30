/*
 *  Written March 1998 by Paul Crowley <paul@hedonism.demon.co.uk>.
 *
 *  This program is placed in the public domain.
 */
static char *rcs_id = "$Id: rc4-test.c 1.1 Sat, 10 Jul 1999 21:28:23 +0100 paul $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

typedef int Bool_t;

typedef unsigned int UInt_t;
typedef signed int Int_t;
typedef unsigned char UByte_t;

static UByte_t s[256];
static const UByte_t *send = s + 256;
static UByte_t y;
static Int_t last_sz;

    static void
init_state(
    char *key
)
{
    UInt_t keylen, i, key_index;
    UByte_t si, perm_index;

    keylen = strlen(key);
    for (i = 0; i < 256; i++) {
        s[i] = (UByte_t) i;
    }
    key_index = 0; perm_index = 0;
    for (i = 0; i < 256; i++) {
	si = s[i];
	perm_index += si + key[key_index];
	s[i] = s[perm_index]; s[perm_index] = si;
	key_index++; key_index %= keylen;
    }
    y = 0;
    last_sz = -1;
}

    static Int_t
make_report(
    Int_t iters
)
{
    UByte_t yy = y;
    Int_t coince = 0, sza = last_sz, i;

    for (i = 0; i < iters; i++) {
        UByte_t z, sx, sy, *xp;
        Int_t szb ;

            /*
             *     x = (x + 1) % 256;
             *     y = (state[x] + y) % 256;
             *     swap_byte(&state[x], &state[y]);
             *     xorIndex = (state[x] + state[y]) % 256;
             *     buffer_ptr[counter] ^= state[xorIndex];
             * 
             *     x = xo + xpp - s
             */
#define RC4_STEP(xpp,xo,rsl,lrs) ( \
                sx = xpp[xo], yy += sx, sy = s[yy], \
                xpp[xo] = sy, s[yy] = sx, \
                z = sx + sy, rsl = s[z], \
                coince += (rsl == lrs))

        RC4_STEP(s,1,szb,sza);
        RC4_STEP(s,2,sza,szb);
        RC4_STEP(s,3,szb,sza);
        for (xp = s + 4; xp < send; xp += 4) {
            RC4_STEP(xp,0,sza,szb);
            RC4_STEP(xp,1,szb,sza);
            RC4_STEP(xp,2,sza,szb);
            RC4_STEP(xp,3,szb,sza);
        }
        RC4_STEP(s,0,sza,szb);
#undef RC4_STEP
    }
    y = yy;
    last_sz = sza;
    return coince - iters;  /* Difference between number of coincidences
                               and expected number */
}

#define MAX_RC4KEY (256)

char rc4_key[MAX_RC4KEY + 1];

    int
main(
    int argc,
    char *argv[]
) {
    UInt_t num_reports, iters, j;
    time_t old, new;

    if (argc != 4) {
        printf("Usage: %s <report-size> <num-reports> <passphrase>\n",
               argv[0]);
        return -1;
    }
    iters = atoi(argv[1]);
    num_reports = atoi(argv[2]);
    sprintf(rc4_key, "%d ", (int) time(NULL));
    strncat(rc4_key, argv[3], MAX_RC4KEY - strlen(rc4_key));
    printf("Version: %s\n"
           "Key: \"%s\"\n"
           "Iters: %d\n"
           "Reports: %d\n", 
           rcs_id, rc4_key, iters, num_reports);
    init_state(rc4_key);
    old = time(NULL);
    printf("Start time: %d\n", (int) old);
        /* 1 << 21 iterations takes about a minute on my machine */
    for (j = 0; j < num_reports; j++) {
        Int_t delta;
        
        delta = make_report(iters);
        new = time(NULL);
        printf("Result: iters=%d time=%d delta=%d\n",
               iters, (int)(new - old), delta);
        old = new;
    }
    return 0;
}
