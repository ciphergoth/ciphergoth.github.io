/* -*- mode: c -*- */
/* Mercy-5 - a 4096-bit block cipher for disk block encryption
 * 
 * Paul Crowley, 1996 - 1999
 * This software is placed in the public domain.
 *
 * See http://www.cluefactory.org.uk/paul/mercy/ or mail
 * mercy@paul.cluefactory.org.uk */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>

#include "mercy.h"

static uint32_t text[BLOCK_SIZE]; /* Sample text to encrypt */
static uint32_t spice[4]; /* Sample spice */

/* This flag is cleared by a signal after N seconds */
static volatile int keep_going;

/* Keep encrypting until the keep_going flag is set */
static int crypt_cycle()
{
    int i;

    for (i = 0; keep_going; i += 2) {
        spice[0] = i;
        encrypt_block(spice, text);
        decrypt_block(spice, text);
    }
    return i;
}

/* Clear the keep_going flag after a times-up signal */
void handle_alarm(
    int signal
)
{
    keep_going = 0;
}

/* Read how long to test for, in seconds, from the command line.
   Crash if that's not there! */

int main(
    int argc,
    char *argv[]
)
{
    int seconds = atoi(argv[1]);
    double blockssec;
    int i;

    schedule_stringkey("Mercy test key");
    for (i = 0; i < BLOCK_SIZE; i++) {
        text[i] = i;
    }
    for (i = 0; i < 4; i++) {
        spice[i] = i;
    }
    printf("Running test for about %d seconds\n", seconds);
    signal(SIGALRM, handle_alarm);
    blockssec = 0;
    for (i = 0; i < seconds; i++) {
        int loops;
        struct timeval start, finish;
        double total_time, t_blockssec;

        keep_going = 1;
        alarm(1);
        gettimeofday(&start, NULL);
        loops = crypt_cycle();
        gettimeofday(&finish, NULL);
        total_time = (finish.tv_sec - start.tv_sec) +
            (finish.tv_usec - start.tv_usec) / 1000000.0;
        t_blockssec = loops / total_time;
        printf("%d: Total time for %d encryptions: %g seconds (%g blocks/sec)\n",
               i, loops, total_time, t_blockssec);
        if (t_blockssec > blockssec) 
            blockssec = t_blockssec;
    }
    printf("Blocks per second: %g\n", blockssec);
    printf("or %g bits/sec %g kbits/sec %g Mbits/sec\n",
           blockssec * BLOCK_BITS, blockssec * BLOCK_BITS / (1<<10),
           blockssec * BLOCK_BITS / (1<<20));
    printf("or %g bytes/sec %g kbytes/sec %g Mbytes/sec\n",
           blockssec * BLOCK_BYTES, blockssec * BLOCK_BYTES / (1<<10),
           blockssec * BLOCK_BYTES / (1<<20));
#ifdef CLOCK_SPEED
    printf("or %g cycles/bit %g cycles/byte %g cycles/word\n",
           CLOCK_SPEED / (blockssec * BLOCK_BITS),
           CLOCK_SPEED / (blockssec * BLOCK_BYTES),
           CLOCK_SPEED / (blockssec * BLOCK_WORDS));
#endif
    
    return 0;
}
