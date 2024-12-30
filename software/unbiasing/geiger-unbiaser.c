/* Integer unbiaser version 1.2

   Convert a biased uncorrelated stream of integers (eg, click
   intervals from a Geiger counter) into an unbiased uncorrelated
   bitstream.  See
   
   http://www.ciphergoth.org/software/unbiasing

   Copyright notice and license at end. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if TIME_EXTRACTION
#include <sys/time.h>
#endif

#define MAG_ENTRIES (13)
#define NUM_ENTRIES (1<<MAG_ENTRIES)

char output_array[NUM_ENTRIES * MAG_ENTRIES];
char *output_ptr;

int all_data[NUM_ENTRIES];

char same_entries[NUM_ENTRIES];
char comp_entries[NUM_ENTRIES];

#define SWAP(a, b, t) (((t) = (a)), ((a) = (b)), ((b) = (t)))

void amls_extract(
    char *flips,
    char *flips_end /* Post-ultimate pointer into flips array */
)
{
    char *low = flips;
    char *high = flips_end -1;
    char *doubles = flips;
    
    if (flips_end - flips < 2)
        return;
    do {
        if (*low == *high) {
            *doubles++ = *low;
            *high = '0';
        } else {
            *output_ptr++ = *low;
            *high = '1';
        }
        low++;
        high--;
    } while (high > low);
    amls_extract(flips, doubles);
    amls_extract(high + 1, flips_end);
}

void extract_entropy(
    int *data,
    int *data_end /* Post-ultimate pointer into sample array */
)
{
    int pivot, tmp, *lesser, *equal, *greater;
    char *same_ptr = same_entries;
    char *comp_ptr = comp_entries;
    
    if (data_end - data < 3)
        return;
    pivot = data[0];
    lesser = equal = data + 1;
    greater = data_end;
    for (;;) {
        while (*lesser <= pivot) {
            if (*lesser == pivot) {
                *same_ptr++ = '1'; /* == pivot */
                SWAP(*equal, *lesser, tmp); equal++;
            } else {
                *same_ptr++ = '0'; *comp_ptr++ = '0'; /* < pivot */
            }
            lesser++;
            if (lesser == greater)
                goto partition_finished;
        }
        do {
            *same_ptr++ = '0'; *comp_ptr++ = '1'; /* > pivot */
            greater--;
            if (lesser == greater)
                goto partition_finished;
        } while (*greater > pivot);
        SWAP(*lesser, *greater, tmp);
            /* Note that at this point *lesser <= pivot */
    }
  partition_finished:
    amls_extract(same_entries, same_ptr);
    amls_extract(comp_entries, comp_ptr);
    extract_entropy(equal, lesser);
    extract_entropy(lesser, data_end);
}
    /* This is a three-way "Dutch National Flag" partition, just
       like Quicksort, but with modifications.
       
       - The "pivot" is not included in the partitioned data, it
       is discarded.
       
       - Samples equal to the pivot are moved to the beginning
       
       - The results of each comparison are recorded in
       same_entries, comp_entries (1 if they're the same, (0,0) if
       less than the pivot, (0,1) if greater)
       
       - Care is taken to make sure no element is compared twice
       
       For these reasons, none of the usual tricks to avoid
       doing a comparison on the lesser and greater pointers after
       every increment/decrement can be used; they must be
       compared explicitly every time.
    */

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
        a[i] = (int)(x1 * w * sd + mean + 0.5);
        a[i + 1] = (int)(x2 * w * sd + mean + 0.5);
    }
}

int main(
    int argc,
    char *argv[]
)
{
    int i, bits, total_bits = 0;
#if TIME_EXTRACTION
    struct timeval t_start, t_end;
    double total_worktime = 0, worktime;
#endif

    for (i = 0; i < 100; i++) {
        printf("Run %d: Generating %d samples...\n", i, NUM_ENTRIES);
            /* Mean and SD from HotBits data: hbhist.gif */
        gen_intervals(1889.35, 26.4, all_data, NUM_ENTRIES);
        printf("Extracting bits...\n");
#if TIME_EXTRACTION
        gettimeofday(&t_start, NULL);
#endif
        output_ptr = output_array;
        extract_entropy(all_data, all_data + NUM_ENTRIES);
#if TIME_EXTRACTION
        gettimeofday(&t_end, NULL);
#endif
        bits = output_ptr - output_array;
        printf("Extracted %d bits, %f bits/click\n",
               bits, bits * 1.0 / NUM_ENTRIES);
        total_bits += bits;
#if TIME_EXTRACTION
        worktime = t_end.tv_sec - t_start.tv_sec +
            0.000001 * (t_end.tv_usec - t_start.tv_usec);
        printf("Time: %g seconds, %g / click\n",
               worktime, worktime / NUM_ENTRIES);
        total_worktime += worktime;
#endif
    }
    printf("Totals:\n");
    printf("Extracted total %d bits, %f bits/click\n",
           total_bits, total_bits * 1.0 / (NUM_ENTRIES * i));
#if TIME_EXTRACTION
        printf("Time: %g seconds, %g / click\n",
               total_worktime, total_worktime / (NUM_ENTRIES * i));
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
