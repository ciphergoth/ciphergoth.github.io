/* -*- mode: c -*- */
/* Mercy-6 - a 4096-bit block cipher for disk block encryption
 * 
 * Paul Crowley, 1996 - 2000
 * This software is placed in the public domain.
 *
 * See http://www.cluefactory.org.uk/paul/mercy/ or mail
 * mercy@paul.cluefactory.org.uk */

#ifndef MERCY_H
#define MERCY_H

#define BLOCK_ORDER (6)
#define BLOCK_SIZE (2 << BLOCK_ORDER) /* 128 */
#define BLOCK_MASK (BLOCK_SIZE -1)

#define BLOCK_WORDS BLOCK_SIZE /* 128 */
#define BLOCK_BYTES (BLOCK_WORDS << 2) /* 512 */
#define BLOCK_BITS (BLOCK_BYTES << 3) /* 4096 */

#define HALF_SIZE (1 << BLOCK_ORDER) /* 64 */

#define ROUNDS (6)
#define SPICE_SIZE (ROUNDS * 4)

void crypt_block(
    int direction,
    uint32_t *spice,
    uint32_t *text
);

#define decrypt_block(s,c) (crypt_block(0,s,c))
#define encrypt_block(s,c) (crypt_block(1,s,c))

void schedule_key(
    uint8_t *key,
    int keylen
);

void schedule_stringkey(
    char *key
);

#endif /* MERCY_H */
