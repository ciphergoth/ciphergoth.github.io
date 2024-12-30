/*
 *  Written March 1998 by Paul Crowley <paul@hedonism.demon.co.uk>.
 *
 *  This program is placed in the public domain.
 */
static char *rcs_id = "$Id: normal.c 1.1 Sat, 10 Jul 1999 13:06:17 +0100 paul $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

typedef int Bool_t;

typedef unsigned int UInt_t;
typedef signed int Int_t;
typedef unsigned char UByte_t;

#define STEPS (1024)

    int
main(
    int argc,
    char *argv[]
) {
    double x, sum, sample;
    Int_t i, factor;

    x = atof(argv[1]);
    sum = 0;
    for (i = 0; i <= STEPS; i++) {
        if ((i & 1) != 0) {
            factor = 4;
        } else if (i == 0 || i == STEPS) {
            factor = 1;
        } else {
            factor = 2;
        }
        sample = x * i / STEPS;
        sum += factor * exp( sample * sample / -2.0 );
    }
    sum *= x / (STEPS * 3 * sqrt(2 * M_PI));
    printf("Big phi(%g) = %6.4f\n", x, 0.5 + sum);
    printf("1 - Big phi(%g) = %17e\n", x, 0.5 - sum);
    return 0;
}
