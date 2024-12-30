#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


typedef unsigned char byte;

#define WINDOW_SIZE (64)
#define THRESHOLD (52)
#define READ_SIZE (16384)

#define ADDC(c) ((void)((occur_count[c]++)?0:(++distinct)))
#define SUBC(c) ((void)((--occur_count[c])?0:(--distinct)))

void process_he_sequence(
    byte *seq, char *filename, int bof, int distinct
)
{
    int j, sum, ssum;
    
    sum = 0; ssum = 0;
    for (j = 0; j < WINDOW_SIZE; j++) {
        sum += seq[j]; ssum += sum;
        
    }
    printf("%2d (sum %d,%d): byte %10d of \"%s\"\n",
           distinct, ssum, sum, bof, filename);
#if 0
    printf("  %d distinct bytes\n", distinct);
    for (k = 0; k < 8; k++) {
        bit_pop = 0;
        for (j = 0; j < WINDOW_SIZE; j++)
            bit_pop += 1 & (seq[j] >> k);
        printf("  Popularity of bit %d is %d\n", k, bit_pop);
    }
#endif
}


void search_file(
    char *filename, 
    int fd
)
{
    byte rbuf[READ_SIZE + WINDOW_SIZE];
    int occur_count[256];
    int bytes_read, offset, i, distinct;
    byte *bufp, *wend, *b2;
    
    for (i = 0; i < 256; i++)
        occur_count[i] = 0;
    distinct = 0;
    bytes_read = read(fd, rbuf, READ_SIZE);
    if (bytes_read < WINDOW_SIZE) {
        if (bytes_read < 0)
            fprintf(stderr, "Read from file %s failed: %s\n",
                   filename, strerror(errno));
        return;
    }
    offset = 0;
    bufp = rbuf;
    wend = bufp + WINDOW_SIZE;
    for (; bufp < wend; bufp++)
        ADDC(bufp[0]);
    bufp = rbuf;
    wend = bufp + bytes_read - WINDOW_SIZE;
    for (;;) {
        if (distinct >= THRESHOLD)
            process_he_sequence(bufp, filename,
                                bufp - rbuf + offset, distinct);
        if (bufp == wend) {
            b2 = rbuf;
            wend = b2 + WINDOW_SIZE;
            offset += bufp - b2;
            while (b2 < wend)
                *b2++ = *bufp++;
            bytes_read = read(fd, wend, READ_SIZE);
            if (bytes_read <= 0) {
                if (bytes_read < 0)
                    fprintf(stderr, "Read from file %s failed: %s\n",
                           filename, strerror(errno));
                return;
            }
            bufp = rbuf;
            wend = bufp + bytes_read;
        }
        SUBC(bufp[0]);
        ADDC(bufp[WINDOW_SIZE]);
        bufp++;
    }
}
    

int 
main(
    int argc,
    char *argv[]
)
{
    if (argc == 1) {
        search_file("(stdin)", 0);
    } else {
        int i;

        for (i = 1; i < argc; i++) {
            int fd;

            fd = open(argv[i], O_RDONLY);
            if (fd < 0) {
                fprintf(stderr, "Open of file %s failed: %s\n",
                        argv[i], strerror(errno));
                continue ;
            }
            search_file(argv[i], fd);
            if (close(fd) < 0) {
                fprintf(stderr, "Close of file %s failed: %s\n",
                        argv[i], strerror(errno));
            }
        }
    }
    return 0;
}
